# AirGradient GO - Driver Contracts

## 📋 Overview

> **⚠️ FUTURE ARCHITECTURE**: This document describes the **target** unified driver interface for all hardware components. Current sensor implementations (`stcc4`, `sgp4x`, `sps30`) use component-specific APIs and will be refactored to this standard interface in a future phase.

> **Current Status (Jan 13, 2026)**:
> - ✅ Sensor drivers working with component-specific APIs
> - ✅ I2C bus shared between sensors
> - ✅ **STCC4**: Continuous measurement mode (1s interval)
> - ✅ **SPS30**: Continuous measurement mode (1s interval)
> - ✅ **SGP4x**: Continuous sampling (1s interval)
> - ✅ **DPS368**: Pressure reading every 5 seconds
> - ✅ **GPS**: Status logging every 5 seconds
> - ⏳ Unified driver interface **NOT YET IMPLEMENTED**
> - 📋 This document serves as design specification for future refactoring

All drivers in this project **should** follow a unified API contract to ensure consistency, maintainability, and ease of testing. This document defines the standard interface that all hardware drivers **will** implement.

**SDK**: ESP-IDF (Espressif IoT Development Framework)
**Language**: C++ (main application), C-compatible drivers

---

## 🔧 Standard Driver Interface (C++ Class)

Every driver MUST implement the `IDriver` interface:

```cpp
// driver_interface.hpp
#pragma once

#include "esp_err.h"
#include <cstdint>

namespace drivers {

// Driver state enumeration
enum class DriverState {
    UNINITIALIZED,
    INITIALIZED,
    ACTIVE,
    SLEEPING,
    ERROR
};

// Base data structure for all readings
struct DriverData {
    bool valid = false;
    int64_t timestamp = 0;  // esp_timer_get_time()
};

// Abstract driver interface
class IDriver {
public:
    virtual ~IDriver() = default;
    
    virtual esp_err_t init() = 0;
    virtual esp_err_t start() = 0;
    virtual esp_err_t read(DriverData* data) = 0;
    virtual esp_err_t sleep() = 0;
    virtual esp_err_t wake() = 0;
    virtual esp_err_t deinit() = 0;
    
    virtual DriverState getState() const = 0;
    virtual const char* getName() const = 0;
};

} // namespace drivers
```

---

## 📐 Base Driver Class

All drivers should inherit from the base driver class:

```cpp
// driver_base.hpp
#pragma once

#include "driver_interface.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/i2c_master.h"

namespace drivers {

struct I2CConfig {
    i2c_master_bus_handle_t bus_handle;
    uint8_t device_addr;
    SemaphoreHandle_t mutex;
    uint32_t timeout_ms = 100;
};

class DriverBase : public IDriver {
protected:
    DriverState m_state = DriverState::UNINITIALIZED;
    I2CConfig m_i2c_config;
    i2c_master_dev_handle_t m_dev_handle = nullptr;
    const char* m_name;
    
    // I2C helper with automatic mutex handling
    esp_err_t i2cWrite(const uint8_t* data, size_t len);
    esp_err_t i2cRead(uint8_t* data, size_t len);
    esp_err_t i2cWriteRead(const uint8_t* write_data, size_t write_len,
                           uint8_t* read_data, size_t read_len);
    
    // Mutex helpers
    bool acquireMutex(uint32_t timeout_ms = 1000);
    void releaseMutex();
    
public:
    explicit DriverBase(const char* name) : m_name(name) {}
    virtual ~DriverBase() = default;
    
    DriverState getState() const override { return m_state; }
    const char* getName() const override { return m_name; }
};

} // namespace drivers
```

### Base Driver Implementation

