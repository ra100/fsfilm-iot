#pragma once

#include "effects.h"
#include "led_driver.h"
#include "config.h"
#include "config_manager.h"
#ifndef UNIT_TEST
#include <Arduino.h>
#include <math.h>
#else
// When running unit tests on host, provide a declaration for millis() with C linkage
extern "C" unsigned long millis();
#endif

#ifdef UNIT_TEST
// Provide small helpers to emulate Arduino behavior used in portal_effect
#include <cstdlib>
static inline int rnd(int max) { return rand() % max; }
static inline int rndRange(int a, int b) { return a + (rand() % (b - a)); }
static inline float rndf(int max) { return (float)(rand() % max); }
static inline float constrainf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }

// Simple CHSV -> CRGB, using hue only as index into a small palette approximation
static inline CRGB CHSV(uint8_t h, uint8_t s, uint8_t v)
{
  // naive mapping for tests: treat hue 0 -> red, 85 -> green, 170 -> blue
  if (h < 85)
    return CRGB(v, (uint8_t)((h * s) / 85), 0);
  if (h < 170)
    return CRGB((uint8_t)(((170 - h) * s) / 85), v, 0);
  return CRGB(0, (uint8_t)(((h - 170) * s) / 85), v);
}

// Provide Arduino-like random() overloads for host tests but avoid defining a
// function named `random` that conflicts with libc; use arduino_random and map
// the macro name `random` to it.
static inline long arduino_random(long max)
{
  if (max <= 0)
    return 0;
  return rand() % max;
}
static inline long arduino_random(long min, long max)
{
  if (max <= min)
    return min;
  return min + (rand() % (max - min));
}
#define random(...) arduino_random(__VA_ARGS__)

// constrain macro compatibility
#define constrain(x, a, b) (constrainf((x), (a), (b)))
#endif

// Template PortalEffect uses a driver and static buffers sized at compile time
template <int N, int GRADIENT_STEP, int GRADIENT_MOVE>
class PortalEffectTemplate
{
public:
  PortalEffectTemplate(ILEDDriver *driver) : _driver(driver)
  {
    NUM_LEDS = N;
    gradientPosition = 0;
    gradientPos1 = 0;
    gradientPos2 = 0;
    animationActive = false;
    fadeInActive = false;
    fadeInStart = 0;
    fadeOutActive = false;
    fadeOutStart = 0;
    malfunctionActive = false;
    lastUpdate = 0;
    numGradientPoints = 0;
    sequenceInitialized = false;
  }

  void begin()
  {
    _driver->begin();
    _leds = _driver->getBuffer();
    // effectLeds are static arrays
  }

  void setBrightness(uint8_t b) { _driver->setBrightness(b); }
  void fillSolid(const CRGB &c)
  {
    _driver->fillSolid(c);
    _driver->show();
  }
  void clear()
  {
    _driver->clear();
    _driver->show();
  }

  void start()
  {
    if (!animationActive)
    {
      animationActive = true;
      fadeInActive = true;
      fadeInStart = millis();
      gradientPosition = 0;
      generatePortalEffect(effectLeds);
    }
  }

  void stop()
  {
    animationActive = false;
    _driver->clear();
    _driver->show();
  }

  void triggerFadeOut()
  {
    if (!fadeOutActive && (animationActive || malfunctionActive))
    {
      fadeOutActive = true;
      fadeOutStart = millis();
      fadeInActive = false;
      animationActive = false;
      malfunctionActive = false;
    }
  }

  void triggerMalfunction()
  {
    if (!malfunctionActive)
    {
      malfunctionActive = true;
      animationActive = false;
    }
  }

