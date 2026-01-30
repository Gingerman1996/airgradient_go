#include "bq25629.h"
#include "cap1203.h"
#include "gps.h"
#include "log_storage.h"
#include "lp5036.h"
#include "color_utils.h"
#include "led_effects.h"
#include "sensor.h"
#include "ui_display.h"
#include "i2c_scanner.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "epaper_config.h"
#include "epaper_lvgl.h"
#include "epaper_panel.h"
#include "lvgl.h"

#include <string.h>

// Display resolution selector
// setResolution = 0 -> 144x296 (GDEY0213B74H 2.13")
// setResolution = 1 -> 122x250 (GDEY0213B74 2.13" standard)
#define setResolution 1

#if setResolution == 0
// GDEY0213B74H (2.13" 144x296) - uses 2.9" panel config (closest match)
#define DISPLAY_WIDTH 144
#define DISPLAY_HEIGHT 296
#define DISPLAY_PANEL_TYPE EPD_PANEL_SSD16XX_290
#elif setResolution == 1
// GDEY0213B74 (2.13" 122x250) - standard 2.13" panel config
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 254
#define DISPLAY_PANEL_TYPE EPD_PANEL_SSD16XX_213
#else
#error "setResolution must be 0 or 1"
#endif

// Hardware panel width (144 is already byte-aligned, multiple of 8)
#define DISPLAY_HW_WIDTH DISPLAY_WIDTH

#define DISPLAY_ROT_WIDTH DISPLAY_HEIGHT
#define DISPLAY_ROT_HEIGHT DISPLAY_WIDTH

// LED configuration
// LED_ENABLED = 0 -> LED bar disabled (saves power)
// LED_ENABLED = 1 -> LED bar enabled (shows air quality)
#define LED_ENABLED 0

// LVGL configuration
#define LVGL_TICK_PERIOD_MS 5
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 10
#define LVGL_REFRESH_MIN_INTERVAL_MS 1500

// QON button configuration (GPIO5)
#define QON_PIN GPIO_NUM_5
#define SHUTDOWN_HOLD_MS 5000 // 5 seconds long press to shutdown
// External hardware watchdog (TPL5010)
#define HW_WD_RST_PIN GPIO_NUM_2
#define HW_WD_PULSE_MS 20
#define HW_WD_KICK_INTERVAL_MS (4 * 60 * 1000ULL) // Kick before 5 min timeout

static SemaphoreHandle_t lvgl_mux = NULL;
static bool shutdown_requested = false;
static volatile bool display_task_running = true;
static Display *g_display = nullptr;
static epd_handle_t g_epd = nullptr;
static lv_display_t *g_disp = nullptr;
static drivers::BQ25629 *g_charger = nullptr;
static drivers::LP5036 *g_led_driver = nullptr;
static CAP1203 *g_buttons = nullptr;
static GPS *g_gps = nullptr;
static Sensors *g_sensors = nullptr;
static TaskHandle_t g_display_task_handle = nullptr;
static Display::FocusTile g_focus_tile = Display::FocusTile::CO2;
static volatile bool g_focus_dirty = true;
static volatile bool g_lvgl_refresh_requested = false;
static volatile bool g_lvgl_refresh_urgent = false;
static TaskHandle_t g_lvgl_task_handle = nullptr;
static SemaphoreHandle_t g_display_data_mux = nullptr;

static const uint64_t kRecordingIntervalMs =
    1000; // 1s - update display every 1 second
static const uint64_t kUiBlinkIntervalMs = UI_BLINK_INTERVAL_MS;
static const float kBatteryCapacityMah = 2000.0f;
static const uint16_t kBatteryEmptyMv = 3300;
static const uint16_t kBatteryFullMv = 4200;
static const int16_t kBatteryRelaxedCurrentMa = 50;

// Global battery SOC state for persistence across restarts and shutdown
struct BatterySOCState {
  float soc_pct = 0.0f;
  uint16_t last_vbat_mv = 0;
  bool initialized = false;
  SemaphoreHandle_t mutex = nullptr;
};

static BatterySOCState g_battery_soc = {};

struct DisplaySnapshot {
  sensor_values_t sensor = {};
  bool sensor_valid = false;
  uint64_t sensor_update_ms = 0;
  int battery_percent = 0;
  bool battery_charging = false;
  bool battery_valid = false;
  Display::GPSStatus gps_status = Display::GPSStatus::Off;
  bool gps_time_valid = false;
  int gps_hour = 0;
  int gps_min = 0;
  float gps_lat = 0.0f;
  float gps_lon = 0.0f;
  bool gps_fix_valid = false;
};

static DisplaySnapshot g_display_snapshot = {};

static int rand_range_int(int min_val, int max_val) {
  if (max_val <= min_val) return min_val;
  uint32_t span = static_cast<uint32_t>(max_val - min_val + 1);
  return min_val + static_cast<int>(esp_random() % span);
}

static float rand_range_1dp(int min_tenths, int max_tenths) {
  int v = rand_range_int(min_tenths, max_tenths);
  return static_cast<float>(v) / 10.0f;
}

static void apply_random_display(Display *display) {
  if (!display) return;
  display->setWiFiStatus(Display::WiFiStatus::Connected);
  display->setGPSStatus(Display::GPSStatus::Fix);
  display->setBLEStatus(Display::BLEStatus::Connected);
  display->setRecording(false);
  display->setBattery(rand_range_int(20, 100), false);
  display->setFocusTile(Display::FocusTile::CO2);

  for (int i = 0; i < 30; i += 1) {
    display->setCO2(rand_range_int(400, 2000));
  }

  display->setPM25f(rand_range_1dp(0, 2500));
  display->setTempCf(rand_range_1dp(180, 320));
  display->setRHf(rand_range_1dp(300, 800));
  display->setVOC(rand_range_int(0, 500));
  display->setNOx(rand_range_int(0, 300));
  display->setPressure(rand_range_int(980, 1030));
  display->setTimeHM(rand_range_int(0, 23), rand_range_int(0, 59), true);
}

// Flag set by T1 button to trigger random display refresh
static volatile bool g_static_test_refresh_requested = false;

enum class AirLevel : uint8_t {
  Off = 0,
  Green,
  Yellow,
  Orange,
  Red,
  Purple,
  PurpleRed,
};

static const char *charge_status_to_string(drivers::ChargeStatus status);
static const char *antenna_status_to_string(GPS::AntennaStatus status);
static int battery_percent_from_mv(uint16_t vbat_mv);
static int battery_percent_from_soc(float soc_pct);
static esp_err_t battery_soc_nvs_init(void);
static esp_err_t battery_soc_nvs_load(float *soc_pct, uint16_t *last_vbat_mv);
static esp_err_t battery_soc_nvs_save(float soc_pct, uint16_t vbat_mv);
static AirLevel pm25_level_from_ugm3(float pm25_ugm3);
static AirLevel co2_level_from_ppm(int co2_ppm);
static void led_show_air_levels(AirLevel pm_level, AirLevel co2_level);
static void led_show_battery(int battery_percent, bool charging);
static void led_off();
static void button_event_callback(ButtonState state, void *user_data);
static esp_err_t init_cap1203(i2c_master_bus_handle_t i2c_bus);
static void lv_tick_timer_cb(void *arg);
static bool lvgl_lock(int timeout_ms);
static void lvgl_unlock(void);
static void request_lvgl_refresh(void);
static void request_lvgl_refresh_urgent(void);
static void initiate_shutdown(void);
static void qon_button_task(void *arg);
static void lv_handler_task(void *arg);
static void display_task(void *arg);

