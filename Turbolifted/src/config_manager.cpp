#include "config_manager.h"

// Legacy gradient effect parameters
int ConfigManager::rotationSpeed = 2;
uint8_t ConfigManager::maxBrightness = 255;
uint8_t ConfigManager::hueMin = 160;
uint8_t ConfigManager::hueMax = 200;
uint8_t ConfigManager::satMin = 128;
uint8_t ConfigManager::satMax = 255;
bool ConfigManager::effectNeedsRegeneration = false;
int ConfigManager::turboliftMode = 0;

// New lift animation parameters
uint8_t ConfigManager::liftSpeed = TurboliftConfig::Effects::DEFAULT_SPEED;
uint8_t ConfigManager::liftWidth = TurboliftConfig::Effects::DEFAULT_WIDTH;
uint8_t ConfigManager::liftSpacing = TurboliftConfig::Effects::DEFAULT_SPACING;
uint8_t ConfigManager::liftHue = TurboliftConfig::Effects::DEFAULT_HUE;
uint8_t ConfigManager::liftSaturation = TurboliftConfig::Effects::DEFAULT_SATURATION;
uint8_t ConfigManager::liftBrightness = TurboliftConfig::Effects::DEFAULT_BRIGHTNESS;
uint8_t ConfigManager::effectMode = static_cast<uint8_t>(TurboliftConfig::Effects::EffectMode::SINGLE_COLOR);
