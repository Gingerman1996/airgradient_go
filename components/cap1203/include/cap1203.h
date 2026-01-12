#pragma once

#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"

// CAP1203 I2C Address (ADDR pin to GND)
#define CAP1203_I2C_ADDRESS 0x28  // 0101_000(r/w) in binary

// Product identification (from datasheet CAP1203-1-AC3-TR)
#define CAP1203_PRODUCT_ID 0x6D       // Product ID register (0xFD)
#define CAP1203_MANUFACTURER_ID 0x5D  // Manufacturer ID - Microchip Technology (0xFE)

// CAP1203 Register Addresses
#define CAP1203_REG_MAIN_CONTROL        0x00
#define CAP1203_REG_GENERAL_STATUS      0x02
#define CAP1203_REG_SENSOR_INPUT_STATUS 0x03
#define CAP1203_REG_NOISE_FLAG_STATUS   0x0A
#define CAP1203_REG_SENSOR_INPUT_1_DELTA 0x10
#define CAP1203_REG_SENSOR_INPUT_2_DELTA 0x11
#define CAP1203_REG_SENSOR_INPUT_3_DELTA 0x12
#define CAP1203_REG_SENSITIVITY_CONTROL 0x1F
#define CAP1203_REG_CONFIG              0x20
#define CAP1203_REG_SENSOR_INPUT_ENABLE 0x21
#define CAP1203_REG_SENSOR_INPUT_CONFIG 0x22
#define CAP1203_REG_SENSOR_INPUT_CONFIG_2 0x23
#define CAP1203_REG_AVERAGING_SAMPLING  0x24
#define CAP1203_REG_CALIBRATION_ACTIVATE 0x26
#define CAP1203_REG_INTERRUPT_ENABLE    0x27
#define CAP1203_REG_REPEAT_RATE_ENABLE  0x28
#define CAP1203_REG_SIGNAL_GUARD_ENABLE 0x29
#define CAP1203_REG_MULTIPLE_TOUCH_CONFIG 0x2A
#define CAP1203_REG_MULTIPLE_TOUCH_PATTERN_CONFIG 0x2B
#define CAP1203_REG_MULTIPLE_TOUCH_PATTERN 0x2D
#define CAP1203_REG_BASE_COUNT_OUT      0x2E
#define CAP1203_REG_POWER_BUTTON        0x60
#define CAP1203_REG_POWER_BUTTON_CONFIG 0x61
#define CAP1203_REG_SENSOR_INPUT_1_THRESHOLD 0x30
#define CAP1203_REG_SENSOR_INPUT_2_THRESHOLD 0x31
#define CAP1203_REG_SENSOR_INPUT_3_THRESHOLD 0x32
#define CAP1203_REG_SENSOR_INPUT_NOISE_THRESHOLD 0x38
#define CAP1203_REG_STANDBY_CHANNEL     0x40
#define CAP1203_REG_STANDBY_CONFIG      0x41
#define CAP1203_REG_STANDBY_SENSITIVITY 0x42
#define CAP1203_REG_STANDBY_THRESHOLD   0x43
#define CAP1203_REG_CONFIG_2            0x44
#define CAP1203_REG_SENSOR_INPUT_1_BASE 0x50
#define CAP1203_REG_SENSOR_INPUT_2_BASE 0x51
#define CAP1203_REG_SENSOR_INPUT_3_BASE 0x52
#define CAP1203_REG_PRODUCT_ID          0xFD
#define CAP1203_REG_MANUFACTURER_ID     0xFE
#define CAP1203_REG_REVISION            0xFF

// Button IDs (mapped to hardware connections)
enum class ButtonID : uint8_t {
    BUTTON_LEFT = 1,    // CS2 → T1 (Left)
    BUTTON_MIDDLE = 2,  // CS3 → T2 (Middle)
    BUTTON_RIGHT = 0,   // CS1 → T3 (Right)
    BUTTON_NONE = 0xFF
};

