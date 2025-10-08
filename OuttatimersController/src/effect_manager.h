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
};

/**
 * @brief Purple chase effect - LEDs light up in sequence with purple color
 *
 * Requirements:
 * - All LEDs are purple (RGB: 128, 0, 128)
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
  uint8_t currentLed_;
  static constexpr uint8_t PURPLE_R = 128;
  static constexpr uint8_t PURPLE_G = 0;
  static constexpr uint8_t PURPLE_B = 128;
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

  uint8_t getCurrentEffectIndex() const { return currentEffectIndex_; }
  uint8_t getEffectCount() const { return effectCount_; }
  const char *getCurrentEffectName() const;

private:
  ILEDDriver &driver_;
  static constexpr uint8_t MAX_EFFECTS = 6;
  std::unique_ptr<ILEDEffect> effects_[MAX_EFFECTS];
  uint8_t currentEffectIndex_;
  uint8_t effectCount_;
  uint8_t brightness_;

  void initializeEffects();
  void switchToEffect(uint8_t effectIndex);
};

#endif // EFFECT_MANAGER_H