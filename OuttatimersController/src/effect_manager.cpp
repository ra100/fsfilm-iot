#include "effect_manager.h"
#include <esp_log.h>

// PurpleChaseEffect implementation

void PurpleChaseEffect::begin(ILEDDriver &driver)
{
  ESP_LOGI("PurpleChase", "Starting purple chase effect");
  lastStepTime_ = esp_timer_get_time();
  currentLed_ = 0;

  // Start with first LED on
  driver.clear();
  driver.setPixel(0, driver.Color(PURPLE_R, PURPLE_G, PURPLE_B));
  driver.show();
}

void PurpleChaseEffect::update(ILEDDriver &driver, int64_t currentTime)
{
  if (currentTime - lastStepTime_ >= stepDurationMs_ * 1000)
  {
    // Turn off current LED
    driver.setPixel(currentLed_, 0);

    // Move to next LED
    currentLed_ = (currentLed_ + 1) % ControllerConfig::Hardware::NUM_LEDS;

    // Turn on next LED
    driver.setPixel(currentLed_, driver.Color(PURPLE_R, PURPLE_G, PURPLE_B));
    driver.show();

    lastStepTime_ = currentTime;
  }
}

void PurpleChaseEffect::end(ILEDDriver &driver)
{
  ESP_LOGI("PurpleChase", "Ending purple chase effect");
  driver.clear();
  driver.show();
}

// EffectManager implementation

EffectManager::EffectManager(ILEDDriver &driver) : driver_(driver), currentEffectIndex_(0), brightness_(ControllerConfig::Hardware::DEFAULT_BRIGHTNESS)
{
  initializeEffects();
}

void EffectManager::begin()
{
  ESP_LOGI("EffectManager", "Starting effect manager with %d effects", effectCount_);
  switchToEffect(0); // Start with purple chase as default
}

void EffectManager::update(int64_t currentTime)
{
  if (effectCount_ > 0 && effects_[currentEffectIndex_])
  {
    effects_[currentEffectIndex_]->update(driver_, currentTime);
  }
}

void EffectManager::nextEffect()
{
  uint8_t nextIndex = (currentEffectIndex_ + 1) % effectCount_;
  switchToEffect(nextIndex);
}

void EffectManager::previousEffect()
{
  uint8_t prevIndex = (currentEffectIndex_ + effectCount_ - 1) % effectCount_;
  switchToEffect(prevIndex);
}

void EffectManager::setEffect(uint8_t effectIndex)
{
  if (effectIndex < effectCount_)
  {
    switchToEffect(effectIndex);
  }
}

void EffectManager::setBrightness(uint8_t brightness)
{
  brightness_ = brightness;
  driver_.setBrightness(brightness_);
}

const char *EffectManager::getCurrentEffectName() const
{
  if (currentEffectIndex_ < effectCount_ && effects_[currentEffectIndex_])
  {
    return effects_[currentEffectIndex_]->getName();
  }
  return "None";
}

void EffectManager::initializeEffects()
{
  // Initialize purple chase effect as the first effect (index 0)
  effects_[0] = std::make_unique<PurpleChaseEffect>(200); // 200ms per step
  effectCount_ = 1;

  // TODO: Add more effects here in the future
  // effects_[1] = std::make_unique<SolidColorEffect>();
  // effects_[2] = std::make_unique<RainbowEffect>();
  // etc.

  ESP_LOGI("EffectManager", "Initialized %d effects", effectCount_);
}

void EffectManager::switchToEffect(uint8_t effectIndex)
{
  if (effectIndex >= effectCount_)
  {
    return;
  }

  // End current effect
  if (effects_[currentEffectIndex_])
  {
    effects_[currentEffectIndex_]->end(driver_);
  }

  // Switch to new effect
  currentEffectIndex_ = effectIndex;

  // Begin new effect
  if (effects_[currentEffectIndex_])
  {
    effects_[currentEffectIndex_]->begin(driver_);
    ESP_LOGI("EffectManager", "Switched to effect %d: %s", currentEffectIndex_, getCurrentEffectName());
  }
}