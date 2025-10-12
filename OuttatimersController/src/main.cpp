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
#include "config.h"
#include "led_driver.h"
#include "effect_manager.h"
#include "button_handler.h"

// LED driver for visual feedback
RmtLedDriver ledDriver;

// Effect manager for LED effects
EffectManager effectManager(ledDriver);

// Button handler for physical input
ButtonHandler buttonHandler;

// Battery monitoring state
float batteryVoltage = 0.0f;
int batteryPercentage = 0; // Global variable accessible by effects
int64_t lastBatteryRead = 0;
bool batteryConnected = false; // Battery presence detection
int64_t lastBatteryCheck = 0;

// Sleep functionality removed - no autosleep state needed

// ADC calibration handle
adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc1_cali_handle = NULL;

// Logging tag
static const char *TAG = "outtatimers";

// Forward declaration for the main task
void main_task(void *pvParameter);

// Power management functions
void enterLightSleep()
{
  ESP_LOGI(TAG, "Entering light sleep mode for battery conservation...");

  // Turn off all LEDs
  ledDriver.clear();
  ledDriver.show();

  // Turn off onboard LED
  gpio_set_level((gpio_num_t)ControllerConfig::Hardware::ONBOARD_LED_PIN, 0);

  ESP_LOGI(TAG, "LEDs off - entering light sleep");

  ESP_LOGI(TAG, "Light sleep configured - press Button2 to wake up");

  // Enter light sleep
  esp_light_sleep_start();
}

