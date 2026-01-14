/**
 * @file lis2dh12.cpp
 * @brief LIS2DH12 3-Axis MEMS Accelerometer Driver Implementation
 */

#include "lis2dh12.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cmath>
#include <cstring>

namespace drivers {

const char *LIS2DH12::TAG = "LIS2DH12";

// CTRL_REG1 bit definitions
#define CTRL_REG1_ODR_SHIFT 4
#define CTRL_REG1_LPEN_BIT (1 << 3)
#define CTRL_REG1_ZEN_BIT (1 << 2)
#define CTRL_REG1_YEN_BIT (1 << 1)
#define CTRL_REG1_XEN_BIT (1 << 0)

// CTRL_REG4 bit definitions
#define CTRL_REG4_BDU_BIT (1 << 7)
#define CTRL_REG4_FS_SHIFT 4
#define CTRL_REG4_HR_BIT (1 << 3)

// STATUS_REG bit definitions
#define STATUS_REG_ZYXDA_BIT (1 << 3)

// INT1_CFG bit definitions
#define INT1_CFG_AOI_BIT (1 << 7)
#define INT1_CFG_6D_BIT (1 << 6)
#define INT1_CFG_ZHIE_BIT (1 << 5)
#define INT1_CFG_ZLIE_BIT (1 << 4)
#define INT1_CFG_YHIE_BIT (1 << 3)
#define INT1_CFG_YLIE_BIT (1 << 2)
#define INT1_CFG_XHIE_BIT (1 << 1)
#define INT1_CFG_XLIE_BIT (1 << 0)

// CTRL_REG3 bit definitions
#define CTRL_REG3_I1_IA1_BIT (1 << 6)

// CTRL_REG2 bit definitions
#define CTRL_REG2_HPM_SHIFT 6
#define CTRL_REG2_HPM_MASK (0x3 << CTRL_REG2_HPM_SHIFT)
#define CTRL_REG2_HPCF_SHIFT 4
#define CTRL_REG2_HPCF_MASK (0x3 << CTRL_REG2_HPCF_SHIFT)
#define CTRL_REG2_HPIS1_BIT (1 << 0)

// CTRL_REG5 bit definitions
#define CTRL_REG5_LIR_INT1_BIT (1 << 3)

// CTRL_REG6 bit definitions
#define CTRL_REG6_I2_ACT_BIT (1 << 3)
#define CTRL_REG6_H_LACTIVE_BIT (1 << 1)  // 0=active-high (default), 1=active-low

// I2C device configuration
#define I2C_TIMEOUT_MS 1000
#define I2C_CLOCK_SPEED_HZ 100000 // 100 kHz

LIS2DH12::LIS2DH12(i2c_master_bus_handle_t i2c_handle, uint8_t i2c_addr)
  : i2c_bus_(i2c_handle), dev_handle_(nullptr), i2c_address_(i2c_addr),
    current_scale_(FullScale::FS_2G), current_mode_(PowerMode::NORMAL) {}

LIS2DH12::~LIS2DH12() {
  deinit();
}

esp_err_t LIS2DH12::init() {
  ESP_LOGI(TAG, "Initializing LIS2DH12 at address 0x%02X", i2c_address_);

  // Create I2C device handle
  i2c_device_config_t dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = i2c_address_,
    .scl_speed_hz = I2C_CLOCK_SPEED_HZ,
    .scl_wait_us = 0,
    .flags = {}
  };

