# AirGradient GO - Power States

## 🔌 QON Button Connection

The QON button is connected to both:
- **ESP32-C5 (IO5)**: For detecting button press in firmware
- **BQ25628 QON pin**: For hardware power control

```
              ┌─────────────────┐
              │   QON Button    │
              └────────┬────────┘
                       │
           ┌───────────┴───────────┐
           │                       │
           ▼                       ▼
    ┌──────────────┐        ┌──────────────┐
    │  ESP32-C5    │        │   BQ25628    │
    │   (IO5)      │        │   (QON pin)  │
    └──────────────┘        └──────────────┘
```

---

## ⚡ Power States Overview

| State | Description | System Status |
|-------|-------------|---------------|
| **Ship Mode** | Ultra-low power, charger disabled | OFF (< 1µA) |
| **Active Mode** | Normal operation | ON |
| **Charging** | Battery charging while active | ON + Charging |

---

## 🟢 Power ON Sequence

### Trigger
Press the QON button (short press)

### Sequence
```
1. QON button pressed
       │
       ▼
2. BQ25628 receives QON signal
       │
       ▼
3. BQ25628 exits Ship Mode
       │
       ▼
4. SYS output enabled → TPS63802DLAR (3.1V DC-DC)
       │
       ▼
5. System power available
       │
       ▼
6. ESP32-C5 boots up
       │
       ▼
7. Firmware initialization (see detailed sequence below)
       │
       ▼
8. Device ready for operation
```

### Detailed Firmware Initialization Sequence

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    FIRMWARE BOOT SEQUENCE                               │
└─────────────────────────────────────────────────────────────────────────┘

Phase 1: SPI + Display (First Priority)
─────────────────────────────────────────
    │
    ├──► Initialize SPI bus (IO23=SCLK, IO25=MOSI, IO0=CS)
    │
    ├──► Initialize E-paper display (SSD1680)
    │
    └──► Display TEST SCREEN:
         ┌─────────────────────────────────────────┐
         │  Status icons: WiFi, GPS, BLE, Battery  │
         │  (Show all icons visible for testing)   │
         │                                         │
         │  PM2.5:  999.9  µg/m³                  │
         │  CO2:    9999   ppm                     │
         │  VOC:     999                           │
         │  NOx:     999                           │
         │  Temp:   99.99  °C                      │
         │  RH:     99.99  %                       │
         │  Press:   9999  hPa                     │
         │                                         │
         │  [All max values for display testing]   │
         └─────────────────────────────────────────┘
         │
         ▼
Phase 2: I2C + UART Initialization
─────────────────────────────────────────
    │
    ├──► Initialize I2C bus (IO6=SCL, IO7=SDA)
    │
    └──► Initialize UART for GPS (TXD0, RXD0)
         │
         ▼
Phase 3: Hardware Check
─────────────────────────────────────────
    │
    ├──► Check BQ25628 (Charger) @ 0x6A
    │    └── Read status, battery voltage
    │
    ├──► Check Flash Memory (W25N512GVEIG)
    │    └── Read JEDEC ID, verify communication
    │
    ├──► Check GPS Module
    │    └── Wait for NMEA response (timeout: 2s)
    │
    ├──► Check Sensors:
    │    ├── STCC4/SCD41 @ 0x64/0x62 (CO2)
    │    ├── SHT40 @ 0x44 (Temp/RH)
    │    ├── SGP41 @ 0x59 (VOC/NOx)
    │    ├── DPS368 @ 0x76 (Pressure)
    │    ├── SPS30/PMSA003I @ 0x69/0x12 (PM)
    │    └── LIS2DH12 @ 0x18 (Accelerometer)
    │
    ├──► Check Peripherals:
    │    ├── CAP1203 @ 0x28 (Buttons)
    │    └── LP5030/LP5036 @ 0x30 (LEDs)
    │
    └──► If any errors detected:
         │
         ▼
    ┌─────────────────────────────────────────┐
    │  ERROR SCREEN (displayed immediately)   │
    │                                         │
    │  ⚠ Hardware Check Failed               │
    │                                         │
    │  ✗ Charger: NOT FOUND                  │
    │  ✓ Flash: OK                           │
    │  ✗ GPS: TIMEOUT                        │
    │  ✓ CO2 Sensor: OK (STCC4)              │
    │  ✓ VOC Sensor: OK                      │
    │  ✗ PM Sensor: NOT FOUND                │
    │  ...                                    │
    │                                         │
    │  Press any button to continue           │
    └─────────────────────────────────────────┘
         │
         ▼