  void update(unsigned long now)
  {
    if (fadeOutActive || malfunctionActive || animationActive)
    {
      if (now - lastUpdate >= 10)
      {
        if (animationActive && ConfigManager::needsEffectRegeneration())
        {
          if (ConfigManager::getPortalMode() == 0)
          {
            generatePortalEffect(effectLeds);
          }
          else
            generateVirtualGradients();
          ConfigManager::clearEffectRegenerationFlag();
        }

        int speed = ConfigManager::getRotationSpeed();
        if (ConfigManager::getPortalMode() == 0)
          gradientPosition = (gradientPosition + speed) % NUM_LEDS;
        else
        {
          // Move at half speed for virtual gradient effect
          // Ensure balanced speeds for wave effect
          gradientPos1 = (gradientPos1 + speed / 2) % NUM_LEDS;
          gradientPos2 = (gradientPos2 - speed / 2 + NUM_LEDS) % NUM_LEDS;
        }

        if (fadeOutActive || animationActive)
        {
          if (ConfigManager::getPortalMode() == 0)
            portalEffect();
          else
            virtualGradientEffect();
        }
        else if (malfunctionActive)
          portalMalfunctionEffect();
        lastUpdate = now;
      }
    }
  }

private:
  ILEDDriver *_driver;
  CRGB *_leds;
#ifdef UNIT_TEST
public:
  CRGB *testGeneratePortalEffect(CRGB *effectLeds) { return generatePortalEffect(effectLeds); }
  int testGetDriverIndex(int i) { return driverIndices[i]; }
  void testGenerateVirtualGradients() { generateVirtualGradients(); }
  CRGB *testGetSequence1() { return sequence1; }
  CRGB *testGetSequence2() { return sequence2; }
  bool testIsSequenceInitialized() { return sequenceInitialized; }
#endif
  CRGB effectLeds[N]; // Changed from static to instance storage
  int numGradientPoints;

  int NUM_LEDS;
  int gradientPosition;
  int gradientPos1;
  int gradientPos2;
  bool animationActive;
  bool fadeInActive;
  unsigned long fadeInStart;
  bool fadeOutActive;
  unsigned long fadeOutStart;
  bool malfunctionActive;
  unsigned long lastUpdate;
  // Virtual gradient sequences (instance storage)
  CRGB sequence1[PortalConfig::Hardware::NUM_LEDS];
  CRGB sequence2[PortalConfig::Hardware::NUM_LEDS];
  bool sequenceInitialized;

  void generateVirtualGradients()
  {
    // Generate and seed virtual gradient sequences used by virtualGradientEffect
    if (sequenceInitialized)
      return;

    // Create lambda functions for virtual gradient color generation
    auto colorGen1 = [](int driverIndex) -> CRGB
    {
      return CRGB(0, 0, 0);
    };

    auto colorGen2 = [](int driverIndex) -> CRGB
    {
      return CRGB(0, 0, 0);
    };

    // Actual generation happens below using the same logic as previously in virtualGradientEffect
    // Use local color generators that reference ConfigManager when placing drivers
    auto vg_colorGen1 = [](int driverIndex) -> CRGB
    {
      if (driverIndex % 3 == 0) // 1 color, 2 black pattern
      {
        uint8_t hue = ConfigManager::getHueMin();
        uint8_t sat = PortalConfig::Effects::PORTAL_SAT_BASE + random(PortalConfig::Effects::PORTAL_SAT_RANGE);
        uint8_t val = PortalConfig::Effects::PORTAL_VAL_BASE + random(PortalConfig::Effects::PORTAL_VAL_RANGE);
        return CHSV(hue, sat, val);
      }
      else
      {
        return CRGB(0, 0, 0); // Black for breaks
      }
    };

    auto vg_colorGen2 = [](int driverIndex) -> CRGB
    {
      if (driverIndex % 3 == 0) // 1 color, 2 black pattern
      {
        uint8_t hue = ConfigManager::getHueMax();
        uint8_t sat = PortalConfig::Effects::PORTAL_SAT_BASE + random(PortalConfig::Effects::PORTAL_SAT_RANGE);
        uint8_t val = PortalConfig::Effects::PORTAL_VAL_BASE + random(PortalConfig::Effects::PORTAL_VAL_RANGE);
        return CHSV(hue, sat, val);
      }
      else
      {
        return CRGB(0, 0, 0); // Black for breaks
      }
    };

    generatePortalEffect(sequence1, vg_colorGen1);
    generatePortalEffect(sequence2, vg_colorGen2);

    // Seed random once
    randomSeed(millis());

    sequenceInitialized = true;
  }

