#pragma once

#include <stdint.h>
#include <Arduino.h>
#include "config.h"

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
    // Initialize default values for legacy gradient effects
    rotationSpeed = 2;   // Default gradient move speed
    maxBrightness = 255; // Default max brightness
    hueMin = 160;        // Default minimum hue (blue)
    hueMax = 200;        // Default maximum hue (purple)
    satMin = 128;        // Default minimum saturation
    satMax = 255;        // Default maximum saturation
    turboliftMode = 0;   // Default to classic mode
    effectNeedsRegeneration = false;

    // Initialize new lift animation parameters with defaults from config.h
    liftSpeed = TurboliftConfig::Effects::DEFAULT_SPEED;
    liftWidth = TurboliftConfig::Effects::DEFAULT_WIDTH;
    liftSpacing = TurboliftConfig::Effects::DEFAULT_SPACING;
    liftHue = TurboliftConfig::Effects::DEFAULT_HUE;
    liftSaturation = TurboliftConfig::Effects::DEFAULT_SATURATION;
    liftBrightness = TurboliftConfig::Effects::DEFAULT_BRIGHTNESS;
    effectMode = static_cast<uint8_t>(TurboliftConfig::Effects::EffectMode::SINGLE_COLOR);
  }

  // =====================================================
  // LEGACY GRADIENT EFFECT PARAMETERS
  // =====================================================

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
   * @brief Get the current turbolift mode
   * @return Turbolift mode (0: classic, 1: virtual gradients)
   */
  static int getTurboliftMode()
  {
    return turboliftMode;
  }

  /**
   * @brief Set the turbolift mode
   * @param mode Turbolift mode (0: classic, 1: virtual gradients)
   */
  static void setTurboliftMode(int mode)
  {
    turboliftMode = constrain(mode, 0, 1);
    effectNeedsRegeneration = true;
  }

  // =====================================================
  // NEW LIFT ANIMATION PARAMETERS
  // =====================================================

  /**
   * @brief Get the lift animation speed
   * @return Speed (0-10 scale)
   */
  static uint8_t getLiftSpeed()
  {
    return liftSpeed;
  }

  /**
   * @brief Set the lift animation speed
   * @param speed Speed (0-10 scale, maps to delay per LED)
   */
  static void setLiftSpeed(uint8_t speed)
  {
    liftSpeed = constrain(speed, 0, 10);
  }

  /**
   * @brief Get the light beam width
   * @return Width in LEDs (1-20)
   */
  static uint8_t getLiftWidth()
  {
    return liftWidth;
  }

  /**
   * @brief Set the light beam width
   * @param width Width in LEDs (1-20)
   */
  static void setLiftWidth(uint8_t width)
  {
    liftWidth = constrain(width, 1, 20);
  }

  /**
   * @brief Get the spacing between beam packets
   * @return Spacing in LEDs (0-50)
   */
  static uint8_t getLiftSpacing()
  {
    return liftSpacing;
  }

  /**
   * @brief Set the spacing between beam packets
   * @param spacing Spacing in LEDs (0-50)
   */
  static void setLiftSpacing(uint8_t spacing)
  {
    liftSpacing = constrain(spacing, 0, 50);
  }

  /**
   * @brief Get the lift animation hue
   * @return Hue value (0-255)
   */
  static uint8_t getLiftHue()
  {
    return liftHue;
  }

  /**
   * @brief Set the lift animation hue
   * @param hue Hue value (0-255)
   */
  static void setLiftHue(uint8_t hue)
  {
    liftHue = hue;
  }

  /**
   * @brief Get the lift animation saturation
   * @return Saturation value (0-255)
   */
  static uint8_t getLiftSaturation()
  {
    return liftSaturation;
  }

  /**
   * @brief Set the lift animation saturation
   * @param saturation Saturation value (0-255)
   */
  static void setLiftSaturation(uint8_t saturation)
  {
    liftSaturation = saturation;
  }

  /**
   * @brief Get the lift animation brightness
   * @return Brightness value (0-255)
   */
  static uint8_t getLiftBrightness()
  {
    return liftBrightness;
  }

  /**
   * @brief Set the lift animation brightness
   * @param brightness Brightness value (0-255)
   */
  static void setLiftBrightness(uint8_t brightness)
  {
    liftBrightness = constrain(brightness, 0, 255);
  }

  /**
   * @brief Get the current effect mode
   * @return Effect mode (0: single color, 1: lift animation, 2: classic, 3: virtual gradient)
   */
  static uint8_t getEffectMode()
  {
    return effectMode;
  }

  /**
   * @brief Set the current effect mode
   * @param mode Effect mode (0: single color, 1: lift animation, 2: classic, 3: virtual gradient)
   */
  static void setEffectMode(uint8_t mode)
  {
    effectMode = constrain(mode, 0, 3);
    effectNeedsRegeneration = true;
  }

  /**
   * @brief Convert speed (0-10) to delay in milliseconds per LED
   * @param speed Speed value (0-10)
   * @return Delay in milliseconds
   */
  static unsigned long speedToDelay(uint8_t speed)
  {
    // Linear interpolation: speed 0 = 100ms, speed 10 = 5ms
    // delay = MIN_DELAY - (speed / 10) * (MIN_DELAY - MAX_DELAY)
    return TurboliftConfig::Effects::SPEED_MIN_DELAY_MS - 
           (speed * (TurboliftConfig::Effects::SPEED_MIN_DELAY_MS - TurboliftConfig::Effects::SPEED_MAX_DELAY_MS) / 10);
  }

private:
  // Legacy gradient effect parameters
  static int rotationSpeed;
  static uint8_t maxBrightness;
  static uint8_t hueMin;
  static uint8_t hueMax;
  static uint8_t satMin;
  static uint8_t satMax;
  static bool effectNeedsRegeneration;
  static int turboliftMode;

  // New lift animation parameters
  static uint8_t liftSpeed;      // Animation speed (0-10)
  static uint8_t liftWidth;      // Beam width in LEDs (1-20)
  static uint8_t liftSpacing;    // Gap between beams (0-50)
  static uint8_t liftHue;        // Beam color hue (0-255)
  static uint8_t liftSaturation; // Beam color saturation (0-255)
  static uint8_t liftBrightness; // Overall brightness (0-255)
  static uint8_t effectMode;     // Current effect mode
};
