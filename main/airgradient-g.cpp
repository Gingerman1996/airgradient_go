#include "sensor.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Application entry point.
// Initializes sensors and runs a 50 ms loop updating sensor state machines.
extern "C" void app_main(void) {
    static const char *TAG = "app_main";

    Sensors sensors;
    esp_err_t ret = sensors.init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Sensors init failed: %s", esp_err_to_name(ret));
    }

    while (true) {
        int64_t now_ms = esp_timer_get_time() / 1000;
        sensors.update(now_ms);

        // Optional: fetch averaged values for display/logging
        // sensor_values_t vals;
        // sensors.getValues(now_ms, &vals);
        // ESP_LOGD(TAG, "CO2=%d ppm avg=%d", vals.co2_ppm_avg, vals.have_co2_avg);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