// Application entry point.
// Phase 1.1: Dashboard display with real sensor data
extern "C" void app_main(void) {
  static const char *TAG = "app_main";
  esp_log_level_set(TAG, ESP_LOG_DEBUG);

  ESP_LOGI(TAG, "=== AirGradient GO - Phase 1.1 ===");
  ESP_LOGI(TAG, "Initializing display and sensors...");

  // ==================== NVS INITIALIZATION ====================
  
  // Initialize NVS for battery SOC persistence
  esp_err_t nvs_ret = battery_soc_nvs_init();
  if (nvs_ret != ESP_OK) {
    ESP_LOGW(TAG, "NVS init failed: %s (battery SOC persistence disabled)", 
             esp_err_to_name(nvs_ret));
  } else {
    ESP_LOGI(TAG, "NVS initialized for battery SOC storage");
  }

  // Create battery SOC mutex
  g_battery_soc.mutex = xSemaphoreCreateMutex();
  if (g_battery_soc.mutex == NULL) {
    ESP_LOGE(TAG, "Failed to create battery SOC mutex");
    return;
  }

  // ==================== GPIO INITIALIZATION ======================================

  // Configure QON button (GPIO5) as input with pull-up
  gpio_config_t qon_cfg = {
      .pin_bit_mask = (1ULL << QON_PIN),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  ESP_ERROR_CHECK(gpio_config(&qon_cfg));
  ESP_LOGI(TAG, "QON button (GPIO%d) configured", QON_PIN);

  // Configure external watchdog reset (TPL5010 DONE pulse, active low)
  gpio_config_t wd_cfg = {
      .pin_bit_mask = (1ULL << HW_WD_RST_PIN),
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE,
  };
  ESP_ERROR_CHECK(gpio_config(&wd_cfg));
  gpio_set_level(HW_WD_RST_PIN, 1);
  ESP_LOGI(TAG, "HW watchdog reset configured on GPIO%d", HW_WD_RST_PIN);

  // ==================== DISPLAY INITIALIZATION ====================

  // Initialize LVGL
  lv_init();

  // Configure e-paper display (hardware width is byte-aligned for driver)
  // MISO is set to GPIO24 to share SPI bus with W25N512 NAND flash
  epd_config_t epd_cfg = EPD_CONFIG_DEFAULT();
  epd_cfg.pins.busy = 10; // IO10
  epd_cfg.pins.rst = 9;   // IO9
  epd_cfg.pins.dc = 15;   // IO15
  epd_cfg.pins.cs = 0;    // IO0
  epd_cfg.pins.sck = 23;  // IO23
  epd_cfg.pins.mosi = 25; // IO25
  epd_cfg.pins.miso = 24; // IO24 - shared with NAND flash
  epd_cfg.spi.host = SPI2_HOST;
  epd_cfg.spi.speed_hz = 4000000; // 4 MHz (same as working ESP32-C6 code)
  epd_cfg.panel.type = DISPLAY_PANEL_TYPE; // Panel type based on resolution
  epd_cfg.panel.width = DISPLAY_HW_WIDTH;
  epd_cfg.panel.height = DISPLAY_HEIGHT;
  epd_cfg.panel.mirror_x = false;
  epd_cfg.panel.mirror_y = false;
  epd_cfg.panel.rotation = 0;

  esp_err_t ret = epd_init(&epd_cfg, &g_epd);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to init e-paper: %s", esp_err_to_name(ret));
    return;
  }
  epd_handle_t epd = g_epd; // Local alias for compatibility

  // Get panel info
  epd_panel_info_t panel_info;
  epd_get_info(epd, &panel_info);
  ESP_LOGI(TAG, "Panel: %dx%d, buffer: %lu bytes", panel_info.width,
           panel_info.height, panel_info.buffer_size);

  // Initialize LVGL display with PARTIAL refresh for faster updates
  epd_lvgl_config_t lvgl_cfg = EPD_LVGL_CONFIG_DEFAULT();
  lvgl_cfg.epd = epd;
  lvgl_cfg.update_mode =
      EPD_UPDATE_PARTIAL; // Use PARTIAL refresh for fast value updates
  lvgl_cfg.use_partial_refresh = true; // Enable partial refresh
  lvgl_cfg.partial_threshold = 1000;   // Partial refresh threshold
  lvgl_cfg.dither_mode =
      EPD_DITHER_NONE; // Disable dithering for crisp B/W text

  g_disp = epd_lvgl_init(&lvgl_cfg);
  if (!g_disp) {
    ESP_LOGE(TAG, "Failed to init LVGL display");
    epd_deinit(epd);
    return;
  }
  lv_display_t *disp = g_disp; // Local alias for compatibility

  // Setup LVGL tick timer
  esp_timer_create_args_t lvgl_tick_timer_args = {.callback = &lv_tick_timer_cb,
                                                  .name = "lvgl_tick"};
  esp_timer_handle_t lvgl_tick_timer = NULL;
  ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
  ESP_ERROR_CHECK(
      esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

  // Create LVGL task (ESP32-C5 is unicore, use tskNO_AFFINITY)
  lvgl_mux = xSemaphoreCreateMutex();
  g_display_data_mux = xSemaphoreCreateMutex();
  if (g_display_data_mux == NULL) {
    ESP_LOGE(TAG, "Failed to create display data mutex");
    return;
  }
  xTaskCreatePinnedToCore(lv_handler_task, "LVGL", 8 * 1024, NULL, 4,
                          &g_lvgl_task_handle, tskNO_AFFINITY);

  // ==================== UI CREATION ====================

  Display display;
  g_display = &display; // Store pointer for shutdown access

  if (lvgl_lock(-1)) {
    bool ui_ok = display.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (!ui_ok) {
      ESP_LOGE(TAG, "Failed to init Display UI");
      lvgl_unlock();
      epd_deinit(epd);
      return;
    }

    // INITIAL SCREEN: show a clear init state before random values.
    ESP_LOGI(TAG, "Displaying INITIAL screen...");
    display.setTimeText("INITIAL");
    lvgl_unlock();
  }

  ESP_LOGI(TAG, "INITIAL screen displayed, performing full refresh...");
  request_lvgl_refresh();

  // Wait a moment to let user see INITIAL screen
  ESP_LOGI(TAG, "INITIAL screen visible, waiting 1.5 seconds...");
  vTaskDelay(pdMS_TO_TICKS(1500));

  // Initialize I2C for sensors and buttons
  ESP_LOGI(TAG, "Initializing I2C bus and sensors for static test...");
  Sensors sensors_static;
  GPS gps_static;
  g_sensors = &sensors_static;
  g_gps = &gps_static;
  bool static_boost_requested = false;
  
  ret = sensors_static.init();
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "Sensors init failed: %s", esp_err_to_name(ret));
  } else {
    ESP_LOGI(TAG, "Sensors initialized successfully");
  }

  // Initialize GPS for static test
  ret = gps_static.init();
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "GPS init failed: %s", esp_err_to_name(ret));
  } else {
    ESP_LOGI(TAG, "GPS initialized successfully");
  }

  i2c_master_bus_handle_t i2c_bus_static = sensors_static.getI2CBusHandle();
  if (i2c_bus_static != NULL) {
    ESP_LOGI(TAG, "Initializing CAP1203 capacitive buttons...");
    ret = init_cap1203(i2c_bus_static);
    if (ret != ESP_OK) {
      ESP_LOGW(TAG, "CAP1203 init failed: %s", esp_err_to_name(ret));
    } else {
      ESP_LOGI(TAG, "CAP1203 ready - Press T1 button to refresh sensor values!");
    }

#if LED_ENABLED
    // Initialize LP5036 LED driver
    ESP_LOGI(TAG, "Initializing LP5036 LED driver...");
    g_led_driver = new drivers::LP5036(i2c_bus_static, drivers::LP5036_I2C::ADDR_33);
    if (g_led_driver == nullptr) {
      ESP_LOGE(TAG, "Failed to allocate LP5036 driver");
    } else {
      ret = g_led_driver->init();
      if (ret != ESP_OK) {
        ESP_LOGW(TAG, "LP5036 init failed: %s", esp_err_to_name(ret));
        delete g_led_driver;
        g_led_driver = nullptr;
      } else {
        g_led_driver->enable(true);
        g_led_driver->set_log_scale(false);
        g_led_driver->set_global_off(false);
        g_led_driver->set_bank_brightness(0xFF);
        g_led_driver->set_power_save(false);
        ESP_LOGI(TAG, "LP5036 initialized successfully");
      }
    }
#else
    ESP_LOGI(TAG, "LED bar disabled by configuration");
#endif

    // ==================== BQ25629 CHARGER INITIALIZATION ====================
    ESP_LOGI(TAG, "Initializing BQ25629 battery charger...");
    g_charger = new drivers::BQ25629(i2c_bus_static);

    drivers::BQ25629_Config charger_cfg = {
        .charge_voltage_mv = 4200,      // 4.2V (full charge for Li-ion)
        .charge_current_ma = 500,       // 500mA charging
        .input_current_limit_ma = 1500, // 1500mA input limit
        .input_voltage_limit_mv = 4600, // 4.6V VINDPM (works with 5V input)
        .min_system_voltage_mv = 3520,  // 3.52V minimum
        .precharge_current_ma = 30,     // 30mA pre-charge
        .term_current_ma = 20,          // 20mA termination
        .enable_charging = true,        // Enable charging
        .enable_otg = false,            // Disable OTG mode
        .enable_adc = true,             // Enable ADC for monitoring
    };

    ret = g_charger->init(charger_cfg);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "BQ25629 init failed: %s", esp_err_to_name(ret));
      delete g_charger;
      g_charger = nullptr;
    } else {
      ESP_LOGI(TAG, "BQ25629 initialized successfully");

      // Log charger limit registers for verification
      g_charger->log_charger_limits();

      // Set watchdog timeout to maximum supported (200s)
      esp_err_t wd_cfg_ret =
          g_charger->set_watchdog_timeout(drivers::WatchdogTimeout::Sec200);
      if (wd_cfg_ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to set BQ25629 watchdog timeout: %s",
                 esp_err_to_name(wd_cfg_ret));
      }

      // Configure JEITA temperature profile
      ret = g_charger->configure_jeita_profile();
      if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to configure JEITA profile: %s", esp_err_to_name(ret));
      }

      // Verify by reading ADC
      drivers::BQ25629_ADC_Data adc_data;
      ret = g_charger->read_adc(adc_data);
      if (ret == ESP_OK) {
        ESP_LOGI(TAG, "ADC: VPMID=%umV VSYS=%umV VBAT=%umV VBUS=%umV",
                 adc_data.vpmid_mv, adc_data.vsys_mv, adc_data.vbat_mv, adc_data.vbus_mv);
        ESP_LOGI(TAG, "     IBUS=%dmA IBAT=%dmA TS=%.1f%% TDIE=%d°C",
                 adc_data.ibus_ma, adc_data.ibat_ma, adc_data.ts_percent, adc_data.tdie_c);
      } else {
        ESP_LOGW(TAG, "Failed to read BQ25629 ADC: %s", esp_err_to_name(ret));
      }

      // Read NTC temperature
      drivers::BQ25629_NTC_Data ntc_data;
      ret = g_charger->read_ntc_temperature(ntc_data);
      if (ret == ESP_OK) {
        const char* zone_names[] = {"COLD", "COOL", "NORM", "WARM", "HOT", "UNKN"};
        ESP_LOGI(TAG, "NTC: %.1f°C (%.0fΩ) Zone=%s",
                 ntc_data.temperature_c, ntc_data.resistance_ohm, 
                 zone_names[static_cast<int>(ntc_data.zone)]);
      }

      // ==================== ENABLE PMID 5V BOOST FOR SPS30 ====================
      // SPS30 PM sensor requires 5V supply from PMID output
      ESP_LOGI(TAG, "Enabling PMID 5V boost for SPS30 PM sensor...");
      ret = g_charger->enable_pmid_5v_boost();
      if (ret == ESP_OK) {
        ESP_LOGI(TAG, "PMID 5V boost enabled successfully!");
        static_boost_requested = true;
        
        // Wait for boost to stabilize and read ADC again
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // Read ADC after enabling boost to verify VPMID
        ret = g_charger->read_adc(adc_data);
        if (ret == ESP_OK) {
          ESP_LOGI(TAG, "=== BQ25629 ADC AFTER PMID BOOST ===");
          ESP_LOGI(TAG, "  VPMID = %u mV (should be ~5000mV)", adc_data.vpmid_mv);
          ESP_LOGI(TAG, "  VSYS  = %u mV", adc_data.vsys_mv);
          ESP_LOGI(TAG, "  VBAT  = %u mV", adc_data.vbat_mv);
          ESP_LOGI(TAG, "  VBUS  = %u mV", adc_data.vbus_mv);
          ESP_LOGI(TAG, "  IBUS  = %d mA", adc_data.ibus_ma);
          ESP_LOGI(TAG, "  IBAT  = %d mA (+=charging, -=discharging)", adc_data.ibat_ma);
          ESP_LOGI(TAG, "  TS    = %.1f %%", adc_data.ts_percent);
          ESP_LOGI(TAG, "  TDIE  = %d °C", adc_data.tdie_c);
          ESP_LOGI(TAG, "=================================");
          
          // Calculate and explain battery percentage
          int bat_pct = battery_percent_from_mv(adc_data.vbat_mv);
          ESP_LOGI(TAG, "Battery SOC Calculation (Voltage-based, NOT Coulomb Counting):");
          ESP_LOGI(TAG, "  VBAT=%umV, Empty=%umV, Full=%umV",
                   adc_data.vbat_mv, kBatteryEmptyMv, kBatteryFullMv);
          ESP_LOGI(TAG, "  Formula: (VBAT - Empty) * 100 / (Full - Empty)");
          ESP_LOGI(TAG, "  Result: (%u - %u) * 100 / (%u - %u) = %d%%",
                   adc_data.vbat_mv, kBatteryEmptyMv, kBatteryFullMv, kBatteryEmptyMv, bat_pct);
        }
      } else {
        ESP_LOGE(TAG, "Failed to enable PMID 5V boost: %s", esp_err_to_name(ret));
        ESP_LOGW(TAG, "SPS30 PM sensor may not work without 5V supply!");
      }
    }
  }

  // Initialize display status icons
  if (lvgl_lock(-1)) {
    ESP_LOGI(TAG, "Initializing display status icons...");
    display.setWiFiStatus(Display::WiFiStatus::Off);
    display.setGPSStatus(Display::GPSStatus::Searching);
    display.setBLEStatus(Display::BLEStatus::Disconnected);
    display.setRecording(false);
    display.setFocusTile(Display::FocusTile::CO2);
    display.setIntervalSeconds(1);
    display.setAlert(false);
    lvgl_unlock();
  }
  request_lvgl_refresh();

  // ==================== DISPLAY UPDATE TASK (STATIC TEST) ====================
  ESP_LOGI(TAG, "Starting display update task for static test...");
  xTaskCreatePinnedToCore(display_task, "DisplayTask", 6 * 1024, &display, 4,
                          &g_display_task_handle, tskNO_AFFINITY);

  // ==================== QON BUTTON TASK ====================
  ESP_LOGI(TAG, "Starting QON button monitor task...");
  xTaskCreatePinnedToCore(qon_button_task, "QON_Task", 4096, NULL,
                          6, // High priority for shutdown detection
                          NULL, tskNO_AFFINITY);

  ESP_LOGI(TAG, "Sensor mode active; auto-updating every 1s.");
  uint64_t static_last_sensor_update_ms = 0;
  uint64_t static_last_display_update_ms = 0;
  const uint64_t STATIC_DISPLAY_UPDATE_INTERVAL_MS = 100;  // Update display snapshot every 100ms
  uint64_t static_last_summary_ms = 0;
  const uint64_t STATIC_SUMMARY_INTERVAL_MS = 5000;  // Log summary every 5 seconds
  const uint64_t STATIC_GPS_UI_UPDATE_INTERVAL_MS = 1000;  // Update GPS UI every 1 second
  uint64_t static_last_gps_ui_ms = 0;
  
  // Charger/power-path monitoring variables
  uint64_t static_last_charger_log_ms = 0;
  const uint64_t STATIC_CHARGER_LOG_INTERVAL_MS = 2000;
  uint64_t static_last_power_path_check_ms = 0;
  const uint64_t STATIC_POWER_PATH_CHECK_INTERVAL_MS = 250;
  const uint64_t STATIC_VBUS_PRESENT_STABLE_MS = 1000;
  const uint64_t STATIC_VBUS_ABSENT_STABLE_MS = 1000;
  const uint64_t STATIC_VBUS_HANDOVER_DELAY_MS = 1000;
  const uint64_t STATIC_VBUS_HANDOVER_STABLE_MS = 2000;
  const uint64_t STATIC_VBUS_HANDOVER_LOG_INTERVAL_MS = 2000;
  const uint16_t STATIC_VBUS_OK_MV = 4300;
  const uint16_t STATIC_VSYS_OK_MV = 3500;
  uint64_t static_vbus_present_since_ms = 0;
  uint64_t static_vbus_absent_since_ms = 0;
  bool static_vbus_present_last = false;
  bool static_vbus_handover_active = false;
  uint64_t static_vbus_handover_start_ms = 0;
  uint64_t static_vbus_handover_last_log_ms = 0;
  uint64_t static_vbus_handover_ok_since_ms = 0;
  bool static_vbus_stable_known = false;
  bool static_vbus_stable_last = false;
  const int64_t STATIC_SPS30_READ_STALE_MS = 3000;
  const uint16_t STATIC_PMID_LOW_MV = 4000;
  uint64_t static_last_battery_soc_ms = 0;
  uint64_t static_last_battery_soc_save_ms = 0;
  const uint64_t STATIC_BATTERY_SOC_SAVE_INTERVAL_MS = 60000;
  uint64_t static_last_wd_kick_ms = 0;
  const uint64_t STATIC_WD_KICK_INTERVAL_MS = 150000; // Kick before 200s timeout
  int static_battery_percent = 0;
  bool static_battery_charging = false;
  bool static_battery_charging_valid = false;
  bool static_battery_valid = false;
  drivers::ChargeStatus static_last_charge_status = drivers::ChargeStatus::NOT_CHARGING;
  bool static_charge_status_valid = false;
  
#if LED_ENABLED
  AirLevel prev_pm_level = AirLevel::Green;  // Track previous PM2.5 level
  AirLevel prev_co2_level = AirLevel::Green;  // Track previous CO2 level
