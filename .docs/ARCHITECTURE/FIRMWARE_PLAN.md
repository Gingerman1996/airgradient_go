# AirGradient GO - Firmware Planning & Implementation Checklist

## 📊 Current Progress (Updated: Jan 14, 2026)

### Overall Status: ~40% MVP Complete

| Phase | Status | Progress | Notes |
|-------|--------|----------|-------|
| Phase 1.1 | 🟢 Complete | 100% | Dashboard + sensors integrated, TEST SCREEN working |
| Phase 1.2 | 🟢 Complete | 100% | Status bar + blink animations working |
| Phase 1.3 | 🟡 In Progress | 60% | CAP1203 integrated, focus navigation (T1/T2) done, menu actions pending |
| Phase 1.4 | 🔴 Not Started | 0% | Quick menu pending |
| Phase 2 | 🔴 Not Started | 0% | Recording system |
| Phase 3 | 🟡 In Progress | 10% | GPS UART + basic NMEA parsing started |
| Phase 4 | 🔴 Not Started | 0% | Tagging |
| Phase 5 | 🔴 Not Started | 0% | LEDs & alerts |
| Phase 6 | 🔴 Not Started | 0% | Settings menu |
| Phase 7 | 🔴 Not Started | 0% | BLE |
| Phase 8 | 🔴 Not Started | 0% | WiFi auto-sync |

### ✅ Infrastructure Complete
- Display driver (SSD1680 e-paper 144x296) with partial refresh
- LVGL integration with proper mutex locking
- Power-on TEST SCREEN with max values verification
- Sensor drivers:
  - STCC4 (CO2/Temp/RH @ 0x62) with state machine and 5s averaging
  - SGP41 (VOC/NOx @ 0x59) with Gas Index Algorithm
  - SPS30 (PM sensor @ 0x69) with EN_PM1 load switch control
  - **DPS368 (Pressure/Temp @ 0x77) - INTEGRATED ✅** (Jan 13, 2026)
    - Full calibration coefficient reading (c0-c30)
    - 16x oversampling with P_SHIFT/T_SHIFT enabled
    - Compensated pressure (Pa) and temperature (°C) calculation
  - **LIS2DH12 (Accelerometer @ 0x18) - INTEGRATED ✅** (Jan 14, 2026)
    - Accel readouts + motion interrupt on INT1 (GPIO3)
    - High-event motion config with HPF baseline reset for IA1
- Display API (`Display` class) - fully implemented with all status bar elements
- Sensor API (`Sensors` class) - working with ring buffer averaging
- I2C bus initialization (GPIO 6=SCL, GPIO 7=SDA @ 100kHz)
- PM sensor power control (GPIO 26 EN_PM1 for TPS27081A + TMUX121)
- **BQ25629 battery charger driver - INTEGRATED ✅**
  - Ship mode support (0.15 μA battery leakage)
  - Shutdown mode support (0.1 μA battery leakage)
  - System power reset capability
  - ADC for battery/charging monitoring
  - **Register limit logging (Jan 13, 2026)** - `log_charger_limits()` function
    - REG0x02 (ICHG): Charge current setting
    - REG0x04 (VREG): Battery regulation voltage
    - REG0x06 (IINDPM): Input current limit
    - REG0x08 (VINDPM): Input voltage limit
  - **Formula corrections per datasheet** - all register calculations verified
- **QON Button Power Management - IMPLEMENTED ✅**
  - GPIO5 QON button monitoring task
  - 5-second long press shutdown detection
  - Visual shutdown sequence with e-paper feedback
  - Automatic ship mode entry on shutdown

### 🎯 Next Priorities
1. **Finish button controls** (Phase 1.3)
   - Map Middle (T3) short/long press actions
   - Add touch lock + auto-lock
2. **Create Quick Menu UI** (Phase 1.4)
   - Menu navigation system
   - Button integration for navigation
3. **Build recording ring buffer system** (Phase 2)
   - Flash storage integration
   - Configurable recording intervals
4. **Add battery monitoring UI**
   - Display battery voltage/percentage
   - Show charging status
   - Update status bar battery icon with real data

---

## Hardware Summary

### 🔌 MCU & Core
| Component | Part Number | Notes |
|-----------|-------------|-------|
| MCU | ESP32-C5-MINI-1 | Main processor, WiFi + BLE |
| Flash Memory | W25N512GVEIG | 512Mbit NAND flash for data storage |
| Hardware Watchdog | TPL5010DDCR | External watchdog timer |

### ⚡ Power Management
| Component | Part Number | Notes |
|-----------|-------------|-------|
| Charge Controller | BQ25629 | Battery charging + NTC, ShipMode support |
| BMS | DW01 | Battery protection IC |
| 3.1V DC-DC | TPS63802DLAR | Buck-boost converter |
| LDO 2.8V | TPS7A0228 | Low-noise LDO |
| Loadswitch (PM Sensors) | TPS27081A | IO4 controls SPS30 power |
| Not Gate | SN74LVC1G06 | Logic inverter |

### 📟 Display & LEDs
| Component | Part Number | Notes |
|-----------|-------------|-------|
| E-Paper Display | GDEY0213B74H | 2.13" 144x296 (SSD1680 compatible) |
| RGB LEDs | XL-3210RGBC-YG | Full brightness RGB |
| LED Controller | LP5030 / LP5036 | I2C LED driver |

### 🌡️ Sensors
| Component | Part Number | Notes |
|-----------|-------------|-------|
| CO2 + Temp + RH | STCC4 | Primary CO2 sensor **(IN USE @ 0x62)** |
| CO2 (alt) | SCD41 | Alternative CO2 option (same address family) |
| Temp + Humidity | SHT40 | Backup T/RH sensor (optional, not currently used) |
| VOC + NOx | SGP41 | Air quality index **(IN USE @ 0x59)** |
| PM Sensor | SPS30 | Laser PM sensor **(IN USE @ 0x69)** |
| PM Sensor (alt) | PMSA003I | Alternative PM option |
| Pressure | DPS368 | Barometric pressure + temperature (integrated) |
| Accelerometer | LIS2DH12 | Accel read + motion interrupt (integrated) |

> **Sensor Implementation Status (Jan 2026)**:
> - ✅ **STCC4**: State machine with 10s measurement cycle, 5s moving average for CO2/Temp/RH
> - ✅ **SGP41**: VOC/NOx raw ticks with Gas Index Algorithm (1-500 scale), 1s sampling
> - ✅ **SPS30**: PM2.5 mass concentration (µg/m³)
> - ✅ **DPS368**: I2C address 0x77, pressure (Pa) + temperature (°C) with full compensation
> - ✅ **LIS2DH12**: I2C address 0x18, accel read + motion interrupt (INT1)

