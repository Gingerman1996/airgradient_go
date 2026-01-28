#include "ui_display.h"
#include "lvgl.h"

extern "C" {
extern const lv_font_t lv_font_montserrat_32;
extern const lv_font_t lv_font_montserrat_28;
extern const lv_font_t lv_font_montserrat_24;
extern const lv_font_t lv_font_montserrat_20;
extern const lv_font_t lv_font_montserrat_16;
}

// Layout parameters structure for different display resolutions
struct LayoutParams {
  // Display dimensions
  uint16_t width;
  uint16_t height;
  // Font sizes
  const lv_font_t *large_font;     // CO2/PM2.5 values
  const lv_font_t *medium_font;    // Unit labels
  const lv_font_t *small_font;     // Status, temp, RH
  const lv_font_t *tiny_font;      // GPS coords, pressure
  const lv_font_t *icon_font;      // Status bar icons
  // Tile dimensions
  int tile_w;
  int tile_h;
  int tile_x_offset;
  int tile_pad_x;
  int tile_pad_y;
  // Vertical positions
  int co2_tile_y;
  int pm_tile_y;
  // Status bar
  int status_icon_spacing;
  int status_y;
  // Bottom row
  int bottom_row_y;
  int temp_rh_y;
  int voc_nox_y;
  int gps_y;
  int pressure_y;
  int interval_y;
};

