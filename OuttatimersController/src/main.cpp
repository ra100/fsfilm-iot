#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <esp_netif.h>
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

// Sleep functionality removed - no autosleep state needed

// ADC calibration handle
adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc1_cali_handle = NULL;

// WiFi state for effects
int wifiState = 0; // 0: disconnected, 1: connecting, 2: connected, 3: AP mode

// Logging tag
static const char *TAG = "outtatimers";

// Forward declaration for the main task
void main_task(void *pvParameter);

// Power management functions
void enterLightSleep()
{
  ESP_LOGI(TAG, "Entering light sleep mode for battery conservation...");

  // Disconnect from WiFi if connected
  if (wifiInput.isConnected_)
  {
    ESP_LOGI(TAG, "Disconnecting from WiFi before sleep...");
    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(100)); // Give time for disconnect
  }

  // Stop WiFi entirely
  esp_wifi_stop();
  vTaskDelay(pdMS_TO_TICKS(100));

  // Turn off all LEDs
  ledDriver.clear();
  ledDriver.show();

  // Turn off onboard LED
  gpio_set_level((gpio_num_t)ControllerConfig::Hardware::ONBOARD_LED_PIN, 0);

  ESP_LOGI(TAG, "WiFi disconnected, LEDs off - entering light sleep");

  ESP_LOGI(TAG, "Light sleep configured - press Button2 to wake up");

  // Enter light sleep
  esp_light_sleep_start();
}

// Battery monitoring functions
void initBatteryMonitoring()
{
  // Initialize ADC oneshot unit
  adc_oneshot_unit_init_cfg_t init_config = {
      .unit_id = ADC_UNIT_1,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

  // Configure ADC channel
  adc_oneshot_chan_cfg_t config = {
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, (adc_channel_t)ControllerConfig::Battery::VOLTAGE_PIN, &config));

  // Initialize ADC calibration
  adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = ADC_UNIT_1,
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &adc1_cali_handle));

  ESP_LOGI(TAG, "Battery monitoring initialized on GPIO %d (A%d/D%d)", ControllerConfig::Battery::VOLTAGE_PIN, ControllerConfig::Battery::VOLTAGE_PIN, ControllerConfig::Battery::VOLTAGE_PIN);
}

float readBatteryVoltage()
{
  int adc_raw;
  int voltage_mv;

  // Read ADC value
  ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, (adc_channel_t)ControllerConfig::Battery::VOLTAGE_PIN, &adc_raw));

  // Calibrate ADC reading to millivolts
  ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw, &voltage_mv));

  // Debug: Log raw ADC value and calibrated voltage
  ESP_LOGI(TAG, "Battery ADC raw value: %d, calibrated: %d mV", adc_raw, voltage_mv);

  // Convert millivolts to volts
  float adcVoltage = voltage_mv / 1000.0f;

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
  static uint8_t previousEffectIndex = 255; // Initialize to invalid value
  int64_t currentTime = esp_timer_get_time();
  uint8_t currentEffectIndex = effectManager.getCurrentEffectIndex();

  // Only monitor battery when BatteryStatusEffect is active (index 2)
  if (currentEffectIndex == 2)
  {
    // Force immediate update when switching to BatteryStatusEffect
    bool forceUpdate = (previousEffectIndex != 2);

    // Read battery voltage every 1 second for real-time display
    if (forceUpdate || currentTime - lastBatteryRead > 1000000) // 1 second in microseconds
    {
      batteryVoltage = readBatteryVoltage();
      batteryPercentage = calculateBatteryPercentage(batteryVoltage);

      ESP_LOGI(TAG, "Battery: %.2fV (%d%%)", batteryVoltage, batteryPercentage);
      lastBatteryRead = currentTime;
    }
  }

  previousEffectIndex = currentEffectIndex;
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

  // Initialize WiFi input source (hardware only, no connection)
  wifiInput.init();
  ESP_LOGI(TAG, "WiFi input source initialized (hardware only)");

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
        else if (buttonEvent.buttonId == 1) // Button2 (D6) - LED on/off toggle or deep sleep
        {
          if (buttonEvent.state == ButtonState::Pressed)
          {
            effectManager.toggleLeds();
            ESP_LOGI(TAG, "Button2 pressed - LEDs %s", effectManager.areLedsOn() ? "ON" : "OFF");
            // Autosleep removed - no timer needed
          }
          else if (buttonEvent.state == ButtonState::LightSleep)
          {
            ESP_LOGI(TAG, "Button2 held for 3 seconds - triggering light sleep");
            enterLightSleep();
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
        wifiState = 2; // Connected state
      }
      else
      {
        ESP_LOGI(TAG, "WiFi connecting - effect system active");
        ESP_LOGI(TAG, "WiFi connecting - onboard LED blinking");
        wifiState = 1; // Connecting state
      }
      wasConnected = isConnected;
      lastActivityTime = currentTime;
    }
    else if (!isConnected && !wasConnected)
    {
      // Check if we're in AP mode or completely disconnected
      // For now, assume disconnected if not connected and not connecting
      wifiState = 0; // Disconnected state
      // TODO: Add AP mode detection if needed
    }

    // Update onboard LED based on connection status
    updateOnboardLed(isConnected);

    // Update battery status periodically
    updateBatteryStatus();

    // Autosleep disabled - removed 10-minute timeout functionality

    // Update current effect
    effectManager.update(currentTime);

    // Power-efficient delay for effect updates
    vTaskDelay(pdMS_TO_TICKS(ControllerConfig::Timing::EFFECT_UPDATE_INTERVAL));
  }
}

// WiFi-only implementation - no LED functions needed
