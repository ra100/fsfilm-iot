#pragma once

#define COLOR_ORDER GRB

#include <stdint.h>

/**
 * @file config.h
 * @brief Configuration constants for the Portal LED Controller
 *
 * This file centralizes all magic numbers and configuration parameters
 * to improve maintainability and make the system easier to tune.
 */

// WiFi Enable Flag (for preprocessor)
#define ENABLE_WIFI_CONTROL 1 // Set to 1 to enable WiFi control

namespace PortalConfig
{
  // Hardware Configuration
  namespace Hardware
  {
    constexpr int LED_PIN = 4;                    // GPIO4 (D2 on Lolin D1)
    constexpr int NUM_LEDS = 756;                 // Total LED count in strip 800 - 46 CUT
    constexpr uint8_t DEFAULT_BRIGHTNESS = 255;   // Maximum brightness
    constexpr uint8_t DIAGNOSTIC_BRIGHTNESS = 25; // ~10% for startup diagnostics

    // Button pin assignments
    constexpr int BUTTON1_PIN = 14; // GPIO14 (D5) - Portal toggle
    constexpr int BUTTON2_PIN = 12; // GPIO12 (D6) - Malfunction trigger
    constexpr int BUTTON3_PIN = 13; // GPIO13 (D7) - Fade out

    // Status LED (on-board LED)
    constexpr int STATUS_LED_PIN = 2;            // GPIO2 (D4) - On-board LED on most ESP8266 boards
    constexpr bool STATUS_LED_ACTIVE_LOW = true; // Most on-board LEDs are active low

    enum class WiFiStatus
    {
      STARTED_NOT_CONNECTED, // Started, not connected to WiFi
      CONNECTING_STA,        // Connecting to STA
      STA_CONNECTED,         // Connected to WiFi STA
      STA_CONNECTED_CLIENTS, // STA connected with clients (though STA doesn't have AP clients, for consistency)
      AP_MODE,               // In AP mode, no clients
      AP_WITH_CLIENTS        // AP mode with connected clients
    };
  }

  // Timing Configuration
  namespace Timing
  {
    constexpr unsigned long UPDATE_INTERVAL_MS = 10;   // ~100 FPS update rate
    constexpr unsigned long DEBOUNCE_INTERVAL_MS = 50; // Button debounce time

    // Startup sequence timing
    constexpr unsigned long STARTUP_INITIAL_DELAY_MS = 100;
    constexpr unsigned long STARTUP_COLOR_DURATION_MS = 500;

    // Effect timing
    constexpr unsigned long FADE_IN_DURATION_MS = 3000; // 3 second fade in
    constexpr unsigned long FADE_OUT_DURATION_MS = 200; // 200ms fade out

    // Malfunction effect timing
    constexpr unsigned long MALFUNCTION_MIN_JUMP_MS = 40;
    constexpr unsigned long MALFUNCTION_MAX_JUMP_MS = 200;

    // Status LED timing
    constexpr unsigned long STATUS_LED_BLINK_INTERVAL_MS = 1000;      // Slow blink when WiFi connected
    constexpr unsigned long STATUS_LED_FAST_BLINK_MS = 200;           // Fast blink during WiFi connection
    constexpr unsigned long STATUS_LED_AP_BLINK_MS = 1000;            // 1s blink for AP mode
    constexpr unsigned long STATUS_LED_STARTED_BLINK_MS = 2000;       // 2s blink for started not connected
    constexpr unsigned long STATUS_LED_SHORT_OFF_MS = 100;            // Short off duration for connected states
    constexpr unsigned long STATUS_LED_STA_CONNECTED_CYCLE_MS = 5000; // Cycle for STA connected (short off every 5s)
    constexpr unsigned long STATUS_LED_AP_CLIENTS_CYCLE_MS = 10000;   // Cycle for AP with clients (short off every 10s)
  }

  // Effect Configuration
  namespace Effects
  {
    constexpr int GRADIENT_STEP_DEFAULT = 10; // Generate color every Nth LED
    constexpr int GRADIENT_MOVE_DEFAULT = 2;  // LEDs to move per update (2x speed)

    // Portal effect parameters
    constexpr int MIN_DRIVER_DISTANCE = 5;  // Minimum distance between color drivers
    constexpr int MAX_DRIVER_DISTANCE = 15; // Maximum distance between color drivers

    // Color generation parameters
    constexpr uint8_t PORTAL_HUE_BASE = 160;       // Base hue for portal colors (blue-purple range)
    constexpr uint8_t PORTAL_HUE_RANGE = 41;       // Hue variation range
    constexpr uint8_t PORTAL_SAT_BASE = 180;       // Base saturation
    constexpr uint8_t PORTAL_SAT_RANGE = 76;       // Saturation variation range
    constexpr uint8_t PORTAL_SAT_LOW_BASE = 30;    // Low saturation base (rare occurrence)
    constexpr uint8_t PORTAL_SAT_LOW_RANGE = 40;   // Low saturation range
    constexpr uint8_t PORTAL_VAL_BASE = 51;        // Base value (brightness)
    constexpr uint8_t PORTAL_VAL_RANGE = 205;      // Value variation range
    constexpr int PORTAL_LOW_SAT_PROBABILITY = 10; // 1 in 10 chance for low saturation

    // Malfunction effect parameters
    constexpr float MALFUNCTION_BRIGHTNESS_MIN = 0.2f;
    constexpr float MALFUNCTION_BRIGHTNESS_RANGE = 1.3f;
    constexpr float MALFUNCTION_BRIGHTNESS_SMOOTHING_MIN = 0.3f;
    constexpr float MALFUNCTION_BRIGHTNESS_SMOOTHING_RANGE = 0.5f;
    constexpr int MALFUNCTION_NOISE_RANGE = 61; // -30 to +30
    constexpr int MALFUNCTION_NOISE_OFFSET = 30;
    constexpr float MALFUNCTION_BRIGHTNESS_CLAMP_MIN = 0.05f;
    constexpr float MALFUNCTION_BRIGHTNESS_CLAMP_MAX = 1.5f;
    constexpr uint8_t MALFUNCTION_BASE_BRIGHTNESS = 170;
    constexpr uint8_t MALFUNCTION_BRIGHTNESS_OFFSET = 85;
  }

  // Mathematical Constants
  namespace Math
  {
    constexpr float PI_F = 3.14159265358979323846f;
  }

  // Pin States (type-safe alternatives to HIGH/LOW)
  enum class PinState : int
  {
    Low = 0,
    High = 1
  };

  // WiFi Configuration
  namespace WiFi
  {
    constexpr int HTTP_PORT = 80;                    // Web server port
    constexpr unsigned long WIFI_TIMEOUT_MS = 10000; // WiFi connection timeout

    // WiFi credentials are loaded from wifi_credentials.h (git-ignored)
    // Copy wifi_credentials.h.template to wifi_credentials.h and configure
#if ENABLE_WIFI_CONTROL
#include "../wifi_credentials.h"
    constexpr const char *DEFAULT_SSID = WIFI_SSID;
    constexpr const char *DEFAULT_PASSWORD = WIFI_PASSWORD;
    constexpr const char *AP_NAME = AP_SSID;
    constexpr const char *AP_PASS = AP_PASSWORD;
#else
    // Dummy values when WiFi is disabled
    constexpr const char *DEFAULT_SSID = "WiFi_Disabled";
    constexpr const char *DEFAULT_PASSWORD = "WiFi_Disabled";
    constexpr const char *AP_NAME = "WiFi_Disabled";
    constexpr const char *AP_PASS = "WiFi_Disabled";
#endif
  }
}