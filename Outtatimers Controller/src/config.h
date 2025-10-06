#pragma once

#define COLOR_ORDER GRB
#define LED_TYPE WS2812B

#include <stdint.h>

/**
 * @file config.h
 * @brief Configuration constants for the Controller LED Controller
 */

namespace ControllerConfig
{
  // Hardware Configuration
  namespace Hardware
  {
    constexpr int LED_PIN = 4;                  // GPIO4 (D2 on Lolin D1)
    constexpr int NUM_LEDS = 9;                 // Number of LEDs in the strip
    constexpr uint8_t DEFAULT_BRIGHTNESS = 255; // Maximum brightness

    // Button pin assignments
    constexpr int BUTTON1_PIN = 14; // GPIO14 (D5) - Effect cycle button
    constexpr int BUTTON2_PIN = 12; // GPIO12 (D6) - Effect intensity/modifier button
  }

  // Timing Configuration
  namespace Timing
  {
    constexpr unsigned long DEBOUNCE_DELAY = 200;        // Debounce time in ms
    constexpr unsigned long EFFECT_UPDATE_INTERVAL = 20; // Effect update interval
  }

  // Effect Configuration
  namespace Effects
  {
    constexpr uint8_t MIN_BRIGHTNESS = 64;  // Minimum brightness level
    constexpr uint8_t BRIGHTNESS_STEP = 64; // Brightness step for modifier button
  }
}