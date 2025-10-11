#pragma once

#ifndef EFFECT_MANAGER_H
#define EFFECT_MANAGER_H

#include "led_driver.h"
#include "config.h"
#include <esp_timer.h>
#include <memory>

// Forward declaration
class ILEDDriver;

/**
 * @brief Base class for LED effects
 */
class ILEDEffect
{
public:
  virtual void begin(ILEDDriver &driver) = 0;
  virtual void update(ILEDDriver &driver, int64_t currentTime) = 0;
  virtual void end(ILEDDriver &driver) = 0;
  virtual const char *getName() const = 0;
  virtual ~ILEDEffect() {}

protected:
  /**
   * @brief Create packed GRB color value with brightness control
   * @param g Green component (0-255)
   * @param r Red component (0-255)
   * @param b Blue component (0-255)
   * @param brightness Brightness multiplier (0.0-1.0)
   * @return Packed 32-bit GRB value with brightness applied
   */
  static constexpr uint32_t makeColor(uint8_t g, uint8_t r, uint8_t b, float brightness = 1.0f)
  {
    // Apply brightness scaling
    uint8_t g_scaled = static_cast<uint8_t>(g * brightness);
    uint8_t r_scaled = static_cast<uint8_t>(r * brightness);
    uint8_t b_scaled = static_cast<uint8_t>(b * brightness);

    // Pack as GRB format (Green, Red, Blue)
    return (static_cast<uint32_t>(g_scaled) << 16) |
           (static_cast<uint32_t>(r_scaled) << 8) |
           static_cast<uint32_t>(b_scaled);
  }
};

/**
 * @brief Rotating darkness effect - red color with blue/green, first 6 LEDs all on except one rotating
 *
 * Features:
 * - Uses red color with bit of blue and green
 * - Only first 6 LEDs are used (indices 0-5)
 * - All 6 LEDs are on, but one is off and rotates through them
 * - Creates a "chase the darkness" effect
 * - 200ms cycle time for smooth animation
 */
class RotatingDarknessEffect : public ILEDEffect
{
public:
  RotatingDarknessEffect(uint32_t stepDurationMs = 5) : stepDurationMs_(stepDurationMs), lastStepTime_(0), darkLed_(0) {}

  void begin(ILEDDriver &driver) override;
  void update(ILEDDriver &driver, int64_t currentTime) override;
  void end(ILEDDriver &driver) override;
  const char *getName() const override { return "Rotating Darkness"; }

private:
  uint32_t stepDurationMs_;
  int64_t lastStepTime_;
  size_t darkLed_; // Which LED is currently "dark" (off)

  // Color definitions using helper function
  // Red color with bit of blue and green
  static constexpr uint32_t COLOR_MAIN_GRB = makeColor(20, 150, 20); // Red with some green and blue

  /**
   * @brief Convert logical LED index to physical LED index
   * @param logicalIndex Logical index (0-5 for first 6 LEDs only)
   * @return Physical LED index from ACTIVE_LEDS array
   */
  uint8_t logicalToPhysical(size_t logicalIndex) const
  {
    return ControllerConfig::Effects::ACTIVE_LEDS[logicalIndex];
  }

  /**
   * @brief Set LED with special handling for LEDs 8 and 9 behaving as one
   * @param driver LED driver instance
   * @param logicalIndex Logical LED index
   * @param color Color to set
   */
  void setLedWithDualControl(ILEDDriver &driver, size_t logicalIndex, uint32_t color) const
  {
    if (logicalIndex == ControllerConfig::Effects::ACTIVE_LED_COUNT - 1)
    {
      // Last logical LED controls both physical LEDs 8 and 9
      driver.setPixel(8, color); // Physical LED 8
      driver.setPixel(9, color); // Physical LED 9
    }
    else
    {
      driver.setPixel(logicalToPhysical(logicalIndex), color);
    }
  }
};

/**
 * @brief Portal opening effect - runs once: sequential LED activation, climax, then stops
 *
 * Sequence:
 * 1. First 6 LEDs turn on one by one with 0.5s pauses (buildup phase)
 * 2. 7th LED turns on (portal climax)
 * 3. Wait 1 second
 * 4. All LEDs turn off and effect stops (no repetition)
 */
class PortalOpenEffect : public ILEDEffect
{
public:
  PortalOpenEffect(uint32_t stepDurationMs = 500) : stepDurationMs_(stepDurationMs), lastStepTime_(0), currentPhase_(0), currentLed_(0), isComplete_(false) {}

