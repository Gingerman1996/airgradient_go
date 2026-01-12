#include "cap1203.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char* TAG = "CAP1203";

// Timing constants
#define LONG_PRESS_MS 1000  // 1 second for long press

CAP1203::CAP1203(i2c_master_bus_handle_t bus_handle)
    : _bus_handle(bus_handle)
    , _dev_handle(nullptr)
    , _last_button_status(0)
    , _callback(nullptr)
    , _callback_user_data(nullptr)
{
    memset(_button_press_time, 0, sizeof(_button_press_time));
    memset(_button_long_fired, 0, sizeof(_button_long_fired));
}

CAP1203::~CAP1203() {
    if (_dev_handle) {
        i2c_master_bus_rm_device(_dev_handle);
    }
}

esp_err_t CAP1203::readRegister(uint8_t reg, uint8_t* data) {
    return i2c_master_transmit_receive(_dev_handle, &reg, 1, data, 1, 1000);
}

esp_err_t CAP1203::writeRegister(uint8_t reg, uint8_t data) {
    uint8_t write_buf[2] = {reg, data};
    return i2c_master_transmit(_dev_handle, write_buf, 2, 1000);
}

esp_err_t CAP1203::readRegisters(uint8_t reg, uint8_t* data, size_t len) {
    return i2c_master_transmit_receive(_dev_handle, &reg, 1, data, len, 1000);
}

esp_err_t CAP1203::init() {
    esp_err_t ret;

    // Add device to I2C bus
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = CAP1203_I2C_ADDRESS,
        .scl_speed_hz = 100000,  // 100 kHz
    };

    ret = i2c_master_bus_add_device(_bus_handle, &dev_cfg, &_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add I2C device: %s", esp_err_to_name(ret));
        return ret;
    }

    // Probe device
    ret = probe();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to probe CAP1203");
        return ret;
    }

    // Soft reset and wait for device ready
    ret = writeRegister(CAP1203_REG_MAIN_CONTROL, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to reset device");
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(100));

    // Enable all 3 sensor inputs
    ret = writeRegister(CAP1203_REG_SENSOR_INPUT_ENABLE, 0x07);  // Enable CS1, CS2, CS3
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable sensor inputs");
        return ret;
    }

    // Set sensitivity (default: moderate sensitivity 0x2F = 128x, 4 samples)
    // Bits 6-4: Sensitivity (0x2 = 128x gain)
    // Bits 3-0: Delta sense controls (0xF = max)
    ret = setSensitivity(0x02);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set sensitivity");
        return ret;
    }

    // Set averaging and sampling config
    // Default: 8 samples, 1.28ms sampling time, 35ms cycle time
    ret = writeRegister(CAP1203_REG_AVERAGING_SAMPLING, 0x39);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set averaging/sampling");
        return ret;
    }

    // Set touch thresholds (default 0x40 = 64)
    ret = setThreshold(0, 0x40);  // CS1 (Left)
    ret |= setThreshold(1, 0x40);  // CS2 (Middle)
    ret |= setThreshold(2, 0x40);  // CS3 (Right)
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set thresholds");
        return ret;
    }

    // Configure general settings
    // Bit 5: INT bit = 1 (interrupt repeats)
    // Bit 4: TIMEOUT bit = 0 (timeout disabled)
    // Bit 3-0: MAX_DUR = 0x0 (max duration 560ms)
    ret = writeRegister(CAP1203_REG_CONFIG, 0x20);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write config register");
        return ret;
    }

    // Configure sensor input settings
    // Bit 7: MAX_DUR_EN = 0 (max duration disabled for all)
    // Bit 6-5: RPT_RATE = 00 (35ms repeat rate)
    // Bit 4: M_PRESS = 0 (multiple touch off)
    // Bit 3-0: Reserved = 0
    ret = writeRegister(CAP1203_REG_SENSOR_INPUT_CONFIG, 0x00);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write sensor input config");
        return ret;
    }

    // Enable interrupts for all sensor inputs
    ret = setInterruptEnable(true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable interrupts");
        return ret;
    }

    // Calibrate sensors
    ret = calibrate();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to calibrate sensors");
        return ret;
    }

    // Clear any pending interrupts
    ret = clearInterrupt();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear interrupts");
        return ret;
    }

    ESP_LOGI(TAG, "CAP1203 initialized successfully");
    return ESP_OK;
}

esp_err_t CAP1203::probe() {
    uint8_t product_id, manufacturer_id, revision;
    esp_err_t ret;

    ret = readRegister(CAP1203_REG_PRODUCT_ID, &product_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read product ID");
        return ret;
    }

    ret = readRegister(CAP1203_REG_MANUFACTURER_ID, &manufacturer_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read manufacturer ID");
        return ret;
    }

    ret = readRegister(CAP1203_REG_REVISION, &revision);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read revision");
        return ret;
    }

    ESP_LOGI(TAG, "CAP1203 found - Product ID: 0x%02X, Manufacturer: 0x%02X, Revision: 0x%02X",
             product_id, manufacturer_id, revision);

    // Verify product ID (should be 0x6D for CAP1203)
    if (product_id != 0x6D) {
        ESP_LOGE(TAG, "Invalid product ID: expected 0x6D, got 0x%02X", product_id);
        return ESP_ERR_NOT_FOUND;
    }

    return ESP_OK;
}

esp_err_t CAP1203::getButtonStatus(uint8_t* status) {
    return readRegister(CAP1203_REG_SENSOR_INPUT_STATUS, status);
}

bool CAP1203::isButtonPressed(ButtonID button_id) {
    uint8_t status;
    if (getButtonStatus(&status) != ESP_OK) {
        return false;
    }

    uint8_t button_bit = static_cast<uint8_t>(button_id);
    return (status & (1 << button_bit)) != 0;
}