### 📍 GPS
| Component | Part Number | Notes |
|-----------|-------------|-------|
| GPS Module | TAU1113 | NMEA output via UART |
| Passive Antenna | Rainsun GPS1003 | Chip antenna |
| Active Antenna | M20047-1 | External antenna option |

### 🎛️ User Interface
| Component | Part Number | Notes |
|-----------|-------------|-------|
| Capacitive Buttons (3x) | CAP1203-1-AC3-TR | Touch sensing IC, I2C |

### 🔗 Other ICs
| Component | Part Number | Notes |
|-----------|-------------|-------|
| I2C Isolation | TMUX121 | I2C bus multiplexer/isolation |

### 📋 I2C Device Address Map
| Device | Address | Bus | Status |
|--------|---------|-----|--------|
| STCC4/SCD4x | 0x62 | Main | ✅ Active |
| SCD41 | 0x62 | Main | Alternative |
| SHT40 | 0x44 | Main | Optional |
| SGP41 | 0x59 | Main | ✅ Active |
| DPS368 | 0x77 | Main | ✅ Active |
| LIS2DH12 | 0x18 | Main | ✅ Active |
| CAP1203 | 0x28 | Main | ✅ Component ready (CS1/CS2/CS3 mapped; UI: T1=Up, T2=Down, T3=Middle) |
| LP5030/36 | 0x30 | Main | Pending Phase 5 |
| BQ25629 | 0x6A | Main | ✅ Active |
| SPS30 | 0x69 | Isolated (TMUX121) | ✅ Active |
| PMSA003I | 0x12 | Isolated (TMUX121) | Alternative |

> **Note**: Current I2C bus operates at **100 kHz** on GPIO 6 (SCL) and GPIO 7 (SDA) with internal pull-ups enabled.

### 🔌 Key GPIO Assignments
| GPIO | Function | Notes |
|------|----------|-------|
| IO0 | E-Paper CS | SPI chip select |
| IO1 | INT (Second Board) | Interrupt from second board |
| IO2 | WD_RST | External watchdog reset (TPL5010) |
| IO3 | ACC_INT | Accelerometer interrupt (LIS2DH12) |
| IO4 | NAND_CS | Flash memory chip select (W25N512GVEIG) |
| IO5 | QON | Charger control (BQ25629) |
| IO6 | SCL | I2C Clock |
| IO7 | SDA (Jumper) | I2C Data - Jumper selectable with IO8 |
| IO8 | SDA (Jumper) | I2C Data - Jumper selectable with IO7 |
| IO9 | E-Paper RES | Display reset |
| IO10 | E-Paper BUSY | Display busy signal |
| IO15 | E-Paper D/C | Data/Command |
| IO23 | SCLK | SPI clock (E-paper + Flash) |
| IO24 | MISO | SPI MISO |
| IO25 | MOSI | SPI MOSI |
| IO26 | EN_PM1 | PM Sensor power enable (TPS27081A) |
| TXD0 | MCU_TX | GPS UART TX |
| RXD0 | MCU_RX | GPS UART RX (NMEA) |

> **See also**: [HARDWARE_MAP.md](HARDWARE_MAP.md) for complete pin assignments and peripheral connections.

---

## Implementation Phases

> **📝 Current Code Architecture (Jan 14, 2026)**:
> ```
> main/
>   ├── airgradient-go.cpp       ✅ Main loop with TEST SCREEN + sensor integration
>   ├── sensor.cpp/.h            ✅ Sensor management with state machines + EN_PM1 control
>   │   └── Sensors class:
>   │       - STCC4 state machine (10s cycle, 5s averaging)
>   │       - SGP4x VOC/NOx with Gas Index Algorithm
>   │       - SPS30 PM sensor with load switch control (GPIO26)
>   │       - DPS368 pressure/temperature
>   │       - LIS2DH12 accel + motion interrupt (INT1)
>   │       - getValues() for display-ready data
>   ├── ui_display.cpp/.h        ✅ Complete UI with LVGL + partial refresh
>   │   └── Display class:
>   │       - TEST SCREEN on boot (max values verification)
>   │       - All status bar elements (battery, BLE, WiFi, GPS)
>   │       - Sensor value setters (PM2.5, CO2, VOC, etc.)
>   │       - Blink animations (REC, charging)
>   └── lv_conf.h                ✅ LVGL configuration
>
> components/
>   ├── esp_epaper/              ✅ SSD1680 e-paper driver (144x296)
>   ├── stcc4/                   ✅ STCC4/SCD4x CO2 sensor driver
>   ├── esp_sgp4x/               ✅ SGP41 VOC/NOx sensor driver
>   ├── sps30/                   ✅ SPS30 PM sensor driver
>   ├── dps368/                  ✅ DPS368 pressure/temp sensor (Jan 13)
>   ├── lis2dh12/                ✅ LIS2DH12 accelerometer driver (Jan 14)
>   ├── sensirion_gas_index_algorithm/  ✅ Gas index calculation
>   ├── bq25629/                 ✅ BQ25629 charger driver + log_charger_limits()
>   └── cap1203/                 ✅ CAP1203 capacitive touch buttons (T1/T2/T3)
> ```
>
> **Implementation Status**:
> - Phase 1.1: ✅ Complete - Dashboard with TEST SCREEN → real sensor values
> - Phase 1.2: ✅ Complete - Status bar with all icons and animations
> - Phase 1.3: 🟡 In Progress - CAP1203 integrated in main, button actions pending
> - Next: Finish button actions + focus highlighting (Phase 1.3)
> - Then: Phase 1.4 Quick Menu → Phase 2 Recording system

---

### Phase 1: Core UI & Display (MVP)
**Goal**: Establish UI framework and dashboard display

#### 1.1 Main Dashboard ✅ **(COMPLETE)**
- [x] Single-screen layout showing all sensor values ✅
  - [x] PM2.5 (large, prominent) - `Display::setPM25f()`
  - [x] CO2 (ppm) - `Display::setCO2()`
  - [x] VOC Index - `Display::setVOC()`
  - [x] NOx Index - `Display::setNOx()`
  - [x] Temperature (°C/°F) - `Display::setTempCf()`
  - [x] Humidity (%) - `Display::setRHf()`
  - [x] Pressure (hPa) - `Display::setPressure()`
