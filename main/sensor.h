#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"  // For i2c_master_bus_handle_t

// Aggregated sensor values for display
typedef struct {
    bool have_co2_avg;    // true if 5s average is available
    int co2_ppm_avg;      // averaged CO2 ppm
    bool have_co2_primary; // true if primary CO2 source has a recent value
    int co2_ppm_primary;   // primary CO2 source value (typically STCC4 5s avg)
    bool have_co2_scd4x;   // true if SCD4x CO2 has a recent value
    int co2_ppm_scd4x;     // latest SCD4x CO2 ppm
    bool co2_display_is_scd4x; // true when display CO2 is sourced from SCD4x
    float temp_c_avg;     // averaged temperature in °C
    float rh_avg;         // averaged relative humidity in %RH
    bool have_temp_rh;    // true if temperature/RH is available (STCC4 avg or SHT4x)

    float pm25_mass;      // PM2.5 µg/m³ (SPS30), 0 if unavailable
    int voc_ticks;        // SGP4x VOC ticks, 0 if unavailable
    int nox_ticks;        // SGP4x NOx ticks, 0 if unavailable
    int voc_index;        // VOC gas index (1-500), 0 during blackout
    int nox_index;        // NOx gas index (1-500), 0 during blackout
    float pressure_pa;    // Atmospheric pressure in Pascals (DPS368), 0 if unavailable

    // LIS2DH12 3-axis accelerometer
    bool have_accel;      // true if accelerometer data available
    int16_t accel_x_mg;   // X-axis acceleration in mg
    int16_t accel_y_mg;   // Y-axis acceleration in mg
    int16_t accel_z_mg;   // Z-axis acceleration in mg
    bool motion_detected; // true if motion interrupt triggered
} sensor_values_t;

class Sensors {
public:
    // Forward declaration of opaque state struct (defined in .cpp)
    struct SensorsState;

    // Constructor and destructor
    Sensors();
    ~Sensors();

    // Initialize I2C bus only (can be called before init). Returns ESP_OK on success.
    esp_err_t initI2CBus(void);

    // Initialize I2C and all sensors. Returns ESP_OK on success.
    esp_err_t init(void);

    // Periodic non-blocking update; call ~every 50ms with current millis.
    void update(int64_t current_millis);

    // Retrieve current display-ready values. Uses 5s average for CO2/Temp/RH.
    void getValues(int64_t now_ms, sensor_values_t *out);

    // Check if SPS30 has a recent successful read.
    bool isSps30Reading(int64_t now_ms, int64_t max_age_ms);

    // Get I2C bus handle (for sharing with other components like CAP1203)
    i2c_master_bus_handle_t getI2CBusHandle(void);

private:
    SensorsState *state;
};
