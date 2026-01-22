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
#include "dps368.h"
#include "lis2dh12.h"
#include "driver/gpio.h"

static const char *TAG_SENS = "sensors";

// I2C Configuration (shared by all sensors)
#define I2C_MASTER_SCL_IO    6      // GPIO 6 for SCL
#define I2C_MASTER_SDA_IO    7      // GPIO 7 for SDA
#define I2C_MASTER_NUM       I2C_NUM_0
#define I2C_MASTER_FREQ_HZ   100000 // 100 kHz

// PM Sensor Power Control
#define EN_PM1_GPIO          26     // GPIO 26 - PM sensor load switch + I2C isolator enable

// LIS2DH12 Accelerometer Interrupt Pin
#define LIS2DH12_INT1_GPIO   3      // GPIO 3 - Motion detection interrupt

// CO2 ring buffer capacity
#define CO2_RING_CAP 12

// STCC4 continuous measurement mode - reads every 1 second
// Sensor provides new data every 1 second in continuous mode
#define STCC4_READ_INTERVAL_MS 1000

// STCC4 automatic conditioning interval (milliseconds). Cleans sensor every 3 hours.
// Required for long-term accuracy in high CO2 environments.
#define STCC4_CONDITIONING_INTERVAL_MS (3 * 60 * 60 * 1000)

// DPS368 pressure sensor read interval (milliseconds)
#define DPS368_READ_INTERVAL_MS 5000

enum class STCC4State {
    INIT,               // Need to start continuous measurement
    STARTING,           // Starting continuous measurement mode
    MEASURING,          // Continuous measurement active, reading every 1s
    ERROR               // Error state, will retry
};

// Private implementation struct - defined outside class at file scope
struct Sensors::SensorsState {
    // I2C
    i2c_master_bus_handle_t i2c_bus_handle;

    // STCC4 CO2 sensor
    stcc4_dev_t co2_sensor;
    stcc4_measurement_t co2_measurement;
    
    // STCC4 state machine (continuous mode)
    STCC4State stcc4_state;
    int64_t stcc4_state_time;
    int64_t stcc4_last_conditioning;
    int64_t stcc4_last_read;         // Last successful read time

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
    int64_t sps30_last_read;
    int sps30_not_ready_count;
    
    // DPS368 Pressure sensor
    dps368_handle_t *dps368_handle;
    dps368_data_t dps368_data;
    int64_t last_dps_read;

    // LIS2DH12 Accelerometer
    drivers::LIS2DH12 *lis2dh12;
    drivers::AccelData accel_data;
    bool have_accel_data;
    bool motion_detected;
    int64_t last_accel_read;
    volatile bool motion_interrupt_triggered;
};