- [x] Power-on TEST SCREEN with max values ✅
  - [x] Verify all display segments working (PM2.5=999.9, CO2=9999, VOC/NOx=999, Temp=100°C)
  - [x] All status icons visible (WiFi, GPS, BLE, Battery 100%, REC, Alert)
  - [x] 3-second display → transition to real sensor values
- [x] Real sensor data integration ✅
  - [x] Wire `Sensors::getValues()` to display setters
  - [x] Update display every 1 second
- [x] Partial refresh every recording interval (default 10s)
- [x] Initial refresh after first data update (exit TEST SCREEN)
- [x] Value focus highlighting for CO2/PM2.5 tiles (inverted colors) ✅
- [x] No graphs/trends (MVP) ✅

> **Implementation Status**: Display layout complete with TEST SCREEN on boot. All sensors integrated and updating display in real-time. Focus highlighting for CO2/PM2.5 implemented with T1/T2 navigation; T3 actions still pending (Phase 1.3).

#### 1.2 Status Bar (Always Visible) ✅ **(COMPLETE)**
**Row 1**:
- [x] BLE icon (connected/disconnected) - `Display::setBLEStatus()` ✅
- [x] WiFi icon (off/connecting/connected) - `Display::setWiFiStatus()` ✅
- [x] GPS icon (off/searching/fix) - `Display::setGPSStatus()` ✅
- [x] Battery percentage + icon (with charging bolt) - `Display::setBattery()` ✅
  - [x] Charging blink animation (1s interval) ✅

**Row 2**:
- [x] Recording indicator: ● REC / PAUSE (with blink animation) - `Display::setRecording()` ✅
  - [x] Blink animation at 500ms interval ✅
- [x] Recording interval display (e.g., "10s") - `Display::setIntervalSeconds()` ✅
- [x] Time HH:MM (or "--:--" if not set) - `Display::setTimeHM()` ✅
- [x] Alert indicator (!) for faults/warm-up/storage/timing issues - `Display::setAlert()` ✅

> **Implementation Status**: All status bar elements implemented in `ui_display.cpp`. Icons use opacity to indicate states (30%=off, 60%=connecting, 100%=active). Battery icon changes based on charge level.

#### 1.3 Button Controls (3 Capacitive) 🟡 **(IN PROGRESS)**
- [x] CAP1203 init + polling task + button callback integrated in main ✅
- [x] T1 (Up): Move focus up between tiles (CO2/PM2.5) ✅
- [x] T2 (Down): Move focus down between tiles (CO2/PM2.5) ✅
- [x] Immediate focus update on press (PRESS/SHORT/LONG) + urgent refresh ✅
- [x] Enable CAP1203 interrupts so latch clears for repeated presses ✅
- [ ] T3 (Middle) short press: Quick Menu
- [ ] T3 (Middle) long press: Start/Stop Session (with confirmation)
- [ ] T2 (Down) long press: Touch lock/unlock
- [ ] Auto-lock after 20s inactivity

#### 1.4 Quick Menu
- [ ] Menu navigation with Up/Down buttons (T1/T2)
- [ ] Middle button (T3) to select items:
  - [ ] Add Tag
  - [ ] Settings
  - [ ] Pause/Resume Recording
  - [ ] About
- [ ] Back/Cancel option

---

### Phase 2: Recording System
**Goal**: Autonomous local data logging with ring buffer

#### 2.1 Ring Buffer Storage
- [ ] Implement circular buffer for sensor data
- [ ] Storage high watermark detection (trigger ! alert)
- [ ] Data persistence (flash or PSRAM)
- [ ] Format: timestamp, PM2.5, CO2, VOC index, temp, RH, pressure, battery, power mode, sensor flags

#### 2.2 Recording Intervals
- [ ] Configurable intervals: 1s, 10s, 30s, 60s, 5m, 15m, 1h (default: 5m)
- [ ] Revisit UI refresh cadence (currently tied to recording interval)
- [ ] Optimize for battery life (don't refresh faster than needed)
- [ ] Handle 1s interval special case (storage + battery impact)

#### 2.3 Power Mode Tracking
- [ ] Detect BAT (battery) vs PWR (power adapter) mode
- [ ] Log power mode in each record
- [ ] Store sensor quality flags per record

---

### Phase 3: GPS & Sessions (Routes)
**Goal**: Enable autonomous GPS-enabled recording sessions

#### 3.1 GPS Integration
- [x] NMEA parser for GPS module (GGA/RMC)
- [x] GPS fix detection (GGA/RMC status)
- [x] UTC time from NMEA for UI (HH:MM)
- [x] libnmea component integrated for NMEA parsing
- [ ] RTC sync from GPS (when fix available)

#### 3.2 Session Management
- [ ] Start/stop via middle long press
- [ ] During session:
  - [ ] Record at global recording interval
  - [ ] Record GPS points at configurable interval (default 10s)
  - [ ] Option to change GPS frequency: 1s, 10s, 30s, 60s
- [ ] Session storage with session ID
- [ ] Associate recordings + tags with session ID

#### 3.3 Session Summary Display
- [ ] Duration
- [ ] PM2.5 average & peak
- [ ] CO2 average & peak
- [ ] Tag count
- [ ] Display 5-10s after session stop

#### 3.4 GPS Modes (Settings)
- [ ] OFF
- [ ] SESSIONS ONLY (default)
- [ ] HOME DETECT (for WiFi sync location)
- [ ] ALWAYS (continuous GPS)

---

### Phase 4: Tagging System
**Goal**: On-device annotation with structured tags

#### 4.1 Tag Categories & Values

**A) Activity Tags**:
- [ ] run, walk, bike, commute, cooking, sleep, workout, working

**B) Context Tags**:
- [ ] indoor, outdoor, car, bus, train/metro, motorbike, bicycle, office, home

**C) Source Tags** (expanded):
- [ ] traffic
- [ ] smoke
- [ ] wildfire smoke
- [ ] incense
- [ ] cooking
- [ ] grilling/BBQ
- [ ] cleaning products
- [ ] paint/solvents
- [ ] perfume/fragrance
- [ ] candles
- [ ] vaping/smoking
- [ ] dust/construction
- [ ] road dust
- [ ] generator/exhaust
- [ ] factory/industrial
- [ ] garbage burning
- [ ] fireworks
- [ ] pesticides/spraying
- [ ] humidity/steam (shower)
- [ ] unknown

#### 4.2 Tagging UI Flow
- [ ] Quick Menu -> Add Tag
- [ ] Category selection
- [ ] Value selection
- [ ] Confirmation
- [ ] Optional: "last used" shortcut at top