  void begin(ILEDDriver &driver) override;
  void update(ILEDDriver &driver, int64_t currentTime) override;
  void end(ILEDDriver &driver) override;
  const char *getName() const override { return "Portal Open"; }

private:
  uint32_t stepDurationMs_;
  int64_t lastStepTime_;
  uint8_t currentPhase_; // 0: buildup, 1: climax, 2: wait, 3: complete
  size_t currentLed_;    // Current LED in sequence
  bool isComplete_;      // Whether the effect has completed its sequence

  // Color definitions using helper function
  // Purple color: Green=0, Red=75, Blue=250 (reddish-purple)
  static constexpr uint32_t COLOR_PORTAL_GRB = makeColor(10, 30, 250); // Full brightness purple

  // Dimmed purple for buildup LEDs (80% brightness)
  static constexpr uint32_t COLOR_PORTAL_DIM_GRB = makeColor(10, 30, 250); // 80% brightness purple

  /**
   * @brief Convert logical LED index to physical LED index
   * @param logicalIndex Logical index (0-6)
   * @return Physical LED index from ACTIVE_LEDS array
   */
  uint8_t logicalToPhysical(size_t logicalIndex) const
  {
    return ControllerConfig::Effects::ACTIVE_LEDS[logicalIndex];
  }
};

/**
 * @brief Battery status effect - shows battery level with 1-7 green LEDs
 *
 * Battery level mapping:
 * - 0-14%: 1 LED
 * - 15-28%: 2 LEDs
 * - 29-42%: 3 LEDs
 * - 43-56%: 4 LEDs
 * - 57-70%: 5 LEDs
 * - 71-84%: 6 LEDs
 * - 85-100%: 7 LEDs (all LEDs)
 */
class BatteryStatusEffect : public ILEDEffect
{
public:
  BatteryStatusEffect() {}

  void begin(ILEDDriver &driver) override;
  void update(ILEDDriver &driver, int64_t currentTime) override;
  void end(ILEDDriver &driver) override;
  const char *getName() const override { return "Battery Status"; }

private:
  int64_t lastUpdateTime_;

  // Color definitions using helper function
  static constexpr uint32_t COLOR_GREEN_GRB = makeColor(255, 0, 0); // Green color for battery status

  /**
   * @brief Convert logical LED index to physical LED index
   * @param logicalIndex Logical index (0-6)
   * @return Physical LED index from ACTIVE_LEDS array
   */
  uint8_t logicalToPhysical(size_t logicalIndex) const
  {
    return ControllerConfig::Effects::ACTIVE_LEDS[logicalIndex];
  }

  /**
   * @brief Set LED with special handling for LEDs 8 and 9 behaving as one
   * @param driver LED driver instance
   * @param logicalIndex Logical LED index
   * @param color Color to set
   */
  void setLedWithDualControl(ILEDDriver &driver, size_t logicalIndex, uint32_t color) const
  {
    if (logicalIndex == ControllerConfig::Effects::ACTIVE_LED_COUNT - 1)
    {
      // Last logical LED controls both physical LEDs 8 and 9
      driver.setPixel(8, color); // Physical LED 8
      driver.setPixel(9, color); // Physical LED 9
    }
    else
    {
      driver.setPixel(logicalToPhysical(logicalIndex), color);
    }
  }

  /**
   * @brief Calculate how many LEDs should be lit based on battery percentage
   * @param batteryPercentage Battery level (0-100)
   * @return Number of LEDs to light (1-7)
   */
  size_t calculateLedCount(int batteryPercentage) const
  {
    if (batteryPercentage <= 0)
      return 0;
    if (batteryPercentage >= 100)
      return ControllerConfig::Effects::ACTIVE_LED_COUNT;

    // Calculate which range the percentage falls into
    // Each 14.28% represents one LED (100/7 ≈ 14.28)
    return (batteryPercentage + 14) / 15; // Round up to ensure at least 1 LED for any positive percentage
  }
};

/**
 * @brief Random blink effect - random red blinking with 15-second auto-stop
 *
 * Features:
 * - Uses only LEDs 1-6 (skips first LED, logical indices 1-6)
 * - Randomly blinks in red color
 * - Maximum 2 LEDs on at any time
 * - Runs for 15 seconds then auto-stops
 * - Only restarts when on/off button is pressed again
 * - Creates dynamic random blinking pattern
 */
class RandomBlinkEffect : public ILEDEffect
{
public:
  RandomBlinkEffect() : startTime_(0), isRunning_(false), lastUpdateTime_(0), activeLedCount_(0) {}

