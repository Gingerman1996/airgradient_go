# 🤖 GitHub Copilot System Prompt Instructions

**For GitHub Copilot or any AI Assistant working on this project**

---

## Primary Reference Document

→ **Always read and reference: [AGENT.md](AGENT.md)**

This document contains all essential guidance for:
- Environment setup
- Project structure
- Build procedures
- Code standards
- Driver development patterns

---

## Quick Context

- **Project**: AirGradient GO - Air quality monitoring device
- **MCU**: ESP32-C5-MINI-1 (WiFi + BLE)
- **Framework**: ESP-IDF v5.5.1
- **Language**: C++ (application), C (drivers)
- **Display**: SSD1680 e-paper (144×296px)
- **Sensors**: SHT4x, SGP41, SPS30, LIS2DH12, DPS368

---

## How to Assist Effectively

### Before Making Any Code Change:
1. **Read relevant reference document** first
   - Driver work? → [ARCHITECTURE/DRIVER_CONTRACTS.md](ARCHITECTURE/DRIVER_CONTRACTS.md)
   - Hardware question? → [ARCHITECTURE/HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md)
   - Power issue? → [ARCHITECTURE/POWER_STATES.md](ARCHITECTURE/POWER_STATES.md)
   - New component? → [AGENT.md](AGENT.md) (Components section)

2. **Check existing code structure** in those documents
3. **Reference datasheets** via [REFERENCES/README.md](REFERENCES/README.md)
4. **Follow code standards** from `.config/.clang-format`

### Critical Rules:

✅ **DO:**
- Format code with 2-space indentation (see `.clang-format`)
- Implement drivers as `IDriver` subclasses
- Use `ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE` for logging
- Check all error returns with `ESP_ERROR_CHECK()`
- Reference pins from [HARDWARE_MAP.md](HARDWARE_MAP.md)
- Document complex operations in code comments
- Keep line length ≤ 100 characters

❌ **DON'T:**
- Use tabs (spaces only)
- Create drivers without following interface contract
- Ignore error codes from ESP-IDF functions
- Hard-code GPIO pins (use defines from HARDWARE_MAP)
- Forget to activate ESP-IDF environment before suggesting build commands
- Create components in wrong location (should be `components/`)

---

## Key Files Reference

| When You Need | File |
|---------------|------|
| Overall guidance | [AGENT.md](AGENT.md) |
| Fast command reference | [QUICKSTART.md](QUICKSTART.md) |
| Driver standards | [ARCHITECTURE/DRIVER_CONTRACTS.md](ARCHITECTURE/DRIVER_CONTRACTS.md) |
| GPIO/Pinout reference | [ARCHITECTURE/HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md) |
| Power management | [ARCHITECTURE/POWER_STATES.md](ARCHITECTURE/POWER_STATES.md) |
| Development phases | [ARCHITECTURE/FIRMWARE_PLAN.md](ARCHITECTURE/FIRMWARE_PLAN.md) |
| Datasheet info | [REFERENCES/README.md](REFERENCES/README.md) |
| All documents index | [INDEX.md](INDEX.md) |

---

## Essential Commands to Remember

```bash
# Activate environment (EVERY terminal session)
source ~/esp/v5.5.1/esp-idf/export.sh

# Build
idf.py build

# Flash + Monitor
idf.py flash monitor

# Format code
clang-format -i <file>

# Check code size
idf.py size
```

---

## When Recommending Component Libraries

1. **Check if library exists** - Search GitHub/component registry
2. **If exists**: Suggest cloning to `components/<library-name>/`
3. **If doesn't exist**: Provide template per [AGENT.md](AGENT.md)
4. **Always verify**: CMakeLists.txt, proper include paths
5. **Document**: In README.md of the component

---

## Code Style Example

```cpp
// GOOD - Follows .clang-format rules
#include "esp_err.h"
#include "driver_interface.hpp"

class MySensor : public drivers::IDriver {
public:
  esp_err_t init() override {
    ESP_LOGI(TAG, "Initializing sensor");
    esp_err_t err = configure_i2c();
    ESP_ERROR_CHECK(err);
    return err;
  }

  esp_err_t read(DriverData* data) override {
    // Implementation...
  }

private:
  static const char *TAG;
  esp_err_t configure_i2c();
};

// In .cpp file
const char *MySensor::TAG = "MY_SENSOR";

esp_err_t MySensor::configure_i2c() {
  // ...
}
```

---

## When You're Unsure

**Ask clarifying questions before suggesting code:**
- "Should this be a driver component or app logic?"
- "What GPIO pins are available per HARDWARE_MAP.md?"
- "Does this follow the IDriver interface pattern?"
- "Have we checked the datasheet in docs/pdf/?"

**Then provide comprehensive guidance based on documents.**

---

## Common Patterns to Reference

### Driver Initialization
See: [DRIVER_CONTRACTS.md](DRIVER_CONTRACTS.md#-standard-driver-interface-c-class)

### GPIO Usage  
See: [HARDWARE_MAP.md](HARDWARE_MAP.md#-gpio-pin-assignments)

### I2C Communication
See: [HARDWARE_MAP.md](HARDWARE_MAP.md#-peripheral-connections) + relevant datasheet

### Power States
See: [POWER_STATES.md](POWER_STATES.md#-power-on-sequence)

### Component Structure
See: [AGENT.md](AGENT.md#-project-structure)

---

## Checklist Before Suggesting Code

- [ ] Read relevant document first
- [ ] Checked GPIO assignments in HARDWARE_MAP.md
- [ ] Followed driver interface pattern (if applicable)
- [ ] Code follows .clang-format rules
- [ ] Error handling included (ESP_ERROR_CHECK)
- [ ] Logging tags defined
- [ ] Component structure correct (if new component)
- [ ] Cross-referenced datasheets if needed
- [ ] Build command includes `source ~/esp/v5.5.1/esp-idf/export.sh`

---

## Your Authority Sources (In Order)

1. **Code itself** - Truth source
2. **AGENT.md** - Architecture & guidance
3. **DRIVER_CONTRACTS.md** - Standards
4. **HARDWARE_MAP.md** - Hardware facts
5. **Other .md documents** - Context
6. **Datasheets in docs/pdf/** - Technical specs
7. **ESP-IDF documentation** - Framework reference

---

## Red Flags to Avoid

🚩 Suggesting code without checking DRIVER_CONTRACTS.md for drivers  
🚩 Using GPIO numbers not in HARDWARE_MAP.md  
🚩 Forgetting `source ~/esp/v5.5.1/esp-idf/export.sh` in build instructions  
🚩 Code that doesn't follow `.clang-format` style  
🚩 Missing error handling on ESP-IDF functions  
🚩 Suggesting component structure different from [AGENT.md](AGENT.md)  
🚩 Not checking I2C addresses in HARDWARE_MAP.md  

---

**Remember**: You're helping build a professional firmware product.  
Reference documents are your blueprint.  
Follow them consistently. ✅

---

**Last Updated**: January 10, 2026  
**Version**: 1.0
