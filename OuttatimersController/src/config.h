#pragma once

#define COLOR_ORDER GRB
#define LED_TYPE WS2812B

#include <stdint.h>
#include <cstddef>

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
    constexpr int NUM_LEDS = 10;                // Number of LEDs in the strip (including LED 9)
    constexpr uint8_t DEFAULT_BRIGHTNESS = 128; // Maximum brightness

    // RMT Configuration for LED strip
    constexpr uint32_t RMT_RESOLUTION_HZ = 10 * 1000 * 1000; // 10MHz resolution for precise WS2812B timing
    constexpr size_t RMT_MEM_BLOCK_SYMBOLS = 64;             // RMT memory block size
    constexpr uint8_t RMT_TRANSMIT_QUEUE_DEPTH = 4;          // RMT transmit queue depth

    // Button pin assignments
    constexpr int BUTTON1_PIN = 23; // GPIO23 (D5) - Effect cycle button
    constexpr int BUTTON2_PIN = 16; // GPIO16 (D6) - Effect intensity/modifier button
  }

  // Timing Configuration
  namespace Timing
  {
    constexpr unsigned long EFFECT_UPDATE_INTERVAL = 20; // Effect update interval
  }

  // Effect Configuration
  namespace Effects
  {
    constexpr uint8_t MIN_BRIGHTNESS = 64;  // Minimum brightness level
    constexpr uint8_t BRIGHTNESS_STEP = 64; // Brightness step for modifier button

    // LED Selection Configuration
    // Specify which physical LED indices to use for effects (0-8 for 9 LEDs)
    // Currently using LEDs: 1,2,3,5,6,7,8,9 (8 LEDs, with 8 and 9 behaving as one)
    constexpr size_t ACTIVE_LED_COUNT = 8;
    constexpr uint8_t ACTIVE_LEDS[ACTIVE_LED_COUNT] = {1, 2, 3, 5, 6, 7, 8, 9};
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
    constexpr uint8_t BATTERY_BRIGHTNESS = 64;                // Lower brightness for battery (0-255)
    constexpr uint8_t NORMAL_BRIGHTNESS = 128;                // Normal brightness (0-255)
    constexpr unsigned long LED_UPDATE_INTERVAL_MS = 50;      // LED update interval (slower = more power save)
    constexpr unsigned long HEARTBEAT_INTERVAL_MS = 3000;     // Heartbeat interval (longer = more power save)
    constexpr bool ENABLE_SLEEP_MODE = true;                  // Enable automatic sleep when LEDs off
    constexpr unsigned long AUTOSLEEP_TIMEOUT_US = 600000000; // 10 minutes in microseconds
  }

  // Battery Monitoring Configuration
  namespace Battery
  {
    constexpr int VOLTAGE_PIN = 0;                   // GPIO0 (A0/D0) - ADC for battery voltage (XIAO ESP32C6 BAT+ pin)
    constexpr float VOLTAGE_DIVIDER_RATIO = 1.0f;    // Voltage divider ratio (R2/(R1+R2)) - no divider on this board
    constexpr float MIN_VOLTAGE = 3.0f;              // Minimum battery voltage (discharged)
    constexpr float MAX_VOLTAGE = 4.2f;              // Maximum battery voltage (fully charged)
    constexpr unsigned long READ_INTERVAL_MS = 5000; // Battery read interval (5 seconds)
  }
}