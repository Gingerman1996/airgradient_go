#include "sensor.h"
#include "ui_display.h"
#include "cap1203.h"
#include "bq25629.h"
#include "gps.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "epaper_config.h"
#include "epaper_panel.h"
#include "epaper_lvgl.h"
#include "lvgl.h"

// Display dimensions for GDEY0213B74H (2.13" 144x296)
#define DISPLAY_WIDTH  144
#define DISPLAY_HEIGHT 296

// LVGL configuration
#define LVGL_TICK_PERIOD_MS     5
#define LVGL_TASK_MAX_DELAY_MS  500
#define LVGL_TASK_MIN_DELAY_MS  10
#define LVGL_REFRESH_MIN_INTERVAL_MS 2000

// QON button configuration (GPIO5)
#define QON_PIN                GPIO_NUM_5
#define SHUTDOWN_HOLD_MS       5000  // 5 seconds long press to shutdown
// External hardware watchdog (TPL5010)
#define HW_WD_RST_PIN          GPIO_NUM_2
#define HW_WD_PULSE_MS         20
#define HW_WD_KICK_INTERVAL_MS (4 * 60 * 1000ULL)  // Kick before 5 min timeout

static SemaphoreHandle_t lvgl_mux = NULL;
static bool shutdown_requested = false;
static Display* g_display = nullptr;
static epd_handle_t g_epd = nullptr;
static lv_display_t* g_disp = nullptr;
static drivers::BQ25629* g_charger = nullptr;
static CAP1203* g_buttons = nullptr;
static Display::FocusTile g_focus_tile = Display::FocusTile::CO2;
static bool g_focus_dirty = true;
static volatile bool g_lvgl_refresh_requested = false;
static volatile bool g_lvgl_refresh_urgent = false;
static TaskHandle_t g_lvgl_task_handle = nullptr;

// Button callback for CAP1203 (called when button events occur)
static void button_event_callback(ButtonState state, void* user_data) {
    static const char *TAG_BTN = "CAP1203";
    
    const char* button_name = "UNKNOWN";
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
            ESP_LOGI(TAG_BTN, "[%s] RELEASE (duration: %lu ms)", button_name, state.press_duration_ms);
            break;
        case ButtonEvent::SHORT_PRESS:
            ESP_LOGI(TAG_BTN, "[%s] SHORT_PRESS", button_name);
            break;
        case ButtonEvent::LONG_PRESS:
            ESP_LOGI(TAG_BTN, "[%s] LONG_PRESS (duration: %lu ms)", button_name, state.press_duration_ms);
            break;
        default:
            break;
    }

    bool focus_event = (state.event == ButtonEvent::PRESS ||
                        state.event == ButtonEvent::SHORT_PRESS ||
                        state.event == ButtonEvent::LONG_PRESS);
    if (focus_event) {
        bool focus_changed = false;
        const char* focus_label = nullptr;
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
        }
        if (focus_changed) {
            g_focus_dirty = true;
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
        [](void* param) {
            CAP1203* buttons = (CAP1203*)param;
            while (true) {
                buttons->processButtons();
                vTaskDelay(pdMS_TO_TICKS(10));  // Poll every 10ms
            }
        },
        "ButtonTask",
        4096,
        g_buttons,
        5,  // Priority
        NULL,
        tskNO_AFFINITY
    );

    ESP_LOGI(TAG_BTN, "CAP1203 initialized successfully");
    return ESP_OK;
}