esp_err_t CAP1203::clearInterrupt() {
    uint8_t main_ctrl = 0;
    esp_err_t ret = readRegister(CAP1203_REG_MAIN_CONTROL, &main_ctrl);
    if (ret != ESP_OK) {
        return ret;
    }

    // Clear INT bit (bit 0) to release latched touch status.
    if (main_ctrl & 0x01) {
        main_ctrl &= (uint8_t)~0x01;
        ret = writeRegister(CAP1203_REG_MAIN_CONTROL, main_ctrl);
        if (ret != ESP_OK) {
            return ret;
        }
    }

    // Read general status to complete interrupt clear sequence.
    uint8_t status = 0;
    return readRegister(CAP1203_REG_GENERAL_STATUS, &status);
}

esp_err_t CAP1203::setSensitivity(uint8_t sensitivity) {
    if (sensitivity > 7) {
        ESP_LOGE(TAG, "Invalid sensitivity: %d (must be 0-7)", sensitivity);
        return ESP_ERR_INVALID_ARG;
    }

    // Set sensitivity multiplier (bits 6-4) and delta sense (bits 3-0)
    uint8_t value = (sensitivity << 4) | 0x0F;
    return writeRegister(CAP1203_REG_SENSITIVITY_CONTROL, value);
}

esp_err_t CAP1203::setThreshold(uint8_t sensor_id, uint8_t threshold) {
    if (sensor_id > 2) {
        ESP_LOGE(TAG, "Invalid sensor ID: %d (must be 0-2)", sensor_id);
        return ESP_ERR_INVALID_ARG;
    }

    if (threshold > 127) {
        ESP_LOGE(TAG, "Invalid threshold: %d (must be 0-127)", threshold);
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t reg = CAP1203_REG_SENSOR_INPUT_1_THRESHOLD + sensor_id;
    return writeRegister(reg, threshold);
}

esp_err_t CAP1203::setInterruptEnable(bool enable) {
    uint8_t value = enable ? 0x07 : 0x00;  // Enable/disable interrupts for CS1, CS2, CS3
    return writeRegister(CAP1203_REG_INTERRUPT_ENABLE, value);
}

void CAP1203::setButtonCallback(button_callback_t callback, void* user_data) {
    _callback = callback;
    _callback_user_data = user_data;
}

void CAP1203::processButtons() {
    uint8_t current_status;
    if (getButtonStatus(&current_status) != ESP_OK) {
        return;
    }

    // Mask to only 3 buttons
    current_status &= 0x07;

    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Check each button
    for (int i = 0; i < 3; i++) {
        bool was_pressed = (_last_button_status & (1 << i)) != 0;
        bool is_pressed = (current_status & (1 << i)) != 0;

        if (is_pressed && !was_pressed) {
            // Button just pressed
            _button_press_time[i] = current_time;
            _button_long_fired[i] = false;

            // Fire PRESS event
            if (_callback) {
                ButtonState state;
                state.id = static_cast<ButtonID>(i);
                state.event = ButtonEvent::PRESS;
                state.press_duration_ms = 0;
                _callback(state, _callback_user_data);
            }
        } 
        else if (!is_pressed && was_pressed) {
            // Button just released
            uint32_t press_duration = current_time - _button_press_time[i];

            // Fire RELEASE event
            if (_callback) {
                ButtonState state;
                state.id = static_cast<ButtonID>(i);
                state.event = ButtonEvent::RELEASE;
                state.press_duration_ms = press_duration;
                _callback(state, _callback_user_data);
            }

            // Fire SHORT_PRESS or LONG_PRESS event if not already fired
            if (!_button_long_fired[i]) {
                if (_callback) {
                    ButtonState state;
                    state.id = static_cast<ButtonID>(i);
                    state.event = (press_duration >= LONG_PRESS_MS) ? ButtonEvent::LONG_PRESS : ButtonEvent::SHORT_PRESS;
                    state.press_duration_ms = press_duration;
                    _callback(state, _callback_user_data);
                }
            }

            _button_press_time[i] = 0;
            _button_long_fired[i] = false;
        }
        else if (is_pressed && was_pressed) {
            // Button still pressed - check for long press
            uint32_t press_duration = current_time - _button_press_time[i];
            
            if (!_button_long_fired[i] && press_duration >= LONG_PRESS_MS) {
                // Fire LONG_PRESS event
                if (_callback) {
                    ButtonState state;
                    state.id = static_cast<ButtonID>(i);
                    state.event = ButtonEvent::LONG_PRESS;
                    state.press_duration_ms = press_duration;
                    _callback(state, _callback_user_data);
                }
                _button_long_fired[i] = true;
            }
        }
    }

    _last_button_status = current_status;

    // Clear interrupt if any
    clearInterrupt();
}

esp_err_t CAP1203::getDeltaValues(int8_t* cs1_delta, int8_t* cs2_delta, int8_t* cs3_delta) {
    esp_err_t ret;
    uint8_t delta[3];

    ret = readRegisters(CAP1203_REG_SENSOR_INPUT_1_DELTA, delta, 3);
    if (ret != ESP_OK) {
        return ret;
    }

    // Convert from unsigned to signed
    *cs1_delta = (int8_t)delta[0];
    *cs2_delta = (int8_t)delta[1];
    *cs3_delta = (int8_t)delta[2];

    return ESP_OK;
}

esp_err_t CAP1203::calibrate() {
    // Write 0x07 to calibration register to calibrate CS1, CS2, CS3
    esp_err_t ret = writeRegister(CAP1203_REG_CALIBRATION_ACTIVATE, 0x07);
    if (ret != ESP_OK) {
        return ret;
    }

    // Wait for calibration to complete (typically takes a few cycles)
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "Calibration complete");
    return ESP_OK;
}