#endif
  
  while (true) {
    int64_t now_ms = esp_timer_get_time() / 1000;
    uint64_t now_ms_u = (uint64_t)now_ms;
    
    // Update GPS and sensors periodically (every 100ms)
    if (now_ms_u - static_last_sensor_update_ms >= 100) {
      gps_static.update(now_ms_u);
      sensors_static.update(now_ms);
      static_last_sensor_update_ms = now_ms_u;
    }
    
    // Update display snapshot every 100ms (like DISPLAY_STATIC_TEST 0)
    if (now_ms_u - static_last_display_update_ms >= STATIC_DISPLAY_UPDATE_INTERVAL_MS) {
      static_last_display_update_ms = now_ms_u;
      
      sensor_values_t values = {};
      sensors_static.getValues(now_ms, &values);
      
      // Update display snapshot for display_task to consume
      if (g_display_data_mux &&
          xSemaphoreTake(g_display_data_mux, pdMS_TO_TICKS(10)) == pdTRUE) {
        g_display_snapshot.sensor = values;
        g_display_snapshot.sensor_valid = true;
        g_display_snapshot.sensor_update_ms = now_ms_u;
        
        // Battery status is updated separately in the charger logging section
        // Don't overwrite here with hardcoded values
        
        xSemaphoreGive(g_display_data_mux);
      }
      
#if LED_ENABLED
      // Update LED bar only if air quality levels changed
      AirLevel pm_level = pm25_level_from_ugm3(values.pm25_mass);
      AirLevel co2_level = values.have_co2_avg ? 
                          co2_level_from_ppm(values.co2_ppm_avg) : AirLevel::Off;
      
      if (g_led_driver != nullptr && 
          (pm_level != prev_pm_level || co2_level != prev_co2_level)) {
        led_show_air_levels(pm_level, co2_level);
        prev_pm_level = pm_level;
        prev_co2_level = co2_level;
      }
#endif
    }
    
    // Update GPS status in snapshot every 1 second (like DISPLAY_STATIC_TEST 0)
    if (now_ms_u - static_last_gps_ui_ms >= STATIC_GPS_UI_UPDATE_INTERVAL_MS) {
      static_last_gps_ui_ms = now_ms_u;
      
      Display::GPSStatus gps_status = Display::GPSStatus::Off;
      bool has_sentence = gps_static.has_recent_sentence(now_ms_u, 5000);
      
      if (has_sentence) {
        gps_status = gps_static.has_fix() ? Display::GPSStatus::Fix 
                                          : Display::GPSStatus::Searching;
      }
      
      bool time_valid = has_sentence && gps_static.has_time();
      int hour = gps_static.utc_hour();
      int min = gps_static.utc_min();
      bool fix_valid = gps_static.has_fix();
      float lat = gps_static.latitude_deg();
      float lon = gps_static.longitude_deg();
      
      if (g_display_data_mux &&
          xSemaphoreTake(g_display_data_mux, pdMS_TO_TICKS(10)) == pdTRUE) {
        g_display_snapshot.gps_status = gps_status;
        g_display_snapshot.gps_time_valid = time_valid;
        g_display_snapshot.gps_hour = hour;
        g_display_snapshot.gps_min = min;
        g_display_snapshot.gps_lat = lat;
        g_display_snapshot.gps_lon = lon;
        g_display_snapshot.gps_fix_valid = fix_valid;
        xSemaphoreGive(g_display_data_mux);
      }
    }

    // Power-path / OTG handover handling (match main loop)
    if (g_charger &&
        (now_ms_u - static_last_power_path_check_ms >=
         STATIC_POWER_PATH_CHECK_INTERVAL_MS)) {
      static_last_power_path_check_ms = now_ms_u;
      drivers::VBusStatus vbus_status = drivers::VBusStatus::NO_ADAPTER;
      esp_err_t vbus_ret = g_charger->get_vbus_status(vbus_status);
      if (vbus_ret == ESP_OK) {
        bool otg_active = (vbus_status == drivers::VBusStatus::OTG_MODE);
        bool vbus_present =
            (vbus_status != drivers::VBusStatus::NO_ADAPTER) && !otg_active;
        if (otg_active) {
          drivers::BQ25629_ADC_Data adc_data = {};
          esp_err_t adc_ret = g_charger->read_adc(adc_data);
          if (adc_ret == ESP_OK) {
            vbus_present = adc_data.vbus_mv >= 4000;
          }
        }

        if (vbus_present != static_vbus_present_last) {
          if (vbus_present) {
            static_vbus_present_since_ms = now_ms_u;
            static_vbus_absent_since_ms = 0;
          } else {
            static_vbus_absent_since_ms = now_ms_u;
            static_vbus_present_since_ms = 0;
          }
          static_vbus_present_last = vbus_present;
        } else if (vbus_present) {
          if (static_vbus_present_since_ms == 0)
            static_vbus_present_since_ms = now_ms_u;
        } else {
          if (static_vbus_absent_since_ms == 0)
            static_vbus_absent_since_ms = now_ms_u;
        }

        bool vbus_present_stable =
            vbus_present &&
            (now_ms_u - static_vbus_present_since_ms >=
             STATIC_VBUS_PRESENT_STABLE_MS);
        bool vbus_absent_stable =
            !vbus_present &&
            (now_ms_u - static_vbus_absent_since_ms >=
             STATIC_VBUS_ABSENT_STABLE_MS);

        bool vbus_stable_event = false;
        bool vbus_stable_present = false;
        if (vbus_present_stable || vbus_absent_stable) {
          vbus_stable_present = vbus_present_stable;
          if (!static_vbus_stable_known ||
              vbus_stable_present != static_vbus_stable_last) {
            vbus_stable_event = true;
            static_vbus_stable_last = vbus_stable_present;
            static_vbus_stable_known = true;
          }
        }

        if (vbus_stable_event) {
          bool sps30_reading =
              sensors_static.isSps30Reading(now_ms, STATIC_SPS30_READ_STALE_MS);
          if (!sps30_reading) {
            ESP_LOGW(TAG, "VBUS changed (%s), SPS30 not reading",
                     vbus_stable_present ? "present" : "absent");
            drivers::BQ25629_ADC_Data adc_data = {};
            esp_err_t adc_ret = g_charger->read_adc(adc_data);
            if (adc_ret == ESP_OK) {
              if (adc_data.vpmid_mv < STATIC_PMID_LOW_MV) {
                ESP_LOGW(TAG, "PMID low (%u mV), attempting recovery",
                         adc_data.vpmid_mv);
                if (!vbus_stable_present) {
                  esp_err_t boost_ret = g_charger->enable_pmid_5v_boost();
                  if (boost_ret != ESP_OK) {
                    ESP_LOGW(TAG, "Failed to enable PMID 5V boost: %s",
                             esp_err_to_name(boost_ret));
                  } else {
                    static_boost_requested = true;
                  }
                } else {
                  esp_err_t chg_ret = g_charger->enable_charging(true);
                  if (chg_ret != ESP_OK) {
                    ESP_LOGW(TAG, "Failed to enable charging: %s",
                             esp_err_to_name(chg_ret));
                  }
                }
              }
            } else {
              ESP_LOGW(TAG, "PMID check failed: %s", esp_err_to_name(adc_ret));
            }
          }
        }

        if (vbus_absent_stable && !otg_active) {
          static_vbus_handover_active = false;
          ESP_LOGI(TAG, "VBUS removed, enabling PMID 5V boost");
          esp_err_t boost_ret = g_charger->enable_pmid_5v_boost();
          if (boost_ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to enable PMID 5V boost: %s",
                     esp_err_to_name(boost_ret));
          } else {
            static_boost_requested = true;
          }
        } else if (vbus_present_stable &&
                   (otg_active || static_boost_requested)) {
          if (!static_vbus_handover_active) {
            static_vbus_handover_active = true;
            static_vbus_handover_start_ms = now_ms_u;
            static_vbus_handover_last_log_ms = 0;
            static_vbus_handover_ok_since_ms = 0;
            ESP_LOGI(TAG, "VBUS stable, starting OTG->charge handover");
            esp_err_t chg_ret = g_charger->enable_charging(true);
            if (chg_ret != ESP_OK) {
              ESP_LOGW(TAG, "Failed to enable charging: %s",
                       esp_err_to_name(chg_ret));
            }
          }

          if (now_ms_u - static_vbus_handover_start_ms >=
              STATIC_VBUS_HANDOVER_DELAY_MS) {
            drivers::BQ25629_ADC_Data adc_data = {};
            esp_err_t adc_ret = g_charger->read_adc(adc_data);
            if (adc_ret == ESP_OK) {
              bool vbus_ok = adc_data.vbus_mv >= STATIC_VBUS_OK_MV;
              bool vsys_ok = adc_data.vsys_mv >= STATIC_VSYS_OK_MV;
              if (vbus_ok && vsys_ok) {
                if (static_vbus_handover_ok_since_ms == 0) {
                  static_vbus_handover_ok_since_ms = now_ms_u;
                }
                if (now_ms_u - static_vbus_handover_ok_since_ms >=
                    STATIC_VBUS_HANDOVER_STABLE_MS) {
                  ESP_LOGI(TAG, "VBUS/VSYS stable, disabling OTG boost");
                  esp_err_t otg_ret = g_charger->enable_otg(false);
                  if (otg_ret != ESP_OK) {
                    ESP_LOGW(TAG, "Failed to disable OTG: %s",
                             esp_err_to_name(otg_ret));
                  } else {
                    static_boost_requested = false;
                    static_vbus_handover_active = false;
                    static_vbus_handover_ok_since_ms = 0;
                    esp_err_t chg_ret = g_charger->enable_charging(true);
                    if (chg_ret != ESP_OK) {
                      ESP_LOGW(TAG, "Failed to enable charging: %s",
                               esp_err_to_name(chg_ret));
                    }
                  }
                }
              } else if (now_ms_u - static_vbus_handover_last_log_ms >=
                         STATIC_VBUS_HANDOVER_LOG_INTERVAL_MS) {
                static_vbus_handover_last_log_ms = now_ms_u;
                static_vbus_handover_ok_since_ms = 0;
                ESP_LOGW(TAG,
                         "VBUS handover waiting: VBUS=%u mV VSYS=%u mV "
                         "(keeping boost)",
                         adc_data.vbus_mv, adc_data.vsys_mv);
              }
            } else if (now_ms_u - static_vbus_handover_last_log_ms >=
                       STATIC_VBUS_HANDOVER_LOG_INTERVAL_MS) {
              static_vbus_handover_last_log_ms = now_ms_u;
              ESP_LOGW(TAG, "VBUS handover ADC read failed: %s",
                       esp_err_to_name(adc_ret));
            }
          }
        } else {
          static_vbus_handover_active = false;
        }
      }
    }

    // Periodic charger watchdog reset to avoid OTG drop
    if (g_charger &&
        (now_ms_u - static_last_wd_kick_ms >= STATIC_WD_KICK_INTERVAL_MS)) {
      static_last_wd_kick_ms = now_ms_u;
      esp_err_t wd_ret = g_charger->reset_watchdog();
      if (wd_ret != ESP_OK) {
        ESP_LOGW(TAG, "BQ25629 watchdog reset failed: %s",
                 esp_err_to_name(wd_ret));
      }
    }

    // Periodic charger debug logging + SOC update (match main loop)
    if (g_charger &&
        (now_ms_u - static_last_charger_log_ms >=
         STATIC_CHARGER_LOG_INTERVAL_MS)) {
      static_last_charger_log_ms = now_ms_u;

      drivers::BQ25629_Status charger_status = {};
      drivers::BQ25629_Fault charger_fault = {};
      drivers::BQ25629_ADC_Data adc_data = {};

      esp_err_t status_ret = g_charger->read_status(charger_status);
      esp_err_t fault_ret = g_charger->read_fault(charger_fault);
      esp_err_t adc_ret = g_charger->read_adc(adc_data);

      if (status_ret == ESP_OK) {
        const char *charge_status =
            charge_status_to_string(charger_status.charge_status);
        static_last_charge_status = charger_status.charge_status;
        static_charge_status_valid = true;
        ESP_LOGI(TAG,
                 "BQ25629 dbg - VBUS: %d, CHG: %d (%s), IOTG_REG: %d, "
                 "VOTG_REG: %d, WD: %d",
                 (int)charger_status.vbus_status,
                 (int)charger_status.charge_status, charge_status,
                 charger_status.iindpm_stat, charger_status.vindpm_stat,
                 charger_status.wd_stat);
        static_battery_charging = (charger_status.charge_status !=
                                   drivers::ChargeStatus::NOT_CHARGING);
        static_battery_charging_valid = true;
        if (g_display_data_mux &&
            xSemaphoreTake(g_display_data_mux, pdMS_TO_TICKS(10)) == pdTRUE) {
          g_display_snapshot.battery_charging = static_battery_charging;
          xSemaphoreGive(g_display_data_mux);
        }

      } else {
        ESP_LOGW(TAG, "BQ25629 status read failed: %s",
                 esp_err_to_name(status_ret));
        static_charge_status_valid = false;
        static_battery_charging_valid = false;
      }

      if (fault_ret == ESP_OK) {
        ESP_LOGI(TAG,
                 "BQ25629 fault - OTG: %d, TSHUT: %d, TS_STAT: %u, VBUS: %d, "
                 "BAT: %d, SYS: %d",
                 charger_fault.otg_fault, charger_fault.tshut_fault,
                 charger_fault.ts_stat, charger_fault.vbus_fault,
                 charger_fault.bat_fault, charger_fault.sys_fault);
        if (charger_fault.otg_fault || charger_fault.tshut_fault ||
            charger_fault.ts_stat != 0) {
          ESP_LOGW(TAG,
                   "BQ25629 fault/TS active - OTG: %d, TSHUT: %d, TS_STAT: %u",
                   charger_fault.otg_fault, charger_fault.tshut_fault,
                   charger_fault.ts_stat);
        }
      } else {
        ESP_LOGW(TAG, "BQ25629 fault read failed: %s",
                 esp_err_to_name(fault_ret));
      }

      if (adc_ret == ESP_OK) {
        ESP_LOGI(
            TAG,
            "BQ25629 adc - VPMID: %u mV, VBAT: %u mV, VSYS: %u mV, VBUS: %u mV, IBAT: %d mA, IBUS: %d mA",
            adc_data.vpmid_mv, adc_data.vbat_mv, adc_data.vsys_mv,
            adc_data.vbus_mv, adc_data.ibat_ma, adc_data.ibus_ma);
        if (!g_battery_soc.initialized && g_battery_soc.mutex &&
            xSemaphoreTake(g_battery_soc.mutex, pdMS_TO_TICKS(10)) ==
                pdTRUE) {
          // Try to load SOC from NVS first
          float nvs_soc_pct = 0.0f;
          uint16_t nvs_vbat_mv = 0;
          esp_err_t nvs_err = battery_soc_nvs_load(&nvs_soc_pct, &nvs_vbat_mv);

          if (nvs_err == ESP_OK && nvs_vbat_mv > 0) {
            // Calculate voltage difference
            int16_t vbat_diff_mv =
                (int16_t)adc_data.vbat_mv - (int16_t)nvs_vbat_mv;
            ESP_LOGI(TAG,
                     "Battery SOC loaded from NVS: %.1f%% (saved at %u mV, current %u mV, diff %d mV)",
                     nvs_soc_pct, nvs_vbat_mv, adc_data.vbat_mv, vbat_diff_mv);

            // If voltage difference is small (<100mV), trust the saved SOC
            if (vbat_diff_mv >= -100 && vbat_diff_mv <= 100) {
              g_battery_soc.soc_pct = nvs_soc_pct;
              ESP_LOGI(TAG, "Using saved SOC (voltage stable)");
            } else if (vbat_diff_mv > 100) {
              // Voltage increased significantly - battery was charged
              // Estimate SOC increase based on voltage change
              float soc_from_voltage =
                  (float)battery_percent_from_mv(adc_data.vbat_mv);
              g_battery_soc.soc_pct =
                  (nvs_soc_pct + soc_from_voltage) / 2.0f;
              ESP_LOGI(TAG,
                       "Voltage increased - estimating SOC: %.1f%% (blend of saved and voltage-based)",
                       g_battery_soc.soc_pct);
            } else {
              // Voltage decreased significantly - battery self-discharged or measurement error
              g_battery_soc.soc_pct = nvs_soc_pct - 2.0f; // Conservative 2% penalty
              if (g_battery_soc.soc_pct < 0.0f)
                g_battery_soc.soc_pct = 0.0f;
              ESP_LOGI(TAG, "Voltage decreased - applying 2%% penalty: %.1f%%",
                       g_battery_soc.soc_pct);
            }
          } else {
            // No saved SOC, calculate from voltage
            g_battery_soc.soc_pct =
                (float)battery_percent_from_mv(adc_data.vbat_mv);
            ESP_LOGI(TAG,
                     "No saved SOC in NVS - initializing from voltage: %.1f%%",
                     g_battery_soc.soc_pct);
          }

          g_battery_soc.last_vbat_mv = adc_data.vbat_mv;
          g_battery_soc.initialized = true;
          static_last_battery_soc_ms = now_ms_u;
          static_last_battery_soc_save_ms = now_ms_u;
          xSemaphoreGive(g_battery_soc.mutex);
        } else if (static_last_battery_soc_ms > 0 && g_battery_soc.mutex &&
                   xSemaphoreTake(g_battery_soc.mutex, pdMS_TO_TICKS(10)) ==
                       pdTRUE) {
          uint64_t dt_ms = now_ms_u - static_last_battery_soc_ms;
          static_last_battery_soc_ms = now_ms_u;
          float delta_pct =
              ((float)adc_data.ibat_ma * (float)dt_ms * 100.0f) /
              (kBatteryCapacityMah * 3600000.0f);
          g_battery_soc.soc_pct += delta_pct;
          g_battery_soc.last_vbat_mv = adc_data.vbat_mv;
          xSemaphoreGive(g_battery_soc.mutex);
        } else {
          static_last_battery_soc_ms = now_ms_u;
        }

        if (g_battery_soc.initialized && g_battery_soc.mutex &&
            xSemaphoreTake(g_battery_soc.mutex, pdMS_TO_TICKS(10)) ==
                pdTRUE) {
          int16_t ibat_abs = adc_data.ibat_ma < 0
                                 ? (int16_t)-adc_data.ibat_ma
                                 : adc_data.ibat_ma;
          if (static_charge_status_valid &&
              static_last_charge_status ==
                  drivers::ChargeStatus::TOPOFF_TIMER_ACTIVE) {
            g_battery_soc.soc_pct = 100.0f;
          } else if (static_charge_status_valid &&
                     static_last_charge_status ==
                         drivers::ChargeStatus::TAPER_CHARGE &&
                     adc_data.vbat_mv >= (kBatteryFullMv - 10) &&
                     ibat_abs <= kBatteryRelaxedCurrentMa) {
            g_battery_soc.soc_pct = 100.0f;
          } else if (static_battery_charging_valid &&
                     !static_battery_charging &&
                     adc_data.vbat_mv <= kBatteryEmptyMv) {
            g_battery_soc.soc_pct = 0.0f;
          }
          if (g_battery_soc.soc_pct < 0.0f)
            g_battery_soc.soc_pct = 0.0f;
          if (g_battery_soc.soc_pct > 100.0f)
            g_battery_soc.soc_pct = 100.0f;
          g_battery_soc.last_vbat_mv = adc_data.vbat_mv;
          xSemaphoreGive(g_battery_soc.mutex);
        }

        // Update display snapshot
        int battery_percent = 0;
        if (g_battery_soc.mutex &&
            xSemaphoreTake(g_battery_soc.mutex, pdMS_TO_TICKS(10)) ==
                pdTRUE) {
          battery_percent = battery_percent_from_soc(g_battery_soc.soc_pct);
          xSemaphoreGive(g_battery_soc.mutex);
        }

        static_battery_percent = battery_percent;
        static_battery_valid = g_battery_soc.initialized;

        if (g_display_data_mux &&
            xSemaphoreTake(g_display_data_mux, pdMS_TO_TICKS(10)) ==
                pdTRUE) {
          g_display_snapshot.battery_percent = battery_percent;
          g_display_snapshot.battery_valid = g_battery_soc.initialized;
          xSemaphoreGive(g_display_data_mux);
        }

        // Periodically save SOC to NVS
        if (g_battery_soc.initialized &&
            (now_ms_u - static_last_battery_soc_save_ms) >=
                STATIC_BATTERY_SOC_SAVE_INTERVAL_MS &&
            g_battery_soc.mutex &&
            xSemaphoreTake(g_battery_soc.mutex, pdMS_TO_TICKS(10)) ==
                pdTRUE) {
          float soc_to_save = g_battery_soc.soc_pct;
          uint16_t vbat_to_save = g_battery_soc.last_vbat_mv;
          xSemaphoreGive(g_battery_soc.mutex);

          static_last_battery_soc_save_ms = now_ms_u;
          esp_err_t save_err = battery_soc_nvs_save(soc_to_save, vbat_to_save);
          if (save_err == ESP_OK) {
            ESP_LOGD(TAG, "Battery SOC saved to NVS: %.1f%% at %u mV",
                     soc_to_save, vbat_to_save);
          } else {
            ESP_LOGW(TAG, "Failed to save battery SOC to NVS: %s",
                     esp_err_to_name(save_err));
          }
        }
        if (adc_data.vpmid_mv < STATIC_PMID_LOW_MV) {
          ESP_LOGW(TAG, "PMID low: %u mV (VBAT: %u mV)", adc_data.vpmid_mv,
                   adc_data.vbat_mv);
        }
      } else {
        ESP_LOGW(TAG, "BQ25629 adc read failed: %s", esp_err_to_name(adc_ret));
      }
    }
    
    // Periodic sensor summary log every 5 seconds
    if (now_ms_u - static_last_summary_ms >= STATIC_SUMMARY_INTERVAL_MS) {
      static_last_summary_ms = now_ms_u;
      
      sensor_values_t values = {};
      sensors_static.getValues(now_ms, &values);

      bool gps_fix_valid = gps_static.has_fix();
      bool gps_has_sentence = gps_static.has_recent_sentence(now_ms_u, 5000);
      const char *gps_state = gps_fix_valid ? "FIX" : (gps_has_sentence ? "SEARCH" : "OFF");
      GPS::AntennaStatus ant_status = gps_static.antenna_status();

      ESP_LOGI(TAG, "━━━━━━━━━━━━━ SENSOR SUMMARY ━━━━━━━━━━━━━");
      ESP_LOGI(TAG, "  CO2: %d ppm | T: %.1f°C | RH: %.1f%%",
               values.co2_ppm_avg, values.temp_c_avg, values.rh_avg);
      ESP_LOGI(TAG, "  PM2.5: %.1f µg/m³ | VOC: %d | NOx: %d",
               values.pm25_mass, values.voc_index, values.nox_index);
      ESP_LOGI(TAG, "  Pressure: %.1f hPa", values.pressure_pa / 100.0f);
      ESP_LOGI(TAG, "  GPS: %s | Lat: %.6f | Lon: %.6f | ANT: %s",
               gps_state, gps_static.latitude_deg(), gps_static.longitude_deg(),
               antenna_status_to_string(ant_status));
      if (static_battery_valid) {
        const char *chg_str = static_battery_charging_valid
                                  ? (static_battery_charging ? "YES" : "NO")
                                  : "UNKNOWN";
        ESP_LOGI(TAG, "  Battery: %d%% | Charging: %s", 
                 static_battery_percent, chg_str);
      } else {
        ESP_LOGI(TAG, "  Battery: N/A (BQ not ready)");
      }
      ESP_LOGI(TAG, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    }
    
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  // Note: Code below is unreachable - main loop runs forever

  // ==================== DISPLAY UPDATE TASK ====================
  xTaskCreatePinnedToCore(display_task, "DisplayTask", 6 * 1024, &display, 4,
                          &g_display_task_handle, tskNO_AFFINITY);

  // ==================== I2C BUS INITIALIZATION ====================

  // Initialize I2C bus first (needed for BQ25629 and sensors)
  ESP_LOGI(TAG, "Initializing I2C bus...");
  Sensors sensors;
  GPS gps;
  g_sensors = &sensors;  // Set global pointer for shutdown access
  g_gps = &gps;          // Set global pointer for shutdown access
  bool gps_ready = false;
  bool pmid_boost_requested = false;
  i2c_master_bus_handle_t i2c_bus = sensors.getI2CBusHandle();
  if (i2c_bus == NULL) {
    ESP_LOGE(TAG, "Failed to get I2C bus handle");
    return;
  }

  // Scan I2C bus for connected devices (enable/disable via #define)
  // i2c_scanner_scan(i2c_bus, 2);

  // ==================== CHARGER INITIALIZATION (BEFORE SENSORS!)
  // ====================

  // CRITICAL: Initialize BQ25629 BEFORE sensors to ensure PMID power path is
  // ready PMID provides power to VSYS which powers all sensors including SPS30
  ESP_LOGI(TAG, "Initializing BQ25629 battery charger (PMID power path)...");
  g_charger = new drivers::BQ25629(i2c_bus);

  drivers::BQ25629_Config charger_cfg = {
      .charge_voltage_mv = 4200,      // 4.2V (full charge for Li-ion)
      .charge_current_ma = 500,       // 500mA charging
      .input_current_limit_ma = 1500, // 1500mA input limit
      .input_voltage_limit_mv = 4600, // 4.6V VINDPM (works with 5V input)
      .min_system_voltage_mv = 3520,  // 3.52V minimum
      .precharge_current_ma = 30,     // 30mA pre-charge
      .term_current_ma = 20,          // 20mA termination
      .enable_charging = true,        // Enable charging
      .enable_otg = false,            // Disable OTG mode
      .enable_adc = true,             // Enable ADC for monitoring
  };

  ret = g_charger->init(charger_cfg);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "BQ25629 init failed: %s", esp_err_to_name(ret));
    delete g_charger;
    g_charger = nullptr;
  } else {
    ESP_LOGI(TAG, "BQ25629 initialized successfully");

    // Log charger limit registers for verification
    g_charger->log_charger_limits();

    // Set watchdog timeout to maximum supported (200s)
    esp_err_t wd_cfg_ret =
        g_charger->set_watchdog_timeout(drivers::WatchdogTimeout::Sec200);
    if (wd_cfg_ret != ESP_OK) {
      ESP_LOGW(TAG, "Failed to set BQ25629 watchdog timeout: %s",
               esp_err_to_name(wd_cfg_ret));
    }

    // Configure JEITA temperature profile
    ret = g_charger->configure_jeita_profile();
    if (ret != ESP_OK) {
      ESP_LOGW(TAG, "Failed to configure JEITA profile: %s", esp_err_to_name(ret));
    }
    vTaskDelay(pdMS_TO_TICKS(50)); // Give UART time to flush

    // Verify PMID is working by reading ADC
    drivers::BQ25629_ADC_Data adc_data;
    bool vbus_adc_present = false;
    ret = g_charger->read_adc(adc_data);
    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "ADC: VPMID=%umV VSYS=%umV VBAT=%umV VBUS=%umV",
               adc_data.vpmid_mv, adc_data.vsys_mv, adc_data.vbat_mv, adc_data.vbus_mv);
      ESP_LOGI(TAG, "     IBUS=%dmA IBAT=%dmA TS=%.1f%% TDIE=%d°C",
               adc_data.ibus_ma, adc_data.ibat_ma, adc_data.ts_percent, adc_data.tdie_c);
      vbus_adc_present = adc_data.vbus_mv >= 4000;

      if (adc_data.vpmid_mv < 3000) {
        ESP_LOGW(
            TAG,
            "PMID voltage too low! Power path may not be working correctly");
      }
    } else {
      ESP_LOGW(TAG, "Failed to read BQ25629 ADC: %s", esp_err_to_name(ret));
    }
    vTaskDelay(pdMS_TO_TICKS(20)); // Give UART time to flush

    // Read NTC temperature
    drivers::BQ25629_NTC_Data ntc_data;
    ret = g_charger->read_ntc_temperature(ntc_data);
    if (ret == ESP_OK) {
      const char* zone_names[] = {"COLD", "COOL", "NORM", "WARM", "HOT", "UNKN"};
      ESP_LOGI(TAG, "NTC: %.1f°C (%.0fΩ) Zone=%s",
               ntc_data.temperature_c, ntc_data.resistance_ohm, 
               zone_names[static_cast<int>(ntc_data.zone)]);
    } else {
      ESP_LOGW(TAG, "Failed to read NTC temperature: %s", esp_err_to_name(ret));
    }
    vTaskDelay(pdMS_TO_TICKS(20)); // Give UART time to flush

    // Ensure PMID discharge is disabled (not forcing discharge)
    g_charger->enable_pmid_discharge(false);

    // Read initial status and faults
    drivers::BQ25629_Status charger_status;
    drivers::BQ25629_Fault charger_fault;

    bool vbus_present = vbus_adc_present;
    ret = g_charger->read_status(charger_status);
    if (ret == ESP_OK) {
      ESP_LOGI(TAG,
               "Initial status - VBUS: %d, CHG: %d, VSYS_REG: %d, IINDPM: %d, "
               "VINDPM: %d",
               (int)charger_status.vbus_status,
               (int)charger_status.charge_status, charger_status.vsys_stat,
               charger_status.iindpm_stat, charger_status.vindpm_stat);
      vbus_present = vbus_present || (charger_status.vbus_status !=
                                      drivers::VBusStatus::NO_ADAPTER);
    }

    ret = g_charger->read_fault(charger_fault);
    if (ret == ESP_OK) {
      ESP_LOGI(TAG,
               "Fault status - VBUS_FAULT: %d, BAT_FAULT: %d, SYS_FAULT: %d, "
               "OTG_FAULT: %d, TSHUT: %d, TS_STAT: %u",
               charger_fault.vbus_fault, charger_fault.bat_fault,
               charger_fault.sys_fault, charger_fault.otg_fault,
               charger_fault.tshut_fault, charger_fault.ts_stat);
    }

    if (vbus_present) {
      ESP_LOGI(TAG, "VBUS detected, keeping charging enabled");
    } else {
      // Debounce VBUS on startup to avoid false OTG enable while VBUS is
      // ramping.
      vTaskDelay(pdMS_TO_TICKS(200));
      ret = g_charger->read_status(charger_status);
      if (ret == ESP_OK) {
        vbus_present = vbus_present || (charger_status.vbus_status !=
                                        drivers::VBusStatus::NO_ADAPTER);
      }
      ret = g_charger->read_adc(adc_data);
      if (ret == ESP_OK) {
        vbus_present = vbus_present || (adc_data.vbus_mv >= 4000);
      }
    }

    if (!vbus_present) {
      // Enable PMID 5V boost using complete setup sequence
      ESP_LOGI(TAG, "Enabling PMID 5V boost output...");
      ret = g_charger->enable_pmid_5v_boost();
      if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to enable PMID 5V boost: %s",
                 esp_err_to_name(ret));
      } else {
        pmid_boost_requested = true;
      }

      // Wait for boost to stabilize
      vTaskDelay(pdMS_TO_TICKS(100));

      // Read status and faults after boost enable
      ret = g_charger->read_status(charger_status);
      if (ret == ESP_OK) {
        ESP_LOGI(
            TAG, "After boost - VBUS: %d, CHG: %d, IOTG_REG: %d, VOTG_REG: %d",
            (int)charger_status.vbus_status, (int)charger_status.charge_status,
            charger_status.iindpm_stat, charger_status.vindpm_stat);
      }

      ret = g_charger->read_fault(charger_fault);
      if (ret == ESP_OK) {
        if (charger_fault.otg_fault || charger_fault.tshut_fault ||
            charger_fault.ts_stat != 0) {
          ESP_LOGW(TAG,
                   "OTG fault/TS condition - OTG: %d, TSHUT: %d, TS_STAT: %u",
                   charger_fault.otg_fault, charger_fault.tshut_fault,
                   charger_fault.ts_stat);
          ESP_LOGW(TAG, "PMID_GD may be LOW due to fault/TS condition");
        } else {
          ESP_LOGI(TAG, "No OTG faults detected");
        }
      }

      // Re-read ADC to verify boost voltage
      ret = g_charger->read_adc(adc_data);
      if (ret == ESP_OK) {
        ESP_LOGI(TAG,
                 "Final PMID: %u mV, VSYS: %u mV, VBUS: %u mV, VBAT: %u mV",
                 adc_data.vpmid_mv, adc_data.vsys_mv, adc_data.vbus_mv,
                 adc_data.vbat_mv);

        if (adc_data.vpmid_mv >= 4500) {
          ESP_LOGI(TAG, "PMID boost successful! PMID_GD should be HIGH");
        } else {
          ESP_LOGW(TAG,
                   "PMID voltage lower than expected (%umV vs 5000mV target)",
                   adc_data.vpmid_mv);
        }
      }
    }

    ESP_LOGI(TAG, "PMID power path ready for sensors");
  }

  // Wait a bit for PMID to stabilize
  vTaskDelay(pdMS_TO_TICKS(100));

  // ==================== LP5036 LED INIT SEQUENCE ====================

  ESP_LOGI(TAG, "Initializing LP5036 LED driver...");
  g_led_driver = new drivers::LP5036(i2c_bus, drivers::LP5036_I2C::ADDR_33);
  
  if (g_led_driver == nullptr) {
    ESP_LOGE(TAG, "Failed to allocate LP5036 driver");
  } else {
    ret = g_led_driver->init();
    if (ret != ESP_OK) {
      ESP_LOGW(TAG, "LP5036 init failed: %s", esp_err_to_name(ret));
      delete g_led_driver;
      g_led_driver = nullptr;
    } else {
      // Configure LED driver
      g_led_driver->enable(true);
      g_led_driver->set_log_scale(false);      // Linear brightness
      g_led_driver->set_global_off(false);     // Turn on output
      g_led_driver->set_bank_brightness(0xFF); // Full brightness
      g_led_driver->set_power_save(false);     // Disable auto power save
      
      ESP_LOGI(TAG, "LP5036 initialized - Starting boot animation...");
      
      // Boot animation: Rainbow sweep with breathing
      const int animation_steps = 24;
      const int delay_ms = 80;
      
      for (int step = 0; step < animation_steps; step++) {
        float progress = step / (float)animation_steps;
        float brightness = led_effects::breathing_effect(step * delay_ms, 2000);
        
        for (uint8_t i = 0; i < 12; i++) {
          // Rainbow wave effect
          float hue = fmodf((progress * 360.0f) + (i * 30.0f), 360.0f);
          color::HSV hsv(hue, 1.0f, brightness);
          color::RGB rgb = color::hsv_to_rgb(hsv);
          
          g_led_driver->set_led_brightness(i, 0xFF);
          g_led_driver->set_led_color(i, rgb.b, rgb.g, rgb.r);
        }
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
      }
      
      // Success indicator: Green pulse
      color::RGB green = color::Colors::GREEN;
      for (int pulse = 0; pulse < 2; pulse++) {
        for (int brightness = 0; brightness <= 255; brightness += 15) {
          for (uint8_t i = 0; i < 12; i++) {
            g_led_driver->set_led_brightness(i, brightness);
            g_led_driver->set_led_color(i, green.b, green.g, green.r);
          }
          vTaskDelay(pdMS_TO_TICKS(10));
        }
        vTaskDelay(pdMS_TO_TICKS(100));
        for (int brightness = 255; brightness >= 0; brightness -= 15) {
          for (uint8_t i = 0; i < 12; i++) {
            g_led_driver->set_led_brightness(i, brightness);
          }
          vTaskDelay(pdMS_TO_TICKS(10));
        }
      }
      
      // Turn off LEDs after boot animation
      g_led_driver->set_global_off(true);
      ESP_LOGI(TAG, "LP5036 boot animation complete");
    }
  }

  // ==================== SENSOR INITIALIZATION ====================

  ESP_LOGI(TAG, "Initializing sensors (now that PMID power is ready)...");
  ret = sensors.init();
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "Sensors init failed: %s", esp_err_to_name(ret));
  } else {
    ESP_LOGI(TAG, "Sensors initialized successfully");
  }

  // ==================== BUTTON INITIALIZATION ====================

  ESP_LOGI(TAG, "Initializing CAP1203 capacitive buttons...");
  ret = init_cap1203(i2c_bus);
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "CAP1203 init did not complete: %s", esp_err_to_name(ret));
  }

  // ==================== GPS INITIALIZATION ====================

  ESP_LOGI(TAG, "Initializing GPS UART...");
  ret = gps.init();
  if (ret != ESP_OK) {
    ESP_LOGW(TAG, "GPS init failed: %s", esp_err_to_name(ret));
  } else {
    gps_ready = true;
    esp_log_level_set("gps", ESP_LOG_DEBUG);
    
    // Configure for Active Antenna (with ANT_BIAS)
    ret = gps.set_antenna_type(GPS::AntennaType::Active);
    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "GPS configured for Active Antenna");
    } else {
      ESP_LOGW(TAG, "Failed to set GPS antenna type: %s", esp_err_to_name(ret));
    }
  }

  // ==================== QON BUTTON TASK ====================

  ESP_LOGI(TAG, "Starting QON button monitor task...");
  xTaskCreatePinnedToCore(qon_button_task, "QON_Task", 4096, NULL,
                          6, // High priority for shutdown detection
                          NULL, tskNO_AFFINITY);

  // ==================== MAIN LOOP ====================

  ESP_LOGI(TAG, "Starting normal operation with real sensor values...");

  uint64_t last_display_update_ms = 0;
  const uint64_t DISPLAY_UPDATE_INTERVAL_MS = 100; // Update values every 100ms
  const uint64_t GPS_UI_UPDATE_INTERVAL_MS = 1000;
  const uint64_t GPS_SENTENCE_TIMEOUT_MS = 3000;
  const uint64_t GPS_FIX_TIMEOUT_MS = 5000;
  uint64_t last_charger_log_ms = 0;
  const uint64_t CHARGER_LOG_INTERVAL_MS = 2000; // Debug log interval
  uint64_t last_power_path_check_ms = 0;
  const uint64_t POWER_PATH_CHECK_INTERVAL_MS = 250;
  const uint64_t VBUS_PRESENT_STABLE_MS = 1000;
  const uint64_t VBUS_ABSENT_STABLE_MS = 1000;
  const uint64_t VBUS_HANDOVER_DELAY_MS = 1000;
  const uint64_t VBUS_HANDOVER_STABLE_MS = 2000;
  const uint64_t VBUS_HANDOVER_LOG_INTERVAL_MS = 2000;
  const uint16_t VBUS_OK_MV = 4300;
  const uint16_t VSYS_OK_MV = 3500;
  uint64_t vbus_present_since_ms = 0;
  uint64_t vbus_absent_since_ms = 0;
  bool vbus_present_last = false;
  bool vbus_handover_active = false;
  uint64_t vbus_handover_start_ms = 0;
  uint64_t vbus_handover_last_log_ms = 0;
  uint64_t vbus_handover_ok_since_ms = 0;
  uint64_t last_wd_kick_ms = 0;
  const uint64_t WD_KICK_INTERVAL_MS = 150000; // Kick before 200s watchdog timeout
  uint64_t last_hw_wd_kick_ms = 0;
  uint64_t last_gps_ui_ms = 0;
  uint64_t last_sensor_summary_ms = 0;
  const uint64_t SENSOR_SUMMARY_INTERVAL_MS = 5000; // Sensor summary every 5s
  bool boost_requested = pmid_boost_requested;
  bool vbus_stable_known = false;
  bool vbus_stable_last = false;
  const int64_t SPS30_READ_STALE_MS = 3000;
  const uint16_t PMID_LOW_MV = 4000;
  uint64_t last_battery_soc_ms = 0;
  uint64_t last_battery_soc_save_ms = 0;
  const uint64_t BATTERY_SOC_SAVE_INTERVAL_MS = 60000; // Save to NVS every 60s
  bool battery_charging = false;
  bool battery_charging_valid = false;
  drivers::ChargeStatus last_charge_status = drivers::ChargeStatus::NOT_CHARGING;
  bool charge_status_valid = false;
  AirLevel last_pm_level = AirLevel::Off;
  AirLevel last_co2_level = AirLevel::Off;
  bool led_levels_initialized = false;

  while (true) {
    int64_t now_ms = esp_timer_get_time() / 1000;
    uint64_t now_ms_u = (uint64_t)now_ms;

    // Update sensors
    sensors.update(now_ms);

    // Update GPS parser
    if (gps_ready) {
      gps.update(now_ms_u);
      gps.log_status(now_ms_u, GPS_SENTENCE_TIMEOUT_MS, GPS_FIX_TIMEOUT_MS);
    }

    // Update display data snapshot for the display task
    if (now_ms_u - last_display_update_ms >= DISPLAY_UPDATE_INTERVAL_MS) {
      last_display_update_ms = now_ms_u;

      sensor_values_t vals;
      sensors.getValues(now_ms, &vals);

      AirLevel pm_level = pm25_level_from_ugm3(vals.pm25_mass);
      AirLevel co2_level =
          vals.have_co2_avg ? co2_level_from_ppm(vals.co2_ppm_avg)
                            : AirLevel::Off;
      if (!led_levels_initialized || pm_level != last_pm_level ||
          co2_level != last_co2_level) {
        led_show_air_levels(pm_level, co2_level);
        last_pm_level = pm_level;
        last_co2_level = co2_level;
        led_levels_initialized = true;
      }

      if (g_display_data_mux &&
          xSemaphoreTake(g_display_data_mux, pdMS_TO_TICKS(10)) == pdTRUE) {
        g_display_snapshot.sensor = vals;
        g_display_snapshot.sensor_valid = true;
        g_display_snapshot.sensor_update_ms = now_ms_u;
        xSemaphoreGive(g_display_data_mux);
      }
    }

    // Consolidated sensor summary log every 5 seconds
    if (now_ms_u - last_sensor_summary_ms >= SENSOR_SUMMARY_INTERVAL_MS) {
      last_sensor_summary_ms = now_ms_u;
      sensor_values_t vals;
      sensors.getValues(now_ms, &vals);

      int battery_percent = -1;
      bool battery_valid = false;
      if (g_battery_soc.mutex &&
          xSemaphoreTake(g_battery_soc.mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        battery_valid = g_battery_soc.initialized;
        if (battery_valid) {
          battery_percent = battery_percent_from_soc(g_battery_soc.soc_pct);
        }
        xSemaphoreGive(g_battery_soc.mutex);
      }

      bool gps_fix_valid = gps_ready && gps.has_fix();
      bool gps_has_sentence = gps_ready &&
                             gps.has_recent_sentence(now_ms_u, GPS_SENTENCE_TIMEOUT_MS);
      const char *gps_state = gps_fix_valid ? "FIX" : (gps_has_sentence ? "SEARCH" : "OFF");
      GPS::AntennaStatus ant_status = gps_ready ? gps.antenna_status()
                                                : GPS::AntennaStatus::Unknown;
      
      ESP_LOGI(TAG, "━━━━━━━━━━━━━ SENSOR SUMMARY ━━━━━━━━━━━━━");
      ESP_LOGI(TAG, "  CO2: %d ppm | T: %.1f°C | RH: %.1f%%",
               vals.co2_ppm_avg, vals.temp_c_avg, vals.rh_avg);
      ESP_LOGI(TAG, "  PM2.5: %.1f µg/m³ | VOC: %d | NOx: %d",
               vals.pm25_mass, vals.voc_index, vals.nox_index);
      ESP_LOGI(TAG, "  Pressure: %.1f hPa", vals.pressure_pa / 100.0f);
      ESP_LOGI(TAG, "  GPS: %s | Lat: %.6f | Lon: %.6f | ANT: %s",
               gps_state, gps_ready ? gps.latitude_deg() : 0.0f,
               gps_ready ? gps.longitude_deg() : 0.0f,
               antenna_status_to_string(ant_status));
      if (battery_valid) {
        const char *chg_str = battery_charging_valid
                                  ? (battery_charging ? "YES" : "NO")
                                  : "UNKNOWN";
        ESP_LOGI(TAG, "  Battery: %d%% | Charging: %s", battery_percent, chg_str);
      } else {
        ESP_LOGI(TAG, "  Battery: N/A | Charging: UNKNOWN");
      }
      ESP_LOGI(TAG, "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    }

    if (gps_ready && (now_ms_u - last_gps_ui_ms >= GPS_UI_UPDATE_INTERVAL_MS)) {
      last_gps_ui_ms = now_ms_u;
      Display::GPSStatus gps_status = Display::GPSStatus::Off;
      bool has_sentence =
          gps.has_recent_sentence(now_ms_u, GPS_SENTENCE_TIMEOUT_MS);
      if (has_sentence) {
        gps_status = gps.has_recent_fix(now_ms_u, GPS_FIX_TIMEOUT_MS)
                         ? Display::GPSStatus::Fix
                         : Display::GPSStatus::Searching;
      }

      bool time_valid = has_sentence && gps.has_time();
      int hour = gps.utc_hour();
      int min = gps.utc_min();
      bool fix_valid = gps.has_fix();
      float lat = gps.latitude_deg();
      float lon = gps.longitude_deg();

      if (g_display_data_mux &&
          xSemaphoreTake(g_display_data_mux, pdMS_TO_TICKS(10)) == pdTRUE) {
        g_display_snapshot.gps_status = gps_status;
        g_display_snapshot.gps_time_valid = time_valid;
        g_display_snapshot.gps_hour = hour;
        g_display_snapshot.gps_min = min;
        g_display_snapshot.gps_lat = lat;
        g_display_snapshot.gps_lon = lon;
        g_display_snapshot.gps_fix_valid = fix_valid;
        xSemaphoreGive(g_display_data_mux);
      }
    }

    if (g_charger &&
        (now_ms_u - last_power_path_check_ms >= POWER_PATH_CHECK_INTERVAL_MS)) {
      last_power_path_check_ms = now_ms_u;
      drivers::VBusStatus vbus_status = drivers::VBusStatus::NO_ADAPTER;
      esp_err_t vbus_ret = g_charger->get_vbus_status(vbus_status);
      if (vbus_ret != ESP_OK) {
        continue;
      }

      bool otg_active = (vbus_status == drivers::VBusStatus::OTG_MODE);
      bool vbus_present =
          (vbus_status != drivers::VBusStatus::NO_ADAPTER) && !otg_active;
      if (otg_active) {
        drivers::BQ25629_ADC_Data adc_data = {};
        esp_err_t adc_ret = g_charger->read_adc(adc_data);
        if (adc_ret == ESP_OK) {
          vbus_present = adc_data.vbus_mv >= 4000;
        }
      }

      if (vbus_present != vbus_present_last) {
        if (vbus_present) {
          vbus_present_since_ms = now_ms_u;
          vbus_absent_since_ms = 0;
        } else {
          vbus_absent_since_ms = now_ms_u;
          vbus_present_since_ms = 0;
        }
        vbus_present_last = vbus_present;
      } else if (vbus_present) {
        if (vbus_present_since_ms == 0)
          vbus_present_since_ms = now_ms_u;
      } else {
        if (vbus_absent_since_ms == 0)
          vbus_absent_since_ms = now_ms_u;
      }

      bool vbus_present_stable =
          vbus_present &&
          (now_ms_u - vbus_present_since_ms >= VBUS_PRESENT_STABLE_MS);
      bool vbus_absent_stable =
          !vbus_present &&
          (now_ms_u - vbus_absent_since_ms >= VBUS_ABSENT_STABLE_MS);

      bool vbus_stable_event = false;
      bool vbus_stable_present = false;
      if (vbus_present_stable || vbus_absent_stable) {
        vbus_stable_present = vbus_present_stable;
        if (!vbus_stable_known || vbus_stable_present != vbus_stable_last) {
          vbus_stable_event = true;
          vbus_stable_last = vbus_stable_present;
          vbus_stable_known = true;
        }
      }

      if (vbus_stable_event) {
        bool sps30_reading = sensors.isSps30Reading(now_ms, SPS30_READ_STALE_MS);
        if (!sps30_reading) {
          ESP_LOGW(TAG, "VBUS changed (%s), SPS30 not reading",
                   vbus_stable_present ? "present" : "absent");
          drivers::BQ25629_ADC_Data adc_data = {};
          esp_err_t adc_ret = g_charger->read_adc(adc_data);
          if (adc_ret == ESP_OK) {
            if (adc_data.vpmid_mv < PMID_LOW_MV) {
              ESP_LOGW(TAG, "PMID low (%u mV), attempting recovery", adc_data.vpmid_mv);
              if (!vbus_stable_present) {
                esp_err_t boost_ret = g_charger->enable_pmid_5v_boost();
                if (boost_ret != ESP_OK) {
                  ESP_LOGW(TAG, "Failed to enable PMID 5V boost: %s",
                           esp_err_to_name(boost_ret));
                } else {
                  boost_requested = true;
                }
              } else {
                esp_err_t chg_ret = g_charger->enable_charging(true);
                if (chg_ret != ESP_OK) {
                  ESP_LOGW(TAG, "Failed to enable charging: %s",
                           esp_err_to_name(chg_ret));
                }
              }
            }
          } else {
            ESP_LOGW(TAG, "PMID check failed: %s", esp_err_to_name(adc_ret));
          }
        }
      }

      if (vbus_absent_stable && !otg_active) {
        vbus_handover_active = false;
        ESP_LOGI(TAG, "VBUS removed, enabling PMID 5V boost");
        esp_err_t boost_ret = g_charger->enable_pmid_5v_boost();
        if (boost_ret != ESP_OK) {
          ESP_LOGW(TAG, "Failed to enable PMID 5V boost: %s",
                   esp_err_to_name(boost_ret));
        } else {
          boost_requested = true;
        }
      } else if (vbus_present_stable && (otg_active || boost_requested)) {
        if (!vbus_handover_active) {
          vbus_handover_active = true;
          vbus_handover_start_ms = now_ms_u;
          vbus_handover_last_log_ms = 0;
          vbus_handover_ok_since_ms = 0;
          ESP_LOGI(TAG, "VBUS stable, starting OTG->charge handover");
          esp_err_t chg_ret = g_charger->enable_charging(true);
          if (chg_ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to enable charging: %s",
                     esp_err_to_name(chg_ret));
          }
        }

        if (now_ms_u - vbus_handover_start_ms >= VBUS_HANDOVER_DELAY_MS) {
          drivers::BQ25629_ADC_Data adc_data = {};
          esp_err_t adc_ret = g_charger->read_adc(adc_data);
          if (adc_ret == ESP_OK) {
            bool vbus_ok = adc_data.vbus_mv >= VBUS_OK_MV;
            bool vsys_ok = adc_data.vsys_mv >= VSYS_OK_MV;
            if (vbus_ok && vsys_ok) {
              if (vbus_handover_ok_since_ms == 0) {
                vbus_handover_ok_since_ms = now_ms_u;
              }
              if (now_ms_u - vbus_handover_ok_since_ms >=
                  VBUS_HANDOVER_STABLE_MS) {
                ESP_LOGI(TAG, "VBUS/VSYS stable, disabling OTG boost");
                esp_err_t otg_ret = g_charger->enable_otg(false);
                if (otg_ret != ESP_OK) {
                  ESP_LOGW(TAG, "Failed to disable OTG: %s",
                           esp_err_to_name(otg_ret));
                } else {
                  boost_requested = false;
                  vbus_handover_active = false;
                  vbus_handover_ok_since_ms = 0;
                  esp_err_t chg_ret = g_charger->enable_charging(true);
                  if (chg_ret != ESP_OK) {
                    ESP_LOGW(TAG, "Failed to enable charging: %s",
                             esp_err_to_name(chg_ret));
                  }
                }
              }
            } else if (now_ms_u - vbus_handover_last_log_ms >=
                       VBUS_HANDOVER_LOG_INTERVAL_MS) {
              vbus_handover_last_log_ms = now_ms_u;
              vbus_handover_ok_since_ms = 0;
              ESP_LOGW(TAG,
                       "VBUS handover waiting: VBUS=%u mV VSYS=%u mV "
                       "(keeping boost)",
                       adc_data.vbus_mv, adc_data.vsys_mv);
            }
          } else if (now_ms_u - vbus_handover_last_log_ms >=
                     VBUS_HANDOVER_LOG_INTERVAL_MS) {
            vbus_handover_last_log_ms = now_ms_u;
            ESP_LOGW(TAG, "VBUS handover ADC read failed: %s",
                     esp_err_to_name(adc_ret));
          }
        }
      } else {
        vbus_handover_active = false;
      }
    }

    // Periodic charger watchdog reset to avoid OTG drop
    // Must reset before configured timeout or EN_OTG will be reset to 0
    if (g_charger && (now_ms_u - last_wd_kick_ms >= WD_KICK_INTERVAL_MS)) {
      last_wd_kick_ms = now_ms_u;
      esp_err_t wd_ret = g_charger->reset_watchdog();
      if (wd_ret != ESP_OK) {
        ESP_LOGW(TAG, "BQ25629 watchdog reset failed: %s",
                 esp_err_to_name(wd_ret));
      } else {
        ESP_LOGD(TAG, "BQ25629 watchdog reset OK");
      }
    }

    // Periodic hardware watchdog kick (TPL5010 DONE pulse)
    if (now_ms_u - last_hw_wd_kick_ms >= HW_WD_KICK_INTERVAL_MS) {
      last_hw_wd_kick_ms = now_ms_u;
      gpio_set_level(HW_WD_RST_PIN, 0);
      vTaskDelay(pdMS_TO_TICKS(HW_WD_PULSE_MS));
      gpio_set_level(HW_WD_RST_PIN, 1);
      ESP_LOGI(TAG, "HW watchdog kicked");
    }

    // Periodic charger debug logging (PMID/OTG behavior)
    if (g_charger &&
        (now_ms_u - last_charger_log_ms >= CHARGER_LOG_INTERVAL_MS)) {
      last_charger_log_ms = now_ms_u;

      drivers::BQ25629_Status charger_status = {};
      drivers::BQ25629_Fault charger_fault = {};
      drivers::BQ25629_ADC_Data adc_data = {};

      esp_err_t status_ret = g_charger->read_status(charger_status);
      esp_err_t fault_ret = g_charger->read_fault(charger_fault);
      esp_err_t adc_ret = g_charger->read_adc(adc_data);

      if (status_ret == ESP_OK) {
        const char *charge_status =
            charge_status_to_string(charger_status.charge_status);
        last_charge_status = charger_status.charge_status;
        charge_status_valid = true;
        ESP_LOGI(TAG,
                 "BQ25629 dbg - VBUS: %d, CHG: %d (%s), IOTG_REG: %d, "
                 "VOTG_REG: %d, WD: %d",
                 (int)charger_status.vbus_status,
                 (int)charger_status.charge_status, charge_status,
                 charger_status.iindpm_stat, charger_status.vindpm_stat,
                 charger_status.wd_stat);
        battery_charging = (charger_status.charge_status !=
                            drivers::ChargeStatus::NOT_CHARGING);
        battery_charging_valid = true;
        if (g_display_data_mux &&
            xSemaphoreTake(g_display_data_mux, pdMS_TO_TICKS(10)) == pdTRUE) {
          g_display_snapshot.battery_charging = battery_charging;
          xSemaphoreGive(g_display_data_mux);
        }

      } else {
        ESP_LOGW(TAG, "BQ25629 status read failed: %s",
                 esp_err_to_name(status_ret));
        charge_status_valid = false;
        battery_charging_valid = false;
      }

      if (fault_ret == ESP_OK) {
        ESP_LOGI(TAG,
                 "BQ25629 fault - OTG: %d, TSHUT: %d, TS_STAT: %u, VBUS: %d, "
                 "BAT: %d, SYS: %d",
                 charger_fault.otg_fault, charger_fault.tshut_fault,
                 charger_fault.ts_stat, charger_fault.vbus_fault,
                 charger_fault.bat_fault, charger_fault.sys_fault);
        if (charger_fault.otg_fault || charger_fault.tshut_fault ||
            charger_fault.ts_stat != 0) {
          ESP_LOGW(TAG,
                   "BQ25629 fault/TS active - OTG: %d, TSHUT: %d, TS_STAT: %u",
                   charger_fault.otg_fault, charger_fault.tshut_fault,
                   charger_fault.ts_stat);
        }
      } else {
        ESP_LOGW(TAG, "BQ25629 fault read failed: %s",
                 esp_err_to_name(fault_ret));
      }

      if (adc_ret == ESP_OK) {
        ESP_LOGI(
            TAG,
            "BQ25629 adc - VPMID: %u mV, VBAT: %u mV, VSYS: %u mV, VBUS: %u mV, IBAT: %d mA, IBUS: %d mA",
            adc_data.vpmid_mv, adc_data.vbat_mv, adc_data.vsys_mv,
            adc_data.vbus_mv, adc_data.ibat_ma, adc_data.ibus_ma);
        if (!g_battery_soc.initialized && g_battery_soc.mutex &&
            xSemaphoreTake(g_battery_soc.mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
          // Try to load SOC from NVS first
          float nvs_soc_pct = 0.0f;
          uint16_t nvs_vbat_mv = 0;
          esp_err_t nvs_err = battery_soc_nvs_load(&nvs_soc_pct, &nvs_vbat_mv);
          
          if (nvs_err == ESP_OK && nvs_vbat_mv > 0) {
            // Calculate voltage difference
            int16_t vbat_diff_mv = (int16_t)adc_data.vbat_mv - (int16_t)nvs_vbat_mv;
            ESP_LOGI(TAG, "Battery SOC loaded from NVS: %.1f%% (saved at %u mV, current %u mV, diff %d mV)",
                     nvs_soc_pct, nvs_vbat_mv, adc_data.vbat_mv, vbat_diff_mv);
            
            // If voltage difference is small (<100mV), trust the saved SOC
            if (vbat_diff_mv >= -100 && vbat_diff_mv <= 100) {
              g_battery_soc.soc_pct = nvs_soc_pct;
              ESP_LOGI(TAG, "Using saved SOC (voltage stable)");
            } else if (vbat_diff_mv > 100) {
              // Voltage increased significantly - battery was charged
              // Estimate SOC increase based on voltage change
              float soc_from_voltage = (float)battery_percent_from_mv(adc_data.vbat_mv);
              g_battery_soc.soc_pct = (nvs_soc_pct + soc_from_voltage) / 2.0f;
              ESP_LOGI(TAG, "Voltage increased - estimating SOC: %.1f%% (blend of saved and voltage-based)",
                       g_battery_soc.soc_pct);
            } else {
              // Voltage decreased significantly - battery self-discharged or measurement error
              g_battery_soc.soc_pct = nvs_soc_pct - 2.0f; // Conservative 2% penalty
              if (g_battery_soc.soc_pct < 0.0f) g_battery_soc.soc_pct = 0.0f;
              ESP_LOGI(TAG, "Voltage decreased - applying 2%% penalty: %.1f%%", g_battery_soc.soc_pct);
            }
          } else {
            // No saved SOC, calculate from voltage
            g_battery_soc.soc_pct = (float)battery_percent_from_mv(adc_data.vbat_mv);
            ESP_LOGI(TAG, "No saved SOC in NVS - initializing from voltage: %.1f%%", g_battery_soc.soc_pct);
          }
          
          g_battery_soc.last_vbat_mv = adc_data.vbat_mv;
          g_battery_soc.initialized = true;
          last_battery_soc_ms = now_ms_u;
          last_battery_soc_save_ms = now_ms_u;
          xSemaphoreGive(g_battery_soc.mutex);
        } else if (last_battery_soc_ms > 0 && g_battery_soc.mutex &&
                   xSemaphoreTake(g_battery_soc.mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
          uint64_t dt_ms = now_ms_u - last_battery_soc_ms;
          last_battery_soc_ms = now_ms_u;
          float delta_pct =
              ((float)adc_data.ibat_ma * (float)dt_ms * 100.0f) /
              (kBatteryCapacityMah * 3600000.0f);
          g_battery_soc.soc_pct += delta_pct;
          g_battery_soc.last_vbat_mv = adc_data.vbat_mv;
          xSemaphoreGive(g_battery_soc.mutex);
        } else {
          last_battery_soc_ms = now_ms_u;
        }

        if (g_battery_soc.initialized && g_battery_soc.mutex &&
            xSemaphoreTake(g_battery_soc.mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
          int16_t ibat_abs = adc_data.ibat_ma < 0 ? (int16_t)-adc_data.ibat_ma
                                                 : adc_data.ibat_ma;
          if (charge_status_valid &&
              last_charge_status ==
                  drivers::ChargeStatus::TOPOFF_TIMER_ACTIVE) {
            g_battery_soc.soc_pct = 100.0f;
          } else if (charge_status_valid &&
                     last_charge_status ==
                         drivers::ChargeStatus::TAPER_CHARGE &&
                     adc_data.vbat_mv >= (kBatteryFullMv - 10) &&
                     ibat_abs <= kBatteryRelaxedCurrentMa) {
            g_battery_soc.soc_pct = 100.0f;
          } else if (battery_charging_valid && !battery_charging &&
                     adc_data.vbat_mv <= kBatteryEmptyMv) {
            g_battery_soc.soc_pct = 0.0f;
          }
          if (g_battery_soc.soc_pct < 0.0f)
            g_battery_soc.soc_pct = 0.0f;
          if (g_battery_soc.soc_pct > 100.0f)
            g_battery_soc.soc_pct = 100.0f;
          g_battery_soc.last_vbat_mv = adc_data.vbat_mv;
          xSemaphoreGive(g_battery_soc.mutex);
        }

        // Update display snapshot
        int battery_percent = 0;
        if (g_battery_soc.mutex &&
            xSemaphoreTake(g_battery_soc.mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
          battery_percent = battery_percent_from_soc(g_battery_soc.soc_pct);
          xSemaphoreGive(g_battery_soc.mutex);
        }
        
        if (g_display_data_mux &&
            xSemaphoreTake(g_display_data_mux, pdMS_TO_TICKS(10)) == pdTRUE) {
          g_display_snapshot.battery_percent = battery_percent;
          g_display_snapshot.battery_valid = g_battery_soc.initialized;
          xSemaphoreGive(g_display_data_mux);
        }
        
        // Periodically save SOC to NVS
        if (g_battery_soc.initialized && 
            (now_ms_u - last_battery_soc_save_ms) >= BATTERY_SOC_SAVE_INTERVAL_MS &&
            g_battery_soc.mutex &&
            xSemaphoreTake(g_battery_soc.mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
          float soc_to_save = g_battery_soc.soc_pct;
          uint16_t vbat_to_save = g_battery_soc.last_vbat_mv;
          xSemaphoreGive(g_battery_soc.mutex);
          
          last_battery_soc_save_ms = now_ms_u;
          esp_err_t save_err = battery_soc_nvs_save(soc_to_save, vbat_to_save);
          if (save_err == ESP_OK) {
            ESP_LOGD(TAG, "Battery SOC saved to NVS: %.1f%% at %u mV", 
                     soc_to_save, vbat_to_save);
          } else {
            ESP_LOGW(TAG, "Failed to save battery SOC to NVS: %s", 
                     esp_err_to_name(save_err));
          }
        }
        if (adc_data.vpmid_mv < 4000) {
          ESP_LOGW(TAG, "PMID low: %u mV (VBAT: %u mV)", adc_data.vpmid_mv,
                   adc_data.vbat_mv);
        }
      } else {
        ESP_LOGW(TAG, "BQ25629 adc read failed: %s", esp_err_to_name(adc_ret));
      }
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

static const char *charge_status_to_string(drivers::ChargeStatus status) {
  switch (status) {
  case drivers::ChargeStatus::NOT_CHARGING:
    return "NOT_CHARGING";
  case drivers::ChargeStatus::TRICKLE_PRECHARGE_FASTCHARGE:
    return "PRECHARGE_OR_FAST";
  case drivers::ChargeStatus::TAPER_CHARGE:
    return "TAPER";
  case drivers::ChargeStatus::TOPOFF_TIMER_ACTIVE:
    return "TOPOFF";
  default:
    return "UNKNOWN";
  }
}

static const char *antenna_status_to_string(GPS::AntennaStatus status) {
  switch (status) {
  case GPS::AntennaStatus::Ok:
    return "OK";
  case GPS::AntennaStatus::Open:
    return "OPEN";
  case GPS::AntennaStatus::Short:
    return "SHORT";
  case GPS::AntennaStatus::Unknown:
  default:
    return "UNKNOWN";
  }
}

static int battery_percent_from_mv(uint16_t vbat_mv) {
  if (vbat_mv <= kBatteryEmptyMv)
    return 0;
  if (vbat_mv >= kBatteryFullMv)
    return 100;

  return (int)(((int)vbat_mv - kBatteryEmptyMv) * 100 /
               (kBatteryFullMv - kBatteryEmptyMv));
}

static int battery_percent_from_soc(float soc_pct) {
  if (soc_pct < 0.0f)
    soc_pct = 0.0f;
  if (soc_pct > 100.0f)
    soc_pct = 100.0f;
  return (int)(soc_pct + 0.5f);
}

// ==================== NVS BATTERY SOC FUNCTIONS ====================

static const char *TAG_NVS = "BatterySOC_NVS";

/**
 * @brief Initialize NVS for battery SOC storage
 * @return ESP_OK on success
 */
static esp_err_t battery_soc_nvs_init(void) {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(TAG_NVS, "NVS partition was truncated, erasing...");
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  return err;
}

/**
 * @brief Load battery SOC from NVS
 * @param soc_pct Output pointer for SOC percentage (0.0-100.0)
 * @param last_vbat_mv Output pointer for last saved VBAT voltage in mV
 * @return ESP_OK if loaded successfully, ESP_ERR_NVS_NOT_FOUND if no saved data
 */
static esp_err_t battery_soc_nvs_load(float *soc_pct, uint16_t *last_vbat_mv) {
  if (!soc_pct || !last_vbat_mv) {
    return ESP_ERR_INVALID_ARG;
  }
  
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open("battery", NVS_READONLY, &nvs_handle);
  if (err != ESP_OK) {
    return err;
  }
  
  // Load SOC as float (stored as blob for precision)
  size_t soc_size = sizeof(float);
  err = nvs_get_blob(nvs_handle, "soc_pct", soc_pct, &soc_size);
  if (err != ESP_OK) {
    nvs_close(nvs_handle);
    return err;
  }
  
  // Load last VBAT voltage
  err = nvs_get_u16(nvs_handle, "vbat_mv", last_vbat_mv);
  if (err != ESP_OK) {
    nvs_close(nvs_handle);
    return err;
  }
  
  nvs_close(nvs_handle);
  return ESP_OK;
}

/**
 * @brief Save battery SOC to NVS
 * @param soc_pct SOC percentage (0.0-100.0) to save
 * @param vbat_mv Current VBAT voltage in mV
 * @return ESP_OK on success
 */
static esp_err_t battery_soc_nvs_save(float soc_pct, uint16_t vbat_mv) {
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open("battery", NVS_READWRITE, &nvs_handle);
  if (err != ESP_OK) {
    return err;
  }
  
  // Save SOC as blob for float precision
  err = nvs_set_blob(nvs_handle, "soc_pct", &soc_pct, sizeof(float));
  if (err != ESP_OK) {
    nvs_close(nvs_handle);
    return err;
  }
  
  // Save VBAT voltage
  err = nvs_set_u16(nvs_handle, "vbat_mv", vbat_mv);
  if (err != ESP_OK) {
    nvs_close(nvs_handle);
    return err;
  }
  
  // Commit changes
  err = nvs_commit(nvs_handle);
  nvs_close(nvs_handle);
  return err;
}

// ==================== LED HELPER FUNCTIONS ====================

struct LedPattern {
  uint8_t lit_count = 0;
  color::RGB primary = color::Colors::BLACK;
  color::RGB secondary = color::Colors::BLACK;
  bool alternate = false;
};

static LedPattern pattern_for_level(AirLevel level) {
  LedPattern pattern;
  switch (level) {
  case AirLevel::Green:
    pattern.lit_count = 1;
    pattern.primary = color::Colors::GREEN;
    return pattern;
  case AirLevel::Yellow:
    pattern.lit_count = 1;
    pattern.primary = color::Colors::YELLOW;
    return pattern;
  case AirLevel::Orange:
    pattern.lit_count = 2;
    pattern.primary = color::Colors::ORANGE;
    pattern.alternate = false;
    return pattern;
  case AirLevel::Red:
    pattern.lit_count = 2;
    pattern.primary = color::Colors::RED;
    pattern.alternate = false;
    return pattern;
  case AirLevel::Purple:
    pattern.lit_count = 3;
    pattern.primary = color::Colors::PURPLE;
    return pattern;
  case AirLevel::PurpleRed:
    pattern.lit_count = 4;
    pattern.primary = color::Colors::PURPLE;
    pattern.secondary = color::Colors::RED;
    pattern.alternate = true;
    return pattern;
  case AirLevel::Off:
  default:
    return pattern;
  }
}

static AirLevel pm25_level_from_ugm3(float pm25_ugm3) {
  if (pm25_ugm3 <= 5.0f) {
    return AirLevel::Green;
  }
  if (pm25_ugm3 <= 9.0f) {
    return AirLevel::Green;
  }
  if (pm25_ugm3 <= 20.0f) {
    return AirLevel::Yellow;
  }
  if (pm25_ugm3 <= 35.4f) {
    return AirLevel::Yellow;
  }
  if (pm25_ugm3 <= 45.0f) {
    return AirLevel::Orange;
  }
  if (pm25_ugm3 <= 55.4f) {
    return AirLevel::Orange;
  }
  if (pm25_ugm3 <= 100.0f) {
    return AirLevel::Red;
  }
  if (pm25_ugm3 <= 125.4f) {
    return AirLevel::Red;
  }
  if (pm25_ugm3 <= 225.4f) {
    return AirLevel::Purple;
  }
  return AirLevel::PurpleRed;
}

static AirLevel co2_level_from_ppm(int co2_ppm) {
  if (co2_ppm <= 800) {
    return AirLevel::Green;
  }
  if (co2_ppm <= 1000) {
    return AirLevel::Yellow;
  }
  if (co2_ppm <= 1500) {
    return AirLevel::Orange;
  }
  if (co2_ppm <= 2000) {
    return AirLevel::Red;
  }
  if (co2_ppm <= 3000) {
    return AirLevel::Purple;
  }
  return AirLevel::PurpleRed;
}

static void apply_led_pattern(uint8_t first_index, int8_t step,
                              const LedPattern &pattern) {
  int idx = first_index;
  for (uint8_t i = 0; i < 4; i++, idx += step) {
    if (i < pattern.lit_count) {
      const bool use_secondary = pattern.alternate && (i % 2 == 1);
      const color::RGB &level_color =
          use_secondary ? pattern.secondary : pattern.primary;
      g_led_driver->set_led_brightness(static_cast<uint8_t>(idx), 0xFF);
      g_led_driver->set_led_color(static_cast<uint8_t>(idx), level_color.b,
                                  level_color.g, level_color.r);
    } else {
      g_led_driver->set_led_brightness(static_cast<uint8_t>(idx), 0);
    }
  }
}

// Show PM2.5 (LED1-4) and CO2 (LED12-9) level bars.
static void led_show_air_levels(AirLevel pm_level, AirLevel co2_level) {
  if (g_led_driver == nullptr) return;
  static const char *TAG = "LEDs";

  g_led_driver->set_global_off(false);

  for (uint8_t i = 0; i < drivers::LP5036_LED_COUNT; i++) {
    g_led_driver->set_led_brightness(i, 0);
  }

  const LedPattern pm_pattern = pattern_for_level(pm_level);
  const LedPattern co2_pattern = pattern_for_level(co2_level);

  ESP_LOGI(TAG, "PM level: %d (R:%d G:%d B:%d lit:%d), CO2 level: %d (R:%d G:%d B:%d lit:%d)",
           (int)pm_level, pm_pattern.primary.r, pm_pattern.primary.g, pm_pattern.primary.b, pm_pattern.lit_count,
           (int)co2_level, co2_pattern.primary.r, co2_pattern.primary.g, co2_pattern.primary.b, co2_pattern.lit_count);

  // PM2.5: LED1-4 (indices 0-3), low -> high.
  apply_led_pattern(0, 1, pm_pattern);

  // CO2: LED12-9 (indices 11-8), low -> high.
  apply_led_pattern(11, -1, co2_pattern);
}

// Show battery status on LED ring (charging animation or battery level)
static void led_show_battery(int battery_percent, bool charging) {
  if (g_led_driver == nullptr) return;
  
  g_led_driver->set_global_off(false);
  
  if (charging) {
    // Charging: breathing blue animation
    uint32_t now_ms = esp_timer_get_time() / 1000;
    float brightness = led_effects::breathing_effect(now_ms, 2000);
    uint8_t brightness_8bit = static_cast<uint8_t>(brightness * 255);
    
    color::RGB blue = color::Colors::BLUE;
    for (uint8_t i = 0; i < 12; i++) {
      g_led_driver->set_led_brightness(i, brightness_8bit);
      g_led_driver->set_led_color(i, blue.b, blue.g, blue.r);
    }
  } else {
    // Battery level: progress bar (green/yellow/red based on level)
    int lit_leds = (battery_percent * 12) / 100;
    color::RGB color;
    
    if (battery_percent > 60) {
      color = color::Colors::GREEN;
    } else if (battery_percent > 20) {
      color = color::Colors::YELLOW;
    } else {
      color = color::Colors::RED;
    }
    
    for (uint8_t i = 0; i < 12; i++) {
      if (i < lit_leds) {
        g_led_driver->set_led_brightness(i, 0xFF);
        g_led_driver->set_led_color(i, color.b, color.g, color.r);
      } else {
        g_led_driver->set_led_brightness(i, 0);
      }
    }
  }
}

// Turn off all LEDs
static void led_off() {
  if (g_led_driver == nullptr) return;
  g_led_driver->set_global_off(true);
}

// Button callback for CAP1203 (called when button events occur)
static void button_event_callback(ButtonState state, void *user_data) {
  static const char *TAG_BTN = "CAP1203";

  const char *button_name = "UNKNOWN";
  switch (state.id) {
  case ButtonID::BUTTON_LEFT:
    button_name = "T1 (Up)";
    break;
  case ButtonID::BUTTON_MIDDLE:
    button_name = "T2 (Down)";
    break;
  case ButtonID::BUTTON_RIGHT:
    button_name = "T3 (Middle)";
    break;
  default:
    button_name = "NONE";
    break;
  }

  switch (state.event) {
  case ButtonEvent::PRESS:
    ESP_LOGI(TAG_BTN, "[%s] PRESS", button_name);
    break;
  case ButtonEvent::RELEASE:
    ESP_LOGI(TAG_BTN, "[%s] RELEASE (duration: %lu ms)", button_name,
             state.press_duration_ms);
    break;
  case ButtonEvent::SHORT_PRESS:
    ESP_LOGI(TAG_BTN, "[%s] SHORT_PRESS", button_name);
    break;
  case ButtonEvent::LONG_PRESS:
    ESP_LOGI(TAG_BTN, "[%s] LONG_PRESS (duration: %lu ms)", button_name,
             state.press_duration_ms);
    break;
  default:
    break;
  }

  bool focus_event = (state.event == ButtonEvent::PRESS ||
                      state.event == ButtonEvent::SHORT_PRESS ||
                      state.event == ButtonEvent::LONG_PRESS);
  if (focus_event) {
    bool focus_changed = false;
    const char *focus_label = nullptr;
    
    // T1 (left) -> CO2 tile, T3 (right) -> PM2.5 tile
    if (state.id == ButtonID::BUTTON_LEFT) {
      if (g_focus_tile != Display::FocusTile::CO2) {
        g_focus_tile = Display::FocusTile::CO2;
        focus_changed = true;
        focus_label = "CO2";
      }
    } else if (state.id == ButtonID::BUTTON_MIDDLE) {
      if (g_focus_tile != Display::FocusTile::PM25) {
        g_focus_tile = Display::FocusTile::PM25;
        focus_changed = true;
        focus_label = "PM2.5";
      }
    } else if (state.id == ButtonID::BUTTON_RIGHT) {
      if (g_focus_tile != Display::FocusTile::PM25) {
        g_focus_tile = Display::FocusTile::PM25;
        focus_changed = true;
        focus_label = "PM2.5";
      }
    }
    if (focus_changed) {
      g_focus_dirty = true;
      // Wake up display task to process focus change immediately
      if (g_display_task_handle) {
        xTaskNotifyGive(g_display_task_handle);
      }
      ESP_LOGI(TAG_BTN, "Focus set to %s tile", focus_label);
    }
  }
}

static esp_err_t init_cap1203(i2c_master_bus_handle_t i2c_bus) {
  static const char *TAG_BTN = "CAP1203";
  if (i2c_bus == NULL) {
    ESP_LOGE(TAG_BTN, "I2C bus handle is NULL");
    return ESP_ERR_INVALID_ARG;
  }

  if (g_buttons) {
    ESP_LOGW(TAG_BTN, "CAP1203 already initialized");
    return ESP_OK;
  }

  g_buttons = new CAP1203(i2c_bus);
  esp_err_t ret = g_buttons->init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG_BTN, "CAP1203 init failed: %s", esp_err_to_name(ret));
    delete g_buttons;
    g_buttons = nullptr;
    return ret;
  }

  g_buttons->setButtonCallback(button_event_callback, NULL);
  ESP_LOGI(TAG_BTN, "Button callback registered. Touch T1/T2/T3 to test!");

  xTaskCreatePinnedToCore(
      [](void *param) {
        CAP1203 *buttons = (CAP1203 *)param;
        while (true) {
          buttons->processButtons();
          vTaskDelay(pdMS_TO_TICKS(10)); // Poll every 10ms
        }
      },
      "ButtonTask", 4096, g_buttons,
      5, // Priority
      NULL, tskNO_AFFINITY);

  ESP_LOGI(TAG_BTN, "CAP1203 initialized successfully");
  return ESP_OK;
}

// LVGL tick timer callback (increments LVGL internal tick counter)
static void lv_tick_timer_cb(void *arg) { lv_tick_inc(LVGL_TICK_PERIOD_MS); }

// Lock LVGL mutex (blocking)
static bool lvgl_lock(int timeout_ms) {
  const TickType_t timeout_ticks =
      (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTake(lvgl_mux, timeout_ticks) == pdTRUE;
}

// Unlock LVGL mutex
static void lvgl_unlock(void) { xSemaphoreGive(lvgl_mux); }

static void request_lvgl_refresh(void) {
  g_lvgl_refresh_requested = true;
  if (g_lvgl_task_handle) {
    xTaskNotifyGive(g_lvgl_task_handle);
  }
}

static void request_lvgl_refresh_urgent(void) {
  g_lvgl_refresh_urgent = true;
  if (g_lvgl_task_handle) {
    xTaskNotifyGive(g_lvgl_task_handle);
  }
}

// Shutdown sequence with visual feedback
static void initiate_shutdown(void) {
  static const char *TAG = "Shutdown";
  ESP_LOGI(TAG, "========== SHUTDOWN SEQUENCE STARTED ==========");

  // Stop display task first to prevent UI update conflicts
  ESP_LOGI(TAG, "Stopping display task...");
  display_task_running = false;
  if (g_display_task_handle) {
    // Wait for display task to exit (max 1 second)
    for (int i = 0; i < 20 && eTaskGetState(g_display_task_handle) != eDeleted; i++) {
      vTaskDelay(pdMS_TO_TICKS(50));
    }
    ESP_LOGI(TAG, "Display task stopped");
  }

  // Phase 1: Stopping sensors
  ESP_LOGI(TAG, "Phase 1: Stopping sensors...");
  if (lvgl_lock(1000) && g_display) {
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);  // Remove all UI elements
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);  // White background
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);  // Opaque
    
    // Create rotated container (like ui_display.cpp)
    lv_obj_t *root = lv_obj_create(screen);
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, DISPLAY_ROT_WIDTH, DISPLAY_ROT_HEIGHT);
    lv_obj_align(root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_transform_pivot_x(root, DISPLAY_ROT_WIDTH / 2, 0);
    lv_obj_set_style_transform_pivot_y(root, DISPLAY_ROT_HEIGHT / 2, 0);
    lv_obj_set_style_transform_rotation(root, 1800, 0);  // Rotate 180 degrees
    
    lv_obj_t *label = lv_label_create(root);
    lv_label_set_text(label, "Shutting down...\n\nStopping sensors...");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);  // Black text
    lvgl_unlock();
  }
  request_lvgl_refresh();
  vTaskDelay(pdMS_TO_TICKS(800));

  // Stop GPS to release UART resources
  if (g_gps) {
    ESP_LOGI(TAG, "Stopping GPS...");
    g_gps->stop();
  }

  // Turn off LEDs
  if (g_led_driver) {
    ESP_LOGI(TAG, "Turning off LEDs...");
    g_led_driver->set_global_off(true);
  }

  // Note: Sensors class doesn't have stop() yet, but I2C will be released
  // when device enters ship mode. SPS30 fan will stop with power off.
  ESP_LOGI(TAG, "Phase 1 complete");

  // Phase 2: Stopping recording
  ESP_LOGI(TAG, "Phase 2: Stopping recording...");
  if (lvgl_lock(1000) && g_display) {
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);  // Remove all UI elements
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);  // White background
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);  // Opaque
    
    // Create rotated container (like ui_display.cpp)
    lv_obj_t *root = lv_obj_create(screen);
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, DISPLAY_ROT_WIDTH, DISPLAY_ROT_HEIGHT);
    lv_obj_align(root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_transform_pivot_x(root, DISPLAY_ROT_WIDTH / 2, 0);
    lv_obj_set_style_transform_pivot_y(root, DISPLAY_ROT_HEIGHT / 2, 0);
    lv_obj_set_style_transform_rotation(root, 1800, 0);  // Rotate 180 degrees
    
    lv_obj_t *label = lv_label_create(root);
    lv_label_set_text(label, "Shutting down...\n\nStopping recording...");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);  // Black text
    lvgl_unlock();
  }
  request_lvgl_refresh();
  vTaskDelay(pdMS_TO_TICKS(800));

  // Flush and close log storage (NAND flash)
  if (log_storage_is_ready()) {
    ESP_LOGI(TAG, "Flushing log storage...");
    log_storage_flush();
    ESP_LOGI(TAG, "Deinitializing log storage...");
    log_storage_deinit();
  }
  ESP_LOGI(TAG, "Phase 2 complete");

  // Phase 3: Saving data
  ESP_LOGI(TAG, "Phase 3: Saving data...");
  if (lvgl_lock(1000) && g_display) {
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);  // Remove all UI elements
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);  // White background
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);  // Opaque
    
    // Create rotated container (like ui_display.cpp)
    lv_obj_t *root = lv_obj_create(screen);
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, DISPLAY_ROT_WIDTH, DISPLAY_ROT_HEIGHT);
    lv_obj_align(root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_transform_pivot_x(root, DISPLAY_ROT_WIDTH / 2, 0);
    lv_obj_set_style_transform_pivot_y(root, DISPLAY_ROT_HEIGHT / 2, 0);
    lv_obj_set_style_transform_rotation(root, 1800, 0);  // Rotate 180 degrees
    
    lv_obj_t *label = lv_label_create(root);
    lv_label_set_text(label, "Shutting down...\n\nSaving data...");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);  // Black text
    lvgl_unlock();
  }
  request_lvgl_refresh();
  vTaskDelay(pdMS_TO_TICKS(800));

  // Save battery SOC to NVS
  if (g_battery_soc.initialized && g_battery_soc.mutex &&
      xSemaphoreTake(g_battery_soc.mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    float soc_to_save = g_battery_soc.soc_pct;
    uint16_t vbat_to_save = g_battery_soc.last_vbat_mv;
    xSemaphoreGive(g_battery_soc.mutex);
    
    esp_err_t save_err = battery_soc_nvs_save(soc_to_save, vbat_to_save);
    if (save_err == ESP_OK) {
      ESP_LOGI(TAG, "Battery SOC saved to NVS on shutdown: %.1f%% at %u mV", 
               soc_to_save, vbat_to_save);
    } else {
      ESP_LOGW(TAG, "Failed to save battery SOC on shutdown: %s", 
               esp_err_to_name(save_err));
    }
  }

  // TODO: Save pending data to flash when implemented

  // Phase 4: Powering off
  ESP_LOGI(TAG, "Phase 4: Powering off...");
  if (lvgl_lock(1000) && g_display) {
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);  // Remove all UI elements
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);  // White background
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);  // Opaque
    
    // Create rotated container (like ui_display.cpp)
    lv_obj_t *root = lv_obj_create(screen);
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, DISPLAY_ROT_WIDTH, DISPLAY_ROT_HEIGHT);
    lv_obj_align(root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_transform_pivot_x(root, DISPLAY_ROT_WIDTH / 2, 0);
    lv_obj_set_style_transform_pivot_y(root, DISPLAY_ROT_HEIGHT / 2, 0);
    lv_obj_set_style_transform_rotation(root, 1800, 0);  // Rotate 180 degrees
    
    lv_obj_t *label = lv_label_create(root);
    lv_label_set_text(label, "Powering off...\n\nGoodbye!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);  // Black text
    lvgl_unlock();
  }
  request_lvgl_refresh();
  vTaskDelay(pdMS_TO_TICKS(1000));

  // Phase 5: Clear display (prevent ghosting)
  ESP_LOGI(TAG, "Phase 5: Clearing display...");
  if (lvgl_lock(1000) && g_display) {
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);  // Remove all UI elements (including "Goodbye")
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);  // White background
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);  // Opaque
    lvgl_unlock();
  }
  request_lvgl_refresh();
  vTaskDelay(pdMS_TO_TICKS(500)); // Wait for white screen to complete

  // Phase 6: Enter ship mode
  ESP_LOGI(TAG, "Phase 6: Entering ship mode...");
  if (g_charger) {
    g_charger->enter_ship_mode();
    ESP_LOGI(TAG, "Ship mode command sent. System will power off shortly.");
  } else {
    ESP_LOGW(TAG, "Charger not initialized, cannot enter ship mode!");
  }

  // Wait for ship mode to take effect (system should power off)
  // If we reach here, ship mode failed
  ESP_LOGI(TAG, "Waiting for power off...");
  vTaskDelay(pdMS_TO_TICKS(2000));

  ESP_LOGE(TAG, "System did not power off! Ship mode may have failed.");
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// QON button monitor task
static void qon_button_task(void *arg) {
  static const char *TAG = "QON_Button";

  uint64_t qon_press_start_ms = 0;
  bool was_pressed = false;

  ESP_LOGI(TAG, "QON button monitor started on GPIO%d", QON_PIN);
  ESP_LOGI(TAG, "Hold for %d ms to initiate shutdown", SHUTDOWN_HOLD_MS);

  while (1) {
    // Active low (pressed = 0)
    bool is_pressed = (gpio_get_level(QON_PIN) == 0);
    uint64_t now_ms = esp_timer_get_time() / 1000;

    if (is_pressed && !was_pressed) {
      // Button just pressed
      qon_press_start_ms = now_ms;
      ESP_LOGI(TAG, "QON button pressed");
    } else if (is_pressed && was_pressed && !shutdown_requested) {
      // Button still held
      uint64_t hold_duration_ms = now_ms - qon_press_start_ms;

      if (hold_duration_ms >= SHUTDOWN_HOLD_MS) {
        ESP_LOGI(TAG, "QON button held for %llu ms - SHUTDOWN!",
                 hold_duration_ms);
        shutdown_requested = true;
        initiate_shutdown();
      }
    } else if (!is_pressed && was_pressed) {
      // Button released
      uint64_t hold_duration_ms = now_ms - qon_press_start_ms;
      ESP_LOGI(TAG, "QON button released after %llu ms", hold_duration_ms);
      qon_press_start_ms = 0;
    }

    was_pressed = is_pressed;
    vTaskDelay(pdMS_TO_TICKS(50)); // Poll every 50ms
  }
}

