/**
 * @file display_example.cpp
 * @brief Standalone e-paper display example with random values and all icons
 * 
 * This example demonstrates:
 * - E-paper display initialization (144x296, SSD1680)
 * - LVGL integration with partial refresh
 * - Dashboard UI with all icons visible
 * - Random sensor values update
 * 
 * To use this example instead of main app:
 * 1. Rename app_main() in airgradient-go.cpp to app_main_original()
 * 2. Build and flash: idf.py build flash monitor
 */

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "epaper_config.h"
#include "epaper_lvgl.h"
#include "epaper_panel.h"
#include "lvgl.h"

#include "ui_display.h"

#include <stdlib.h>
#include <time.h>

// Display dimensions for GDEY0213B74H (2.13" 144x296)
#define DISPLAY_WIDTH 144
#define DISPLAY_HEIGHT 296

// LVGL configuration
#define LVGL_TICK_PERIOD_MS 5
#define LVGL_TASK_MAX_DELAY_MS 500
#define LVGL_TASK_MIN_DELAY_MS 10

static const char *TAG = "display_example";
static SemaphoreHandle_t lvgl_mux = NULL;
static Display *g_display = nullptr;
static epd_handle_t g_epd = nullptr;
static lv_display_t *g_disp = nullptr;
static volatile bool g_lvgl_refresh_requested = false;
static TaskHandle_t g_lvgl_task_handle = nullptr;

// Random value generators
static float random_float(float min, float max) {
  return min + ((float)rand() / RAND_MAX) * (max - min);
}

static int random_int(int min, int max) {
  return min + (rand() % (max - min + 1));
}

