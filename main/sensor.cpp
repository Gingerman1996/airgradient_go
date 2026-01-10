#include "sensor.h"

#include <string.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

#include "stcc4.h"
#include "sgp4x.h"
#include "sps30.h"
#include "sensirion_gas_index_algorithm.h"

static const char *TAG_SENS = "sensors";

// I2C Configuration (shared by all sensors)
#define I2C_MASTER_SCL_IO    6      // GPIO 6 for SCL
#define I2C_MASTER_SDA_IO    7      // GPIO 7 for SDA
#define I2C_MASTER_NUM       I2C_NUM_0
#define I2C_MASTER_FREQ_HZ   100000 // 100 kHz

// CO2 ring buffer capacity
#define CO2_RING_CAP 12

// STCC4 state machine enum - follows recommended operation sequence
// STCC4 measurement cycle interval (milliseconds). Follows Sensirion spec: 5-600 seconds.
// Currently set to 10 seconds as optimal balance between power and responsiveness.
#define STCC4_MEASUREMENT_CYCLE_MS 10000

// STCC4 automatic conditioning interval (milliseconds). Cleans sensor every 3 hours.
// Required for long-term accuracy in high CO2 environments.
#define STCC4_CONDITIONING_INTERVAL_MS (3 * 60 * 60 * 1000)

enum class STCC4State {
    SLEEPING,           // Sensor in sleep mode, waiting for next measurement cycle
    WAKING_UP,          // Exiting sleep mode to idle
    IDLE,               // Sensor awake and ready
    TRIGGER_SINGLE,     // Sending measure_single_shot command
    WAIT_MEASUREMENT,   // Waiting for measurement execution time
    READ_RESULT,        // Reading measurement data
    ENTERING_SLEEP      // Putting sensor back to sleep
};

// Private implementation struct - defined outside class at file scope
struct Sensors::SensorsState {
    // I2C
    i2c_master_bus_handle_t i2c_bus_handle;

    // STCC4 CO2 sensor
    stcc4_dev_t co2_sensor;
    stcc4_measurement_t co2_measurement;
    
    // STCC4 state machine
    STCC4State stcc4_state;
    int64_t stcc4_state_time;
    int64_t stcc4_last_conditioning;
    int stcc4_wake_retries;
    int stcc4_read_attempts;
    int64_t stcc4_next_cycle_time;  // Next measurement cycle (5-600s interval)

    // CO2/Temp/RH ring buffer for last samples (5s average)
    int co2_ring_ppm[CO2_RING_CAP];
    float co2_ring_temp[CO2_RING_CAP];
    float co2_ring_rh[CO2_RING_CAP];
    int64_t co2_ring_ts[CO2_RING_CAP];
    int co2_ring_head;
    int co2_ring_size;

    // SGP4x VOC/NOx sensor
    sgp4x_handle_t sgp4x_handle;
    uint16_t sgp_voc_ticks;
    uint16_t sgp_nox_ticks;
    int64_t last_sgp_read;

    // Gas Index Algorithm for VOC and NOx
    GasIndexAlgorithmParams voc_algo_params;
    GasIndexAlgorithmParams nox_algo_params;
    int32_t voc_index;  // Calculated VOC gas index (1-500), 0 during blackout
    int32_t nox_index;  // Calculated NOx gas index (1-500), 0 during blackout

    // SPS30 PM sensor
    sps30_handle_t sps30_handle;
    sps30_measurement_t sps30_data;
};

