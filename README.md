# 🌍 AirGradient GO Firmware

Professional air quality monitoring device using **ESP32-C5-MINI-1** with e-paper display, multiple sensors, and smart power management.

---

## 📋 Quick Navigation

👉 **All documentation is organized in [`.docs/`](.docs/) folder**

### 🚀 Start Here
- **New developer?** → Read [`.docs/START_HERE.md`](.docs/START_HERE.md)
- **GitHub Copilot?** → Read [`.docs/AGENT.md`](.docs/AGENT.md)
- **Need quick help?** → Check [`.docs/QUICKSTART.md`](.docs/QUICKSTART.md)
- **Find anything?** → Use [`.docs/INDEX.md`](.docs/INDEX.md)

### 🔌 Hardware & Architecture
- **GPIO reference** → [`.docs/ARCHITECTURE/HARDWARE_MAP.md`](.docs/ARCHITECTURE/HARDWARE_MAP.md)
- **Power states** → [`.docs/ARCHITECTURE/POWER_STATES.md`](.docs/ARCHITECTURE/POWER_STATES.md)
- **Driver standards** → [`.docs/ARCHITECTURE/DRIVER_CONTRACTS.md`](.docs/ARCHITECTURE/DRIVER_CONTRACTS.md)
- **Development plan** → [`.docs/ARCHITECTURE/FIRMWARE_PLAN.md`](.docs/ARCHITECTURE/FIRMWARE_PLAN.md)

### 📚 Datasheets & References
- **Datasheet guide** → [`.docs/REFERENCES/README.md`](.docs/REFERENCES/README.md)
- **PDFs** → [`.docs/REFERENCES/pdf/`](.docs/REFERENCES/pdf/) (15 datasheets)

### 🛠️ Configuration
- **Code style** → [`.config/.clang-format`](.config/.clang-format)
- **Git ignore** → [`.config/.gitignore`](.config/.gitignore)
- **Partitions** → [`.config/partitions.csv`](.config/partitions.csv) (OTA support)

---

## 📁 Project Structure

```
airgradient-go/
├── .config/                 ← Configuration files
├── .docs/                   ← Complete documentation
│   ├── ARCHITECTURE/        ← Specs & standards
│   └── REFERENCES/          ← Datasheets & guides
├── main/                    ← Application code (create when ready)
├── components/              ← Reusable drivers (create when ready)
├── README.md                ← This file
├── CMakeLists.txt          ← Project build config
└── idf_component.yml       ← Component metadata
```

---

## ⚠️ Flash Memory Optimization (4 MB)

Your device has **4 MB Flash** (not 8 MB). The partition table is optimized:

| Partition | Size | Purpose |
|-----------|------|---------|
| Bootloader | 64 KB | Auto |
| NVS | 20 KB | Settings storage |
| OTA_0 | 1 MB | Current firmware |
| OTA_1 | 1 MB | OTA backup |
| RAMDISK | 1.8 MB | Data storage |

### Optimization Tips:
```bash
# Check binary size
idf.py size

# Size per component
idf.py size-components
```

Enable in `idf.py menuconfig`:
- **Compiler optimization**: `-Os` (size)
- **LTO**: Link Time Optimization
- **Remove**: Debug symbols, assertions in release build

---

```bash
# 1. Activate ESP-IDF environment
source ~/esp/v5.5.1/esp-idf/export.sh

# 2. Install Python tools
pip install markitdown clang-format

# 3. Build project
idf.py build

# 4. Flash & monitor
idf.py flash monitor
```

---

## 📊 Key Info

| Item | Details |
|------|---------|
| **MCU** | ESP32-C5-MINI-1 (WiFi + BLE) |
| **Flash** | 4 MB (W25N512 NAND for storage) |
| **Display** | SSD1680 e-paper (144×296px) |
| **Sensors** | SHT4x, SGP41, SPS30, LIS2DH12, DPS368 |
| **GPS** | TAU1113 (NMEA via UART) |
| **GPS Parser** | libnmea (ESP-IDF component) |
| **Power** | Li-ion battery with BQ25628 charger |
| **Build System** | ESP-IDF v5.5.1 |
| **Language** | C++ (app), C (drivers) |

---

## ✅ What's Ready

✓ Complete documentation (3,572 lines)  
✓ Architecture & design standards  
✓ Hardware mapping & GPIO reference  
✓ Configuration files (.clang-format, .gitignore, partitions.csv)  
✓ 15 organized datasheets  
✓ GitHub Copilot integration guide  
✓ Quick reference & tutorials  

---

## 🚀 Next Steps

1. **Read** [`.docs/START_HERE.md`](.docs/START_HERE.md) for overview
2. **Set up** ESP-IDF environment (see Quick Setup above)
3. **Create** `main/` and `components/` directories when ready
4. **Follow** architecture docs when adding features

---

## 📞 Documentation Index

- **Getting Started** → [`.docs/START_HERE.md`](.docs/START_HERE.md)
- **Developer Guide** → [`.docs/AGENT.md`](.docs/AGENT.md)
- **Quick Commands** → [`.docs/QUICKSTART.md`](.docs/QUICKSTART.md)
- **All Documents** → [`.docs/INDEX.md`](.docs/INDEX.md)

---

**Last Updated**: January 10, 2026  
**Target**: ESP32-C5-MINI-1  
**Status**: Ready for development ✅
