/**
 * @file status_led.h
 * @brief Status LED manager for WiFi connection indication
 *
 * This module provides a simple status LED manager that blinks the on-board LED
 * to indicate WiFi connection status. The LED blinks slowly when connected and
 * fast when attempting to connect.
 */

#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>
#include "config.h"

/**
 * @class StatusLED
 * @brief Manages the on-board status LED for WiFi connection indication
 *
 * The status LED provides visual feedback about WiFi connection state:
 * - 2s blink: Started, not connected
 * - Fast blink: Attempting to connect to WiFi
 * - On with short off every 5s: Connected to WiFi STA
 * - 1s blink: In AP mode, no clients
 * - On with short off every 10s: AP with clients
 */
class StatusLED
{
public:
  /**
   * @brief Initialize the status LED
   *
   * Sets up the LED pin as output and initializes to off state.
   */
  static void begin();

  /**
   * @brief Update the LED state based on WiFi status
   *
   * @param status Current WiFi connection status
   * @param currentTime Current time in milliseconds (from millis())
   */
  static void update(TurboliftConfig::Hardware::WiFiStatus status, unsigned long currentTime);

  /**
   * @brief Turn off the status LED
   */
  static void off();

private:
  static bool initialized_;
  static unsigned long lastToggleTime_;
  static bool ledState_;
  static TurboliftConfig::Hardware::WiFiStatus currentStatus_;
  static unsigned long cycleStartTime_; // For cycle-based blinking

  /**
   * @brief Set the LED state (handles active-low logic)
   * @param state true for on, false for off
   */
  static void setLED(bool state);

  /**
   * @brief Get the blink interval based on status
   * @param status The WiFi status
   * @return Blink interval in milliseconds
   */
  static unsigned long getBlinkInterval(TurboliftConfig::Hardware::WiFiStatus status);
};

#endif // STATUS_LED_H