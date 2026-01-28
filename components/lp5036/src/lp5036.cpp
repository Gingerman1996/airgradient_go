/**
 * @file lp5036.cpp
 * @brief LP5036 36-channel RGB LED driver implementation
 */

#include "lp5036.h"
#include "esp_log.h"

namespace drivers {

static const char *TAG = "LP5036";

constexpr int I2C_TIMEOUT_MS = 1000;
constexpr uint32_t I2C_CLOCK_SPEED_HZ = 100000;

// DEVICE_CONFIG0
constexpr uint8_t DEVICE_CONFIG0_CHIP_EN = (1 << 6);

// DEVICE_CONFIG1
constexpr uint8_t DEVICE_CONFIG1_LOG_SCALE_EN = (1 << 5);
constexpr uint8_t DEVICE_CONFIG1_POWER_SAVE_EN = (1 << 4);
constexpr uint8_t DEVICE_CONFIG1_AUTO_INCR_EN = (1 << 3);
constexpr uint8_t DEVICE_CONFIG1_PWM_DITHER_EN = (1 << 2);
constexpr uint8_t DEVICE_CONFIG1_MAX_CURRENT_OPTION = (1 << 1);
constexpr uint8_t DEVICE_CONFIG1_LED_GLOBAL_OFF = (1 << 0);

LP5036::LP5036(i2c_master_bus_handle_t i2c_bus, uint8_t i2c_addr)
    : i2c_bus_(i2c_bus), dev_handle_(nullptr), device_address_(i2c_addr) {}

LP5036::~LP5036() {
  deinit();
}

esp_err_t LP5036::init() {
  if (i2c_bus_ == nullptr) {
    ESP_LOGE(TAG, "I2C bus handle is null");
    return ESP_ERR_INVALID_STATE;
  }

  i2c_device_config_t dev_cfg = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = device_address_,
      .scl_speed_hz = I2C_CLOCK_SPEED_HZ,
      .scl_wait_us = 0,
      .flags = {}};

  esp_err_t ret = i2c_master_bus_add_device(i2c_bus_, &dev_cfg, &dev_handle_);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add I2C device: %s", esp_err_to_name(ret));
    return ret;
  }

  ret = enable(true);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable device: %s", esp_err_to_name(ret));
    return ret;
  }

  ret = set_auto_increment(true);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable auto-increment: %s", esp_err_to_name(ret));
    return ret;
  }

  ret = set_global_off(false);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to clear global off: %s", esp_err_to_name(ret));
    return ret;
  }

  return ESP_OK;
}

esp_err_t LP5036::deinit() {
  if (dev_handle_ != nullptr) {
    esp_err_t ret = i2c_master_bus_rm_device(dev_handle_);
    if (ret != ESP_OK) {
      ESP_LOGW(TAG, "Failed to remove I2C device: %s", esp_err_to_name(ret));
    }
    dev_handle_ = nullptr;
  }
  return ESP_OK;
}

esp_err_t LP5036::enable(bool enable) {
  uint8_t value = enable ? DEVICE_CONFIG0_CHIP_EN : 0x00;
  return modify_register(LP5036_REG::DEVICE_CONFIG0, DEVICE_CONFIG0_CHIP_EN,
                         value);
}

esp_err_t LP5036::reset() {
  return write_register(LP5036_REG::RESET, 0xFF);
}

esp_err_t LP5036::set_global_off(bool off) {
  uint8_t value = off ? DEVICE_CONFIG1_LED_GLOBAL_OFF : 0x00;
  return modify_register(LP5036_REG::DEVICE_CONFIG1,
                         DEVICE_CONFIG1_LED_GLOBAL_OFF, value);
}

esp_err_t LP5036::set_log_scale(bool enable) {
  uint8_t value = enable ? DEVICE_CONFIG1_LOG_SCALE_EN : 0x00;
  return modify_register(LP5036_REG::DEVICE_CONFIG1,
                         DEVICE_CONFIG1_LOG_SCALE_EN, value);
}

esp_err_t LP5036::set_power_save(bool enable) {
  uint8_t value = enable ? DEVICE_CONFIG1_POWER_SAVE_EN : 0x00;
  return modify_register(LP5036_REG::DEVICE_CONFIG1,
                         DEVICE_CONFIG1_POWER_SAVE_EN, value);
}

esp_err_t LP5036::set_auto_increment(bool enable) {
  uint8_t value = enable ? DEVICE_CONFIG1_AUTO_INCR_EN : 0x00;
  return modify_register(LP5036_REG::DEVICE_CONFIG1,
                         DEVICE_CONFIG1_AUTO_INCR_EN, value);
}