// Constructor
Sensors::Sensors() {
    state = new SensorsState();
    memset(state, 0, sizeof(*state));
    state->i2c_bus_handle = NULL;
    state->stcc4_state = STCC4State::SLEEPING;
    state->stcc4_next_cycle_time = 0; // Start first cycle immediately
    state->co2_measurement = {
        .co2_ppm = 0,
        .temperature_raw = 0,
        .humidity_raw = 0,
        .sensor_status = 0,
        .temperature_c = 0.0f,
        .humidity_rh = 0.0f,
        .testing_mode = false
    };
    state->sgp4x_handle = NULL;
    state->sps30_handle = NULL;

    // Initialize Gas Index Algorithms (1s sampling interval matches SGP4x update rate)
    GasIndexAlgorithm_init_with_sampling_interval(&state->voc_algo_params, GasIndexAlgorithm_ALGORITHM_TYPE_VOC, 1.0f);
    GasIndexAlgorithm_init_with_sampling_interval(&state->nox_algo_params, GasIndexAlgorithm_ALGORITHM_TYPE_NOX, 1.0f);
    state->voc_index = 0;
    state->nox_index = 0;
}

// Destructor
Sensors::~Sensors() {
    if (state) delete state;
}

// Push CO2, temperature, and humidity reading to circular ring buffer.
// Overwrites oldest entry when buffer is full (capacity: 12 samples).
// Used for 5-second moving average calculation.
static inline void co2_ring_push(Sensors::SensorsState *st, int ppm, float t, float rh, int64_t ts_ms) {
    st->co2_ring_ppm[st->co2_ring_head] = ppm;
    st->co2_ring_temp[st->co2_ring_head] = t;
    st->co2_ring_rh[st->co2_ring_head] = rh;
    st->co2_ring_ts[st->co2_ring_head] = ts_ms;
    st->co2_ring_head = (st->co2_ring_head + 1) % CO2_RING_CAP;
    if (st->co2_ring_size < CO2_RING_CAP) st->co2_ring_size++;
}

// Calculate 5-second moving average for CO2, temperature, and humidity.
// Scans ring buffer for samples within 5000 ms window from now_ms.
// Returns true if at least one sample found, false otherwise.
static inline bool co2_avg_last_5s(Sensors::SensorsState *st, int64_t now_ms, int *ppm_out, float *t_out, float *rh_out) {
    int count = 0;
    long sum_ppm = 0;
    double sum_t = 0.0;
    double sum_rh = 0.0;
    int64_t window_start = now_ms - 5000;
    for (int i = 0; i < st->co2_ring_size; ++i) {
        int idx = (st->co2_ring_head - 1 - i + CO2_RING_CAP) % CO2_RING_CAP;
        if (st->co2_ring_ts[idx] >= window_start) {
            sum_ppm += st->co2_ring_ppm[idx];
            sum_t += st->co2_ring_temp[idx];
            sum_rh += st->co2_ring_rh[idx];
            count++;
        } else {
            break; // older than 5s window
        }
    }
    if (count > 0) {
        if (ppm_out) *ppm_out = (int)(sum_ppm / count);
        if (t_out) *t_out = (float)(sum_t / count);
        if (rh_out) *rh_out = (float)(sum_rh / count);
        return true;
    }
    return false;
}

// Initialize I2C master bus with GPIO 6 (SCL) and GPIO 7 (SDA) at 100 kHz.
// Enables internal pull-ups for I2C lines.
// Returns ESP_OK on success, ESP_FAIL if already initialized or bus creation fails.
static esp_err_t init_i2c_bus(i2c_master_bus_handle_t &i2c_bus_handle) {
    if (i2c_bus_handle) return ESP_OK;
    i2c_master_bus_config_t i2c_mst_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO,
        .scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {.enable_internal_pullup = true, .allow_pd = false}
    };
    esp_err_t ret = i2c_new_master_bus(&i2c_mst_config, &i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG_SENS, "I2C bus init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG_SENS, "I2C bus initialized (SCL=%d, SDA=%d)", I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO);
    return ESP_OK;
}