// Constructor
Sensors::Sensors() {
    state = new SensorsState();
    memset(state, 0, sizeof(*state));
    state->i2c_bus_handle = NULL;
    state->stcc4_state = STCC4State::INIT;
    state->stcc4_last_read = 0;
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
    state->dps368_handle = NULL;
    state->lis2dh12 = nullptr;
    state->have_accel_data = false;
    state->motion_detected = false;
    state->last_accel_read = 0;
    state->motion_interrupt_triggered = false;

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

// Public method to initialize I2C bus (can be called before full init())
esp_err_t Sensors::initI2CBus(void) {
    if (!state) {
        ESP_LOGE(TAG_SENS, "Sensors state not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    return init_i2c_bus(state->i2c_bus_handle);
}

// Initialize STCC4 CO2 sensor (SCD4x variant) on I2C bus.
// Reads product ID with 3 retries, then sets up for continuous measurement.
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
    
    // Set initial state for continuous measurement mode
    state->stcc4_state = STCC4State::INIT;
    state->stcc4_state_time = esp_timer_get_time() / 1000;
    state->stcc4_last_conditioning = state->stcc4_state_time;
    state->stcc4_last_read = 0;
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
    // Configure EN_PM1 GPIO (IO26) for PM sensor power control
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << EN_PM1_GPIO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    
    // Enable PM sensor power (TPS27081A load switch + TMUX121 I2C isolator)
    gpio_set_level((gpio_num_t)EN_PM1_GPIO, 1);
    ESP_LOGI(TAG_SENS, "EN_PM1 enabled (IO26=HIGH) - PM sensor powered");
    
    // Wait for power stabilization
    vTaskDelay(pdMS_TO_TICKS(100));
    
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
    
    // Initialize DPS368 pressure sensor (address 0x77 per hardware config)
    ret = dps368_init(state->i2c_bus_handle, DPS368_I2C_ADDR_SDO_VDD, &state->dps368_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG_SENS, "DPS368 init failed (optional sensor): %s", esp_err_to_name(ret));
        state->dps368_handle = NULL;
    } else {
        ESP_LOGI(TAG_SENS, "DPS368 pressure sensor ready");
    }

    // Initialize LIS2DH12 accelerometer (address 0x18, SA0=GND)
    state->lis2dh12 = new drivers::LIS2DH12(state->i2c_bus_handle, drivers::LIS2DH12_I2C::ADDR_SA0_LOW);
    ret = state->lis2dh12->init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG_SENS, "LIS2DH12 init failed (optional sensor): %s", esp_err_to_name(ret));
        delete state->lis2dh12;
        state->lis2dh12 = nullptr;
    } else {
        // Configure motion detection: threshold=250mg (above typical ±20mg noise), duration=0
        // Higher threshold reduces false triggers from vibration/noise
        // Will still detect deliberate movement (>0.25g acceleration)
        drivers::MotionConfig motion_cfg = {
            .threshold_mg = 250,
            .duration_ms = 0,  // 0 = immediate trigger, no minimum duration required
            .enable_x = true,
            .enable_y = true,
            .enable_z = true
        };
        ret = state->lis2dh12->configure_motion_detect(motion_cfg);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG_SENS, "LIS2DH12 motion detection configured (threshold=250mg, duration=0ms)");
            
            // Clear any pending interrupt by reading INT1_SRC (important for latched mode)
            uint8_t int_src = 0;
            state->lis2dh12->get_int1_source(&int_src);
            ESP_LOGI(TAG_SENS, "LIS2DH12 cleared INT1_SRC=0x%02X", int_src);
        }

        // Enable sleep-to-wake: activity threshold=80mg, inactivity duration=30s
        drivers::SleepToWakeConfig sleep_cfg = {
            .activity_threshold_mg = 80,
            .inactivity_duration_s = 30
        };
        ret = state->lis2dh12->enable_sleep_to_wake(sleep_cfg);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG_SENS, "LIS2DH12 sleep-to-wake enabled (30s inactivity)");
        }

        // Configure GPIO3 for interrupt input
        // INT1 is configured as active-high: LOW (idle) -> HIGH (interrupt)
        gpio_config_t io_int_conf = {};
        io_int_conf.intr_type = GPIO_INTR_POSEDGE; // Rising edge (LOW→HIGH) for active-high
        io_int_conf.mode = GPIO_MODE_INPUT;
        io_int_conf.pin_bit_mask = (1ULL << LIS2DH12_INT1_GPIO);
        io_int_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;  // Pull-down for active-high signal
        io_int_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        ret = gpio_config(&io_int_conf);
        if (ret == ESP_OK) {
            // Install ISR service if not already installed
            gpio_install_isr_service(0);
            gpio_isr_handler_add((gpio_num_t)LIS2DH12_INT1_GPIO, 
                                  [](void *arg) {
                                      Sensors::SensorsState *st = (Sensors::SensorsState *)arg;
                                      if (st) {
                                          st->motion_interrupt_triggered = true;
                                      }
                                  }, state);
            ESP_LOGI(TAG_SENS, "LIS2DH12 INT1 configured on GPIO%d", LIS2DH12_INT1_GPIO);
        } else {
            ESP_LOGW(TAG_SENS, "Failed to configure INT1 GPIO: %s", esp_err_to_name(ret));
        }

        ESP_LOGI(TAG_SENS, "LIS2DH12 accelerometer ready");
    }
    
    return ESP_OK;
}

