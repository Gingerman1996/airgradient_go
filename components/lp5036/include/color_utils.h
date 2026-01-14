/**
 * @file color_utils.h
 * @brief Color conversion utilities for LED control
 * 
 * Provides HSV to RGB conversion and color manipulation helpers
 * for use with LP5036 RGB LED driver.
 */

#pragma once

#include <cstdint>
#include <algorithm>
#include <cmath>

namespace color {

/**
 * @brief RGB color structure (0-255 per channel)
 */
struct RGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;

  constexpr RGB() : r(0), g(0), b(0) {}
  constexpr RGB(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
};

/**
 * @brief HSV color structure
 * - H: Hue (0-360 degrees)
 * - S: Saturation (0.0-1.0)
 * - V: Value/Brightness (0.0-1.0)
 */
struct HSV {
  float h; // 0-360
  float s; // 0.0-1.0
  float v; // 0.0-1.0

  HSV() : h(0), s(0), v(0) {}
  HSV(float hue, float saturation, float value) 
    : h(hue), s(saturation), v(value) {}
};

/**
 * @brief Convert HSV color to RGB
 * 
 * HSV is often more intuitive for color selection:
 * - Hue: Color wheel position (0=red, 120=green, 240=blue)
 * - Saturation: Color intensity (0=white, 1=pure color)
 * - Value: Brightness (0=black, 1=full brightness)
 * 
 * @param hsv Input HSV color
 * @return RGB color (0-255 per channel)
 * 
 * @example
 *   HSV red_hsv(0, 1.0, 1.0);        // Pure red
 *   RGB red_rgb = hsv_to_rgb(red_hsv);
 *   
 *   HSV blue_hsv(240, 1.0, 0.5);     // Half-bright blue
 *   RGB blue_rgb = hsv_to_rgb(blue_hsv);
 */
inline RGB hsv_to_rgb(const HSV& hsv) {
  float h = hsv.h;
  float s = hsv.s;
  float v = hsv.v;

  // Clamp values
  h = fmodf(h, 360.0f);
  s = std::max(0.0f, std::min(1.0f, s));
  v = std::max(0.0f, std::min(1.0f, v));

  float c = v * s;  // Chroma
  float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
  float m = v - c;

  float r_prime, g_prime, b_prime;

  if (h < 60) {
    r_prime = c; g_prime = x; b_prime = 0;
  } else if (h < 120) {
    r_prime = x; g_prime = c; b_prime = 0;
  } else if (h < 180) {
    r_prime = 0; g_prime = c; b_prime = x;
  } else if (h < 240) {
    r_prime = 0; g_prime = x; b_prime = c;
  } else if (h < 300) {
    r_prime = x; g_prime = 0; b_prime = c;
  } else {
    r_prime = c; g_prime = 0; b_prime = x;
  }

  return RGB(
    static_cast<uint8_t>((r_prime + m) * 255.0f),
    static_cast<uint8_t>((g_prime + m) * 255.0f),
    static_cast<uint8_t>((b_prime + m) * 255.0f)
  );
}

/**
 * @brief Convert RGB to HSV
 * 
 * @param rgb Input RGB color
 * @return HSV color
 */
inline HSV rgb_to_hsv(const RGB& rgb) {
  float r = rgb.r / 255.0f;
  float g = rgb.g / 255.0f;
  float b = rgb.b / 255.0f;

  float max_val = std::max({r, g, b});
  float min_val = std::min({r, g, b});
  float delta = max_val - min_val;

  HSV hsv;
  hsv.v = max_val;

  if (max_val == 0) {
    hsv.s = 0;
    hsv.h = 0;
    return hsv;
  }

  hsv.s = delta / max_val;

  if (delta == 0) {
    hsv.h = 0;
  } else if (max_val == r) {
    hsv.h = 60.0f * fmodf((g - b) / delta, 6.0f);
  } else if (max_val == g) {
    hsv.h = 60.0f * ((b - r) / delta + 2.0f);
  } else {
    hsv.h = 60.0f * ((r - g) / delta + 4.0f);
  }

  if (hsv.h < 0) {
    hsv.h += 360.0f;
  }

  return hsv;
}

/**
 * @brief Blend two RGB colors
 * 
 * @param color1 First color
 * @param color2 Second color
 * @param ratio Blend ratio (0.0 = color1, 1.0 = color2)
 * @return Blended RGB color
 */
inline RGB blend(const RGB& color1, const RGB& color2, float ratio) {
  ratio = std::max(0.0f, std::min(1.0f, ratio));
  return RGB(
    static_cast<uint8_t>(color1.r * (1.0f - ratio) + color2.r * ratio),
    static_cast<uint8_t>(color1.g * (1.0f - ratio) + color2.g * ratio),
    static_cast<uint8_t>(color1.b * (1.0f - ratio) + color2.b * ratio)
  );
}

/**
 * @brief Apply gamma correction to RGB value
 * 
 * Human perception of brightness is non-linear. Gamma correction
 * makes LED brightness changes appear more natural.
 * 
 * @param value Input value (0-255)
 * @param gamma Gamma value (typically 2.2 for LEDs)
 * @return Gamma-corrected value (0-255)
 */
inline uint8_t gamma_correct(uint8_t value, float gamma = 2.2f) {
  float normalized = value / 255.0f;
  float corrected = powf(normalized, gamma);
  return static_cast<uint8_t>(corrected * 255.0f);
}

/**
 * @brief Apply gamma correction to RGB color
 * 
 * @param rgb Input RGB color
 * @param gamma Gamma value (default: 2.2)
 * @return Gamma-corrected RGB color
 */
inline RGB gamma_correct(const RGB& rgb, float gamma = 2.2f) {
  return RGB(
    gamma_correct(rgb.r, gamma),
    gamma_correct(rgb.g, gamma),
    gamma_correct(rgb.b, gamma)
  );
}

/**
 * @brief Pre-defined colors
 */
namespace Colors {
  const RGB BLACK(0, 0, 0);
  const RGB WHITE(255, 255, 255);
  const RGB RED(255, 0, 0);
  const RGB GREEN(0, 255, 0);
  const RGB BLUE(0, 0, 255);
  const RGB YELLOW(255, 255, 0);
  const RGB CYAN(0, 255, 255);
  const RGB MAGENTA(255, 0, 255);
  const RGB ORANGE(255, 165, 0);
  const RGB PURPLE(128, 0, 128);
  const RGB PINK(255, 192, 203);
}

} // namespace color