// Initialize STCC4 CO2 sensor (SCD4x variant) on I2C bus.
// Reads product ID with 3 retries, then puts sensor to sleep mode.
// Sensor will wake up on first measurement cycle.
// Returns ESP_OK on success, error code from driver on failure.
static esp_err_t init_stcc4_sensor(Sensors::SensorsState *state) {
    ESP_LOGI(TAG_SENS, "Initializing STCC4...");
    esp_err_t ret = stcc4_init(&state->co2_sensor, state->i2c_bus_handle, STCC4_I2C_ADDR_DEFAULT);
    if (ret != ESP_OK) return ret;

    uint32_t product_id = 0;
    int retries = 3;
    while (retries-- > 0) {
        vTaskDelay(pdMS_TO_TICKS(50));
        ret = stcc4_get_product_id(&state->co2_sensor, &product_id, NULL);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG_SENS, "STCC4 Product ID: 0x%08X", product_id);
            break;
        }
    }
    if (ret != ESP_OK) {
        ESP_LOGW(TAG_SENS, "Product ID read failed, continuing...");
    }
    stcc4_enter_sleep_mode(&state->co2_sensor);
    state->stcc4_state = STCC4State::SLEEPING;
    state->stcc4_state_time = esp_timer_get_time() / 1000;
    state->stcc4_last_conditioning = state->stcc4_state_time;
    return ESP_OK;
}

// Initialize SGP4x VOC/NOx sensor on I2C bus.
// Uses default SGP41 configuration with 100 kHz bus speed.
// Returns ESP_OK on success, error code from driver on failure.
static esp_err_t init_sgp4x_sensor(Sensors::SensorsState *state) {
    sgp4x_config_t cfg = I2C_SGP41_CONFIG_DEFAULT;
    cfg.i2c_clock_speed = I2C_MASTER_FREQ_HZ;
    esp_err_t ret = sgp4x_init(state->i2c_bus_handle, &cfg, &state->sgp4x_handle);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG_SENS, "SGP4x initialized (addr 0x%02X)", cfg.i2c_address);
    }
    return ret;
}

// Initialize SPS30 particulate matter sensor on I2C bus (address 0x69).
// Performs initialization test: start measurement, wait 3s, check status, then sleep.
// Returns ESP_OK on success, error code from driver on failure.
static esp_err_t init_sps30_sensor(Sensors::SensorsState *state) {
    sps30_config_t cfg = {.i2c_address = 0x69, .i2c_clock_speed = I2C_MASTER_FREQ_HZ};
    esp_err_t ret = sps30_init(state->i2c_bus_handle, &cfg, &state->sps30_handle);
    if (ret != ESP_OK) return ret;

    ret = sps30_start_measurement(state->sps30_handle);
    if (ret != ESP_OK) return ret;
    vTaskDelay(pdMS_TO_TICKS(3000));
    uint32_t status = 0;
    if (sps30_read_status_register(state->sps30_handle, &status) == ESP_OK) {
        ESP_LOGI(TAG_SENS, "SPS30 status after 3s: 0x%08X", status);
    }
    bool ready = false;
    if (sps30_read_data_ready(state->sps30_handle, &ready) == ESP_OK) {
        ESP_LOGI(TAG_SENS, "SPS30 data-ready: %s", ready ? "YES" : "NO");
    }
    sps30_stop_measurement(state->sps30_handle);
    sps30_sleep(state->sps30_handle);
    ESP_LOGI(TAG_SENS, "SPS30 ready (sleep mode)");
    return ESP_OK;
}