// Update STCC4 CO2 sensor using continuous measurement mode (non-blocking).
// State flow: INIT → STARTING → MEASURING (continuous 1s reads).
// Automatic conditioning every 3 hours for long-term accuracy.
static void update_stcc4(Sensors::SensorsState *st, int64_t now_ms) {
    int64_t elapsed = now_ms - st->stcc4_state_time;
    esp_err_t ret;
    
    switch (st->stcc4_state) {
        case STCC4State::INIT:
            // Initial state - start continuous measurement
            ESP_LOGI(TAG_SENS, "STCC4: Starting continuous measurement mode");
            st->stcc4_state = STCC4State::STARTING;
            st->stcc4_state_time = now_ms;
            break;
            
        case STCC4State::STARTING:
            // Start continuous measurement mode (1s sampling)
            if (elapsed >= 100) {
                ret = stcc4_start_continuous_measurement(&st->co2_sensor);
                if (ret == ESP_OK) {
                    ESP_LOGI(TAG_SENS, "STCC4: Continuous measurement started");
                    st->stcc4_state = STCC4State::MEASURING;
                    st->stcc4_state_time = now_ms;
                    st->stcc4_last_read = now_ms;
                } else {
                    ESP_LOGW(TAG_SENS, "STCC4: Failed to start continuous mode: %s", esp_err_to_name(ret));
                    st->stcc4_state = STCC4State::ERROR;
                    st->stcc4_state_time = now_ms;
                }
            }
            break;
            
        case STCC4State::MEASURING:
            // Read measurement every 1 second in continuous mode
            if (now_ms - st->stcc4_last_read >= STCC4_READ_INTERVAL_MS) {
                ret = stcc4_read_measurement(&st->co2_sensor, &st->co2_measurement);
                if (ret == ESP_OK) {
                    st->stcc4_last_read = now_ms;
                    
                    // Push to ring buffer
                    co2_ring_push(st, (int)st->co2_measurement.co2_ppm,
                                  st->co2_measurement.temperature_c,
                                  st->co2_measurement.humidity_rh,
                                  now_ms);
                    
                    // Perform conditioning if needed (every 3 hours)
                    if (now_ms - st->stcc4_last_conditioning >= (int64_t)STCC4_CONDITIONING_INTERVAL_MS) {
                        ESP_LOGI(TAG_SENS, "STCC4: Performing conditioning");
                        stcc4_stop_continuous_measurement(&st->co2_sensor);
                        vTaskDelay(pdMS_TO_TICKS(100));
                        stcc4_perform_conditioning(&st->co2_sensor);
                        st->stcc4_last_conditioning = now_ms;
                        vTaskDelay(pdMS_TO_TICKS(100));
                        stcc4_start_continuous_measurement(&st->co2_sensor);
                    }
                }
                // Note: ESP_ERR_INVALID_RESPONSE means no data ready yet, which is normal
            }
            break;
            
        case STCC4State::ERROR:
            // Retry after 5 seconds
            if (elapsed >= 5000) {
                ESP_LOGI(TAG_SENS, "STCC4: Retrying continuous measurement...");
                st->stcc4_state = STCC4State::INIT;
                st->stcc4_state_time = now_ms;
            }
            break;
    }
}

// Update SGP4x VOC/NOx sensor with 1-second sampling interval.
// Calculates gas index (1-500 scale) from raw ticks using Sensirion algorithm.
// Automatic retry on first read failure (40ms delay).
static void update_sgp4x(Sensors::SensorsState *st, int64_t now_ms) {
    if (!st->sgp4x_handle) return;
    
    // Read SGP4x every 1 second
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
    }
}

