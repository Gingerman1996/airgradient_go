# 🌍 AirGradient GO Firmware

Professional air quality monitoring device using **ESP32-C5-MINI-1** with e-paper display, multiple sensors, and smart power management.

---

## 📋 Quick Navigation

### 🤖 **For Developers (GitHub Copilot)**
Start here: **[AGENT.md](AGENT.md)**
- Environment setup instructions
- Project structure overview
- Component development workflow
- Build & flash commands
- Code style guidelines

### 🔌 **Hardware Reference**
- [HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md) - GPIO assignments, pinout, peripheral connections
- [POWER_STATES.md](ARCHITECTURE/POWER_STATES.md) - Power management, ship mode, OTA updates

### 🏗️ **Architecture & Standards**
- [DRIVER_CONTRACTS.md](ARCHITECTURE/DRIVER_CONTRACTS.md) - Driver interface standards
- [FIRMWARE_PLAN.md](ARCHITECTURE/FIRMWARE_PLAN.md) - Development roadmap & progress tracking

### 📚 **Documentation**
- [REFERENCES/README.md](REFERENCES/README.md) - Datasheet reference guide, markitdown setup

## ⚠️ Important: 4 MB Flash Limitation

Your ESP32-C5-MINI-1 has **4 MB Flash**, which is limited. The partition table has been optimized for this:

```
NVS:      20 KB   (Settings)
OTA_0:    1 MB    (Current app)
OTA_1:    1 MB    (Backup app for OTA updates)
RAMDISK:  ~1.8 MB (Data storage)
```

### Build Optimization Tips:
- Use **size optimization**: `-Os` instead of `-O2`
- Enable **LTO** (Link Time Optimization)
- Remove unnecessary features in menuconfig
- Monitor binary size: `idf.py size`

---

### 1. Install ESP-IDF
```bash
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git v5.5.1
cd v5.5.1
git checkout v5.5.1
./install.sh esp32c5
```

### 2. Activate Environment (Every Terminal Session)
```bash
source ~/esp/v5.5.1/esp-idf/export.sh
```

### 3. Install Python Dependencies
```bash
pip install markitdown clang-format
```

### 4. Build Project
```bash
idf.py build
```

### 5. Flash Device
```bash
idf.py flash monitor
```

---

## 🗂️ Project Structure

```
airgradient-go/
│
├── � .config/                      ← Configuration files
│   ├── .clang-format                ← Code style config
│   ├── .gitignore                   ← Git ignore rules
│   └── partitions.csv               ← Partition table (OTA)
│
├── 📁 .docs/                        ← All documentation
│   ├── START_HERE.md                ← This file
│   ├── AGENT.md                     ← Copilot instructions
│   ├── QUICKSTART.md                ← 5-min command reference
│   ├── INDEX.md                     ← Documentation index
│   ├── COPILOT_INSTRUCTIONS.md      ← AI system prompt
│   │
│   ├── 📁 ARCHITECTURE/             ← Architecture docs
│   │   ├── DRIVER_CONTRACTS.md
│   │   ├── FIRMWARE_PLAN.md
│   │   ├── HARDWARE_MAP.md
│   │   └── POWER_STATES.md
│   │
│   └── 📁 REFERENCES/               ← Datasheet reference
│       ├── README.md                ← Datasheet guide
│       ├── pdf/                     ← Original datasheets
│       └── datasheet_md/            ← Generated markdown
│
├── 📁 main/                         ← Application source code
│   ├── CMakeLists.txt
│   ├── main.cpp
│   ├── app/
│   ├── display/
│   ├── sensors/
│   └── power/
│
├── 📁 components/                   ← Reusable driver components
│   ├── <driver-1>/
│   ├── <driver-2>/
│   └── [add more as needed]
│
├── CMakeLists.txt                   ← Project-level CMake
└── idf_component.yml                ← Component metadata
```

---

## 🔧 Common Commands

### Build
```bash
source ~/esp/v5.5.1/esp-idf/export.sh
idf.py build
```

### Flash & Monitor
```bash
idf.py flash monitor
```

### Clean Build
```bash
idf.py fullclean && idf.py build
```

### Check Code Size
```bash
idf.py size
idf.py size-components
```

### Format Code
```bash
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

---

## 📊 Key Specifications

| Aspect | Details |
|--------|---------|
| **MCU** | ESP32-C5-MINI-1 (WiFi + BLE) |
| **Flash** | 4 MB (with W25N512 NAND for storage) |
| **Display** | SSD1680 e-paper, 144×296 px |
| **Sensors** | SHT4x, SGP41, SPS30, LIS2DH12, DPS368 |
| **Power** | Li-ion battery, BQ25628 charger, external watchdog |
| **UI Framework** | LVGL with e-paper partial refresh |

---

## 📈 Development Progress

**Overall Status**: ~8% MVP Complete

| Phase | Status | Description |
|-------|--------|-------------|
| Phase 1 | 🔴 In Progress | UI & display basics |
| Phase 2 | 🔴 Not Started | Data recording system |
| Phase 3 | 🔴 Not Started | GPS & session management |
| Phase 4 | 🔴 Not Started | Tagging & notes |
| Phase 5 | 🔴 Not Started | LED indicators & alerts |
| Phase 6 | 🔴 Not Started | Settings menu |
| Phase 7 | 🔴 Not Started | Bluetooth (BLE) |
| Phase 8 | 🔴 Not Started | WiFi auto-sync |

**Infrastructure**: ✅ Complete
- Display driver & LVGL integration
- Sensor interfaces
- Power management
- Driver framework

---

## 🎨 Code Style Guide

**Format**: LLVM with 2-space indentation  
**Max Line Length**: 100 characters  
**No Tabs**: Use spaces only

```cpp
// Good
if (condition) {
  statement();
}

// Bad
if(condition)
{
    statement();
}
```

Always run formatter before committing:
```bash
clang-format -i <modified-files>
```

---

## 🔐 Partition Layout (OTA Support)

The device supports over-the-air (OTA) firmware updates:

```
0x0000_0000 ┌─────────────────────┐
            │   Bootloader        │
0x0000_9000 ├─────────────────────┤
            │   NVS (20 KB)       │
0x000E_0000 ├─────────────────────┤
            │   OTA_0 (2 MB)      │ ← Current app
0x0200_E000 ├─────────────────────┤
            │   OTA_1 (2 MB)      │ ← Backup app
0x040E_0000 ├─────────────────────┤
            │   RAMDISK (1.9 MB)  │ ← Data storage
0x0600_0000 └─────────────────────┘
```

---

## 📞 Resources

- **ESP-IDF Docs**: https://docs.espressif.com/projects/esp-idf/
- **LVGL**: https://docs.lvgl.io/
- **Component Registry**: https://components.espressif.com/

---

## 📝 Contributing

1. Read [AGENT.md](AGENT.md) for guidelines
2. Follow code style (see [.clang-format](.clang-format))
3. Implement drivers per [DRIVER_CONTRACTS.md](DRIVER_CONTRACTS.md)
4. Update [FIRMWARE_PLAN.md](FIRMWARE_PLAN.md) progress
5. Document in code comments

---

**Last Updated**: January 10, 2026  
**Version**: 1.0  
**MCU Target**: ESP32-C5-MINI-1  
**Build System**: ESP-IDF v5.5.1
