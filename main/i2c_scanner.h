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
// Source: HARDWARE_MAP.md I2C Device Map section
static const I2cDevice I2C_KNOWN_DEVICES[] = {
    {0x12, "PMSA003I (PM sensor alternative)"},
    {0x18, "LIS2DH12 (Accelerometer)"},
    {0x28, "CAP1203 (Capacitive buttons 3x)"},
    {0x30, "LP5030/LP5036 (LED driver)"},
    {0x44, "SHT40 (Temp/RH backup)"},
    {0x59, "SGP41 (VOC + NOx)"},
    {0x62, "STCC4 / SCD4x (CO2 + Temp + RH)"},
    {0x69, "SPS30 (PM sensor primary)"},
    {0x6A, "BQ25628 (Battery charger)"},
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
