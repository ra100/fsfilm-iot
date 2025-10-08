#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include "config.h"
#include "wifi_input_source.h"
#include "led_driver.h"
#include "effect_manager.h"
#include "button_handler.h"

// WiFi input source
WiFiInputSource wifiInput;

// LED driver for visual feedback
RmtLedDriver ledDriver;

// Effect manager for LED effects
EffectManager effectManager(ledDriver);

// Button handler for physical input
ButtonHandler buttonHandler;

// Onboard LED state
bool onboardLedState = false;
int64_t lastOnboardLedToggle = 0;

// Battery monitoring state
float batteryVoltage = 0.0f;
int batteryPercentage = 0; // Global variable accessible by effects
int64_t lastBatteryRead = 0;

// Logging tag
static const char *TAG = "outtatimers";

// Forward declaration for the main task
void main_task(void *pvParameter);

// Battery monitoring functions
void initBatteryMonitoring()
{
  // Configure ADC for battery voltage monitoring
  adc1_config_width(ADC_WIDTH_BIT_12); // 12-bit resolution
  adc1_config_channel_atten((adc1_channel_t)ControllerConfig::Battery::VOLTAGE_PIN, ADC_ATTEN_DB_11);

  ESP_LOGI(TAG, "Battery monitoring initialized on GPIO %d (A%d/D%d)", ControllerConfig::Battery::VOLTAGE_PIN, ControllerConfig::Battery::VOLTAGE_PIN, ControllerConfig::Battery::VOLTAGE_PIN);
}

float readBatteryVoltage()
{
  // Read ADC value and convert to voltage
  int adcValue = adc1_get_raw((adc1_channel_t)ControllerConfig::Battery::VOLTAGE_PIN);

  // Convert ADC reading to actual voltage
  // ESP32-C6 ADC reference voltage is typically 1.1V internal, but calibrated to 3.3V range
  float adcVoltage = (adcValue * 3.3f) / 4095.0f;

  // Apply voltage divider ratio (battery voltage = ADC voltage * divider ratio)
  float actualVoltage = adcVoltage * ControllerConfig::Battery::VOLTAGE_DIVIDER_RATIO;

  return actualVoltage;
}

int calculateBatteryPercentage(float voltage)
{
  // Clamp voltage to valid range
  if (voltage < ControllerConfig::Battery::MIN_VOLTAGE)
    return 0;
  if (voltage > ControllerConfig::Battery::MAX_VOLTAGE)
    return 100;

  // Calculate percentage
  float range = ControllerConfig::Battery::MAX_VOLTAGE - ControllerConfig::Battery::MIN_VOLTAGE;
  return (int)((voltage - ControllerConfig::Battery::MIN_VOLTAGE) / range * 100.0f);
}

void updateBatteryStatus()
{
  int64_t currentTime = esp_timer_get_time();

  // Read battery voltage periodically
  if (currentTime - lastBatteryRead > ControllerConfig::Battery::READ_INTERVAL_MS * 1000)
  {
    batteryVoltage = readBatteryVoltage();
    batteryPercentage = calculateBatteryPercentage(batteryVoltage);

    ESP_LOGI(TAG, "Battery: %.2fV (%d%%)", batteryVoltage, batteryPercentage);
    lastBatteryRead = currentTime;
  }
}

// Onboard LED control functions
void updateOnboardLed(bool isConnected)
{
  int64_t currentTime = esp_timer_get_time();

  if (isConnected)
  {
    // Connected: Keep onboard LED ON solid
    gpio_set_level((gpio_num_t)ControllerConfig::Hardware::ONBOARD_LED_PIN, 1);
    onboardLedState = true;
  }
  else
  {
    // Connecting: Blink onboard LED (500ms intervals)
    if (currentTime - lastOnboardLedToggle > 500000) // 500ms in microseconds
    {
      onboardLedState = !onboardLedState;
      gpio_set_level((gpio_num_t)ControllerConfig::Hardware::ONBOARD_LED_PIN, onboardLedState ? 1 : 0);
      lastOnboardLedToggle = currentTime;
    }
  }
}

