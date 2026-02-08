#define UNIT_TEST
#include "mock_led_driver.h"
#include "../src/turbolift_effect.h"
#include <cassert>
#include <iostream>

// Simulated millis() for unit tests
static unsigned long simulated_time = 0;
extern "C" unsigned long millis() { return simulated_time; }

// Simple CRGB subtraction and multiplication for testing
CRGB crgbSubtractAndScale(const CRGB &c1, const CRGB &c2, float scale)
{
  return CRGB(
      (uint8_t)(c1.r + (c2.r - c1.r) * scale),
      (uint8_t)(c1.g + (c2.g - c1.g) * scale),
      (uint8_t)(c1.b + (c2.b - c1.b) * scale));
}

int main()
{
  // Use small N for native test
  const int N = 32;
  MockLEDDriver<N> mock;
  TurboliftEffectTemplate<N, 4, 1> turbolift(&mock);

  turbolift.begin();
  turbolift.setBrightness(128);
  turbolift.fillSolid(CRGB::Red());
  // verify buffer filled with red
  for (int i = 0; i < N; ++i)
    assert(mock.buffer[i].r == 255 && mock.buffer[i].g == 0 && mock.buffer[i].b == 0);

  turbolift.clear();
  for (int i = 0; i < N; ++i)
    assert(mock.buffer[i].r == 0 && mock.buffer[i].g == 0 && mock.buffer[i].b == 0);

  std::cout << "Testing generateTurboliftEffect()" << std::endl;
  CRGB testBuffer[N];
  CRGB *result = turbolift.testGenerateTurboliftEffect(testBuffer);
  assert(result == testBuffer);

  // Check gradient generation
  int numDrivers = 0;
  CRGB driverColors[N];
  turbolift.generateDriverColors(driverColors, numDrivers);

  for (int d = 0; d < numDrivers - 1; d++)
  {
    int start = turbolift.testGetDriverIndex(d);
    int end = turbolift.testGetDriverIndex(d + 1);
    CRGB c1 = driverColors[d];
    CRGB c2 = driverColors[d + 1];

    for (int i = start; i < end; i++)
    {
      float ratio = (float)(i - start) / (end - start);
      CRGB expectedColor = c1 + (c2 - c1) * ratio;
      assert(testBuffer[i].r == expectedColor.r);
      assert(testBuffer[i].g == expectedColor.g);
      assert(testBuffer[i].b == expectedColor.b);
    }
  }
  // Start turbolift and run a few updates to ensure no crashes
  turbolift.start();
  unsigned long t = 0;
  for (int k = 0; k < 10; ++k)
  {
    t += 50;
    turbolift.update(t);
  }

  std::cout << "Turbolift native test passed" << std::endl;
  return 0;
}
