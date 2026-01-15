# AirGradient GO - Hardware Map

## 🔌 Block Diagram Overview

![Block Diagram](/Users/formylife/.gemini/antigravity/brain/48d45a61-db12-49e8-b7e5-a5960ff21b8d/uploaded_image_0_1768011912977.jpg)

---

## 🧠 MCU: ESP32-C5-MINI-1

![ESP32-C5-MINI-1 Schematic](/Users/formylife/.gemini/antigravity/brain/48d45a61-db12-49e8-b7e5-a5960ff21b8d/uploaded_image_1_1768011912977.png)

---

## 📋 GPIO Pin Assignments

| GPIO | Function | Direction | Notes |
|------|----------|-----------|-------|
| IO0 | ePaper CS | Output | E-paper chip select |
| IO1 | INT (Second Board) | Input | Interrupt from second board |
| IO2 | WD_RST | Output | Watchdog reset (TPL5010) |
| IO3 | ACC_INT | Input | Accelerometer interrupt (LIS2DH12) |
| IO4 | NAND_CS | Output | Flash memory chip select (W25N512GVEIG) |
| IO5 | QON | Input/Output | Charger QON pin (BQ25628) |
| IO6 | SCL | I/O | I2C Clock |
| IO7 | SDA (via Jumper) | I/O | I2C Data - Jumper selectable with IO8 |
| IO8 | SDA (via Jumper) | I/O | I2C Data - Jumper selectable with IO7 |
| IO9 | ePaper RES | Output | E-paper reset |
| IO10 | ePaper BUSY | Input | E-paper busy signal |
| IO13 | USB- | I/O | USB Data Minus |
| IO14 | USB+ | I/O | USB Data Plus |
| IO15 | ePaper D/C | Output | E-paper Data/Command |
| IO23 | SCLK | Output | SPI Clock (E-paper + Flash) |
| IO24 | MISO | Input | SPI MISO |
| IO25 | MOSI | Output | SPI MOSI |
| IO26 | EN_PM1 | Output | PM Sensor power + I2C isolate enable |
| IO27 | - | - | Available |
| IO28 | - | - | Available |
| TXD0 (IO39) | MCU_TX | Output | UART TX (GPS NMEA) |
| RXD0 (IO40) | MCU_RX | Input | UART RX (GPS NMEA) |

---

## 🔌 Peripheral Connections

### SPI Bus
| Device | CS Pin | SCK | MOSI | MISO | Notes |
|--------|--------|-----|------|------|-------|
| E-paper (SSD1680) | IO0 | IO23 | IO25 | - | 144x296 display |
| Flash (W25N512GVEIG) | IO4 | IO23 | IO25 | IO24 | 512Mbit NAND |

### E-paper Display (SSD1680 144x296)
| Signal | GPIO | Notes |
|--------|------|-------|
| CS | IO0 | Chip Select |
| SCLK | IO23 | SPI Clock |
| MOSI | IO25 | SPI Data |
| D/C | IO15 | Data/Command |
| RES | IO9 | Reset |
| BUSY | IO10 | Busy Signal |

### I2C Bus (Main)
| GPIO | Function |
|------|----------|
| IO6 | SCL |
| IO7 | SDA |

### I2C Device Map
| Device | Address | Notes |
|--------|---------|-------|
| STCC4/SCD4x | 0x62 | CO2 + Temp + RH (all-in-one, current implementation) |
| SCD41 | 0x62 | CO2 only (alternative, same address family) |
| SHT40 | 0x44 | Temp + RH backup (optional, not currently used). Alt: 0x45, 0x46 |
| SGP41 | 0x59 | VOC + NOx sensor |
| DPS368 | 0x76 | Pressure sensor. Alt: 0x77 (SDO pin) |
| LIS2DH12 | 0x18 | Accelerometer. Alt: 0x19 (SA0/SDO pin) |
| CAP1203 | 0x28 | Capacitive buttons (3x) |
| LP5030/LP5036 | 0x30 | LED driver. Range: 0x30-0x33 (ADDR0/ADDR1) |
| BQ25628 | 0x6A | Battery charger |
| SPS30 | 0x69 | PM sensor (primary) |
| PMSA003I | 0x12 | PM sensor (alternative) |

### UART (GPS)
| Signal | GPIO | Notes |
|--------|------|-------|
| MCU_TX | TXD0 (IO39) | To GPS RX |
| MCU_RX | RXD0 (IO40) | From GPS TX (NMEA) |

---

## ⚡ Power Management

### Power Path Overview
```
┌─────────────────────────────────────────────────────────────────────────┐
│                           POWER PATH                                    │
└─────────────────────────────────────────────────────────────────────────┘

  ┌─────────┐         ┌─────────────┐         ┌─────────────────┐
  │  USB-C  │────────►│  BQ25628    │◄───────►│    Battery      │
  └─────────┘         │  (Charger)  │         └────────┬────────┘
                      └──────┬──────┘                  │
                             │                         │
              ┌──────────────┼                         │
              │              │                         │
              ▼              ▼                         ▼
        ┌──────────┐   ┌──────────┐             ┌──────────────┐
        │   SYS    │   │   PMID   │             │  TPS7A0228   │
        └────┬─────┘   └────┬─────┘             │  (2.8V LDO)  │
             │              │                   └──────────────┘
             ▼              ▼                
     ┌──────────────┐  ┌──────────────┐      
     │ TPS63802DLAR │  │  TPS27081A   │      
     │ (3.1V DC-DC) │  │ (Load Switch)│      
     └──────┬───────┘  │ EN: IO26     │      
            │          └──────┬───────┘      
            ▼                 │              
     ┌──────────────┐         ▼              
     │ System Power │  ┌──────────────┐      
     │ (MCU, etc.)  │  │  PM Sensor   │      
     └──────────────┘  │(SPS30/PMSA)  │      
                       └──────────────┘      
                                             
```