  esp_err_t err = i2c_master_bus_add_device(i2c_bus_, &dev_cfg, &dev_handle_);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add I2C device: %s", esp_err_to_name(err));
    return err;
  }

  // Check WHO_AM_I register
  uint8_t who_am_i = 0;
  err = read_register(LIS2DH12_REG::WHO_AM_I, &who_am_i);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read WHO_AM_I: %s", esp_err_to_name(err));
    return err;
  }

  if (who_am_i != LIS2DH12_WHO_AM_I_VALUE) {
    ESP_LOGE(TAG, "WHO_AM_I mismatch: expected 0x%02X, got 0x%02X",
             LIS2DH12_WHO_AM_I_VALUE, who_am_i);
    return ESP_ERR_NOT_FOUND;
  }

  ESP_LOGI(TAG, "WHO_AM_I: 0x%02X (OK)", who_am_i);

  // Configure CTRL_REG0 - disconnect SDO/SA0 pull-up for power saving
  err = write_register(LIS2DH12_REG::CTRL_REG0, 0x10);
  ESP_ERROR_CHECK(err);

  // Configure CTRL_REG1: ODR=10Hz, normal mode, all axes enabled
  // ODR[3:0]=0010 (10Hz), LPen=0 (normal mode), Zen=Yen=Xen=1
  uint8_t ctrl1 = (static_cast<uint8_t>(DataRate::ODR_10HZ) << CTRL_REG1_ODR_SHIFT) |
                  CTRL_REG1_ZEN_BIT | CTRL_REG1_YEN_BIT | CTRL_REG1_XEN_BIT;
  err = write_register(LIS2DH12_REG::CTRL_REG1, ctrl1);
  ESP_ERROR_CHECK(err);

  // Configure CTRL_REG4: BDU=1 (block data update), FS=±2g, HR=0 (normal mode)
  uint8_t ctrl4 = CTRL_REG4_BDU_BIT |
                  (static_cast<uint8_t>(FullScale::FS_2G) << CTRL_REG4_FS_SHIFT);
  err = write_register(LIS2DH12_REG::CTRL_REG4, ctrl4);
  ESP_ERROR_CHECK(err);

  current_scale_ = FullScale::FS_2G;
  current_mode_ = PowerMode::NORMAL;

  ESP_LOGI(TAG, "LIS2DH12 initialized successfully (ODR=10Hz, ±2g, normal mode)");

  return ESP_OK;
}

esp_err_t LIS2DH12::deinit() {
  if (dev_handle_ != nullptr) {
    // Power down the device
    write_register(LIS2DH12_REG::CTRL_REG1, 0x00);

    // Remove I2C device
    esp_err_t err = i2c_master_bus_rm_device(dev_handle_);
    if (err != ESP_OK) {
      ESP_LOGW(TAG, "Failed to remove I2C device: %s", esp_err_to_name(err));
    }
    dev_handle_ = nullptr;
  }
  return ESP_OK;
}

esp_err_t LIS2DH12::set_full_scale(FullScale scale) {
  uint8_t ctrl4;
  esp_err_t err = read_register(LIS2DH12_REG::CTRL_REG4, &ctrl4);
  ESP_ERROR_CHECK(err);

  // Clear FS bits [5:4] and set new value
  ctrl4 &= ~(0x03 << CTRL_REG4_FS_SHIFT);
  ctrl4 |= (static_cast<uint8_t>(scale) << CTRL_REG4_FS_SHIFT);

  err = write_register(LIS2DH12_REG::CTRL_REG4, ctrl4);
  ESP_ERROR_CHECK(err);

  current_scale_ = scale;

  const char *scale_str[] = {"±2g", "±4g", "±8g", "±16g"};
  ESP_LOGI(TAG, "Full scale set to %s", scale_str[static_cast<uint8_t>(scale)]);

  return ESP_OK;
}

esp_err_t LIS2DH12::set_data_rate(DataRate rate) {
  uint8_t ctrl1;
  esp_err_t err = read_register(LIS2DH12_REG::CTRL_REG1, &ctrl1);
  ESP_ERROR_CHECK(err);

  // Clear ODR bits [7:4] and set new value
  ctrl1 &= ~(0x0F << CTRL_REG1_ODR_SHIFT);
  ctrl1 |= (static_cast<uint8_t>(rate) << CTRL_REG1_ODR_SHIFT);

  err = write_register(LIS2DH12_REG::CTRL_REG1, ctrl1);
  ESP_ERROR_CHECK(err);

  ESP_LOGI(TAG, "Data rate set to ODR=0x%02X", static_cast<uint8_t>(rate));

  return ESP_OK;
}