```cpp
// driver_base.cpp
#include "driver_base.hpp"
#include "esp_log.h"

namespace drivers {

bool DriverBase::acquireMutex(uint32_t timeout_ms) {
    if (m_i2c_config.mutex == nullptr) return true;
    return xSemaphoreTake(m_i2c_config.mutex, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void DriverBase::releaseMutex() {
    if (m_i2c_config.mutex != nullptr) {
        xSemaphoreGive(m_i2c_config.mutex);
    }
}

esp_err_t DriverBase::i2cWrite(const uint8_t* data, size_t len) {
    if (!acquireMutex()) return ESP_ERR_TIMEOUT;
    
    esp_err_t ret = i2c_master_transmit(m_dev_handle, data, len, 
                                         pdMS_TO_TICKS(m_i2c_config.timeout_ms));
    releaseMutex();
    return ret;
}

esp_err_t DriverBase::i2cRead(uint8_t* data, size_t len) {
    if (!acquireMutex()) return ESP_ERR_TIMEOUT;
    
    esp_err_t ret = i2c_master_receive(m_dev_handle, data, len,
                                        pdMS_TO_TICKS(m_i2c_config.timeout_ms));
    releaseMutex();
    return ret;
}

esp_err_t DriverBase::i2cWriteRead(const uint8_t* write_data, size_t write_len,
                                    uint8_t* read_data, size_t read_len) {
    if (!acquireMutex()) return ESP_ERR_TIMEOUT;
    
    esp_err_t ret = i2c_master_transmit_receive(m_dev_handle, 
                                                 write_data, write_len,
                                                 read_data, read_len,
                                                 pdMS_TO_TICKS(m_i2c_config.timeout_ms));
    releaseMutex();
    return ret;
}

} // namespace drivers
```

---

## 📐 Function Specifications

### `init()`

**Purpose**: Initialize the driver with configuration parameters.

| Aspect | Specification |
|--------|---------------|
| **When to call** | Once at boot, during Phase 2/3 of boot sequence |
| **Thread safety** | NOT thread-safe, call only from main task during init |
| **I2C mutex** | Handled internally by base class |
| **Timeout** | Max 1000ms for I2C probe/init |
| **Return** | `ESP_OK` on success, error code on failure |
| **State** | UNINITIALIZED → INITIALIZED |

---

### `start()`

**Purpose**: Start continuous operation or periodic measurements.

| Aspect | Specification |
|--------|---------------|
| **When to call** | After successful `init()`, when ready to take measurements |
| **Thread safety** | Thread-safe after init |
| **Timeout** | Max 500ms |
| **Return** | `ESP_OK` on success |
| **State** | INITIALIZED → ACTIVE |

---

### `read(data)`

**Purpose**: Read current sensor values or device status.

| Aspect | Specification |
|--------|---------------|
| **When to call** | Periodically or on-demand after `start()` |
| **Thread safety** | Thread-safe, can be called from any task |
| **Timeout** | Max 500ms for I2C read |
| **Return** | `ESP_OK` with valid data, error code if failed |
| **State** | Must be in ACTIVE state |

---

### `sleep()`

**Purpose**: Put device into low-power sleep mode.

| Aspect | Specification |
|--------|---------------|
| **When to call** | Before system sleep or to save power |
| **Thread safety** | Thread-safe |
| **Timeout** | Max 200ms |
| **Return** | `ESP_OK` on success |
| **State** | ACTIVE → SLEEPING |

---

### `wake()`

**Purpose**: Wake device from sleep mode, restore to active state.

| Aspect | Specification |
|--------|---------------|
| **When to call** | After system wake or when readings needed again |
| **Thread safety** | Thread-safe |
| **Timeout** | Max 500ms (some sensors need warm-up) |
| **Return** | `ESP_OK` on success |
| **State** | SLEEPING → ACTIVE |

---

### `deinit()`

**Purpose**: Clean up resources, prepare for shutdown.

| Aspect | Specification |
|--------|---------------|
| **When to call** | Before system shutdown or driver removal |
| **Thread safety** | NOT thread-safe, ensure no other tasks using driver |
| **Timeout** | Max 200ms |
| **Return** | `ESP_OK` on success |
| **State** | Any → UNINITIALIZED |

