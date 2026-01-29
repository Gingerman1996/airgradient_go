#include "ui_display.h"
#include "lvgl.h"

extern "C" {
extern const lv_font_t lv_font_montserrat_32;
extern const lv_font_t lv_font_montserrat_24;
extern const lv_font_t lv_font_montserrat_16;
extern const lv_font_t lv_font_montserrat_14;
extern const lv_font_t lv_font_montserrat_12;
extern const lv_font_t lv_font_montserrat_10;
}

namespace {
constexpr int kPercentScale = 100;
constexpr int kPercentRoundHalfUp = 50;
constexpr float kChartPadFactor = 0.08f;
constexpr float kMinChartRange = 1e-3f;
constexpr float kFallbackChartRange = 1.0f;

// Generated from apps/website/content/images/logos/logo.svg at height=28, threshold=210.
struct LogoRun {
  unsigned char y;
  unsigned char x0;
  unsigned char x1;
};
static constexpr unsigned short kAirGradientLogoRunCount = 104;
static constexpr LogoRun kAirGradientLogoRuns[kAirGradientLogoRunCount] = {
    {2, 57, 61},  {3, 54, 64},  {3, 74, 75},  {4, 52, 57},  {4, 63, 65},
    {4, 74, 75},  {5, 51, 55},  {5, 65, 66},  {5, 75, 76},  {6, 50, 54},
    {6, 66, 67},  {6, 75, 76},  {7, 50, 53},  {7, 75, 77},  {8, 49, 52},
    {8, 75, 77},  {9, 49, 51},  {9, 60, 62},  {9, 76, 77},  {9, 81, 84},
    {10, 48, 51}, {10, 52, 53}, {10, 60, 61}, {10, 81, 83}, {10, 95, 96},
    {11, 48, 50}, {11, 52, 53}, {11, 55, 59}, {11, 60, 70}, {11, 71, 77},
    {11, 78, 84}, {11, 85, 89}, {11, 90, 97}, {12, 48, 50}, {12, 51, 53},
    {12, 56, 67}, {12, 69, 70}, {12, 71, 72}, {12, 76, 79}, {12, 81, 86},
    {12, 88, 91}, {12, 93, 94}, {12, 95, 96}, {13, 48, 50}, {13, 51, 53},
    {13, 55, 64}, {13, 65, 67}, {13, 69, 70}, {13, 71, 72}, {13, 74, 79},
    {13, 81, 91}, {13, 93, 94}, {13, 95, 96}, {14, 48, 50}, {14, 51, 53},
    {14, 55, 64}, {14, 66, 67}, {14, 69, 70}, {14, 71, 72}, {14, 73, 75},
    {14, 76, 79}, {14, 81, 86}, {14, 90, 91}, {14, 93, 94}, {14, 95, 96},
    {15, 48, 50}, {15, 52, 53}, {15, 55, 64}, {15, 66, 70}, {15, 71, 72},
    {15, 74, 77}, {15, 79, 84}, {15, 86, 89}, {15, 90, 91}, {15, 93, 94},
    {15, 95, 97}, {16, 48, 50}, {16, 52, 54}, {16, 69, 70}, {16, 74, 76},
    {17, 48, 50}, {17, 52, 54}, {17, 66, 69}, {17, 73, 76}, {18, 48, 51},
    {18, 53, 55}, {18, 72, 75}, {19, 49, 51}, {19, 53, 56}, {19, 71, 75},
    {20, 49, 52}, {20, 54, 57}, {20, 70, 74}, {21, 50, 52}, {21, 55, 59},
    {21, 67, 73}, {22, 51, 53}, {22, 56, 71}, {23, 52, 55}, {23, 58, 69},
    {24, 53, 56}, {24, 61, 67}, {25, 55, 59}, {26, 58, 61},
};

struct GoSimBase {
  static constexpr int kW = 144;
  static constexpr int kH = 296;

  // Toolbar
  static constexpr int kTimeX = 4;
  static constexpr int kTimeBaselineY = 16;
  static constexpr int kToolbarDividerY = 22;

  // Recording indicator (shown only when recording)
  static constexpr int kRecCx = 48;
  static constexpr int kRecCy = 12;
  static constexpr int kRecOuterR = 3;
  static constexpr int kRecInnerR = 1;

  // Icons (positions already include SVG transforms)
  static constexpr int kBleX = 55;
  static constexpr int kBleBaselineY = 15;

  // WiFi icon (approximate polyline points in simulator coordinates)
  static constexpr int kWifiOuterX1 = 81;
  static constexpr int kWifiOuterY1 = 10;
  static constexpr int kWifiOuterX2 = 87;
  static constexpr int kWifiOuterY2 = 7;
  static constexpr int kWifiOuterX3 = 93;
  static constexpr int kWifiOuterY3 = 10;

  static constexpr int kWifiInnerX1 = 83;
  static constexpr int kWifiInnerY1 = 12;
  static constexpr int kWifiInnerX2 = 87;
  static constexpr int kWifiInnerY2 = 10;
  static constexpr int kWifiInnerX3 = 91;
  static constexpr int kWifiInnerY3 = 12;

  static constexpr int kWifiDotCx = 87;
  static constexpr int kWifiDotCy = 15;

  // GPS icon (triangle outline points)
  static constexpr int kGpsX1 = 97;
  static constexpr int kGpsY1 = 7;
  static constexpr int kGpsX2 = 111;
  static constexpr int kGpsY2 = 12;
  static constexpr int kGpsX3 = 102;
  static constexpr int kGpsY3 = 18;

  // Battery icon (outline + nub + fill). Coords approximate the simulator.
  static constexpr int kBattOutlineX = 116;
  static constexpr int kBattOutlineY = 6;
  static constexpr int kBattOutlineW = 23;
  static constexpr int kBattOutlineH = 12;

  static constexpr int kBattNubX = 139;
  static constexpr int kBattNubY = 9;
  static constexpr int kBattNubW = 2;
  static constexpr int kBattNubH = 5;

