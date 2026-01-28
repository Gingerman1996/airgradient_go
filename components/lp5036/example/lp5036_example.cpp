#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lp5036.h"

static const char *TAG = "LP5036_EXAMPLE";

// Example task: Cycle LED0 through RGB colors.
void lp5036_basic_example(void *arg) {
  auto i2c_bus = static_cast<i2c_master_bus_handle_t>(arg);
  drivers::LP5036 led_driver(i2c_bus, drivers::LP5036_I2C::ADDR_33);

  esp_err_t ret = led_driver.init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "LP5036 init failed: %s", esp_err_to_name(ret));
    vTaskDelete(NULL);
    return;
  }

  // Use linear brightness curve for predictable values.
  led_driver.set_log_scale(false);
  led_driver.set_bank_brightness(0xFF);

  const struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    const char *name;
  } color_cycle[] = {
      {0xFF, 0x00, 0x00, "red"},
      {0x00, 0xFF, 0x00, "green"},
      {0xFF, 0xFF, 0x00, "yellow"},
      {0xFF, 0x80, 0x00, "orange"},
      {0x80, 0x00, 0xFF, "purple"},
      {0x00, 0x00, 0xFF, "blue"},
  };

  for (uint8_t i = 0; i < drivers::LP5036_LED_COUNT; ++i) {
    led_driver.set_led_brightness(i, 0xFF);
    led_driver.set_led_color(i, 0x00, 0x00, 0xFF); // Red (BGR)
    ESP_LOGI(TAG, "LED%u -> red", i + 1);
    vTaskDelay(pdMS_TO_TICKS(120));
  }

  vTaskDelay(pdMS_TO_TICKS(300));

  for (const auto &c : color_cycle) {
    for (uint8_t i = 0; i < drivers::LP5036_LED_COUNT; ++i) {
      led_driver.set_led_color(i, c.b, c.g, c.r);
    }
    ESP_LOGI(TAG, "All LEDs -> %s", c.name);
    vTaskDelay(pdMS_TO_TICKS(350));
  }

  led_driver.set_global_off(true);
  led_driver.deinit();
  vTaskDelete(NULL);
}
