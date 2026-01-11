#include "sensor.h"
#include "ui_display.h"

#include "esp_log.h"
#include "esp_timer.h"
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

static SemaphoreHandle_t lvgl_mux = NULL;

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

// LVGL handler task (processes display updates)
static void lv_handler_task(void *arg) {
    uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
    while (1) {
        if (lvgl_lock(-1)) {
            task_delay_ms = lv_timer_handler();
            lvgl_unlock();
        }
        if (task_delay_ms > LVGL_TASK_MAX_DELAY_MS) {
            task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

// Application entry point.
// Phase 1.1: Dashboard display with real sensor data
extern "C" void app_main(void) {
    static const char *TAG = "app_main";

    ESP_LOGI(TAG, "=== AirGradient GO - Phase 1.1 ===");
    ESP_LOGI(TAG, "Initializing display and sensors...");

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

    epd_handle_t epd = NULL;
    esp_err_t ret = epd_init(&epd_cfg, &epd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init e-paper: %s", esp_err_to_name(ret));
        return;
    }

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

    lv_display_t *disp = epd_lvgl_init(&lvgl_cfg);
    if (!disp) {
        ESP_LOGE(TAG, "Failed to init LVGL display");
        epd_deinit(epd);
        return;
    }

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
    xTaskCreatePinnedToCore(lv_handler_task, "LVGL", 8 * 1024, NULL, 4, NULL, tskNO_AFFINITY);

    // ==================== UI CREATION ====================
    
    Display display;
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
        display.setIntervalSeconds(10);                          // Interval shown
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
    
    // Initial full refresh to show test screen
    if (lvgl_lock(-1)) {
        epd_lvgl_refresh(disp);
        lvgl_unlock();
    }
    
    // Wait 3 seconds to let user see test screen
    ESP_LOGI(TAG, "TEST SCREEN visible, waiting 3 seconds...");
    vTaskDelay(pdMS_TO_TICKS(3000));

    // ==================== SENSOR INITIALIZATION ====================
    
    Sensors sensors;
    ret = sensors.init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Sensors init failed: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Sensors initialized");
    }

    // ==================== MAIN LOOP ====================
    
    ESP_LOGI(TAG, "Starting normal operation with real sensor values...");
    
    uint64_t last_display_update_ms = 0;
    uint64_t last_display_refresh_ms = 0;
    const uint64_t DISPLAY_UPDATE_INTERVAL_MS = 1000;    // Update values every 1s
    const uint64_t DISPLAY_REFRESH_INTERVAL_MS = 30000;  // Partial refresh every 30s

    while (true) {
        int64_t now_ms = esp_timer_get_time() / 1000;
        uint64_t now_ms_u = (uint64_t)now_ms;

        // Update sensors
        sensors.update(now_ms);

        // Update display with real sensor values every 1 second
        if (now_ms_u - last_display_update_ms >= DISPLAY_UPDATE_INTERVAL_MS) {
            last_display_update_ms = now_ms_u;

            // Update display (thread-safe with LVGL mutex)
            if (lvgl_lock(100)) {
                // Get real sensor values
                sensor_values_t vals;
                sensors.getValues(now_ms, &vals);
                
                display.setCO2(vals.co2_ppm_avg);
                display.setTempCf(vals.temp_c_avg);
                display.setRHf(vals.rh_avg);
                display.setPM25f(vals.pm25_mass);
                display.setVOC(vals.voc_index);
                display.setNOx(vals.nox_index);
                display.setPressure(1013);  // TODO: Add pressure sensor when DPS368 is implemented
                
                // Update blink animations (REC, charging)
                display.update(now_ms_u);
                
                lvgl_unlock();
            }
        }

        // Periodic display refresh (partial) every 30 seconds
        if (now_ms_u - last_display_refresh_ms >= DISPLAY_REFRESH_INTERVAL_MS) {
            last_display_refresh_ms = now_ms_u;
            
            ESP_LOGI(TAG, "Display refresh at %llu ms", now_ms_u);
            if (lvgl_lock(100)) {
                epd_lvgl_refresh(disp);
                lvgl_unlock();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
