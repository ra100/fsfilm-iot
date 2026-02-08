// Lightweight effects helpers made testable
#pragma once

#include <stdint.h>
#include <math.h>

#ifdef UNIT_TEST
// Provide a minimal CRGB struct for unit tests
struct CRGB
{
  uint8_t r, g, b;
  CRGB() : r(0), g(0), b(0) {}
  CRGB(uint8_t rr, uint8_t gg, uint8_t bb) : r(rr), g(gg), b(bb) {}
  void nscale8(uint8_t scale)
  {
    r = (uint8_t)((r * scale) / 255);
    g = (uint8_t)((g * scale) / 255);
    b = (uint8_t)((b * scale) / 255);
  }
  static CRGB Red() { return CRGB(255, 0, 0); }
  static CRGB Green() { return CRGB(0, 255, 0); }
  static CRGB Blue() { return CRGB(0, 0, 255); }
};
#else
#include <FastLED.h>
#endif

/**
 * @brief Convert LED index to circular coordinates
 * @param ledIndex Index of the LED (0-based)
 * @param numLeds Total number of LEDs in the strip
 * @param radius Radius of the circular arrangement
 * @param x Output parameter for X coordinate
 * @param y Output parameter for Y coordinate
 * @return true if conversion successful, false if invalid parameters
 */
bool getLEDPosition(int ledIndex, int numLeds, float radius, float &x, float &y);

/**
 * @brief Calculate distance from LED to circle center
 * @param x X coordinate
 * @param y Y coordinate
 * @return Distance from origin
 */
float getCircleDistance(float x, float y);

/**
 * @brief Linear interpolate between two CRGB colors
 * @param c1 First color
 * @param c2 Second color
 * @param ratio Interpolation ratio (0.0 = c1, 1.0 = c2, automatically clamped)
 * @return Interpolated color
 */
CRGB interpolateColor(const CRGB &c1, const CRGB &c2, float ratio);