  static constexpr int kBattFillX = 118;
  static constexpr int kBattFillY = 8;
  static constexpr int kBattFillWMax = 15;
  static constexpr int kBattFillH = 8;

  // Main tiles
  static constexpr int kPmBlockY = 32;
  static constexpr int kPmBlockH = 52;
  static constexpr int kPmLabelBaselineY = 50;
  static constexpr int kPmValueBaselineY = 78;

  static constexpr int kCo2BlockY = 88;
  static constexpr int kCo2BlockH = 56;
  static constexpr int kCo2LabelBaselineY = 108;
  static constexpr int kCo2ValueBaselineY = 136;

  static constexpr int kMainDividerY = 150;
  static constexpr int kMainDividerH = 2;

  // Bottom grid lines
  static constexpr int kGridVLineX = 72;
  static constexpr int kGridVLineY = 158;
  static constexpr int kGridVLineH = 102;
  static constexpr int kGridLineY_Row2 = 192;
  static constexpr int kGridLineY_Row3_None = 226;
  static constexpr int kGridLineY_Row3_Chart = 225;
  static constexpr int kGridLineH_Row3_None = 1;
  static constexpr int kGridLineH_Row3_Chart = 2;
  static constexpr int kGridLineY_Bottom = 260;

  // Cell content anchors
  static constexpr int kCellLeftTextX = 12;
  static constexpr int kCellRightTextX = 80;
  static constexpr int kRow1LabelBaselineY = 168;
  static constexpr int kRow1ValueBaselineY = 184;
  static constexpr int kRow2LabelBaselineY = 202;
  static constexpr int kRow2ValueBaselineY = 218;
  static constexpr int kRow3LabelBaselineY = 236;
  static constexpr int kRow3ValueBaselineY = 252;

  // Chart
  static constexpr int kChartX = 0;
  static constexpr int kChartY = 262;
  static constexpr int kChartW = 144;
  static constexpr int kChartH = 34;
  static constexpr int kChartPadX = 4;
  static constexpr int kChartPadY = 4;