// Update STCC4 CO2 sensor state machine (non-blocking).
// State flow: SLEEPING → WAKING_UP → IDLE → TRIGGER_SINGLE → WAIT_MEASUREMENT → READ_RESULT → ENTERING_SLEEP → repeat.
// Measurement cycle: 10 seconds (configurable via STCC4_MEASUREMENT_CYCLE_MS).
// Automatic conditioning every 3 hours for long-term accuracy.
static void update_stcc4(Sensors::SensorsState *st, int64_t now_ms) {
    int64_t elapsed = now_ms - st->stcc4_state_time;
    esp_err_t ret;
    
    switch (st->stcc4_state) {
        case STCC4State::SLEEPING:
            // Wait for next measurement cycle (recommended: 5-600s)
            if (now_ms >= st->stcc4_next_cycle_time) {
                ESP_LOGI(TAG_SENS, "STCC4: Starting measurement cycle");
                st->stcc4_wake_retries = 0;
                st->stcc4_state = STCC4State::WAKING_UP;
                st->stcc4_state_time = now_ms;
            }
            break;
            
        case STCC4State::WAKING_UP:
            // Wait a bit before sending exit_sleep_mode
            if (elapsed >= 100) {
                ret = stcc4_exit_sleep_mode(&st->co2_sensor);
                if (ret == ESP_OK) {
                    ESP_LOGI(TAG_SENS, "STCC4: Woke up from sleep");
                    
                    // Perform conditioning if needed (every 3 hours)
                    if (now_ms - st->stcc4_last_conditioning >= (int64_t)3 * 60 * 60 * 1000) {
                        ESP_LOGI(TAG_SENS, "STCC4: Performing conditioning");
                        (void)stcc4_perform_conditioning(&st->co2_sensor);
                        st->stcc4_last_conditioning = now_ms;
                    }
                    
                    st->stcc4_state = STCC4State::IDLE;
                    st->stcc4_state_time = now_ms;
                } else {
                    ESP_LOGW(TAG_SENS, "STCC4: Failed to wake up (retry %d)", st->stcc4_wake_retries);
                    if (st->stcc4_wake_retries < 3) {
                        st->stcc4_wake_retries++;
                        st->stcc4_state_time = now_ms;
                    } else {
                        // Give up, schedule next cycle
                        ESP_LOGE(TAG_SENS, "STCC4: Wake up failed after retries");
                        st->stcc4_state = STCC4State::SLEEPING;
                        st->stcc4_next_cycle_time = now_ms + 10000; // Retry in 10s
                    }
                }
            }
            break;
            
        case STCC4State::IDLE:
            // Sensor is awake and ready, proceed to trigger measurement
            st->stcc4_state = STCC4State::TRIGGER_SINGLE;
            st->stcc4_state_time = now_ms;
            break;
            
        case STCC4State::TRIGGER_SINGLE:
            ret = stcc4_measure_single_shot(&st->co2_sensor);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG_SENS, "STCC4: Triggered single shot measurement");
                st->stcc4_state = STCC4State::WAIT_MEASUREMENT;
                st->stcc4_state_time = now_ms;
                st->stcc4_read_attempts = 0;
            } else {
                ESP_LOGW(TAG_SENS, "STCC4: Failed to trigger measurement");
                // Skip to sleep
                st->stcc4_state = STCC4State::ENTERING_SLEEP;
                st->stcc4_state_time = now_ms;
            }
            break;
            
        case STCC4State::WAIT_MEASUREMENT:
            // Wait for measurement execution time (typically 500ms)
            if (elapsed >= 500) {
                st->stcc4_state = STCC4State::READ_RESULT;
                st->stcc4_state_time = now_ms;
            }
            break;
            
        case STCC4State::READ_RESULT:
            ret = stcc4_read_measurement(&st->co2_sensor, &st->co2_measurement);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG_SENS, "STCC4: CO2=%d ppm, T=%.1f°C, RH=%.1f%%",
                         (int)st->co2_measurement.co2_ppm,
                         st->co2_measurement.temperature_c,
                         st->co2_measurement.humidity_rh);
                
                // Push to ring buffer
                co2_ring_push(st, (int)st->co2_measurement.co2_ppm,
                              st->co2_measurement.temperature_c,
                              st->co2_measurement.humidity_rh,
                              now_ms);
                
                // Proceed to sleep
                st->stcc4_state = STCC4State::ENTERING_SLEEP;
                st->stcc4_state_time = now_ms;
            } else {
                ESP_LOGW(TAG_SENS, "STCC4: Failed to read measurement (attempt %d)", st->stcc4_read_attempts);
                if (st->stcc4_read_attempts < 2) {
                    // Retry after 100ms
                    st->stcc4_read_attempts++;
                    vTaskDelay(pdMS_TO_TICKS(100));
                } else {
                    // Give up, go to sleep
                    ESP_LOGE(TAG_SENS, "STCC4: Read failed after retries");
                    st->stcc4_state = STCC4State::ENTERING_SLEEP;
                    st->stcc4_state_time = now_ms;
                }
            }
            break;
            
        case STCC4State::ENTERING_SLEEP:
            ret = stcc4_enter_sleep_mode(&st->co2_sensor);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG_SENS, "STCC4: Entered sleep mode");
            } else {
                ESP_LOGW(TAG_SENS, "STCC4: Failed to enter sleep mode");
            }
            
            // Schedule next cycle (10 seconds as per main loop interval requirement)
            st->stcc4_next_cycle_time = now_ms + STCC4_MEASUREMENT_CYCLE_MS;
            st->stcc4_state = STCC4State::SLEEPING;
            st->stcc4_state_time = now_ms;
            ESP_LOGI(TAG_SENS, "STCC4: Cycle complete, next in 10s");
            break;
    }
}