#### 4.3 Tag Storage
- [ ] Store with timestamp
- [ ] Include sensor snapshot (PM2.5, etc.)
- [ ] Store session ID (if active)
- [ ] Store GPS position (if fix available)

---

### Phase 5: Alerts & LEDs
**Goal**: Visual feedback for warnings/alerts

#### 5.1 LED Behavior Rules
- [ ] BAT mode: LEDs OFF normally
- [ ] LEDs ON only for:
  - [ ] Warnings (brief pulses)
  - [ ] Optional "touch preview" (2-3s)
- [ ] PWR mode: May be ON continuously (configurable)

#### 5.2 LED Patterns (2 only)
- [ ] Warning: short pulse every 60s (while condition persists)
- [ ] Critical: faster pulse every 15s (while condition persists)

#### 5.3 Warning Triggers
- [ ] **PM2.5**: Warning > 35 µg/m³ (or USAQI > 100), Critical > 75 µg/m³ (USAQI > 150)
- [ ] **CO2**: Warning > 1200 ppm, Critical > 2000 ppm
- [ ] **VOC Index**: Warning > 200, Critical > 400
- [ ] **Battery**: Warning < 20%, Critical < 10%
- [ ] If any critical active → show Critical pattern (all LEDs)

#### 5.4 Alert Indicator
- [ ] Display ! in status bar if any warning/critical condition exists

---

### Phase 6: Settings Menu (MVP)
**Goal**: On-device configuration

#### 6.1 Recording Settings
- [ ] Recording: On/Off
- [ ] Interval: 1s, 10s, 30s, 60s, 5m, 15m, 1h

#### 6.2 Display Settings
- [ ] Temperature units: °C / °F
- [ ] PM display format: µg/m³ / USAQI

#### 6.3 GPS Settings
- [ ] GPS Mode: OFF / SESSIONS ONLY / HOME DETECT / ALWAYS
- [ ] Session GPS interval: 1s / 10s / 30s / 60s

#### 6.4 Connectivity Settings
- [ ] WiFi: On/Off
- [ ] Auto-sync: Off / Home Zone only / Home Zone + PWR only (default)
- [ ] Home Zone radius: 200m / 500m / 1km

#### 6.5 Data Management
- [ ] Storage used (display)
- [ ] Clear data (with confirmation)

#### 6.6 Device Info
- [ ] FW version
- [ ] Device ID
- [ ] Sensor status

---

### Phase 7: BLE Communication (MVP)
**Goal**: Phone app integration for provisioning and sync

#### 7.1 BLE Services
- [ ] Device info: FW version, device ID
- [ ] Settings: Read/Write
- [ ] Data sync: Records since timestamp/cursor
- [ ] Time sync: Fallback if GPS unavailable

#### 7.2 BLE Provisioning
- [ ] WiFi credentials upload
- [ ] Home Zone (lat/lon + radius) setup
- [ ] Time synchronization

#### 7.3 Data Sync Over BLE
- [ ] Stream records since cursor/timestamp
- [ ] Stream sessions with GPS points
- [ ] Stream tags with snapshots

---

### Phase 8: WiFi Auto-Sync (AFTER MVP)
**Goal**: Automatic cloud data upload when conditions met

#### 8.1 Home Zone WiFi Sync
- [ ] Sync triggers when:
  - [ ] WiFi enabled
  - [ ] Auto-sync enabled
  - [ ] GPS fix confirmed (inside home zone)
  - [ ] Power mode check (if configured)

#### 8.2 Upload Data
- [ ] Incremental records since last successful sync
- [ ] Sessions with all GPS points
- [ ] Tags with sensor snapshots

#### 8.3 Battery vs Power Mode
- [ ] BAT mode: Duty-cycle WiFi (connect → upload → disconnect)
- [ ] Timeout if upload too slow
- [ ] Only sync if configured for BAT mode

#### 8.4 Sync Status Display
- [ ] Last sync time in Settings
- [ ] Last result (OK / Fail)

---

## File Structure & Organization

```
main/
  ├── airgradient-go-esp32c6.cpp       # App entry, scheduling, power mgmt
  ├── ui_display.cpp / .h              # UI/Display layer (LVGL)
  ├── sensor.cpp / .h                  # Sensor reading + filtering
  ├── lv_conf.h                        # LVGL configuration
  └── (new) recording.cpp / .h         # Ring buffer, data logging
  └── (new) gps.cpp / .h               # GPS/NMEA handling
  └── (new) tagging.cpp / .h           # Tag management
  └── (new) settings.cpp / .h          # Settings storage/retrieval
  └── (new) ble_service.cpp / .h       # BLE communication

components/
  └── (existing) esp_epaper/           # Display driver
  └── (existing) esp_sgp4x/            # VOC sensor
  └── (existing) sps30/                # Particulate sensor
  └── (existing) stcc4/                # Charger driver
  └── (new) esp_gps/                   # GPS module driver
  └── (new) esp_battery/               # Battery management
  └── (new) esp_storage/               # Flash storage abstraction

documents/
  ├── FIRMWARE_PLAN.md                 # This file
  ├── HARDWARE_MAP.md                  # Pin definitions, I2C addresses, power path
  ├── POWER_STATES.md                  # Boot sequence, shutdown, power management
  ├── DRIVER_CONTRACTS.md              # Unified driver API contracts
  ├── BLE_PROTOCOL.md                  # (New) BLE message format
  ├── DATA_FORMAT.md                   # (New) Record/session/tag formats
  └── (existing) *.md                  # Sensor datasheets
```

---

## Development Priorities

### MVP (Must Have) - Phases 1-4
1. Dashboard display + status bar
2. Capacitive button handling + Quick Menu
3. Ring buffer recording at specified intervals
4. GPS integration with session support
5. Tagging with structured categories
6. Basic settings menu

### Phase 5 Integration - Alerts & LEDs
7. Warning/critical logic
8. LED alert patterns

### Post-MVP - Phases 6-8
9. BLE provisioning + data sync
10. WiFi auto-sync with home zone
11. Advanced settings + data management

---

## 🧪 Testing Plan

### Test Categories Overview

| Category | Type | Priority | Environment |
|----------|------|----------|-------------|
| HW-1xx | Hardware | P0-P1 | Bench + Device |
| FW-2xx | Firmware Unit | P0-P1 | ESP-IDF + Mock |
| FW-3xx | Integration | P1 | Device |
| FW-4xx | System/E2E | P1-P2 | Field |
| FW-5xx | Stress/Endurance | P2 | Lab + Field |

---

## 🔧 Hardware Testing (HW-1xx)