// Get layout parameters based on display size
static LayoutParams get_layout_params(uint16_t w, uint16_t h) {
  LayoutParams p = {};
  p.width = w;
  p.height = h;

  if (h <= 250) {
    // Small display: 122x250
    p.large_font = &lv_font_montserrat_24;
    p.medium_font = &lv_font_montserrat_12;
    p.small_font = &lv_font_montserrat_12;
    p.tiny_font = &lv_font_montserrat_10;
    p.icon_font = &lv_font_montserrat_14;
    p.tile_w = 105;
    p.tile_h = 48;
    p.tile_x_offset = -6;
    p.tile_pad_x = 4;
    p.tile_pad_y = 2;
    p.co2_tile_y = -48;
    p.pm_tile_y = 8;
    p.status_icon_spacing = 20;
    p.status_y = 2;
    p.bottom_row_y = -16;
    p.temp_rh_y = -62;
    p.voc_nox_y = 0;
    p.gps_y = 0;
    p.pressure_y = 32;
    p.interval_y = 16;
  } else {
    // Large display: 144x296
    p.large_font = &lv_font_montserrat_32;
    p.medium_font = &lv_font_montserrat_16;
    p.small_font = &lv_font_montserrat_14;
    p.tiny_font = &lv_font_montserrat_10;
    p.icon_font = &lv_font_montserrat_16;
    p.tile_w = 127;
    p.tile_h = 60;
    p.tile_x_offset = -8;
    p.tile_pad_x = 6;
    p.tile_pad_y = 4;
    p.co2_tile_y = -60;
    p.pm_tile_y = 10;
    p.status_icon_spacing = 25;
    p.status_y = 2;
    p.bottom_row_y = -20;
    p.temp_rh_y = -80;
    p.voc_nox_y = 0;
    p.gps_y = 0;
    p.pressure_y = 40;
    p.interval_y = 20;
  }

  return p;
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

  // Layout parameters for current display size
  LayoutParams layout = {};

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
  // Get layout parameters based on display size
  LayoutParams &L = state->layout;
  L = get_layout_params(w, h);

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

  // Status icons row - adaptive spacing based on display width
  int icon_spacing = L.status_icon_spacing;
  int icon_base_x = (h <= 250) ? -45 : -55;

  state->bt_logo = lv_label_create(state->root);
  lv_label_set_text(state->bt_logo, LV_SYMBOL_BLUETOOTH);
  lv_obj_set_style_text_color(state->bt_logo, lv_color_black(), 0);
  lv_obj_set_style_text_font(state->bt_logo, L.icon_font, 0);
  lv_obj_align(state->bt_logo, LV_ALIGN_TOP_MID, icon_base_x, L.status_y);

  state->wifi_logo = lv_label_create(state->root);
  lv_label_set_text(state->wifi_logo, LV_SYMBOL_WIFI);
  lv_obj_set_style_text_color(state->wifi_logo, lv_color_black(), 0);
  lv_obj_set_style_text_font(state->wifi_logo, L.icon_font, 0);
  lv_obj_align(state->wifi_logo, LV_ALIGN_TOP_MID, icon_base_x + icon_spacing,
               L.status_y);

  state->gps_logo = lv_label_create(state->root);
  lv_label_set_text(state->gps_logo, LV_SYMBOL_GPS);
  lv_obj_set_style_text_color(state->gps_logo, lv_color_black(), 0);
  lv_obj_set_style_text_font(state->gps_logo, L.icon_font, 0);
  lv_obj_align(state->gps_logo, LV_ALIGN_TOP_MID,
               icon_base_x + icon_spacing * 2, L.status_y);

  state->battery_logo = lv_label_create(state->root);
  lv_label_set_text(state->battery_logo, LV_SYMBOL_BATTERY_EMPTY);
  lv_obj_set_style_text_color(state->battery_logo, lv_color_black(), 0);
  lv_obj_set_style_text_font(state->battery_logo, L.icon_font, 0);
  lv_obj_align(state->battery_logo, LV_ALIGN_TOP_MID,
               icon_base_x + icon_spacing * 3, L.status_y);

  state->battery_percent_label = lv_label_create(state->root);
  lv_label_set_text(state->battery_percent_label, "100%");
  lv_obj_set_style_text_color(state->battery_percent_label, lv_color_black(),
                              0);
  lv_obj_set_style_text_font(state->battery_percent_label, L.tiny_font, 0);
  lv_obj_align_to(state->battery_percent_label, state->battery_logo,
                  LV_ALIGN_OUT_RIGHT_MID, 2, 0);

  // Second row: Alert, REC, Time, Interval
  state->alert_label = lv_label_create(state->root);
  lv_label_set_text(state->alert_label, "");
  lv_obj_set_style_text_color(state->alert_label, lv_color_black(), 0);
  lv_obj_set_style_text_font(state->alert_label, L.icon_font, 0);
  lv_obj_align(state->alert_label, LV_ALIGN_BOTTOM_LEFT, 12, L.bottom_row_y);

  state->rec_label = lv_label_create(state->root);
  lv_label_set_text(state->rec_label, "");
  lv_obj_set_style_text_color(state->rec_label, lv_color_black(), 0);
  lv_obj_set_style_text_font(state->rec_label, L.tiny_font, 0);
  lv_obj_align(state->rec_label, LV_ALIGN_BOTTOM_LEFT, 30, L.bottom_row_y);

  state->time_label = lv_label_create(state->root);
  lv_label_set_text(state->time_label, "--:--");
  lv_obj_set_style_text_color(state->time_label, lv_color_black(), 0);
  lv_obj_set_style_text_font(state->time_label, L.icon_font, 0);
  lv_obj_align(state->time_label, LV_ALIGN_BOTTOM_RIGHT, -12, L.bottom_row_y);

  state->interval_label = lv_label_create(state->root);
  lv_label_set_text(state->interval_label, "10s");
  lv_obj_set_style_text_color(state->interval_label, lv_color_black(), 0);
  lv_obj_set_style_text_font(state->interval_label, L.icon_font, 0);
  lv_obj_align(state->interval_label, LV_ALIGN_TOP_RIGHT, -12, L.interval_y);

  // Main large values (CO2 / PM2.5 tiles)
  state->co2_tile = lv_obj_create(state->root);
  lv_obj_remove_style_all(state->co2_tile);
  lv_obj_set_size(state->co2_tile, L.tile_w, L.tile_h);
  lv_obj_align(state->co2_tile, LV_ALIGN_RIGHT_MID, L.tile_x_offset,
               L.co2_tile_y);
  lv_obj_set_style_radius(state->co2_tile, 4, 0);
  lv_obj_set_style_pad_all(state->co2_tile, 0, 0);
  lv_obj_clear_flag(state->co2_tile, LV_OBJ_FLAG_SCROLLABLE);

  state->co2_label = lv_label_create(state->co2_tile);
  lv_label_set_text(state->co2_label, "-");
  lv_obj_set_style_text_font(state->co2_label, L.large_font, 0);
  lv_obj_align(state->co2_label, LV_ALIGN_BOTTOM_RIGHT, -L.tile_pad_x,
               -L.tile_pad_y);

  state->co2_unit_label = lv_label_create(state->co2_tile);
  lv_label_set_text(state->co2_unit_label, "CO2 (ppm)");
  lv_obj_set_style_text_font(state->co2_unit_label, L.medium_font, 0);
  lv_obj_align_to(state->co2_unit_label, state->co2_label,
                  LV_ALIGN_OUT_TOP_RIGHT, 0, -2);

  state->pm25_tile = lv_obj_create(state->root);
  lv_obj_remove_style_all(state->pm25_tile);
  lv_obj_set_size(state->pm25_tile, L.tile_w, L.tile_h);
  lv_obj_align(state->pm25_tile, LV_ALIGN_RIGHT_MID, L.tile_x_offset,
               L.pm_tile_y);
  lv_obj_set_style_radius(state->pm25_tile, 4, 0);
  lv_obj_set_style_pad_all(state->pm25_tile, 0, 0);
  lv_obj_clear_flag(state->pm25_tile, LV_OBJ_FLAG_SCROLLABLE);

  state->pm25_label = lv_label_create(state->pm25_tile);
  lv_label_set_text(state->pm25_label, "-");
  lv_obj_set_style_text_font(state->pm25_label, L.large_font, 0);
  lv_obj_align(state->pm25_label, LV_ALIGN_BOTTOM_RIGHT, -L.tile_pad_x,
               -L.tile_pad_y);

  state->pm25_unit_label = lv_label_create(state->pm25_tile);
  lv_label_set_text(state->pm25_unit_label, (h <= 250) ? "PM2.5" : "PM2.5 (ug/m3)");
  lv_obj_set_style_text_font(state->pm25_unit_label, L.tiny_font, 0);
  lv_obj_align_to(state->pm25_unit_label, state->pm25_label,
                  LV_ALIGN_OUT_TOP_RIGHT, 0, -2);

  // Bottom: Temp/RH on same line; VOC/NOx on right; pressure under interval
  int temp_x = (h <= 250) ? -75 : -90;
  int temp_rh_spacing = (h <= 250) ? 18 : 25;

  state->temp_label = lv_label_create(state->root);
  lv_label_set_text(state->temp_label, "--C");
  lv_obj_set_style_text_font(state->temp_label, L.small_font, 0);
  lv_obj_set_style_text_color(state->temp_label, lv_color_black(), 0);
  lv_obj_align(state->temp_label, LV_ALIGN_BOTTOM_RIGHT, temp_x, L.temp_rh_y);

  state->rh_label = lv_label_create(state->root);
  lv_label_set_text(state->rh_label, "RH:--%");
  lv_obj_set_style_text_font(state->rh_label, L.small_font, 0);
  lv_obj_set_style_text_color(state->rh_label, lv_color_black(), 0);
  lv_obj_align_to(state->rh_label, state->temp_label, LV_ALIGN_OUT_RIGHT_MID,
                  temp_rh_spacing, 0);

  state->voc_label = lv_label_create(state->root);
  lv_label_set_text(state->voc_label, "VOC:-");
  lv_obj_set_style_text_font(state->voc_label, L.small_font, 0);
  lv_obj_set_style_text_color(state->voc_label, lv_color_black(), 0);
  lv_obj_align_to(state->voc_label, state->temp_label,
                  LV_ALIGN_OUT_BOTTOM_RIGHT, -5, 0);

  state->nox_label = lv_label_create(state->root);
  lv_label_set_text(state->nox_label, "NOx:-");
  lv_obj_set_style_text_font(state->nox_label, L.small_font, 0);
  lv_obj_set_style_text_color(state->nox_label, lv_color_black(), 0);
  lv_obj_align_to(state->nox_label, state->rh_label, LV_ALIGN_OUT_BOTTOM_RIGHT,
                  -8, 0);

  // GPS coordinates below VOC and NOx
  state->lat_label = lv_label_create(state->root);
  lv_label_set_text(state->lat_label, "--,--");
  lv_obj_set_style_text_font(state->lat_label, L.tiny_font, 0);
  lv_obj_set_style_text_color(state->lat_label, lv_color_black(), 0);
  lv_obj_align_to(state->lat_label, state->voc_label, LV_ALIGN_OUT_BOTTOM_LEFT,
                  0, 0);

  state->lon_label = lv_label_create(state->root);
  lv_label_set_text(state->lon_label, "--,--");
  lv_obj_set_style_text_font(state->lon_label, L.tiny_font, 0);
  lv_obj_set_style_text_color(state->lon_label, lv_color_black(), 0);
  lv_obj_align_to(state->lon_label, state->nox_label, LV_ALIGN_OUT_BOTTOM_LEFT,
                  0, 0);

  state->pressure_label = lv_label_create(state->root);
  lv_label_set_text(state->pressure_label, "----hPa");
  lv_obj_set_style_text_font(state->pressure_label, L.tiny_font, 0);
  lv_obj_set_style_text_color(state->pressure_label, lv_color_black(), 0);
  lv_obj_align(state->pressure_label, LV_ALIGN_TOP_RIGHT, -12, L.pressure_y);

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

  // Use layout-appropriate fonts
  const LayoutParams &L = state->layout;
  const lv_font_t *charge_font = L.icon_font ? L.icon_font : &lv_font_montserrat_14;
  const lv_font_t *normal_font = L.tiny_font ? L.tiny_font : &lv_font_montserrat_10;

  if (charging) {
    if (!was_charging) {
      state->batt_anim_index = 0;
      state->last_batt_anim_ms = 0;
      state->batt_blink_state = true;
      state->last_batt_blink_ms = 0;
    }
    lv_obj_set_style_text_font(state->battery_percent_label, charge_font, 0);
    lv_label_set_text(state->battery_logo,
                      kBatteryChargeAnimSymbols[state->batt_anim_index]);
  } else {
    lv_obj_set_style_text_font(state->battery_percent_label, normal_font, 0);
    lv_label_set_text(state->battery_logo, battery_symbol_for_percent(percent));
    state->batt_blink_state = true;
    state->last_batt_blink_ms = 0;
    state->last_batt_anim_ms = 0;
  }

  lv_obj_align_to(state->battery_percent_label, state->battery_logo,
                  LV_ALIGN_OUT_RIGHT_MID, 2, 0);
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