esp_err_t LIS2DH12::set_power_mode(PowerMode mode) {
  esp_err_t err;
  uint8_t ctrl1, ctrl4;

  err = read_register(LIS2DH12_REG::CTRL_REG1, &ctrl1);
  ESP_ERROR_CHECK(err);
  err = read_register(LIS2DH12_REG::CTRL_REG4, &ctrl4);
  ESP_ERROR_CHECK(err);

  switch (mode) {
  case PowerMode::LOW_POWER:
    ctrl1 |= CTRL_REG1_LPEN_BIT;  // Set LPen bit
    ctrl4 &= ~CTRL_REG4_HR_BIT;   // Clear HR bit
    break;

  case PowerMode::NORMAL:
    ctrl1 &= ~CTRL_REG1_LPEN_BIT; // Clear LPen bit
    ctrl4 &= ~CTRL_REG4_HR_BIT;   // Clear HR bit
    break;

  case PowerMode::HIGH_RESOLUTION:
    ctrl1 &= ~CTRL_REG1_LPEN_BIT; // Clear LPen bit
    ctrl4 |= CTRL_REG4_HR_BIT;    // Set HR bit
    break;
  }

  err = write_register(LIS2DH12_REG::CTRL_REG1, ctrl1);
  ESP_ERROR_CHECK(err);
  err = write_register(LIS2DH12_REG::CTRL_REG4, ctrl4);
  ESP_ERROR_CHECK(err);

  current_mode_ = mode;

  const char *mode_str[] = {"low-power", "normal", "high-resolution"};
  ESP_LOGI(TAG, "Power mode set to %s", mode_str[static_cast<uint8_t>(mode)]);

  return ESP_OK;
}

esp_err_t LIS2DH12::read_accel(AccelData *data) {
  if (data == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }

  // Check if data is ready
  uint8_t status;
  esp_err_t err = read_register(LIS2DH12_REG::STATUS_REG, &status);
  ESP_ERROR_CHECK(err);

  if (!(status & STATUS_REG_ZYXDA_BIT)) {
    ESP_LOGW(TAG, "Acceleration data not ready");
    return ESP_ERR_NOT_FINISHED;
  }

  // Read all 6 bytes (X, Y, Z low and high bytes) in one transaction
  uint8_t raw_data[6];
  err = read_registers(LIS2DH12_REG::OUT_X_L | 0x80, raw_data, 6);
  ESP_ERROR_CHECK(err);

  // Combine low and high bytes (little-endian, left-justified)
  int16_t x_raw = static_cast<int16_t>((raw_data[1] << 8) | raw_data[0]);
  int16_t y_raw = static_cast<int16_t>((raw_data[3] << 8) | raw_data[2]);
  int16_t z_raw = static_cast<int16_t>((raw_data[5] << 8) | raw_data[4]);

  // Convert to milligravities
  data->x_mg = raw_to_mg(x_raw);
  data->y_mg = raw_to_mg(y_raw);
  data->z_mg = raw_to_mg(z_raw);

  return ESP_OK;
}

