# 📚 Documentation Setup & Reference Guide

> **Purpose**: Centralized guide for accessing and generating documentation from datasheets and technical resources.

---

## 🎯 Quick Start

### Convert PDF Datasheets to Markdown

Install **markitdown**:
```bash
pip install markitdown
```

Convert any PDF datasheet to Markdown format:
```bash
markitdown docs/pdf/<datasheet>.pdf > docs/datasheet_md/<datasheet>.md
```

---

## 📋 Available Datasheets

### Microcontroller
- **esp32-c5-mini-1_datasheet_en.pdf** - ESP32-C5-MINI-1 MCU specification
  - GPIO mapping
  - Memory configuration
  - Peripheral specifications

### Display & UI
- **SSD1680.pdf** - E-paper display controller (144x296)
  - Command set
  - Timing diagrams
  - Display refresh modes

### Storage
- **W25N512GV.pdf** - 512 Mbit NAND Flash memory
  - SPI interface specification
  - Read/Write commands
  - Timing specifications

### Power Management & Charging
- **bq25629.pdf** - Battery charge controller (BQ25628)
  - Charging algorithms
  - Thermal management
  - Register mapping

### Sensors (Sensirion)
- **Sensirion_Datasheet_SHT4x.pdf** - Temperature & humidity sensor
  - I2C communication protocol
  - Measurement specifications
  - Power consumption

- **Sensirion_Gas_Sensors_Datasheet_SGP41.pdf** - VOC sensor
  - Gas sensing principles
  - Output format
  - Operating conditions

- **Sensirion_PM_Sensors_Datasheet_SPS30.pdf** - Particulate matter sensor
  - Particle size classification
  - UART/I2C protocol
  - Calibration

- **Sensirion_SCD4x_Datasheet.pdf** - CO₂ sensor
  - Non-dispersive infrared (NDIR) technology
  - I2C interface
  - Measurement accuracy

### Other Sensors
- **LIS2DH12.pdf** - 3-axis accelerometer
  - Interrupt configuration
  - Data formats
  - Operating modes

- **4505_PMSA003I_series_data_manual_English_V2.6.pdf** - PM sensor (alternative)
  - UART protocol
  - Particulate measurement

### Input/Control
- **CAP1203-1-AC3-TR.pdf** - Capacitive button controller
  - 3-channel capacitive touch input
  - I2C interface
  - Interrupt handling

### Sensors (Other Manufacturers)
- **infineon-dps368-datasheet-en.pdf** - Barometric pressure sensor
  - Altitude calculation
  - I2C/SPI interface
  - Temperature compensation

- **T-5-2208-TAU1113 Datasheet-V1.4.pdf** - (TBD - Check folder for details)

- **lp5036.pdf** - LED driver
  - PWM control
  - Color mixing
  - Current regulation

---

## 🔧 Architecture References

### Core Documents
1. **[AGENT.md](../AGENT.md)** - GitHub Copilot instructions & project guidelines
2. **[DRIVER_CONTRACTS.md](../DRIVER_CONTRACTS.md)** - Driver interface standards
3. **[HARDWARE_MAP.md](../HARDWARE_MAP.md)** - GPIO & peripheral pinout
4. **[POWER_STATES.md](../POWER_STATES.md)** - Power management & states
5. **[FIRMWARE_PLAN.md](../FIRMWARE_PLAN.md)** - Development roadmap

---

## 📖 Using Datasheet Documentation

### Workflow

1. **Identify required IC** - Check [HARDWARE_MAP.md](../HARDWARE_MAP.md)
2. **Locate PDF** - Find in `docs/pdf/`
3. **Convert to Markdown** (optional):
   ```bash
   markitdown docs/pdf/SSD1680.pdf > docs/datasheet_md/SSD1680.md
   ```
4. **Reference in code** - Include converted .md files in code comments for quick lookup
5. **Update documentation** - As you implement drivers, document key findings

---

## 🎓 Key Technical Concepts by Component

### Display System (SSD1680 E-Paper)
- **Technology**: Electrophoretic display (e-ink)
- **Resolution**: 144×296 pixels
- **Interface**: 4-wire SPI
- **Refresh**: Full & partial modes
- **Power**: Ultra-low in standby

**Relevant Pins** (from HARDWARE_MAP.md):
- IO0: Chip Select (CS)
- IO23: Clock (SCLK)
- IO25: Data (MOSI)
- IO9: Reset (RES)
- IO15: Data/Command (D/C)
- IO10: Busy status (BUSY)

### Storage System (W25N512 NAND Flash)
- **Capacity**: 512 Mb (64 MB)
- **Interface**: SPI
- **Page Size**: 2 KB
- **Block Size**: 64 KB
- **Typical Use**: Data logging, firmware storage

**Relevant Pins**:
- IO4: Chip Select (CS)
- IO23: Clock (SCLK)
- IO25: Data (MOSI)
- IO24: Data (MISO)

### Power Management (BQ25628)
- **Type**: Integrated battery charger & power manager
- **Input**: USB 5V or external power
- **Output**: 3.7V Li-ion support
- **Features**: Thermal regulation, ShipMode for ultra-low standby
- **Key Pin**: IO5 (QON) for power button integration

### Sensor Bus (I2C)
**Address Space** (7-bit addresses):
- **Temperature/Humidity (SHT4x)**: 0x44
- **VOC (SGP41)**: 0x59
- **PM Sensor (SPS30)**: 0x69
- **Accelerometer (LIS2DH12)**: 0x18 or 0x19 (selectable)
- **Barometer (DPS368)**: 0x76 or 0x77 (selectable)
- **Capacitive Touch (CAP1203)**: 0x28 (default)

**Relevant Pins**:
- IO6: I2C Clock (SCL)
- IO7/IO8: I2C Data (SDA) - jumper selectable

---

## 🔍 Finding Information in Datasheets

### Common Sections in Datasheets

| Section | Information |
|---------|-------------|
| **Pin Configuration** | Physical package, pin assignments |
| **Electrical Characteristics** | Voltage, current, timing specs |
| **Functional Description** | How the device works, modes of operation |
| **Register Map** | Control and status registers (for I2C/SPI devices) |
| **Communication Protocol** | SPI, I2C, UART frame formats |
| **Timing Diagrams** | Signal relationships and timing requirements |
| **Application Circuits** | Recommended component values and layout |
| **Thermal Management** | Temperature ranges, heat dissipation |

---

## 📝 Best Practices

1. **Read datasheets before coding** - Understand electrical characteristics first
2. **Convert complex PDFs to .md** - Makes them searchable and version-controllable
3. **Document non-obvious implementations** - Include datasheet sections in code comments
4. **Keep datasheets organized** - PDF folder for originals, datasheet_md for generated versions
5. **Cross-reference with HARDWARE_MAP** - Verify pin assignments before implementation

---

## 🔗 External Resources

- **ESP-IDF Component Registry**: https://components.espressif.com/
- **Sensirion Sensor Library**: https://github.com/Sensirion/arduino-i2c-sgp41
- **LVGL Documentation**: https://docs.lvgl.io/
- **Espressif Documentation**: https://docs.espressif.com/

---

**Last Updated**: January 10, 2026  
**Version**: 1.0
