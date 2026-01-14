/**
 * @file led_effects.h
 * @brief LED animation and effect helpers for LP5036
 * 
 * Provides common LED effects like breathing, rainbow, fade, etc.
 * for use with LP5036 RGB LED driver.
 */

#pragma once

#include "lp5036.h"
#include "color_utils.h"
#include <cmath>

namespace led_effects {

/**
 * @brief Breathing effect (smooth fade in/out)
 * 
 * @param time_ms Current time in milliseconds
 * @param period_ms Period of one breath cycle
 * @return Brightness value (0.0-1.0)
 * 
 * @example
 *   uint32_t now = esp_timer_get_time() / 1000;
 *   float brightness = breathing_effect(now, 2000); // 2-second cycle
 *   led.set_led_brightness(0, brightness * 255);
 */
inline float breathing_effect(uint32_t time_ms, uint32_t period_ms) {
  float phase = (time_ms % period_ms) / (float)period_ms;
  return (sinf(phase * 2.0f * M_PI) + 1.0f) * 0.5f;
}

/**
 * @brief Rainbow color wheel
 * 
 * @param position Position on rainbow (0-255)
 * @return RGB color
 * 
 * @example
 *   for (uint8_t i = 0; i < 12; i++) {
 *     auto color = rainbow_wheel(i * 21); // Spread across 12 LEDs
 *     led.set_led_color(i, color.r, color.g, color.b);
 *   }
 */
inline color::RGB rainbow_wheel(uint8_t position) {
  position = 255 - position;
  if (position < 85) {
    return color::RGB(255 - position * 3, 0, position * 3);
  } else if (position < 170) {
    position -= 85;
    return color::RGB(0, position * 3, 255 - position * 3);
  } else {
    position -= 170;
    return color::RGB(position * 3, 255 - position * 3, 0);
  }
}

/**
 * @brief Rainbow cycle effect
 * 
 * @param time_ms Current time in milliseconds
 * @param period_ms Period of one full rainbow cycle
 * @return RGB color for current time
 */
inline color::RGB rainbow_cycle(uint32_t time_ms, uint32_t period_ms) {
  uint8_t position = ((time_ms % period_ms) * 256) / period_ms;
  return rainbow_wheel(position);
}

/**
 * @brief Fade between two colors
 * 
 * @param color1 Start color
 * @param color2 End color
 * @param time_ms Current time
 * @param duration_ms Fade duration
 * @return Interpolated RGB color
 */
inline color::RGB fade_between(const color::RGB& color1, 
                                const color::RGB& color2,
                                uint32_t time_ms, 
                                uint32_t duration_ms) {
  float ratio = std::min(1.0f, time_ms / (float)duration_ms);
  return color::blend(color1, color2, ratio);
}

/**
 * @brief Pulse effect (quick on, slow fade out)
 * 
 * @param time_ms Time since pulse start
 * @param decay_ms Decay time
 * @return Brightness (0.0-1.0)
 */
inline float pulse_effect(uint32_t time_ms, uint32_t decay_ms) {
  if (time_ms >= decay_ms) return 0.0f;
  return 1.0f - (time_ms / (float)decay_ms);
}

/**
 * @brief Twinkle/sparkle effect
 * 
 * Random brightness variation using simple PRNG
 * 
 * @param seed Seed value (use LED index or time)
 * @return Brightness multiplier (0.5-1.0)
 */
inline float twinkle_effect(uint32_t seed) {
  // Simple LCG PRNG
  seed = seed * 1103515245u + 12345u;
  float random = (seed & 0x7FFF) / (float)0x7FFF;
  return 0.5f + random * 0.5f;
}

/**
 * @brief Color temperature (warm white to cool white)
 * 
 * @param kelvin Color temperature in Kelvin (2000-10000)
 * @return RGB color approximation
 * 
 * @example
 *   auto warm_white = color_temperature(2700); // Warm white
 *   auto cool_white = color_temperature(6500); // Cool white
 */
inline color::RGB color_temperature(uint16_t kelvin) {
  // Clamp to valid range
  kelvin = std::max((uint16_t)1000, std::min((uint16_t)40000, kelvin));
  
  float temp = kelvin / 100.0f;
  float r, g, b;

  // Red calculation
  if (temp <= 66) {
    r = 255;
  } else {
    r = temp - 60;
    r = 329.698727446f * powf(r, -0.1332047592f);
    r = std::max(0.0f, std::min(255.0f, r));
  }

  // Green calculation
  if (temp <= 66) {
    g = temp;
    g = 99.4708025861f * logf(g) - 161.1195681661f;
  } else {
    g = temp - 60;
    g = 288.1221695283f * powf(g, -0.0755148492f);
  }
  g = std::max(0.0f, std::min(255.0f, g));

  // Blue calculation
  if (temp >= 66) {
    b = 255;
  } else if (temp <= 19) {
    b = 0;
  } else {
    b = temp - 10;
    b = 138.5177312231f * logf(b) - 305.0447927307f;
    b = std::max(0.0f, std::min(255.0f, b));
  }

  return color::RGB((uint8_t)r, (uint8_t)g, (uint8_t)b);
}

/**
 * @brief Air quality color mapping
 * 
 * Maps air quality index (AQI) to color gradient
 * 
 * @param aqi Air Quality Index (0-500)
 * @return RGB color (green=good, yellow=moderate, red=unhealthy, purple=hazardous)
 */
inline color::RGB aqi_to_color(uint16_t aqi) {
  if (aqi <= 50) {
    // Good: Green
    return color::RGB(0, 255, 0);
  } else if (aqi <= 100) {
    // Moderate: Yellow
    float ratio = (aqi - 50) / 50.0f;
    return color::blend(color::RGB(0, 255, 0), color::RGB(255, 255, 0), ratio);
  } else if (aqi <= 150) {
    // Unhealthy for Sensitive: Orange
    float ratio = (aqi - 100) / 50.0f;
    return color::blend(color::RGB(255, 255, 0), color::RGB(255, 165, 0), ratio);
  } else if (aqi <= 200) {
    // Unhealthy: Red
    float ratio = (aqi - 150) / 50.0f;
    return color::blend(color::RGB(255, 165, 0), color::RGB(255, 0, 0), ratio);
  } else if (aqi <= 300) {
    // Very Unhealthy: Purple
    float ratio = (aqi - 200) / 100.0f;
    return color::blend(color::RGB(255, 0, 0), color::RGB(128, 0, 128), ratio);
  } else {
    // Hazardous: Maroon
    return color::RGB(128, 0, 0);
  }
}

} // namespace led_effects
