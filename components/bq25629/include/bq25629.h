/**
 * @file bq25629.h
 * @brief BQ25629 Battery Charger Driver
 *
 * Driver for TI BQ25629 battery charge management IC
 * Features:
 * - I2C-controlled 1-cell 2A battery charger
 * - NVDC power path management
 * - Input voltage/current regulation
 * - OTG boost mode support
 * - Integrated 12-bit ADC for monitoring
 * - USB BC1.2 detection
 *
 * Datasheet: bq25629.pdf (Texas Instruments)
 * I2C Address: 0x6A
 */

#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"
#include <cstdint>

namespace drivers {

/**
 * @brief BQ25629 Register Addresses
 */
namespace BQ25629_REG {
  // Charging configuration registers
  constexpr uint8_t CHARGE_CURRENT_LIMIT = 0x02;  // 16-bit
  constexpr uint8_t CHARGE_VOLTAGE_LIMIT = 0x04;  // 16-bit
  constexpr uint8_t INPUT_CURRENT_LIMIT = 0x06;   // 16-bit
  constexpr uint8_t INPUT_VOLTAGE_LIMIT = 0x08;   // 16-bit
  constexpr uint8_t VOTG_REGULATION = 0x0C;       // 16-bit
  constexpr uint8_t MIN_SYSTEM_VOLTAGE = 0x0E;    // 16-bit
  constexpr uint8_t PRECHARGE_CONTROL = 0x10;     // 16-bit
  constexpr uint8_t TERMINATION_CONTROL = 0x12;   // 16-bit

  // Control registers
  constexpr uint8_t CHARGE_CONTROL = 0x14;
  constexpr uint8_t CHARGE_TIMER_CONTROL = 0x15;
  constexpr uint8_t CHARGER_CONTROL_0 = 0x16;
  constexpr uint8_t CHARGER_CONTROL_1 = 0x17;
  constexpr uint8_t CHARGER_CONTROL_2 = 0x18;
  constexpr uint8_t CHARGER_CONTROL_3 = 0x19;

  // NTC control registers
  constexpr uint8_t NTC_CONTROL_0 = 0x1A;
  constexpr uint8_t NTC_CONTROL_1 = 0x1B;
  constexpr uint8_t NTC_CONTROL_2 = 0x1C;

  // Status and fault registers
  constexpr uint8_t CHARGER_STATUS_0 = 0x1D;
  constexpr uint8_t CHARGER_STATUS_1 = 0x1E;
  constexpr uint8_t FAULT_STATUS_0 = 0x1F;
  constexpr uint8_t CHARGER_FLAG_0 = 0x20;
  constexpr uint8_t CHARGER_FLAG_1 = 0x21;
  constexpr uint8_t FAULT_FLAG_0 = 0x22;

  // Mask registers
  constexpr uint8_t CHARGER_MASK_0 = 0x23;
  constexpr uint8_t CHARGER_MASK_1 = 0x24;
  constexpr uint8_t FAULT_MASK_0 = 0x25;

  // ADC registers
  constexpr uint8_t ADC_CONTROL = 0x26;
  constexpr uint8_t ADC_FUNCTION_DISABLE = 0x27;
  constexpr uint8_t IBUS_ADC = 0x28;  // 16-bit
  constexpr uint8_t IBAT_ADC = 0x2A;  // 16-bit
  constexpr uint8_t VBUS_ADC = 0x2C;  // 16-bit
  constexpr uint8_t VPMID_ADC = 0x2E; // 16-bit
  constexpr uint8_t VBAT_ADC = 0x30;  // 16-bit
  constexpr uint8_t VSYS_ADC = 0x32;  // 16-bit
  constexpr uint8_t TS_ADC = 0x34;    // 16-bit
  constexpr uint8_t TDIE_ADC = 0x36;  // 16-bit

  // Part information
  constexpr uint8_t PART_INFORMATION = 0x38;
} // namespace BQ25629_REG

/**
 * @brief BQ25629 Charge Status
 */
enum class ChargeStatus : uint8_t {
  NOT_CHARGING = 0x00,
  TRICKLE_PRECHARGE_FASTCHARGE = 0x01,
  TAPER_CHARGE = 0x02,
  TOPOFF_TIMER_ACTIVE = 0x03
};

/**
 * @brief BQ25629 VBUS Status
 */
enum class VBusStatus : uint8_t {
  NO_ADAPTER = 0x00,
  USB_SDP = 0x01,      // 500mA
  USB_CDP = 0x02,      // 1.5A
  USB_DCP = 0x03,      // 1.5A
  UNKNOWN_ADAPTER = 0x04,
  NON_STANDARD = 0x05,  // 1A/2.1A/2.4A
  OTG_MODE = 0x07
};

/**
 * @brief BQ25629 Configuration Structure
 */
struct BQ25629_Config {
  uint16_t charge_voltage_mv;      // Battery regulation voltage (3500-4800mV)
  uint16_t charge_current_ma;      // Charge current limit (40-2000mA)
  uint16_t input_current_limit_ma; // Input current limit (100-3200mA)
  uint16_t input_voltage_limit_mv; // Input voltage limit (3800-16800mV)
  uint16_t min_system_voltage_mv;  // Minimum system voltage (2560-3840mV)
  uint16_t precharge_current_ma;   // Pre-charge current (10-310mA)
  uint16_t term_current_ma;        // Termination current (5-310mA)
  bool enable_charging;            // Enable battery charging
  bool enable_otg;                 // Enable OTG boost mode
  bool enable_adc;                 // Enable ADC conversion
};

/**
 * @brief BQ25629 ADC Measurements
 */
struct BQ25629_ADC_Data {
  int16_t ibus_ma;   // Input current (negative = discharging)
  int16_t ibat_ma;   // Battery current (positive = charging, negative = discharging)
  uint16_t vbus_mv;  // Input voltage
  uint16_t vpmid_mv; // PMID voltage
  uint16_t vbat_mv;  // Battery voltage
  uint16_t vsys_mv;  // System voltage
  float ts_percent;  // TS pin voltage percentage
  int16_t tdie_c;    // Die temperature (°C)
};

/**
 * @brief BQ25629 Battery Charger Driver Class
 */
class BQ25629 {
public:
  /**
   * @brief Constructor
   * @param i2c_bus I2C bus handle
   * @param device_address I2C device address (default: 0x6A)
   */
  BQ25629(i2c_master_bus_handle_t i2c_bus, uint8_t device_address = 0x6A);

