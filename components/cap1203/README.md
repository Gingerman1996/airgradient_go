# CAP1203 Capacitive Touch Sensor Driver

ESP-IDF component for the CAP1203-1-AC3-TR capacitive touch sensor IC.

## Features

- ✅ 3 capacitive touch sensor inputs (CS1, CS2, CS3)
- ✅ I2C/SMBus interface compatible (100kHz standard, up to 400kHz fast mode)
- ✅ I2C Address: **0x28** (7-bit, ADDR pin to GND)
- ✅ Product ID: **0x6D** @ register 0xFD (CAP1203-1-AC3-TR)
- ✅ Manufacturer ID: **0x5D** @ register 0xFE (Microchip Technology)
- ✅ Button event detection (PRESS, RELEASE, SHORT_PRESS, LONG_PRESS)
- ✅ Programmable sensitivity (128 levels via DELTA_SENSE register)
- ✅ Individual thresholds for each button (configurable 0-255)
- ✅ Automatic calibration and recalibration
- ✅ Multiple button pattern detection support
- ✅ Press and hold feature (configurable duration)
- ✅ Polling mode supported (ALERT# pin optional)
- ✅ Event callback system with user context
- ✅ Low power operation
  - 5µA in Deep Sleep mode
  - 50µA in Standby (1 sensor monitored)
  - 120µA typical in Active mode
- ✅ Wide supply voltage: 3.0V to 5.5V
- ✅ 8kV ESD protection (HBM) on all pins
- ✅ RF noise detection and filtering
- ✅ Package: 8-pin TDFN 2mm x 3mm (RoHS compliant)

## Hardware Connections

| CAP1203 Pin | Pin# | ESP32-C5 GPIO | Touch Pad | Description |
|-------------|------|---------------|-----------|-------------|
| VDD | 4 | 3.3V | - | Power supply (3.0V-5.5V) |
| GND | 5 | GND | - | Ground reference |
| SMCLK | 3 | GPIO 6 | - | I2C Clock (requires pull-up, 5V tolerant) |
| SMDATA | 2 | GPIO 7 | - | I2C Data (open-drain, requires pull-up, 5V tolerant) |
| ALERT# | 1 | (optional) | - | Interrupt output (active-low, optional for polling) |
| CS1 | 8 | - | T3 | Right button - Capacitive sensor pad |
| CS2 | 7 | - | T1 | Left button - Capacitive sensor pad |
| CS3 | 6 | - | T2 | Middle button - Capacitive sensor pad |
| Exposed Pad | - | GND | - | Connect to ground (not internally connected) |

**Notes:**
- ADDR pin should be connected to GND for I2C address 0x28
- Alternative address 0x29 available if ADDR connected to VDD (not used)
- Package: CAP1203-1-AC3-TR (8-pin TDFN 2mm x 3mm)

**⚠️ Power Requirements:**
- CAP1203 likely powered by **2.8V LDO (TPS7A0228)** which requires **battery connected**
- If CAP1203 not detected on I2C scan (address 0x28), check:
  1. Battery is connected and charged
  2. 2.8V power rail is active
  3. VDD pin has proper voltage (2.8V or 3.3V)
- Alternative: May use 3.1V system rail (verify with schematic)

## Usage Example

```cpp
#include "cap1203.h"

// Callback function for button events
void button_callback(ButtonState state, void* user_data) {
    switch (state.event) {
        case ButtonEvent::SHORT_PRESS:
            ESP_LOGI("BUTTON", "Button %d short press", (int)state.id);
            break;
        case ButtonEvent::LONG_PRESS:
            ESP_LOGI("BUTTON", "Button %d long press (%lu ms)", 
                     (int)state.id, state.press_duration_ms);
            break;
        default:
            break;
    }
}

// Initialize
CAP1203 touchSensor(i2c_bus_handle);
touchSensor.init();

// Set callback
touchSensor.setButtonCallback(button_callback, nullptr);

// In main loop or timer task
while (1) {
    touchSensor.processButtons();
    vTaskDelay(pdMS_TO_TICKS(10));  // Poll every 10ms
}
```

## API Reference

### Initialization

- `CAP1203(i2c_master_bus_handle_t bus_handle)` - Constructor
- `esp_err_t init()` - Initialize sensor with default settings
- `esp_err_t probe()` - Verify sensor communication

### Button Reading

- `esp_err_t getButtonStatus(uint8_t* status)` - Get raw button bitmap
- `bool isButtonPressed(ButtonID button_id)` - Check specific button
- `void processButtons()` - Process button state changes (call frequently)

### Configuration

- `esp_err_t setSensitivity(uint8_t sensitivity)` - Set sensitivity (0-7)
- `esp_err_t setThreshold(uint8_t sensor_id, uint8_t threshold)` - Set touch threshold
- `esp_err_t setInterruptEnable(bool enable)` - Enable/disable interrupts

### Event Handling

- `void setButtonCallback(button_callback_t callback, void* user_data)` - Register callback

### Utilities

- `esp_err_t calibrate()` - Recalibrate all sensors
- `esp_err_t clearInterrupt()` - Clear interrupt flag
- `esp_err_t getDeltaValues(int8_t* cs1, int8_t* cs2, int8_t* cs3)` - Debug info

## Button Events

- `ButtonEvent::PRESS` - Button pressed down
- `ButtonEvent::RELEASE` - Button released
- `ButtonEvent::SHORT_PRESS` - Short press detected (<1s)
- `ButtonEvent::LONG_PRESS` - Long press detected (≥1s)

## Button IDs

Button mapping according to hardware schematic:

- `ButtonID::BUTTON_LEFT` (1) - CS2 → T1 (Left touch pad)
- `ButtonID::BUTTON_MIDDLE` (2) - CS3 → T2 (Middle touch pad)
- `ButtonID::BUTTON_RIGHT` (0) - CS1 → T3 (Right touch pad)

## Configuration

Default settings on initialization:
- Sensitivity: 128x gain (moderate)
- Threshold: 64 (default touch detection)
- Averaging: 8 samples
- Sampling time: 1.28ms
- Cycle time: 35ms
- Long press timeout: 1000ms

## Dependencies

- `esp_driver_i2c` - ESP-IDF I2C master driver
- `freertos` - FreeRTOS for timing
- `log` - ESP logging

## References

- Datasheet: `.docs/REFERENCES/pdf/CAP1203-1-AC3-TR.pdf`
- I2C Address: 0x28
- Product ID: 0x6D

## License

Part of the AirGradient GO firmware project.