// LVGL handler task (processes display updates)
static void lv_handler_task(void *arg) {
  // Unsubscribe from watchdog - LVGL operations can take time
  esp_task_wdt_delete(xTaskGetCurrentTaskHandle());
  
  uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
  uint64_t last_refresh_ms = 0;
  while (1) {
    bool do_refresh = false;
    uint64_t now_ms = esp_timer_get_time() / 1000;
    if (g_lvgl_refresh_urgent) {
      g_lvgl_refresh_urgent = false;
      g_lvgl_refresh_requested = false;
      do_refresh = true;
      last_refresh_ms = now_ms;
    } else if (g_lvgl_refresh_requested) {
      if (now_ms - last_refresh_ms >= LVGL_REFRESH_MIN_INTERVAL_MS) {
        g_lvgl_refresh_requested = false;
        do_refresh = true;
        last_refresh_ms = now_ms;
      }
    }
    if (lvgl_lock(-1)) {
      task_delay_ms = lv_timer_handler();
      if (do_refresh && g_disp) {
        epd_lvgl_refresh(g_disp);
      }
      lvgl_unlock();
    }
    if (task_delay_ms > LVGL_TASK_MAX_DELAY_MS) {
      task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
    } else if (task_delay_ms < LVGL_TASK_MIN_DELAY_MS) {
      task_delay_ms = LVGL_TASK_MIN_DELAY_MS;
    }
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(task_delay_ms));
  }
}

