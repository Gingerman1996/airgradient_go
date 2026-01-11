#include "ui_display.h"
#include "lvgl.h"

extern "C" {
    extern const lv_font_t lv_font_montserrat_32;
    extern const lv_font_t lv_font_montserrat_28;
    extern const lv_font_t lv_font_montserrat_24;
    extern const lv_font_t lv_font_montserrat_20;
    extern const lv_font_t lv_font_montserrat_16;
}

// Select largest available Montserrat font for sensor value display.
// Fallback order: 32 → 28 → 24 → 20 → 16 (based on compiled fonts).
// Returns pointer to lv_font_t structure.
static const lv_font_t* get_large_font()
{
#if defined(LV_FONT_MONTSERRAT_32)
    return &lv_font_montserrat_32;
#elif defined(LV_FONT_MONTSERRAT_28)
    return &lv_font_montserrat_28;
#elif defined(LV_FONT_MONTSERRAT_24)
    return &lv_font_montserrat_24;
#elif defined(LV_FONT_MONTSERRAT_20)
    return &lv_font_montserrat_20;
#else
    return &lv_font_montserrat_16;
#endif
}

// Format floating-point value to 1 decimal place with half-up rounding.
// Avoids float printf overhead by converting to integer math (multiply by 10).
// Supports negative values and optional suffix string.
static void format_1dp(char* out, size_t out_sz, float v, const char* suffix)
{
    // Convert to tenths with half-up rounding
    int sign = (v < 0.0f) ? -1 : 1;
    float av = v * sign;
    int tenths = (int)(av * 10.0f + 0.5f); // half-up
    int i_part = (tenths / 10) * sign;
    int d_part = tenths % 10;

    if (suffix) {
        lv_snprintf(out, out_sz, "%d.%d%s", i_part, d_part, suffix);
    } else {
        lv_snprintf(out, out_sz, "%d.%d", i_part, d_part);
    }
}

// Display class instance private members
struct Display::DisplayState {
    lv_obj_t *root = nullptr;
    lv_obj_t *battery_logo = nullptr;
    lv_obj_t *rec_label = nullptr;
    lv_obj_t *interval_label = nullptr;
    lv_obj_t *time_label = nullptr;
    lv_obj_t *alert_label = nullptr;
    lv_obj_t *pm25_label = nullptr;
    lv_obj_t *pm25_unit_label = nullptr;
    lv_obj_t *co2_label = nullptr;
    lv_obj_t *co2_unit_label = nullptr;
    lv_obj_t *voc_label = nullptr;
    lv_obj_t *nox_label = nullptr;
    lv_obj_t *temp_label = nullptr;
    lv_obj_t *rh_label = nullptr;
    lv_obj_t *pressure_label = nullptr;
    lv_obj_t *bt_logo = nullptr;
    lv_obj_t *wifi_logo = nullptr;
    lv_obj_t *gps_logo = nullptr;

    // Internal states for blinking
    bool recording = false;
    bool rec_blink_state = true;
    uint64_t last_rec_blink_ms = 0;

    bool batt_charging = false;
    bool batt_blink_state = true;
    uint64_t last_batt_blink_ms = 0;
};

// Display constructor and destructor
Display::Display() : state(new DisplayState()) {
}

Display::~Display() {
    delete state;
}

