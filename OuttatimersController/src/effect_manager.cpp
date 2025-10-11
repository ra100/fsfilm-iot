#include "effect_manager.h"
#include <esp_log.h>

// Global battery percentage variable accessible by effects
extern int batteryPercentage;

// RandomBlinkEffect implementation

void RandomBlinkEffect::begin(ILEDDriver &driver)
{
  ESP_LOGI("RandomBlink", "Starting random blink effect");
  startTime_ = esp_timer_get_time();
  lastUpdateTime_ = startTime_;
  isRunning_ = true;
  activeLedCount_ = 0;

  // Clear all LEDs first
  driver.clear();

  // Turn on 1-2 random LEDs from the 1-6 range
  size_t ledsToTurnOn = (rand() % 2) + 1; // 1 or 2 LEDs

  for (size_t i = 0; i < ledsToTurnOn; i++)
  {
    size_t randomLed;
    do
    {
      randomLed = getRandomLedIndex(); // Get random LED 1-6
    } while (isLedActive(randomLed)); // Avoid duplicates

    driver.setPixel(logicalToPhysical(randomLed), COLOR_RED_GRB);
    addActiveLed(randomLed);
    ESP_LOGI("RandomBlink", "Turned on LED %d", randomLed);
  }

  driver.show();
  ESP_LOGI("RandomBlink", "Random blink started with %d active LEDs", activeLedCount_);
}

void RandomBlinkEffect::update(ILEDDriver &driver, int64_t currentTime)
{
  if (!isRunning_)
    return;

  // Check if 15 seconds have elapsed
  if (shouldStop(currentTime))
  {
    ESP_LOGI("RandomBlink", "15 seconds elapsed, stopping effect");
    isRunning_ = false;
    driver.clear();
    driver.show();
    return;
  }

  // Update every 200ms
  if (currentTime - lastUpdateTime_ >= BLINK_INTERVAL_MS * 1000)
  {
    // Turn off currently active LEDs
    for (size_t i = 0; i < activeLedCount_; i++)
    {
      driver.setPixel(logicalToPhysical(activeLeds_[i]), 0);
    }

    // Reset active LED count
    activeLedCount_ = 0;

    // Turn on 1-2 new random LEDs
    size_t ledsToTurnOn = (rand() % 2) + 1; // 1 or 2 LEDs

    for (size_t i = 0; i < ledsToTurnOn; i++)
    {
      size_t randomLed;
      do
      {
        randomLed = getRandomLedIndex(); // Get random LED 1-6
      } while (isLedActive(randomLed)); // Avoid duplicates

      driver.setPixel(logicalToPhysical(randomLed), COLOR_RED_GRB);
      addActiveLed(randomLed);
      ESP_LOGI("RandomBlink", "Turned on LED %d", randomLed);
    }

    driver.show();
    lastUpdateTime_ = currentTime;
  }
}

void RandomBlinkEffect::end(ILEDDriver &driver)
{
  ESP_LOGI("RandomBlink", "Ending random blink effect");
  isRunning_ = false;
  activeLedCount_ = 0;
  driver.clear();
  driver.show();
}

// RotatingDarknessEffect implementation

void RotatingDarknessEffect::begin(ILEDDriver &driver)
{
  ESP_LOGI("RotatingDarkness", "Starting rotating darkness effect");
  lastStepTime_ = esp_timer_get_time();
  darkLed_ = 0;

  // Turn on all first 6 LEDs
  for (size_t i = 0; i < 6; i++)
  {
    driver.setPixel(logicalToPhysical(i), COLOR_MAIN_GRB);
  }

  // Turn off the first dark LED
  driver.setPixel(logicalToPhysical(darkLed_), 0);
  driver.show();

  ESP_LOGI("RotatingDarkness", "Initialized with LED %d dark", darkLed_);
}

void RotatingDarknessEffect::update(ILEDDriver &driver, int64_t currentTime)
{
  if (currentTime - lastStepTime_ >= stepDurationMs_ * 1000)
  {
    // Turn on the currently dark LED
    driver.setPixel(logicalToPhysical(darkLed_), COLOR_MAIN_GRB);

    // Move to next dark LED position (cycle through first 6 LEDs only)
    darkLed_ = (darkLed_ + 1) % 6;

    // Turn off the new dark LED
    driver.setPixel(logicalToPhysical(darkLed_), 0);
    driver.show();

    ESP_LOGI("RotatingDarkness", "Dark LED moved to position %d", darkLed_);
    lastStepTime_ = currentTime;
  }
}

void RotatingDarknessEffect::end(ILEDDriver &driver)
{
  ESP_LOGI("RotatingDarkness", "Ending rotating darkness effect");
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

// PortalOpenEffect implementation
void PortalOpenEffect::begin(ILEDDriver &driver)
{
  ESP_LOGI("PortalOpen", "Starting portal open effect");
  lastStepTime_ = esp_timer_get_time();
  currentPhase_ = 0; // Start with buildup phase
  currentLed_ = 0;
  isComplete_ = false;

  // Start with all LEDs off
  driver.clear();
  driver.show();
}

void PortalOpenEffect::update(ILEDDriver &driver, int64_t currentTime)
{
  if (isComplete_)
    return;

  switch (currentPhase_)
  {
  case 0: // Sequential activation of first 6 LEDs
    if (currentTime - lastStepTime_ >= stepDurationMs_ * 1000)
    {
      if (currentLed_ < 6)
      {
        driver.setPixel(logicalToPhysical(currentLed_), COLOR_PORTAL_DIM_GRB);
        driver.show();
        ESP_LOGI("PortalOpen", "Sequential: LED %d turned on", currentLed_);
        currentLed_++;
        lastStepTime_ = currentTime;
      }
      else
      {
        currentPhase_ = 1;
        ESP_LOGI("PortalOpen", "Sequential complete, moving to last LED");
      }
    }
    break;

  case 1: // Turn on last LED
    driver.setPixel(logicalToPhysical(7), COLOR_PORTAL_GRB);
    driver.show();
    ESP_LOGI("PortalOpen", "Last LED turned on");
    currentPhase_ = 2;
    lastStepTime_ = currentTime; // Start 1s timer
    break;

  case 2: // Wait 1 second
    if (currentTime - lastStepTime_ >= 1000000)
    {
      currentPhase_ = 3;
      ESP_LOGI("PortalOpen", "1 second wait complete, turning all off");
    }
    break;

  case 3: // Turn all off and complete
    driver.clear();
    driver.show();
    isComplete_ = true;
    ESP_LOGI("PortalOpen", "All LEDs off, effect complete");
    break;
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
  // Initialize rotating darkness effect as the first effect (index 0)
  effects_[0] = std::make_unique<RotatingDarknessEffect>(200); // 200ms per step

  // Initialize portal open effect as the second effect (index 1)
  effects_[1] = std::make_unique<PortalOpenEffect>(500); // 500ms per step

  // Initialize battery status effect as the third effect (index 2)
  effects_[2] = std::make_unique<BatteryStatusEffect>();

  // Initialize random blink effect as the fourth effect (index 3)
  effects_[3] = std::make_unique<RandomBlinkEffect>();
  effectCount_ = 4;

  // TODO: Add more effects here in the future
  // effects_[4] = std::make_unique<SolidColorEffect>();
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