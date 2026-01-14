/**
 * @file lis2dh12.h
 * @brief LIS2DH12 3-Axis MEMS Accelerometer Driver
 *
 * Driver for STMicroelectronics LIS2DH12 ultra-low-power high-performance
 * 3-axis accelerometer with motion and orientation detection
 *
 * Features:
 * - 3-axis linear acceleration sensing
 * - ±2g/±4g/±8g/±16g selectable full scales
 * - I2C/SPI digital output interface
 * - 2 independent programmable interrupt generators for motion detection
 * - 6D/4D orientation detection
 * - Free-fall detection
 * - Sleep-to-wake and return-to-sleep activation
 * - Motion-activated GPS tagging feature
 *
 * Datasheet: LIS2DH12.md (STMicroelectronics)
 * I2C Address: 0x18 (SA0=0) or 0x19 (SA0=1)
 */

#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"
#include <cstdint>

namespace drivers {

/**
 * @brief LIS2DH12 Register Addresses
 */
namespace LIS2DH12_REG {
// Status registers
constexpr uint8_t STATUS_REG_AUX = 0x07;

// Temperature output registers
constexpr uint8_t OUT_TEMP_L = 0x0C;
constexpr uint8_t OUT_TEMP_H = 0x0D;

// Device identification
constexpr uint8_t WHO_AM_I = 0x0F; // Expected value: 0x33

// Control registers
constexpr uint8_t CTRL_REG0 = 0x1E;
constexpr uint8_t TEMP_CFG_REG = 0x1F;
constexpr uint8_t CTRL_REG1 = 0x20;
constexpr uint8_t CTRL_REG2 = 0x21;
constexpr uint8_t CTRL_REG3 = 0x22;
constexpr uint8_t CTRL_REG4 = 0x23;
constexpr uint8_t CTRL_REG5 = 0x24;
constexpr uint8_t CTRL_REG6 = 0x25;

// Reference/Data capture
constexpr uint8_t REFERENCE = 0x26;
constexpr uint8_t STATUS_REG = 0x27;

// Output registers (X, Y, Z acceleration data)
constexpr uint8_t OUT_X_L = 0x28;
constexpr uint8_t OUT_X_H = 0x29;
constexpr uint8_t OUT_Y_L = 0x2A;
constexpr uint8_t OUT_Y_H = 0x2B;
constexpr uint8_t OUT_Z_L = 0x2C;
constexpr uint8_t OUT_Z_H = 0x2D;

// FIFO control
constexpr uint8_t FIFO_CTRL_REG = 0x2E;
constexpr uint8_t FIFO_SRC_REG = 0x2F;

// Interrupt 1 configuration
constexpr uint8_t INT1_CFG = 0x30;
constexpr uint8_t INT1_SRC = 0x31;
constexpr uint8_t INT1_THS = 0x32;
constexpr uint8_t INT1_DURATION = 0x33;

// Interrupt 2 configuration
constexpr uint8_t INT2_CFG = 0x34;
constexpr uint8_t INT2_SRC = 0x35;
constexpr uint8_t INT2_THS = 0x36;
constexpr uint8_t INT2_DURATION = 0x37;

// Click detection
constexpr uint8_t CLICK_CFG = 0x38;
constexpr uint8_t CLICK_SRC = 0x39;
constexpr uint8_t CLICK_THS = 0x3A;
constexpr uint8_t TIME_LIMIT = 0x3B;
constexpr uint8_t TIME_LATENCY = 0x3C;
constexpr uint8_t TIME_WINDOW = 0x3D;

// Sleep-to-wake, return-to-sleep activation threshold and duration
constexpr uint8_t ACT_THS = 0x3E;
constexpr uint8_t ACT_DUR = 0x3F;
} // namespace LIS2DH12_REG

/**
 * @brief LIS2DH12 I2C Addresses (7-bit)
 */
namespace LIS2DH12_I2C {
constexpr uint8_t ADDR_SA0_LOW = 0x18;  // SA0 connected to GND
constexpr uint8_t ADDR_SA0_HIGH = 0x19; // SA0 connected to VDD
} // namespace LIS2DH12_I2C

/**
 * @brief WHO_AM_I expected value
 */
constexpr uint8_t LIS2DH12_WHO_AM_I_VALUE = 0x33;

/**
 * @brief LIS2DH12 Full Scale Selection
 */
enum class FullScale : uint8_t {
  FS_2G = 0x00,  // ±2g (1 LSb = 16 mg @ 10-bit, 4 mg @ 12-bit)
  FS_4G = 0x01,  // ±4g (1 LSb = 32 mg @ 10-bit, 8 mg @ 12-bit)
  FS_8G = 0x02,  // ±8g (1 LSb = 62 mg @ 10-bit, 16 mg @ 12-bit)
  FS_16G = 0x03  // ±16g (1 LSb = 186 mg @ 10-bit, 48 mg @ 12-bit)
};

/**
 * @brief LIS2DH12 Output Data Rate
 */
enum class DataRate : uint8_t {
  POWER_DOWN = 0x00,
  ODR_1HZ = 0x01,
  ODR_10HZ = 0x02,
  ODR_25HZ = 0x03,
  ODR_50HZ = 0x04,
  ODR_100HZ = 0x05,
  ODR_200HZ = 0x06,
  ODR_400HZ = 0x07,
  ODR_1620HZ_LP = 0x08, // Low-power mode only
  ODR_1344HZ_NP_5376HZ_LP = 0x09 // 1.344kHz normal/high-res, 5.376kHz low-power
};

/**
 * @brief LIS2DH12 Power Mode
 */
enum class PowerMode : uint8_t {
  LOW_POWER = 0,       // 8-bit resolution
  NORMAL = 1,          // 10-bit resolution
  HIGH_RESOLUTION = 2  // 12-bit resolution
};

/**
 * @brief LIS2DH12 Interrupt Mode
 */
enum class InterruptMode : uint8_t {
  OR_COMBINATION = 0x00,   // OR combination of interrupt events
  MOVEMENT_6D = 0x01,      // 6-direction movement recognition
  AND_COMBINATION = 0x02,  // AND combination of interrupt events
  POSITION_6D = 0x03       // 6-direction position recognition
};

/**
 * @brief LIS2DH12 3-axis acceleration data
 */
struct AccelData {
  int16_t x_mg; // X-axis acceleration in milligravities (mg)
  int16_t y_mg; // Y-axis acceleration in milligravities (mg)
  int16_t z_mg; // Z-axis acceleration in milligravities (mg)
};

/**
 * @brief LIS2DH12 motion detection configuration
 */
struct MotionConfig {
  uint8_t threshold_mg;  // Motion threshold in mg (0-2032mg for ±2g scale)
  uint8_t duration_ms;   // Motion duration in milliseconds
  bool enable_x;         // Enable X-axis motion detection
  bool enable_y;         // Enable Y-axis motion detection
  bool enable_z;         // Enable Z-axis motion detection
};

/**
 * @brief LIS2DH12 sleep-to-wake configuration
 */
struct SleepToWakeConfig {
  uint8_t activity_threshold_mg; // Activity threshold in mg
  uint8_t inactivity_duration_s; // Inactivity duration in seconds before sleep
};

/**
 * @brief LIS2DH12 Accelerometer Driver Class
 */
class LIS2DH12 {
public:
  /**
   * @brief Construct a new LIS2DH12 driver instance
   * @param i2c_handle I2C master bus handle
   * @param i2c_addr I2C device address (0x18 or 0x19)
   */
  LIS2DH12(i2c_master_bus_handle_t i2c_handle,
           uint8_t i2c_addr = LIS2DH12_I2C::ADDR_SA0_LOW);

