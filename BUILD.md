# Building AirGradient GO Firmware

## Prerequisites

- **ESP-IDF v5.5.1** installed and configured
- **ESP32-C5-MINI-1** device with 4 MB Flash
- **Python 3.7+** for build tools
- **Clang-format** for code style (optional)

## Quick Start Build

```bash
# 1. Set up IDF environment
source $HOME/esp/esp-idf/export.sh

# 2. Set target to ESP32-C5
idf.py set-target esp32c5

# 3. Configure project (opens menuconfig)
idf.py menuconfig

# 4. Build firmware
idf.py build

# 5. Flash to device (adjust COM port as needed)
idf.py -p /dev/ttyUSB0 flash

# 6. Monitor serial output
idf.py -p /dev/ttyUSB0 monitor
```

## Build Configuration

### Default Settings (sdkconfig.defaults)

The project includes `sdkconfig.defaults` with pre-configured settings:

- **Chip**: ESP32-C5
- **Flash Size**: 4 MB (CONFIG_ESPTOOLPY_FLASHSIZE_4MB)
- **Partition Table**: `.config/partitions.csv` (Custom table with 1 MB OTA slots)
- **Compiler Optimization**: -Os (size optimization, critical for 4 MB constraint)
- **OTA Enabled**: Yes (dual-boot capability)

### Partition Layout

```
Name       Type  SubType  Offset    Size       Purpose
────────────────────────────────────────────────────────
nvs        data  nvs      0x9000    20 KB      Non-volatile storage
ota_0      app   ota_0    0xe000    1 MB       Main app partition
ota_1      app   ota_1    0x10e000  1 MB       Backup app for OTA
ramdisk    data  0x40     0x20e000  ~1.8 MB    Data/log storage

Total Used: 3.96 MB / 4 MB Flash ✅
```

## Build Size Management

### Current Constraints

- **Max app binary**: ~900 KB (1 MB OTA partition - overhead)
- **Optimization flags**: `-Os` (enabled by default in main/CMakeLists.txt)
- **Link-time optimization (LTO)**: Available if needed (commented in CMakeLists.txt)

### Check Binary Size

```bash
# After successful build, check actual binary size
ls -lh build/airgradient_go.bin

# Expected: ~400-500 KB (50-60% of source code size after compression)
```

### If Binary Exceeds Limits

1. **Enable LTO** (Link-Time Optimization):
   ```cmake
   # In main/CMakeLists.txt, uncomment:
   target_compile_options(${COMPONENT_LIB} PRIVATE "-flto")
   target_link_options(${COMPONENT_LIB} PRIVATE "-flto")
   ```

2. **Disable debug symbols**:
   ```bash
   idf.py menuconfig
   # Navigate: Compiler options → Strip symbols in ELF file
   ```

3. **Use PSRAM** (if available on variant):
   ```bash
   # Move large data structures to external RAM
   ```

## Components

The project uses these ESP-IDF components:

- **stcc4**: CO2 sensor driver (SCD4x variant)
- **esp_sgp4x**: VOC/NOx sensor driver
- **sps30**: Particulate matter sensor
- **sensirion_gas_index_algorithm**: Gas index calculation (1-500 scale)
- **esp_epaper**: e-paper display driver (SSD1680)
- **lvgl**: UI framework with Montserrat fonts (16, 20, 24 pt)

## Code Style

All code follows **LLVM style** with Clang-format:

```bash
# Format all source files
clang-format -i main/*.cpp main/*.h components/**/src/*.c

# Or use the .clang-format config in .config/
```

## Environment Setup (First Time)

```bash
# Install ESP-IDF (one-time)
mkdir -p ~/esp
cd ~/esp
git clone --branch v5.5.1 https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32c5

# Source setup for each new terminal
source ~/esp/esp-idf/export.sh
```

## Troubleshooting

### Flash programming fails
- Check USB cable connection
- Verify correct COM port: `ls /dev/tty*` (Linux/Mac) or `COM3` (Windows)
- Try: `idf.py erase-flash` then `idf.py flash`

### Binary too large
- See "Build Size Management" section above
- Check build warnings: `idf.py build --verbose`

### I2C bus errors
- Verify GPIO 6 (SCL) and GPIO 7 (SDA) connections
- Check pullup resistors (typically 4.7K on e-ink board)
- Confirm all sensor I2C addresses (see [HARDWARE_MAP.md](../.docs/HARDWARE_MAP.md))

## Next Steps

1. Review [CODE_REVIEW.md](../.docs/CODE_REVIEW.md) for code standards
2. Check [DRIVER_CONTRACTS.md](../.docs/ARCHITECTURE/DRIVER_CONTRACTS.md) for driver interfaces
3. See [FLASH_OPTIMIZATION.md](../.docs/FLASH_OPTIMIZATION.md) for advanced size reduction

## Build Output Example

```
Executing 'version_handler' in '/Users/formylife/Documents/airgradient_code/airgradient-go/build'...
Project is using the default empty main component
Building with IDF v5.5.1

...

Linking ELF file...
hello_world.elf file size: 434512 bytes. Max size: 983040 bytes
Max size: 983040 bytes. Free space: 548528 bytes

Generating project_description.json...
*** Flashing tools will be launched next ***

Hard resetting via RTS pin...
```

✅ Build successful when you see no errors and binary size is under 900 KB!
