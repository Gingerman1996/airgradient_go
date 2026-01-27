# E-Paper Display Example

## 📺 Overview

Standalone example demonstrating e-paper display with:
- Random sensor values (PM2.5, CO2, TVOC, NOx, Temperature, Humidity, Pressure)
- All icons visible on dashboard (battery, GPS, sensors)
- Automatic updates every 5 seconds
- LVGL + partial refresh for smooth updates

---

## 🚀 How to Use

### Method 1: Run Example Only (Recommended)

1. **Edit** `main/airgradient-go.cpp` - Rename the main function:
   ```cpp
   // Change line ~109:
   extern "C" void app_main_original(void) {  // Add _original
   ```

2. **Edit** `main/display_example.cpp` - Enable the alias (line ~280):
   ```cpp
   // Uncomment this line:
   extern "C" void app_main(void) __attribute__((alias("app_main_display_example")));
   ```

3. **Build and flash**:
   ```bash
   idf.py build flash monitor
   ```

### Method 2: Run Alongside Main App

Call `app_main_display_example()` from your own code:
```cpp
extern "C" void app_main_display_example(void);

void your_test_function() {
    app_main_display_example();  // Will take over and run forever
}
```

---

## 📊 What It Shows

### Dashboard Layout (144x296 pixels)

```
┌────────────────────────────┐
│  🔋 85% ⚡  📡 12:34  🛰️  │  ← Header (battery + GPS)
├────────────────────────────┤
│                            │
│      PM2.5  █████          │  ← PM2.5 gauge + value
│      145 µg/m³             │
│                            │
│  CO2: 850 ppm             │  ← Large CO2 tile (focused)
│  ████████░░░               │
│                            │
│  🌡️ 25.3°C  💧 65%        │  ← Temp + Humidity tiles
│                            │
│  ⚗️ TVOC  150             │  ← TVOC + NOx tiles
│  ☢️ NOx   120             │
│                            │
│  🌍 1013 hPa              │  ← Pressure tile
│                            │
│  GPS: 13.7563, 100.5018   │  ← GPS coordinates (if fix)
└────────────────────────────┘
```

### Random Values Generated

| Parameter | Range | Unit |
|-----------|-------|------|
| PM2.5 | 5 - 150 | µg/m³ |
| CO2 | 400 - 2000 | ppm |
| TVOC | 50 - 400 | index |
| NOx | 50 - 400 | index |
| Temperature | 15 - 35 | °C |
| Humidity | 30 - 80 | % |
| Pressure | 980 - 1030 | hPa |
| Battery | 20 - 100 | % |
| GPS Status | Off / Searching / Fix | - |

---

## 🎨 Icons Displayed

All icons remain visible throughout operation:

- **🔋 Battery Icon**: Shows percentage and charging state
- **📡 GPS Time**: Clock icon with UTC time
- **🛰️ GPS Status**: Satellite icon (off/searching/fix)
- **🌡️ Temperature**: Thermometer icon
- **💧 Humidity**: Water drop icon
- **⚗️ TVOC**: Flask icon for volatile compounds
- **☢️ NOx**: Radiation symbol for nitrogen oxides
- **🌍 Pressure**: Globe icon for atmospheric pressure

---

## ⚙️ Technical Details

### Display Configuration
- **Model**: GDEY0213B74H (2.13" e-paper)
- **Resolution**: 144 × 296 pixels
- **Controller**: SSD1680
- **Colors**: Black & White (2-level)
- **Refresh Mode**: PARTIAL (fast updates)
- **SPI Speed**: 4 MHz

### GPIO Pins
| Pin | Function |
|-----|----------|
| GPIO10 | BUSY |
| GPIO9 | RST |
| GPIO15 | DC |
| GPIO0 | CS |
| GPIO23 | SCK |
| GPIO25 | MOSI |
| GPIO24 | MISO |

### Update Behavior
- **Initial**: Full refresh (3 seconds) to show all icons
- **Periodic**: Partial refresh every 5 seconds
- **Values**: Random generation for demonstration
- **Logging**: ESP_LOGI every update with current values

---

## 🔧 Customization

### Change Update Interval

Edit line ~246 in `display_example.cpp`:
```cpp
const uint64_t UPDATE_INTERVAL_MS = 5000; // Change to desired ms
```

### Modify Value Ranges

Edit random value generation (lines ~257-265):
```cpp
sensor_values_t random_values = {
  .pm25_ugm3 = (uint16_t)random_int(5, 150),    // Change min/max
  .co2_ppm = (uint16_t)random_int(400, 2000),   // Change min/max
  // ... etc
};
```

### Change Initial GPS State

Edit line ~234:
```cpp
display.set_gps_status(Display::GPSStatus::Fix); // Off/Searching/Fix
```

---

## 📝 Example Log Output

```
I (1234) display_example: === E-Paper Display Example ===
I (1235) display_example: Display: 144x296 SSD1680
I (1456) display_example: Panel: 144x296, buffer: 5328 bytes
I (1457) display_example: Display initialized successfully
I (1458) display_example: Dashboard created
I (1459) display_example: Performing initial full refresh...
I (4560) display_example: Starting random value updates (all icons visible)...
I (9561) display_example: Updated: PM2.5=78 CO2=1234 TVOC=156 NOx=203 T=24.5°C H=62.3% P=101325 Pa
I (9562) display_example: Battery: 85% (charging) | GPS: FIX
I (14563) display_example: Updated: PM2.5=45 CO2=890 TVOC=98 NOx=145 T=26.1°C H=58.7% P=100980 Pa
I (14564) display_example: Battery: 67% | GPS: SEARCHING
```

---

## 🎯 Use Cases

1. **Display Testing**: Verify e-paper hardware without sensors
2. **UI Development**: Test dashboard layouts and icon visibility
3. **Performance Check**: Measure refresh rates and partial updates
4. **Demo Mode**: Show device capabilities without real sensors
5. **Icon Verification**: Ensure all icons render correctly

---

## ⚠️ Notes

- **No Real Sensors**: Values are randomly generated
- **No Real GPS**: Status cycles through states randomly
- **Battery State**: Random charging/discharging simulation
- **Continuous Operation**: Loop runs forever with updates
- **Memory Usage**: ~8KB stack for LVGL task

---

## 🔙 Reverting to Main App

1. **Restore** `main/airgradient-go.cpp`:
   ```cpp
   extern "C" void app_main(void) {  // Remove _original
   ```

2. **Disable** example in `main/display_example.cpp`:
   ```cpp
   // Comment this line:
   // extern "C" void app_main(void) __attribute__((alias("app_main_display_example")));
   ```

3. **Rebuild**:
   ```bash
   idf.py build flash
   ```

---

**Created**: January 27, 2026  
**Display**: GDEY0213B74H (144×296)  
**Framework**: ESP-IDF v5.5.1 + LVGL