esp_err_t LIS2DH12::configure_motion_detect(const MotionConfig &config) {
  esp_err_t err;

  ESP_LOGI(TAG, "Configuring motion detection: threshold=%dmg, duration=%dms",
           config.threshold_mg, config.duration_ms);

  // Configure INT1_THS register (7-bit threshold)
  uint8_t threshold = mg_to_threshold(config.threshold_mg);
  err = write_register(LIS2DH12_REG::INT1_THS, threshold);
  ESP_ERROR_CHECK(err);

  // Configure INT1_DURATION register
  // Duration in samples = duration_ms * (ODR / 1000)
  // For 10Hz ODR: duration_samples = duration_ms / 100
  uint8_t duration = config.duration_ms / 100; // Assuming 10Hz ODR
  if (duration > 127) {
    duration = 127; // Max 7-bit value
  }
  err = write_register(LIS2DH12_REG::INT1_DURATION, duration);
  ESP_ERROR_CHECK(err);

  // Configure INT1_CFG register (OR combination, enable high events only)
  // Low events stay true when axes sit near 0 mg, which keeps IA asserted.
  uint8_t int1_cfg = 0;
  if (config.enable_x) {
    int1_cfg |= INT1_CFG_XHIE_BIT;
  }
  if (config.enable_y) {
    int1_cfg |= INT1_CFG_YHIE_BIT;
  }
  if (config.enable_z) {
    int1_cfg |= INT1_CFG_ZHIE_BIT;
  }
  err = write_register(LIS2DH12_REG::INT1_CFG, int1_cfg);
  ESP_ERROR_CHECK(err);

  // Enable high-pass filter for INT1 to remove static gravity offset.
  // Use default HPF mode/cutoff and route HPF output to INT1.
  err = modify_register(LIS2DH12_REG::CTRL_REG2,
                        CTRL_REG2_HPIS1_BIT | CTRL_REG2_HPM_MASK | CTRL_REG2_HPCF_MASK,
                        CTRL_REG2_HPIS1_BIT);
  ESP_ERROR_CHECK(err);

  // Reset HPF reference to current acceleration baseline.
  uint8_t ref = 0;
  err = read_register(LIS2DH12_REG::REFERENCE, &ref);
  ESP_ERROR_CHECK(err);

  // Enable interrupt 1 on INT1 pin (CTRL_REG3)
  err = write_register(LIS2DH12_REG::CTRL_REG3, CTRL_REG3_I1_IA1_BIT);
  ESP_ERROR_CHECK(err);

  // Configure INT1 as active-high (CTRL_REG6, H_LACTIVE=0)
  // This makes INT1: LOW when idle, HIGH when interrupt triggered
  err = modify_register(LIS2DH12_REG::CTRL_REG6, CTRL_REG6_H_LACTIVE_BIT, 0x00);
  ESP_ERROR_CHECK(err);

  // Disable latch mode for non-latched interrupt (CTRL_REG5)
  // Non-latched: INT1 pin auto-clears when motion stops (better for edge-triggered ISR)
  // Latched mode was causing continuous triggers due to sensor noise
  err = modify_register(LIS2DH12_REG::CTRL_REG5, CTRL_REG5_LIR_INT1_BIT, 0x00);
  ESP_ERROR_CHECK(err);

  // Debug: Read back configuration to verify
  uint8_t read_int1_cfg, read_ctrl_reg2, read_ctrl_reg3, read_ctrl_reg5,
    read_ctrl_reg6;
  read_register(LIS2DH12_REG::INT1_CFG, &read_int1_cfg);
  read_register(LIS2DH12_REG::CTRL_REG2, &read_ctrl_reg2);
  read_register(LIS2DH12_REG::CTRL_REG3, &read_ctrl_reg3);
  read_register(LIS2DH12_REG::CTRL_REG5, &read_ctrl_reg5);
  read_register(LIS2DH12_REG::CTRL_REG6, &read_ctrl_reg6);
  
  ESP_LOGI(TAG, "Motion detection configured: THS=0x%02X, DUR=%d", threshold, duration);
  ESP_LOGI(TAG,
           "Verify: INT1_CFG=0x%02X, CTRL_REG2=0x%02X, CTRL_REG3=0x%02X, "
           "CTRL_REG5=0x%02X, CTRL_REG6=0x%02X",
           read_int1_cfg, read_ctrl_reg2, read_ctrl_reg3, read_ctrl_reg5,
           read_ctrl_reg6);
  ESP_LOGI(TAG, "INT1 configured as: %s, %s", 
           (read_ctrl_reg6 & CTRL_REG6_H_LACTIVE_BIT) ? "active-low" : "active-high",
           (read_ctrl_reg5 & CTRL_REG5_LIR_INT1_BIT) ? "latched" : "non-latched");

  return ESP_OK;
}