---

## ⚠️ Error Codes

All drivers use standard ESP-IDF error codes plus custom driver errors:

### Standard ESP-IDF Errors
| Error Code | Value | Meaning |
|------------|-------|---------|
| `ESP_OK` | 0 | Success |
| `ESP_FAIL` | -1 | General failure |
| `ESP_ERR_NO_MEM` | 0x101 | Out of memory |
| `ESP_ERR_INVALID_ARG` | 0x102 | Invalid argument |
| `ESP_ERR_INVALID_STATE` | 0x103 | Invalid state (e.g., not initialized) |
| `ESP_ERR_NOT_FOUND` | 0x105 | Device not found on I2C bus |
| `ESP_ERR_TIMEOUT` | 0x107 | Operation timed out |

### Custom Driver Errors (0x2000+)

```cpp
// driver_errors.hpp
#pragma once

namespace drivers {

constexpr int DRV_ERR_BASE        = 0x2000;
constexpr int DRV_ERR_NOT_INIT    = DRV_ERR_BASE + 1;
constexpr int DRV_ERR_BUSY        = DRV_ERR_BASE + 2;
constexpr int DRV_ERR_CRC         = DRV_ERR_BASE + 3;
constexpr int DRV_ERR_WARMUP      = DRV_ERR_BASE + 4;
constexpr int DRV_ERR_CALIBRATION = DRV_ERR_BASE + 5;
constexpr int DRV_ERR_RANGE       = DRV_ERR_BASE + 6;
constexpr int DRV_ERR_HW_FAULT    = DRV_ERR_BASE + 7;

} // namespace drivers
```

| Error Code | Value | Meaning |
|------------|-------|---------|
| `DRV_ERR_NOT_INIT` | 0x2001 | Driver not initialized |
| `DRV_ERR_BUSY` | 0x2002 | Device busy, try again |
| `DRV_ERR_CRC` | 0x2003 | CRC/checksum mismatch |
| `DRV_ERR_WARMUP` | 0x2004 | Sensor warming up, data not ready |
| `DRV_ERR_CALIBRATION` | 0x2005 | Calibration required or failed |
| `DRV_ERR_RANGE` | 0x2006 | Reading out of valid range |
| `DRV_ERR_HW_FAULT` | 0x2007 | Hardware fault detected |

---

## ⏱️ Timeout Specifications

### I2C Operation Timeouts
| Operation | Timeout | Notes |
|-----------|---------|-------|
| I2C probe | 100ms | Check if device exists |
| I2C write | 100ms | Single register write |
| I2C read | 100ms | Single register read |
| Mutex acquire | 1000ms | Wait for I2C bus |

### Sensor-Specific Timeouts
| Sensor | Measurement Time | Warm-up Time |
|--------|------------------|--------------|
| STCC4 (CO2) | 5000ms | 30s |
| SCD41 (CO2) | 5000ms | 30s |
| SHT40 (T/RH) | 10ms | None |
| SGP41 (VOC/NOx) | 50ms | 10s (conditioning) |
| DPS368 (Pressure) | 40ms | None |
| SPS30 (PM) | 1000ms | 30s |
| PMSA003I (PM) | 1000ms | 30s |
| LIS2DH12 (Accel) | 10ms | None |

---

## 🔒 I2C Mutex Management

### Mutex Rules

1. **Single shared mutex** for all I2C devices on the same bus
2. **Mutex handled by base class** - drivers don't manage directly
3. **Always released** after I2C operation, even on error
4. **Use timeout** when acquiring mutex (max 1000ms)
5. **RAII pattern** preferred for exception safety

### RAII Mutex Lock

