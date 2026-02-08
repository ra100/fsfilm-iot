#include "effects.h"
#include "config.h"
#include <algorithm>

bool getLEDPosition(int ledIndex, int numLeds, float radius, float &x, float &y)
{
  // Input validation
  if (ledIndex < 0 || numLeds <= 0 || radius <= 0.0f)
  {
    return false;
  }
  if (ledIndex >= numLeds)
  {
    return false;
  }

  float angle = (2.0f * TurboliftConfig::Math::PI_F * ledIndex) / numLeds;
  x = radius * cosf(angle);
  y = radius * sinf(angle);
  return true;
}

float getCircleDistance(float x, float y)
{
  return sqrtf(x * x + y * y);
}

CRGB interpolateColor(const CRGB &c1, const CRGB &c2, float ratio)
{
  // Clamp ratio to valid range using standard library
  ratio = std::clamp(ratio, 0.0f, 1.0f);

  uint8_t r = (uint8_t)(c1.r + (c2.r - c1.r) * ratio);
  uint8_t g = (uint8_t)(c1.g + (c2.g - c1.g) * ratio);
  uint8_t b = (uint8_t)(c1.b + (c2.b - c1.b) * ratio);
  return CRGB(r, g, b);
}