// Initialize LVGL display UI with specified width and height.
// Creates layout: status icons (top), sensor values (center), time/interval (bottom).
// Display is rotated 180° for correct orientation.
// Returns true on success, false on failure.
bool Display::init(uint16_t w, uint16_t h)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);

    state->root = lv_obj_create(scr);
    lv_obj_remove_style_all(state->root);
    lv_obj_set_size(state->root, w, h);
    lv_obj_align(state->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_update_layout(state->root);
    lv_obj_set_style_transform_pivot_x(state->root, w / 2, 0);
    lv_obj_set_style_transform_pivot_y(state->root, h / 2, 0);
    lv_obj_set_style_transform_rotation(state->root, 1800, 0);

    // Buttons/rect placeholders removed; keep layout minimal

    // lv_obj_t *rect_tl = lv_obj_create(state->root);
    // lv_obj_remove_style_all(rect_tl);
    // lv_obj_set_size(rect_tl, rect_w, rect_h);
    // lv_obj_align(rect_tl, LV_ALIGN_TOP_LEFT, 0, 0);
    // lv_obj_set_style_bg_opa(rect_tl, LV_OPA_COVER, 0);
    // lv_obj_set_style_bg_color(rect_tl, lv_color_black(), 0);
    // lv_obj_set_style_border_width(rect_tl, 2, 0);
    // lv_obj_set_style_border_color(rect_tl, lv_color_black(), 0);

    // lv_obj_t *label_t1 = lv_label_create(rect_tl);
    // lv_label_set_text(label_t1, "left");
    // lv_obj_set_style_text_color(label_t1, lv_color_white(), 0);
    // lv_obj_set_style_transform_rotation(label_t1, -900, 0);
    // lv_obj_set_style_text_font(label_t1, &lv_font_montserrat_12, 0);
    // lv_obj_align(label_t1, LV_ALIGN_BOTTOM_MID, 2, 0);

    // lv_obj_t *rect_tr = lv_obj_create(state->root);
    // lv_obj_remove_style_all(rect_tr);
    // lv_obj_set_size(rect_tr, rect_w, rect_h);
    // lv_obj_align(rect_tr, LV_ALIGN_TOP_RIGHT, 0, 0);
    // lv_obj_set_style_bg_opa(rect_tr, LV_OPA_COVER, 0);
    // lv_obj_set_style_bg_color(rect_tr, lv_color_black(), 0);
    // lv_obj_set_style_border_width(rect_tr, 2, 0);
    // lv_obj_set_style_border_color(rect_tr, lv_color_black(), 0);

    // lv_obj_t *label_t2 = lv_label_create(rect_tr);
    // lv_label_set_text(label_t2, "right");
    // lv_obj_set_style_text_color(label_t2, lv_color_white(), 0);
    // lv_obj_set_style_transform_rotation(label_t2, -900, 0);
    // lv_obj_set_style_text_font(label_t2, &lv_font_montserrat_12, 0);
    // lv_obj_align(label_t2, LV_ALIGN_BOTTOM_MID, 5, 0);

    // lv_obj_t *rect_bl = lv_obj_create(state->root);
    // lv_obj_remove_style_all(rect_bl);
    // lv_obj_set_size(rect_bl, rect_w, rect_h);
    // lv_obj_align(rect_bl, LV_ALIGN_TOP_LEFT, 0, 65);
    // lv_obj_set_style_bg_opa(rect_bl, LV_OPA_COVER, 0);
    // lv_obj_set_style_bg_color(rect_bl, lv_color_black(), 0);
    // lv_obj_set_style_border_width(rect_bl, 2, 0);
    // lv_obj_set_style_border_color(rect_bl, lv_color_black(), 0);

    // lv_obj_t *label_b1 = lv_label_create(rect_bl);
    // lv_label_set_text(label_b1, "select");
    // lv_obj_set_style_text_color(label_b1, lv_color_white(), 0);
    // lv_obj_set_style_transform_rotation(label_b1, -900, 0);
    // lv_obj_set_style_text_font(label_b1, &lv_font_montserrat_12, 0);
    // lv_obj_align(label_b1, LV_ALIGN_BOTTOM_MID, 8, 0);

    // Status icons row
    state->bt_logo = lv_label_create(state->root);
    lv_label_set_text(state->bt_logo, LV_SYMBOL_BLUETOOTH);
    lv_obj_set_style_text_color(state->bt_logo, lv_color_black(), 0);
    lv_obj_set_style_text_font(state->bt_logo, &lv_font_montserrat_16, 0);
    lv_obj_align(state->bt_logo, LV_ALIGN_TOP_MID, -35, 2);

    state->wifi_logo = lv_label_create(state->root);
    lv_label_set_text(state->wifi_logo, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(state->wifi_logo, lv_color_black(), 0);
    lv_obj_set_style_text_font(state->wifi_logo, &lv_font_montserrat_16, 0);
    lv_obj_align(state->wifi_logo, LV_ALIGN_TOP_MID, -10, 2);

    state->gps_logo = lv_label_create(state->root);
    lv_label_set_text(state->gps_logo, LV_SYMBOL_GPS);
    lv_obj_set_style_text_color(state->gps_logo, lv_color_black(), 0);
    lv_obj_set_style_text_font(state->gps_logo, &lv_font_montserrat_16, 0);
    lv_obj_align(state->gps_logo, LV_ALIGN_TOP_MID, 15, 2);

    state->battery_logo = lv_label_create(state->root);
    lv_label_set_text(state->battery_logo, LV_SYMBOL_BATTERY_EMPTY);
    lv_obj_set_style_text_color(state->battery_logo, lv_color_black(), 0);
    lv_obj_set_style_text_font(state->battery_logo, &lv_font_montserrat_16, 0);
    lv_obj_align(state->battery_logo, LV_ALIGN_TOP_MID, 40, 2);

    // Second row: Alert, REC, Time, Interval
    state->alert_label = lv_label_create(state->root);
    lv_label_set_text(state->alert_label, "");
    lv_obj_set_style_text_color(state->alert_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(state->alert_label, &lv_font_montserrat_16, 0);
    lv_obj_align(state->alert_label, LV_ALIGN_BOTTOM_LEFT, 15, -20);

    state->rec_label = lv_label_create(state->root);
    lv_label_set_text(state->rec_label, "");
    lv_obj_set_style_text_color(state->rec_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(state->rec_label, &lv_font_montserrat_16, 0);
    lv_obj_align(state->rec_label, LV_ALIGN_BOTTOM_LEFT, 35, -20);

    state->time_label = lv_label_create(state->root);
    lv_label_set_text(state->time_label, "--:--");
    lv_obj_set_style_text_color(state->time_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(state->time_label, &lv_font_montserrat_16, 0);
    lv_obj_align(state->time_label, LV_ALIGN_BOTTOM_RIGHT, -15, -20);

    state->interval_label = lv_label_create(state->root);
    lv_label_set_text(state->interval_label, "10s");
    lv_obj_set_style_text_color(state->interval_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(state->interval_label, &lv_font_montserrat_16, 0);
    lv_obj_align(state->interval_label, LV_ALIGN_TOP_RIGHT, -15, 40);

    // Main large values
    state->co2_label = lv_label_create(state->root);
    lv_label_set_text(state->co2_label, "-");
    lv_obj_set_style_text_font(state->co2_label, get_large_font(), 0);
    lv_obj_set_style_text_color(state->co2_label, lv_color_black(), 0);
    lv_obj_align(state->co2_label, LV_ALIGN_RIGHT_MID, -15, -40);

    state->co2_unit_label = lv_label_create(state->root);
    lv_label_set_text(state->co2_unit_label, "CO2 (ppm)");
    lv_obj_set_style_text_font(state->co2_unit_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(state->co2_unit_label, lv_color_black(), 0);
    lv_obj_align_to(state->co2_unit_label, state->co2_label, LV_ALIGN_TOP_RIGHT, 0, -10);

    state->pm25_label = lv_label_create(state->root);
    lv_label_set_text(state->pm25_label, "-");
    lv_obj_set_style_text_font(state->pm25_label, get_large_font(), 0);
    lv_obj_set_style_text_color(state->pm25_label, lv_color_black(), 0);
    lv_obj_align(state->pm25_label, LV_ALIGN_RIGHT_MID, -15, 20);

    state->pm25_unit_label = lv_label_create(state->root);
    lv_label_set_text(state->pm25_unit_label, "PM2.5 (ug/m3)");
    lv_obj_set_style_text_font(state->pm25_unit_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(state->pm25_unit_label, lv_color_black(), 0);
    lv_obj_align_to(state->pm25_unit_label, state->pm25_label, LV_ALIGN_TOP_RIGHT, 0, -10);

    // Bottom: Temp/RH on same line; VOC/NOx on right; pressure under interval
    state->temp_label = lv_label_create(state->root);
    lv_label_set_text(state->temp_label, "--C");
    lv_obj_set_style_text_font(state->temp_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(state->temp_label, lv_color_black(), 0);
    lv_obj_align(state->temp_label, LV_ALIGN_BOTTOM_RIGHT, -90, -80);

    state->rh_label = lv_label_create(state->root);
    lv_label_set_text(state->rh_label, "RH:--%");
    lv_obj_set_style_text_font(state->rh_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(state->rh_label, lv_color_black(), 0);
    lv_obj_align_to(state->rh_label, state->temp_label, LV_ALIGN_OUT_RIGHT_MID, 25, 0);

    state->voc_label = lv_label_create(state->root);
    lv_label_set_text(state->voc_label, "VOC:-");
    lv_obj_set_style_text_font(state->voc_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(state->voc_label, lv_color_black(), 0);
    lv_obj_align_to(state->voc_label, state->temp_label, LV_ALIGN_OUT_BOTTOM_RIGHT, -5, 0);

    state->nox_label = lv_label_create(state->root);
    lv_label_set_text(state->nox_label, "NOx:-");
    lv_obj_set_style_text_font(state->nox_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(state->nox_label, lv_color_black(), 0);
    lv_obj_align_to(state->nox_label, state->rh_label, LV_ALIGN_OUT_BOTTOM_RIGHT, -8, 0);

    state->pressure_label = lv_label_create(state->root);
    lv_label_set_text(state->pressure_label, "----hPa");
    lv_obj_set_style_text_font(state->pressure_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(state->pressure_label, lv_color_black(), 0);
    lv_obj_align(state->pressure_label, LV_ALIGN_TOP_RIGHT, -15, 55);

    return true;
}

void Display::update(uint64_t millis_now)
{
    // REC blink at 500ms when recording
    if (state->recording) {
        if (millis_now - state->last_rec_blink_ms >= 500) {
            state->rec_blink_state = !state->rec_blink_state;
            state->last_rec_blink_ms = millis_now;
        }
        lv_label_set_text(state->rec_label, state->rec_blink_state ? LV_SYMBOL_PLAY " REC" : "");
    }

    // Battery blink when charging every 1s (toggle icon visibility)
    if (state->batt_charging) {
        if (millis_now - state->last_batt_blink_ms >= 1000) {
            state->batt_blink_state = !state->batt_blink_state;
            state->last_batt_blink_ms = millis_now;
        }
        lv_obj_set_style_opa(state->battery_logo, state->batt_blink_state ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    } else {
        lv_obj_set_style_opa(state->battery_logo, LV_OPA_COVER, 0);
    }
}

void Display::setBLEStatus(BLEStatus s)
{
    // Use bold icon when connected; faded when disconnected
    lv_obj_set_style_opa(state->bt_logo, s == BLEStatus::Connected ? LV_OPA_COVER : LV_OPA_30, 0);
}

void Display::setWiFiStatus(WiFiStatus s)
{
    // Off: low opacity, Connecting: medium, Connected: full
    lv_opa_t opa = LV_OPA_30;
    if (s == WiFiStatus::Connecting) opa = LV_OPA_60;
    else if (s == WiFiStatus::Connected) opa = LV_OPA_COVER;
    lv_obj_set_style_opa(state->wifi_logo, opa, 0);
}

void Display::setGPSStatus(GPSStatus s)
{
    // Off: low, Searching: medium, Fix: full
    lv_opa_t opa = LV_OPA_30;
    if (s == GPSStatus::Searching) opa = LV_OPA_60;
    else if (s == GPSStatus::Fix) opa = LV_OPA_COVER;
    lv_obj_set_style_opa(state->gps_logo, opa, 0);
}

void Display::setBattery(int percent, bool charging)
{
    state->batt_charging = charging;
    const char *bat_symbol;
    if (percent >= 90) bat_symbol = LV_SYMBOL_BATTERY_FULL;
    else if (percent >= 60) bat_symbol = LV_SYMBOL_BATTERY_3;
    else if (percent >= 40) bat_symbol = LV_SYMBOL_BATTERY_2;
    else if (percent >= 20) bat_symbol = LV_SYMBOL_BATTERY_1;
    else bat_symbol = LV_SYMBOL_BATTERY_EMPTY;
    lv_label_set_text(state->battery_logo, bat_symbol);
}

void Display::setRecording(bool on)
{
    state->recording = on;
    if (!state->recording) {
        lv_label_set_text(state->rec_label, LV_SYMBOL_PAUSE " PAUSE");
    }
}

void Display::setAlert(bool on)
{
    lv_label_set_text(state->alert_label, on ? LV_SYMBOL_WARNING : "");
}

void Display::setIntervalSeconds(int seconds)
{
    char buf[12];
    if (seconds > 0) {
        lv_snprintf(buf, sizeof(buf), "%ds", seconds);
    } else {
        lv_snprintf(buf, sizeof(buf), "--s");
    }
    lv_label_set_text(state->interval_label, buf);
}

void Display::setTimeHM(int hours, int minutes, bool valid)
{
    char time_buf[8];
    if (valid) lv_snprintf(time_buf, sizeof(time_buf), "%02d:%02d", hours, minutes);
    else lv_snprintf(time_buf, sizeof(time_buf), "--:--");
    lv_label_set_text(state->time_label, time_buf);
}

void Display::setPM25(int v)
{
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d", v);
    lv_label_set_text(state->pm25_label, buf);
}

void Display::setCO2(int v)
{
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d", v);
    lv_label_set_text(state->co2_label, buf);
}

void Display::setVOC(int v)
{
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "VOC:%d", v);
    lv_label_set_text(state->voc_label, buf);
}

void Display::setNOx(int v)
{
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "NOx:%d", v);
    lv_label_set_text(state->nox_label, buf);
}

void Display::setTempC(int v)
{
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%dC", v);
    lv_label_set_text(state->temp_label, buf);
}

void Display::setRH(int v)
{
    char buf[12];
    lv_snprintf(buf, sizeof(buf), "RH:%d%%", v);
    lv_label_set_text(state->rh_label, buf);
}

void Display::setPressure(int v)
{
    char buf[12];
    lv_snprintf(buf, sizeof(buf), "%dhPa", v);
    lv_label_set_text(state->pressure_label, buf);
}

void Display::setPM25f(float v)
{
    char buf[16];
    format_1dp(buf, sizeof(buf), v, nullptr);
    lv_label_set_text(state->pm25_label, buf);
}

void Display::setTempCf(float v)
{
    char buf[16];
    // Append 'C' suffix
    format_1dp(buf, sizeof(buf), v, "C");
    lv_label_set_text(state->temp_label, buf);
}

void Display::setRHf(float v)
{
    char buf[20];
    // Prefix with RH: and append %
    char val[16];
    format_1dp(val, sizeof(val), v, "%");
    lv_snprintf(buf, sizeof(buf), "RH:%s", val);
    lv_label_set_text(state->rh_label, buf);
}