  /**
   * @brief Destructor
   */
  ~BQ25629();

  /**
   * @brief Initialize the BQ25629 device
   * @param config Configuration structure
   * @return ESP_OK on success
   */
  esp_err_t init(const BQ25629_Config &config);

  /**
   * @brief Deinitialize the device
   * @return ESP_OK on success
   */
  esp_err_t deinit();

  /**
   * @brief Enable battery charging
   * @param enable True to enable, false to disable
   * @return ESP_OK on success
   */
  esp_err_t enable_charging(bool enable);

  /**
   * @brief Enable OTG boost mode
   * @param enable True to enable, false to disable
   * @return ESP_OK on success
   */
  esp_err_t enable_otg(bool enable);

  /**
   * @brief Set charge current limit
   * @param current_ma Charge current in mA (40-2000mA)
   * @return ESP_OK on success
   */
  esp_err_t set_charge_current(uint16_t current_ma);

  /**
   * @brief Set charge voltage limit
   * @param voltage_mv Battery voltage in mV (3500-4800mV)
   * @return ESP_OK on success
   */
  esp_err_t set_charge_voltage(uint16_t voltage_mv);

  /**
   * @brief Set input current limit
   * @param current_ma Input current in mA (100-3200mA)
   * @return ESP_OK on success
   */
  esp_err_t set_input_current_limit(uint16_t current_ma);

  /**
   * @brief Get charge status
   * @param status Output charge status
   * @return ESP_OK on success
   */
  esp_err_t get_charge_status(ChargeStatus &status);

  /**
   * @brief Get VBUS status
   * @param status Output VBUS status
   * @return ESP_OK on success
   */
  esp_err_t get_vbus_status(VBusStatus &status);

  /**
   * @brief Read ADC measurements
   * @param data Output ADC data structure
   * @return ESP_OK on success
   */
  esp_err_t read_adc(BQ25629_ADC_Data &data);

  /**
   * @brief Enable ADC conversion
   * @param enable True to enable, false to disable
   * @param continuous True for continuous mode, false for one-shot
   * @return ESP_OK on success
   */
  esp_err_t enable_adc(bool enable, bool continuous = true);

  /**
   * @brief Reset watchdog timer
   * @return ESP_OK on success
   */
  esp_err_t reset_watchdog();

  /**
   * @brief Check if battery is present
   * @param present Output: true if battery present
   * @return ESP_OK on success
   */
  esp_err_t is_battery_present(bool &present);

  /**
   * @brief Check if charging
   * @param charging Output: true if charging
   * @return ESP_OK on success
   */
  esp_err_t is_charging(bool &charging);

  /**
   * @brief Read register (8-bit)
   * @param reg_addr Register address
   * @param value Output value
   * @return ESP_OK on success
   */
  esp_err_t read_register(uint8_t reg_addr, uint8_t &value);

  /**
   * @brief Write register (8-bit)
   * @param reg_addr Register address
   * @param value Value to write
   * @return ESP_OK on success
   */
  esp_err_t write_register(uint8_t reg_addr, uint8_t value);

  /**
   * @brief Read 16-bit register (little-endian)
   * @param reg_addr Register address (LSB)
   * @param value Output value
   * @return ESP_OK on success
   */
  esp_err_t read_register_16(uint8_t reg_addr, uint16_t &value);

  /**
   * @brief Write 16-bit register (little-endian)
   * @param reg_addr Register address (LSB)
   * @param value Value to write
   * @return ESP_OK on success
   */
  esp_err_t write_register_16(uint8_t reg_addr, uint16_t value);

private:
  i2c_master_bus_handle_t i2c_bus_;
  i2c_master_dev_handle_t dev_handle_;
  uint8_t device_address_;
  bool initialized_;

  // Helper functions
  esp_err_t modify_register(uint8_t reg_addr, uint8_t mask, uint8_t value);
};

} // namespace drivers
