# 🤖 GitHub Copilot Agent Instructions

> **Purpose**: This document guides GitHub Copilot on how to assist with AirGradient GO firmware development using ESP-IDF and the established project structure.

---

## 📋 Quick Reference

- **MCU**: ESP32-C5-MINI-1 (with WiFi + BLE)
- **Build System**: ESP-IDF (v5.5.1)
- **Language**: C++ (application), C (drivers)
- **Display**: SSD1680 (144x296 e-paper)
- **Framework**: LVGL (Light and Versatile Graphics Library)

---

## 🔧 Environment Setup

### ESP-IDF Environment

Always activate the ESP-IDF environment before building:

```bash
source ~/esp/v5.5.1/esp-idf/export.sh
```

**Important**: For ESP32-C5-MINI-1 with **4 MB Flash**, ensure your `sdkconfig` is optimized for limited flash space:
- Use `-Os` optimization (size optimization)
- Enable LTO (Link Time Optimization)
- Disable unnecessary debug symbols

### Python Environment

Ensure Python dependencies are installed:
```bash
pip install -r requirements.txt
```

---

## 📚 Documentation & References

### Core Architecture Documents

| Document | Purpose | Key Info |
|----------|---------|----------|
| [DRIVER_CONTRACTS.md](ARCHITECTURE/DRIVER_CONTRACTS.md) | Driver API standards | `IDriver` interface, state management |
| [FIRMWARE_PLAN.md](ARCHITECTURE/FIRMWARE_PLAN.md) | Implementation roadmap | Phases, priorities, infrastructure status |
| [HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md) | GPIO & connections | Pin assignments, SPI/I2C config |
| [POWER_STATES.md](ARCHITECTURE/POWER_STATES.md) | Power management | Ship mode, active, charging states |

### Reading Datasheets & PDFs

Use **markitdown** to convert PDF datasheets to Markdown:

```bash
markitdown .docs/REFERENCES/pdf/<datasheet>.pdf > .docs/REFERENCES/datasheet_md/<datasheet>.md
```

Available datasheets in `.docs/REFERENCES/pdf/`:
- `esp32-c5-mini-1_datasheet_en.pdf` - MCU specification
- `SSD1680.pdf` - E-paper display controller
- `W25N512GV.pdf` - NAND flash memory
- `bq25629.pdf` - Battery charge controller
- Sensor datasheets: `SHT4x`, `SGP41`, `SPS30` (Sensirion)
- `LIS2DH12.pdf` - Accelerometer
- `CAP1203-1-AC3-TR.pdf` - Capacitive buttons

> See [REFERENCES/README.md](REFERENCES/README.md) for complete datasheet guide

---

## 🏗️ Project Structure

### Main Source Code
```
main/
├── CMakeLists.txt
├── main.cpp                 # Entry point
├── app/                     # Application layer
│   ├── app.hpp/cpp
│   └── states/              # State machines
├── display/                 # Display driver & UI
│   ├── display.hpp/cpp
│   ├── ui/                  # LVGL UI components
│   └── fonts/               # Embedded fonts
├── sensors/                 # Sensor interfaces
└── power/                   # Power management
```

### Components (Reusable Libraries)

```
components/
├── <library-name>/
│   ├── CMakeLists.txt       # Build configuration
│   ├── include/
│   │   └── *.h              # Public headers
│   ├── src/
│   │   └── *.cpp/.c         # Implementation
│   ├── example/             # Optional usage examples
│   └── README.md            # Component documentation
├── esp-idf-components/      # Third-party components (via IDF)
└── [other-components]/
```

### Build & Configuration
```
.
├── CMakeLists.txt           # Project-level CMake config
├── sdkconfig                # IDF project settings (auto-generated)
├── sdkconfig.defaults       # Default configuration
├── partitions.csv           # Partition table with OTA
├── .clang-format            # Code style configuration
├── .gitignore               # Git ignore rules
└── idf_component.yml        # Component metadata
```

