#pragma once

#include "config.h"
#include <FastLED.h>

// LED driver interface to allow for future testing and flexibility
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

// FastLED-backed driver for Controller with static buffer of size N
template <int N>
class FastLEDDriver : public ILEDDriver
{
public:
  FastLEDDriver(uint8_t pin = ControllerConfig::Hardware::LED_PIN) : _pin(pin) {}

  void begin() override
  {
    FastLED.addLeds<LED_TYPE, ControllerConfig::Hardware::LED_PIN, COLOR_ORDER>(buffer, N);
    FastLED.setBrightness(ControllerConfig::Hardware::DEFAULT_BRIGHTNESS);
    FastLED.clear();
    FastLED.show();
  }

  void setBrightness(uint8_t b) override
  {
    FastLED.setBrightness(b);
  }

  void setPixel(int idx, const CRGB &color) override
  {
    if (idx >= 0 && idx < N)
      buffer[idx] = color;
  }

  void fillSolid(const CRGB &color) override
  {
    fill_solid(buffer, N, color);
  }

  void clear() override
  {
    FastLED.clear();
  }

  void show() override
  {
    FastLED.show();
  }

  CRGB *getBuffer() override
  {
    return buffer;
  }

private:
  uint8_t _pin;
  static CRGB buffer[N];
};

// Define static storage
template <int N>
CRGB FastLEDDriver<N>::buffer[N];