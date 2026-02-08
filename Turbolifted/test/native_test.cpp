#include <iostream>
#include <cmath>
#include <cassert>
#include "../src/effects.h"

int main()
{
  // Test interpolateColor midpoint
  CRGB a(0, 0, 0);
  CRGB b(255, 255, 255);
  CRGB mid = interpolateColor(a, b, 0.5f);
  assert(std::abs((int)mid.r - 127) <= 1);
  assert(std::abs((int)mid.g - 127) <= 1);
  assert(std::abs((int)mid.b - 127) <= 1);

  // Test getLEDPosition and distance
  float x, y;
  int numLeds = 100;
  float radius = 10.0f;
  bool result = getLEDPosition(0, numLeds, radius, x, y);
  assert(result == true);
  // LED 0 should be at (radius, 0)
  assert(fabs(x - radius) < 1e-3f);
  assert(fabs(y - 0.0f) < 1e-3f);
  float d = getCircleDistance(x, y);
  assert(fabs(d - radius) < 1e-3f);

  // Test invalid parameters
  assert(getLEDPosition(-1, numLeds, radius, x, y) == false);
  assert(getLEDPosition(0, 0, radius, x, y) == false);
  assert(getLEDPosition(0, numLeds, -1.0f, x, y) == false);

  std::cout << "All native tests passed\n";
  return 0;
}