### HW-100: Power System
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| HW-101 | Battery voltage reading | Measure at known charge levels (0%, 25%, 50%, 75%, 100%) | ADC reading within ±5% of multimeter | ✅ |
| HW-102 | Charging detection | Connect/disconnect USB power | Charging flag toggles correctly | [ ] |
| HW-103 | Charging current | Measure with current clamp during charge | Within spec (≤1A typical) | [ ] |
| HW-104 | ShioMode entry | Long press 5s | Display clears, charger enters ship mode | [ ] |
| HW-105 | ShioMode exit | Press QON button | Device boots, display initializes | [ ] |
| HW-106 | Load switch (IO4) | Toggle load switch | SPS30 and peripherals power on/off | [ ] |
| HW-107 | Watchdog reset | Allow 5min+ timeout | External watchdog resets device | ✅ |
| HW-108 | Sleep current | Measure in ShioMode | < 10µA (target) | [ ] |
| HW-109 | BQ25629 I2C comm | Read charger registers | Device ID and status readable | [ ] |
| HW-10A | BQ25629 charge termination | Charge to 100% | Charging stops at correct voltage | [ ] |
| HW-10B | BQ25629 NTC sensing | Vary battery temperature | Charger adjusts/stops per temp | [ ] |
| HW-10C | BQ25629 fault flags | Simulate fault (over-temp) | Fault register set correctly | [ ] |
| HW-10D | BQ25629 pre-charge | Deeply discharged battery | Pre-charge current applied | [ ] |
| HW-10E | DW01 protection | Short circuit simulation | BMS cuts off battery | [ ] |
| HW-10F | DC-DC efficiency | Measure input/output power | Efficiency > 85% | [ ] |


### HW-110: Display (SSD1680 E-Paper)
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| HW-111 | Full refresh | Trigger full screen update | Clean B/W display, no ghosting | ✅ |
| HW-112 | Partial refresh | Update single region | Only affected area changes | ✅ |
| HW-113 | Refresh cycle time | Measure with scope | Full: <2s, Partial: <0.5s | ✅ |
| HW-114 | Display clear on power-off | Enter ShioMode | Screen shows blank/off message | [ ] |
| HW-115 | Gray levels (if supported) | Display gradient test pattern | Visible gray shades | [ ] |
| HW-116 | Temperature effect | Test at 0°C, 25°C, 40°C | Display functional at all temps | [ ] |

### HW-120: Sensors
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| HW-121 | STCC4 CO2 range | Expose to 400-2000ppm | Readings match reference meter ±50ppm | ✅ |
| HW-122 | STCC4 temperature | Compare to calibrated thermometer | Within ±0.5°C | ✅ |
| HW-123 | STCC4 humidity | Compare to calibrated hygrometer | Within ±3% RH | ✅ |
| HW-124 | SGP4x VOC response | Expose to alcohol vapor | VOC index rises significantly | ✅ |
| HW-125 | SGP4x NOx response | Expose to source (match, etc.) | NOx index rises | ✅ |
| HW-126 | SPS30 PM2.5 | Compare to reference PM monitor | Within ±10µg/m³ or ±25% | ✅ |
| HW-127 | SPS30 fan | Listen/measure vibration | Fan runs, no rattling | ✅ |
| HW-128 | Sensor warm-up | Power on from cold | Readings stabilize within spec time | ✅ |
| HW-129 | I2C bus scan | Scan at startup | All sensors detected at expected addresses | ✅ |
| HW-12A | DPS368 pressure range | Compare to reference barometer | Within ±1 hPa | ✅ |
| HW-12B | DPS368 altitude calc | At known elevation | Calculated altitude within ±10m | [ ] |
| HW-12C | DPS368 temperature | Compare to thermometer | Within ±0.5°C | ✅ |
| HW-12D | SHT40 temp/humidity | Compare to reference | T: ±0.2°C, RH: ±1.8% | [ ] |
| HW-12E | Sensor cross-check | Compare STCC4 vs SHT40 temp | Within ±1°C of each other | [ ] |
| HW-12F | SCD41 CO2 range | Expose to 400-2000ppm | Readings match reference meter ±50ppm | [ ] |
| HW-12G | S12 CO2 range | Expose to 400-2000ppm | Readings match reference meter ±50ppm | [ ] |
| HW-12H | PMSA003I PM2.5 | Compare to reference PM monitor | Within ±10µg/m³ or ±25% | [ ] |

### HW-130: GPS Module
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| HW-131 | UART communication | Check NMEA output | Valid NMEA sentences received | ✅ |
| HW-132 | Cold start fix | Power on outdoors | Fix acquired within 60s | ✅ |
| HW-133 | Hot start fix | Resume from sleep | Fix acquired within 10s | ✅ |
| HW-134 | Position accuracy | Compare to known location | Within 5m CEP | ✅ |
| HW-135 | Time sync | Get GPS time | RTC set to UTC ±1s | ✅ |
| HW-136 | Indoor behavior | Test indoors | No fix, timeout handled gracefully | [ ] |

### HW-140: Capacitive Buttons (3x)
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| HW-141 | Touch detection | Touch each button | Each button triggers event | ✅ |
| HW-142 | Debounce | Rapid touches | No false triggers, clean edges | ✅ |
| HW-143 | Long press detection | Hold for 3s | Long press event fires | ✅ |
| HW-144 | Sensitivity | Touch with dry/wet finger | Works in both conditions | [ ] |
| HW-145 | Touch through enclosure | Touch with case installed | All buttons responsive | [ ] |
| HW-146 | No ghost touches | Leave device idle | No spurious button events | [ ] |

### HW-150: LEDs
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| HW-151 | Individual control | Toggle each LED | Each LED lights independently | [ ] |
| HW-152 | PWM brightness | Set 0%, 50%, 100% | Visible brightness change | [ ] |
| HW-153 | Warning pattern | Trigger warning condition | 60s pulse pattern | [ ] |
| HW-154 | Critical pattern | Trigger critical condition | 15s pulse pattern | [ ] |

### HW-160: Communication
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| HW-161 | BLE advertising | Scan with phone | Device visible with correct name | [ ] |
| HW-162 | BLE connection | Connect from app | Stable connection established | [ ] |
| HW-163 | BLE range | Move away from device | 10m+ range in open space | [ ] |
| HW-164 | WiFi scan | Enable WiFi, scan | Nearby networks detected | [ ] |
| HW-165 | WiFi connect | Connect to known AP | IP obtained, ping works | [ ] |
| HW-166 | WiFi range | Test at various distances | Usable signal at 10m+ | [ ] |

