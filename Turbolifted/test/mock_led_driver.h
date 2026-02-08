// Mock ILEDDriver for native tests
#pragma once
#include "../src/led_driver.h"
#ifdef UNIT_TEST
#include <vector>
using std::vector;

template <int N>
class MockLEDDriver : public ILEDDriver
{
public:
  MockLEDDriver(int pin = 0) {}
  void begin() override {}
  CRGB *getBuffer() override { return buffer; }
  void show() override {}
  void setBrightness(uint8_t b) override { brightness = b; }
  void fillSolid(const CRGB &c) override
  {
    for (int i = 0; i < N; ++i)
      buffer[i] = c;
  }
  void clear() override
  {
    for (int i = 0; i < N; ++i)
      buffer[i] = CRGB();
  }
  void setPixel(int i, const CRGB &c) override
  {
    if (i >= 0 && i < N)
      buffer[i] = c;
  }

  CRGB buffer[N];
  uint8_t brightness = 255;
};
#endif