```cpp
// mutex_lock.hpp
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace drivers {

class MutexLock {
    SemaphoreHandle_t m_mutex;
    bool m_acquired = false;
    
public:
    explicit MutexLock(SemaphoreHandle_t mutex, uint32_t timeout_ms = 1000) 
        : m_mutex(mutex) {
        if (mutex != nullptr) {
            m_acquired = xSemaphoreTake(mutex, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
        }
    }
    
    ~MutexLock() {
        if (m_acquired && m_mutex != nullptr) {
            xSemaphoreGive(m_mutex);
        }
    }
    
    bool isAcquired() const { return m_acquired; }
    
    // Non-copyable
    MutexLock(const MutexLock&) = delete;
    MutexLock& operator=(const MutexLock&) = delete;
};

} // namespace drivers
```

### Mutex Creation (in main.cpp)

```cpp
// main.cpp
extern "C" void app_main() {
    // Create shared I2C mutex
    SemaphoreHandle_t i2c_mutex = xSemaphoreCreateMutex();
    assert(i2c_mutex != nullptr);
    
    // Create I2C bus
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_7,
        .scl_io_num = GPIO_NUM_6,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = { .enable_internal_pullup = true }
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));
    
    // Initialize drivers
    drivers::I2CConfig i2c_config = {
        .bus_handle = bus_handle,
        .device_addr = 0x64,
        .mutex = i2c_mutex
    };
    
    auto co2_sensor = std::make_unique<drivers::CO2SensorSTCC4>(i2c_config);
    ESP_ERROR_CHECK(co2_sensor->init());
    ESP_ERROR_CHECK(co2_sensor->start());
}
```

---

## 📦 Sensor Data Structures

```cpp
// sensor_data.hpp
#pragma once

#include "driver_interface.hpp"

namespace drivers {

// CO2 Sensor (STCC4/SCD41)
struct CO2SensorData : public DriverData {
    uint16_t co2_ppm = 0;        // 0-40000 ppm
    float temperature_c = 0;     // -40 to 85 °C
    float humidity_percent = 0;  // 0-100 %
};

// VOC/NOx Sensor (SGP41)
struct VOCSensorData : public DriverData {
    uint16_t voc_index = 0;      // 0-500
    uint16_t nox_index = 0;      // 0-500
    uint16_t voc_raw = 0;        // Raw ticks
    uint16_t nox_raw = 0;        // Raw ticks
};

// PM Sensor (SPS30/PMSA003I)
struct PMSensorData : public DriverData {
    float pm1_0 = 0;             // µg/m³
    float pm2_5 = 0;             // µg/m³
    float pm10 = 0;              // µg/m³
};

// Temperature/Humidity (SHT40)
struct TempRHData : public DriverData {
    float temperature_c = 0;
    float humidity_percent = 0;
};

// Pressure (DPS368)
struct PressureData : public DriverData {
    float pressure_hpa = 0;
    float temperature_c = 0;
};

// Accelerometer (LIS2DH12)
struct AccelData : public DriverData {
    int16_t x_mg = 0;            // milli-g
    int16_t y_mg = 0;
    int16_t z_mg = 0;
};

// Charger (BQ25628)
struct ChargerData : public DriverData {
    uint16_t battery_mv = 0;     // Battery voltage in mV
    uint8_t battery_percent = 0; // Estimated SOC
    bool is_charging = false;
    bool usb_connected = false;
    uint8_t fault_status = 0;
};

} // namespace drivers
```

---

## 🎯 Example Driver Implementation

```cpp
// co2_stcc4.hpp
#pragma once

#include "driver_base.hpp"
#include "sensor_data.hpp"

namespace drivers {

class CO2SensorSTCC4 : public DriverBase {
    static constexpr uint8_t DEFAULT_ADDR = 0x64;
    
public:
    explicit CO2SensorSTCC4(const I2CConfig& config);
    
    esp_err_t init() override;
    esp_err_t start() override;
    esp_err_t read(DriverData* data) override;
    esp_err_t sleep() override;
    esp_err_t wake() override;
    esp_err_t deinit() override;
    
    // Sensor-specific methods
    esp_err_t readCO2(CO2SensorData& data);
    esp_err_t forceRecalibration(uint16_t target_ppm);
};

} // namespace drivers
```