// Update SGP4x VOC/NOx sensor with 1-second sampling interval.
// Skips I2C access during STCC4 critical operations to avoid bus contention.
// Calculates gas index (1-500 scale) from raw ticks using Sensirion algorithm.
// Automatic retry on first read failure (40ms delay).
static void update_sgp4x(Sensors::SensorsState *st, int64_t now_ms) {
    if (!st->sgp4x_handle) return;
    
    // Skip while STCC4 is actively using I2C bus to avoid contention
    // Only skip during critical I2C operations: WAKING_UP, TRIGGER_SINGLE, WAIT_MEASUREMENT, READ_RESULT
    if (st->stcc4_state == STCC4State::WAKING_UP ||
        st->stcc4_state == STCC4State::TRIGGER_SINGLE ||
        st->stcc4_state == STCC4State::WAIT_MEASUREMENT ||
        st->stcc4_state == STCC4State::READ_RESULT ||
        st->stcc4_state == STCC4State::ENTERING_SLEEP) {
        return;
    }
    
    // Read SGP4x every 1 second when STCC4 is SLEEPING or IDLE
    if (now_ms - st->last_sgp_read < 1000) return;
    st->last_sgp_read = now_ms;
    
    uint16_t voc_raw = 0, nox_raw = 0;
    esp_err_t ret = sgp4x_measure_signals(st->sgp4x_handle, &voc_raw, &nox_raw);
    if (ret != ESP_OK) {
        vTaskDelay(pdMS_TO_TICKS(40));
        ret = sgp4x_measure_signals(st->sgp4x_handle, &voc_raw, &nox_raw);
    }
    if (ret == ESP_OK) {
        st->sgp_voc_ticks = voc_raw;
        st->sgp_nox_ticks = nox_raw;
        
        // Calculate gas index from raw ticks using Sensirion algorithm
        int32_t voc_idx = 0, nox_idx = 0;
        GasIndexAlgorithm_process(&st->voc_algo_params, (int32_t)voc_raw, &voc_idx);
        GasIndexAlgorithm_process(&st->nox_algo_params, (int32_t)nox_raw, &nox_idx);
        st->voc_index = voc_idx;
        st->nox_index = nox_idx;
        
        ESP_LOGD(TAG_SENS, "SGP4x: VOC=%d (idx=%ld), NOx=%d (idx=%ld)", voc_raw, voc_idx, nox_raw, nox_idx);
    }
}

