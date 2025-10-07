#pragma once

#include "config.h"
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_rom_sys.h>

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

// Implementation of NeoPixelDriver methods

void NeoPixelDriver::begin()
{
  ESP_LOGI(TAG, "Initializing NeoPixel driver on pin %d with %d LEDs", pin_, numPixels_);

  // Configure GPIO for output
  gpio_reset_pin((gpio_num_t)pin_);
  gpio_set_direction((gpio_num_t)pin_, GPIO_MODE_OUTPUT);

  // Clear all pixels
  clear();
  show();

  ESP_LOGI(TAG, "NeoPixel driver initialized");
}

void NeoPixelDriver::setPixel(int idx, uint32_t color)
{
  if (idx >= 0 && idx < numPixels_)
  {
    pixelBuffer_[idx] = applyBrightness(color);
  }
}

void NeoPixelDriver::fillSolid(uint32_t color)
{
  uint32_t adjustedColor = applyBrightness(color);
  for (int i = 0; i < numPixels_; i++)
  {
    pixelBuffer_[i] = adjustedColor;
  }
}

void NeoPixelDriver::clear()
{
  for (int i = 0; i < numPixels_; i++)
  {
    pixelBuffer_[i] = 0;
  }
}

void NeoPixelDriver::show()
{
  // Simple bit-banging implementation for WS2812B
  for (int i = 0; i < numPixels_; i++)
  {
    sendPixel(pixelBuffer_[i]);
  }

  // Latch/reset time (>50us)
  gpio_set_level((gpio_num_t)pin_, 0);
  esp_rom_delay_us(60);
}

uint32_t NeoPixelDriver::Color(uint8_t r, uint8_t g, uint8_t b)
{
  return applyBrightness(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

uint32_t NeoPixelDriver::ColorHSV(uint16_t hue, uint8_t sat, uint8_t val)
{
  // Simple HSV to RGB conversion
  uint8_t r, g, bl;
  uint8_t region = hue / 43;
  uint8_t remainder = (hue - (region * 43)) * 6;
  uint8_t p = (val * (255 - sat)) >> 8;
  uint8_t q = (val * (255 - ((sat * remainder) >> 8))) >> 8;
  uint8_t t = (val * (255 - ((sat * (255 - remainder)) >> 8))) >> 8;

  switch (region)
  {
  case 0:
    r = val;
    g = t;
    bl = p;
    break;
  case 1:
    r = q;
    g = val;
    bl = p;
    break;
  case 2:
    r = p;
    g = val;
    bl = t;
    break;
  case 3:
    r = p;
    g = q;
    bl = val;
    break;
  case 4:
    r = t;
    g = p;
    bl = val;
    break;
  default:
    r = val;
    g = p;
    bl = q;
    break;
  }

  return Color(r, g, bl);
}

void NeoPixelDriver::sendPixel(uint32_t pixel)
{
  // Send 24 bits (GRB order for WS2812B)
  for (int bit = 23; bit >= 0; bit--)
  {
    sendByte((pixel >> bit) & 1);
  }
}

void NeoPixelDriver::sendByte(uint8_t byte)
{
  for (int i = 0; i < 8; i++)
  {
    if (byte & (1 << (7 - i)))
    {
      // Send '1': High for ~0.8us, Low for ~0.4us
      gpio_set_level((gpio_num_t)pin_, 1);
      esp_rom_delay_us(1);
      gpio_set_level((gpio_num_t)pin_, 0);
      esp_rom_delay_us(1);
    }
    else
    {
      // Send '0': High for ~0.4us, Low for ~0.8us
      gpio_set_level((gpio_num_t)pin_, 1);
      esp_rom_delay_us(1);
      gpio_set_level((gpio_num_t)pin_, 0);
      esp_rom_delay_us(1);
    }
  }
}

uint32_t NeoPixelDriver::applyBrightness(uint32_t color)
{
  if (brightness_ >= 255)
    return color;

  // Apply brightness scaling to each color component
  uint8_t r = (color >> 16) & 0xFF;
  uint8_t g = (color >> 8) & 0xFF;
  uint8_t b = color & 0xFF;

  r = (r * brightness_) / 255;
  g = (g * brightness_) / 255;
  b = (b * brightness_) / 255;

  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}