extern "C" void app_main(void)
{
  // Very early debug output - this should appear immediately
  ESP_LOGI(TAG, "========== APP_MAIN STARTED ==========");
  ESP_LOGI(TAG, "Outtatimers Controller Starting...");
  ESP_LOGI(TAG, "ESP-IDF Version: %s", esp_get_idf_version());
  ESP_LOGI(TAG, "Free heap: %d bytes", esp_get_free_heap_size());

  // Initialize battery monitoring
  initBatteryMonitoring();

  // Initialize onboard LED for WiFi status (GPIO2/D2 from pinout)
  ESP_LOGI(TAG, "Initializing onboard LED on GPIO %d (D%d)...", ControllerConfig::Hardware::ONBOARD_LED_PIN, ControllerConfig::Hardware::ONBOARD_LED_PIN);
  gpio_reset_pin((gpio_num_t)ControllerConfig::Hardware::ONBOARD_LED_PIN);
  gpio_set_direction((gpio_num_t)ControllerConfig::Hardware::ONBOARD_LED_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t)ControllerConfig::Hardware::ONBOARD_LED_PIN, 0); // Start OFF
  ESP_LOGI(TAG, "Onboard LED initialized on GPIO %d (D%d)", ControllerConfig::Hardware::ONBOARD_LED_PIN, ControllerConfig::Hardware::ONBOARD_LED_PIN);

  // Test onboard LED immediately
  gpio_set_level((gpio_num_t)ControllerConfig::Hardware::ONBOARD_LED_PIN, 1);
  ESP_LOGI(TAG, "Testing onboard LED - should be ON now");
  vTaskDelay(pdMS_TO_TICKS(500));
  gpio_set_level((gpio_num_t)ControllerConfig::Hardware::ONBOARD_LED_PIN, 0);
  ESP_LOGI(TAG, "Testing onboard LED - should be OFF now");

  // Initialize LED driver for external LED strip with power-efficient settings
  ESP_LOGI(TAG, "Initializing external LED driver on GPIO %d (D%d/A%d)...", ControllerConfig::Hardware::LED_PIN, ControllerConfig::Hardware::LED_PIN, ControllerConfig::Hardware::LED_PIN);
  ledDriver.begin();
  ledDriver.setBrightness(ControllerConfig::Hardware::DEFAULT_BRIGHTNESS); // Use full brightness for proper testing
  ESP_LOGI(TAG, "External LED driver initialized with %d brightness on GPIO %d (D%d)", ControllerConfig::Hardware::DEFAULT_BRIGHTNESS, ControllerConfig::Hardware::LED_PIN, ControllerConfig::Hardware::LED_PIN);

  // Simple LED test - turn on first LED red, then clear all
  ESP_LOGI(TAG, "Testing LED strip - setting first LED to red...");
  ledDriver.setPixel(0, ledDriver.Color(255, 0, 0)); // Red
  ledDriver.show();
  vTaskDelay(pdMS_TO_TICKS(1000));

  ESP_LOGI(TAG, "Testing LED strip - clearing all LEDs...");
  ledDriver.clear();
  ledDriver.show();
  vTaskDelay(pdMS_TO_TICKS(500));

  ESP_LOGI(TAG, "LED test complete");

  // Initialize effect manager
  effectManager.begin();
  ESP_LOGI(TAG, "Effect manager initialized with purple chase as default effect");

  // Initialize WiFi input source
  wifiInput.begin(WIFI_SSID, WIFI_PASSWORD);
  ESP_LOGI(TAG, "WiFi input source initialized");

  // Initialize button handler
  buttonHandler.begin();
  ESP_LOGI(TAG, "Button handler initialized");

  ESP_LOGI(TAG, "Setup complete, starting main task");

  // Start the main task
  xTaskCreate(main_task, "main_task", 4096, NULL, 5, NULL);
}

void main_task(void *pvParameter)
{
  ESP_LOGI(TAG, "Main task started - Effect-based LED control mode");

  // Track connection status for LED indication
  bool wasConnected = false;
  int64_t lastActivityTime = esp_timer_get_time();

  while (true)
  {
    int64_t currentTime = esp_timer_get_time();

    // Handle WiFi input events
    if (wifiInput.update(currentTime))
    {
      if (wifiInput.hasEvents())
      {
        InputEvent wifiEvent = wifiInput.getNextEvent();
        if (wifiEvent.type == EventType::Pressed)
        {
          ESP_LOGI(TAG, "WiFi event received - Input ID: %d", wifiEvent.inputId);
          lastActivityTime = currentTime;

          if (wifiEvent.inputId >= 0 && wifiEvent.inputId < effectManager.getEffectCount())
          {
            // Effect change event (0-5)
            effectManager.setEffect(wifiEvent.inputId);
          }
          else if (wifiEvent.inputId < 0)
          {
            // Brightness change event (negative value = brightness)
            uint8_t brightness = static_cast<uint8_t>(-wifiEvent.inputId);
            effectManager.setBrightness(brightness);
            ESP_LOGI(TAG, "Brightness set to %d via WiFi", brightness);
          }
        }
      }
    }

    // Handle button input events
    if (buttonHandler.update(currentTime))
    {
      if (buttonHandler.hasEvents())
      {
        ButtonEvent buttonEvent = buttonHandler.getNextEvent();
        ESP_LOGI(TAG, "Button event - Button ID: %d, State: %d", buttonEvent.buttonId, static_cast<int>(buttonEvent.state));
        lastActivityTime = currentTime;

        if (buttonEvent.buttonId == 0) // Button1 (D5) - Effect cycling
        {
          if (buttonEvent.state == ButtonState::Pressed)
          {
            effectManager.nextEffect();
            ESP_LOGI(TAG, "Button1 pressed - Next effect: %s", effectManager.getCurrentEffectName());
          }
        }
        else if (buttonEvent.buttonId == 1) // Button2 (D6) - LED on/off toggle
        {
          if (buttonEvent.state == ButtonState::Pressed)
          {
            effectManager.toggleLeds();
            ESP_LOGI(TAG, "Button2 pressed - LEDs %s", effectManager.areLedsOn() ? "ON" : "OFF");
          }
        }
      }
    }

    // Update connection status LED indication (overlay on current effect)
    bool isConnected = wifiInput.isConnected_;
    if (isConnected != wasConnected)
    {
      if (isConnected)
      {
        ESP_LOGI(TAG, "WiFi connected - effect system active");
        ESP_LOGI(TAG, "WiFi connected - onboard LED solid ON");
      }
      else
      {
        ESP_LOGI(TAG, "WiFi connecting - effect system active");
        ESP_LOGI(TAG, "WiFi connecting - onboard LED blinking");
      }
      wasConnected = isConnected;
      lastActivityTime = currentTime;
    }

    // Update onboard LED based on connection status
    updateOnboardLed(isConnected);

    // Update battery status periodically
    updateBatteryStatus();

    // Update current effect
    effectManager.update(currentTime);

    // Power-efficient delay for effect updates
    vTaskDelay(pdMS_TO_TICKS(ControllerConfig::Timing::EFFECT_UPDATE_INTERVAL));
  }
}

// WiFi-only implementation - no LED functions needed
