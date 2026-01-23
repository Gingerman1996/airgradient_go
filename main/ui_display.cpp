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
static const lv_font_t *get_large_font() {
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
static void format_1dp(char *out, size_t out_sz, float v, const char *suffix) {
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

static void apply_tile_focus(lv_obj_t *tile, lv_obj_t *value_label,
                             lv_obj_t *unit_label, bool focused) {
  if (!tile || !value_label || !unit_label) {
    return;
  }

  lv_obj_set_style_bg_color(tile, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(tile, focused ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(tile, 1, 0);
  lv_obj_set_style_border_color(tile, lv_color_black(), 0);
  lv_obj_set_style_border_opa(tile, focused ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
  lv_obj_set_style_text_color(value_label,
                              focused ? lv_color_white() : lv_color_black(), 0);
  lv_obj_set_style_text_color(unit_label,
                              focused ? lv_color_white() : lv_color_black(), 0);
}

static const char *battery_symbol_for_percent(int percent) {
  if (percent >= 90)
    return LV_SYMBOL_BATTERY_FULL;
  if (percent >= 60)
    return LV_SYMBOL_BATTERY_3;
  if (percent >= 40)
    return LV_SYMBOL_BATTERY_2;
  if (percent >= 20)
    return LV_SYMBOL_BATTERY_1;
  return LV_SYMBOL_BATTERY_EMPTY;
}

static const char *const kBatteryChargeAnimSymbols[] = {
    LV_SYMBOL_BATTERY_1,    LV_SYMBOL_BATTERY_2,     LV_SYMBOL_BATTERY_3,
    LV_SYMBOL_BATTERY_FULL, LV_SYMBOL_BATTERY_EMPTY,
};

// Display class instance private members
struct Display::DisplayState {
  lv_obj_t *root = nullptr;
  lv_obj_t *battery_logo = nullptr;
  lv_obj_t *battery_percent_label = nullptr;
  lv_obj_t *rec_label = nullptr;
  lv_obj_t *interval_label = nullptr;
  lv_obj_t *time_label = nullptr;
  lv_obj_t *alert_label = nullptr;
  lv_obj_t *co2_tile = nullptr;
  lv_obj_t *pm25_tile = nullptr;
  lv_obj_t *pm25_label = nullptr;
  lv_obj_t *pm25_unit_label = nullptr;
  lv_obj_t *co2_label = nullptr;
  lv_obj_t *co2_unit_label = nullptr;
  lv_obj_t *voc_label = nullptr;
  lv_obj_t *nox_label = nullptr;
  lv_obj_t *temp_label = nullptr;
  lv_obj_t *rh_label = nullptr;
  lv_obj_t *pressure_label = nullptr;
  lv_obj_t *lat_label = nullptr;
  lv_obj_t *lon_label = nullptr;
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
  uint64_t last_batt_anim_ms = 0;
  uint8_t batt_anim_index = 0;
  int batt_percent = 0;
  bool bt_blink_state = true;
  uint64_t last_bt_blink_ms = 0;
  Display::BLEStatus ble_status = Display::BLEStatus::Disconnected;

  Display::FocusTile focus_tile = Display::FocusTile::CO2;
};

// Display constructor and destructor
Display::Display() : state(new DisplayState()) {}

Display::~Display() { delete state; }

// Initialize LVGL display UI with specified width and height.
// Creates layout: status icons (top), sensor values (center), time/interval
// (bottom). Display is rotated 180° for correct orientation. Returns true on
// success, false on failure.
bool Display::init(uint16_t w, uint16_t h) {
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
  lv_obj_align(state->bt_logo, LV_ALIGN_TOP_MID, -55, 2);

  state->wifi_logo = lv_label_create(state->root);
  lv_label_set_text(state->wifi_logo, LV_SYMBOL_WIFI);
  lv_obj_set_style_text_color(state->wifi_logo, lv_color_black(), 0);
  lv_obj_set_style_text_font(state->wifi_logo, &lv_font_montserrat_16, 0);
  lv_obj_align(state->wifi_logo, LV_ALIGN_TOP_MID, -30, 2);

  state->gps_logo = lv_label_create(state->root);
  lv_label_set_text(state->gps_logo, LV_SYMBOL_GPS);
  lv_obj_set_style_text_color(state->gps_logo, lv_color_black(), 0);
  lv_obj_set_style_text_font(state->gps_logo, &lv_font_montserrat_16, 0);
  lv_obj_align(state->gps_logo, LV_ALIGN_TOP_MID, -5, 2);

  state->battery_logo = lv_label_create(state->root);
  lv_label_set_text(state->battery_logo, LV_SYMBOL_BATTERY_EMPTY);
  lv_obj_set_style_text_color(state->battery_logo, lv_color_black(), 0);
  lv_obj_set_style_text_font(state->battery_logo, &lv_font_montserrat_16, 0);
  lv_obj_align(state->battery_logo, LV_ALIGN_TOP_MID, 15, 2);

  state->battery_percent_label = lv_label_create(state->root);
  lv_label_set_text(state->battery_percent_label, "100%");
  lv_obj_set_style_text_color(state->battery_percent_label, lv_color_black(),
                              0);
  lv_obj_set_style_text_font(state->battery_percent_label,
                             &lv_font_montserrat_12, 0);
  lv_obj_align_to(state->battery_percent_label, state->battery_logo,
                  LV_ALIGN_OUT_RIGHT_MID, 4, 0);

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
  lv_obj_align(state->interval_label, LV_ALIGN_TOP_RIGHT, -15, 20);

  // Main large values (CO2 / PM2.5 tiles)
  const int tile_w = 127;
  const int tile_h = 60;
  const int tile_x = -8;
  const int co2_tile_y = -60;
  const int pm_tile_y = 10;
  const int tile_pad_x = 6;
  const int tile_pad_y = 4;

  state->co2_tile = lv_obj_create(state->root);
  lv_obj_remove_style_all(state->co2_tile);
  lv_obj_set_size(state->co2_tile, tile_w, tile_h);
  lv_obj_align(state->co2_tile, LV_ALIGN_RIGHT_MID, tile_x, co2_tile_y);
  lv_obj_set_style_radius(state->co2_tile, 6, 0);
  lv_obj_set_style_pad_all(state->co2_tile, 0, 0);
  lv_obj_clear_flag(state->co2_tile, LV_OBJ_FLAG_SCROLLABLE);

  state->co2_label = lv_label_create(state->co2_tile);
  lv_label_set_text(state->co2_label, "-");
  lv_obj_set_style_text_font(state->co2_label, get_large_font(), 0);
  lv_obj_align(state->co2_label, LV_ALIGN_BOTTOM_RIGHT, -tile_pad_x,
               -tile_pad_y);

  state->co2_unit_label = lv_label_create(state->co2_tile);
  lv_label_set_text(state->co2_unit_label, "CO2 (ppm)");
  lv_obj_set_style_text_font(state->co2_unit_label, &lv_font_montserrat_16, 0);
  lv_obj_align_to(state->co2_unit_label, state->co2_label,
                  LV_ALIGN_OUT_TOP_RIGHT, 0, -4);

  state->pm25_tile = lv_obj_create(state->root);
  lv_obj_remove_style_all(state->pm25_tile);
  lv_obj_set_size(state->pm25_tile, tile_w, tile_h);
  lv_obj_align(state->pm25_tile, LV_ALIGN_RIGHT_MID, tile_x, pm_tile_y);
  lv_obj_set_style_radius(state->pm25_tile, 6, 0);
  lv_obj_set_style_pad_all(state->pm25_tile, 0, 0);
  lv_obj_clear_flag(state->pm25_tile, LV_OBJ_FLAG_SCROLLABLE);

  state->pm25_label = lv_label_create(state->pm25_tile);
  lv_label_set_text(state->pm25_label, "-");
  lv_obj_set_style_text_font(state->pm25_label, get_large_font(), 0);
  lv_obj_align(state->pm25_label, LV_ALIGN_BOTTOM_RIGHT, -tile_pad_x,
               -tile_pad_y);

  state->pm25_unit_label = lv_label_create(state->pm25_tile);
  lv_label_set_text(state->pm25_unit_label, "PM2.5 (ug/m3)");
  lv_obj_set_style_text_font(state->pm25_unit_label, &lv_font_montserrat_12, 0);
  lv_obj_align_to(state->pm25_unit_label, state->pm25_label,
                  LV_ALIGN_OUT_TOP_RIGHT, 0, -4);

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
  lv_obj_align_to(state->rh_label, state->temp_label, LV_ALIGN_OUT_RIGHT_MID,
                  25, 0);

  state->voc_label = lv_label_create(state->root);
  lv_label_set_text(state->voc_label, "VOC:-");
  lv_obj_set_style_text_font(state->voc_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(state->voc_label, lv_color_black(), 0);
  lv_obj_align_to(state->voc_label, state->temp_label,
                  LV_ALIGN_OUT_BOTTOM_RIGHT, -5, 0);

  state->nox_label = lv_label_create(state->root);
  lv_label_set_text(state->nox_label, "NOx:-");
  lv_obj_set_style_text_font(state->nox_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(state->nox_label, lv_color_black(), 0);
  lv_obj_align_to(state->nox_label, state->rh_label, LV_ALIGN_OUT_BOTTOM_RIGHT,
                  -8, 0);

  // GPS coordinates below VOC and NOx
  state->lat_label = lv_label_create(state->root);
  lv_label_set_text(state->lat_label, "--,--");
  lv_obj_set_style_text_font(state->lat_label, &lv_font_montserrat_10, 0);
  lv_obj_set_style_text_color(state->lat_label, lv_color_black(), 0);
  lv_obj_align_to(state->lat_label, state->voc_label, LV_ALIGN_OUT_BOTTOM_LEFT,
                  0, 0);

  state->lon_label = lv_label_create(state->root);
  lv_label_set_text(state->lon_label, "--,--");
  lv_obj_set_style_text_font(state->lon_label, &lv_font_montserrat_10, 0);
  lv_obj_set_style_text_color(state->lon_label, lv_color_black(), 0);
  lv_obj_align_to(state->lon_label, state->nox_label, LV_ALIGN_OUT_BOTTOM_LEFT,
                  0, 0);

  state->pressure_label = lv_label_create(state->root);
  lv_label_set_text(state->pressure_label, "----hPa");
  lv_obj_set_style_text_font(state->pressure_label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(state->pressure_label, lv_color_black(), 0);
  lv_obj_align(state->pressure_label, LV_ALIGN_TOP_RIGHT, -15, 40);

  setFocusTile(state->focus_tile);

  return true;
}

void Display::update(uint64_t millis_now) {
  // BLE blink every 500ms
  if (millis_now - state->last_bt_blink_ms >= 500) {
    state->bt_blink_state = !state->bt_blink_state;
    state->last_bt_blink_ms = millis_now;
  }
  lv_opa_t ble_base_opa =
      (state->ble_status == BLEStatus::Connected) ? LV_OPA_COVER : LV_OPA_30;
  lv_obj_set_style_opa(state->bt_logo,
                       state->bt_blink_state ? ble_base_opa : LV_OPA_TRANSP, 0);

  // REC blink at 500ms when recording
  if (state->recording) {
    if (millis_now - state->last_rec_blink_ms >= 500) {
      state->rec_blink_state = !state->rec_blink_state;
      state->last_rec_blink_ms = millis_now;
    }
    lv_label_set_text(state->rec_label,
                      state->rec_blink_state ? LV_SYMBOL_PLAY " REC" : "");
  }

  // Battery blink and charge animation when charging
  if (state->batt_charging) {
    if (millis_now - state->last_batt_blink_ms >= 500) {
      state->batt_blink_state = !state->batt_blink_state;
      state->last_batt_blink_ms = millis_now;
    }
    if (millis_now - state->last_batt_anim_ms >= 500) {
      state->batt_anim_index =
          (state->batt_anim_index + 1) % (sizeof(kBatteryChargeAnimSymbols) /
                                          sizeof(kBatteryChargeAnimSymbols[0]));
      lv_label_set_text(state->battery_logo,
                        kBatteryChargeAnimSymbols[state->batt_anim_index]);
      state->last_batt_anim_ms = millis_now;
    }
    lv_obj_set_style_opa(state->battery_logo,
                         state->batt_blink_state ? LV_OPA_COVER : LV_OPA_TRANSP,
                         0);
  } else {
    lv_obj_set_style_opa(state->battery_logo, LV_OPA_COVER, 0);
  }
}

void Display::setBLEStatus(BLEStatus s) { state->ble_status = s; }

void Display::setWiFiStatus(WiFiStatus s) {
  // Off: low opacity, Connecting: medium, Connected: full
  lv_opa_t opa = LV_OPA_30;
  if (s == WiFiStatus::Connecting)
    opa = LV_OPA_60;
  else if (s == WiFiStatus::Connected)
    opa = LV_OPA_COVER;
  lv_obj_set_style_opa(state->wifi_logo, opa, 0);
}

void Display::setGPSStatus(GPSStatus s) {
  // Off: low, Searching: medium, Fix: full
  lv_opa_t opa = LV_OPA_30;
  if (s == GPSStatus::Searching)
    opa = LV_OPA_60;
  else if (s == GPSStatus::Fix)
    opa = LV_OPA_COVER;
  lv_obj_set_style_opa(state->gps_logo, opa, 0);
}

void Display::setBattery(int percent, bool charging) {
  bool was_charging = state->batt_charging;

  if (percent < 0)
    percent = 0;
  if (percent > 100)
    percent = 100;

  state->batt_percent = percent;
  state->batt_charging = charging;

  char percent_buf[8];
  lv_snprintf(percent_buf, sizeof(percent_buf), "%d%%", percent);
  lv_label_set_text(state->battery_percent_label, percent_buf);

  if (charging) {
    if (!was_charging) {
      state->batt_anim_index = 0;
      state->last_batt_anim_ms = 0;
      state->batt_blink_state = true;
      state->last_batt_blink_ms = 0;
    }
    lv_obj_set_style_text_font(state->battery_percent_label,
                               &lv_font_montserrat_16, 0);
    lv_label_set_text(state->battery_logo,
                      kBatteryChargeAnimSymbols[state->batt_anim_index]);
  } else {
    lv_obj_set_style_text_font(state->battery_percent_label,
                               &lv_font_montserrat_12, 0);
    lv_label_set_text(state->battery_logo, battery_symbol_for_percent(percent));
    state->batt_blink_state = true;
    state->last_batt_blink_ms = 0;
    state->last_batt_anim_ms = 0;
  }

  lv_obj_align_to(state->battery_percent_label, state->battery_logo,
                  LV_ALIGN_OUT_RIGHT_MID, 4, 0);
}

void Display::setRecording(bool on) {
  state->recording = on;
  if (!state->recording) {
    lv_label_set_text(state->rec_label, LV_SYMBOL_PAUSE " PAUSE");
  }
}

void Display::setAlert(bool on) {
  lv_label_set_text(state->alert_label, on ? LV_SYMBOL_WARNING : "");
}

void Display::setIntervalSeconds(int seconds) {
  char buf[12];
  if (seconds > 0) {
    lv_snprintf(buf, sizeof(buf), "%ds", seconds);
  } else {
    lv_snprintf(buf, sizeof(buf), "--s");
  }
  lv_label_set_text(state->interval_label, buf);
}

void Display::setTimeHM(int hours, int minutes, bool valid) {
  char time_buf[8];
  if (valid)
    lv_snprintf(time_buf, sizeof(time_buf), "%02d:%02d", hours, minutes);
  else
    lv_snprintf(time_buf, sizeof(time_buf), "--:--");
  lv_label_set_text(state->time_label, time_buf);
}

void Display::setFocusTile(FocusTile tile) {
  state->focus_tile = tile;

  const bool co2_focused = (tile == FocusTile::CO2);
  const bool pm_focused = (tile == FocusTile::PM25);

  apply_tile_focus(state->co2_tile, state->co2_label, state->co2_unit_label,
                   co2_focused);
  apply_tile_focus(state->pm25_tile, state->pm25_label, state->pm25_unit_label,
                   pm_focused);
}

void Display::setPM25(int v) {
  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%d", v);
  lv_label_set_text(state->pm25_label, buf);
}

void Display::setCO2(int v) {
  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%d", v);
  lv_label_set_text(state->co2_label, buf);
}

void Display::setVOC(int v) {
  char buf[16];
  lv_snprintf(buf, sizeof(buf), "VOC:%d", v);
  lv_label_set_text(state->voc_label, buf);
}

void Display::setNOx(int v) {
  char buf[16];
  lv_snprintf(buf, sizeof(buf), "NOx:%d", v);
  lv_label_set_text(state->nox_label, buf);
}

void Display::setTempC(int v) {
  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%dC", v);
  lv_label_set_text(state->temp_label, buf);
}

void Display::setRH(int v) {
  char buf[12];
  lv_snprintf(buf, sizeof(buf), "RH:%d%%", v);
  lv_label_set_text(state->rh_label, buf);
}

void Display::setPressure(int v) {
  char buf[12];
  lv_snprintf(buf, sizeof(buf), "%dhPa", v);
  lv_label_set_text(state->pressure_label, buf);
}

void Display::setPM25f(float v) {
  char buf[16];
  format_1dp(buf, sizeof(buf), v, nullptr);
  lv_label_set_text(state->pm25_label, buf);
}

void Display::setTempCf(float v) {
  char buf[16];
  // Append 'C' suffix
  format_1dp(buf, sizeof(buf), v, "C");
  lv_label_set_text(state->temp_label, buf);
}

void Display::setRHf(float v) {
  char buf[20];
  // Prefix with RH: and append %
  char val[16];
  format_1dp(val, sizeof(val), v, "%");
  lv_snprintf(buf, sizeof(buf), "RH:%s", val);
  lv_label_set_text(state->rh_label, buf);
}

void Display::setLatLon(float lat, float lon, bool valid) {
  if (!valid) {
    lv_label_set_text(state->lat_label, "--,--");
    lv_label_set_text(state->lon_label, "--,--");
  } else {
    char lat_buf[16];
    char lon_buf[16];
    lv_snprintf(lat_buf, sizeof(lat_buf), "%.4f", lat);
    lv_snprintf(lon_buf, sizeof(lon_buf), "%.4f", lon);
    lv_label_set_text(state->lat_label, lat_buf);
    lv_label_set_text(state->lon_label, lon_buf);
  }
}
