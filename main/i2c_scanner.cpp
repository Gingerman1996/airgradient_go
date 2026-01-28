#include "i2c_scanner.h"

#if ENABLE_I2C_SCANNER

static const char *TAG_I2C = "I2C_SCANNER";

/**
 * Get device name for a given I2C address
 * @param address I2C address
 * @return Device name or NULL if unknown
 */
static const char *i2c_get_device_name(uint8_t address) {
    for (size_t i = 0; i < I2C_NUM_KNOWN_DEVICES; i++) {
        if (I2C_KNOWN_DEVICES[i].address == address) {
            return I2C_KNOWN_DEVICES[i].device_name;
        }
    }
    return NULL;
}

void i2c_scanner_scan(i2c_master_bus_handle_t bus_handle, uint32_t timeout_ms) {
    ESP_LOGI(TAG_I2C, "Scanning I2C bus (timeout: %lu ms)...", timeout_ms);
    ESP_LOGI(TAG_I2C, "=== I2C Devices Found ===");
    
    int device_count = 0;

    esp_err_t reset_ret = i2c_master_bus_reset(bus_handle);
    if (reset_ret != ESP_OK) {
        ESP_LOGE(TAG_I2C, "I2C bus reset failed: %s", esp_err_to_name(reset_ret));
        return;
    }
    
    // Scan all 128 possible I2C addresses (7-bit addressing)
    for (uint8_t addr = 0x00; addr < 0x80; addr++) {
        // Skip reserved addresses
        if ((addr < 0x03) || (addr >= 0x78)) {
            continue;
        }
        
    // Try to probe this address for a device
    esp_err_t ret = i2c_master_probe(bus_handle, addr, timeout_ms);
        
        if (ret == ESP_OK) {
            // Found a device
            device_count++;
            const char *device_name = i2c_get_device_name(addr);
            
            if (device_name) {
                ESP_LOGI(TAG_I2C, "  0x%02X - %s", addr, device_name);
            } else {
                ESP_LOGI(TAG_I2C, "  0x%02X - Unknown device", addr);
            }
        }
    }
    
    ESP_LOGI(TAG_I2C, "=== Scan Complete ===");
    ESP_LOGI(TAG_I2C, "Found %d device(s)", device_count);
}

#endif // ENABLE_I2C_SCANNER
