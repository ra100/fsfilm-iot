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
    constexpr int LED_PIN = 22;                 // GPIO22 (D4 on Seeed Xiao ESP32-C6) - External LED strip (changed from GPIO1 which may be used for USB/Serial)
    constexpr int ONBOARD_LED_PIN = 2;          // GPIO2 (D2/A2) - Onboard LED (from pinout diagram)
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
    constexpr bool POWER_SAVE_MODE = true;                    // Enable WiFi power save
  }

  // Power Management Configuration
  namespace Power
  {
    constexpr uint8_t BATTERY_BRIGHTNESS = 64;            // Lower brightness for battery (0-255)
    constexpr uint8_t NORMAL_BRIGHTNESS = 128;            // Normal brightness (0-255)
    constexpr unsigned long LED_UPDATE_INTERVAL_MS = 50;  // LED update interval (slower = more power save)
    constexpr unsigned long HEARTBEAT_INTERVAL_MS = 3000; // Heartbeat interval (longer = more power save)
    constexpr bool ENABLE_SLEEP_MODE = false;             // Enable automatic sleep (future feature)
  }

  // Battery Monitoring Configuration
  namespace Battery
  {
    constexpr int VOLTAGE_PIN = 0;                   // GPIO0 (A0/D0) - ADC for battery voltage
    constexpr float VOLTAGE_DIVIDER_RATIO = 2.0f;    // Voltage divider ratio (R2/(R1+R2))
    constexpr float MIN_VOLTAGE = 3.0f;              // Minimum battery voltage (discharged)
    constexpr float MAX_VOLTAGE = 4.2f;              // Maximum battery voltage (fully charged)
    constexpr unsigned long READ_INTERVAL_MS = 5000; // Battery read interval (5 seconds)
  }
}