// LVGL tick timer callback (increments LVGL internal tick counter)
static void lv_tick_timer_cb(void *arg) {
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

// Lock LVGL mutex (blocking)
static bool lvgl_lock(int timeout_ms) {
    const TickType_t timeout_ticks = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(lvgl_mux, timeout_ticks) == pdTRUE;
}

// Unlock LVGL mutex
static void lvgl_unlock(void) {
    xSemaphoreGive(lvgl_mux);
}

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
    
    // Phase 1: Stopping sensors
    ESP_LOGI(TAG, "Phase 1: Stopping sensors...");
    if (lvgl_lock(1000) && g_display) {
        lv_obj_t* label = lv_label_create(lv_screen_active());
        lv_label_set_text(label, "Shutting down...\n\nStopping sensors...");
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lvgl_unlock();
    }
    request_lvgl_refresh();
    vTaskDelay(pdMS_TO_TICKS(800));
    
    // TODO: Stop sensor tasks when implemented
    
    // Phase 2: Stopping recording
    ESP_LOGI(TAG, "Phase 2: Stopping recording...");
    if (lvgl_lock(1000) && g_display) {
        lv_obj_t* label = lv_label_create(lv_screen_active());
        lv_label_set_text(label, "Shutting down...\n\nStopping recording...");
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lvgl_unlock();
    }
    request_lvgl_refresh();
    vTaskDelay(pdMS_TO_TICKS(800));
    
    // TODO: Stop recording when implemented
    
    // Phase 3: Saving data
    ESP_LOGI(TAG, "Phase 3: Saving data...");
    if (lvgl_lock(1000) && g_display) {
        lv_obj_t* label = lv_label_create(lv_screen_active());
        lv_label_set_text(label, "Shutting down...\n\nSaving data...");
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lvgl_unlock();
    }
    request_lvgl_refresh();
    vTaskDelay(pdMS_TO_TICKS(800));
    
    // TODO: Save pending data to flash when implemented
    
    // Phase 4: Powering off
    ESP_LOGI(TAG, "Phase 4: Powering off...");
    if (lvgl_lock(1000) && g_display) {
        lv_obj_t* label = lv_label_create(lv_screen_active());
        lv_label_set_text(label, "Powering off...\n\nGoodbye!");
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lvgl_unlock();
    }
    request_lvgl_refresh();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Phase 5: Clear display (prevent ghosting)
    ESP_LOGI(TAG, "Phase 5: Clearing display...");
    if (g_epd) {
        epd_clear(g_epd);
        vTaskDelay(pdMS_TO_TICKS(100));  // Wait for clear to complete
    }
    
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
        }
        else if (is_pressed && was_pressed && !shutdown_requested) {
            // Button still held
            uint64_t hold_duration_ms = now_ms - qon_press_start_ms;
            
            if (hold_duration_ms >= SHUTDOWN_HOLD_MS) {
                ESP_LOGI(TAG, "QON button held for %llu ms - SHUTDOWN!", hold_duration_ms);
                shutdown_requested = true;
                initiate_shutdown();
            }
        }
        else if (!is_pressed && was_pressed) {
            // Button released
            uint64_t hold_duration_ms = now_ms - qon_press_start_ms;
            ESP_LOGI(TAG, "QON button released after %llu ms", hold_duration_ms);
            qon_press_start_ms = 0;
        }
        
        was_pressed = is_pressed;
        vTaskDelay(pdMS_TO_TICKS(50));  // Poll every 50ms
    }
}

