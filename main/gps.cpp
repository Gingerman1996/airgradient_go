#include "gps.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

#include <nmea.h>
#include <gpgga.h>
#include <gprmc.h>
#include <gptxt.h>

static const char* TAG = "gps";
static const bool kLogRawNmea = true;

// TAU1113 antenna configuration
// Note: ANT_BIAS GPIO may need hardware-specific configuration
// For now, this is a placeholder. Update with actual GPIO if hardware supports it.
static constexpr gpio_num_t kAntBiasPin = GPIO_NUM_NC;  // Not Connected - update if needed
static constexpr bool kAntBiasHasHardwareControl = false;  // Set true if GPIO is available

static bool contains_token(const char* data, size_t len, const char* token) {
    if (!data || !token) {
        return false;
    }
    size_t token_len = strlen(token);
    if (token_len == 0 || token_len > len) {
        return false;
    }
    for (size_t i = 0; i + token_len <= len; ++i) {
        if (memcmp(data + i, token, token_len) == 0) {
            return true;
        }
    }
    return false;
}

// TAU1113 default UART settings (NMEA output)
static constexpr uart_port_t kGpsUart = UART_NUM_1;
static constexpr int kGpsBaud = 9600;
static constexpr gpio_num_t kGpsTxPin = GPIO_NUM_11;  // MCU_TX -> GPS RX
static constexpr gpio_num_t kGpsRxPin = GPIO_NUM_12;  // MCU_RX <- GPS TX
static constexpr int kGpsRxBufSize = 1024;

static float position_to_decimal(const nmea_position* pos) {
    if (!pos || pos->cardinal == NMEA_CARDINAL_DIR_UNKNOWN) {
        return 0.0f;
    }
    double degrees = (double)pos->degrees + (pos->minutes / 60.0);
    if (pos->cardinal == NMEA_CARDINAL_DIR_SOUTH || pos->cardinal == NMEA_CARDINAL_DIR_WEST) {
        degrees = -degrees;
    }
    return (float)degrees;
}

GPS::GPS()
    : initialized_(false)
    , antenna_type_(AntennaType::Passive)  // Default to passive antenna
    , fix_valid_(false)
    , fix_quality_(0)
    , satellites_(0)
    , lat_deg_(0.0f)
    , lon_deg_(0.0f)
    , time_valid_(false)
    , utc_hour_(0)
    , utc_min_(0)
    , utc_sec_(0)
    , last_sentence_ms_(0)
    , last_fix_ms_(0)
    , last_time_ms_(0)
    , last_antenna_ms_(0)
    , antenna_status_(AntennaStatus::Unknown)
    , last_status_log_ms_(0)
    , last_logged_antenna_(AntennaStatus::Unknown)
    , last_logged_fix_valid_(false)
    , last_logged_fix_quality_(-1)
    , last_logged_sats_(-1)
    , last_logged_time_valid_(false)
    , last_logged_hour_(-1)
    , last_logged_min_(-1)
    , line_len_(0)
    , uart_num_((int)kGpsUart) {
    memset(line_buf_, 0, sizeof(line_buf_));
}

GPS::~GPS() {
    if (initialized_) {
        uart_driver_delete((uart_port_t)uart_num_);
    }
}

