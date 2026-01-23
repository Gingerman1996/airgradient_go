# 🚀 Quick Start Guide

> **Purpose**: Fast reference for common development tasks without reading full documentation.

---

## ⚡ 5-Minute Setup

```bash
# 1. Activate ESP-IDF environment
source ~/esp/v5.5.1/esp-idf/export.sh

# 2. Build project
idf.py build

# 3. Flash to device
idf.py flash monitor

# 4. Watch serial output in new terminal
idf.py monitor
```

---

## 🔥 Most Common Commands

### Build & Deploy
```bash
idf.py build                    # Build only
idf.py flash                    # Flash only
idf.py flash monitor            # Flash + see serial output
idf.py -p /dev/ttyUSB0 flash   # Flash to specific port
idf.py fullclean && idf.py build # Clean build (nuke build/)
```

### Code Formatting
```bash
# Format single file
clang-format -i src/main.cpp

# Format all C++ files
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Check without modifying (dry-run)
clang-format --dry-run src/main.cpp
```

### Project Info
```bash
idf.py size                     # App binary size
idf.py size-components          # Component breakdown
idf.py menuconfig               # Configure project (WiFi, debug, etc.)
```

---

## 🔌 Hardware Reference (No Doc Lookup)

### GPIO Quick Map
| GPIO | Function |
|------|----------|
| IO0 | E-paper CS |
| IO4 | Flash CS |
| IO5 | QON button |
| IO6 | I2C SCL |
| IO7/IO8 | I2C SDA |
| IO9 | E-paper RES |
| IO10 | E-paper BUSY |
| IO15 | E-paper D/C |
| IO23 | SPI SCLK |
| IO24 | SPI MISO |
| IO25 | SPI MOSI |
| IO39 | UART TX (GPS) |
| IO40 | UART RX (GPS) |

### I2C Sensors
```
0x44 - SHT4x (temp/humidity)
0x59 - SGP41 (VOC)
0x69 - SPS30 (PM sensor)
0x18 - LIS2DH12 (accelerometer, alt 0x19)
0x76 - DPS368 (barometer, alt 0x77)
0x28 - CAP1203 (capacitive buttons)
```

### Power States
```
Press QON button → Exit Ship Mode → Active mode
```

---

## 📦 Adding a Component

### Method 1: Clone Existing Library
```bash
cd components
git clone <repo-url> my-component
cd ..
```

### Method 2: Create from Scratch
```bash
# Create structure
mkdir -p components/my-component/{include,src}

# Create CMakeLists.txt in components/my-component/
cat > components/my-component/CMakeLists.txt << 'EOF'
idf_component_register(
    SRCS "src/my_component.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp-idf
)
EOF

# Reference in main CMakeLists.txt
# REQUIRES my-component
```

---

## 🐛 Quick Debugging

### Add Logging
```cpp
#include "esp_log.h"

static const char *TAG = "MY_MODULE";

ESP_LOGI(TAG, "Message: %d", value);    // Info
ESP_LOGW(TAG, "Warning: %s", text);     // Warning
ESP_LOGE(TAG, "Error code: 0x%x", err); // Error
```

### Check Errors
```bash
# See compile errors
idf.py build 2>&1 | grep error

# Check ESP-IDF doctor
idf.py doctor
```

---

## 📄 Code Templates

### Minimal Driver
```cpp
// my_driver.hpp
#pragma once
#include "esp_err.h"
#include "driver_interface.hpp"

class MyDriver : public drivers::IDriver {
public:
  esp_err_t init() override;
  esp_err_t start() override;
  esp_err_t read(DriverData* data) override;
  esp_err_t sleep() override;
  esp_err_t wake() override;
  esp_err_t deinit() override;
};
```

### I2C Read
```cpp
#include "driver/i2c_master.h"

i2c_master_dev_handle_t dev;
uint8_t buffer[8];
ESP_ERROR_CHECK(i2c_master_transmit_receive(
    dev, &reg, 1, buffer, 8, -1));
```

### GPIO Output
```cpp
#include "driver/gpio.h"

gpio_config_t cfg = {
  .pin_bit_mask = BIT(IO_NUM),
  .mode = GPIO_MODE_OUTPUT,
};
gpio_config(&cfg);
gpio_set_level(IO_NUM, 1);
```

---

## ✅ Pre-Commit Checklist

- [ ] Code formatted: `clang-format -i <file>`
- [ ] No compiler warnings
- [ ] Logged key operations: `ESP_LOGI(TAG, ...)`
- [ ] Error codes checked: `ESP_ERROR_CHECK()`
- [ ] Built successfully: `idf.py build`

---

## 🆘 Troubleshooting

| Issue | Solution |
|-------|----------|
| `esp-idf` not found | Run: `source ~/esp/v5.5.1/esp-idf/export.sh` |
| Port not found | Check: `ls /dev/tty*` or `idf.py monitor --port /dev/ttyUSB0` |
| Build fails | Try: `idf.py fullclean && idf.py build` |
| Formatting mismatch | Run: `clang-format -i <file>` before commit |
| Component not found | Verify CMakeLists.txt has `REQUIRES component-name` |

---

## 📚 Full Docs

- Architecture: [AGENT.md](AGENT.md)
- Drivers: [ARCHITECTURE/DRIVER_CONTRACTS.md](ARCHITECTURE/DRIVER_CONTRACTS.md)
- Hardware: [ARCHITECTURE/HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md)
- Power: [ARCHITECTURE/POWER_STATES.md](ARCHITECTURE/POWER_STATES.md)
- Plan: [ARCHITECTURE/FIRMWARE_PLAN.md](ARCHITECTURE/FIRMWARE_PLAN.md)
- Docs: [REFERENCES/README.md](REFERENCES/README.md)

---

**Tip**: Bookmark this file! 🔖