// Update SPS30 particulate matter sensor with continuous 1-second readings.
// Per datasheet: "New readings are available every second" (Section 4.1).
// State machine: INIT -> START -> WARMUP -> MEASURING (continuous 1s reads)
// After initialization, sensor stays in Measurement Mode for continuous readings.
// Startup time: 8-30 seconds depending on concentration level (Table 1).
static void update_sps30(Sensors::SensorsState *st, int64_t now_ms) {
    if (!st->sps30_handle) return;
    
    // SPS30 state machine for continuous measurement
    static enum { 
        SPS30_INIT,      // Initial state - need to wake up sensor
        SPS30_START,     // Start measurement mode
        SPS30_WARMUP,    // Wait for sensor warmup (8-30s per datasheet)
        SPS30_MEASURING  // Continuous 1-second readings
    } sps30_state = SPS30_INIT;
    static int64_t sps30_state_time = 0;
    static int64_t last_read_time = 0;
    
    int64_t elapsed = now_ms - sps30_state_time;
    esp_err_t ret;
    
    switch (sps30_state) {
        case SPS30_INIT:
            // Wake up sensor from sleep mode
            ret = sps30_wakeup(st->sps30_handle);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG_SENS, "SPS30: Waking up sensor");
                sps30_state = SPS30_START;
                sps30_state_time = now_ms;
                st->sps30_not_ready_count = 0;
                st->sps30_last_read = 0;
            }
            break;
            
        case SPS30_START:
            // Wait 100ms after wake-up before starting measurement
            if (elapsed >= 100) {
                ret = sps30_start_measurement(st->sps30_handle);
                if (ret == ESP_OK) {
                    ESP_LOGI(TAG_SENS, "SPS30: Starting continuous measurement mode");
                    sps30_state = SPS30_WARMUP;
                    sps30_state_time = now_ms;
                    st->sps30_not_ready_count = 0;
                } else {
                    ESP_LOGE(TAG_SENS, "SPS30: Failed to start measurement");
                    sps30_sleep(st->sps30_handle);
                    sps30_state = SPS30_INIT;
                    sps30_state_time = now_ms + 5000; // Retry in 5s
                }
            }
            break;
            
        case SPS30_WARMUP:
            // Wait for sensor warmup (8-30s per datasheet Table 1)
            // Using 8 seconds minimum for typical concentrations
            if (elapsed >= 8000) {
                ESP_LOGI(TAG_SENS, "SPS30: Warmup complete, entering continuous read mode");
                sps30_state = SPS30_MEASURING;
                sps30_state_time = now_ms;
                last_read_time = now_ms;
                st->sps30_not_ready_count = 0;
            }
            break;
            
        case SPS30_MEASURING:
            // Read measurement every 1 second (per datasheet: new data every 1s)
            if (now_ms - last_read_time >= 1000) {
                last_read_time = now_ms;
                
                // Check if data is ready before reading
                bool ready = false;
                ret = sps30_read_data_ready(st->sps30_handle, &ready);
                
                if (ret == ESP_OK && ready) {
                    st->sps30_not_ready_count = 0;
                    ret = sps30_read_measurement(st->sps30_handle, &st->sps30_data);
                    if (ret == ESP_OK) {
                        st->sps30_last_read = now_ms;
                        ESP_LOGD(TAG_SENS, "SPS30: PM1.0=%.1f, PM2.5=%.1f, PM4.0=%.1f, PM10=%.1f µg/m³",
                                 st->sps30_data.pm1p0_mass, st->sps30_data.pm2p5_mass,
                                 st->sps30_data.pm4p0_mass, st->sps30_data.pm10p0_mass);
                    }
                } else if (ret == ESP_OK && !ready) {
                    st->sps30_not_ready_count++;
                    if (st->sps30_not_ready_count > 2) {
                        ESP_LOGW(TAG_SENS, "SPS30: Data-ready not ready too long, restarting");
                        sps30_stop_measurement(st->sps30_handle);
                        sps30_sleep(st->sps30_handle);
                        st->sps30_not_ready_count = 0;
                        st->sps30_last_read = 0;
                        sps30_state = SPS30_INIT;
                        sps30_state_time = now_ms;
                        last_read_time = 0;
                    }
                } else if (ret != ESP_OK) {
                    ESP_LOGW(TAG_SENS, "SPS30: Data ready check failed");
                }
            }
            break;
    }
}