static void display_task(void *arg) {
  Display *display = static_cast<Display *>(arg);
  DisplaySnapshot snapshot = {};
  uint64_t last_sensor_update_ms = 0;
  uint64_t last_display_refresh_ms = 0;
  uint64_t last_ui_refresh_ms = 0;
  bool sensor_initialized = false;
  bool gps_initialized = false;
  bool battery_initialized = false;
  Display::GPSStatus last_gps_status = Display::GPSStatus::Off;
  bool last_gps_time_valid = false;
  int last_gps_hour = -1;
  int last_gps_min = -1;
  int last_battery_percent = -1;
  bool last_battery_charging = false;

  while (display_task_running) {
    uint64_t now_ms_u = (uint64_t)(esp_timer_get_time() / 1000);
    if (g_display_data_mux &&
        xSemaphoreTake(g_display_data_mux, pdMS_TO_TICKS(10)) == pdTRUE) {
      snapshot = g_display_snapshot;
      xSemaphoreGive(g_display_data_mux);
    }

    bool request_refresh = false;
    bool request_refresh_urgent = false;

    if (lvgl_lock(100)) {
      if (snapshot.sensor_valid &&
          (!sensor_initialized ||
           snapshot.sensor_update_ms != last_sensor_update_ms)) {
        sensor_initialized = true;
        last_sensor_update_ms = snapshot.sensor_update_ms;

        if (snapshot.sensor.have_co2_avg) {
          display->setCO2(snapshot.sensor.co2_ppm_avg);
          display->setTempCf(snapshot.sensor.temp_c_avg);
          display->setRHf(snapshot.sensor.rh_avg);
        }
        display->setPM25f(snapshot.sensor.pm25_mass);
        display->setVOC(snapshot.sensor.voc_index);
        display->setNOx(snapshot.sensor.nox_index);
        
        // Set pressure from sensor (convert Pa to hPa)
        int pressure_hpa = (snapshot.sensor.pressure_pa > 0) ? 
                          (int)(snapshot.sensor.pressure_pa / 100.0f) : 1013;
        display->setPressure(pressure_hpa);

        if (!last_display_refresh_ms) {
          last_display_refresh_ms = now_ms_u;
        }
        request_refresh = true;
      }

      if (snapshot.battery_valid &&
          (!battery_initialized ||
           snapshot.battery_percent != last_battery_percent ||
           snapshot.battery_charging != last_battery_charging)) {
        battery_initialized = true;
        last_battery_percent = snapshot.battery_percent;
        last_battery_charging = snapshot.battery_charging;
        display->setBattery(snapshot.battery_percent,
                            snapshot.battery_charging);
        request_refresh = true;
      }

      bool gps_changed =
          !gps_initialized || snapshot.gps_status != last_gps_status ||
          snapshot.gps_time_valid != last_gps_time_valid ||
          (snapshot.gps_time_valid && (snapshot.gps_hour != last_gps_hour ||
                                       snapshot.gps_min != last_gps_min));
      if (gps_changed) {
        gps_initialized = true;
        last_gps_status = snapshot.gps_status;
        last_gps_time_valid = snapshot.gps_time_valid;
        last_gps_hour = snapshot.gps_hour;
        last_gps_min = snapshot.gps_min;
        display->setGPSStatus(snapshot.gps_status);
        display->setTimeHM(snapshot.gps_hour, snapshot.gps_min,
                           snapshot.gps_time_valid);
        display->setLatLon(snapshot.gps_lat, snapshot.gps_lon,
                           snapshot.gps_fix_valid);
        request_refresh = true;
      }

      if (g_focus_dirty) {
        display->setFocusTile(g_focus_tile);
        g_focus_dirty = false;
        request_refresh_urgent = true;
        last_display_refresh_ms = now_ms_u;
      }

      display->update(now_ms_u);
      lvgl_unlock();
    }

    if (now_ms_u - last_ui_refresh_ms >= kUiBlinkIntervalMs) {
      last_ui_refresh_ms = now_ms_u;
      request_refresh = true;
    }

    if (now_ms_u - last_display_refresh_ms >= kRecordingIntervalMs) {
      last_display_refresh_ms = now_ms_u;
      request_refresh = true;
    }

    if (request_refresh_urgent) {
      request_lvgl_refresh_urgent();
    } else if (request_refresh) {
      request_lvgl_refresh();
    }

    // Wait for notification or timeout (allows immediate wake-up on button press)
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50));
  }
  
  // Task must delete itself before returning
  ESP_LOGI("DisplayTask", "Display task exiting");
  vTaskDelete(NULL);
}