esp_err_t GPS::init() {
    if (initialized_) {
        return ESP_OK;
    }

    uart_config_t cfg = {
        .baud_rate = kGpsBaud,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
    };

    esp_err_t ret = uart_param_config(kGpsUart, &cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = uart_set_pin(kGpsUart, kGpsTxPin, kGpsRxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "UART set pins failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = uart_driver_install(kGpsUart, kGpsRxBufSize, 0, 0, nullptr, 0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "UART driver install failed: %s", esp_err_to_name(ret));
        return ret;
    }

    uart_flush_input(kGpsUart);
    initialized_ = true;
    ESP_LOGI(TAG, "GPS UART initialized (baud=%d, TX=%d, RX=%d)",
             kGpsBaud, (int)kGpsTxPin, (int)kGpsRxPin);
    ESP_LOGI(TAG, "Antenna mode: %s (ANT_BIAS: %s)",
             antenna_type_ == AntennaType::Passive ? "PASSIVE" : "ACTIVE",
             antenna_type_ == AntennaType::Active ? "enabled" : "disabled");
    return ESP_OK;
}

esp_err_t GPS::set_antenna_type(AntennaType type) {
    if (!initialized_) {
        ESP_LOGE(TAG, "GPS not initialized, call init() first");
        return ESP_ERR_INVALID_STATE;
    }

    antenna_type_ = type;

    if (type == AntennaType::Active) {
        ESP_LOGI(TAG, "Configuring ACTIVE antenna (with ANT_BIAS)");
        
        // If hardware supports ANT_BIAS control via GPIO
        if (kAntBiasHasHardwareControl && kAntBiasPin != GPIO_NUM_NC) {
            // Configure ANT_BIAS pin as output
            gpio_config_t ant_cfg = {
                .pin_bit_mask = (1ULL << kAntBiasPin),
                .mode = GPIO_MODE_OUTPUT,
                .pull_up_en = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_DISABLE,
            };
            esp_err_t ret = gpio_config(&ant_cfg);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to configure ANT_BIAS GPIO: %s", esp_err_to_name(ret));
                return ret;
            }
            
            // Enable ANT_BIAS (active high)
            gpio_set_level(kAntBiasPin, 1);
            ESP_LOGI(TAG, "ANT_BIAS enabled on GPIO%d (max 25 mA)", (int)kAntBiasPin);
            ESP_LOGI(TAG, "Active antenna status detection enabled (ANT_OK/ANT_OPEN/ANT_SHORT)");
        } else {
            ESP_LOGW(TAG, "Active antenna mode set, but hardware control not available");
            ESP_LOGW(TAG, "ANT_BIAS control must be handled externally or via hardware jumper");
        }
    } else {
        ESP_LOGI(TAG, "Configuring PASSIVE antenna (no ANT_BIAS)");
        
        // Disable ANT_BIAS if hardware control is available
        if (kAntBiasHasHardwareControl && kAntBiasPin != GPIO_NUM_NC) {
            gpio_set_level(kAntBiasPin, 0);
            ESP_LOGI(TAG, "ANT_BIAS disabled on GPIO%d", (int)kAntBiasPin);
        }
        
        ESP_LOGI(TAG, "Using built-in LNA with internal bias");
        ESP_LOGI(TAG, "Suitable for patch antenna, whip antenna, or short cable");
    }

    return ESP_OK;
}

GPS::AntennaType GPS::get_antenna_type() const {
    return antenna_type_;
}

void GPS::update(uint64_t now_ms) {
    if (!initialized_) {
        return;
    }

    uint8_t rx[64];
    int len = 0;
    do {
        len = uart_read_bytes((uart_port_t)uart_num_, rx, sizeof(rx), 0);
        for (int i = 0; i < len; ++i) {
            char c = (char)rx[i];
            if (c == '$') {
                line_len_ = 0;
            }
            if (c == '\n') {
                if (line_len_ > 0 && line_buf_[line_len_ - 1] != '\r') {
                    if (line_len_ < sizeof(line_buf_) - 1) {
                        line_buf_[line_len_++] = '\r';
                    }
                }
                if (line_len_ < sizeof(line_buf_) - 1) {
                    line_buf_[line_len_++] = '\n';
                }
                if (line_len_ > 0) {
                    if (kLogRawNmea) {
                        size_t log_len = line_len_;
                        if (log_len >= 2 &&
                            line_buf_[log_len - 2] == '\r' &&
                            line_buf_[log_len - 1] == '\n') {
                            log_len -= 2;
                        }
                        ESP_LOGV(TAG, "NMEA RX: %.*s", (int)log_len, line_buf_);
                    }
                    handle_sentence(line_buf_, line_len_, now_ms);
                }
                line_len_ = 0;
                continue;
            }
            if (line_len_ < sizeof(line_buf_) - 1) {
                line_buf_[line_len_++] = c;
            } else {
                line_len_ = 0;
            }
        }
    } while (len > 0);
}

bool GPS::has_fix() const {
    return fix_valid_;
}

bool GPS::has_time() const {
    return time_valid_;
}

int GPS::utc_hour() const {
    return utc_hour_;
}

int GPS::utc_min() const {
    return utc_min_;
}

int GPS::utc_sec() const {
    return utc_sec_;
}

float GPS::latitude_deg() const {
    return lat_deg_;
}

float GPS::longitude_deg() const {
    return lon_deg_;
}

