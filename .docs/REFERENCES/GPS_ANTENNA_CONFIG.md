# GPS TAU1113 Antenna Configuration Guide

## 📡 Overview

GPS module TAU1113 supports both **Passive Antenna** and **Active Antenna** according to the datasheet, with the following differences:

---

## 🔌 Passive Antenna (Default)

### Features
- **Uses built-in LNA** with internal bias
- **No external power required** (ANT_BIAS disabled)
- **Direct RF_IN (50Ω) connection** without 82 nH inductor
- **Suitable for**:
  - Patch antenna on PCB
  - Whip antenna
  - Short cable runs
  - Good signal environments

### Configuration
```cpp
GPS gps;
gps.init();
// Default is Passive - no need to call set_antenna_type()
```

### Log Output
```
I (xxx) gps: GPS UART initialized (baud=9600, TX=11, RX=12)
I (xxx) gps: Antenna mode: PASSIVE (ANT_BIAS: disabled)
I (xxx) gps: GPS status=FIX fix_q=1 sats=8 time=12:34 ant=PASSIVE lat=13.7563 lon=100.5018
```

---

## 📶 Active Antenna

### Features
- **Uses ANT_BIAS to power** external LNA (inside the antenna)
- **Maximum current 25 mA**
- **Includes short/open circuit detection**:
  - `ANT_OK` - Antenna working normally
  - `ANT_OPEN` - Antenna disconnected/broken
  - `ANT_SHORT` - Antenna short circuit
- **Suitable for**:
  - Long cable runs (> 1 meter)
  - Poor signal environments
  - Enclosed/shielded installations
  - Applications requiring maximum signal stability

### Configuration
```cpp
GPS gps;
gps.init();

// Enable active antenna with ANT_BIAS
esp_err_t ret = gps.set_antenna_type(GPS::AntennaType::Active);
if (ret == ESP_OK) {
    ESP_LOGI(TAG, "Active antenna enabled");
}
```

### Log Output
```
I (xxx) gps: GPS UART initialized (baud=9600, TX=11, RX=12)
I (xxx) gps: Antenna mode: PASSIVE (ANT_BIAS: disabled)
I (xxx) gps: Configuring ACTIVE antenna (with ANT_BIAS)
W (xxx) gps: Active antenna mode set, but hardware control not available
W (xxx) gps: ANT_BIAS control must be handled externally or via hardware jumper
I (xxx) gps: GPS status=FIX fix_q=1 sats=8 time=12:34 ant=ACTIVE(OK) lat=13.7563 lon=100.5018
```

**Note**: If hardware supports GPIO ANT_BIAS control, you will see:
```
I (xxx) gps: ANT_BIAS enabled on GPIO13 (max 25 mA)
I (xxx) gps: Active antenna status detection enabled (ANT_OK/ANT_OPEN/ANT_SHORT)
```

---

## ⚙️ Hardware Configuration

### Current Implementation
Currently `kAntBiasPin` is set to `GPIO_NUM_NC` (Not Connected) because:
- No clear GPIO information available from hardware schematic yet
- May use hardware jumper for enabling/disabling ANT_BIAS

### Configuring for Hardware with GPIO Control

Edit in [`main/gps.cpp`](../../main/gps.cpp):

```cpp
// Current (placeholder):
static constexpr gpio_num_t kAntBiasPin = GPIO_NUM_NC;
static constexpr bool kAntBiasHasHardwareControl = false;

// Update to actual GPIO:
static constexpr gpio_num_t kAntBiasPin = GPIO_NUM_13;  // Example
static constexpr bool kAntBiasHasHardwareControl = true;
```

---

## 🔍 Antenna Status Detection

When using **Active Antenna**, TAU1113 module monitors antenna status and reports via NMEA messages:

### Status Values
| Status | Meaning | Action |
|--------|---------|--------|
| `ANT_OK` | Working normally | - |
| `ANT_OPEN` | Disconnected/broken | Check connection |
| `ANT_SHORT` | Short circuit | Check cable and antenna |
| `UNKNOWN` | No data received | Check NMEA output |

### Reading Status
```cpp
GPS::AntennaStatus status = gps.antenna_status();
switch (status) {
    case GPS::AntennaStatus::Ok:
        ESP_LOGI(TAG, "Antenna OK");
        break;
    case GPS::AntennaStatus::Open:
        ESP_LOGW(TAG, "Antenna open/disconnected!");
        break;
    case GPS::AntennaStatus::Short:
        ESP_LOGE(TAG, "Antenna short circuit!");
        break;
    case GPS::AntennaStatus::Unknown:
        ESP_LOGD(TAG, "Antenna status unknown");
        break;
}
```

---

## 🎯 Design Recommendations