```cpp
// co2_stcc4.cpp
#include "co2_stcc4.hpp"
#include "esp_log.h"
#include "esp_timer.h"

static const char* TAG = "CO2_STCC4";

namespace drivers {

CO2SensorSTCC4::CO2SensorSTCC4(const I2CConfig& config) 
    : DriverBase("STCC4") {
    m_i2c_config = config;
    if (m_i2c_config.device_addr == 0) {
        m_i2c_config.device_addr = DEFAULT_ADDR;
    }
}

esp_err_t CO2SensorSTCC4::init() {
    if (m_state != DriverState::UNINITIALIZED) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Add device to I2C bus
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = m_i2c_config.device_addr,
        .scl_speed_hz = 100000,
    };
    
    esp_err_t ret = i2c_master_bus_add_device(
        m_i2c_config.bus_handle, &dev_config, &m_dev_handle);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device: %s", esp_err_to_name(ret));
        m_state = DriverState::ERROR;
        return ret;
    }
    
    // Probe device
    uint8_t cmd[] = {0x36, 0x82};  // Read serial number command
    uint8_t response[9];
    ret = i2cWriteRead(cmd, sizeof(cmd), response, sizeof(response));
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Device not found at 0x%02X", m_i2c_config.device_addr);
        m_state = DriverState::ERROR;
        return ESP_ERR_NOT_FOUND;
    }
    
    ESP_LOGI(TAG, "Initialized at 0x%02X", m_i2c_config.device_addr);
    m_state = DriverState::INITIALIZED;
    return ESP_OK;
}

esp_err_t CO2SensorSTCC4::start() {
    if (m_state != DriverState::INITIALIZED && m_state != DriverState::SLEEPING) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Start periodic measurement
    uint8_t cmd[] = {0x21, 0xB1};
    esp_err_t ret = i2cWrite(cmd, sizeof(cmd));
    
    if (ret == ESP_OK) {
        m_state = DriverState::ACTIVE;
        ESP_LOGI(TAG, "Started periodic measurement");
    }
    return ret;
}

esp_err_t CO2SensorSTCC4::read(DriverData* data) {
    if (m_state != DriverState::ACTIVE) {
        return ESP_ERR_INVALID_STATE;
    }
    
    CO2SensorData* sensor_data = static_cast<CO2SensorData*>(data);
    return readCO2(*sensor_data);
}

esp_err_t CO2SensorSTCC4::readCO2(CO2SensorData& data) {
    uint8_t cmd[] = {0xEC, 0x05};  // Read measurement
    uint8_t response[9];
    
    esp_err_t ret = i2cWriteRead(cmd, sizeof(cmd), response, sizeof(response));
    if (ret != ESP_OK) {
        data.valid = false;
        return ret;
    }
    
    // Parse response (CO2, Temp, RH with CRC)
    data.co2_ppm = (response[0] << 8) | response[1];
    // ... parse temperature and humidity
    
    data.valid = true;
    data.timestamp = esp_timer_get_time();
    
    return ESP_OK;
}

esp_err_t CO2SensorSTCC4::sleep() {
    if (m_state != DriverState::ACTIVE) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t cmd[] = {0x01, 0x04};  // Stop measurement
    esp_err_t ret = i2cWrite(cmd, sizeof(cmd));
    
    if (ret == ESP_OK) {
        m_state = DriverState::SLEEPING;
    }
    return ret;
}

esp_err_t CO2SensorSTCC4::wake() {
    return start();  // Same as start
}

esp_err_t CO2SensorSTCC4::deinit() {
    if (m_state == DriverState::ACTIVE) {
        sleep();
    }
    
    if (m_dev_handle != nullptr) {
        i2c_master_bus_rm_device(m_dev_handle);
        m_dev_handle = nullptr;
    }
    
    m_state = DriverState::UNINITIALIZED;
    return ESP_OK;
}

} // namespace drivers
```

