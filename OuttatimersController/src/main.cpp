#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_timer.h>
#include "config.h"
#include "wifi_input_source.h"

// WiFi input source
WiFiInputSource wifiInput;

// Logging tag
static const char *TAG = "outtatimers";

// Forward declaration for the main task
void main_task(void *pvParameter);

extern "C" void app_main(void)
{
  ESP_LOGI(TAG, "Outtatimers Controller Starting...");

  // Initialize WiFi input source
  wifiInput.begin(WIFI_SSID, WIFI_PASSWORD);
  ESP_LOGI(TAG, "WiFi input source initialized");

  ESP_LOGI(TAG, "Setup complete, starting main task");

  // Start the main task
  xTaskCreate(main_task, "main_task", 4096, NULL, 5, NULL);
}

void main_task(void *pvParameter)
{
  ESP_LOGI(TAG, "Main task started - WiFi only mode");

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
        }
      }
    }

    // Simple delay for WiFi polling
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// WiFi-only implementation - no LED functions needed