Phase 4: Start Normal Operation
─────────────────────────────────────────
    │
    ├──► Start sensor reading tasks
    │
    ├──► Start recording (if enabled)
    │
    ├──► Update display with real sensor values
    │
    └──► Enter main loop
```

### Test Screen Values

| Display Field | Test Value | Format | Notes |
|---------------|------------|--------|-------|
| PM2.5 | 999.9 | ###.# | 1 decimal place |
| CO2 | 9999 | #### | Max 4 digits |
| VOC Index | 999 | ### | Max 3 digits |
| NOx Index | 999 | ### | Max 3 digits |
| Temperature | 99.99 | ##.## | 2 decimal places |
| Humidity | 99.99 | ##.## | 2 decimal places |
| Pressure | 9999 | #### | Max 4 digits |
| Battery | 100% | ###% | Percentage |

> [!NOTE]
> The test screen displays maximum values to verify:
> 1. All display segments are working
> 2. Font rendering is correct
> 3. Layout alignment is proper
> 4. Status icons are visible

---

## 🔴 Power OFF Sequence

### Trigger
Long press QON button for **5 seconds**

### Sequence
```
1. QON button held for 5 seconds
       │
       ▼
2. ESP32-C5 detects long press on IO5
       │
       ▼
3. Display: "Shutting down..."
       │
       ▼
4. Firmware initiates shutdown with status messages:
   │
   ├──► Display: "Stopping sensors..."
   │    └── Stop all sensor readings
   │
   ├──► Display: "Stopping recording..."
   │    └── Stop recording (if active)
   │
   ├──► Display: "Saving data..."
   │    └── Save pending data to flash
   │
   ├──► Display: "Powering off..."
   │    └── Wait 500ms for user to see message
   │
   └──► Clear E-paper display (blank screen)
       │
       ▼
5. Send Ship Mode command to BQ25628 via I2C
       │
       ▼
6. BQ25628 enters Ship Mode
       │
       ▼
7. SYS output disabled
       │
       ▼
8. System power cut off
       │
       ▼
9. Device OFF (ultra-low power)
```

### Shutdown Screen Sequence

```
┌──────────────────────────────────┐
│                                  │
│      Shutting down...            │
│                                  │
│      Stopping sensors...         │
│                                  │
└──────────────────────────────────┘
              │
              ▼
┌──────────────────────────────────┐
│                                  │
│      Shutting down...            │
│                                  │
│      Stopping recording...       │
│                                  │
└──────────────────────────────────┘
              │
              ▼
┌──────────────────────────────────┐
│                                  │
│      Shutting down...            │
│                                  │
│      Saving data...              │
│                                  │
└──────────────────────────────────┘
              │
              ▼
┌──────────────────────────────────┐
│                                  │
│      Powering off...             │
│                                  │
│      Goodbye!                    │
│                                  │
└──────────────────────────────────┘
              │
              ▼
┌──────────────────────────────────┐
│                                  │
│          (blank screen)          │
│                                  │
└──────────────────────────────────┘
```

---

## ⚙️ BQ25628 Ship Mode Command

### I2C Command Sequence
```c
// BQ25628 I2C Address: 0x6A

// To enter Ship Mode, write to REG0x09 (Charger Control 1)
// Set BATFET_DLY bit and EN_SHIP_DCHG bit

// Step 1: Set BATFET off delay
bq25628_write_reg(0x09, 0x40);  // Set appropriate bits