  CRGB getRandomDriverColorInternal()
  {
    // Handle hue range with wrap-around (e.g., min=250, max=10 for crossing 0/255)
    uint8_t hueMin = ConfigManager::getHueMin();
    uint8_t hueMax = ConfigManager::getHueMax();
    uint8_t length;
    if (hueMin <= hueMax)
    {
      length = hueMax - hueMin + 1;
    }
    else
    {
      length = 256 - hueMin + hueMax + 1;
    }
    uint8_t offset = random(length);
    uint8_t hue = (hueMin + offset) % 256;

    uint8_t sat = PortalConfig::Effects::PORTAL_SAT_BASE + random(PortalConfig::Effects::PORTAL_SAT_RANGE);
    if (random(PortalConfig::Effects::PORTAL_LOW_SAT_PROBABILITY) == 0)
      sat = PortalConfig::Effects::PORTAL_SAT_LOW_BASE + random(PortalConfig::Effects::PORTAL_SAT_LOW_RANGE);
    uint8_t val = PortalConfig::Effects::PORTAL_VAL_BASE + random(PortalConfig::Effects::PORTAL_VAL_RANGE);
    return CHSV(hue, sat, val);
  }

  CRGB *generateDriverColors(CRGB *driverColors, int &numDrivers, bool useBlackDrivers = false, uint8_t hue = 0)
  {
    const int minDist = PortalConfig::Effects::MIN_DRIVER_DISTANCE;
    const int maxDist = PortalConfig::Effects::MAX_DRIVER_DISTANCE;
    static int driverIndices[N];
    numDrivers = 0;
    int idx = 0;
    while (idx < NUM_LEDS - minDist && numDrivers < N - 1)
    {
      driverIndices[numDrivers] = idx;
      driverColors[numDrivers] = getRandomDriverColorInternal();
      numDrivers++;
      int step = minDist + random(maxDist - minDist + 1);
      if (idx + step > NUM_LEDS - minDist)
        break;
      idx += step;
    }
    driverIndices[numDrivers] = NUM_LEDS;
    driverColors[numDrivers] = driverColors[0];
    numDrivers++;

    if (useBlackDrivers)
    {
      for (int i = 0; i < numDrivers; i++)
      {
        if (i % 2 != 0)
        {
          driverColors[i] = CRGB(0, 0, 0); // Black for odd drivers
        }
        else
        {
          driverColors[i] = CHSV(hue,
                                 PortalConfig::Effects::PORTAL_SAT_BASE + random(PortalConfig::Effects::PORTAL_SAT_RANGE),
                                 PortalConfig::Effects::PORTAL_VAL_BASE + random(PortalConfig::Effects::PORTAL_VAL_RANGE));
        }
      }
    }

    return driverColors;
  }

  typedef CRGB (*DriverColorGenerator)(int driverIndex);

  void generatePortalEffect(CRGB *effectLeds, DriverColorGenerator colorGen = nullptr)
  {
    const int minDist = PortalConfig::Effects::MIN_DRIVER_DISTANCE;
    const int maxDist = PortalConfig::Effects::MAX_DRIVER_DISTANCE;
    int driverIndices[N];
    CRGB driverColors[N];
    int numDrivers = 0;
    int idx = 0;

    while (idx < NUM_LEDS - minDist && numDrivers < N - 1)
    {
      driverIndices[numDrivers] = idx;
      if (colorGen)
        driverColors[numDrivers] = colorGen(numDrivers);
      else
        driverColors[numDrivers] = getRandomDriverColorInternal();
      numDrivers++;
      int step = minDist + random(maxDist - minDist + 1);
      if (idx + step > NUM_LEDS - minDist)
        break;
      idx += step;
    }
    driverIndices[numDrivers] = NUM_LEDS;
    driverColors[numDrivers] = driverColors[0];
    numDrivers++;

    for (int d = 0; d < numDrivers - 1; d++)
    {
      int start = driverIndices[d];
      int end = driverIndices[d + 1];
      CRGB c1 = driverColors[d];
      CRGB c2 = driverColors[d + 1];
      int segLen = end - start;
      for (int i = 0; i < segLen; i++)
      {
        float ratio = (segLen == 1) ? 0.0f : (float)i / (segLen - 1);
        CRGB col = interpolateColor(c1, c2, ratio);
        int pos = start + i;
        if (pos >= 0 && pos < NUM_LEDS)
          effectLeds[pos] = col;
      }
    }
  }

