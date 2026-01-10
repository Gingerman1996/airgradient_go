# ⚠️ Flash Memory Management - 4 MB Optimization Guide

**ESP32-C5-MINI-1**: 4 MB Flash (Limited Space)

---

## 📊 Current Partition Layout

```
0x000000 ┌─────────────────────┐
         │   Bootloader        │ 64 KB (auto)
0x00E000 ├─────────────────────┤
         │   NVS (20 KB)       │
0x010000 ├─────────────────────┤
         │   OTA_0 (1 MB)      │ ← Current app
0x110000 ├─────────────────────┤
         │   OTA_1 (1 MB)      │ ← Backup app
0x210000 ├─────────────────────┤
         │   RAMDISK (1.8 MB)  │ ← Data storage
0x3F0000 └─────────────────────┘
```

---

## 🎯 Build Size Optimization

### 1. Enable Compiler Optimizations

```bash
idf.py menuconfig
```

Navigate to:
- **Compiler options** → **Optimization Level** → **-Os (size)**
- **Component config** → **ESP-IDF** → **Enable LTO** ✓

### 2. Monitor Binary Sizes

```bash
# Total app size
idf.py size

# Per-component breakdown
idf.py size-components

# Detailed ELF analysis
idf.py size-files
```

### 3. Remove Unnecessary Features

In `idf.py menuconfig`, disable:
- **Debug logging** (in production)
- **Assert statements** (in release)
- **Verbose output**
- **Unused drivers**

### 4. LVGL Optimization

If using LVGL, optimize in `lv_conf.h`:
```c
#define LV_MEM_SIZE (64 * 1024)    // Reduce to 64KB
#define LV_FONT_ROBOTO_24 1        // Use only needed fonts
// Disable unused widgets
#define LV_USE_SLIDER 0
#define LV_USE_ROLLER 0
```

---

## 💾 Storage Strategy

### Primary Storage (1.8 MB RAMDISK)
- **Use W25N512 NAND flash** for data logging
- **Not** internal ESP-IDF SPIFFS (too large)
- Access via SPI (IO4 CS, IO23 SCLK, IO25 MOSI, IO24 MISO)

### Session Limits
With 1.8 MB available:
- **~1800** entries of 1 KB each
- **Ring buffer** for continuous logging (older data overwritten)
- Compress or downsample sensor data

### Data Format
```cpp
struct SensorReading {
  uint32_t timestamp;      // 4 bytes
  uint16_t co2;            // 2 bytes
  uint8_t  temp;           // 1 byte (scaled)
  uint8_t  humidity;       // 1 byte
  uint8_t  pm25;           // 1 byte
} __attribute__((packed)); // 9 bytes per entry
```

With 9-byte readings: **~200,000 entries** in 1.8 MB

---

## 🚨 Common Issues & Solutions

### Issue: Binary Too Large
```
error: Partition 'ota_0' is too small. Binary is X bytes, partition is Y bytes
```

**Solution:**
1. Check current size: `idf.py size`
2. Enable `-Os` optimization
3. Enable LTO
4. Remove unused components
5. Check for large libraries (LVGL, crypto)

### Issue: OTA Fails
OTA updates require **free space** in OTA_1 partition (1 MB max).

**Solution:**
- Keep app binary ≤ 900 KB
- Use `--compress` flag: `idf.py secure_bin_to_elf --compress`

### Issue: Data Corrupted
RAMDISK corruption from concurrent access.

**Solution:**
- Use **ring buffer** with proper write synchronization
- Implement **checksum verification**
- Keep backup copy of critical data

---

## 📈 Monitoring

### Check Flash Usage

```bash
# Before build
ls -lh build/firmware.bin

# After flash
idf.py partition-table-digest

# Runtime memory
idf.py monitor
# Look for "heap" in output
```

### CMakeLists.txt Size Checking

```cmake
# Add to main/CMakeLists.txt
get_filename_component(FIRMWARE_BIN 
    "${CMAKE_BINARY_DIR}/firmware.bin" ABSOLUTE)

function(check_firmware_size)
    file(SIZE "${FIRMWARE_BIN}" FIRMWARE_SIZE)
    message("Firmware size: ${FIRMWARE_SIZE} bytes")
    if(FIRMWARE_SIZE GREATER 1048576)
        message(WARNING "Firmware > 1 MB!")
    endif()
endfunction()
```

---

## ✅ Pre-Flash Checklist

- [ ] Binary size ≤ 900 KB
- [ ] Optimization: `-Os` enabled
- [ ] LTO enabled
- [ ] Debug logging disabled
- [ ] Unused features removed
- [ ] `idf.py size` reviewed
- [ ] OTA test: Can update from OTA_0 → OTA_1

---

## 🔗 Related Files

- Configuration: [`.config/partitions.csv`](../../.config/partitions.csv)
- Build guide: [`.docs/AGENT.md`](.././AGENT.md)
- Hardware: [`.docs/ARCHITECTURE/HARDWARE_MAP.md`](./HARDWARE_MAP.md)

---

## 📚 ESP-IDF References

- [Partition Tables](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c5/api-guides/partition-tables.html)
- [Size Optimization](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c5/security/index.html)
- [OTA Updates](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c5/api-reference/system/ota.html)

---

**Last Updated**: January 10, 2026  
**Target**: ESP32-C5-MINI-1 (4 MB Flash)
