#pragma once

#include "config.h"

#ifdef UNIT_TEST
using unsigned_long_t = unsigned long;
constexpr int HIGH = static_cast<int>(TurboliftConfig::PinState::High);
constexpr int LOW = static_cast<int>(TurboliftConfig::PinState::Low);
#else
#include <Arduino.h>
using unsigned_long_t = unsigned long;
#endif

/**
 * @brief Debounces digital input signals to filter out mechanical switch bounce
 *
 * This class implements a simple time-based debouncing algorithm that waits for
 * a configurable interval after detecting a state change before accepting it as stable.
 * This prevents false triggers from mechanical switch bounce or electrical noise.
 *
 * @example
 * ```cpp
 * Debounce buttonDebouncer(50); // 50ms debounce interval
 *
 * void loop() {
 *     unsigned long now = millis();
 *     int rawState = digitalRead(BUTTON_PIN);
 *     if (buttonDebouncer.sample(rawState, now)) {
 *         // Stable state change detected
 *         handleButtonPress();
 *     }
 * }
 * ```
 *
 * @note This class is not thread-safe
 * @performance O(1) time complexity for all operations
 */
class Debounce
{
public:
  /**
   * @brief Construct a new Debounce object
   * @param intervalMs Debounce interval in milliseconds (default: 50ms)
   */
  explicit Debounce(unsigned_long_t intervalMs = TurboliftConfig::Timing::DEBOUNCE_INTERVAL_MS)
      : _interval(intervalMs), _lastChange(0), _stableState(HIGH), _lastRead(HIGH) {}

  /**
   * @brief Sample the current pin state and detect stable state changes
   * @param rawState Current raw pin state (HIGH or LOW)
   * @param now Current timestamp in milliseconds
   * @return true if a stable state change was detected, false otherwise
   *
   * Call this method periodically with the current pin reading and timestamp.
   * The method will return true only when a state change has been stable for
   * the configured debounce interval.
   */
  bool sample(int rawState, unsigned_long_t now)
  {
    if (rawState != _lastRead)
    {
      _lastChange = now;
      _lastRead = rawState;
    }
    else if (now >= _lastChange && (now - _lastChange) >= _interval) // Prevent overflow
    {
      if (_stableState != _lastRead)
      {
        _stableState = _lastRead;
        return true;
      }
    }
    return false;
  }

  /**
   * @brief Get the current stable state
   * @return Current debounced state (HIGH or LOW)
   */
  int getState() const { return _stableState; }

private:
  unsigned_long_t _interval;   ///< Debounce interval in milliseconds
  unsigned_long_t _lastChange; ///< Timestamp of last state change
  int _stableState;            ///< Current stable state
  int _lastRead;               ///< Last raw reading
};