  // Helper function for virtual gradient sequences with black drivers
  CRGB virtualGradientColorGen(int driverIndex, uint8_t hue)
  {
    if (driverIndex % 3 == 0) // 1 color, 2 black pattern
    {
      uint8_t sat = PortalConfig::Effects::PORTAL_SAT_BASE + random(PortalConfig::Effects::PORTAL_SAT_RANGE);
      uint8_t val = PortalConfig::Effects::PORTAL_VAL_BASE + random(PortalConfig::Effects::PORTAL_VAL_RANGE);
      return CHSV(hue, sat, val);
    }
    else
    {
      return CRGB(0, 0, 0); // Black for breaks
    }
  }

  void portalEffect()
  {
    float fadeScale = 1.0f;
    if (fadeInActive)
    {
      unsigned long now = millis();
      float t = (now - fadeInStart) / (float)PortalConfig::Timing::FADE_IN_DURATION_MS;
      fadeScale = constrain(t, 0.0f, 1.0f);
      if (fadeScale >= 1.0f)
      {
        fadeInActive = false;
        fadeScale = 1.0f;
      }
    }
    else if (fadeOutActive)
    {
      unsigned long now = millis();
      float t = (now - fadeOutStart) / (float)PortalConfig::Timing::FADE_OUT_DURATION_MS;
      fadeScale = 1.0f - constrain(t, 0.0f, 1.0f);
      if (fadeScale <= 0.0f)
      {
        fadeOutActive = false;
        fadeScale = 0.0f;
        animationActive = false;
        _driver->clear();
        _driver->show();
        return;
      }
    }
    for (int i = 0; i < NUM_LEDS; i++)
    {
      _driver->setPixel(i, effectLeds[(i + gradientPosition) % NUM_LEDS]);
      if (fadeScale < 1.0f)
        _driver->getBuffer()[i].nscale8((uint8_t)(fadeScale * 255));
    }
    _driver->setBrightness(ConfigManager::getMaxBrightness());
    _driver->show();
  }

  void portalMalfunctionEffect()
  {
    unsigned long now = millis();
    static unsigned long lastJump = 0;
    static float targetBrightness = 1.0f;
    static float currentBrightness = 1.0f;
    static int jumpInterval = 100;
    gradientPosition = (gradientPosition + GRADIENT_MOVE) % NUM_LEDS;

    if (now - lastJump > (unsigned long)jumpInterval)
    {
      targetBrightness = PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_MIN +
                         PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_RANGE * (random(1000) / 1000.0f);
      jumpInterval = PortalConfig::Timing::MALFUNCTION_MIN_JUMP_MS +
                     random(PortalConfig::Timing::MALFUNCTION_MAX_JUMP_MS - PortalConfig::Timing::MALFUNCTION_MIN_JUMP_MS);
      lastJump = now;
    }
    float delta = targetBrightness - currentBrightness;
    currentBrightness += delta * (PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_SMOOTHING_MIN +
                                  PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_SMOOTHING_RANGE * (random(1000) / 1000.0f));
    currentBrightness += (random(-PortalConfig::Effects::MALFUNCTION_NOISE_OFFSET, PortalConfig::Effects::MALFUNCTION_NOISE_OFFSET + 1)) / 255.0f;
    currentBrightness = constrain(currentBrightness,
                                  PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_CLAMP_MIN,
                                  PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_CLAMP_MAX);

    for (int i = 0; i < NUM_LEDS; i++)
    {
      _driver->setPixel(i, effectLeds[(i + gradientPosition) % NUM_LEDS]);
      _driver->getBuffer()[i].nscale8((uint8_t)(currentBrightness * PortalConfig::Effects::MALFUNCTION_BASE_BRIGHTNESS + PortalConfig::Effects::MALFUNCTION_BRIGHTNESS_OFFSET));
    }
    _driver->show();
  }