int GPS::satellites() const {
    return satellites_;
}

int GPS::fix_quality() const {
    return fix_quality_;
}

GPS::AntennaStatus GPS::antenna_status() const {
    return antenna_status_;
}

bool GPS::has_recent_sentence(uint64_t now_ms, uint64_t timeout_ms) const {
    if (last_sentence_ms_ == 0) return false;
    return (now_ms - last_sentence_ms_) <= timeout_ms;
}

bool GPS::has_recent_fix(uint64_t now_ms, uint64_t timeout_ms) const {
    if (!fix_valid_ || last_fix_ms_ == 0) return false;
    return (now_ms - last_fix_ms_) <= timeout_ms;
}

bool GPS::handle_sentence(char* line, size_t length, uint64_t now_ms) {
    if (!line || length < 9) {
        return false;
    }

    if (contains_token(line, length, "ANT_OK")) {
        antenna_status_ = AntennaStatus::Ok;
        last_antenna_ms_ = now_ms;
    } else if (contains_token(line, length, "ANT_OPEN")) {
        antenna_status_ = AntennaStatus::Open;
        last_antenna_ms_ = now_ms;
    } else if (contains_token(line, length, "ANT_SHORT")) {
        antenna_status_ = AntennaStatus::Short;
        last_antenna_ms_ = now_ms;
    }

    nmea_s* data = nmea_parse(line, length, 1);
    if (!data) {
        return false;
    }

    last_sentence_ms_ = now_ms;

    if (data->type == NMEA_GPGGA) {
        update_from_gga((nmea_gpgga_s*)data, now_ms);
    } else if (data->type == NMEA_GPRMC) {
        update_from_rmc((nmea_gprmc_s*)data, now_ms);
    } else if (data->type == NMEA_GPTXT) {
        update_from_txt((nmea_gptxt_s*)data, now_ms);
    }

    nmea_free(data);
    return true;
}

void GPS::update_from_gga(const nmea_gpgga_s* gga, uint64_t now_ms) {
    if (!gga) {
        return;
    }

    fix_quality_ = gga->position_fix;
    fix_valid_ = (gga->position_fix > 0);
    if (fix_valid_) {
        last_fix_ms_ = now_ms;
    }

    satellites_ = gga->n_satellites;
    lat_deg_ = position_to_decimal(&gga->latitude);
    lon_deg_ = position_to_decimal(&gga->longitude);

    update_time_from_tm(&gga->time, now_ms);
}

void GPS::update_from_rmc(const nmea_gprmc_s* rmc, uint64_t now_ms) {
    if (!rmc) {
        return;
    }

    fix_valid_ = rmc->valid;
    if (fix_valid_) {
        last_fix_ms_ = now_ms;
    }

    lat_deg_ = position_to_decimal(&rmc->latitude);
    lon_deg_ = position_to_decimal(&rmc->longitude);

    update_time_from_tm(&rmc->date_time, now_ms);
}

void GPS::update_from_txt(const nmea_gptxt_s* txt, uint64_t now_ms) {
    if (!txt || !txt->text[0]) {
        return;
    }

    if (strstr(txt->text, "ANT_OK")) {
        antenna_status_ = AntennaStatus::Ok;
        last_antenna_ms_ = now_ms;
    } else if (strstr(txt->text, "ANT_OPEN")) {
        antenna_status_ = AntennaStatus::Open;
        last_antenna_ms_ = now_ms;
    } else if (strstr(txt->text, "ANT_SHORT")) {
        antenna_status_ = AntennaStatus::Short;
        last_antenna_ms_ = now_ms;
    }
}

void GPS::update_time_from_tm(const struct tm* timeinfo, uint64_t now_ms) {
    if (!timeinfo) {
        return;
    }
    if (timeinfo->tm_hour < 0 || timeinfo->tm_hour > 23 ||
        timeinfo->tm_min < 0 || timeinfo->tm_min > 59 ||
        timeinfo->tm_sec < 0 || timeinfo->tm_sec > 59) {
        return;
    }

    utc_hour_ = timeinfo->tm_hour;
    utc_min_ = timeinfo->tm_min;
    utc_sec_ = timeinfo->tm_sec;
    time_valid_ = true;
    last_time_ms_ = now_ms;
}