---

## 🔌 Adding New Components/Libraries

### If Library Exists (GitHub/External Source)

1. **Clone to components/**:
   ```bash
   cd components
   git clone <library-repo> <library-name>
   cd ..
   ```

2. **Verify structure** matches expected layout:
   - `components/<library-name>/CMakeLists.txt`
   - `components/<library-name>/include/`
   - `components/<library-name>/src/`

3. **Reference in main CMakeLists.txt**:
   ```cmake
   idf_component_register(
       SRCS main.cpp
       INCLUDE_DIRS .
       REQUIRES <library-name>
   )
   ```

### If Library Doesn't Exist (Create from Scratch)

1. **Create component structure**:
   ```bash
   mkdir -p components/my-component/{include,src,example}
   ```

2. **Create CMakeLists.txt**:
   ```cmake
   idf_component_register(
       SRCS "src/my_component.cpp"
       INCLUDE_DIRS "include"
       REQUIRES esp-idf
   )
   ```

3. **Create header** (`include/my_component.hpp`):
   ```cpp
   #pragma once
   #include "esp_err.h"

   class MyComponent {
   public:
       esp_err_t init();
       esp_err_t read();
   };
   ```

4. **Create implementation** (`src/my_component.cpp`)

5. **Create README.md** documenting the component

---

## 🎨 Code Style & Formatting

### Using clang-format

All code must follow the project's clang-format configuration (`.clang-format`):

```bash
# Format a single file
clang-format -i src/file.cpp

# Format all C++ files
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Check formatting without modifying
clang-format --dry-run src/file.cpp
```

### Style Configuration

```yaml
BasedOnStyle: LLVM
ColumnLimit: 100
UseTab: Never
IndentWidth: 2
TabWidth: 2
BreakBeforeBraces: Attach
CommentPragmas: "^"
SortIncludes: false
```

**Key Rules**:
- 2-space indentation (no tabs)
- Max line length: 100 characters
- LLVM-style bracing: `} else {` on same line
- No automatic include sorting (preserve existing order with `// clang-format off/on`)

### Before Committing

Always run formatter:
```bash
clang-format -i <modified-files>
```

---

## 🧪 Building & Flashing

### Build Project
```bash
# Activate IDF environment first
source ~/esp/v5.5.1/esp-idf/export.sh

# Build
idf.py build

# Build specific component
idf.py build --component <component-name>

# Clean build
idf.py fullclean && idf.py build
```

### Flash to Device

**For ESP32-C5-MINI-1**:
```bash
# Auto-detect port and flash
idf.py flash

# Or specify port
idf.py -p /dev/ttyUSB0 flash

# Flash with monitor
idf.py -p /dev/ttyUSB0 flash monitor
```

### Monitor Serial Output
```bash
idf.py monitor
idf.py -p /dev/ttyUSB0 monitor
```

---

## 🔐 Partition Table (OTA Support)

The project uses **partitions.csv** with OTA (Over-The-Air) update support:

```
# Name,   Type, SubType, Offset,  Size
nvs,      data, nvs,     0x9000,  0x5000
ota_0,    app,  ota_0,   0xe000,  0x200000
ota_1,    app,  ota_1,   0x20e000, 0x200000
ramdisk,  data, 0x40,    0x40e000, 0x1f0000
```

**Partition Details**:
- **NVS** (4KB): Non-volatile storage for settings
- **OTA_0** (2MB): Current active app partition
- **OTA_1** (2MB): Standby app partition for OTA updates
- **RAMDISK** (1.9MB): Data storage (NAND flash via W25N512)

### Using OTA
```cpp
#include "esp_ota_ops.h"

// Get current partition info
const esp_partition_t *running = esp_ota_get_running_partition();
```

---

## 🚀 Driver Development Workflow

### 1. Define Driver Interface

All drivers inherit from `IDriver`:
```cpp
namespace drivers {
  class MyDriver : public IDriver {
  public:
    esp_err_t init() override;
    esp_err_t start() override;
    esp_err_t read(DriverData* data) override;
    esp_err_t sleep() override;
    esp_err_t wake() override;
    esp_err_t deinit() override;
  };
}
```

### 2. Implement in Component

Create in `components/my-driver/` with proper CMake configuration.

### 3. Reference Hardware Map

Consult [HARDWARE_MAP.md](HARDWARE_MAP.md) for GPIO pins and peripheral connections.

### 4. Test with Unit Tests

Place tests in `components/<driver>/test/` and run via `idf.py test`.

---

## 🐛 Debugging Tips

### Enable Debug Logging
```cpp
#include "esp_log.h"

static const char *TAG = "MY_MODULE";

ESP_LOGI(TAG, "Debug message: %d", value);
ESP_LOGW(TAG, "Warning");
ESP_LOGE(TAG, "Error");
```

### Check RAM/Flash Usage
```bash
idf.py size
idf.py size-components
```

### Monitor Power Consumption
See [POWER_STATES.md](POWER_STATES.md) for power management details.

---

## ✅ Checklist Before Commit

- [ ] Code formatted with `clang-format`
- [ ] Follows `IDriver` interface if it's a driver
- [ ] Component has proper `CMakeLists.txt`
- [ ] No unused `#include` statements
- [ ] ESP log tags defined (`static const char *TAG = "..."`)
- [ ] All error returns checked (`ESP_ERROR_CHECK`)
- [ ] Updated relevant documentation
- [ ] Tested on actual hardware (or emulator)

---

## 📞 Key Contacts & Resources

- **ESP-IDF Documentation**: https://docs.espressif.com/projects/esp-idf/
- **LVGL Documentation**: https://docs.lvgl.io/
- **Component Registry**: https://components.espressif.com/

---

## 🤖 GitHub Copilot Communication Guidelines

### Language & Style Settings

**For GitHub Copilot (and any AI assistant on this project):**

1. **Response Language**: Always respond in **Thai** (ภาษาไทย)
   - User messages may be in Thai or English
   - Always reply in Thai for clarity
   - Technical terms can remain in English if commonly used

2. **Code Comments**: Follow this pattern:
   - **Function block comments**: English
   - **Inline code comments**: English  
   - **Commit messages**: English
   - **Documentation & error messages**: Thai is acceptable if user prefers

3. **Example Style**:
   ```cpp
   // Function block comment in English
   // Initialize I2C bus with configured SCL/SDA pins.
   // Returns ESP_OK on success, ESP_FAIL on error.
   esp_err_t i2c_init(void) {
     // Inline comments in English
     i2c_config_t conf = {};
     conf.mode = I2C_MODE_MASTER;
     // ...
     return esp_idf_error;
   }
   ```

4. **When Requesting Code Review**:
   - Ask in Thai: "ช่วยตรวจสอบ code หน่อย"
   - AI will respond in Thai with findings
   - Code formatting follows `.clang-format` rules

### Git Commit Message Format

```
[type]: brief description in English

Optional detailed explanation in English or Thai.
```

Examples:
- `[feat]: Add I2C initialization for sensors`
- `[fix]: Correct CO2 averaging calculation`
- `[docs]: Update AGENT.md with communication guidelines`

---

- [DRIVER_CONTRACTS.md](ARCHITECTURE/DRIVER_CONTRACTS.md) - Driver standards
- [HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md) - Pinout reference
- [FIRMWARE_PLAN.md](ARCHITECTURE/FIRMWARE_PLAN.md) - Development roadmap
- [POWER_STATES.md](ARCHITECTURE/POWER_STATES.md) - Power management
- [CODE_REVIEW.md](CODE_REVIEW.md) - Code review checklist & patterns
- [AI_GUIDELINES_TH.md](AI_GUIDELINES_TH.md) - **AI communication guidelines (ไทย)**

---

**Last Updated**: January 10, 2026  
**Version**: 1.0