// Step 2: Enter ship mode
bq25628_write_reg(0x09, 0x20);  // EN_SHIP_DCHG = 1
```

> [!IMPORTANT]
> Before sending Ship Mode command:
> - Clear E-paper display to prevent ghosting
> - Save all pending data
> - Disable all peripherals
> - Allow ~100ms delay for operations to complete

---

## ⏱️ Timing Parameters

| Event | Duration | Notes |
|-------|----------|-------|
| Power ON (button press) | < 500ms | Until ESP32 boots |
| Shutdown detect (long press) | 5 seconds | Firmware-controlled |
| E-paper clear | ~1-2 seconds | Full refresh to blank |
| Ship Mode entry | ~100ms | After I2C command |
| Default HW shutdown | ~12 seconds | If not handled by firmware |

---

## 🔋 Default Hardware Behavior (Fallback)

If firmware does not handle the QON button:

| Action | Duration | Result |
|--------|----------|--------|
| QON press | < 12s | No effect (firmware should handle) |
| QON hold | ~12s | BQ25628 enters Sleep Mode automatically |

> [!NOTE]
> The 12-second hardware timeout acts as a safety fallback if firmware becomes unresponsive.
> Our firmware implementation overrides this with a 5-second software shutdown.

---

## 📊 State Diagram

```
                    ┌──────────────────┐
                    │                  │
                    │    SHIP MODE     │◄────────────────────────┐
                    │   (Device OFF)   │                         │
                    │                  │                         │
                    └────────┬─────────┘                         │
                             │                                   │
                    QON button press                    I2C Ship Mode command
                             │                          (after 5s hold)
                             ▼                                   │
                    ┌──────────────────┐                         │
                    │                  │                         │
                    │   ACTIVE MODE    │─────────────────────────┘
                    │  (Device ON)     │
                    │                  │
                    └────────┬─────────┘
                             │
                    USB connected?
                             │
              ┌──────────────┴──────────────┐
              │                             │
         No (Battery)                  Yes (USB)
              │                             │
              ▼                             ▼
      ┌──────────────┐              ┌──────────────┐
      │              │              │              │
      │  BAT MODE    │              │  CHARGING    │
      │ (Battery pwr)│              │   MODE       │
      │              │              │              │
      └──────────────┘              └──────────────┘
```

---

## 💻 Firmware Implementation

### QON Button Handler (Pseudo-code)
```c
#define QON_PIN         GPIO_NUM_5
#define SHUTDOWN_HOLD_MS  5000

static uint32_t qon_press_start = 0;
static bool shutdown_initiated = false;

void qon_button_task(void *arg) {
    while (1) {
        bool qon_pressed = gpio_get_level(QON_PIN) == 0;  // Active low
        
        if (qon_pressed && qon_press_start == 0) {
            // Button just pressed
            qon_press_start = esp_timer_get_time() / 1000;
        }
        else if (qon_pressed && !shutdown_initiated) {
            // Button still held
            uint32_t hold_time = (esp_timer_get_time() / 1000) - qon_press_start;
            
            if (hold_time >= SHUTDOWN_HOLD_MS) {
                shutdown_initiated = true;
                initiate_shutdown();
            }
        }
        else if (!qon_pressed) {
            // Button released
            qon_press_start = 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void initiate_shutdown(void) {
    ESP_LOGI(TAG, "Shutdown initiated...");
    
    // 1. Stop all tasks
    stop_sensor_tasks();
    stop_recording();
    
    // 2. Save pending data
    save_pending_data_to_flash();
    
    // 3. Clear E-paper display
    epaper_clear_display();
    epaper_wait_busy();
    
    // 4. Small delay to ensure display is cleared
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 5. Enter ship mode
    bq25628_enter_ship_mode();
    
    // System will power off here
    // This code should never be reached
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

---

## ⚠️ Important Notes

1. **E-paper persistence**: E-paper displays retain their image when power is removed. Always clear the display before shutdown to avoid showing stale data on next boot.

2. **Data integrity**: Ensure all sensor data and settings are saved to flash before entering Ship Mode.

3. **USB connected**: If USB is connected, the device may not fully power off even in Ship Mode (charger keeps system powered via SYS).

4. **Wake sources**: The only way to exit Ship Mode is by pressing the QON button. There is no wake-from-timer in Ship Mode.

5. **Battery protection**: Ship Mode disconnects the battery FET, providing maximum battery protection during long-term storage.
