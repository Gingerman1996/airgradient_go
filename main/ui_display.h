#pragma once

#include <stdint.h>

// Blink cadence for UI icons; keep aligned with LVGL refresh limits.
#define UI_BLINK_INTERVAL_MS 1500

class Display {
public:
    enum class BLEStatus { Disconnected, Connected };
    enum class WiFiStatus { Off, Connecting, Connected };
    enum class GPSStatus { Off, Searching, Fix };
    enum class FocusTile { CO2, PM25 };

    // Constructor and destructor
    Display();
    ~Display();

    // Initialize UI for given width/height. Creates all LVGL objects and layout.
    bool init(uint16_t width, uint16_t height);

    // Call periodically to manage blink states (REC, charging) based on time.
    void update(uint64_t millis_now);

    // Status icons
    void setBLEStatus(BLEStatus s);
    void setWiFiStatus(WiFiStatus s);
    void setGPSStatus(GPSStatus s);

    // Battery % and charging flag (blink when charging)
    void setBattery(int percent, bool charging);

    // Recording indicator and alert
    void setRecording(bool recording);
    void setAlert(bool on);

    // Time and interval controls
    void setIntervalSeconds(int seconds);
    void setTimeText(const char *text);
    void setTimeHM(int hours, int minutes, bool valid);

    // Tile focus
    void setFocusTile(FocusTile tile);

    // Sensor values
    void setPM25(int v);
    void setCO2(int v);
    void setVOC(int v);
    void setNOx(int v);
    void setTempC(int v);
    void setRH(int v);
    void setPressure(int v);

    // One-decimal display helpers (rounded half-up)
    void setPM25f(float v);
    void setTempCf(float v);
    void setRHf(float v);

    // GPS coordinates (latitude and longitude)
    void setLatLon(float lat, float lon, bool valid);

public:
    struct DisplayState;

private:
    DisplayState* state;
};