### HW-170: Accelerometer (LIS2DH12)
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| HW-171 | I2C communication | Read WHO_AM_I register | Returns 0x33 | ✅ |
| HW-172 | X-axis reading | Tilt device left/right | X value changes ±1g | ✅ |
| HW-173 | Y-axis reading | Tilt device forward/back | Y value changes ±1g | ✅ |
| HW-174 | Z-axis reading | Flip device upside down | Z value flips sign | ✅ |
| HW-175 | Orientation detection | Lay flat, portrait, landscape | Correct orientation detected | ✅ |
| HW-176 | Motion interrupt | Shake device | INT pin triggers | ✅ |
| HW-177 | Free-fall detection | Drop test (protected) | Free-fall event detected | [ ] |
| HW-178 | Low-power mode | Enable LP mode | Current < 10µA | [ ] |
| HW-179 | Data rate setting | Set ODR 1Hz, 10Hz, 100Hz | Samples at correct rate | [ ] |

Notes (Jan 14, 2026): WHO_AM_I read verified in logs for LIS2DH12.

### HW-180: Flash Memory (W25N512GVEIG)
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| HW-181 | SPI communication | Read JEDEC ID | Returns correct manufacturer/device ID | ✅ |
| HW-182 | Page write | Write 2KB page | Data written without error | ✅ |
| HW-183 | Page read | Read back written page | Data matches written | ✅ |
| HW-184 | Block erase | Erase 128KB block | All bytes = 0xFF | ✅ |
| HW-185 | Bad block check | Scan for bad blocks | Factory bad blocks identified | ✅ |
| HW-186 | ECC functionality | Write/read with bit flip | ECC corrects error | [ ] |
| HW-187 | Write speed | Write 1MB data | Speed within spec (>1MB/s) | [ ] |
| HW-188 | Read speed | Read 1MB data | Speed within spec (>10MB/s) | [ ] |
| HW-189 | Wear leveling | Write same block 1000x | No errors, wear distributed | [ ] |
| HW-18A | Power loss recovery | Pull power during write | No corruption on restart | [ ] |
| HW-18B | Full capacity test | Fill all 512Mbit | No addressing errors | [ ] |

### HW-190: I2C Bus & Isolation (TMUX121)
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| HW-191 | TMUX121 enable | Toggle EN pin | I2C bus connects/disconnects | ✅ |
| HW-192 | Bus isolation | Disable TMUX, scan | Isolated devices not visible | ✅ |
| HW-193 | Bus capacitance | Measure with scope | Rise time < 300ns | [ ] |
| HW-194 | Multi-device scan | Scan all I2C devices | All expected addresses respond | [ ] |
| HW-195 | Bus recovery | Force bus stuck low | Recovery sequence restores bus | [ ] |
| HW-196 | Clock stretching | Device holds SCL | Master waits correctly | [ ] |
| HW-197 | Bus speed 100kHz | Standard mode comms | All devices respond | ✅ |
| HW-198 | Bus speed 400kHz | Fast mode comms | All devices respond | [ ] |
| HW-199 | Hot-plug device | Connect device while running | Device detected, no crash | [ ] |

---

## 💻 Firmware Testing (FW-2xx)

### FW-200: Sensor Driver Unit Tests
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-201 | STCC4 init | Call `sensors.init()` | Returns ESP_OK, sensors detected | ✅ |
| FW-202 | STCC4 read cycle | Trigger measurement, wait, read | Valid CO2/T/RH values | ✅ |
| FW-203 | STCC4 5s averaging | Push 5 samples | Average calculated correctly | [ ] |
| FW-204 | SGP4x conditioning | First read after init | Conditioning flag set | ✅ |
| FW-205 | SGP4x VOC/NOx read | Call measure_raw_signals() | Valid ticks returned | ✅ |
| FW-206 | SPS30 wake/sleep | Cycle wake/measure/sleep | PM values read between cycles | [ ] |
| FW-207 | Sensor error handling | Disconnect I2C | Error logged, no crash | [ ] |
| FW-208 | Sensor timeout | Block I2C bus | Timeout handled gracefully | [ ] |
| FW-209 | DPS368 init | Initialize pressure sensor | Device ID correct (0x10) | ✅ |
| FW-20A | DPS368 pressure read | Read pressure | Valid hPa value (800-1200) | [ ] |
| FW-20B | DPS368 temperature read | Read temperature | Valid °C value | [ ] |
| FW-20C | DPS368 oversampling | Set high precision mode | Readings more stable | [ ] |
| FW-20D | SHT40 init | Initialize backup T/RH | Device responds | [ ] |
| FW-20E | SHT40 heater test | Activate heater briefly | Temp rises, RH drops | [ ] |

### FW-205: Accelerometer Driver Tests (LIS2DH12)
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-2A1 | LIS2DH12 init | Initialize driver | WHO_AM_I = 0x33 | ✅ |
| FW-2A2 | Read XYZ axes | Get acceleration data | Valid mg values | ✅ |
| FW-2A3 | Set data rate | Configure ODR | Samples at correct rate | [ ] |
| FW-2A4 | Set full scale | Configure ±2g, ±4g, ±8g | Range changes correctly | [ ] |
| FW-2A5 | Motion detection | Configure motion interrupt | Callback fires on movement | [ ] |
| FW-2A6 | Orientation detect | Read orientation | Correct face-up/down | [ ] |
| FW-2A7 | Low-power mode | Enable LP mode | Power consumption drops | [ ] |
| FW-2A8 | FIFO mode | Enable FIFO | Buffer fills correctly | ✅ |

Notes (Jan 14, 2026): LIS2DH12 init and XYZ readouts verified in logs; motion INT tuning in progress.

### FW-206: Charger Driver Tests (BQ25629)
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-2B1 | BQ25629 init | Initialize driver | Device ID readable | ✅ |
| FW-2B2 | Read battery voltage | Get VBAT | Value matches ADC reading | ✅ |
| FW-2B3 | Read charge status | Get charging state | Correct: charging/full/fault | ✅ |
| FW-2B4 | Read input voltage | Get VBUS | Matches USB voltage | ✅ |
| FW-2B5 | Set charge current | Configure ICHG | Register set correctly | ✅ |
| FW-2B6 | Set charge voltage | Configure VREG | Register set correctly | ✅ |
| FW-2B7 | Enter ShipMode | Send ship mode command | Device powers down | ✅ |
| FW-2B8 | Read fault status | Get fault register | Faults reported correctly | [ ] |
| FW-2B9 | NTC status | Read battery temperature | Temp within expected range | ✅ |
| FW-2BA | Watchdog timer | Configure WDT | Timer resets correctly | ✅ |

