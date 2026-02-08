#pragma once

#include "config.h"

// When building unit tests on the host, FastLED is not available. Provide a
// minimal CRGB type and avoid including FastLED.h. For device builds, include
// FastLED and provide the actual driver implementation.
#ifdef UNIT_TEST
#include "effects.h"

// ILEDDriver interface for tests
class ILEDDriver
{
public:
  virtual void begin() = 0;
  virtual void setBrightness(uint8_t b) = 0;
  virtual void setPixel(int idx, const CRGB &color) = 0;
  virtual void fillSolid(const CRGB &color) = 0;
  virtual void clear() = 0;
  virtual void show() = 0;
  virtual CRGB *getBuffer() = 0;
  virtual ~ILEDDriver() {}
};

#else

#include <FastLED.h>

// LED driver interface to allow mocking in tests
class ILEDDriver
{
public:
  virtual void begin() = 0;
  virtual void setBrightness(uint8_t b) = 0;
  virtual void setPixel(int idx, const CRGB &color) = 0;
  virtual void fillSolid(const CRGB &color) = 0;
  virtual void clear() = 0;
  virtual void show() = 0;
  virtual CRGB *getBuffer() = 0;
  virtual ~ILEDDriver() {}
};

// FastLED-backed driver owning a static buffer of size N
template <int N>
class FastLEDDriver : public ILEDDriver
{
public:
  FastLEDDriver(uint8_t pin = TurboliftConfig::Hardware::LED_PIN) : _pin(pin) {}
  void begin() override
  {
    FastLED.addLeds<WS2812B, TurboliftConfig::Hardware::LED_PIN, COLOR_ORDER>(buffer, N);
    FastLED.setBrightness(255);
    FastLED.clear();
    FastLED.show();
  }
  void setBrightness(uint8_t b) override { FastLED.setBrightness(b); }
  void setPixel(int idx, const CRGB &color) override
  {
    if (idx >= 0 && idx < N)
      buffer[idx] = color;
  }
  void fillSolid(const CRGB &color) override { fill_solid(buffer, N, color); }
  void clear() override { FastLED.clear(); }
  void show() override { FastLED.show(); }
  CRGB *getBuffer() override { return buffer; }

private:
  uint8_t _pin;
  static CRGB buffer[N];
};

// Define static storage
template <int N>
CRGB FastLEDDriver<N>::buffer[N];

#endif
