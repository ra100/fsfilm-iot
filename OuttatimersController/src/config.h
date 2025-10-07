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
    constexpr int LED_PIN = 4;                  // GPIO4 (D4 on ESP32-C6-DevKitC-1)
    constexpr int NUM_LEDS = 9;                 // Number of LEDs in the strip
    constexpr uint8_t DEFAULT_BRIGHTNESS = 255; // Maximum brightness

    // Button pin assignments
    constexpr int BUTTON1_PIN = 5; // GPIO5 (D5) - Effect cycle button
    constexpr int BUTTON2_PIN = 6; // GPIO6 (D6) - Effect intensity/modifier button
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

  // WiFi Configuration
  namespace WiFi
  {
    constexpr unsigned long CONNECTION_TIMEOUT_MS = 10000;    // 10 seconds timeout
    constexpr int HTTP_PORT = 80;                             // HTTP server port
    constexpr int MAX_CONNECTION_ATTEMPTS = 3;                // Max connection attempts
    constexpr unsigned long CONNECTION_RETRY_DELAY_MS = 1000; // 1 second between attempts
  }
}