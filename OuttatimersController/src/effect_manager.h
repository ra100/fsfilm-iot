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
 * @brief Purple chase effect - LEDs light up in sequence with purple color
 *
 * Requirements:
 * - All LEDs are purple (GRB: 0, 200, 100) → RGB: 200, 0, 100
 * - LEDs light up in sequence (one at a time)
 * - Only one LED is on at any given time
 * - This is the default starting effect
 */
class PurpleChaseEffect : public ILEDEffect
{
public:
  PurpleChaseEffect(uint32_t stepDurationMs = 200) : stepDurationMs_(stepDurationMs), lastStepTime_(0), currentLed_(0) {}

  void begin(ILEDDriver &driver) override;
  void update(ILEDDriver &driver, int64_t currentTime) override;
  void end(ILEDDriver &driver) override;
  const char *getName() const override { return "Purple Chase"; }

private:
  uint32_t stepDurationMs_;
  int64_t lastStepTime_;
  size_t currentLed_; // Logical LED index (0-6 for 7 active LEDs)

  static constexpr uint32_t COLOR_PORTAL_GRB = makeColor(0, 255, 255); // Full brightness purple

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
 * @brief Portal opening effect - sequential LED activation with buildup, climax, and cycling
 *
 * Sequence:
 * 1. First 6 LEDs turn on one by one with 0.5s pauses (buildup phase)
 * 2. All 6 LEDs remain on (climax preparation)
 * 3. 7th LED turns on while first 6 turn off (portal climax)
 * 4. 7th LED stays on while single light cycles through first 6 LEDs (cycling phase)
 */
class PortalOpenEffect : public ILEDEffect
{
public:
  PortalOpenEffect(uint32_t stepDurationMs = 500) : stepDurationMs_(stepDurationMs), lastStepTime_(0), currentPhase_(0), currentLed_(0) {}

  void begin(ILEDDriver &driver) override;
  void update(ILEDDriver &driver, int64_t currentTime) override;
  void end(ILEDDriver &driver) override;
  const char *getName() const override { return "Portal Open"; }

private:
  uint32_t stepDurationMs_;
  int64_t lastStepTime_;
  uint8_t currentPhase_; // 0: buildup, 1: climax prep, 2: climax, 3: cycling
  size_t currentLed_;    // Current LED in sequence

  // Color definitions using helper function
  // Purple color: Green=0, Red=75, Blue=250 (reddish-purple)
  static constexpr uint32_t COLOR_PORTAL_GRB = makeColor(10, 30, 250); // Full brightness purple

  // Dimmed purple for buildup LEDs (80% brightness)
  static constexpr uint32_t COLOR_PORTAL_DIM_GRB = makeColor(10, 30, 250, 0.8f); // 80% brightness purple

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
 * @brief Rainbow effect - displays rainbow colors for LED debugging and identification
 *
 * Features:
 * - Each LED shows a different color of the rainbow (based on actual observation)
 * - Helps identify LED positions and test color accuracy
 * - Uses the full visible spectrum for clear LED identification
 * - Static display (no animation) for easy observation
 *
 * Actual LED Color Mapping:
 * - Logical 0 → Physical LED2: Green
 * - Logical 1 → Physical LED3: Yellow
 * - Logical 2 → Physical LED4: Orange
 * - Logical 3 → Physical LED6: Red
 * - Logical 4 → Physical LED7: Blue
 * - Logical 5 → Physical LED8: Indigo
 * - Logical 6 → Physical LED9: Violet
 */
class RainbowEffect : public ILEDEffect
{
public:
  RainbowEffect() {}

  void begin(ILEDDriver &driver) override;
  void update(ILEDDriver &driver, int64_t currentTime) override;
  void end(ILEDDriver &driver) override;
  const char *getName() const override { return "Rainbow Debug"; }

private:
  // Rainbow colors in GRB format for clear LED identification
  // Updated based on actual LED strip observation
  static constexpr uint32_t COLOR_RED_GRB = 0x80000000;    // Red (for LED6)
  static constexpr uint32_t COLOR_ORANGE_GRB = 0xFFFF8000; // Orange (for LED4)
  static constexpr uint32_t COLOR_YELLOW_GRB = 0xFF800000; // Yellow (for LED3)
  static constexpr uint32_t COLOR_GREEN_GRB = 0x00FF0000;  // Green (for LED2)
  static constexpr uint32_t COLOR_BLUE_GRB = 0xFF0000FF;   // Blue (for LED7) ✓
  static constexpr uint32_t COLOR_INDIGO_GRB = 0xFF0080FF; // Indigo (for LED8) ✓
  static constexpr uint32_t COLOR_VIOLET_GRB = 0xFF00FF80; // Violet (for LED9) ✓

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
   * @brief Get rainbow color for a specific LED position
   * @param ledIndex LED position (0-6)
   * @return GRB color value for that position
   */
  uint32_t getRainbowColor(size_t ledIndex) const
  {
    // Updated based on actual LED strip observation
    switch (ledIndex)
    {
    case 0:
      return COLOR_GREEN_GRB; // Physical LED2: Green
    case 1:
      return COLOR_YELLOW_GRB; // Physical LED3: Yellow
    case 2:
      return COLOR_ORANGE_GRB; // Physical LED4: Orange
    case 3:
      return COLOR_RED_GRB; // Physical LED6: Red
    case 4:
      return COLOR_BLUE_GRB; // Physical LED7: Blue ✓
    case 5:
      return COLOR_INDIGO_GRB; // Physical LED8: Indigo ✓
    case 6:
      return COLOR_VIOLET_GRB; // Physical LED9: Violet ✓
    default:
      return COLOR_RED_GRB; // Fallback to red
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
  static constexpr uint8_t MAX_EFFECTS = 6;
  std::unique_ptr<ILEDEffect> effects_[MAX_EFFECTS];
  uint8_t currentEffectIndex_;
  uint8_t effectCount_;
  uint8_t brightness_;
  bool ledsOn_;

  void initializeEffects();
  void switchToEffect(uint8_t effectIndex);
};

#endif // EFFECT_MANAGER_H