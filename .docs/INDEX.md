# 📑 Project Documentation Index

**Last Updated**: January 10, 2026  
**Target Hardware**: ESP32-C5-MINI-1  
**Framework**: ESP-IDF v5.5.1

---

## 🎯 Entry Points by Role

### 👨‍💻 **Firmware Developer**
1. **[QUICKSTART.md](QUICKSTART.md)** - 5-minute common commands
2. **[AGENT.md](AGENT.md)** - Full setup & architecture guide
3. **[ARCHITECTURE/DRIVER_CONTRACTS.md](ARCHITECTURE/DRIVER_CONTRACTS.md)** - Driver implementation standards

### 🏗️ **Hardware Engineer**
1. **[ARCHITECTURE/HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md)** - GPIO pinout & connections
2. **[ARCHITECTURE/POWER_STATES.md](ARCHITECTURE/POWER_STATES.md)** - Power management & states
3. **[REFERENCES/README.md](REFERENCES/README.md)** - Datasheet reference guide

### 📋 **Project Manager**
1. **[ARCHITECTURE/FIRMWARE_PLAN.md](ARCHITECTURE/FIRMWARE_PLAN.md)** - Development phases & progress
2. **[START_HERE.md](START_HERE.md)** - Project overview & specifications

### 🤖 **GitHub Copilot (AI Assistant)**
→ Start with **[AGENT.md](AGENT.md)** and reference other docs as needed

---

## 📚 All Documents

### Core Documentation

| File | Purpose | Audience |
|------|---------|----------|
| [START_HERE.md](START_HERE.md) | Project overview, setup, key specs | Everyone |
| [QUICKSTART.md](QUICKSTART.md) | Fast reference for common tasks | Developers |
| [AGENT.md](AGENT.md) | Full guidelines for AI/devs, architecture, workflow | Developers, AI |
| [COPILOT_INSTRUCTIONS.md](COPILOT_INSTRUCTIONS.md) | GitHub Copilot system prompt | AI Assistant |
| [INDEX.md](INDEX.md) | Documentation index | Everyone |

### Architecture Documentation

| File | Purpose | Audience |
|------|---------|----------|
| [ARCHITECTURE/DRIVER_CONTRACTS.md](ARCHITECTURE/DRIVER_CONTRACTS.md) | Driver interface standards, code patterns | Developers |
| [ARCHITECTURE/HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md) | GPIO assignments, pinout, block diagram | Hardware & Firmware |
| [ARCHITECTURE/POWER_STATES.md](ARCHITECTURE/POWER_STATES.md) | Power management, ship mode, charging | Firmware & Hardware |
| [ARCHITECTURE/FIRMWARE_PLAN.md](ARCHITECTURE/FIRMWARE_PLAN.md) | Development roadmap, phases, progress | PMs & Developers |

### Reference Documentation

| File | Purpose |
|------|---------|
| [REFERENCES/README.md](REFERENCES/README.md) | Datasheet guide, component reference |

### Configuration Files

| File | Purpose |
|------|---------|
| [.clang-format](.clang-format) | Code style: 2-space indent, 100 char limit |
| [.gitignore](.gitignore) | Git ignore rules for build artifacts |
| [partitions.csv](partitions.csv) | Flash partition table with OTA support |

### Documentation Assets

| Path | Contents |
|------|----------|
| `docs/pdf/` | Original PDF datasheets (15 files) |
| `docs/datasheet_md/` | Generated markdown datasheets (use markitdown) |

---

## 🔗 Cross-Reference Guide

### For Task: "Add a new sensor driver"
1. Read: [ARCHITECTURE/DRIVER_CONTRACTS.md](ARCHITECTURE/DRIVER_CONTRACTS.md) - Interface requirements
2. Reference: [ARCHITECTURE/HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md) - GPIO assignments
3. Look up: [REFERENCES/README.md](REFERENCES/README.md) - Sensor datasheet
4. Follow: [AGENT.md](AGENT.md) - Component structure
5. Check: [QUICKSTART.md](QUICKSTART.md) - Build & test

### For Task: "Debug power consumption"
1. Read: [ARCHITECTURE/POWER_STATES.md](ARCHITECTURE/POWER_STATES.md) - Power states overview
2. Reference: [ARCHITECTURE/HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md) - Power component pins
3. Check: [ARCHITECTURE/FIRMWARE_PLAN.md](ARCHITECTURE/FIRMWARE_PLAN.md) - Current infrastructure status