### FW-207: Flash Storage Driver Tests (W25N512GVEIG)
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-2C1 | Flash init | Initialize SPI driver | JEDEC ID correct | ✅ |
| FW-2C2 | Write page | Write 2KB | Write completes | ✅ |
| FW-2C3 | Read page | Read 2KB | Data matches written | ✅ |
| FW-2C4 | Erase block | Erase 128KB | All 0xFF | ✅ |
| FW-2C5 | Bad block table | Build BBT | Factory BBs identified | ✅ |
| FW-2C6 | Wear leveling | Use FTL layer | Writes distributed | [ ] |
| FW-2C7 | File system mount | Mount LittleFS/SPIFFS | FS available | ✅ |
| FW-2C8 | File write/read | Create file, read back | Content matches | ✅ |
| FW-2C9 | Directory ops | Create/list/delete dir | All ops work | [ ] |
| FW-2CA | Storage stats | Get free/used space | Values accurate | [ ] |
| FW-2CB | Crash recovery | Simulate power loss | FS recovers cleanly | [ ] |

### FW-210: Display/UI Unit Tests
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-211 | Display init | Call `display.init(w,h)` | Returns true, root created | ✅ |
| FW-212 | Set all values | Call all setter methods | Labels update correctly | [ ] |
| FW-213 | Battery icon states | Set 0%, 50%, 100% | Icon changes appropriately | ✅ |
| FW-214 | Charging blink | Set charging=true | Icon blinks at 500ms | [ ] |
| FW-215 | REC blink | Set recording=true | REC label blinks | ✅ |
| FW-216 | Status icons | Set BLE/WiFi/GPS states | Icons update opacity | ✅ |
| FW-217 | Alert indicator | Set alert=true | "!" appears | ✅ |
| FW-218 | Time display | Set valid/invalid time | "HH:MM" or "--:--" shown | ✅ |
| FW-219 | Float formatting | Set 25.67°C | Displays "25.7°C" (rounded) | ✅ |

### FW-220: Recording System Tests
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-221 | Ring buffer init | Create buffer with N slots | Memory allocated | [ ] |
| FW-222 | Buffer write | Write records | Head advances correctly | [ ] |
| FW-223 | Buffer full | Write N+1 records | Oldest record overwritten | [ ] |
| FW-224 | Buffer read | Read all records | Records returned in order | [ ] |
| FW-225 | Interval timer | Set 10s interval | Records at exactly 10s apart | [ ] |
| FW-226 | Pause/resume | Toggle recording | No records during pause | [ ] |
| FW-227 | High watermark | Fill to 90% | Alert flag set | [ ] |
| FW-228 | Persistence | Reboot device | Records survive reset | [ ] |

### FW-230: GPS/Session Tests
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-231 | NMEA parser | Feed valid NMEA strings | Lat/lon/time extracted | ✅ |
| FW-232 | NMEA checksum | Feed invalid checksum | Sentence rejected | ✅ |
| FW-233 | Fix detection | Parse GPGGA with fix | Fix status updated | ✅ |
| FW-234 | No fix handling | Parse GPGGA without fix | Status shows "searching" | ✅ |
| FW-235 | Session start | Long press middle | Session created with ID | [ ] |
| FW-236 | Session stop | Long press middle again | Session closed, summary shown | [ ] |
| FW-237 | GPS in session | Run session with GPS | Points recorded at interval | [ ] |
| FW-238 | Session persistence | Reboot during session | Session recoverable | [ ] |

### FW-240: Tagging Tests
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-241 | Add tag | Navigate Quick Menu → Add Tag | Tag stored with timestamp | [ ] |
| FW-242 | Tag with GPS | Add tag during GPS fix | Tag includes lat/lon | [ ] |
| FW-243 | Tag in session | Add tag during active session | Tag linked to session ID | [ ] |
| FW-244 | Tag categories | Navigate all categories | All options accessible | [ ] |
| FW-245 | Last used tags | Add same tag twice | Appears in "recent" | [ ] |

### FW-250: Button/Menu Tests
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-251 | Up button | Press up | Focus moves up | ✅ |
| FW-252 | Down button | Press down | Focus moves down | ✅ |
| FW-253 | Middle short press | Tap middle | Quick Menu opens | ✅|
| FW-254 | Middle long press | Hold middle 3s | Session toggles | ✅ |
| FW-255 | Down long press | Hold right 3s | Touch lock toggles | ✅ |
| FW-256 | Up long press | Hold left 3s | Enters ShioMode | ✅ |
| FW-257 | Auto-lock | Wait 20s | Touch locked | [ ] |
| FW-258 | Menu navigation | Navigate Quick Menu | All items selectable | [ ] |
| FW-259 | Menu back/cancel | Exit menu | Returns to dashboard | [ ] |

### FW-260: Settings Tests
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-261 | Read settings | Get current settings | All values returned | [ ] |
| FW-262 | Write settings | Change interval to 30s | Value persisted | [ ] |
| FW-263 | Settings persist | Reboot after change | Settings retained | [ ] |
| FW-264 | Factory reset | Clear all data | Defaults restored | [ ] |
| FW-265 | Unit toggle | Switch °C/°F | Display updates | [ ] |
| FW-266 | PM format toggle | Switch µg/m³/USAQI | Display updates | [ ] |

### FW-270: BLE Tests
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-271 | BLE init | Enable BLE | Advertising starts | [ ] |
| FW-272 | Device info char | Read device info | FW version, ID returned | [ ] |
| FW-273 | Settings read | Read settings char | Current settings returned | [ ] |
| FW-274 | Settings write | Write new settings | Settings applied | [ ] |
| FW-275 | WiFi creds write | Write SSID/password | Credentials stored | [ ] |
| FW-276 | Home zone write | Write lat/lon/radius | Zone stored | [ ] |
| FW-277 | Time sync | Write current time | RTC updated | [ ] |
| FW-278 | Data sync request | Request records | Records streamed | [ ] |

### FW-280: WiFi Tests
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-281 | WiFi connect | Connect to configured AP | IP obtained | [ ] |
| FW-282 | Home zone check | GPS fix inside zone | Zone detected | [ ] |
| FW-283 | Auto-sync trigger | In home zone + conditions met | Sync starts | [ ] |
| FW-284 | Sync upload | Upload pending records | Data sent to server | [ ] |
| FW-285 | Sync on BAT | Duty-cycle mode | Connect, upload, disconnect | [ ] |
| FW-286 | Sync timeout | Slow network | Aborts after timeout | [ ] |
| FW-287 | Sync status | After sync attempt | Status/result saved | [ ] |

