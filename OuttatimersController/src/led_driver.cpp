#include "led_driver.h"

// Implementation of NeoPixelDriver methods

void NeoPixelDriver::begin()
{
  ESP_LOGI(TAG, "Initializing NeoPixel driver on pin %d with %d LEDs", pin_, numPixels_);

  // Configure GPIO for output
  gpio_reset_pin((gpio_num_t)pin_);
  gpio_set_direction((gpio_num_t)pin_, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)pin_, 0); // Ensure pin starts LOW

  // Clear all pixels and ensure they're off
  clear();
  show();

  // Additional reset sequence for WS2812B
  gpio_set_level((gpio_num_t)pin_, 0);
  esp_rom_delay_us(100); // >50us reset time

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
  uint8_t red, green, blue;
  uint8_t region = hue / 43;
  uint8_t remainder = (hue - (region * 43)) * 6;
  uint8_t p = (val * (255 - sat)) >> 8;
  uint8_t q = (val * (255 - ((sat * remainder) >> 8))) >> 8;
  uint8_t t = (val * (255 - ((sat * (255 - remainder)) >> 8))) >> 8;

  switch (region)
  {
  case 0:
    red = val;
    green = t;
    blue = p;
    break;
  case 1:
    red = q;
    green = val;
    blue = p;
    break;
  case 2:
    red = p;
    green = val;
    blue = t;
    break;
  case 3:
    red = p;
    green = q;
    blue = val;
    break;
  case 4:
    red = t;
    green = p;
    blue = val;
    break;
  default:
    red = val;
    green = p;
    blue = q;
    break;
  }

  return Color(red, green, blue);
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
      esp_rom_delay_us(1); // ~0.85us high (slightly longer for better compatibility)
      gpio_set_level((gpio_num_t)pin_, 0);
      esp_rom_delay_us(1); // ~0.4us low (shorter delay for '1')
    }
    else
    {
      // Send '0': High for ~0.4us, Low for ~0.8us
      gpio_set_level((gpio_num_t)pin_, 1);
      esp_rom_delay_us(1); // ~0.4us high (shorter delay for '0')
      gpio_set_level((gpio_num_t)pin_, 0);
      esp_rom_delay_us(2); // ~0.8us low
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