// Battery presence detection and monitoring functions
// Enhanced battery detection with hysteresis and improved thresholds
bool detectBatteryPresence()
{
  static bool lastBatteryState = false;      // Track previous state for hysteresis
  static int consecutiveStableReadings = 0;  // Count stable readings for debouncing
  static int consecutiveBatteryReadings = 0; // Count battery readings for debouncing

  // Read ADC multiple times to check for battery vs USB power characteristics
  int adc_raw1, adc_raw2, adc_raw3;
  int voltage_mv1, voltage_mv2, voltage_mv3;

  ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, (adc_channel_t)ControllerConfig::Battery::VOLTAGE_PIN, &adc_raw1));
  ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, (adc_channel_t)ControllerConfig::Battery::VOLTAGE_PIN, &adc_raw2));
  ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, (adc_channel_t)ControllerConfig::Battery::VOLTAGE_PIN, &adc_raw3));

  ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw1, &voltage_mv1));
  ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw2, &voltage_mv2));
  ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw3, &voltage_mv3));

  // Calculate variance in readings
  int max_voltage = voltage_mv1;
  int min_voltage = voltage_mv1;
  if (voltage_mv2 > max_voltage)
    max_voltage = voltage_mv2;
  if (voltage_mv3 > max_voltage)
    max_voltage = voltage_mv3;
  if (voltage_mv2 < min_voltage)
    min_voltage = voltage_mv2;
  if (voltage_mv3 < min_voltage)
    min_voltage = voltage_mv3;

  int voltage_range = max_voltage - min_voltage;
  int avg_voltage = (voltage_mv1 + voltage_mv2 + voltage_mv3) / 3;

  ESP_LOGI(TAG, "Battery presence check - V1: %dmV, V2: %dmV, V3: %dmV, Avg: %dmV, Range: %dmV",
           voltage_mv1, voltage_mv2, voltage_mv3, avg_voltage, voltage_range);

  // REDESIGNED: Range-based detection with hysteresis (replacing variance-based)
  // - USB power: Higher voltage (4.5-5V) that drops through 200k resistor
  // - Battery: 3.0-4.2V without resistor drop
  // - Floating: Outside both ranges or high variance
  // Hysteresis: Use different thresholds based on previous state to prevent oscillation

  bool currentBatteryDetected = false;

  // Apply hysteresis to thresholds based on previous state
  int batteryMaxThreshold = lastBatteryState ? ControllerConfig::Battery::BATTERY_VOLTAGE_MAX_MV + ControllerConfig::Battery::DETECTION_HYSTERESIS_MV / 2 : ControllerConfig::Battery::BATTERY_VOLTAGE_MAX_MV - ControllerConfig::Battery::DETECTION_HYSTERESIS_MV / 2;

  int usbMinThreshold = lastBatteryState ? ControllerConfig::Battery::USB_VOLTAGE_MIN_MV + ControllerConfig::Battery::DETECTION_HYSTERESIS_MV / 2 : ControllerConfig::Battery::USB_VOLTAGE_MIN_MV - ControllerConfig::Battery::DETECTION_HYSTERESIS_MV / 2;

  // DEBUG: Log detailed decision criteria
  ESP_LOGI(TAG, "DEBUG: Range-based detection with hysteresis - Avg(%d mV), Range(%d mV)", avg_voltage, voltage_range);
  ESP_LOGI(TAG, "DEBUG: Hysteresis thresholds - Battery max: %d mV, USB min: %d mV (last state: %s)",
           batteryMaxThreshold, usbMinThreshold, lastBatteryState ? "BATTERY" : "USB");

  // Check for battery voltage range (with hysteresis)
  bool inBatteryRange = (avg_voltage >= ControllerConfig::Battery::BATTERY_VOLTAGE_MIN_MV &&
                         avg_voltage <= batteryMaxThreshold);

  // Check for USB voltage range (with hysteresis)
  bool inUsbRange = (avg_voltage >= usbMinThreshold &&
                     avg_voltage <= ControllerConfig::Battery::USB_VOLTAGE_MAX_MV);

  // Check for floating/high variance
  bool isFloating = (voltage_range > ControllerConfig::Battery::FLOATING_PIN_VARIANCE_THRESHOLD);

  ESP_LOGI(TAG, "DEBUG: inBatteryRange: %s, inUsbRange: %s, isFloating: %s",
           inBatteryRange ? "YES" : "NO", inUsbRange ? "YES" : "NO", isFloating ? "YES" : "NO");

  if (inBatteryRange && !isFloating)
  {
    // Voltage in battery range with acceptable variance - battery connected
    ESP_LOGI(TAG, "Battery presence: BATTERY CONNECTED - Voltage in battery range (%d-%d mV with hysteresis)",
             ControllerConfig::Battery::BATTERY_VOLTAGE_MIN_MV, batteryMaxThreshold);
    currentBatteryDetected = true;
  }
  else if (inUsbRange && !isFloating)
  {
    // Voltage in USB range - USB power (no battery)
    ESP_LOGI(TAG, "Battery presence: NO BATTERY - USB power detected (%d-%d mV range with hysteresis)",
             usbMinThreshold, ControllerConfig::Battery::USB_VOLTAGE_MAX_MV);
    currentBatteryDetected = false;
  }
  else if (isFloating)
  {
    // High variance indicates floating pin
    ESP_LOGW(TAG, "Battery presence: FLOATING PIN - High variance detected (%d mV range)", voltage_range);
    currentBatteryDetected = false;
  }
  else
  {
    // Voltage outside both ranges - unclear, default to no battery
    ESP_LOGW(TAG, "Battery presence: UNKNOWN - Voltage %d mV outside expected ranges with hysteresis", avg_voltage);
    currentBatteryDetected = false;
  }

  // DEBUG: Log final decision
  ESP_LOGI(TAG, "DEBUG: Final detection result - currentBatteryDetected: %s", currentBatteryDetected ? "TRUE" : "FALSE");

  // Apply hysteresis to prevent rapid state changes
  // Require 3 consecutive readings of the same state before changing
  if (currentBatteryDetected == lastBatteryState)
  {
    if (currentBatteryDetected)
      consecutiveBatteryReadings++;
    else
      consecutiveStableReadings++;

    // Change state only after N consecutive consistent readings (configurable)
    if (consecutiveBatteryReadings >= ControllerConfig::Battery::HYSTERESIS_CONFIRMATION_COUNT ||
        consecutiveStableReadings >= ControllerConfig::Battery::HYSTERESIS_CONFIRMATION_COUNT)
    {
      lastBatteryState = currentBatteryDetected;
      consecutiveBatteryReadings = 0;
      consecutiveStableReadings = 0;
      ESP_LOGI(TAG, "Battery state changed to: %s (confirmed by 3 consecutive readings)",
               currentBatteryDetected ? "CONNECTED" : "NOT CONNECTED");
    }
  }
  else
  {
    // State changed, reset counters
    consecutiveBatteryReadings = 0;
    consecutiveStableReadings = 0;
    ESP_LOGI(TAG, "Battery state change detected, waiting for confirmation...");
  }

  return lastBatteryState;
}

