# 📋 Code Organization & Review Guide

**Last Updated**: January 10, 2026

---

## 🏗️ Current Project Structure

```
airgradient-go/
├── main/                  ← Application code
│   ├── sensor.h/cpp       ← Sensor interface (CO2, VOC, NOx, PM2.5)
│   ├── ui_display.h/cpp   ← LVGL display & UI
│   └── lv_conf.h          ← LVGL configuration
│
├── components/            ← Reusable driver libraries
│   ├── esp_epaper/        ← SSD1680 e-paper display driver
│   ├── stcc4/             ← SCD4x CO2 sensor (STCC4 variant)
│   ├── sgp4x/             ← SGP4x VOC/NOx sensor
│   ├── sps30/             ← SPS30 particulate matter sensor
│   └── sensirion_gas_index_algorithm/  ← Gas index calculation
│
├── .config/               ← Build configuration
├── .docs/                 ← Documentation
└── README.md
```

---

## ✅ Code Review Checklist

### Header Files (`.h`)

- [ ] Include guards with `#pragma once`
- [ ] Clear struct/class documentation
- [ ] Forward declarations where needed
- [ ] Function signatures with brief descriptions
- [ ] No implementation code
- [ ] All `#include` statements in header, implementation includes in `.cpp`

### Implementation Files (`.cpp`)

- [ ] `#include` order: local first, then system headers
- [ ] `static const char *TAG` defined for logging
- [ ] Block comments for complex functions
- [ ] Inline comments explaining logic, not obvious code
- [ ] Error handling: check all return codes
- [ ] Use `ESP_ERROR_CHECK()` or `ESP_LOGI()` for errors

### Code Style (`.clang-format`)

```yaml
BasedOnStyle: LLVM
ColumnLimit: 100
UseTab: Never
IndentWidth: 2
```

**Quick check:**
```bash
# Format all files
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Check without modifying
clang-format --dry-run main/sensor.cpp
```

---

## 📐 Typical Code Pattern

### Class Definition (Header)

```cpp
#pragma once

#include "esp_err.h"
#include <stdint.h>

// Brief description of what this class does
class MySensor {
public:
  // Constructor and destructor
  MySensor();
  ~MySensor();

  // Initialize sensor on I2C bus. Returns ESP_OK on success.
  esp_err_t init(void);

  // Periodic update call. Should be non-blocking.
  void update(int64_t now_ms);

  // Get latest reading. Returns true if data is fresh.
  bool getReading(int& value);

private:
  // Internal state
  struct State;
  State* state;
};
```

### Implementation (CPP)

```cpp
#include "my_sensor.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

static const char* TAG = "MY_SENSOR";

// Private state definition
struct MySensor::State {
  i2c_master_dev_handle_t dev;
  int last_value;
};

// Constructor
MySensor::MySensor() : state(nullptr) {}

// Initialize sensor and I2C communication.
// Performs sensor self-test and configures measurement mode.
esp_err_t MySensor::init(void) {
  if (state) {
    ESP_LOGW(TAG, "Already initialized");
    return ESP_ERR_INVALID_STATE;
  }

  state = new State();
  if (!state) {
    ESP_LOGE(TAG, "Failed to allocate state");
    return ESP_ERR_NO_MEM;
  }

  // TODO: Initialize I2C device
  return ESP_OK;
}
```

---

## 🔌 Sensor Integration Pattern

All sensors in `main/sensor.cpp` follow this pattern:

1. **Struct Definition** (in `SensorsState`):
   ```cpp
   struct Sensors::SensorsState {
     // Device handle or data structure
     my_sensor_handle_t device;
     
     // Latest measurements
     sensor_measurement_t data;
     
     // State machine variables
     sensor_state_t state;
     int64_t last_update_time;
   };
   ```

2. **Initialization** (in `Sensors::init()`):
   - Initialize I2C bus (once, shared)
   - Initialize each sensor
   - Set up state machines
   - Return `ESP_OK` or error code

3. **Update Loop** (in `Sensors::update()`):
   - Call non-blocking update for each sensor
   - Handle state transitions
   - Average values for display (5-second window)

4. **Data Retrieval** (in `Sensors::getValues()`):
   - Return `sensor_values_t` struct
   - Always provide averaged values
   - Handle missing sensors gracefully

