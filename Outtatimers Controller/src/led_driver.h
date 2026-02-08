#pragma once

#include "config.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_rom_sys.h>
#include <driver/rmt_tx.h>
#include <driver/rmt_encoder.h>

// LED driver interface to allow for future testing and flexibility
class ILEDDriver
{
public:
  virtual void begin() = 0;
  virtual void setBrightness(uint8_t b) = 0;
  virtual void setPixel(int idx, uint32_t color) = 0;
  virtual void fillSolid(uint32_t color) = 0;
  virtual void clear() = 0;
  virtual void show() = 0;
  virtual uint32_t Color(uint8_t r, uint8_t g, uint8_t b) = 0;
  virtual uint32_t ColorHSV(uint16_t hue, uint8_t sat = 255, uint8_t val = 255) = 0;
  virtual ~ILEDDriver() {}
};

// ESP-IDF RMT-based LED strip driver using official LED strip component
class RmtLedDriver : public ILEDDriver
{
public:
  RmtLedDriver(uint8_t pin = ControllerConfig::Hardware::LED_PIN, uint16_t numPixels = ControllerConfig::Hardware::NUM_LEDS)
      : pin_(pin), numPixels_(numPixels), brightness_(ControllerConfig::Hardware::DEFAULT_BRIGHTNESS), tx_channel_(nullptr), led_encoder_(nullptr) {}

  void begin() override;
  void setBrightness(uint8_t b) override { brightness_ = b; }
  void setPixel(int idx, uint32_t color) override;
  void fillSolid(uint32_t color) override;
  void clear() override;
  void show() override;
  uint32_t Color(uint8_t r, uint8_t g, uint8_t b) override;
  uint32_t ColorHSV(uint16_t hue, uint8_t sat = 255, uint8_t val = 255) override;
  ~RmtLedDriver();

private:
  uint8_t pin_;
  uint16_t numPixels_;
  uint8_t brightness_;
  rmt_channel_handle_t tx_channel_;
  rmt_encoder_handle_t led_encoder_;
  uint32_t pixelBuffer_[ControllerConfig::Hardware::NUM_LEDS];

  uint32_t applyBrightness(uint32_t color);
  static constexpr const char *TAG = "RmtLedDriver";
};

// Simple ESP-IDF GPIO-based NeoPixel driver
class NeoPixelDriver : public ILEDDriver
{
public:
  NeoPixelDriver(uint8_t pin = ControllerConfig::Hardware::LED_PIN, uint16_t numPixels = ControllerConfig::Hardware::NUM_LEDS)
      : pin_(pin), numPixels_(numPixels), brightness_(ControllerConfig::Hardware::DEFAULT_BRIGHTNESS) {}

  void begin() override;
  void setBrightness(uint8_t b) override { brightness_ = b; }
  void setPixel(int idx, uint32_t color) override;
  void fillSolid(uint32_t color) override;
  void clear() override;
  void show() override;
  uint32_t Color(uint8_t r, uint8_t g, uint8_t b) override;
  uint32_t ColorHSV(uint16_t hue, uint8_t sat = 255, uint8_t val = 255) override;

private:
  uint8_t pin_;
  uint16_t numPixels_;
  uint8_t brightness_;
  uint32_t pixelBuffer_[ControllerConfig::Hardware::NUM_LEDS];

  void sendPixel(uint32_t pixel);
  void sendByte(uint8_t byte);
  uint32_t applyBrightness(uint32_t color);
  static constexpr const char *TAG = "NeoPixelDriver";
};