// LVGL handler task (processes display updates)
static void lv_handler_task(void *arg) {
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

// Application entry point.
// Phase 1.1: Dashboard display with real sensor data
extern "C" void app_main(void) {
    static const char *TAG = "app_main";
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    const uint64_t RECORDING_INTERVAL_MS = 10000;  // 10s default, matches UI interval

    ESP_LOGI(TAG, "=== AirGradient GO - Phase 1.1 ===");
    ESP_LOGI(TAG, "Initializing display and sensors...");

    // ==================== GPIO INITIALIZATION ====================
    
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

    // Configure e-paper display (144x296 SSD16xx-based, rotated 180°)
    epd_config_t epd_cfg = EPD_CONFIG_DEFAULT();
    epd_cfg.pins.busy = 10;   // IO10
    epd_cfg.pins.rst = 9;     // IO9
    epd_cfg.pins.dc = 15;     // IO15
    epd_cfg.pins.cs = 0;      // IO0
    epd_cfg.pins.sck = 23;    // IO23
    epd_cfg.pins.mosi = 25;   // IO25
    epd_cfg.spi.host = SPI2_HOST;
    epd_cfg.spi.speed_hz = 4000000;  // 4 MHz (same as working ESP32-C6 code)
    epd_cfg.panel.type = EPD_PANEL_SSD16XX_290;  // Try 2.9" config (128x296) - closer to our 144x296
    epd_cfg.panel.width = DISPLAY_WIDTH;
    epd_cfg.panel.height = DISPLAY_HEIGHT;
    epd_cfg.panel.mirror_x = false;
    epd_cfg.panel.mirror_y = false;
    epd_cfg.panel.rotation = 0;

    esp_err_t ret = epd_init(&epd_cfg, &g_epd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init e-paper: %s", esp_err_to_name(ret));
        return;
    }
    epd_handle_t epd = g_epd;  // Local alias for compatibility

    // Get panel info
    epd_panel_info_t panel_info;
    epd_get_info(epd, &panel_info);
    ESP_LOGI(TAG, "Panel: %dx%d, buffer: %lu bytes", 
             panel_info.width, panel_info.height, panel_info.buffer_size);

    // Initialize LVGL display with PARTIAL refresh for faster updates
    epd_lvgl_config_t lvgl_cfg = EPD_LVGL_CONFIG_DEFAULT();
    lvgl_cfg.epd = epd;
    lvgl_cfg.update_mode = EPD_UPDATE_PARTIAL;  // Use PARTIAL refresh for fast value updates
    lvgl_cfg.use_partial_refresh = true;        // Enable partial refresh
    lvgl_cfg.partial_threshold = 1000;          // Partial refresh threshold
    lvgl_cfg.dither_mode = EPD_DITHER_NONE;     // Disable dithering for crisp B/W text

    g_disp = epd_lvgl_init(&lvgl_cfg);
    if (!g_disp) {
        ESP_LOGE(TAG, "Failed to init LVGL display");
        epd_deinit(epd);
        return;
    }
    lv_display_t *disp = g_disp;  // Local alias for compatibility

    // Setup LVGL tick timer
    esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lv_tick_timer_cb,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));

    // Create LVGL task (ESP32-C5 is unicore, use tskNO_AFFINITY)
    lvgl_mux = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(lv_handler_task,
                            "LVGL",
                            8 * 1024,
                            NULL,
                            4,
                            &g_lvgl_task_handle,
                            tskNO_AFFINITY);

    // ==================== UI CREATION ====================
    
    Display display;
    g_display = &display;  // Store pointer for shutdown access
    
    if (lvgl_lock(-1)) {
        bool ui_ok = display.init(DISPLAY_WIDTH, DISPLAY_HEIGHT);
        if (!ui_ok) {
            ESP_LOGE(TAG, "Failed to init Display UI");
            lvgl_unlock();
            epd_deinit(epd);
            return;
        }
        
        // TEST SCREEN: Show all status icons and max values to verify display
        ESP_LOGI(TAG, "Displaying TEST SCREEN with maximum values...");
        
        // Status bar: Show all icons active
        display.setWiFiStatus(Display::WiFiStatus::Connected);  // WiFi icon visible
        display.setGPSStatus(Display::GPSStatus::Fix);          // GPS icon visible
        display.setBLEStatus(Display::BLEStatus::Connected);    // BLE icon visible
        display.setBattery(100, true);                           // Battery 100% + charging
        display.setRecording(true);                              // REC icon visible
        display.setIntervalSeconds(RECORDING_INTERVAL_MS / 1000); // Interval shown
        display.setTimeHM(23, 59, true);                        // Time 23:59 (4 digits)
        display.setAlert(true);                                  // Alert icon visible
        
        // Sensor values: Maximum values for display segment testing
        display.setPM25f(999.9f);      // PM2.5: 999.9 µg/m³ (max 4 digits)
        display.setCO2(9999);          // CO2: 9999 ppm (max 4 digits)
        display.setVOC(999);           // VOC: 999 (max 3 digits)
        display.setNOx(999);           // NOx: 999 (max 3 digits)
        display.setTempCf(100.0f);     // Temp: 100.0°C (test max display)
        display.setRHf(99.9f);         // RH: 99.9% (reasonable max value)
        display.setPressure(1013.0);   // Pressure: 1013 hPa (standard pressure)
        
        lvgl_unlock();
    }

    ESP_LOGI(TAG, "TEST SCREEN displayed, performing initial full refresh...");
    request_lvgl_refresh();
    
    // Wait 3 seconds to let user see test screen
    ESP_LOGI(TAG, "TEST SCREEN visible, waiting 3 seconds...");
    vTaskDelay(pdMS_TO_TICKS(3000));

    // ==================== I2C BUS INITIALIZATION ====================
    
    // Initialize I2C bus first (needed for BQ25629 and sensors)
    ESP_LOGI(TAG, "Initializing I2C bus...");
    Sensors sensors;
    GPS gps;
    bool gps_ready = false;
    i2c_master_bus_handle_t i2c_bus = sensors.getI2CBusHandle();
    if (i2c_bus == NULL) {
        ESP_LOGE(TAG, "Failed to get I2C bus handle");
        return;
    }
    
    // ==================== CHARGER INITIALIZATION (BEFORE SENSORS!) ====================
    
    // CRITICAL: Initialize BQ25629 BEFORE sensors to ensure PMID power path is ready
    // PMID provides power to VSYS which powers all sensors including SPS30
    ESP_LOGI(TAG, "Initializing BQ25629 battery charger (PMID power path)...");
    g_charger = new drivers::BQ25629(i2c_bus);
    
    drivers::BQ25629_Config charger_cfg = {
        .charge_voltage_mv = 4200,       // 4.2V (full charge for Li-ion)
        .charge_current_ma = 1000,       // 1A charging
        .input_current_limit_ma = 1500,  // 1.5A input limit
        .input_voltage_limit_mv = 4600,  // 4.6V VINDPM
        .min_system_voltage_mv = 3520,   // 3.52V minimum
        .precharge_current_ma = 30,      // 30mA pre-charge
        .term_current_ma = 20,           // 20mA termination
        .enable_charging = true,         // Enable charging
        .enable_otg = false,             // Disable OTG mode
        .enable_adc = true,              // Enable ADC for monitoring
    };
    
    ret = g_charger->init(charger_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "BQ25629 init failed: %s", esp_err_to_name(ret));
        delete g_charger;
        g_charger = nullptr;
    } else {
        ESP_LOGI(TAG, "BQ25629 initialized successfully");
        
        // Verify PMID is working by reading ADC
        drivers::BQ25629_ADC_Data adc_data;
        ret = g_charger->read_adc(adc_data);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "PMID voltage: %u mV, VSYS: %u mV, VBAT: %u mV", 
                     adc_data.vpmid_mv, adc_data.vsys_mv, adc_data.vbat_mv);
            
            if (adc_data.vpmid_mv < 3000) {
                ESP_LOGW(TAG, "PMID voltage too low! Power path may not be working correctly");
            }
        } else {
            ESP_LOGW(TAG, "Failed to read BQ25629 ADC: %s", esp_err_to_name(ret));
        }
        
        // Ensure PMID discharge is disabled (not forcing discharge)
        g_charger->enable_pmid_discharge(false);
        
        // Configure PMID for 5V OTG boost output
        ESP_LOGI(TAG, "Configuring PMID for 5V boost output...");
        
        // Read initial status and faults
        drivers::BQ25629_Status charger_status;
        drivers::BQ25629_Fault charger_fault;
        
        ret = g_charger->read_status(charger_status);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG,
                     "Initial status - VBUS: %d, CHG: %d, VSYS_REG: %d, IINDPM: %d, VINDPM: %d",
                     (int)charger_status.vbus_status,
                     (int)charger_status.charge_status,
                     charger_status.vsys_stat,
                     charger_status.iindpm_stat,
                     charger_status.vindpm_stat);
        }
        
        ret = g_charger->read_fault(charger_fault);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG,
                     "Fault status - VBUS_FAULT: %d, BAT_FAULT: %d, SYS_FAULT: %d, "
                     "OTG_FAULT: %d, TSHUT: %d, TS_STAT: %u",
                     charger_fault.vbus_fault,
                     charger_fault.bat_fault,
                     charger_fault.sys_fault,
                     charger_fault.otg_fault,
                     charger_fault.tshut_fault,
                     charger_fault.ts_stat);
        }
        
        // Enable PMID 5V boost using complete setup sequence
        ESP_LOGI(TAG, "Enabling PMID 5V boost output...");
        ret = g_charger->enable_pmid_5v_boost();
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to enable PMID 5V boost: %s", esp_err_to_name(ret));
        }
        
        // Wait for boost to stabilize
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // Read status and faults after boost enable
        ret = g_charger->read_status(charger_status);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "After boost - VBUS: %d, CHG: %d, IOTG_REG: %d, VOTG_REG: %d",
                     (int)charger_status.vbus_status,
                     (int)charger_status.charge_status,
                     charger_status.iindpm_stat,
                     charger_status.vindpm_stat);
        }
        
        ret = g_charger->read_fault(charger_fault);
        if (ret == ESP_OK) {
            if (charger_fault.otg_fault || charger_fault.tshut_fault ||
                charger_fault.ts_stat != 0) {
                ESP_LOGW(TAG, "OTG fault/TS condition - OTG: %d, TSHUT: %d, TS_STAT: %u",
                         charger_fault.otg_fault,
                         charger_fault.tshut_fault,
                         charger_fault.ts_stat);
                ESP_LOGW(TAG, "PMID_GD may be LOW due to fault/TS condition");
            } else {
                ESP_LOGI(TAG, "No OTG faults detected");
            }
        }
        
        // Re-read ADC to verify boost voltage
        ret = g_charger->read_adc(adc_data);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Final PMID: %u mV, VSYS: %u mV, VBUS: %u mV, VBAT: %u mV", 
                     adc_data.vpmid_mv, adc_data.vsys_mv, adc_data.vbus_mv, adc_data.vbat_mv);
            
            if (adc_data.vpmid_mv >= 4500) {
                ESP_LOGI(TAG, "PMID boost successful! PMID_GD should be HIGH");
            } else {
                ESP_LOGW(TAG, "PMID voltage lower than expected (%umV vs 5000mV target)", 
                         adc_data.vpmid_mv);
            }
        }
        
        ESP_LOGI(TAG, "PMID power path ready for sensors");
    }
    
    // Wait a bit for PMID to stabilize
    vTaskDelay(pdMS_TO_TICKS(100));
    
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
    }
    
    // ==================== QON BUTTON TASK ====================
    
    ESP_LOGI(TAG, "Starting QON button monitor task...");
    xTaskCreatePinnedToCore(
        qon_button_task,
        "QON_Task",
        4096,
        NULL,
        6,  // High priority for shutdown detection
        NULL,
        tskNO_AFFINITY
    );

    // ==================== MAIN LOOP ====================
    
    ESP_LOGI(TAG, "Starting normal operation with real sensor values...");
    
    uint64_t last_display_update_ms = 0;
    uint64_t last_display_refresh_ms = 0;
    const uint64_t DISPLAY_UPDATE_INTERVAL_MS = 1000;    // Update values every 1s
    const uint64_t DISPLAY_REFRESH_INTERVAL_MS = RECORDING_INTERVAL_MS; // Follow recording interval
    const uint64_t UI_BLINK_INTERVAL_MS = 500;           // Blink icons every 500ms
    const uint64_t GPS_UI_UPDATE_INTERVAL_MS = 1000;
    const uint64_t GPS_SENTENCE_TIMEOUT_MS = 3000;
    const uint64_t GPS_FIX_TIMEOUT_MS = 5000;
    bool initial_display_refresh_done = false;
    uint64_t last_charger_log_ms = 0;
    const uint64_t CHARGER_LOG_INTERVAL_MS = 2000;       // Debug log interval
    uint64_t last_wd_kick_ms = 0;
    const uint64_t WD_KICK_INTERVAL_MS = 10000;          // Reset charger watchdog
    uint64_t last_ui_blink_ms = 0;
    uint64_t last_hw_wd_kick_ms = 0;
    uint64_t last_gps_ui_ms = 0;
    Display::GPSStatus last_gps_status = Display::GPSStatus::Off;
    bool last_gps_time_valid = false;
    int last_gps_hour = -1;
    int last_gps_min = -1;

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

        // Update display with real sensor values every 1 second
        if (now_ms_u - last_display_update_ms >= DISPLAY_UPDATE_INTERVAL_MS) {
            last_display_update_ms = now_ms_u;
            bool request_initial_refresh = false;

            // Update display (thread-safe with LVGL mutex)
            if (lvgl_lock(100)) {
                // Get real sensor values
                sensor_values_t vals;
                sensors.getValues(now_ms, &vals);
                
                if (vals.have_co2_avg) {
                    display.setCO2(vals.co2_ppm_avg);
                    display.setTempCf(vals.temp_c_avg);
                    display.setRHf(vals.rh_avg);
                }
                display.setPM25f(vals.pm25_mass);
                display.setVOC(vals.voc_index);
                display.setNOx(vals.nox_index);
                display.setPressure(1013);  // TODO: Add pressure sensor when DPS368 is implemented

                ESP_LOGD(TAG,
                         "Display updated - CO2: %d ppm (avg_ready=%d), T: %.1fC, RH: %.1f%%, "
                         "PM2.5: %.1f, VOC idx: %d, NOx idx: %d",
                         vals.co2_ppm_avg,
                         vals.have_co2_avg,
                         vals.temp_c_avg,
                         vals.rh_avg,
                         vals.pm25_mass,
                         vals.voc_index,
                         vals.nox_index);

                if (!initial_display_refresh_done) {
                    ESP_LOGI(TAG, "Initial display refresh after first data update");
                    initial_display_refresh_done = true;
                    last_display_refresh_ms = now_ms_u;
                    request_initial_refresh = true;
                }

                lvgl_unlock();
            }
            if (request_initial_refresh) {
                request_lvgl_refresh();
            }
        }

        if (now_ms_u - last_ui_blink_ms >= UI_BLINK_INTERVAL_MS) {
            last_ui_blink_ms = now_ms_u;
            bool updated = false;
            if (lvgl_lock(100)) {
                display.update(now_ms_u);
                updated = true;
                lvgl_unlock();
            }
            if (updated) {
                request_lvgl_refresh();
            }
        }

        if (gps_ready && (now_ms_u - last_gps_ui_ms >= GPS_UI_UPDATE_INTERVAL_MS)) {
            last_gps_ui_ms = now_ms_u;
            Display::GPSStatus gps_status = Display::GPSStatus::Off;
            bool has_sentence = gps.has_recent_sentence(now_ms_u, GPS_SENTENCE_TIMEOUT_MS);
            if (has_sentence) {
                gps_status = gps.has_recent_fix(now_ms_u, GPS_FIX_TIMEOUT_MS)
                                 ? Display::GPSStatus::Fix
                                 : Display::GPSStatus::Searching;
            }

            bool time_valid = has_sentence && gps.has_time();
            int hour = gps.utc_hour();
            int min = gps.utc_min();

            bool gps_changed = (gps_status != last_gps_status) ||
                               (time_valid != last_gps_time_valid) ||
                               (time_valid && (hour != last_gps_hour || min != last_gps_min));
            if (gps_changed) {
                bool updated = false;
                if (lvgl_lock(100)) {
                    display.setGPSStatus(gps_status);
                    if (time_valid) {
                        display.setTimeHM(hour, min, true);
                    } else {
                        display.setTimeHM(0, 0, false);
                    }
                    lvgl_unlock();
                    updated = true;
                }
                if (updated) {
                    request_lvgl_refresh();
                    last_gps_status = gps_status;
                    last_gps_time_valid = time_valid;
                    last_gps_hour = hour;
                    last_gps_min = min;
                }
            }
        }

        if (g_focus_dirty) {
            Display::FocusTile focus_tile = g_focus_tile;
            g_focus_dirty = false;
            bool request_focus_refresh = false;
            if (lvgl_lock(100)) {
                display.setFocusTile(focus_tile);
                request_focus_refresh = true;
                lvgl_unlock();
            }
            if (request_focus_refresh) {
                request_lvgl_refresh_urgent();
                last_display_refresh_ms = now_ms_u;
            }
        }

        // Periodic display refresh (partial) per recording interval
        if (now_ms_u - last_display_refresh_ms >= DISPLAY_REFRESH_INTERVAL_MS) {
            last_display_refresh_ms = now_ms_u;
            
            ESP_LOGI(TAG, "Display refresh at %llu ms", now_ms_u);
            request_lvgl_refresh();
        }

        // Periodic charger watchdog reset to avoid OTG drop
        if (g_charger && (now_ms_u - last_wd_kick_ms >= WD_KICK_INTERVAL_MS)) {
            last_wd_kick_ms = now_ms_u;
            esp_err_t wd_ret = g_charger->reset_watchdog();
            if (wd_ret != ESP_OK) {
                ESP_LOGW(TAG, "BQ25629 watchdog reset failed: %s", esp_err_to_name(wd_ret));
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
        if (g_charger && (now_ms_u - last_charger_log_ms >= CHARGER_LOG_INTERVAL_MS)) {
            last_charger_log_ms = now_ms_u;

            drivers::BQ25629_Status charger_status = {};
            drivers::BQ25629_Fault charger_fault = {};
            drivers::BQ25629_ADC_Data adc_data = {};

            esp_err_t status_ret = g_charger->read_status(charger_status);
            esp_err_t fault_ret = g_charger->read_fault(charger_fault);
            esp_err_t adc_ret = g_charger->read_adc(adc_data);

            if (status_ret == ESP_OK) {
                ESP_LOGI(TAG, "BQ25629 dbg - VBUS: %d, CHG: %d, IOTG_REG: %d, VOTG_REG: %d, WD: %d",
                         (int)charger_status.vbus_status,
                         (int)charger_status.charge_status,
                         charger_status.iindpm_stat,
                         charger_status.vindpm_stat,
                         charger_status.wd_stat);
            } else {
                ESP_LOGW(TAG, "BQ25629 status read failed: %s", esp_err_to_name(status_ret));
            }

            if (fault_ret == ESP_OK) {
                ESP_LOGI(TAG, "BQ25629 fault - OTG: %d, TSHUT: %d, TS_STAT: %u, VBUS: %d, BAT: %d, SYS: %d",
                         charger_fault.otg_fault,
                         charger_fault.tshut_fault,
                         charger_fault.ts_stat,
                         charger_fault.vbus_fault,
                         charger_fault.bat_fault,
                         charger_fault.sys_fault);
                if (charger_fault.otg_fault || charger_fault.tshut_fault || charger_fault.ts_stat != 0) {
                    ESP_LOGW(TAG, "BQ25629 fault/TS active - OTG: %d, TSHUT: %d, TS_STAT: %u",
                             charger_fault.otg_fault,
                             charger_fault.tshut_fault,
                             charger_fault.ts_stat);
                }
            } else {
                ESP_LOGW(TAG, "BQ25629 fault read failed: %s", esp_err_to_name(fault_ret));
            }

            if (adc_ret == ESP_OK) {
                ESP_LOGI(TAG, "BQ25629 adc - VPMID: %u mV, VBAT: %u mV, VSYS: %u mV, VBUS: %u mV",
                         adc_data.vpmid_mv,
                         adc_data.vbat_mv,
                         adc_data.vsys_mv,
                         adc_data.vbus_mv);
                if (adc_data.vpmid_mv < 4000) {
                    ESP_LOGW(TAG, "PMID low: %u mV (VBAT: %u mV)", adc_data.vpmid_mv, adc_data.vbat_mv);
                }
            } else {
                ESP_LOGW(TAG, "BQ25629 adc read failed: %s", esp_err_to_name(adc_ret));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
