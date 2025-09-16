#define UNIT_TEST
#include "mock_led_driver.h"
#include "../src/portal_effect.h"
#include <cassert>
#include <iostream>

// Simulated millis() for unit tests
static unsigned long simulated_time = 12345;
extern "C" unsigned long millis() { return simulated_time; }

int main()
{
  const int N = 32;
  MockLEDDriver<N> mock;
  PortalEffectTemplate<N, 4, 1> portal(&mock);

  portal.begin();

  // Initially sequences should not be initialized
  assert(!portal.testIsSequenceInitialized());

  // Trigger generation
  portal.testGenerateVirtualGradients();
  assert(portal.testIsSequenceInitialized());

  CRGB *s1 = portal.testGetSequence1();
  CRGB *s2 = portal.testGetSequence2();
  // Ensure at least one non-black driver exists in each sequence
  bool s1_has_color = false;
  bool s2_has_color = false;
  for (int i = 0; i < PortalConfig::Hardware::NUM_LEDS && i < N; ++i)
  {
    if (!(s1[i].r == 0 && s1[i].g == 0 && s1[i].b == 0))
      s1_has_color = true;
    if (!(s2[i].r == 0 && s2[i].g == 0 && s2[i].b == 0))
      s2_has_color = true;
  }
  assert(s1_has_color && s2_has_color);

  // Run virtualGradientEffect to ensure it uses sequences without crashes
  portal.start();
  // Set portal mode to virtual gradients
  ConfigManager::setPortalMode(1);
  // Do a few updates
  unsigned long t = 0;
  for (int k = 0; k < 5; ++k)
  {
    t += 50;
    portal.update(t);
  }

  std::cout << "Virtual gradient generation test passed" << std::endl;
  return 0;
}
