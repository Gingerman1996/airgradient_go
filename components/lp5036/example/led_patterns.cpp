/**
 * @file led_patterns.cpp
 * @brief Example LED patterns and effects using LP5036
 * 
 * Demonstrates various LED control patterns:
 * - HSV color control
 * - Rainbow animations
 * - Breathing effects
 * - Air quality indicator
 * - Custom animations
 */

#include "lp5036.h"
#include "color_utils.h"
#include "led_effects.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <cmath>

static const char *TAG = "LED_PATTERNS";

// Example 1: Simple HSV rainbow cycle
void example_hsv_rainbow(drivers::LP5036& led) {
  ESP_LOGI(TAG, "Example 1: HSV Rainbow Cycle");
  
  for (int i = 0; i < 12; i++) {
    // Spread colors evenly across 12 LEDs
    float hue = (i * 30.0f); // 360/12 = 30 degrees per LED
    color::HSV hsv(hue, 1.0f, 1.0f); // Full saturation and brightness
    
    color::RGB rgb = color::hsv_to_rgb(hsv);
    led.set_led_brightness(i, 255);
    led.set_led_color(i, rgb.b, rgb.g, rgb.r);
    
    ESP_LOGI(TAG, "LED %d: H=%.0f -> R=%d G=%d B=%d", 
             i, hue, rgb.r, rgb.g, rgb.b);
  }
}

// Example 2: Animated rainbow wave
void example_rainbow_wave(drivers::LP5036& led, uint32_t time_ms, uint32_t period_ms = 3000) {
  for (int i = 0; i < 12; i++) {
    // Calculate phase offset for wave effect
    float phase_offset = (i / 12.0f) * 360.0f;
    float hue = fmodf((time_ms / (float)period_ms) * 360.0f + phase_offset, 360.0f);
    
    color::HSV hsv(hue, 1.0f, 1.0f);
    color::RGB rgb = color::hsv_to_rgb(hsv);
    
    led.set_led_brightness(i, 255);
    led.set_led_color(i, rgb.b, rgb.g, rgb.r);
  }
}

// Example 3: Breathing effect with color
void example_breathing(drivers::LP5036& led, uint32_t time_ms, 
                       const color::RGB& base_color, uint32_t period_ms = 2000) {
  float brightness = led_effects::breathing_effect(time_ms, period_ms);
  uint8_t brightness_8bit = static_cast<uint8_t>(brightness * 255);
  
  for (int i = 0; i < 12; i++) {
    led.set_led_brightness(i, brightness_8bit);
    led.set_led_color(i, base_color.b, base_color.g, base_color.r);
  }
  
  ESP_LOGD(TAG, "Breathing: brightness=%.2f (%d)", brightness, brightness_8bit);
}

// Example 4: Air quality indicator (ring of 12 LEDs)
void example_aqi_ring(drivers::LP5036& led, uint16_t pm25_ugm3) {
  // Convert PM2.5 to AQI (simplified EPA formula)
  uint16_t aqi;
  if (pm25_ugm3 <= 12) {
    aqi = (pm25_ugm3 / 12.0f) * 50;
  } else if (pm25_ugm3 <= 35) {
    aqi = 50 + ((pm25_ugm3 - 12) / 23.0f) * 50;
  } else if (pm25_ugm3 <= 55) {
    aqi = 100 + ((pm25_ugm3 - 35) / 20.0f) * 50;
  } else if (pm25_ugm3 <= 150) {
    aqi = 150 + ((pm25_ugm3 - 55) / 95.0f) * 50;
  } else if (pm25_ugm3 <= 250) {
    aqi = 200 + ((pm25_ugm3 - 150) / 100.0f) * 100;
  } else {
    aqi = 300 + ((pm25_ugm3 - 250) / 250.0f) * 200;
  }
  
  color::RGB aqi_color = led_effects::aqi_to_color(aqi);
  
  // Light up all LEDs with AQI color
  for (int i = 0; i < 12; i++) {
    led.set_led_brightness(i, 255);
    led.set_led_color(i, aqi_color.b, aqi_color.g, aqi_color.r);
  }
  
  ESP_LOGI(TAG, "PM2.5=%.1f -> AQI=%d -> RGB(%d,%d,%d)",
           pm25_ugm3, aqi, aqi_color.r, aqi_color.g, aqi_color.b);
}

// Example 5: Progress bar (fill LEDs based on percentage)
void example_progress_bar(drivers::LP5036& led, float progress, 
                          const color::RGB& color = color::Colors::BLUE) {
  // Clamp progress to 0.0-1.0
  progress = std::max(0.0f, std::min(1.0f, progress));
  
  int lit_leds = static_cast<int>(progress * 12);
  
  for (int i = 0; i < 12; i++) {
    if (i < lit_leds) {
      led.set_led_brightness(i, 255);
      led.set_led_color(i, color.b, color.g, color.r);
    } else if (i == lit_leds && progress * 12 - lit_leds > 0) {
      // Partial brightness for fractional progress
      uint8_t partial = static_cast<uint8_t>((progress * 12 - lit_leds) * 255);
      led.set_led_brightness(i, partial);
      led.set_led_color(i, color.b, color.g, color.r);
    } else {
      led.set_led_brightness(i, 0);
    }
  }
  
  ESP_LOGI(TAG, "Progress: %.1f%% (%d LEDs)", progress * 100, lit_leds);
}