> [!NOTE]
> - **SYS** (from charger): Powers the main DC-DC converter (3.1V) for system operation
> - **PMID** (from charger): Powers the PM sensor via load switch
> - **Battery direct**: Powers the 2.8V LDO (TPS7A0228) directly, bypassing the charger

### PM Sensor Power Control
| Control | Component | Function |
|---------|-----------|----------|
| IO26 (EN_PM1) | TPS27081A | PM Sensor power enable |
| IO26 (EN_PM1) | TMUX121 | I2C bus isolation enable |

> [!IMPORTANT]
> Both the load switch (TPS27081A) and I2C isolator (TMUX121) are controlled by the same EN_PM1 signal.
> When PM sensor is OFF, I2C bus is disconnected to prevent bus issues.

### Hardware Watchdog
| Signal | GPIO | Notes |
|--------|------|-------|
| WD_RST | IO2 | TPL5010DDCR reset pulse |

---

## 📡 Sensor Auto-Switch Capability

### CO2 + Temp/RH Sensors

Sensor usage logic for STCC4/SCD4x and SHT40:

```
┌─────────────────────────────────────────────────────────────┐
│  Current Implementation (Jan 2026)                          │
│  ────────────────────────────────                           │
│  STCC4/SCD4x @ 0x62 → CO2 + Temp + RH (all-in-one)         │
│  SHT40 → Not currently used (optional backup)               │
│                                                             │
│  Note: STCC4 is a variant of SCD4x family with built-in    │
│  temperature and humidity sensors. No external SHT40 needed.│
└─────────────────────────────────────────────────────────────┘
```

> [!NOTE]
> **Current Implementation**: Using STCC4 (SCD4x variant) at address 0x62 which provides CO2, temperature, and humidity in one sensor. The SHT40 is optional and not currently utilized in firmware. Firmware uses the STCC4's built-in temperature/humidity readings with 5-second moving average.

### PM Sensors
The firmware can auto-switch between:
- **SPS30** (Primary, I2C address 0x69)
- **PMSA003I** (Alternative)

---

## 🔗 Special Connections

### SHT40 Connection
```
┌──────────┐      C_I2C      ┌──────────┐
│  STCC4   │◄───────────────►│  SHT40   │
│ (SCD4x)  │                 │ (Temp/RH)│
└────┬─────┘                 └──────────┘
     │
   I2C (Main bus)
```

> [!IMPORTANT]
> **Current Implementation (Jan 2026)**:
> - Using **STCC4 (SCD4x variant)**: Reads CO2 + Temp + RH directly from STCC4
> - **SHT40**: Optional backup sensor, not currently utilized in firmware
> - Firmware implements 5-second moving average for CO2/Temp/RH readings

### I2C Isolation for PM Sensors (TMUX121)
```
                    EN_PM1 (IO26)
                         │
                         ▼
                    ┌─────────┐
Main I2C Bus ◄─────►│ TMUX121 │◄─────► SPS30/PMSA003I
                    └─────────┘
```

> [!NOTE]
> The TMUX121 I2C isolator is controlled by the same EN_PM1 signal as the PM sensor power.
> - **EN_PM1 = HIGH**: Power ON + I2C connected (can communicate with PM sensor)
> - **EN_PM1 = LOW**: Power OFF + I2C disconnected (protects main I2C bus)

### Not Gate (SN74LVC1G06)
Used for signal inversion in the PM sensor power control path.

---

## 🔋 Charger Details (BQ25629)

| Setting | Value | Notes |
|---------|-------|-------|
| VINDPM (Input Voltage Limit) | 4.6V | Works with 5V input |
| IINDPM (Input Current Limit) | 1.5A | Maximum input current |
| Charge Voltage | 4.2V | Li-ion full charge |
| Charge Current | 0.5A | Main charging current |
| Min System Voltage | 3.52V | Minimum VSYS |
| Termination Current | 20mA | End-of-charge threshold |

| Pin | Function |
|-----|----------|
| SYS | System power output (to DC-DC 3.1V) |
| PMID | PM Sensor power source (to load switch) |
| QON (IO5) | Ship Mode control |
| NTC | Battery temperature sensing |

---

## 📊 Interface Summary

| Interface | Devices |
|-----------|---------|
| **SPI** | E-paper (SSD1680), Flash (W25N512GVEIG) |
| **I2C** | STCC4, SCD41, SHT40, SGP4x, DPS368, LIS2DH12, CAP1203, LP5030/36, BQ25628 |
| **UART** | GPS (NMEA) |
| **GPIO** | Watchdog (TPL5010), Load Switch (TPS27081A), I2C Isolate (TMUX121), Accelerometer INT |