### For Task: "Set up development environment"
1. Read: [START_HERE.md](START_HERE.md) - Quick setup section
2. Follow: [AGENT.md](AGENT.md) - Detailed environment setup
3. Use: [QUICKSTART.md](QUICKSTART.md) - Common commands

### For Task: "Review GPIO usage"
1. Check: [ARCHITECTURE/HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md) - Complete GPIO table
2. Cross-reference: [ARCHITECTURE/POWER_STATES.md](ARCHITECTURE/POWER_STATES.md) - Power control pins
3. Implementation: Drivers in `components/`

---

## 🎓 Key Concepts by Document

### README.md
- Project overview
- Hardware specifications
- Quick navigation
- Development progress tracking

### AGENT.md (→ **START HERE for AI**)
- Environment setup (esp-idf, Python)
- Project structure walkthrough
- Component development workflow
- Build & flash procedures
- Driver development patterns
- Code style enforcement
- Debugging tips
- Pre-commit checklist

### DRIVER_CONTRACTS.md
- `IDriver` interface definition
- State enumeration (UNINITIALIZED → ERROR)
- `DriverData` structure
- Standard method signatures
- Error handling patterns
- Example implementations

### HARDWARE_MAP.md
- Block diagram
- GPIO assignments (IO0-IO40)
- SPI bus configuration
- I2C address space
- Peripheral connections
- Power control pins

### POWER_STATES.md
- Ship Mode (< 1µA)
- Active Mode
- Charging state
- QON button integration
- BQ25628 charger control
- Battery management

### FIRMWARE_PLAN.md
- 8-phase development roadmap
- Current progress (~8% MVP)
- Infrastructure status
- Hardware component list
- Next priorities

### QUICKSTART.md
- 5-minute setup
- Most common commands
- Hardware reference tables
- Code templates
- Troubleshooting guide

### docs/README.md
- How to convert PDFs to markdown
- Datasheet inventory (15 files)
- Architecture references
- Component datasheets organized by type
- Best practices for using datasheets

---

## 🔍 How to Find Information

**"I need to use I2C..."**  
→ See [ARCHITECTURE/HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md) "I2C Sensors" section

**"What's the GPIO for the button?"**  
→ Check [ARCHITECTURE/HARDWARE_MAP.md](ARCHITECTURE/HARDWARE_MAP.md) or [ARCHITECTURE/POWER_STATES.md](ARCHITECTURE/POWER_STATES.md)

**"How do I create a driver?"**  
→ Follow [ARCHITECTURE/DRIVER_CONTRACTS.md](ARCHITECTURE/DRIVER_CONTRACTS.md) + [AGENT.md](AGENT.md)

**"How do I build & flash?"**  
→ Use [QUICKSTART.md](QUICKSTART.md) or [AGENT.md](AGENT.md)

**"Where's the SSD1680 datasheet?"**  
→ `REFERENCES/pdf/SSD1680.pdf` or convert to markdown per [REFERENCES/README.md](REFERENCES/README.md)

**"What's the development status?"**  
→ See [ARCHITECTURE/FIRMWARE_PLAN.md](ARCHITECTURE/FIRMWARE_PLAN.md) progress table

**"How do I format code?"**  
→ [QUICKSTART.md](QUICKSTART.md) has the command; [AGENT.md](AGENT.md) explains the rules

---

## 📊 Documentation Statistics

- **Total Documents**: 8 markdown files + 1 CSV + 2 config files
- **PDF Datasheets**: 15 technical sheets
- **GPIO Pins Documented**: 25+ pins
- **I2C Addresses**: 6 sensors
- **Development Phases**: 8 phases planned

---

## 🚀 Next Steps

1. **New Developer?** → Start with [QUICKSTART.md](QUICKSTART.md)
2. **GitHub Copilot?** → Read [AGENT.md](AGENT.md)
3. **Adding Hardware?** → Check [HARDWARE_MAP.md](HARDWARE_MAP.md)
4. **Creating Driver?** → Follow [DRIVER_CONTRACTS.md](DRIVER_CONTRACTS.md)
5. **Converting Datasheets?** → Use [docs/README.md](docs/README.md)
6. **Progress Tracking?** → Update [FIRMWARE_PLAN.md](FIRMWARE_PLAN.md)

---

**All files are cross-referenced and maintained together.**  
**Last verified**: January 10, 2026