// Button event types
enum class ButtonEvent {
    PRESS,        // Button was pressed
    RELEASE,      // Button was released
    SHORT_PRESS,  // Short press detected
    LONG_PRESS,   // Long press detected (>1s)
    NONE
};

// Button state structure
struct ButtonState {
    ButtonID id;
    ButtonEvent event;
    uint32_t press_duration_ms;  // Duration of press in milliseconds
};

// Button callback function type
typedef void (*button_callback_t)(ButtonState state, void* user_data);

class CAP1203 {
public:
    /**
     * @brief Construct a new CAP1203 object
     * 
     * @param bus_handle I2C bus handle
     */
    CAP1203(i2c_master_bus_handle_t bus_handle);

    /**
     * @brief Destructor
     */
    ~CAP1203();

    /**
     * @brief Initialize the CAP1203 sensor
     * 
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t init();

    /**
     * @brief Check if the sensor is properly initialized and communicating
     * 
     * @return esp_err_t ESP_OK if sensor responds correctly
     */
    esp_err_t probe();

    /**
     * @brief Get button touch status (bitmap)
     * 
     * @param status Pointer to store button status (bit 0=CS1/T3/Right, bit 1=CS2/T1/Left, bit 2=CS3/T2/Middle)
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t getButtonStatus(uint8_t* status);

    /**
     * @brief Check if a specific button is pressed
     * 
     * @param button_id Button ID to check
     * @return true if button is pressed, false otherwise
     */
    bool isButtonPressed(ButtonID button_id);

    /**
     * @brief Clear interrupt status
     * 
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t clearInterrupt();

    /**
     * @brief Set sensitivity for all sensors (0-7, higher = more sensitive)
     * 
     * @param sensitivity Sensitivity level 0-7
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t setSensitivity(uint8_t sensitivity);

    /**
     * @brief Set threshold for a specific sensor input (0-127)
     * 
     * @param sensor_id Sensor input (0-2 for CS1-CS3)
     * @param threshold Touch threshold value
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t setThreshold(uint8_t sensor_id, uint8_t threshold);

    /**
     * @brief Enable or disable interrupt generation
     * 
     * @param enable true to enable, false to disable
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t setInterruptEnable(bool enable);

    /**
     * @brief Register a callback for button events
     * 
     * @param callback Callback function
     * @param user_data User data to pass to callback
     */
    void setButtonCallback(button_callback_t callback, void* user_data);

    /**
     * @brief Process button state changes (call this periodically or from ISR)
     * Should be called frequently to detect short/long press events
     */
    void processButtons();

    /**
     * @brief Get delta values for debugging (touch sensitivity diagnostic)
     * 
     * @param cs1_delta CS1 delta value
     * @param cs2_delta CS2 delta value
     * @param cs3_delta CS3 delta value
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t getDeltaValues(int8_t* cs1_delta, int8_t* cs2_delta, int8_t* cs3_delta);

    /**
     * @brief Calibrate all sensors
     * 
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t calibrate();

private:
    i2c_master_bus_handle_t _bus_handle;
    i2c_master_dev_handle_t _dev_handle;
    
    // Button state tracking
    uint8_t _last_button_status;
    uint32_t _button_press_time[3];  // Press start time for each button
    bool _button_long_fired[3];      // Whether long press event was already fired
    
    // Callback
    button_callback_t _callback;
    void* _callback_user_data;

    /**
     * @brief Read a single register
     * 
     * @param reg Register address
     * @param data Pointer to store read data
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t readRegister(uint8_t reg, uint8_t* data);

    /**
     * @brief Write a single register
     * 
     * @param reg Register address
     * @param data Data to write
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t writeRegister(uint8_t reg, uint8_t data);

    /**
     * @brief Read multiple registers
     * 
     * @param reg Starting register address
     * @param data Buffer to store read data
     * @param len Number of bytes to read
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t readRegisters(uint8_t reg, uint8_t* data, size_t len);
};