---

## 🎯 Driver Implementation Checklist

For each new driver, ensure:

- [ ] Inherits from `DriverBase`
- [ ] Implements all 6 interface methods
- [ ] Uses `i2cWrite/i2cRead/i2cWriteRead` helpers (mutex handled automatically)
- [ ] Returns proper error codes
- [ ] Respects timeout specifications
- [ ] Defines data structure inheriting from `DriverData`
- [ ] Handles warm-up period (if applicable)
- [ ] Logs errors with `ESP_LOGE`
- [ ] Logs debug info with `ESP_LOGD`
- [ ] Documents I2C address and register map
- [ ] Includes unit tests

---

## 📝 Driver List

> **Updated: Jan 13, 2026** - DPS368 and BQ25629 drivers completed with datasheet-verified formulas

| Driver Class | Device | I2C Addr | Status | Notes |
|--------------|--------|----------|--------|-------|
| `CO2SensorSTCC4` | STCC4 | 0x62 | ✅ | State machine + 5s averaging |
| `CO2SensorSCD41` | SCD41 | 0x62 | ⏳ | Alternative (same addr family) |
| `TempSensorSHT40` | SHT40 | 0x44 | ⏳ | Optional backup |
| `VOCSensorSGP41` | SGP41 | 0x59 | ✅ | Gas Index Algorithm integrated |
| `PMSensorSPS30` | SPS30 | 0x69 | ✅ | EN_PM1 load switch control |
| `PMSensorPMSA003I` | PMSA003I | 0x12 | ⏳ | Alternative PM option |
| `PressureSensorDPS368` | DPS368 | 0x77 | ✅ | Full calibration + 16x oversampling |
| `AccelSensorLIS2DH12` | LIS2DH12 | 0x18 | ⏳ | Hardware ready |
| `ChargerBQ25629` | BQ25629 | 0x6A | ✅ | Ship mode + ADC + log_charger_limits() |
| `ButtonCAP1203` | CAP1203 | 0x28 | ✅ | T1/T2/T3 navigation working |
| `LEDDriverLP5030` | LP5030/36 | 0x30 | ⏳ | Pending Phase 5 |
| `FlashW25N512` | W25N512 | SPI | ⏳ | Pending Phase 2 |

---

## 🔄 State Machine

```
                    ┌─────────────┐
                    │ UNINITIALIZED│
                    └──────┬──────┘
                           │ init()
                           ▼
                    ┌─────────────┐
                    │ INITIALIZED │
                    └──────┬──────┘
                           │ start()
                           ▼
          ┌────────────────────────────────┐
          │                                │
          ▼                                │
    ┌───────────┐                          │
    │  ACTIVE   │◄─────────────────────────┤
    └─────┬─────┘                          │
          │                                │
          │ sleep()              wake()    │
          ▼                                │
    ┌───────────┐                          │
    │  SLEEPING │──────────────────────────┘
    └───────────┘

    Any state ──── deinit() ────► UNINITIALIZED
    
    Error during any operation ────► ERROR
```

---

## 📚 File Structure

```
components/
  └── drivers/
      ├── include/
      │   ├── driver_interface.hpp
      │   ├── driver_base.hpp
      │   ├── driver_errors.hpp
      │   ├── sensor_data.hpp
      │   ├── mutex_lock.hpp
      │   └── [device]_driver.hpp
      ├── src/
      │   ├── driver_base.cpp
      │   └── [device]_driver.cpp
      └── CMakeLists.txt

main/
  ├── main.cpp              # app_main() entry point
  ├── sensor_manager.hpp    # Manages all sensors
  └── sensor_manager.cpp
```