---

## 🎨 Display UI Pattern

The `Display` class (in `ui_display.cpp`) manages LVGL widgets:

### Initialization
```cpp
bool Display::init(uint16_t width, uint16_t height) {
  // Create LVGL display driver
  // Create screen and main container
  // Create all sub-widgets (status icons, sensor readouts, etc.)
  // Set up event handlers
  return true;
}
```

### Status Updates
```cpp
void Display::setBattery(int percent, bool charging) {
  // Update battery icon (0-100%)
  // Show charging indicator (blinking when charging)
}
```

### Widget Organization
- **Status Bar** (top): WiFi, BLE, GPS, Battery, Recording, Alert
- **Main Content** (center): Sensor readings (CO2, PM2.5, VOC, NOx, Temp, RH, Pressure)
- **Footer** (bottom): Time, Recording interval

---

## 📝 Comment Guidelines

### Good Comments (What & Why)
```cpp
// Average CO2 over 5-second window to reduce noise
int avg_co2 = calculate_average(co2_ring, CO2_RING_CAP);
```

### Bad Comments (Obvious)
```cpp
i++;  // Increment i
int x = y + 1;  // Add 1 to y and store in x
```

### Block Comments (Before Functions)
```cpp
// Initialize I2C master bus with standard configuration.
// SCL=GPIO6, SDA=GPIO7, Frequency=100kHz.
// Returns I2C bus handle on success, NULL on error.
i2c_master_bus_handle_t init_i2c(void) {
  // Implementation
}
```

### Inline Comments (In Code)
```cpp
// Reduce noise by averaging 12 samples over 5 seconds
float avg = sum / 12;

// Retry up to 3 times if I2C read fails
for (int retry = 0; retry < 3; retry++) {
  // Implementation
}
```

---

## 🚀 Build & Verify

### Before Commit

```bash
# 1. Check code style
find main -name "*.cpp" -o -name "*.hpp" | xargs clang-format --dry-run

# 2. Format code
find main -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# 3. Build project
idf.py build

# 4. Check binary size (4 MB limit!)
idf.py size

# 5. Check for compilation warnings
idf.py build 2>&1 | grep -i warning
```

### Typical Size Targets
- Bootloader: ~64 KB
- App binary: ≤ 900 KB
- Free space: For OTA updates

---

## 🔍 Code Review Focus Areas

### Safety
- [ ] All error codes checked
- [ ] No null pointer dereferences
- [ ] Bounds checking on arrays
- [ ] Resource cleanup in destructors

### Performance
- [ ] No blocking calls in update loops
- [ ] Efficient state machines
- [ ] Reasonable memory usage
- [ ] DMA for large data transfers

### Readability
- [ ] Meaningful variable names
- [ ] Consistent code style
- [ ] Comments explain "why", not "what"
- [ ] Functions < 100 lines (ideally < 50)

### Standards
- [ ] Follows `IDriver` interface (if applicable)
- [ ] Uses `esp_err_t` for error codes
- [ ] Logs with `ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE`
- [ ] Respects partition table limits

---

## 📚 Related Files

- **Code style**: [`.config/.clang-format`](../../.config/.clang-format)
- **Build guide**: [`.docs/AGENT.md`](../AGENT.md)
- **Hardware map**: [`.docs/ARCHITECTURE/HARDWARE_MAP.md`](../ARCHITECTURE/HARDWARE_MAP.md)
- **Driver standards**: [`.docs/ARCHITECTURE/DRIVER_CONTRACTS.md`](../ARCHITECTURE/DRIVER_CONTRACTS.md)

---

## 💡 Tips

1. **Use `static` for file-scoped variables**: Don't pollute global namespace
2. **Prefer enums over magic numbers**: Improves readability
3. **Use typedef/using for complex types**: `using ms_t = int64_t;`
4. **Keep state machines simple**: Use explicit enum states
5. **Test on actual hardware**: Emulators miss real-world timing issues
6. **Monitor memory**: Watch heap fragmentation over time
7. **Use assertions during development**: Remove in production build

---

**ขอให้ AI ตรวจสอบ code ด้วยแบบฟอร์มนี้เพื่อให้ได้คุณภาพที่สม่ำเสมอ**
