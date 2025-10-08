#include "led_driver.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cstring>

// Implementation of RmtLedDriver methods

RmtLedDriver::~RmtLedDriver()
{
  if (led_encoder_)
  {
    rmt_del_encoder(led_encoder_);
  }
  if (tx_channel_)
  {
    rmt_disable(tx_channel_);
    rmt_del_channel(tx_channel_);
  }
}

void RmtLedDriver::begin()
{
  ESP_LOGI(TAG, "Initializing RMT LED driver on pin %d with %d LEDs", pin_, numPixels_);

  // Create RMT TX channel
  rmt_tx_channel_config_t tx_channel_config = {
      .gpio_num = (gpio_num_t)pin_,
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = ControllerConfig::Hardware::RMT_RESOLUTION_HZ,
      .mem_block_symbols = ControllerConfig::Hardware::RMT_MEM_BLOCK_SYMBOLS,
      .trans_queue_depth = ControllerConfig::Hardware::RMT_TRANSMIT_QUEUE_DEPTH,
      .flags = {
          .invert_out = false,
          .with_dma = false,
          .io_loop_back = false,
          .io_od_mode = false,
      }};

  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_channel_config, &tx_channel_));

  // Create LED strip encoder (WS2812B timing)
  rmt_bytes_encoder_config_t bytes_encoder_config;
  memset(&bytes_encoder_config, 0, sizeof(bytes_encoder_config));

  bytes_encoder_config.bit0.level0 = 1;
  bytes_encoder_config.bit0.duration0 = 3; // T0H = 0.3us
  bytes_encoder_config.bit0.level1 = 0;
  bytes_encoder_config.bit0.duration1 = 9; // T0L = 0.9us

  bytes_encoder_config.bit1.level0 = 1;
  bytes_encoder_config.bit1.duration0 = 9; // T1H = 0.9us
  bytes_encoder_config.bit1.level1 = 0;
  bytes_encoder_config.bit1.duration1 = 3; // T1L = 0.3us

  bytes_encoder_config.flags.msb_first = 1; // WS2812B: G7...G0 R7...R0 B7...B0

  ESP_ERROR_CHECK(rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder_));

  // Enable RMT channel
  ESP_ERROR_CHECK(rmt_enable(tx_channel_));

  ESP_LOGI(TAG, "RMT LED driver initialized");
}

void RmtLedDriver::setPixel(int idx, uint32_t color)
{
  if (idx >= 0 && idx < numPixels_)
  {
    pixelBuffer_[idx] = applyBrightness(color);
  }
}

void RmtLedDriver::fillSolid(uint32_t color)
{
  uint32_t adjustedColor = applyBrightness(color);
  for (int i = 0; i < numPixels_; i++)
  {
    pixelBuffer_[i] = adjustedColor;
  }
}

void RmtLedDriver::clear()
{
  for (int i = 0; i < numPixels_; i++)
  {
    pixelBuffer_[i] = 0;
  }
}

void RmtLedDriver::show()
{
  // Prepare pixel data in GRB format for WS2812B
  uint8_t pixelData[numPixels_ * 3];
  for (int i = 0; i < numPixels_; i++)
  {
    uint32_t color = pixelBuffer_[i];
    // WS2812B expects GRB order
    pixelData[i * 3 + 0] = (color >> 16) & 0xFF; // Green
    pixelData[i * 3 + 1] = (color >> 8) & 0xFF;  // Red
    pixelData[i * 3 + 2] = color & 0xFF;         // Blue
  }

  // Transmit using RMT
  rmt_transmit_config_t transmit_config = {
      .loop_count = 0, // No loop
  };

  ESP_ERROR_CHECK(rmt_transmit(tx_channel_, led_encoder_, pixelData, sizeof(pixelData), &transmit_config));

  // Wait for transmission to complete
  ESP_ERROR_CHECK(rmt_tx_wait_all_done(tx_channel_, portMAX_DELAY));
}

uint32_t RmtLedDriver::Color(uint8_t r, uint8_t g, uint8_t b)
{
  return applyBrightness(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

uint32_t RmtLedDriver::ColorHSV(uint16_t hue, uint8_t sat, uint8_t val)
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

uint32_t RmtLedDriver::applyBrightness(uint32_t color)
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