### Passive Antenna - Recommended When
✅ Using patch antenna on PCB  
✅ Outdoor installation with clear sky view  
✅ Short cable runs (< 50 cm)  
✅ Good signal environment  
✅ Power efficiency is important  

### Active Antenna - Recommended When
✅ Long cable runs (> 1 meter)  
✅ Enclosed metal/shielded installations  
✅ Poor signal environments (buildings/underground)  
✅ Maximum stability required  
✅ Need short/open detection  

---

## ⚡ Power Consumption

### Passive Antenna
- **GPS Module Only**: ~35 mA (typical)
- **ANT_BIAS**: 0 mA (disabled)
- **Total**: ~35 mA

### Active Antenna
- **GPS Module**: ~35 mA (typical)
- **ANT_BIAS (LNA)**: 5-25 mA (depends on antenna)
- **Total**: ~40-60 mA

---

## 🔧 API Reference

### GPS Class Methods

#### `set_antenna_type(AntennaType type)`
Configure antenna type and enable/disable ANT_BIAS

**Parameters:**
- `type`: `GPS::AntennaType::Passive` or `GPS::AntennaType::Active`

**Returns:**
- `ESP_OK` - Success
- `ESP_ERR_INVALID_STATE` - `init()` not called yet
- `ESP_FAIL` - GPIO configuration failed

**Example:**
```cpp
GPS gps;
gps.init();
gps.set_antenna_type(GPS::AntennaType::Active);
```

#### `get_antenna_type()`
Get current configured antenna type

**Returns:**
- `GPS::AntennaType::Passive` or `GPS::AntennaType::Active`

**Example:**
```cpp
GPS::AntennaType type = gps.get_antenna_type();
if (type == GPS::AntennaType::Active) {
    ESP_LOGI(TAG, "Using active antenna");
}
```

#### `antenna_status()`
Read antenna status (Active Antenna only)

**Returns:**
- `GPS::AntennaStatus::Ok` - Working normally
- `GPS::AntennaStatus::Open` - Disconnected
- `GPS::AntennaStatus::Short` - Short circuit
- `GPS::AntennaStatus::Unknown` - Status unknown

---

## 📝 Example: Runtime Antenna Switching

```cpp
#include "gps.h"
#include "esp_log.h"

static const char *TAG = "gps_example";
GPS gps;

void setup_gps() {
    // Initialize GPS
    esp_err_t ret = gps.init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPS init failed: %s", esp_err_to_name(ret));
        return;
    }

    // Start with passive antenna (default)
    ESP_LOGI(TAG, "GPS initialized with passive antenna");

    // Later, switch to active if needed
    ret = gps.set_antenna_type(GPS::AntennaType::Active);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Switched to active antenna");
    }
}

void monitor_gps() {
    uint64_t now_ms = esp_timer_get_time() / 1000;
    
    // Update GPS parser
    gps.update(now_ms);
    
    // Check antenna status (if active antenna)
    if (gps.get_antenna_type() == GPS::AntennaType::Active) {
        GPS::AntennaStatus status = gps.antenna_status();
        if (status == GPS::AntennaStatus::Open) {
            ESP_LOGW(TAG, "WARNING: Active antenna disconnected!");
        } else if (status == GPS::AntennaStatus::Short) {
            ESP_LOGE(TAG, "ERROR: Active antenna short circuit!");
        }
    }
    
    // Log GPS status (every 5 seconds)
    gps.log_status(now_ms, 3000, 5000);
}
```

---

## 🔬 Testing Antenna Configuration

### Test 1: Passive Antenna (Default)
```
1. Flash firmware (default configuration)
2. Monitor serial output
3. Verify log shows: "Antenna mode: PASSIVE"
4. Verify GPS fix is obtained
5. Check signal quality (satellites count)
```

### Test 2: Active Antenna
```
1. Call set_antenna_type(GPS::AntennaType::Active)
2. Monitor serial output
3. Verify log shows: "Antenna mode: ACTIVE"
4. Check antenna status: ANT_OK/ANT_OPEN/ANT_SHORT
5. Verify improved signal in weak conditions
```

### Test 3: Antenna Detection (Active Only)
```
1. Configure active antenna
2. Disconnect antenna cable
3. Verify log shows: ant=ACTIVE(OPEN)
4. Reconnect antenna
5. Verify log shows: ant=ACTIVE(OK)
```

---

## 📚 References

- TAU1113 Datasheet: `.docs/REFERENCES/pdf/T-5-2208-TAU1113 Datasheet-V1.4.pdf`
- NMEA Protocol: NMEA 0183 standard
- GPS Driver: [`main/gps.h`](../../main/gps.h), [`main/gps.cpp`](../../main/gps.cpp)

---

**Last Updated**: January 26, 2026  
**Version**: 1.0