  /**
   * @brief Destroy the LIS2DH12 driver and cleanup resources
   */
  ~LIS2DH12();

  /**
   * @brief Initialize the LIS2DH12 accelerometer
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t init();

  /**
   * @brief Deinitialize the accelerometer and release resources
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t deinit();

  /**
   * @brief Configure full scale range
   * @param scale Full scale selection (±2g, ±4g, ±8g, ±16g)
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_full_scale(FullScale scale);

  /**
   * @brief Configure output data rate
   * @param rate Output data rate selection
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_data_rate(DataRate rate);

  /**
   * @brief Configure power mode
   * @param mode Power mode (low-power, normal, high-resolution)
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t set_power_mode(PowerMode mode);

  /**
   * @brief Read 3-axis acceleration data
   * @param data Pointer to AccelData structure to store results
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t read_accel(AccelData *data);

  /**
   * @brief Configure motion detection on INT1 pin
   * @param config Motion detection configuration
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t configure_motion_detect(const MotionConfig &config);

  /**
   * @brief Enable sleep-to-wake functionality
   * @param config Sleep-to-wake configuration
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t enable_sleep_to_wake(const SleepToWakeConfig &config);

  /**
   * @brief Get interrupt 1 source (which axis triggered)
   * @param int_src Pointer to store INT1_SRC register value
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t get_int1_source(uint8_t *int_src);

  /**
   * @brief Get interrupt 2 source
   * @param int_src Pointer to store INT2_SRC register value
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t get_int2_source(uint8_t *int_src);

  /**
   * @brief Read temperature sensor data
   * @param temp_c Pointer to store temperature in Celsius
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t read_temperature(int16_t *temp_c);

  /**
   * @brief Check if new acceleration data is available
   * @param available Pointer to store availability status
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t is_data_ready(bool *available);

private:
  i2c_master_bus_handle_t i2c_bus_;
  i2c_master_dev_handle_t dev_handle_;
  uint8_t i2c_address_;
  FullScale current_scale_;
  PowerMode current_mode_;

  static const char *TAG;

  /**
   * @brief Write a single byte to a register
   * @param reg Register address
   * @param data Data byte to write
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t write_register(uint8_t reg, uint8_t data);

  /**
   * @brief Read a single byte from a register
   * @param reg Register address
   * @param data Pointer to store read data
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t read_register(uint8_t reg, uint8_t *data);

  /**
   * @brief Read multiple bytes from consecutive registers
   * @param reg Starting register address
   * @param data Pointer to buffer for read data
   * @param len Number of bytes to read
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t read_registers(uint8_t reg, uint8_t *data, size_t len);

  /**
   * @brief Modify specific bits in a register (read-modify-write)
   * @param reg Register address
   * @param mask Bit mask (1 = modify, 0 = keep)
   * @param value New value for masked bits
   * @return ESP_OK on success, error code otherwise
   */
  esp_err_t modify_register(uint8_t reg, uint8_t mask, uint8_t value);

  /**
   * @brief Convert raw acceleration value to milligravities
   * @param raw_value Raw 16-bit acceleration value (left-justified)
   * @return Acceleration in milligravities (mg)
   */
  int16_t raw_to_mg(int16_t raw_value);

  /**
   * @brief Convert milligravities to threshold register value
   * @param mg_value Threshold in milligravities
   * @return Threshold register value (7-bit)
   */
  uint8_t mg_to_threshold(uint16_t mg_value);
};

} // namespace drivers