esp_err_t LIS2DH12::enable_sleep_to_wake(const SleepToWakeConfig &config) {
  esp_err_t err;

  ESP_LOGI(TAG,
           "Enabling sleep-to-wake: activity_threshold=%dmg, "
           "inactivity_duration=%ds",
           config.activity_threshold_mg, config.inactivity_duration_s);

  // Configure ACT_THS register (activity threshold in mg)
  uint8_t act_ths = mg_to_threshold(config.activity_threshold_mg);
  err = write_register(LIS2DH12_REG::ACT_THS, act_ths);
  ESP_ERROR_CHECK(err);

  // Configure ACT_DUR register (inactivity duration in seconds)
  // Duration in samples = inactivity_duration_s * ODR
  // For 10Hz ODR: duration_samples = inactivity_duration_s * 10
  uint16_t duration_samples = config.inactivity_duration_s * 10;
  if (duration_samples > 255) {
    duration_samples = 255; // Max 8-bit value
  }
  err = write_register(LIS2DH12_REG::ACT_DUR, static_cast<uint8_t>(duration_samples));
  ESP_ERROR_CHECK(err);

  // Enable activity interrupt on INT2 pin (CTRL_REG6)
  err = modify_register(LIS2DH12_REG::CTRL_REG6, CTRL_REG6_I2_ACT_BIT,
                        CTRL_REG6_I2_ACT_BIT);
  ESP_ERROR_CHECK(err);

  ESP_LOGI(TAG, "Sleep-to-wake enabled: ACT_THS=0x%02X, ACT_DUR=%d", act_ths,
           static_cast<uint8_t>(duration_samples));

  return ESP_OK;
}

esp_err_t LIS2DH12::get_int1_source(uint8_t *int_src) {
  if (int_src == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }

  // Reading INT1_SRC clears the interrupt
  esp_err_t err = read_register(LIS2DH12_REG::INT1_SRC, int_src);
  ESP_ERROR_CHECK(err);

  return ESP_OK;
}

esp_err_t LIS2DH12::get_int2_source(uint8_t *int_src) {
  if (int_src == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }

  // Reading INT2_SRC clears the interrupt
  esp_err_t err = read_register(LIS2DH12_REG::INT2_SRC, int_src);
  ESP_ERROR_CHECK(err);

  return ESP_OK;
}

esp_err_t LIS2DH12::read_temperature(int16_t *temp_c) {
  if (temp_c == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }

  // Enable temperature sensor (TEMP_CFG_REG)
  esp_err_t err = write_register(LIS2DH12_REG::TEMP_CFG_REG, 0xC0);
  ESP_ERROR_CHECK(err);

  // Wait for temperature data to be ready (at least 1/ODR)
  vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay

  // Read temperature data (16-bit)
  uint8_t temp_data[2];
  err = read_registers(LIS2DH12_REG::OUT_TEMP_L | 0x80, temp_data, 2);
  ESP_ERROR_CHECK(err);

  // Combine low and high bytes
  int16_t temp_raw = static_cast<int16_t>((temp_data[1] << 8) | temp_data[0]);

  // Convert to Celsius (approximate: LSB = 1°C, offset = 25°C)
  *temp_c = 25 + (temp_raw / 256);

  return ESP_OK;
}

esp_err_t LIS2DH12::is_data_ready(bool *available) {
  if (available == nullptr) {
    return ESP_ERR_INVALID_ARG;
  }

  uint8_t status;
  esp_err_t err = read_register(LIS2DH12_REG::STATUS_REG, &status);
  ESP_ERROR_CHECK(err);

  *available = (status & STATUS_REG_ZYXDA_BIT) != 0;

  return ESP_OK;
}

// Private methods

esp_err_t LIS2DH12::write_register(uint8_t reg, uint8_t data) {
  uint8_t write_buf[2] = {reg, data};
  esp_err_t err =
    i2c_master_transmit(dev_handle_, write_buf, sizeof(write_buf), I2C_TIMEOUT_MS);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write register 0x%02X: %s", reg, esp_err_to_name(err));
  }
  return err;
}

esp_err_t LIS2DH12::read_register(uint8_t reg, uint8_t *data) {
  esp_err_t err =
    i2c_master_transmit_receive(dev_handle_, &reg, 1, data, 1, I2C_TIMEOUT_MS);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read register 0x%02X: %s", reg, esp_err_to_name(err));
  }
  return err;
}