  void virtualGradientEffect()
  {
    float fadeScale = 1.0f;
    if (fadeInActive)
    {
      unsigned long now = millis();
      float t = (now - fadeInStart) / (float)PortalConfig::Timing::FADE_IN_DURATION_MS;
      fadeScale = constrain(t, 0.0f, 1.0f);
      if (fadeScale >= 1.0f)
      {
        fadeInActive = false;
        fadeScale = 1.0f;
      }
    }
    else if (fadeOutActive)
    {
      unsigned long now = millis();
      float t = (now - fadeOutStart) / (float)PortalConfig::Timing::FADE_OUT_DURATION_MS;
      fadeScale = 1.0f - constrain(t, 0.0f, 1.0f);
      if (fadeScale <= 0.0f)
      {
        fadeOutActive = false;
        fadeScale = 0.0f;
        animationActive = false;
        _driver->clear();
        _driver->show();
        return;
      }
    }

    uint8_t hue1 = ConfigManager::getHueMin();
    uint8_t hue2 = ConfigManager::getHueMax();

    // Ensure virtual sequences are generated
    if (!sequenceInitialized)
      generateVirtualGradients();

    for (int i = 0; i < PortalConfig::Hardware::NUM_LEDS; i++)
    {
      // Gradient 1: clockwise rotation
      int pos1 = (i + gradientPos1) % PortalConfig::Hardware::NUM_LEDS;
      uint8_t bright1 = sequence1[pos1].b;

      // Interpolate between drivers for sequence 1
      int nextDriver1 = (pos1 + 10) % PortalConfig::Hardware::NUM_LEDS;
      while (sequence1[nextDriver1] == 0 && nextDriver1 != pos1)
      {
        nextDriver1 = (nextDriver1 + 1) % PortalConfig::Hardware::NUM_LEDS;
      }

      if (nextDriver1 != pos1)
      {
        int dist1 = (nextDriver1 - pos1 + PortalConfig::Hardware::NUM_LEDS) % PortalConfig::Hardware::NUM_LEDS;
        if (dist1 > PortalConfig::Hardware::NUM_LEDS / 2)
        {
          dist1 = PortalConfig::Hardware::NUM_LEDS - dist1;
        }

        float ratio1 = (float)(i - pos1 + PortalConfig::Hardware::NUM_LEDS) / dist1;
        bright1 = (sequence1[pos1].b * (1.0f - ratio1) + sequence1[nextDriver1].b * ratio1);
      }

      CRGB color1 = CHSV(hue1, 255, bright1);

      // Gradient 2: counterclockwise rotation
      int pos2 = (i + gradientPos2) % PortalConfig::Hardware::NUM_LEDS;
      uint8_t bright2 = sequence2[pos2].b;

      // Interpolate between drivers for sequence 2
      int nextDriver2 = (pos2 + 10 + PortalConfig::Hardware::NUM_LEDS) % PortalConfig::Hardware::NUM_LEDS;
      while (sequence2[nextDriver2] == 0 && nextDriver2 != pos2)
      {
        nextDriver2 = (nextDriver2 - 1 + PortalConfig::Hardware::NUM_LEDS) % PortalConfig::Hardware::NUM_LEDS;
      }

      if (nextDriver2 != pos2)
      {
        int dist2 = (pos2 - nextDriver2 + PortalConfig::Hardware::NUM_LEDS) % PortalConfig::Hardware::NUM_LEDS;
        if (dist2 > PortalConfig::Hardware::NUM_LEDS / 2)
        {
          dist2 = PortalConfig::Hardware::NUM_LEDS - dist2;
        }

        float ratio2 = (float)(i - pos2 + PortalConfig::Hardware::NUM_LEDS) / dist2;
        bright2 = (sequence2[pos2].b * (1.0f - ratio2) + sequence2[nextDriver2].b * ratio2);
      }

      CRGB color2 = CHSV(hue2, 255, bright2);

      // Take the whole value of the LED from the sequence with higher brightness
      CRGB blended;
      if (bright1 > bright2)
      {
        blended = color1;
      }
      else
      {
        blended = color2;
      }

      _driver->setPixel(i, blended);
      if (fadeScale < 1.0f)
        _driver->getBuffer()[i].nscale8((uint8_t)(fadeScale * 255));
    }

    _driver->setBrightness(ConfigManager::getMaxBrightness());
    _driver->show();
  }
};

// Static storage definitions removed - now using instance storage
// This eliminates the critical bug where multiple instances would share the same buffers