### FW-290: Alert/LED Tests
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-291 | PM2.5 warning | Set PM > 35 | Warning pattern on LED | [ ] |
| FW-292 | PM2.5 critical | Set PM > 75 | Critical pattern on LED | [ ] |
| FW-293 | CO2 warning | Set CO2 > 1200 | Warning pattern | [ ] |
| FW-294 | CO2 critical | Set CO2 > 2000 | Critical pattern | [ ] |
| FW-295 | VOC warning | Set VOC > 200 | Warning pattern | [ ] |
| FW-296 | Battery warning | Set battery < 20% | Warning pattern | [ ] |
| FW-297 | Battery critical | Set battery < 10% | Critical pattern | [ ] |
| FW-298 | Multiple alerts | Two conditions | Critical takes priority | [ ] |
| FW-299 | LED off on BAT | BAT mode, no alert | LEDs off | [ ] |

---

## 🔗 Integration Testing (FW-3xx)

### FW-300: Sensor-to-Display Flow
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-301 | Live sensor values | Run device | All sensor values update on display | ✅ |
| FW-302 | Sensor averaging | Watch CO2/T/RH | 5s average displayed (not raw) | [ ] |
| FW-303 | Sensor error display | Disconnect sensor | Error indicator shown | [ ] |

### FW-310: Recording Integration
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-311 | Auto-recording | Boot device | Recording starts at default interval | [ ] |
| FW-312 | Recording + display | Run with recording on | UI independent of record interval | [ ] |
| FW-313 | Pause from menu | Quick Menu → Pause | Recording stops, REC shows PAUSE | [ ] |

### FW-320: GPS + Session Integration
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-321 | GPS icon status | Move outdoor/indoor | Icon updates (fix/searching/off) | [ ] |
| FW-322 | Session with GPS | Start session outdoors | GPS points recorded | [ ] |
| FW-323 | Session summary | Stop session | Summary displays correctly | [ ] |

### FW-330: BLE + App Integration
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-331 | Provisioning flow | Connect app, set WiFi | Credentials saved, WiFi connects | [ ] |
| FW-332 | Data sync to app | Sync from app | Records transferred | [ ] |
| FW-333 | Settings from app | Change interval via app | Device updates | [ ] |

---

## 🌐 System/E2E Testing (FW-4xx)

### FW-400: Full User Scenarios
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-401 | Unboxing flow | Power on new device | Boot, display, start recording | [ ] |
| FW-402 | Daily use | Use for 8 hours | Stable, no crashes | [ ] |
| FW-403 | GPS route recording | Walk 1km with session | Route captured with tags | [ ] |
| FW-404 | Home auto-sync | Return to home zone | Auto-sync completes | [ ] |
| FW-405 | Low battery behavior | Drain to 10% | Critical alert, data preserved | [ ] |
| FW-406 | Power cycle | Multiple on/off cycles | Data persists, no corruption | [ ] |

### FW-410: Edge Cases
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-411 | Storage full | Fill ring buffer | Oldest data overwritten, no crash | [ ] |
| FW-412 | No GPS fix | Indoor session | Session works without GPS | [ ] |
| FW-413 | No WiFi | Leave home zone | Offline mode, sync later | [ ] |
| FW-414 | Rapid button press | Spam buttons | No UI glitches | [ ] |
| FW-415 | Sensor disconnect | Unplug during run | Error shown, no crash | [ ] |

---

## 🏋️ Stress/Endurance Testing (FW-5xx)

### FW-500: Long-Duration Tests
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-501 | 24h continuous | Run for 24 hours | No memory leaks, stable | [ ] |
| FW-502 | 7-day battery | BAT mode, 5m interval | Battery lasts 7+ days | [ ] |
| FW-503 | 1000 session cycles | Start/stop 1000 times | No corruption | [ ] |
| FW-504 | Flash wear | 10000 write cycles | No flash errors | [ ] |

### FW-510: Environmental Tests
| ID | Test Case | Procedure | Expected Result | Status |
|----|-----------|-----------|-----------------|--------|
| FW-511 | High temp (40°C) | Operate in warm environment | Stable operation | [ ] |
| FW-512 | Low temp (0°C) | Operate in cold environment | Display/sensors work | [ ] |
| FW-513 | Humidity (80% RH) | High humidity environment | No sensor drift | [ ] |

---

## 📋 Test Execution Summary

### Before Alpha Release
- [ ] All HW-1xx tests pass
- [ ] All FW-2xx core tests pass (200-230)
- [ ] Basic FW-3xx integration pass

### Before Beta Release
- [ ] All FW-2xx tests pass
- [ ] All FW-3xx tests pass
- [ ] FW-401, FW-402 pass

### Before Production Release
- [ ] All FW-4xx tests pass
- [ ] FW-501, FW-502 pass
- [ ] Field testing with 5+ users

---

## 🛠️ Test Equipment Required

| Equipment | Purpose | Quantity |
|-----------|---------|----------|
| Multimeter | Voltage/current measurement | 1 |
| Current clamp / INA219 | Battery current measurement | 1 |
| Reference CO2 meter | Sensor calibration check | 1 |
| Reference PM monitor | SPS30 validation | 1 |
| Calibrated thermometer | Temperature validation | 1 |
| GPS reference device | Position accuracy check | 1 |
| Android/iOS phone | BLE/App testing | 1 each |
| WiFi AP (configurable) | Network testing | 1 |
| Temperature chamber | Environmental testing | 1 (optional) |

---

## Known Constraints & Considerations

### Display Limitations
- E-paper refresh is slow → prefer partial updates
- 144x296 resolution → careful layout planning
- Gray-scale support dependent on SSD1680 driver

### Storage
- Ring buffer size must fit in available flash/PSRAM
- Consider compression for long-term data
- Overflow protection required

### Power Consumption
- 1s recording interval may consume significant battery
- WiFi duty-cycling critical on BAT mode
- GPS continuous use drains battery quickly

### GPS
- NMEA parsing error handling required
- Fix acquisition may take time (cold start)
- Accuracy varies by environment

### Sensor Warm-up
- Some sensors require stabilization time
- Display warm-up status in status bar initially

---

## Notes & Future Enhancements (Post-MVP)

- Map visualization of GPS routes in mobile app
- Data export (CSV/JSON) via BLE or WiFi
- Cloud sync with sharing capabilities
- Route history and replay
- Advanced filtering/querying of tagged data
- Firmware OTA updates
- Persistent session management across power cycles
- Offline map caching for home zone