esp_err_t LP5036::set_pwm_dither(bool enable) {
  uint8_t value = enable ? DEVICE_CONFIG1_PWM_DITHER_EN : 0x00;
  return modify_register(LP5036_REG::DEVICE_CONFIG1,
                         DEVICE_CONFIG1_PWM_DITHER_EN, value);
}

esp_err_t LP5036::set_max_current_option(bool high_current) {
  uint8_t value = high_current ? DEVICE_CONFIG1_MAX_CURRENT_OPTION : 0x00;
  return modify_register(LP5036_REG::DEVICE_CONFIG1,
                         DEVICE_CONFIG1_MAX_CURRENT_OPTION, value);
}

esp_err_t LP5036::set_bank_brightness(uint8_t brightness) {
  return write_register(LP5036_REG::BANK_BRIGHTNESS, brightness);
}

esp_err_t LP5036::set_bank_color(LP5036Bank bank, uint8_t value) {
  uint8_t reg = 0;
  switch (bank) {
  case LP5036Bank::BANK_A:
    reg = LP5036_REG::BANK_A_COLOR;
    break;
  case LP5036Bank::BANK_B:
    reg = LP5036_REG::BANK_B_COLOR;
    break;
  case LP5036Bank::BANK_C:
    reg = LP5036_REG::BANK_C_COLOR;
    break;
  default:
    return ESP_ERR_INVALID_ARG;
  }
  return write_register(reg, value);
}

esp_err_t LP5036::set_led_bank_enable(uint8_t led_index, bool enable) {
  if (led_index >= LP5036_LED_COUNT) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t reg = LP5036_REG::LED_CONFIG0;
  uint8_t bit = 0;
  if (led_index < 8) {
    bit = static_cast<uint8_t>(1 << led_index);
  } else {
    reg = LP5036_REG::LED_CONFIG1;
    bit = static_cast<uint8_t>(1 << (led_index - 8));
  }

  uint8_t value = enable ? bit : 0x00;
  return modify_register(reg, bit, value);
}

esp_err_t LP5036::set_led_brightness(uint8_t led_index, uint8_t brightness) {
  if (led_index >= LP5036_LED_COUNT) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t reg =
    static_cast<uint8_t>(LP5036_REG::LED_BRIGHTNESS_BASE + led_index);
  return write_register(reg, brightness);
}

esp_err_t LP5036::set_out_color(uint8_t out_index, uint8_t value) {
  if (out_index >= LP5036_OUT_COUNT) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t reg = static_cast<uint8_t>(LP5036_REG::OUT_COLOR_BASE + out_index);
  return write_register(reg, value);
}

esp_err_t LP5036::set_led_color(uint8_t led_index, uint8_t blue, uint8_t green,
                                uint8_t red) {
  if (led_index >= LP5036_LED_COUNT) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t out_base = static_cast<uint8_t>(led_index * 3);
  if (out_base + 2 >= LP5036_OUT_COUNT) {
    return ESP_ERR_INVALID_ARG;
  }

  esp_err_t ret = set_out_color(out_base, blue);
  if (ret != ESP_OK) {
    return ret;
  }

  ret = set_out_color(out_base + 1, green);
  if (ret != ESP_OK) {
    return ret;
  }

  return set_out_color(out_base + 2, red);
}

esp_err_t LP5036::read_register(uint8_t reg, uint8_t *value) {
  if (value == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }
  if (dev_handle_ == nullptr) {
    return ESP_ERR_INVALID_STATE;
  }

  esp_err_t ret = i2c_master_transmit_receive(dev_handle_, &reg, 1, value, 1,
                                              I2C_TIMEOUT_MS);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read register 0x%02X: %s", reg, esp_err_to_name(ret));
  }
  return ret;
}

esp_err_t LP5036::write_register(uint8_t reg, uint8_t value) {
  if (dev_handle_ == nullptr) {
    return ESP_ERR_INVALID_STATE;
  }

  uint8_t buffer[2] = {reg, value};
  esp_err_t ret = i2c_master_transmit(dev_handle_, buffer, sizeof(buffer),
                                      I2C_TIMEOUT_MS);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write register 0x%02X: %s", reg, esp_err_to_name(ret));
  }
  return ret;
}

esp_err_t LP5036::modify_register(uint8_t reg, uint8_t mask, uint8_t value) {
  uint8_t current = 0;
  esp_err_t ret = read_register(reg, &current);
  if (ret != ESP_OK) {
    return ret;
  }

  uint8_t updated = (current & ~mask) | (value & mask);
  return write_register(reg, updated);
}

} // namespace drivers
