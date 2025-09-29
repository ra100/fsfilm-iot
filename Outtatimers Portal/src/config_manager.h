#pragma once

#include <stdint.h>
#include <Arduino.h>

/**
 * @brief Configuration manager for runtime parameters
 *
 * This class provides a simple way to store and retrieve configuration
 * parameters that can be modified at runtime.
 */
class ConfigManager
{
public:
  /**
   * @brief Initialize the configuration manager
   */
  static void begin()
  {
    // Initialize default values
    rotationSpeed = 2;   // Default gradient move speed
    maxBrightness = 255; // Default max brightness
    hueMin = 160;        // Default minimum hue (blue)
    hueMax = 200;        // Default maximum hue (purple)
    satMin = 128;        // Default minimum saturation
    satMax = 255;        // Default maximum saturation
    portalMode = 0;      // Default to classic mode
    effectNeedsRegeneration = false;
  }

  /**
   * @brief Get the current rotation speed (gradient move value)
   * @return Rotation speed (0-10)
   */
  static int getRotationSpeed()
  {
    return rotationSpeed;
  }

  /**
   * @brief Set the rotation speed (gradient move value)
   * @param speed Rotation speed (0-10)
   */
  static void setRotationSpeed(int speed)
  {
    rotationSpeed = constrain(speed, 0, 10);
  }

  /**
   * @brief Get the current max brightness
   * @return Max brightness (0-255)
   */
  static uint8_t getMaxBrightness()
  {
    return maxBrightness;
  }

  /**
   * @brief Set the max brightness
   * @param brightness Max brightness (0-255)
   */
  static void setMaxBrightness(uint8_t brightness)
  {
    maxBrightness = constrain(brightness, 0, 255);
  }

  /**
   * @brief Get the minimum hue value
   * @return Minimum hue (0-255)
   */
  static uint8_t getHueMin()
  {
    return hueMin;
  }

  /**
   * @brief Set the minimum hue value
   * @param minHue Minimum hue (0-255)
   */
  static void setHueMin(uint8_t minHue)
  {
    hueMin = constrain(minHue, 0, 255);
    effectNeedsRegeneration = true;
  }

  /**
   * @brief Get the maximum hue value
   * @return Maximum hue (0-255)
   */
  static uint8_t getHueMax()
  {
    return hueMax;
  }

  /**
   * @brief Set the maximum hue value
   * @param maxHue Maximum hue (0-255)
   */
  static void setHueMax(uint8_t maxHue)
  {
    hueMax = constrain(maxHue, 0, 255);
    effectNeedsRegeneration = true;
  }

  /**
   * @brief Get the minimum saturation value
   * @return Minimum saturation (0-255)
   */
  static uint8_t getSatMin()
  {
    return satMin;
  }

  /**
   * @brief Set the minimum saturation value
   * @param minSat Minimum saturation (0-255)
   */
  static void setSatMin(uint8_t minSat)
  {
    satMin = constrain(minSat, 0, 255);
    effectNeedsRegeneration = true;
  }

  /**
   * @brief Get the maximum saturation value
   * @return Maximum saturation (0-255)
   */
  static uint8_t getSatMax()
  {
    return satMax;
  }

  /**
   * @brief Set the maximum saturation value
   * @param maxSat Maximum saturation (0-255)
   */
  static void setSatMax(uint8_t maxSat)
  {
    satMax = constrain(maxSat, 0, 255);
    effectNeedsRegeneration = true;
  }

  /**
   * @brief Check if effect regeneration is needed
   * @return true if regeneration is required
   */
  static bool needsEffectRegeneration()
  {
    return effectNeedsRegeneration;
  }

  /**
   * @brief Clear the effect regeneration flag
   */
  static void clearEffectRegenerationFlag()
  {
    effectNeedsRegeneration = false;
  }

  /**
   * @brief Get the current portal mode
   * @return Portal mode (0: classic, 1: virtual gradients)
   */
  static int getPortalMode()
  {
    return portalMode;
  }

  /**
   * @brief Set the portal mode
   * @param mode Portal mode (0: classic, 1: virtual gradients)
   */
  static void setPortalMode(int mode)
  {
    portalMode = constrain(mode, 0, 1);
    effectNeedsRegeneration = true;
  }

private:
  static int rotationSpeed;
  static uint8_t maxBrightness;
  static uint8_t hueMin;
  static uint8_t hueMax;
  static uint8_t satMin;
  static uint8_t satMax;
  static bool effectNeedsRegeneration;
  static int portalMode;
};