esp_err_t LIS2DH12::read_registers(uint8_t reg, uint8_t *data, size_t len) {
  // For multi-byte read, set MSb of register address to enable auto-increment
  uint8_t reg_addr = reg | 0x80;
  esp_err_t err =
    i2c_master_transmit_receive(dev_handle_, &reg_addr, 1, data, len, I2C_TIMEOUT_MS);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read %d bytes from register 0x%02X: %s", len, reg,
             esp_err_to_name(err));
  }
  return err;
}

esp_err_t LIS2DH12::modify_register(uint8_t reg, uint8_t mask, uint8_t value) {
  uint8_t reg_val;
  esp_err_t err = read_register(reg, &reg_val);
  ESP_ERROR_CHECK(err);

  reg_val = (reg_val & ~mask) | (value & mask);

  err = write_register(reg, reg_val);
  ESP_ERROR_CHECK(err);

  return ESP_OK;
}

int16_t LIS2DH12::raw_to_mg(int16_t raw_value) {
  // Data is left-justified, convert based on power mode
  int16_t shifted_value;

  switch (current_mode_) {
  case PowerMode::LOW_POWER:
    // 8-bit resolution, shift right by 8 bits
    shifted_value = raw_value >> 8;
    break;

  case PowerMode::NORMAL:
    // 10-bit resolution, shift right by 6 bits
    shifted_value = raw_value >> 6;
    break;

  case PowerMode::HIGH_RESOLUTION:
    // 12-bit resolution, shift right by 4 bits
    shifted_value = raw_value >> 4;
    break;

  default:
    shifted_value = raw_value >> 6; // Default to normal mode
    break;
  }

  // Convert to milligravities based on full scale
  int16_t mg_value;
  switch (current_scale_) {
  case FullScale::FS_2G:
    mg_value = (current_mode_ == PowerMode::HIGH_RESOLUTION)
                 ? (shifted_value * 1)  // 12-bit: 1 LSb = 1 mg
                 : (shifted_value * 4); // 10-bit: 1 LSb = 4 mg
    break;

  case FullScale::FS_4G:
    mg_value = (current_mode_ == PowerMode::HIGH_RESOLUTION)
                 ? (shifted_value * 2)  // 12-bit: 1 LSb = 2 mg
                 : (shifted_value * 8); // 10-bit: 1 LSb = 8 mg
    break;

  case FullScale::FS_8G:
    mg_value = (current_mode_ == PowerMode::HIGH_RESOLUTION)
                 ? (shifted_value * 4)  // 12-bit: 1 LSb = 4 mg
                 : (shifted_value * 16); // 10-bit: 1 LSb = 16 mg
    break;

  case FullScale::FS_16G:
    mg_value = (current_mode_ == PowerMode::HIGH_RESOLUTION)
                 ? (shifted_value * 12)  // 12-bit: 1 LSb = 12 mg
                 : (shifted_value * 48); // 10-bit: 1 LSb = 48 mg
    break;

  default:
    mg_value = shifted_value * 4; // Default to ±2g, 10-bit
    break;
  }

  return mg_value;
}

uint8_t LIS2DH12::mg_to_threshold(uint16_t mg_value) {
  // Threshold register is 7-bit, calculate based on full scale
  uint16_t lsb_mg;

  switch (current_scale_) {
  case FullScale::FS_2G:
    lsb_mg = 16; // 1 LSb = 16 mg @ ±2g
    break;
  case FullScale::FS_4G:
    lsb_mg = 32; // 1 LSb = 32 mg @ ±4g
    break;
  case FullScale::FS_8G:
    lsb_mg = 62; // 1 LSb = 62 mg @ ±8g
    break;
  case FullScale::FS_16G:
    lsb_mg = 186; // 1 LSb = 186 mg @ ±16g
    break;
  default:
    lsb_mg = 16;
    break;
  }

  uint8_t threshold = mg_value / lsb_mg;
  if (threshold > 127) {
    threshold = 127; // Max 7-bit value
  }

  return threshold;
}

} // namespace drivers