// Update SPS30 particulate matter sensor with duty-cycling to save power.
// Uses static local state machine to track sensor lifecycle: SLEEPING -> WAKING -> MEASURING -> READY -> SLEEPING.
// Static state persists across function calls and avoids dynamic allocation.
// Measurement cycle: 10s sleep -> 100ms wake -> 10s measurement -> 1s readout -> repeat.
// Returns immediately if sensor handle is NULL (sensor not initialized).
static void update_sps30(Sensors::SensorsState *st, int64_t now_ms) {
    if (!st->sps30_handle) return;
    static enum { PM_SLEEPING, PM_WAKING, PM_MEASURING, PM_READY } pm_state = PM_SLEEPING;
    static int64_t pm_state_time = 0;
    int64_t elapsed = now_ms - pm_state_time;
    esp_err_t ret;
    switch (pm_state) {
        case PM_SLEEPING:
            if (elapsed >= 10000) {
                ret = sps30_wakeup(st->sps30_handle);
                if (ret == ESP_OK) {
                    pm_state = PM_WAKING;
                    pm_state_time = now_ms;
                }
            }
            break;
        case PM_WAKING:
            if (elapsed >= 100) {
                ret = sps30_start_measurement(st->sps30_handle);
                if (ret == ESP_OK) {
                    pm_state = PM_MEASURING;
                    pm_state_time = now_ms;
                } else {
                    sps30_sleep(st->sps30_handle);
                    pm_state = PM_SLEEPING;
                    pm_state_time = now_ms;
                }
            }
            break;
        case PM_MEASURING:
            if (elapsed >= 10000) {
                pm_state = PM_READY;
            }
            break;
        case PM_READY:
            ret = sps30_read_measurement(st->sps30_handle, &st->sps30_data);
            (void)ret;
            sps30_stop_measurement(st->sps30_handle);
            sps30_sleep(st->sps30_handle);
            pm_state = PM_SLEEPING;
            pm_state_time = now_ms;
            break;
    }
}

esp_err_t Sensors::init(void) {
    esp_log_level_set("STCC4", ESP_LOG_DEBUG);
    ESP_ERROR_CHECK_WITHOUT_ABORT(init_i2c_bus(state->i2c_bus_handle));
    esp_err_t ok1 = init_stcc4_sensor(state);
    esp_err_t ok2 = init_sgp4x_sensor(state);
    esp_err_t ok3 = init_sps30_sensor(state);
    if (ok1 != ESP_OK) ESP_LOGW(TAG_SENS, "STCC4 init failed: %s", esp_err_to_name(ok1));
    if (ok2 != ESP_OK) ESP_LOGW(TAG_SENS, "SGP4x init failed: %s", esp_err_to_name(ok2));
    if (ok3 != ESP_OK) ESP_LOGW(TAG_SENS, "SPS30 init failed: %s", esp_err_to_name(ok3));
    state->stcc4_state_time = esp_timer_get_time() / 1000;
    return (ok1 == ESP_OK || ok2 == ESP_OK || ok3 == ESP_OK) ? ESP_OK : ESP_FAIL;
}

void Sensors::update(int64_t current_millis) {
    update_stcc4(state, current_millis);
    update_sgp4x(state, current_millis);
    update_sps30(state, current_millis);
}

void Sensors::getValues(int64_t now_ms, sensor_values_t *out) {
    if (!out) return;
    memset(out, 0, sizeof(*out));
    int co2_avg = 0; float t_avg = 0.0f; float rh_avg = 0.0f;
    bool have = co2_avg_last_5s(state, now_ms, &co2_avg, &t_avg, &rh_avg);
    out->have_co2_avg = have;
    if (have) {
        out->co2_ppm_avg = co2_avg;
        out->temp_c_avg = t_avg;
        out->rh_avg = rh_avg;
    }
    out->pm25_mass = state->sps30_data.pm2p5_mass;
    out->voc_ticks = (int)state->sgp_voc_ticks;
    out->nox_ticks = (int)state->sgp_nox_ticks;
    out->voc_index = (int)state->voc_index;
    out->nox_index = (int)state->nox_index;
}