// Example 6: Color temperature sweep (warm to cool)
void example_color_temperature(drivers::LP5036& led, uint32_t time_ms) {
  // Sweep from 2000K (warm) to 8000K (cool) over 10 seconds
  uint16_t kelvin = 2000 + (time_ms % 10000) * 6 / 10;
  color::RGB temp_color = led_effects::color_temperature(kelvin);
  
  for (int i = 0; i < 12; i++) {
    led.set_led_brightness(i, 255);
    led.set_led_color(i, temp_color.b, temp_color.g, temp_color.r);
  }
  
  ESP_LOGD(TAG, "Color temp: %dK -> RGB(%d,%d,%d)",
           kelvin, temp_color.r, temp_color.g, temp_color.b);
}

// Example 7: Gamma correction comparison
void example_gamma_correction(drivers::LP5036& led) {
  ESP_LOGI(TAG, "Gamma correction demo:");
  ESP_LOGI(TAG, "Left half: Linear, Right half: Gamma-corrected");
  
  for (int i = 0; i < 12; i++) {
    uint8_t linear_brightness = (i * 255) / 11;
    uint8_t gamma_brightness = color::gamma_correct(linear_brightness, 2.2f);
    
    if (i < 6) {
      // Left half: Linear
      led.set_led_brightness(i, linear_brightness);
      ESP_LOGI(TAG, "LED %d: Linear = %d", i, linear_brightness);
    } else {
      // Right half: Gamma-corrected
      led.set_led_brightness(i, gamma_brightness);
      ESP_LOGI(TAG, "LED %d: Gamma = %d (from %d)", i, gamma_brightness, linear_brightness);
    }
    
    led.set_led_color(i, 255, 255, 255); // White (BGR same as RGB)
  }
}

// Example 8: Scanning/Kitt effect
void example_scanner(drivers::LP5036& led, uint32_t time_ms, 
                     const color::RGB& color = color::Colors::RED) {
  int position = (time_ms / 100) % (12 * 2 - 2);
  if (position >= 12) {
    position = 12 * 2 - 2 - position; // Bounce back
  }
  
  for (int i = 0; i < 12; i++) {
    int distance = abs(i - position);
    uint8_t brightness = (distance == 0) ? 255 : 
                         (distance == 1) ? 128 : 
                         (distance == 2) ? 64 : 0;
    
    led.set_led_brightness(i, brightness);
    led.set_led_color(i, color.b, color.g, color.r);
  }
}

// Complete demo sequence
void run_led_demo(drivers::LP5036& led) {
  ESP_LOGI(TAG, "Starting LED pattern demo...");
  
  // Initialize LED driver
  if (led.init() != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize LP5036");
    return;
  }
  
  led.enable(true);
  led.set_log_scale(false); // Linear brightness
  led.set_global_off(false);
  
  // Demo sequence
  uint32_t start_time = esp_timer_get_time() / 1000;
  
  while (true) {
    uint32_t now = esp_timer_get_time() / 1000;
    uint32_t elapsed = now - start_time;
    uint32_t phase = (elapsed / 5000) % 8; // 5 seconds per effect
    
    switch (phase) {
      case 0:
        example_hsv_rainbow(led);
        vTaskDelay(pdMS_TO_TICKS(5000));
        break;
        
      case 1:
        example_rainbow_wave(led, elapsed);
        vTaskDelay(pdMS_TO_TICKS(50));
        break;
        
      case 2:
        example_breathing(led, elapsed, color::Colors::BLUE);
        vTaskDelay(pdMS_TO_TICKS(50));
        break;
        
      case 3:
        example_aqi_ring(led, 50); // Good air quality
        vTaskDelay(pdMS_TO_TICKS(2500));
        example_aqi_ring(led, 150); // Unhealthy
        vTaskDelay(pdMS_TO_TICKS(2500));
        break;
        
      case 4: {
        float progress = ((elapsed % 5000) / 5000.0f);
        example_progress_bar(led, progress, color::Colors::GREEN);
        vTaskDelay(pdMS_TO_TICKS(50));
        break;
      }
        
      case 5:
        example_color_temperature(led, elapsed);
        vTaskDelay(pdMS_TO_TICKS(100));
        break;
        
      case 6:
        example_gamma_correction(led);
        vTaskDelay(pdMS_TO_TICKS(5000));
        break;
        
      case 7:
        example_scanner(led, elapsed, color::Colors::RED);
        vTaskDelay(pdMS_TO_TICKS(50));
        break;
    }
  }
}