  void begin(ILEDDriver &driver) override;
  void update(ILEDDriver &driver, int64_t currentTime) override;
  void end(ILEDDriver &driver) override;
  const char *getName() const override { return "Random Blink"; }

private:
  static constexpr uint32_t EFFECT_DURATION_US = 5 * 1000000; // 15 seconds in microseconds
  static constexpr uint32_t BLINK_INTERVAL_MS = 200;          // Blink every 200ms
  static constexpr size_t MAX_ACTIVE_LEDS = 2;                // Maximum 2 LEDs on at once
  static constexpr size_t FIRST_LED_INDEX = 1;                // Start from LED 1, skip LED 0

  int64_t startTime_;
  bool isRunning_;
  int64_t lastUpdateTime_;
  size_t activeLedCount_;
  size_t activeLeds_[MAX_ACTIVE_LEDS]; // Track which LEDs are currently on

  // Color definitions using helper function
  static constexpr uint32_t COLOR_RED_GRB = makeColor(0, 255, 0); // Bright red color

  /**
   * @brief Convert logical LED index to physical LED index
   * @param logicalIndex Logical index (1-6 for this effect)
   * @return Physical LED index from ACTIVE_LEDS array
   */
  uint8_t logicalToPhysical(size_t logicalIndex) const
  {
    return ControllerConfig::Effects::ACTIVE_LEDS[logicalIndex];
  }

  /**
   * @brief Set LED with special handling for LEDs 8 and 9 behaving as one
   * @param driver LED driver instance
   * @param logicalIndex Logical LED index
   * @param color Color to set
   */
  void setLedWithDualControl(ILEDDriver &driver, size_t logicalIndex, uint32_t color) const
  {
    if (logicalIndex == ControllerConfig::Effects::ACTIVE_LED_COUNT - 1)
    {
      // Last logical LED controls both physical LEDs 8 and 9
      driver.setPixel(8, color); // Physical LED 8
      driver.setPixel(9, color); // Physical LED 9
    }
    else
    {
      driver.setPixel(logicalToPhysical(logicalIndex), color);
    }
  }

  /**
   * @brief Check if effect should stop (15 seconds elapsed)
   * @param currentTime Current time in microseconds
   * @return true if effect should stop
   */
  bool shouldStop(int64_t currentTime) const
  {
    return isRunning_ && (currentTime - startTime_) >= EFFECT_DURATION_US;
  }

  /**
   * @brief Get random LED index (1-6)
   * @return Random logical LED index
   */
  size_t getRandomLedIndex() const
  {
    return (rand() % 6) + FIRST_LED_INDEX; // Random 1-6
  }

  /**
   * @brief Check if LED is currently active
   * @param ledIndex LED index to check
   * @return true if LED is on
   */
  bool isLedActive(size_t ledIndex) const
  {
    for (size_t i = 0; i < activeLedCount_; i++)
    {
      if (activeLeds_[i] == ledIndex)
        return true;
    }
    return false;
  }

  /**
   * @brief Add LED to active list
   * @param ledIndex LED index to add
   */
  void addActiveLed(size_t ledIndex)
  {
    if (activeLedCount_ < MAX_ACTIVE_LEDS)
    {
      activeLeds_[activeLedCount_] = ledIndex;
      activeLedCount_++;
    }
  }

  /**
   * @brief Remove LED from active list
   * @param ledIndex LED index to remove
   */
  void removeActiveLed(size_t ledIndex)
  {
    for (size_t i = 0; i < activeLedCount_; i++)
    {
      if (activeLeds_[i] == ledIndex)
      {
        // Move last element to current position
        activeLeds_[i] = activeLeds_[activeLedCount_ - 1];
        activeLedCount_--;
        break;
      }
    }
  }
};

/**
 * @brief Effect manager to handle different LED effects and transitions
 */
class EffectManager
{
public:
  EffectManager(ILEDDriver &driver);

  void begin();
  void update(int64_t currentTime);
  void nextEffect();
  void previousEffect();
  void setEffect(uint8_t effectIndex);
  void setBrightness(uint8_t brightness);
  void setLedsOn(bool on);
  void toggleLeds();

  uint8_t getCurrentEffectIndex() const { return currentEffectIndex_; }
  uint8_t getEffectCount() const { return effectCount_; }
  const char *getCurrentEffectName() const;
  bool areLedsOn() const { return ledsOn_; }

private:
  ILEDDriver &driver_;
  static constexpr uint8_t MAX_EFFECTS = 5;
  std::unique_ptr<ILEDEffect> effects_[MAX_EFFECTS];
  uint8_t currentEffectIndex_;
  uint8_t effectCount_;
  uint8_t brightness_;
  bool ledsOn_;

  void initializeEffects();
  void switchToEffect(uint8_t effectIndex);
};

#endif // EFFECT_MANAGER_H