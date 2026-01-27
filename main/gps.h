#pragma once

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#include <gpgga.h>
#include <gprmc.h>
#include <gptxt.h>

class GPS {
public:
    /**
     * @brief Antenna type configuration
     * 
     * Passive: Uses built-in LNA with internal bias
     *   - For patch antenna on PCB, whip antenna, or short cable
     *   - No external power needed (ANT_BIAS disabled)
     *   - RF_IN connected directly with 50Ω matching
     *   - No 82 nH inductor required
     * 
     * Active: Uses external LNA in antenna with ANT_BIAS power
     *   - For long cable runs, poor signal environments, or enclosed installations
     *   - Requires ANT_BIAS enable (up to 25 mA)
     *   - Supports short/open circuit detection (ANT_OK/ANT_OPEN/ANT_SHORT)
     *   - More stable in difficult RF conditions
     */
    enum class AntennaType { Passive, Active };
    
    /**
     * @brief Antenna status reported by GPS module
     * Only valid when AntennaType::Active is configured
     */
    enum class AntennaStatus { Unknown, Ok, Open, Short };

    GPS();
    ~GPS();

    // Initialize UART for NMEA input. Returns ESP_OK on success.
    esp_err_t init();

    /**
     * @brief Configure antenna type and enable/disable ANT_BIAS
     * @param type Passive (default, no external power) or Active (with ANT_BIAS)
     * @return ESP_OK on success
     * 
     * Must be called after init() if Active antenna is desired.
     * Default is Passive antenna.
     */
    esp_err_t set_antenna_type(AntennaType type);
    
    /**
     * @brief Get current antenna type configuration
     * @return Current antenna type
     */
    AntennaType get_antenna_type() const;

    /**
     * @brief Stop GPS and release UART resources
     * Call this before shutdown to cleanly release resources.
     * @return ESP_OK on success
     */
    esp_err_t stop();

    // Read and parse NMEA data (non-blocking). Call periodically.
    void update(uint64_t now_ms);

    // Fix/time status
    bool has_fix() const;
    bool has_time() const;
    int utc_hour() const;
    int utc_min() const;
    int utc_sec() const;

    // Lat/Lon in decimal degrees (valid when has_fix() is true)
    float latitude_deg() const;
    float longitude_deg() const;
    int satellites() const;
    int fix_quality() const;
    AntennaStatus antenna_status() const;

    // Sentence freshness
    bool has_recent_sentence(uint64_t now_ms, uint64_t timeout_ms) const;
    bool has_recent_fix(uint64_t now_ms, uint64_t timeout_ms) const;
    void log_status(uint64_t now_ms, uint64_t sentence_timeout_ms, uint64_t fix_timeout_ms);

private:
    bool handle_sentence(char* line, size_t length, uint64_t now_ms);
    void update_from_gga(const nmea_gpgga_s* gga, uint64_t now_ms);
    void update_from_rmc(const nmea_gprmc_s* rmc, uint64_t now_ms);
    void update_from_txt(const nmea_gptxt_s* txt, uint64_t now_ms);
    void update_time_from_tm(const struct tm* timeinfo, uint64_t now_ms);

    bool initialized_;
    AntennaType antenna_type_;  // Passive or Active antenna configuration
    bool fix_valid_;
    int fix_quality_;
    int satellites_;
    float lat_deg_;
    float lon_deg_;
    bool time_valid_;
    int utc_hour_;
    int utc_min_;
    int utc_sec_;
    uint64_t last_sentence_ms_;
    uint64_t last_fix_ms_;
    uint64_t last_time_ms_;
    uint64_t last_antenna_ms_;
    AntennaStatus antenna_status_;
    uint64_t last_status_log_ms_;
    AntennaStatus last_logged_antenna_;
    bool last_logged_fix_valid_;
    int last_logged_fix_quality_;
    int last_logged_sats_;
    bool last_logged_time_valid_;
    int last_logged_hour_;
    int last_logged_min_;

    char line_buf_[128];
    size_t line_len_;
    int uart_num_;
};
