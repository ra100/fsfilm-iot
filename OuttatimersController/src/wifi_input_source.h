#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_http_server.h>
#include <nvs_flash.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <string>
#include "config.h"
#include "../wifi_credentials.h"

// Forward declarations for input manager (to avoid circular dependency)
class InputManager;

// ESP-IDF compatible event types
enum class EventType
{
  Pressed,
  Released
};

struct InputEvent
{
  int inputId;
  EventType type;
  int64_t timestamp;
  std::string sourceName;
};

class IInputSource
{
public:
  virtual bool update(int64_t currentTime) = 0;
  virtual bool hasEvents() const = 0;
  virtual InputEvent getNextEvent() = 0;
  virtual const char *getSourceName() const = 0;
  virtual ~IInputSource() {}
};

/**
 * @brief WiFi-based input source for remote control via HTTP
 *
 * This class provides WiFi connectivity with automatic fallback to Access Point mode.
 * It creates a web server that accepts HTTP requests to control LED effects.
 * Uses ESP-IDF WiFi and HTTP server APIs.
 */
class WiFiInputSource : public IInputSource
{
public:
  /**
   * @brief Construct a new WiFiInputSource
   * @param port HTTP server port (default: 80)
   */
  explicit WiFiInputSource(int port = ControllerConfig::WiFi::HTTP_PORT)
      : server_handle_(nullptr), eventQueueHead_(0), eventQueueTail_(0), isConnected_(false),
        connectionStartTime_(0), inAPMode_(false), apServerStarted_(false), timeoutLogged_(false) {}

  /**
   * @brief Initialize WiFi hardware (NVS, event loop, etc.) without connecting
   * @return true if initialization successful
   */
  bool init();

  /**
   * @brief Start WiFi connection and web server
   * @param ssid WiFi network name
   * @param password WiFi password
   * @return true if connection started successfully
   */
  bool startConnection(const char *ssid, const char *password);

  /**
   * @brief Stop WiFi connection and web server
   */
  void stopConnection();

  // ESP-IDF HTTP server handle
  httpd_handle_t server_handle_;

  // Static TAG for logging
  static constexpr const char *TAG = "WiFiInputSource";

  // Event queue for input events
  static constexpr int MAX_EVENTS = 8;
  InputEvent eventQueue_[MAX_EVENTS];
  int eventQueueHead_;
  int eventQueueTail_;

  // Connection state
  bool isConnected_;
  int64_t connectionStartTime_;
  bool inAPMode_;
  bool apServerStarted_;
  bool timeoutLogged_;

  // WiFi event handler
  static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

  // HTTP request handlers
  static esp_err_t handle_root(httpd_req_t *req);
  static esp_err_t handle_set_effect(httpd_req_t *req);
  static esp_err_t handle_set_brightness(httpd_req_t *req);
  static esp_err_t handle_status(httpd_req_t *req);
  static esp_err_t handle_battery(httpd_req_t *req);
  static esp_err_t handle_list_effects(httpd_req_t *req);

public:
  // IInputSource interface implementation
  bool update(int64_t currentTime)
  {
    // Check connection timeout only if we're trying to connect and haven't logged timeout yet
    if (!isConnected_ && connectionStartTime_ > 0 &&
        (currentTime - connectionStartTime_) > ControllerConfig::WiFi::CONNECTION_TIMEOUT_MS * 1000)
    {
      if (!timeoutLogged_)
      {
        ESP_LOGW(TAG, "WiFi connection timeout - switching to AP mode");
        timeoutLogged_ = true;

        // Switch to AP mode
        if (!inAPMode_)
        {
          ESP_LOGI(TAG, "Starting Access Point mode...");
          start_wifi_ap();
          inAPMode_ = true;
        }
      }
    }

    return hasEvents();
  }

  bool hasEvents() const
  {
    return eventQueueHead_ != eventQueueTail_;
  }

  InputEvent getNextEvent()
  {
    if (!hasEvents())
    {
      return {0, EventType::Released, 0, "none"};
    }

    InputEvent event = eventQueue_[eventQueueHead_];
    eventQueueHead_ = (eventQueueHead_ + 1) % MAX_EVENTS;
    return event;
  }

  const char *getSourceName() const
  {
    return "WiFiInput";
  }

  // Helper methods (needed by implementation)
  esp_err_t setup_web_server();
  esp_err_t start_wifi_station(const char *ssid, const char *password);
  esp_err_t start_wifi_ap();
  void queue_event(const InputEvent &event);
  std::string get_effect_name(int effect);
  std::string get_ip_address();

private:
};
