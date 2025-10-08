#include "effect_manager.h"
#include <esp_log.h>

// Global battery percentage variable accessible by effects
extern int batteryPercentage;

// PurpleChaseEffect implementation

void PurpleChaseEffect::begin(ILEDDriver &driver)
{
  ESP_LOGI("PurpleChase", "Starting purple chase effect");
  lastStepTime_ = esp_timer_get_time();
  currentLed_ = 0;

  // Start with first LED on
  driver.clear();
  driver.setPixel(logicalToPhysical(0), PurpleChaseEffect::COLOR_PORTAL_GRB);
  driver.show();
}

void PurpleChaseEffect::update(ILEDDriver &driver, int64_t currentTime)
{
  if (currentTime - lastStepTime_ >= stepDurationMs_ * 1000)
  {
    // Turn off current LED
    driver.setPixel(logicalToPhysical(currentLed_), 0);

    // Move to next LED (logical index 0-6 for 7 active LEDs)
    currentLed_ = (currentLed_ + 1) % ControllerConfig::Effects::ACTIVE_LED_COUNT;

    // Turn on next LED
    driver.setPixel(logicalToPhysical(currentLed_), PurpleChaseEffect::COLOR_PORTAL_GRB);
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

// BatteryStatusEffect implementation

void BatteryStatusEffect::begin(ILEDDriver &driver)
{
  ESP_LOGI("BatteryStatus", "Starting battery status effect");
  lastUpdateTime_ = esp_timer_get_time();

  // Initial update to show current battery level
  update(driver, lastUpdateTime_);
}

void BatteryStatusEffect::update(ILEDDriver &driver, int64_t currentTime)
{
  // Update battery display every second to avoid excessive updates
  if (currentTime - lastUpdateTime_ >= 1000000) // 1 second in microseconds
  {
    // Access battery percentage from main application
    // Note: This requires the batteryPercentage variable to be accessible
    // For now, we'll use a placeholder - this should be set by the main application
    extern int batteryPercentage; // Access global battery variable from main.cpp

    size_t ledsToLight = calculateLedCount(batteryPercentage);

    ESP_LOGI("BatteryStatus", "Battery: %d%% - LEDs to light: %d", batteryPercentage, ledsToLight);

    // Clear all LEDs first
    for (size_t i = 0; i < ControllerConfig::Effects::ACTIVE_LED_COUNT; i++)
    {
      driver.setPixel(logicalToPhysical(i), 0);
    }

    // Light up the appropriate number of LEDs
    for (size_t i = 0; i < ledsToLight && i < ControllerConfig::Effects::ACTIVE_LED_COUNT; i++)
    {
      driver.setPixel(logicalToPhysical(i), COLOR_GREEN_GRB);
    }

    driver.show();
    lastUpdateTime_ = currentTime;
  }
}

void BatteryStatusEffect::end(ILEDDriver &driver)
{
  ESP_LOGI("BatteryStatus", "Ending battery status effect");
  driver.clear();
  driver.show();
}

// RainbowEffect implementation

void RainbowEffect::begin(ILEDDriver &driver)
{
  ESP_LOGI("Rainbow", "Starting rainbow debug effect");

  // Clear all LEDs first
  driver.clear();

  // Set each LED to a different rainbow color
  for (size_t i = 0; i < ControllerConfig::Effects::ACTIVE_LED_COUNT; i++)
  {
    uint32_t color = getRainbowColor(i);
    driver.setPixel(logicalToPhysical(i), color);
    ESP_LOGI("Rainbow", "LED %d set to color 0x%08X", i, color);
  }

  driver.show();
  ESP_LOGI("Rainbow", "Rainbow pattern displayed - check LED colors and positions");
}

void RainbowEffect::update(ILEDDriver &driver, int64_t currentTime)
{
  // Static rainbow display - no updates needed
  // This effect is for debugging LED positions and colors
}

void RainbowEffect::end(ILEDDriver &driver)
{
  ESP_LOGI("Rainbow", "Ending rainbow debug effect");
  driver.clear();
  driver.show();
}

// PortalOpenEffect implementation

void PortalOpenEffect::begin(ILEDDriver &driver)
{
  ESP_LOGI("PortalOpen", "Starting portal open effect");
  lastStepTime_ = esp_timer_get_time();
  currentPhase_ = 0; // Start with buildup phase
  currentLed_ = 0;

  // Start with all LEDs off
  driver.clear();
  driver.show();
}

void PortalOpenEffect::update(ILEDDriver &driver, int64_t currentTime)
{
  if (currentTime - lastStepTime_ >= stepDurationMs_ * 500)
  {
    switch (currentPhase_)
    {
    case 0:                // Buildup phase: turn on first 6 LEDs one by one
      if (currentLed_ < 6) // First 6 LEDs (indices 0-5)
      {
        // Turn on current LED with dimmed purple (80% brightness)
        driver.setPixel(logicalToPhysical(currentLed_), COLOR_PORTAL_DIM_GRB);
        driver.show();
        ESP_LOGI("PortalOpen", "Buildup: LED %d turned on (dimmed)", currentLed_);

        currentLed_++;
        if (currentLed_ >= 6)
        {
          // All 6 LEDs are now on, move to climax preparation
          currentPhase_ = 1;
          ESP_LOGI("PortalOpen", "Buildup complete, entering climax preparation");
        }
      }
      break;

    case 1: // Climax preparation: all 6 LEDs on, wait for dramatic pause
      // Just wait in this phase - all 6 LEDs are already on
      currentPhase_ = 2; // Move to climax
      ESP_LOGI("PortalOpen", "Climax preparation complete, starting climax");
      break;

    case 2: // Climax: turn on 7th LED and turn off first 6
      // Turn off first 6 LEDs
      for (size_t i = 0; i < 6; i++)
      {
        driver.setPixel(logicalToPhysical(i), 0);
      }

      // Turn on 7th LED (index 6) with full brightness
      driver.setPixel(logicalToPhysical(6), COLOR_PORTAL_GRB);
      driver.show();
      ESP_LOGI("PortalOpen", "Climax: First 6 LEDs off, 7th LED on (full brightness)");

      // Move to cycling phase instead of resetting
      currentPhase_ = 3;
      currentLed_ = 0;
      ESP_LOGI("PortalOpen", "Starting cycling phase");
      break;

    case 3: // Cycling phase: 7th LED stays on, single light cycles through first 6 LEDs
      // Turn off current cycling LED
      driver.setPixel(logicalToPhysical(currentLed_), 0);

      // Move to next LED (cycle through first 6 LEDs only)
      currentLed_ = (currentLed_ + 1) % 6;

      // Turn on next cycling LED with dimmed purple (7th LED stays on at full brightness)
      driver.setPixel(logicalToPhysical(currentLed_), COLOR_PORTAL_DIM_GRB);
      driver.show();

      ESP_LOGI("PortalOpen", "Cycling: LED %d active (dimmed), 7th LED remains on (full brightness)", currentLed_);
      break;
    }

    lastStepTime_ = currentTime;
  }
}

void PortalOpenEffect::end(ILEDDriver &driver)
{
  ESP_LOGI("PortalOpen", "Ending portal open effect");
  driver.clear();
  driver.show();
}

// EffectManager implementation

EffectManager::EffectManager(ILEDDriver &driver) : driver_(driver), currentEffectIndex_(0), brightness_(ControllerConfig::Hardware::DEFAULT_BRIGHTNESS), ledsOn_(true)
{
  initializeEffects();
}

void EffectManager::begin()
{
  ESP_LOGI("EffectManager", "Starting effect manager with %d effects", effectCount_);
  switchToEffect(1); // Start with portal open as default
}

void EffectManager::update(int64_t currentTime)
{
  if (ledsOn_ && effectCount_ > 0 && effects_[currentEffectIndex_])
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

void EffectManager::setLedsOn(bool on)
{
  ledsOn_ = on;

  if (on)
  {
    // Resume current effect
    if (effects_[currentEffectIndex_])
    {
      effects_[currentEffectIndex_]->begin(driver_);
      ESP_LOGI("EffectManager", "LEDs turned ON - Resumed effect: %s", getCurrentEffectName());
    }
  }
  else
  {
    // Turn off all LEDs
    driver_.clear();
    driver_.show();
    ESP_LOGI("EffectManager", "LEDs turned OFF");
  }
}

void EffectManager::toggleLeds()
{
  setLedsOn(!ledsOn_);
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

  // Initialize portal open effect as the second effect (index 1)
  effects_[1] = std::make_unique<PortalOpenEffect>(500); // 500ms per step

  // Initialize battery status effect as the third effect (index 2)
  effects_[2] = std::make_unique<BatteryStatusEffect>();

  // Initialize rainbow debug effect as the fourth effect (index 3)
  effects_[3] = std::make_unique<RainbowEffect>();
  effectCount_ = 4;

  // TODO: Add more effects here in the future
  // effects_[4] = std::make_unique<SolidColorEffect>();
  // effects_[5] = std::make_unique<AdvancedEffect>();
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