  // Logo (shown when no chart)
  static constexpr int kLogoTopY = 268;
  static constexpr int kLogoH = 28;
};

static int scale_axis(int v, int actual, int base) {
  return (v * actual + base / 2) / base;
}
static int sx(int v, int w) { return scale_axis(v, w, GoSimBase::kW); }
static int sy(int v, int h) { return scale_axis(v, h, GoSimBase::kH); }

static int baseline_top_y(int baseline_y, const lv_font_t *font) {
  if (!font) return baseline_y;
  const int above = static_cast<int>(font->line_height) - static_cast<int>(font->base_line);
  return baseline_y - above;
}

static void place_left_baseline(lv_obj_t *label, int x, int baseline_y, const lv_font_t *font) {
  if (!label) return;
  lv_obj_set_pos(label, x, baseline_top_y(baseline_y, font));
}

static void place_center_baseline(lv_obj_t *label, int center_x, int baseline_y,
                                  const lv_font_t *font) {
  if (!label || !font) return;
  lv_point_t size = {};
  const char *txt = lv_label_get_text(label);
  lv_txt_get_size(&size, txt ? txt : "", font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
  lv_obj_set_pos(label, center_x - (size.x / 2), baseline_top_y(baseline_y, font));
}

static lv_obj_t *create_rect(lv_obj_t *parent, int x, int y, int w, int h, bool filled) {
  lv_obj_t *r = lv_obj_create(parent);
  lv_obj_remove_style_all(r);
  lv_obj_set_pos(r, x, y);
  lv_obj_set_size(r, w, h);
  lv_obj_clear_flag(r, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(r, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(r, filled ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
  return r;
}

static lv_obj_t *create_line_rect(lv_obj_t *parent, int x, int y, int w, int h) {
  return create_rect(parent, x, y, w, h, true);
}

static lv_obj_t *create_outline_box(lv_obj_t *parent, int x, int y, int w, int h, int border_w,
                                    int radius) {
  lv_obj_t *r = lv_obj_create(parent);
  lv_obj_remove_style_all(r);
  lv_obj_set_pos(r, x, y);
  lv_obj_set_size(r, w, h);
  lv_obj_clear_flag(r, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_opa(r, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(r, border_w, 0);
  lv_obj_set_style_border_color(r, lv_color_black(), 0);
  lv_obj_set_style_border_opa(r, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(r, radius, 0);
  return r;
}

static lv_obj_t *create_line(lv_obj_t *parent, const lv_point_precise_t *pts, uint16_t count, int width) {
  lv_obj_t *line = lv_line_create(parent);
  lv_obj_remove_style_all(line);
  lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);
  lv_line_set_points(line, pts, count);
  lv_obj_set_style_line_color(line, lv_color_black(), 0);
  lv_obj_set_style_line_width(line, width, 0);
  lv_obj_set_style_line_rounded(line, true, 0);
  return line;
}

static void set_visible(lv_obj_t *obj, bool on) {
  if (!obj) return;
  if (on) {
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
  }
}

static void set_label_style(lv_obj_t *label, const lv_font_t *font, lv_color_t color) {
  if (!label) return;
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, color, 0);
}

static void format_1dp(char *out, size_t out_sz, float v) {
  const int sign = (v < 0.0f) ? -1 : 1;
  const float av = v * sign;
  const int tenths = static_cast<int>(av * 10.0f + 0.5f);
  const int i_part = (tenths / 10) * sign;
  const int d_part = tenths % 10;
  lv_snprintf(out, out_sz, "%d.%d", i_part, d_part);
}

template <size_t N>
struct RingBuffer {
  float values[N] = {};
  size_t count = 0;
  size_t head = 0;

  void push(float v) {
    values[head] = v;
    head = (head + 1) % N;
    if (count < N) count += 1;
  }

  size_t size() const { return count; }

  float at_oldest(size_t idx) const {
    if (count == 0) return 0.0f;
    const size_t start = (head + N - count) % N;
    return values[(start + idx) % N];
  }

  void min_max(float &out_min, float &out_max) const {
    if (count == 0) {
      out_min = 0.0f;
      out_max = 0.0f;
      return;
    }
    float mn = at_oldest(0);
    float mx = at_oldest(0);
    for (size_t i = 1; i < count; i += 1) {
      const float v = at_oldest(i);
      if (v < mn) mn = v;
      if (v > mx) mx = v;
    }
    out_min = mn;
    out_max = mx;
  }
};
} // namespace

struct Display::DisplayState {
  lv_obj_t *root = nullptr;

  lv_obj_t *time_label = nullptr;
  lv_obj_t *toolbar_divider = nullptr;

  lv_obj_t *rec_outer = nullptr;
  lv_obj_t *rec_inner = nullptr;

  lv_obj_t *ble_label = nullptr;
  lv_obj_t *wifi_outer = nullptr;
  lv_obj_t *wifi_inner = nullptr;
  lv_obj_t *wifi_dot = nullptr;
  lv_obj_t *gps_tri = nullptr;
  lv_obj_t *battery_outline = nullptr;
  lv_obj_t *battery_nub = nullptr;
  lv_obj_t *battery_fill = nullptr;

  lv_obj_t *pm_block = nullptr;
  lv_obj_t *pm_label = nullptr;
  lv_obj_t *pm_value = nullptr;

  lv_obj_t *co2_block = nullptr;
  lv_obj_t *co2_label = nullptr;
  lv_obj_t *co2_value = nullptr;

  lv_obj_t *main_divider = nullptr;

  lv_obj_t *grid_vline = nullptr;
  lv_obj_t *grid_row2 = nullptr;
  lv_obj_t *grid_row3 = nullptr;
  lv_obj_t *grid_bottom = nullptr;

  lv_obj_t *temp_label = nullptr;
  lv_obj_t *humidity_label = nullptr;
  lv_obj_t *tvoc_label = nullptr;
  lv_obj_t *nox_label = nullptr;

  lv_obj_t *temp_value = nullptr;
  lv_obj_t *humidity_value = nullptr;
  lv_obj_t *tvoc_value = nullptr;
  lv_obj_t *nox_value = nullptr;

  lv_obj_t *pressure_label = nullptr;
  lv_obj_t *pressure_value = nullptr;
  lv_obj_t *altitude_label = nullptr;
  lv_obj_t *altitude_value = nullptr;

  lv_obj_t *min_label = nullptr;
  lv_obj_t *min_value = nullptr;
  lv_obj_t *max_label = nullptr;
  lv_obj_t *max_value = nullptr;

  lv_obj_t *chart_axis_y = nullptr;
  lv_obj_t *chart_axis_x = nullptr;
  lv_obj_t *chart_polyline = nullptr;
  lv_point_precise_t chart_points[30] = {};
  uint16_t chart_point_count = 0;

  lv_obj_t *logo_group = nullptr;

  uint16_t width = 0;
  uint16_t height = 0;
  const lv_font_t *time_font = nullptr;
  const lv_font_t *main_label_font = nullptr;
  const lv_font_t *main_value_font = nullptr;
  const lv_font_t *cell_label_font = nullptr;
  const lv_font_t *cell_value_font = nullptr;

  bool recording = false;
  Display::WiFiStatus wifi_status = Display::WiFiStatus::Off;
  Display::GPSStatus gps_status = Display::GPSStatus::Off;

  int battery_percent = 0;

  float latest_pm25 = 0.0f;
  bool latest_pm25_is_float = false;
  float latest_co2 = 0.0f;

  float latest_temp_c = 0.0f;
  bool latest_temp_is_float = false;
  float latest_rh = 0.0f;
  bool latest_rh_is_float = false;

  int latest_voc = 0;
  int latest_nox = 0;
  int latest_pressure = 0;

  Display::FocusTile focus_tile = Display::FocusTile::CO2;

  RingBuffer<30> pm25_hist;
  RingBuffer<30> co2_hist;
};

Display::Display() : state(new DisplayState()) {}

Display::~Display() { delete state; }

static void update_battery(Display::DisplayState *S) {
  if (!S || !S->battery_outline || !S->battery_fill || !S->battery_nub) return;

  const int w = S->width;
  const int h = S->height;

  const int outline_x = sx(GoSimBase::kBattOutlineX, w);
  const int outline_y = sy(GoSimBase::kBattOutlineY, h);
  const int outline_w = sx(GoSimBase::kBattOutlineW, w);
  const int outline_h = sy(GoSimBase::kBattOutlineH, h);

  const int nub_x = sx(GoSimBase::kBattNubX, w);
  const int nub_y = sy(GoSimBase::kBattNubY, h);
  const int nub_w = sx(GoSimBase::kBattNubW, w);
  const int nub_h = sy(GoSimBase::kBattNubH, h);

  const int fill_x = sx(GoSimBase::kBattFillX, w);
  const int fill_y = sy(GoSimBase::kBattFillY, h);
  const int fill_h = sy(GoSimBase::kBattFillH, h);
  const int fill_w_max = sx(GoSimBase::kBattFillWMax, w);

  const int pct = (S->battery_percent < 0)
                      ? 0
                      : (S->battery_percent > kPercentScale ? kPercentScale : S->battery_percent);
  const int fill_w = (fill_w_max * pct + kPercentRoundHalfUp) / kPercentScale;

  lv_obj_set_pos(S->battery_outline, outline_x, outline_y);
  lv_obj_set_size(S->battery_outline, outline_w, outline_h);

  lv_obj_set_pos(S->battery_nub, nub_x, nub_y);
  lv_obj_set_size(S->battery_nub, nub_w, nub_h);

  lv_obj_set_pos(S->battery_fill, fill_x, fill_y);
  lv_obj_set_size(S->battery_fill, fill_w, fill_h);
}

static void apply_status(Display::DisplayState *S) {
  if (!S) return;
  set_visible(S->rec_outer, S->recording);
  set_visible(S->rec_inner, S->recording);

  // WiFi: dot only when connected (simulator dims otherwise; 1-bit uses presence/absence).
  set_visible(S->wifi_dot, S->wifi_status == Display::WiFiStatus::Connected);

  // GPS: thicker stroke only when fix (simulator dims otherwise).
  if (S->gps_tri) {
    lv_obj_set_style_line_width(S->gps_tri, S->gps_status == Display::GPSStatus::Fix ? 2 : 1, 0);
  }

  update_battery(S);
}

static void set_block_inverted(lv_obj_t *block, lv_obj_t *label, lv_obj_t *value, bool inverted) {
  if (!block || !label || !value) return;
  lv_obj_set_style_bg_color(block, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(block, inverted ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
  lv_obj_set_style_text_color(label, inverted ? lv_color_white() : lv_color_black(), 0);
  lv_obj_set_style_text_color(value, inverted ? lv_color_white() : lv_color_black(), 0);
}

static void update_pm_value(Display::DisplayState *S) {
  if (!S || !S->pm_value) return;
  char buf[16];
  if (S->latest_pm25_is_float) {
    format_1dp(buf, sizeof(buf), S->latest_pm25);
  } else {
    lv_snprintf(buf, sizeof(buf), "%d", static_cast<int>(S->latest_pm25 + 0.5f));
  }
  lv_label_set_text(S->pm_value, buf);
}

static void update_co2_value(Display::DisplayState *S) {
  if (!S || !S->co2_value) return;
  char buf[16];
  lv_snprintf(buf, sizeof(buf), "%d", static_cast<int>(S->latest_co2 + 0.5f));
  lv_label_set_text(S->co2_value, buf);
}

static void update_temp_value(Display::DisplayState *S) {
  if (!S || !S->temp_value) return;
  char buf[24];
  if (S->latest_temp_is_float) {
    char val[16];
    format_1dp(val, sizeof(val), S->latest_temp_c);
    lv_snprintf(buf, sizeof(buf), "%s °C", val);
  } else {
    lv_snprintf(buf, sizeof(buf), "%d °C", static_cast<int>(S->latest_temp_c + 0.5f));
  }
  lv_label_set_text(S->temp_value, buf);
}

static void update_rh_value(Display::DisplayState *S) {
  if (!S || !S->humidity_value) return;
  char buf[24];
  if (S->latest_rh_is_float) {
    char val[16];
    format_1dp(val, sizeof(val), S->latest_rh);
    lv_snprintf(buf, sizeof(buf), "%s %%", val);
  } else {
    lv_snprintf(buf, sizeof(buf), "%d %%", static_cast<int>(S->latest_rh + 0.5f));
  }
  lv_label_set_text(S->humidity_value, buf);
}

static void update_voc_value(Display::DisplayState *S) {
  if (!S || !S->tvoc_value) return;
  char buf[16];
  lv_snprintf(buf, sizeof(buf), "%d", S->latest_voc);
  lv_label_set_text(S->tvoc_value, buf);
}

static void update_nox_value(Display::DisplayState *S) {
  if (!S || !S->nox_value) return;
  char buf[16];
  lv_snprintf(buf, sizeof(buf), "%d", S->latest_nox);
  lv_label_set_text(S->nox_value, buf);
}

static void update_pressure_value(Display::DisplayState *S) {
  if (!S || !S->pressure_value) return;
  char buf[24];
  if (S->latest_pressure > 0) {
    lv_snprintf(buf, sizeof(buf), "%d hPa", S->latest_pressure);
  } else {
    lv_snprintf(buf, sizeof(buf), "-- hPa");
  }
  lv_label_set_text(S->pressure_value, buf);
}

static void show_chart(Display::DisplayState *S, bool on) {
  if (!S) return;
  set_visible(S->chart_axis_x, on);
  set_visible(S->chart_axis_y, on);
  set_visible(S->chart_polyline, on);
  set_visible(S->min_label, on);
  set_visible(S->min_value, on);
  set_visible(S->max_label, on);
  set_visible(S->max_value, on);

  set_visible(S->pressure_label, !on);
  set_visible(S->pressure_value, !on);
  set_visible(S->altitude_label, !on);
  set_visible(S->altitude_value, !on);

  set_visible(S->logo_group, !on);

  if (S->grid_row3) {
    const int y = on ? sy(GoSimBase::kGridLineY_Row3_Chart, S->height)
                     : sy(GoSimBase::kGridLineY_Row3_None, S->height);
    const int hh = on ? sy(GoSimBase::kGridLineH_Row3_Chart, S->height)
                      : sy(GoSimBase::kGridLineH_Row3_None, S->height);
    lv_obj_set_pos(S->grid_row3, 0, y);
    lv_obj_set_size(S->grid_row3, S->width, hh);
  }
}

static void update_chart(Display::DisplayState *S, bool pm25) {
  if (!S || !S->chart_polyline || !S->min_value || !S->max_value) return;

  const auto &hist = pm25 ? S->pm25_hist : S->co2_hist;
  const float fallback = pm25 ? S->latest_pm25 : S->latest_co2;

  float mn = 0.0f;
  float mx = 0.0f;
  hist.min_max(mn, mx);
  if (hist.size() == 0) {
    mn = fallback;
    mx = fallback;
  }

  const float range = mx - mn;
  const float padded_range = (range < kMinChartRange) ? kFallbackChartRange : range;
  const float pad = padded_range * kChartPadFactor;
  const float y_min = mn - pad;
  const float y_max = mx + pad;

  const int w = S->width;
  const int h = S->height;

  const int chart_x = sx(GoSimBase::kChartX, w);
  const int chart_y = sy(GoSimBase::kChartY, h);
  const int chart_w = sx(GoSimBase::kChartW, w);
  const int chart_h = sy(GoSimBase::kChartH, h);
  const int pad_x = sx(GoSimBase::kChartPadX, w);
  const int pad_y = sy(GoSimBase::kChartPadY, h);

  const int inner_w = chart_w - pad_x * 2;
  const int inner_h = chart_h - pad_y * 2;

  constexpr uint16_t kCount = static_cast<uint16_t>(sizeof(S->chart_points) / sizeof(S->chart_points[0]));
  S->chart_point_count = kCount;

  for (uint16_t i = 0; i < kCount; i += 1) {
    const float t = (kCount <= 1) ? 0.0f : static_cast<float>(i) / static_cast<float>(kCount - 1);
    float v = fallback;
    if (hist.size() > 0 && i < hist.size()) {
      v = hist.at_oldest(i);
    }

    const float norm = (v - y_min) / (y_max - y_min);
    const float clamped = (norm < 0.0f) ? 0.0f : (norm > 1.0f ? 1.0f : norm);

    const int px = chart_x + pad_x + static_cast<int>(t * inner_w + 0.5f);
    const int py = chart_y + pad_y + static_cast<int>((1.0f - clamped) * inner_h + 0.5f);
    S->chart_points[i] = {static_cast<lv_value_precise_t>(px),
                          static_cast<lv_value_precise_t>(py)};
  }

  lv_line_set_points(S->chart_polyline, S->chart_points, S->chart_point_count);

  char min_buf[28];
  char max_buf[28];
  if (pm25) {
    char min_val[16];
    char max_val[16];
    format_1dp(min_val, sizeof(min_val), mn);
    format_1dp(max_val, sizeof(max_val), mx);
    lv_snprintf(min_buf, sizeof(min_buf), "%s µg/m³", min_val);
    lv_snprintf(max_buf, sizeof(max_buf), "%s µg/m³", max_val);
  } else {
    lv_snprintf(min_buf, sizeof(min_buf), "%d ppm", static_cast<int>(mn + 0.5f));
    lv_snprintf(max_buf, sizeof(max_buf), "%d ppm", static_cast<int>(mx + 0.5f));
  }

  lv_label_set_text(S->min_value, min_buf);
  lv_label_set_text(S->max_value, max_buf);

  place_left_baseline(S->min_value, sx(GoSimBase::kCellLeftTextX, w), sy(GoSimBase::kRow3ValueBaselineY, h),
                      S->cell_value_font);
  place_left_baseline(S->max_value, sx(GoSimBase::kCellRightTextX, w), sy(GoSimBase::kRow3ValueBaselineY, h),
                      S->cell_value_font);
}

static lv_obj_t *create_logo_group(lv_obj_t *parent, int display_w, int display_h) {
  const int top_y_abs = sy(GoSimBase::kLogoTopY, display_h);
  const int bottom_y_abs = sy(GoSimBase::kLogoTopY + GoSimBase::kLogoH, display_h);
  const int group_h = bottom_y_abs - top_y_abs;

  lv_obj_t *group = lv_obj_create(parent);
  lv_obj_remove_style_all(group);
  lv_obj_set_pos(group, 0, top_y_abs);
  lv_obj_set_size(group, display_w, group_h > 0 ? group_h : 1);
  lv_obj_clear_flag(group, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_opa(group, LV_OPA_TRANSP, 0);

  for (unsigned short i = 0; i < kAirGradientLogoRunCount; i += 1) {
    const LogoRun &run = kAirGradientLogoRuns[i];

    const int x0_abs = sx(run.x0, display_w);
    const int x1_abs = sx(run.x1, display_w);
    const int y0_abs = sy(GoSimBase::kLogoTopY + run.y, display_h);
    const int y1_abs = sy(GoSimBase::kLogoTopY + run.y + 1, display_h);

    const int x0 = x0_abs;
    const int y0 = y0_abs - top_y_abs;
    int w = x1_abs - x0_abs;
    int h = y1_abs - y0_abs;
    if (w <= 0) w = 1;
    if (h <= 0) h = 1;

    (void)create_rect(group, x0, y0, w, h, true);
  }

  return group;
}

bool Display::init(uint16_t w, uint16_t h) {
  state->width = w;
  state->height = h;

  if (h <= 250) {
    state->time_font = &lv_font_montserrat_12;
    state->main_label_font = &lv_font_montserrat_12;
    state->main_value_font = &lv_font_montserrat_24;
    state->cell_label_font = &lv_font_montserrat_10;
    state->cell_value_font = &lv_font_montserrat_12;
  } else {
    state->time_font = &lv_font_montserrat_14;
    state->main_label_font = &lv_font_montserrat_16;
    state->main_value_font = &lv_font_montserrat_32;
    state->cell_label_font = &lv_font_montserrat_10;
    state->cell_value_font = &lv_font_montserrat_14;
  }

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

  const int center_x = w / 2;

  state->time_label = lv_label_create(state->root);
  lv_label_set_text(state->time_label, "--:--");
  set_label_style(state->time_label, state->time_font, lv_color_black());
  place_left_baseline(state->time_label, sx(GoSimBase::kTimeX, w), sy(GoSimBase::kTimeBaselineY, h),
                      state->time_font);

  state->toolbar_divider =
      create_line_rect(state->root, 0, sy(GoSimBase::kToolbarDividerY, h), w, sy(1, h));

  // Recording indicator (hidden by default)
  {
    const int cx = sx(GoSimBase::kRecCx, w);
    const int cy = sy(GoSimBase::kRecCy, h);
    const int r_outer = sx(GoSimBase::kRecOuterR, w);
    const int r_inner = sx(GoSimBase::kRecInnerR, w);
    state->rec_outer = create_outline_box(state->root, cx - r_outer, cy - r_outer, r_outer * 2, r_outer * 2, 1, r_outer);
    state->rec_inner = create_rect(state->root, cx - r_inner, cy - r_inner, r_inner * 2, r_inner * 2, true);
    lv_obj_set_style_radius(state->rec_inner, r_inner, 0);
    set_visible(state->rec_outer, false);
    set_visible(state->rec_inner, false);
  }

  // BLE label
  state->ble_label = lv_label_create(state->root);
  lv_label_set_text(state->ble_label, "BLE");
  set_label_style(state->ble_label, state->cell_label_font, lv_color_black());
  place_left_baseline(state->ble_label, sx(GoSimBase::kBleX, w), sy(GoSimBase::kBleBaselineY, h), state->cell_label_font);

  // WiFi icon (poly-lines + dot), base coords approximate the SVG.
  {
    const lv_point_precise_t outer_pts[3] = {
        {static_cast<lv_value_precise_t>(sx(GoSimBase::kWifiOuterX1, w)),
         static_cast<lv_value_precise_t>(sy(GoSimBase::kWifiOuterY1, h))},
        {static_cast<lv_value_precise_t>(sx(GoSimBase::kWifiOuterX2, w)),
         static_cast<lv_value_precise_t>(sy(GoSimBase::kWifiOuterY2, h))},
        {static_cast<lv_value_precise_t>(sx(GoSimBase::kWifiOuterX3, w)),
         static_cast<lv_value_precise_t>(sy(GoSimBase::kWifiOuterY3, h))},
    };
    state->wifi_outer = create_line(state->root, outer_pts, 3, 2);

    const lv_point_precise_t inner_pts[3] = {
        {static_cast<lv_value_precise_t>(sx(GoSimBase::kWifiInnerX1, w)),
         static_cast<lv_value_precise_t>(sy(GoSimBase::kWifiInnerY1, h))},
        {static_cast<lv_value_precise_t>(sx(GoSimBase::kWifiInnerX2, w)),
         static_cast<lv_value_precise_t>(sy(GoSimBase::kWifiInnerY2, h))},
        {static_cast<lv_value_precise_t>(sx(GoSimBase::kWifiInnerX3, w)),
         static_cast<lv_value_precise_t>(sy(GoSimBase::kWifiInnerY3, h))},
    };
    state->wifi_inner = create_line(state->root, inner_pts, 3, 2);

    const int dot_r = sx(1, w);
    state->wifi_dot =
        create_rect(state->root, sx(GoSimBase::kWifiDotCx, w) - dot_r,
                    sy(GoSimBase::kWifiDotCy, h) - dot_r, dot_r * 2, dot_r * 2, true);
    lv_obj_set_style_radius(state->wifi_dot, dot_r, 0);
  }

  // GPS icon (triangle outline)
  {
    const lv_point_precise_t tri_pts[4] = {
        {static_cast<lv_value_precise_t>(sx(GoSimBase::kGpsX1, w)),
         static_cast<lv_value_precise_t>(sy(GoSimBase::kGpsY1, h))},
        {static_cast<lv_value_precise_t>(sx(GoSimBase::kGpsX2, w)),
         static_cast<lv_value_precise_t>(sy(GoSimBase::kGpsY2, h))},
        {static_cast<lv_value_precise_t>(sx(GoSimBase::kGpsX3, w)),
         static_cast<lv_value_precise_t>(sy(GoSimBase::kGpsY3, h))},
        {static_cast<lv_value_precise_t>(sx(GoSimBase::kGpsX1, w)),
         static_cast<lv_value_precise_t>(sy(GoSimBase::kGpsY1, h))},
    };
    state->gps_tri = create_line(state->root, tri_pts, 4, 1);
  }

  // Battery outline + nub + fill.
  {
    state->battery_outline =
        create_outline_box(state->root, sx(GoSimBase::kBattOutlineX, w),
                           sy(GoSimBase::kBattOutlineY, h), sx(GoSimBase::kBattOutlineW, w),
                           sy(GoSimBase::kBattOutlineH, h), 1, sx(2, w));
    state->battery_nub =
        create_rect(state->root, sx(GoSimBase::kBattNubX, w), sy(GoSimBase::kBattNubY, h),
                    sx(GoSimBase::kBattNubW, w), sy(GoSimBase::kBattNubH, h), true);
    lv_obj_set_style_radius(state->battery_nub, sx(1, w), 0);
    state->battery_fill =
        create_rect(state->root, sx(GoSimBase::kBattFillX, w), sy(GoSimBase::kBattFillY, h), sx(1, w),
                    sy(GoSimBase::kBattFillH, h), true);
    lv_obj_set_style_radius(state->battery_fill, sx(1, w), 0);
  }

  // PM block
  state->pm_block = create_rect(state->root, 0, sy(GoSimBase::kPmBlockY, h), w, sy(GoSimBase::kPmBlockH, h), false);
  state->pm_label = lv_label_create(state->root);
  lv_label_set_text(state->pm_label, "PM2.5 (µg/m³)");
  set_label_style(state->pm_label, state->main_label_font, lv_color_black());
  place_center_baseline(state->pm_label, center_x, sy(GoSimBase::kPmLabelBaselineY, h), state->main_label_font);

  state->pm_value = lv_label_create(state->root);
  lv_label_set_text(state->pm_value, "--");
  set_label_style(state->pm_value, state->main_value_font, lv_color_black());
  place_center_baseline(state->pm_value, center_x, sy(GoSimBase::kPmValueBaselineY, h), state->main_value_font);

  // CO2 block
  state->co2_block = create_rect(state->root, 0, sy(GoSimBase::kCo2BlockY, h), w, sy(GoSimBase::kCo2BlockH, h), false);
  state->co2_label = lv_label_create(state->root);
  lv_label_set_text(state->co2_label, "CO2 (ppm)");
  set_label_style(state->co2_label, state->main_label_font, lv_color_black());
  place_center_baseline(state->co2_label, center_x, sy(GoSimBase::kCo2LabelBaselineY, h), state->main_label_font);

  state->co2_value = lv_label_create(state->root);
  lv_label_set_text(state->co2_value, "--");
  set_label_style(state->co2_value, state->main_value_font, lv_color_black());
  place_center_baseline(state->co2_value, center_x, sy(GoSimBase::kCo2ValueBaselineY, h), state->main_value_font);

  state->main_divider = create_line_rect(state->root, 0, sy(GoSimBase::kMainDividerY, h), w, sy(GoSimBase::kMainDividerH, h));

  // Grid lines (pure BW, 1px lines are strong but consistent).
  state->grid_vline = create_line_rect(state->root, sx(GoSimBase::kGridVLineX, w), sy(GoSimBase::kGridVLineY, h), sx(1, w),
                                       sy(GoSimBase::kGridVLineH, h));
  state->grid_row2 = create_line_rect(state->root, 0, sy(GoSimBase::kGridLineY_Row2, h), w, sy(1, h));
  state->grid_row3 = create_line_rect(state->root, 0, sy(GoSimBase::kGridLineY_Row3_None, h), w, sy(1, h));
  state->grid_bottom = create_line_rect(state->root, 0, sy(GoSimBase::kGridLineY_Bottom, h), w, sy(1, h));

  const int left_x = sx(GoSimBase::kCellLeftTextX, w);
  const int right_x = sx(GoSimBase::kCellRightTextX, w);

  state->temp_label = lv_label_create(state->root);
  lv_label_set_text(state->temp_label, "Temp");
  set_label_style(state->temp_label, state->cell_label_font, lv_color_black());
  place_left_baseline(state->temp_label, left_x, sy(GoSimBase::kRow1LabelBaselineY, h), state->cell_label_font);

  state->humidity_label = lv_label_create(state->root);
  lv_label_set_text(state->humidity_label, "Humidity");
  set_label_style(state->humidity_label, state->cell_label_font, lv_color_black());
  place_left_baseline(state->humidity_label, right_x, sy(GoSimBase::kRow1LabelBaselineY, h), state->cell_label_font);

  state->tvoc_label = lv_label_create(state->root);
  lv_label_set_text(state->tvoc_label, "TVOC");
  set_label_style(state->tvoc_label, state->cell_label_font, lv_color_black());
  place_left_baseline(state->tvoc_label, left_x, sy(GoSimBase::kRow2LabelBaselineY, h), state->cell_label_font);

  state->nox_label = lv_label_create(state->root);
  lv_label_set_text(state->nox_label, "NOx");
  set_label_style(state->nox_label, state->cell_label_font, lv_color_black());
  place_left_baseline(state->nox_label, right_x, sy(GoSimBase::kRow2LabelBaselineY, h), state->cell_label_font);

  state->temp_value = lv_label_create(state->root);
  lv_label_set_text(state->temp_value, "-- °C");
  set_label_style(state->temp_value, state->cell_value_font, lv_color_black());
  place_left_baseline(state->temp_value, left_x, sy(GoSimBase::kRow1ValueBaselineY, h), state->cell_value_font);

  state->humidity_value = lv_label_create(state->root);
  lv_label_set_text(state->humidity_value, "-- %");
  set_label_style(state->humidity_value, state->cell_value_font, lv_color_black());
  place_left_baseline(state->humidity_value, right_x, sy(GoSimBase::kRow1ValueBaselineY, h), state->cell_value_font);

  state->tvoc_value = lv_label_create(state->root);
  lv_label_set_text(state->tvoc_value, "--");
  set_label_style(state->tvoc_value, state->cell_value_font, lv_color_black());
  place_left_baseline(state->tvoc_value, left_x, sy(GoSimBase::kRow2ValueBaselineY, h), state->cell_value_font);

  state->nox_value = lv_label_create(state->root);
  lv_label_set_text(state->nox_value, "--");
  set_label_style(state->nox_value, state->cell_value_font, lv_color_black());
  place_left_baseline(state->nox_value, right_x, sy(GoSimBase::kRow2ValueBaselineY, h), state->cell_value_font);

  state->pressure_label = lv_label_create(state->root);
  lv_label_set_text(state->pressure_label, "Pressure");
  set_label_style(state->pressure_label, state->cell_label_font, lv_color_black());
  place_left_baseline(state->pressure_label, left_x, sy(GoSimBase::kRow3LabelBaselineY, h), state->cell_label_font);

  state->pressure_value = lv_label_create(state->root);
  lv_label_set_text(state->pressure_value, "-- hPa");
  set_label_style(state->pressure_value, state->cell_value_font, lv_color_black());
  place_left_baseline(state->pressure_value, left_x, sy(GoSimBase::kRow3ValueBaselineY, h), state->cell_value_font);

  state->altitude_label = lv_label_create(state->root);
  lv_label_set_text(state->altitude_label, "Altitude");
  set_label_style(state->altitude_label, state->cell_label_font, lv_color_black());
  place_left_baseline(state->altitude_label, right_x, sy(GoSimBase::kRow3LabelBaselineY, h), state->cell_label_font);

  state->altitude_value = lv_label_create(state->root);
  lv_label_set_text(state->altitude_value, "-- m");
  set_label_style(state->altitude_value, state->cell_value_font, lv_color_black());
  place_left_baseline(state->altitude_value, right_x, sy(GoSimBase::kRow3ValueBaselineY, h), state->cell_value_font);

  state->min_label = lv_label_create(state->root);
  lv_label_set_text(state->min_label, "Min");
  set_label_style(state->min_label, state->cell_label_font, lv_color_black());
  place_left_baseline(state->min_label, left_x, sy(GoSimBase::kRow3LabelBaselineY, h), state->cell_label_font);

  state->min_value = lv_label_create(state->root);
  lv_label_set_text(state->min_value, "--");
  set_label_style(state->min_value, state->cell_value_font, lv_color_black());
  place_left_baseline(state->min_value, left_x, sy(GoSimBase::kRow3ValueBaselineY, h), state->cell_value_font);

  state->max_label = lv_label_create(state->root);
  lv_label_set_text(state->max_label, "Max");
  set_label_style(state->max_label, state->cell_label_font, lv_color_black());
  place_left_baseline(state->max_label, right_x, sy(GoSimBase::kRow3LabelBaselineY, h), state->cell_label_font);

  state->max_value = lv_label_create(state->root);
  lv_label_set_text(state->max_value, "--");
  set_label_style(state->max_value, state->cell_value_font, lv_color_black());
  place_left_baseline(state->max_value, right_x, sy(GoSimBase::kRow3ValueBaselineY, h), state->cell_value_font);

  // Chart axes (hidden in none mode)
  const int chart_x = sx(GoSimBase::kChartX, w);
  const int chart_y = sy(GoSimBase::kChartY, h);
  const int chart_w = sx(GoSimBase::kChartW, w);
  const int chart_h = sy(GoSimBase::kChartH, h);
  const int pad_x = sx(GoSimBase::kChartPadX, w);
  const int pad_y = sy(GoSimBase::kChartPadY, h);

  const lv_point_precise_t axis_y[2] = {
      {static_cast<lv_value_precise_t>(chart_x + pad_x),
       static_cast<lv_value_precise_t>(chart_y + pad_y)},
      {static_cast<lv_value_precise_t>(chart_x + pad_x),
       static_cast<lv_value_precise_t>(chart_y + chart_h - pad_y)},
  };
  state->chart_axis_y = create_line(state->root, axis_y, 2, 1);

  const lv_point_precise_t axis_x[2] = {
      {static_cast<lv_value_precise_t>(chart_x + pad_x),
       static_cast<lv_value_precise_t>(chart_y + chart_h - pad_y)},
      {static_cast<lv_value_precise_t>(chart_x + chart_w - pad_x),
       static_cast<lv_value_precise_t>(chart_y + chart_h - pad_y)},
  };
  state->chart_axis_x = create_line(state->root, axis_x, 2, 1);

  state->chart_polyline = lv_line_create(state->root);
  lv_obj_remove_style_all(state->chart_polyline);
  lv_obj_clear_flag(state->chart_polyline, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_line_color(state->chart_polyline, lv_color_black(), 0);
  lv_obj_set_style_line_width(state->chart_polyline, 2, 0);
  lv_obj_set_style_line_rounded(state->chart_polyline, true, 0);

  state->logo_group = create_logo_group(state->root, w, h);

  // Default: none state (no chart) unless caller sets focus to CO2/PM25.
  show_chart(state, false);
  setFocusTile(state->focus_tile);
  apply_status(state);

  return true;
}

void Display::update(uint64_t /*millis_now*/) {
  // No periodic animations in this simulator-matching 1-bit implementation.
}

void Display::setBLEStatus(BLEStatus /*s*/) {
  // Simulator always shows "BLE" text; no visual state in 1-bit.
}

void Display::setWiFiStatus(WiFiStatus s) {
  state->wifi_status = s;
  apply_status(state);
}

void Display::setGPSStatus(GPSStatus s) {
  state->gps_status = s;
  apply_status(state);
}

void Display::setBattery(int percent, bool /*charging*/) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  state->battery_percent = percent;
  apply_status(state);
}

void Display::setRecording(bool on) {
  state->recording = on;
  apply_status(state);
}

void Display::setAlert(bool /*on*/) {
  // Main screen (simulator) has no alert icon.
}

void Display::setIntervalSeconds(int /*seconds*/) {
  // Main screen (simulator) has no interval label.
}

void Display::setTimeText(const char *text) {
  if (!state->time_label) return;
  lv_label_set_text(state->time_label, text ? text : "--:--");
  place_left_baseline(state->time_label, sx(GoSimBase::kTimeX, state->width),
                      sy(GoSimBase::kTimeBaselineY, state->height), state->time_font);
}

void Display::setTimeHM(int hours, int minutes, bool valid) {
  if (!state->time_label) return;
  char time_buf[8];
  if (valid) {
    lv_snprintf(time_buf, sizeof(time_buf), "%02d:%02d", hours, minutes);
  } else {
    lv_snprintf(time_buf, sizeof(time_buf), "--:--");
  }
  lv_label_set_text(state->time_label, time_buf);
  place_left_baseline(state->time_label, sx(GoSimBase::kTimeX, state->width),
                      sy(GoSimBase::kTimeBaselineY, state->height), state->time_font);
}

void Display::setFocusTile(FocusTile tile) {
  state->focus_tile = tile;

  const bool pm_focused = (tile == FocusTile::PM25);
  const bool co2_focused = (tile == FocusTile::CO2);
  const bool chart = pm_focused || co2_focused;

  set_block_inverted(state->pm_block, state->pm_label, state->pm_value, pm_focused);
  set_block_inverted(state->co2_block, state->co2_label, state->co2_value, co2_focused);

  show_chart(state, chart);
  if (chart) {
    update_chart(state, pm_focused);
  }
}

void Display::setPM25(int v) {
  state->latest_pm25 = static_cast<float>(v);
  state->latest_pm25_is_float = false;
  state->pm25_hist.push(state->latest_pm25);
  update_pm_value(state);
  if (state->focus_tile == FocusTile::PM25) update_chart(state, true);
}

void Display::setCO2(int v) {
  state->latest_co2 = static_cast<float>(v);
  state->co2_hist.push(state->latest_co2);
  update_co2_value(state);
  if (state->focus_tile == FocusTile::CO2) update_chart(state, false);
}

void Display::setVOC(int v) {
  state->latest_voc = v;
  update_voc_value(state);
}

void Display::setNOx(int v) {
  state->latest_nox = v;
  update_nox_value(state);
}

void Display::setTempC(int v) {
  state->latest_temp_c = static_cast<float>(v);
  state->latest_temp_is_float = false;
  update_temp_value(state);
}

void Display::setRH(int v) {
  state->latest_rh = static_cast<float>(v);
  state->latest_rh_is_float = false;
  update_rh_value(state);
}

void Display::setPressure(int v) {
  state->latest_pressure = v;
  update_pressure_value(state);
}

void Display::setPM25f(float v) {
  state->latest_pm25 = v;
  state->latest_pm25_is_float = true;
  state->pm25_hist.push(v);
  update_pm_value(state);
  if (state->focus_tile == FocusTile::PM25) update_chart(state, true);
}

void Display::setTempCf(float v) {
  state->latest_temp_c = v;
  state->latest_temp_is_float = true;
  update_temp_value(state);
}

void Display::setRHf(float v) {
  state->latest_rh = v;
  state->latest_rh_is_float = true;
  update_rh_value(state);
}

void Display::setLatLon(float /*lat*/, float /*lon*/, bool /*valid*/) {
  // Main screen (simulator) does not show coordinates.
}