void configureBatteryPin()
{
  // Configure GPIO with pull-down resistor to prevent floating when no battery
  gpio_reset_pin((gpio_num_t)ControllerConfig::Battery::VOLTAGE_PIN);
  gpio_set_direction((gpio_num_t)ControllerConfig::Battery::VOLTAGE_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode((gpio_num_t)ControllerConfig::Battery::VOLTAGE_PIN, GPIO_PULLDOWN_ONLY);

  vTaskDelay(pdMS_TO_TICKS(10)); // Let it settle

  // Reconfigure for ADC
  adc_oneshot_chan_cfg_t config = {
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, (adc_channel_t)ControllerConfig::Battery::VOLTAGE_PIN, &config));

  ESP_LOGI(TAG, "Battery pin configured with pull-down resistor");
}

// Battery monitoring functions
void testADCPinFloating()
{
  ESP_LOGI(TAG, "=== TESTING ADC PIN FOR FLOATING BEHAVIOR ===");

  // Test 1: Read current configuration
  int adc_raw_float;
  ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, (adc_channel_t)ControllerConfig::Battery::VOLTAGE_PIN, &adc_raw_float));
  ESP_LOGI(TAG, "Current ADC reading (floating): %d", adc_raw_float);

  // Test 2: Configure GPIO as input with pull-down and read
  gpio_reset_pin((gpio_num_t)ControllerConfig::Battery::VOLTAGE_PIN);
  gpio_set_direction((gpio_num_t)ControllerConfig::Battery::VOLTAGE_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode((gpio_num_t)ControllerConfig::Battery::VOLTAGE_PIN, GPIO_PULLDOWN_ONLY);

  vTaskDelay(pdMS_TO_TICKS(10)); // Let it settle

  gpio_reset_pin((gpio_num_t)ControllerConfig::Battery::VOLTAGE_PIN);
  gpio_set_direction((gpio_num_t)ControllerConfig::Battery::VOLTAGE_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode((gpio_num_t)ControllerConfig::Battery::VOLTAGE_PIN, GPIO_PULLUP_ONLY);

  vTaskDelay(pdMS_TO_TICKS(10)); // Let it settle

  // Test 3: Reconfigure ADC and read again
  adc_oneshot_chan_cfg_t config = {
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, (adc_channel_t)ControllerConfig::Battery::VOLTAGE_PIN, &config));

  int adc_raw_after;
  ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, (adc_channel_t)ControllerConfig::Battery::VOLTAGE_PIN, &adc_raw_after));
  ESP_LOGI(TAG, "ADC reading after GPIO config: %d", adc_raw_after);

  ESP_LOGI(TAG, "=== FLOATING TEST COMPLETE ===");
}

void initBatteryMonitoring()
{
  // Initialize ADC oneshot unit
  adc_oneshot_unit_init_cfg_t init_config = {
      .unit_id = ADC_UNIT_1,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

  // Configure battery pin with pull-down resistor first
  configureBatteryPin();

  // Initialize ADC calibration
  adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = ADC_UNIT_1,
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &adc1_cali_handle));

  // Perform initial battery presence check
  batteryConnected = detectBatteryPresence();

  ESP_LOGI(TAG, "Battery monitoring initialized on GPIO %d (A%d/D%d) - Battery connected: %s",
           ControllerConfig::Battery::VOLTAGE_PIN, ControllerConfig::Battery::VOLTAGE_PIN,
           ControllerConfig::Battery::VOLTAGE_PIN, batteryConnected ? "YES" : "NO");
}