// LVGL tick timer callback
static void lv_tick_timer_cb(void *arg) {
  lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

// Lock LVGL mutex
static bool lvgl_lock(int timeout_ms) {
  if (!lvgl_mux) return false;
  return xSemaphoreTake(lvgl_mux, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

// Unlock LVGL mutex
static void lvgl_unlock(void) {
  if (lvgl_mux) {
    xSemaphoreGive(lvgl_mux);
  }
}

// Request LVGL refresh
static void request_lvgl_refresh(void) {
  g_lvgl_refresh_requested = true;
  if (g_lvgl_task_handle) {
    xTaskNotifyGive(g_lvgl_task_handle);
  }
}

// LVGL handler task
static void lv_handler_task(void *arg) {
  const TickType_t max_sleep = pdMS_TO_TICKS(LVGL_TASK_MAX_DELAY_MS);
  
  while (1) {
    uint32_t time_till_next = max_sleep;
    
    if (lvgl_lock(10)) {
      time_till_next = lv_timer_handler();
      
      // Handle refresh requests
      if (g_lvgl_refresh_requested) {
        g_lvgl_refresh_requested = false;
        lv_display_t *disp = lv_display_get_default();
        if (disp) {
          _lv_display_refr_timer(NULL);
        }
      }
      
      lvgl_unlock();
    }
    
    // Sleep until next update needed
    if (time_till_next == LV_NO_TIMER_READY || time_till_next > max_sleep) {
      time_till_next = max_sleep;
    }
    if (time_till_next < pdMS_TO_TICKS(LVGL_TASK_MIN_DELAY_MS)) {
      time_till_next = pdMS_TO_TICKS(LVGL_TASK_MIN_DELAY_MS);
    }
    
    ulTaskNotifyTake(pdTRUE, time_till_next);
  }
}

// Main entry point for display example
extern "C" void app_main_display_example(void) {
  ESP_LOGI(TAG, "=== E-Paper Display Example ===");
  ESP_LOGI(TAG, "Display: 144x296 SSD1680");
  
  // Seed random number generator
  srand(time(NULL));
  
  // ==================== DISPLAY INITIALIZATION ====================
  
  // Initialize LVGL
  lv_init();
  
  // Configure e-paper display (144x296 SSD16xx-based)
  epd_config_t epd_cfg = EPD_CONFIG_DEFAULT();
  epd_cfg.pins.busy = 10;
  epd_cfg.pins.rst = 9;
  epd_cfg.pins.dc = 15;
  epd_cfg.pins.cs = 0;
  epd_cfg.pins.sck = 23;
  epd_cfg.pins.mosi = 25;
  epd_cfg.pins.miso = 24;
  epd_cfg.spi.host = SPI2_HOST;
  epd_cfg.spi.speed_hz = 4000000; // 4 MHz
  epd_cfg.panel.type = EPD_PANEL_SSD16XX_290;
  epd_cfg.panel.width = DISPLAY_WIDTH;
  epd_cfg.panel.height = DISPLAY_HEIGHT;
  epd_cfg.panel.mirror_x = false;
  epd_cfg.panel.mirror_y = false;
  epd_cfg.panel.rotation = 0;
  
  esp_err_t ret = epd_init(&epd_cfg, &g_epd);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "E-paper init failed: %s", esp_err_to_name(ret));
    return;
  }
  
  // Get panel info
  epd_panel_info_t panel_info;
  epd_get_info(g_epd, &panel_info);
  ESP_LOGI(TAG, "Panel: %dx%d, buffer: %lu bytes", 
           panel_info.width, panel_info.height, panel_info.buffer_size);
  
  // Initialize LVGL display with PARTIAL refresh
  epd_lvgl_config_t lvgl_cfg = EPD_LVGL_CONFIG_DEFAULT();
  lvgl_cfg.epd = g_epd;
  lvgl_cfg.update_mode = EPD_UPDATE_PARTIAL;
  lvgl_cfg.use_partial_refresh = true;
  lvgl_cfg.partial_threshold = 1000;
  lvgl_cfg.dither_mode = EPD_DITHER_NONE;
  
  g_disp = epd_lvgl_init(&lvgl_cfg);
  if (!g_disp) {
    ESP_LOGE(TAG, "LVGL display init failed");
    epd_deinit(g_epd);
    return;
  }
  
  ESP_LOGI(TAG, "Display initialized successfully");
  
  // Setup LVGL tick timer
  esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = &lv_tick_timer_cb,
    .name = "lvgl_tick"
  };
  esp_timer_handle_t lvgl_tick_timer = NULL;
  ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));
  
  // Create LVGL task
  lvgl_mux = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(lv_handler_task, "LVGL", 8 * 1024, NULL, 4,
                          &g_lvgl_task_handle, tskNO_AFFINITY);
  
  // ==================== UI CREATION ====================
  
  Display display;
  g_display = &display;
  
  if (lvgl_lock(-1)) {
    // Create dashboard with all icons visible
    display.create_dashboard();
    ESP_LOGI(TAG, "Dashboard created");
    lvgl_unlock();
  }
  
  // Initial full refresh to show all icons
  ESP_LOGI(TAG, "Performing initial full refresh...");
  request_lvgl_refresh();
  vTaskDelay(pdMS_TO_TICKS(3000)); // Wait for first refresh
  
  // ==================== UPDATE LOOP WITH RANDOM VALUES ====================
  
  ESP_LOGI(TAG, "Starting random value updates (all icons visible)...");
  
  // Set GPS status to show GPS icon
  if (lvgl_lock(-1)) {
    display.set_gps_status(Display::GPSStatus::Searching);
    display.set_gps_time(12, 34); // Fixed time for icon visibility
    lvgl_unlock();
  }
  request_lvgl_refresh();
  vTaskDelay(pdMS_TO_TICKS(2000));
  
  uint64_t last_update_ms = esp_timer_get_time() / 1000;
  const uint64_t UPDATE_INTERVAL_MS = 5000; // Update every 5 seconds
  
  while (true) {
    uint64_t now_ms = esp_timer_get_time() / 1000;
    
    if (now_ms - last_update_ms >= UPDATE_INTERVAL_MS) {
      last_update_ms = now_ms;
      
      // Generate random sensor values
      sensor_values_t random_values = {
        .pm25_ugm3 = (uint16_t)random_int(5, 150),        // PM2.5: 5-150 µg/m³
        .co2_ppm = (uint16_t)random_int(400, 2000),       // CO2: 400-2000 ppm
        .tvoc_index = (uint16_t)random_int(50, 400),      // TVOC: 50-400 index
        .nox_index = (uint16_t)random_int(50, 400),       // NOx: 50-400 index
        .temperature_c = random_float(15.0f, 35.0f),      // Temp: 15-35°C
        .humidity_pct = random_float(30.0f, 80.0f),       // Humidity: 30-80%
        .pressure_pa = (uint32_t)random_int(98000, 103000) // Pressure: 980-1030 hPa
      };
      
      // Random battery state (0-100%)
      int battery_pct = random_int(20, 100);
      bool battery_charging = (rand() % 2) == 0; // Random charging state
      
      // Random GPS state
      Display::GPSStatus gps_states[] = {
        Display::GPSStatus::Off,
        Display::GPSStatus::Searching,
        Display::GPSStatus::Fix
      };
      Display::GPSStatus gps_status = gps_states[rand() % 3];
      
      float gps_lat = random_float(13.0f, 14.0f);
      float gps_lon = random_float(100.0f, 101.0f);
      
      // Update display with random values
      if (lvgl_lock(-1)) {
        // Update sensor readings
        display.update_sensor_readings(random_values);
        
        // Update battery
        display.set_battery(battery_pct, battery_charging);
        
        // Update GPS
        display.set_gps_status(gps_status);
        display.set_gps_time(random_int(0, 23), random_int(0, 59));
        if (gps_status == Display::GPSStatus::Fix) {
          display.set_gps_position(gps_lat, gps_lon);
        }
        
        lvgl_unlock();
      }
      
      // Request screen update
      request_lvgl_refresh();
      
      // Log current values
      ESP_LOGI(TAG, "Updated: PM2.5=%u CO2=%u TVOC=%u NOx=%u T=%.1f°C H=%.1f%% P=%lu Pa",
               random_values.pm25_ugm3, random_values.co2_ppm,
               random_values.tvoc_index, random_values.nox_index,
               random_values.temperature_c, random_values.humidity_pct,
               random_values.pressure_pa);
      ESP_LOGI(TAG, "Battery: %d%% %s | GPS: %s",
               battery_pct, battery_charging ? "(charging)" : "",
               gps_status == Display::GPSStatus::Fix ? "FIX" : 
               gps_status == Display::GPSStatus::Searching ? "SEARCHING" : "OFF");
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000)); // Check every 1 second
  }
}

// Comment out this line to use main app instead
// extern "C" void app_main(void) __attribute__((alias("app_main_display_example")));
