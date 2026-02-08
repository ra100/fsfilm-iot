#define UNIT_TEST
#include "mock_led_driver.h"
#include "../src/turbolift_effect.h"
#include <cassert>
#include <iostream>

// Simulated millis() for unit tests
static unsigned long simulated_time = 12345;
extern "C" unsigned long millis() { return simulated_time; }

int main()
{
  const int N = 32;
  MockLEDDriver<N> mock;
  TurboliftEffectTemplate<N, 4, 1> turbolift(&mock);

  turbolift.begin();

  // Initially sequences should not be initialized
  assert(!turbolift.testIsSequenceInitialized());

  // Trigger generation
  turbolift.testGenerateVirtualGradients();
  assert(turbolift.testIsSequenceInitialized());

  CRGB *s1 = turbolift.testGetSequence1();
  CRGB *s2 = turbolift.testGetSequence2();
  // Ensure at least one non-black driver exists in each sequence
  bool s1_has_color = false;
  bool s2_has_color = false;
  for (int i = 0; i < TurboliftConfig::Hardware::NUM_LEDS && i < N; ++i)
  {
    if (!(s1[i].r == 0 && s1[i].g == 0 && s1[i].b == 0))
      s1_has_color = true;
    if (!(s2[i].r == 0 && s2[i].g == 0 && s2[i].b == 0))
      s2_has_color = true;
  }
  assert(s1_has_color && s2_has_color);

  // Run virtualGradientEffect to ensure it uses sequences without crashes
  turbolift.start();
  // Set turbolift mode to virtual gradients
  ConfigManager::setTurboliftMode(1);
  // Do a few updates
  unsigned long t = 0;
  for (int k = 0; k < 5; ++k)
  {
    t += 50;
    turbolift.update(t);
  }

  std::cout << "Virtual gradient generation test passed" << std::endl;
  return 0;
}