float readBatteryVoltage()
{
  // Take 16 ADC readings using the existing ESP-IDF ADC oneshot API on GPIO0
  int adc_raw_sum = 0;
  for (int i = 0; i < 16; i++)
  {
    int adc_raw;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, (adc_channel_t)ControllerConfig::Battery::VOLTAGE_PIN, &adc_raw));
    adc_raw_sum += adc_raw;
  }

  // Average the raw ADC values
  int adc_raw_avg = adc_raw_sum / 16;

  // Convert the average to millivolts using adc_cali_raw_to_voltage()
  int voltage_mv;
  ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw_avg, &voltage_mv));

  // Convert millivolts to volts
  float adcVoltage = voltage_mv / 1000.0f;

  float actualVoltage;

  if (batteryConnected)
  {
    // Multiply by 2 to compensate for the 1:2 voltage divider
    actualVoltage = adcVoltage * 2.0f;
    ESP_LOGI(TAG, "Battery voltage: %.3fV (with 1:2 divider compensation)", actualVoltage);
  }
  else
  {
    // No battery connected - this is USB power reading
    actualVoltage = adcVoltage; // Direct reading of USB power
    ESP_LOGI(TAG, "USB power reading: %.3fV (no battery connected)", actualVoltage);
  }

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
  static bool lastBatteryState = false;     // Track battery state changes
  int64_t currentTime = esp_timer_get_time();
  uint8_t currentEffectIndex = effectManager.getCurrentEffectIndex();

  // Check battery presence every 5 seconds (reduced frequency for stability)
  if (currentTime - lastBatteryCheck > 5000000) // 5 seconds in microseconds
  {
    bool newBatteryState = detectBatteryPresence();
    lastBatteryCheck = currentTime;

    // Log state changes for debugging
    if (newBatteryState != lastBatteryState)
    {
      ESP_LOGI(TAG, "Battery state changed: %s -> %s",
               lastBatteryState ? "CONNECTED" : "NOT CONNECTED",
               newBatteryState ? "CONNECTED" : "NOT CONNECTED");
      lastBatteryState = newBatteryState;
    }

    batteryConnected = newBatteryState;

    if (!batteryConnected)
    {
      ESP_LOGI(TAG, "No battery detected - running on USB power");
      batteryVoltage = 0.0f;
      batteryPercentage = 0;
    }
  }

  // Always monitor voltage when BatteryStatusEffect is active (index 2) for debugging
  if (currentEffectIndex == 2)
  {
    // Force immediate update when switching to BatteryStatusEffect
    bool forceUpdate = (previousEffectIndex != 2);

    // Read voltage every 1 second for real-time display
    if (forceUpdate || currentTime - lastBatteryRead > 1000000) // 1 second in microseconds
    {
      batteryVoltage = readBatteryVoltage();
      if (batteryConnected)
      {
        batteryPercentage = calculateBatteryPercentage(batteryVoltage);
        ESP_LOGI(TAG, "Battery Status Effect: BATTERY CONNECTED - %.2fV (%d%%)", batteryVoltage, batteryPercentage);
      }
      else
      {
        // Show raw voltage reading for debugging when no battery detected
        batteryPercentage = 0; // No battery, so 0%
        ESP_LOGI(TAG, "Battery Status Effect: NO BATTERY DETECTED - Raw voltage: %.2fV", batteryVoltage);
      }
      lastBatteryRead = currentTime;
    }
  }

  previousEffectIndex = currentEffectIndex;
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

  // Test if ADC pin is floating (diagnostic)
  testADCPinFloating();

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
  ESP_LOGI(TAG, "Effect manager initialized with portal open as default effect");

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

  // Track activity time (no WiFi connection tracking needed)
  int64_t lastActivityTime = esp_timer_get_time();

  while (true)
  {
    int64_t currentTime = esp_timer_get_time();

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

    // Update onboard LED - always solid ON (no WiFi status)
    gpio_set_level((gpio_num_t)ControllerConfig::Hardware::ONBOARD_LED_PIN, 1);

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