esp_err_t Sensors::init(void) {
    esp_log_level_set(TAG_SENS, ESP_LOG_DEBUG);
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
    
    // Read DPS368 pressure sensor (every 5 seconds)
    if (state->dps368_handle && (current_millis - state->last_dps_read >= DPS368_READ_INTERVAL_MS)) {
        state->last_dps_read = current_millis;
        esp_err_t ret = dps368_read(state->dps368_handle, &state->dps368_data);
        if (ret == ESP_OK && state->dps368_data.pressure_valid) {
            ESP_LOGD(TAG_SENS, "DPS368: Pressure=%.1f Pa (%.1f hPa), Temp=%.1f°C", 
                     state->dps368_data.pressure_pa, state->dps368_data.pressure_pa / 100.0f, 
                     state->dps368_data.temperature_c);
        }
    }

    // Read LIS2DH12 accelerometer (every 1 second)
    if (state->lis2dh12 && (current_millis - state->last_accel_read >= 1000)) {
        state->last_accel_read = current_millis;
        
        // Debug: Poll INT1_SRC register to see if interrupt is being generated
        uint8_t int_src = 0;
        esp_err_t ret = state->lis2dh12->get_int1_source(&int_src);
        if (ret == ESP_OK && int_src != 0) {
            // Read GPIO3 pin level
            int gpio_level = gpio_get_level((gpio_num_t)LIS2DH12_INT1_GPIO);
            ESP_LOGI(TAG_SENS, "LIS2DH12: INT1_SRC=0x%02X (IA=%d) GPIO3=%d, flag=%d",
                     int_src, (int_src >> 6) & 1, gpio_level, state->motion_interrupt_triggered);
        }
        
        // Check if motion interrupt was triggered
        if (state->motion_interrupt_triggered) {
            state->motion_interrupt_triggered = false;
            state->motion_detected = true;
            ESP_LOGI(TAG_SENS, "LIS2DH12: *** Motion interrupt via ISR! ***");
            // TODO: Trigger GPS tagging here
        }
        
        // Check if data is ready
        bool data_ready = false;
        ret = state->lis2dh12->is_data_ready(&data_ready);
        
        if (ret == ESP_OK && data_ready) {
            ret = state->lis2dh12->read_accel(&state->accel_data);
            if (ret == ESP_OK) {
                state->have_accel_data = true;
                ESP_LOGD(TAG_SENS, "LIS2DH12: X=%d mg, Y=%d mg, Z=%d mg",
                         state->accel_data.x_mg, state->accel_data.y_mg, state->accel_data.z_mg);
            }
        }
    }
}

bool Sensors::isSps30Reading(int64_t now_ms, int64_t max_age_ms) {
    if (!state || !state->sps30_handle) return false;
    if (state->sps30_last_read <= 0) return false;
    return (now_ms - state->sps30_last_read) <= max_age_ms;
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
    out->pressure_pa = state->dps368_data.pressure_pa;

    // Accelerometer data
    out->have_accel = state->have_accel_data;
    if (state->have_accel_data) {
        out->accel_x_mg = state->accel_data.x_mg;
        out->accel_y_mg = state->accel_data.y_mg;
        out->accel_z_mg = state->accel_data.z_mg;
    }
    out->motion_detected = state->motion_detected;
}

i2c_master_bus_handle_t Sensors::getI2CBusHandle(void) {
    if (!state) return NULL;
    
    // Auto-initialize I2C bus if not already done
    if (state->i2c_bus_handle == NULL) {
        esp_err_t ret = initI2CBus();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG_SENS, "Failed to auto-initialize I2C bus: %s", esp_err_to_name(ret));
            return NULL;
        }
    }
    
    return state->i2c_bus_handle;
}
