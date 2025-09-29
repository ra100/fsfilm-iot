#include "config_manager.h"

int ConfigManager::rotationSpeed = 2;
uint8_t ConfigManager::maxBrightness = 255;
uint8_t ConfigManager::hueMin = 160;
uint8_t ConfigManager::hueMax = 200;
uint8_t ConfigManager::satMin = 128;
uint8_t ConfigManager::satMax = 255;
bool ConfigManager::effectNeedsRegeneration = false;
int ConfigManager::portalMode = 0;