/**
 * @file lp5036.h
 * @brief LP5036 36-channel RGB LED driver
 *
 * Driver for Texas Instruments LP5036 36-channel constant-current LED driver.
 * Datasheet: lp5036.pdf (Texas Instruments)
 * I2C Address: 0x30-0x33 (ADDR1/ADDR0), broadcast 0x1C
 */

#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"
#include <cstdint>

namespace drivers {

namespace LP5036_REG {
constexpr uint8_t DEVICE_CONFIG0 = 0x00;
constexpr uint8_t DEVICE_CONFIG1 = 0x01;
constexpr uint8_t LED_CONFIG0 = 0x02;
constexpr uint8_t LED_CONFIG1 = 0x03;
constexpr uint8_t BANK_BRIGHTNESS = 0x04;
constexpr uint8_t BANK_A_COLOR = 0x05;
constexpr uint8_t BANK_B_COLOR = 0x06;
constexpr uint8_t BANK_C_COLOR = 0x07;
constexpr uint8_t LED_BRIGHTNESS_BASE = 0x08;
constexpr uint8_t OUT_COLOR_BASE = 0x14;
constexpr uint8_t RESET = 0x38;
} // namespace LP5036_REG

namespace LP5036_I2C {
constexpr uint8_t ADDR_BASE = 0x30;      // ADDR1=GND, ADDR0=GND
constexpr uint8_t ADDR_33 = 0x33;        // ADDR1=VCC, ADDR0=VCC
constexpr uint8_t ADDR_BROADCAST = 0x1C; // Broadcast address
} // namespace LP5036_I2C

constexpr uint8_t LP5036_LED_COUNT = 12;
constexpr uint8_t LP5036_OUT_COUNT = 36;

enum class LP5036Bank : uint8_t {
  BANK_A = 0,
  BANK_B = 1,
  BANK_C = 2
};

class LP5036 {
public:
  /**
   * @brief Construct a new LP5036 driver instance
   * @param i2c_bus I2C master bus handle
   * @param i2c_addr I2C device address (default: 0x30)
   */
  LP5036(i2c_master_bus_handle_t i2c_bus,
         uint8_t i2c_addr = LP5036_I2C::ADDR_BASE);

  /**
   * @brief Destroy the LP5036 driver and cleanup resources
   */
  ~LP5036();

  /**
   * @brief Initialize the LP5036 device
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t init();

  /**
   * @brief Deinitialize the device and release resources
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t deinit();

  /**
   * @brief Enable or disable the chip
   * @param enable True to enable, false to disable
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t enable(bool enable);

  /**
   * @brief Reset all registers to default values
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t reset();

  /**
   * @brief Enable or disable global LED output
   * @param off True to turn off all LEDs, false for normal operation
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_global_off(bool off);

  /**
   * @brief Enable or disable log scale brightness curve
   * @param enable True for logarithmic curve, false for linear
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_log_scale(bool enable);

  /**
   * @brief Enable or disable auto power save
   * @param enable True to enable, false to disable
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_power_save(bool enable);

  /**
   * @brief Enable or disable auto-increment for register access
   * @param enable True to enable, false to disable
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_auto_increment(bool enable);

  /**
   * @brief Enable or disable PWM dithering
   * @param enable True to enable, false to disable
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_pwm_dither(bool enable);

  /**
   * @brief Configure max output current option
   * @param high_current True for 35 mA, false for 25.5 mA
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_max_current_option(bool high_current);

  /**
   * @brief Set bank brightness (shared intensity)
   * @param brightness 0x00-0xFF
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_bank_brightness(uint8_t brightness);

  /**
   * @brief Set bank color mix for A/B/C banks
   * @param bank Bank selector
   * @param value 0x00-0xFF color mix
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_bank_color(LP5036Bank bank, uint8_t value);

  /**
   * @brief Enable or disable bank control for an LED module
   * @param led_index LED module index (0-11)
   * @param enable True to use bank control, false for independent control
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_led_bank_enable(uint8_t led_index, bool enable);

  /**
   * @brief Set LED module brightness
   * @param led_index LED module index (0-11)
   * @param brightness 0x00-0xFF
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_led_brightness(uint8_t led_index, uint8_t brightness);

  /**
   * @brief Set individual output color mix
   * @param out_index Output index (0-35)
   * @param value 0x00-0xFF
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_out_color(uint8_t out_index, uint8_t value);

  /**
   * @brief Set RGB color for an LED module
   * @param led_index LED module index (0-11)
   * @param red Red channel mix
   * @param green Green channel mix
   * @param blue Blue channel mix
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_led_color(uint8_t led_index, uint8_t red, uint8_t green,
                          uint8_t blue);

  /**
   * @brief Read a register (debug/validation)
   * @param reg Register address
   * @param value Pointer to store register value
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t read_register(uint8_t reg, uint8_t *value);

private:
  i2c_master_bus_handle_t i2c_bus_;
  i2c_master_dev_handle_t dev_handle_;
  uint8_t device_address_;

  esp_err_t write_register(uint8_t reg, uint8_t value);
  esp_err_t modify_register(uint8_t reg, uint8_t mask, uint8_t value);
};

} // namespace drivers