void GPS::log_status(uint64_t now_ms, uint64_t sentence_timeout_ms, uint64_t fix_timeout_ms) {
    const uint64_t log_interval_ms = 5000;  // Log every 5 seconds
    if (now_ms - last_status_log_ms_ < log_interval_ms) {
        return;
    }
    last_status_log_ms_ = now_ms;

    bool has_sentence = has_recent_sentence(now_ms, sentence_timeout_ms);
    bool has_fix = has_recent_fix(now_ms, fix_timeout_ms);
    bool time_valid = has_sentence && time_valid_;
    int hour = utc_hour_;
    int min = utc_min_;

    AntennaStatus ant = antenna_status_;
    if (last_antenna_ms_ == 0 || (now_ms - last_antenna_ms_) > (sentence_timeout_ms * 3)) {
        ant = AntennaStatus::Unknown;
    }

    // Update last logged values for tracking
    last_logged_antenna_ = ant;
    last_logged_fix_valid_ = has_fix;
    last_logged_fix_quality_ = fix_quality_;
    last_logged_sats_ = satellites_;
    last_logged_time_valid_ = time_valid;
    last_logged_hour_ = hour;
    last_logged_min_ = min;

    const char* status_str = has_sentence ? (has_fix ? "FIX" : "SEARCHING") : "OFF";
    const char* ant_str = "UNKNOWN";
    if (ant == AntennaStatus::Ok) ant_str = "OK";
    else if (ant == AntennaStatus::Open) ant_str = "OPEN";
    else if (ant == AntennaStatus::Short) ant_str = "SHORT";
    
    // Antenna mode string (only show status for active antenna)
    const char* ant_mode = antenna_type_ == AntennaType::Passive ? "PASSIVE" : "ACTIVE";
    bool show_ant_status = (antenna_type_ == AntennaType::Active);

    if (time_valid) {
        if (has_fix) {
            if (show_ant_status) {
                ESP_LOGI(TAG,
                         "GPS status=%s fix_q=%d sats=%d time=%02d:%02d ant=%s(%s) "
                         "lat=%.5f lon=%.5f",
                         status_str, fix_quality_, satellites_, hour, min, ant_mode, ant_str,
                         lat_deg_, lon_deg_);
            } else {
                ESP_LOGI(TAG,
                         "GPS status=%s fix_q=%d sats=%d time=%02d:%02d ant=%s "
                         "lat=%.5f lon=%.5f",
                         status_str, fix_quality_, satellites_, hour, min, ant_mode,
                         lat_deg_, lon_deg_);
            }
        } else {
            if (show_ant_status) {
                ESP_LOGI(TAG,
                         "GPS status=%s fix_q=%d sats=%d time=%02d:%02d ant=%s(%s) "
                         "lat=-- lon=--",
                         status_str, fix_quality_, satellites_, hour, min, ant_mode, ant_str);
            } else {
                ESP_LOGI(TAG,
                         "GPS status=%s fix_q=%d sats=%d time=%02d:%02d ant=%s "
                         "lat=-- lon=--",
                         status_str, fix_quality_, satellites_, hour, min, ant_mode);
            }
        }
    } else {
        if (has_fix) {
            if (show_ant_status) {
                ESP_LOGI(TAG,
                         "GPS status=%s fix_q=%d sats=%d time=--:-- ant=%s(%s) "
                         "lat=%.5f lon=%.5f",
                         status_str, fix_quality_, satellites_, ant_mode, ant_str,
                         lat_deg_, lon_deg_);
            } else {
                ESP_LOGI(TAG,
                         "GPS status=%s fix_q=%d sats=%d time=--:-- ant=%s "
                         "lat=%.5f lon=%.5f",
                         status_str, fix_quality_, satellites_, ant_mode,
                         lat_deg_, lon_deg_);
            }
        } else {
            if (show_ant_status) {
                ESP_LOGI(TAG,
                         "GPS status=%s fix_q=%d sats=%d time=--:-- ant=%s(%s) "
                         "lat=-- lon=--",
                         status_str, fix_quality_, satellites_, ant_mode, ant_str);
            } else {
                ESP_LOGI(TAG,
                         "GPS status=%s fix_q=%d sats=%d time=--:-- ant=%s "
                         "lat=-- lon=--",
                         status_str, fix_quality_, satellites_, ant_mode);
            }
        }
    }
}
