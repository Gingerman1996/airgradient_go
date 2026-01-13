#pragma once

#include "driver/i2c_master.h"
#include "esp_log.h"

// Enable/disable I2C scanner at compile time
#define ENABLE_I2C_SCANNER 1

#if ENABLE_I2C_SCANNER

typedef struct {
    uint8_t address;
    const char *device_name;
} I2cDevice;

// Known I2C device addresses (based on AirGradient GO hardware)
static const I2cDevice I2C_KNOWN_DEVICES[] = {
    {0x31, "SHT4x (Temp/RH)"},
    {0x39, "SGP41 (VOC/NOx)"},
    {0x68, "STCC4 (CO2)"},
    {0x69, "SPS30 (PM)"},
    {0x54, "BQ25629 (Charger)"},
    {0x28, "CAP1203 (Buttons)"},
    {0x76, "DPS368 (Pressure)"},
};

static const size_t I2C_NUM_KNOWN_DEVICES = 
    sizeof(I2C_KNOWN_DEVICES) / sizeof(I2C_KNOWN_DEVICES[0]);

/**
 * Scan I2C bus and print found devices with names
 * @param bus_handle I2C master bus handle
 * @param timeout_ms Timeout for each address probe (ms)
 */
void i2c_scanner_scan(i2c_master_bus_handle_t bus_handle, uint32_t timeout_ms);

#else

// No-op when disabled
#define i2c_scanner_scan(bus_handle, timeout_ms) do {} while (0)

#endif // ENABLE_I2C_SCANNER
