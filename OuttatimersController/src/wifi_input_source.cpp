// WiFiInputSource implementation
#include "wifi_input_source.h"
#include <esp_timer.h>
#include <nvs_flash.h>
#include <esp_netif.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_http_server.h>
#include <lwip/err.h>
#include <lwip/sys.h>
#include <cstring>

// Implementation of methods that need to be in a separate file to avoid multiple definitions

esp_err_t WiFiInputSource::setup_web_server()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 10;
  config.stack_size = 16384; // Increase stack size to 16KB to handle large HTML responses

  ESP_ERROR_CHECK(httpd_start(&server_handle_, &config));

  // Register URI handlers
  httpd_uri_t root_handler = {
      .uri = "/",
      .method = HTTP_GET,
      .handler = handle_root,
      .user_ctx = this};
  httpd_register_uri_handler(server_handle_, &root_handler);

  httpd_uri_t effect_handler = {
      .uri = "/effect",
      .method = HTTP_GET,
      .handler = handle_set_effect,
      .user_ctx = this};
  httpd_register_uri_handler(server_handle_, &effect_handler);

  httpd_uri_t brightness_handler = {
      .uri = "/brightness",
      .method = HTTP_GET,
      .handler = handle_set_brightness,
      .user_ctx = this};
  httpd_register_uri_handler(server_handle_, &brightness_handler);

  httpd_uri_t status_handler = {
      .uri = "/status",
      .method = HTTP_GET,
      .handler = handle_status,
      .user_ctx = this};
  httpd_register_uri_handler(server_handle_, &status_handler);

  httpd_uri_t battery_handler = {
      .uri = "/battery",
      .method = HTTP_GET,
      .handler = handle_battery,
      .user_ctx = this};
  httpd_register_uri_handler(server_handle_, &battery_handler);

  return ESP_OK;
}

esp_err_t WiFiInputSource::start_wifi_station(const char *ssid, const char *password)
{
  // Create station network interface
  esp_netif_create_default_wifi_sta();

  // WiFi configuration
  wifi_config_t wifi_config = {};
  strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
  strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
  wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

  // Initialize WiFi
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

  // Enable power save mode for battery efficiency
  if (ControllerConfig::WiFi::POWER_SAVE_MODE)
  {
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM)); // Maximum power save
    ESP_LOGI(TAG, "WiFi power save mode enabled");
  }

  ESP_ERROR_CHECK(esp_wifi_start());

  connectionStartTime_ = esp_timer_get_time();
  ESP_LOGI(TAG, "Connecting to WiFi: %s", ssid);

  return ESP_OK;
}

void WiFiInputSource::queue_event(const InputEvent &event)
{
  int nextTail = (eventQueueTail_ + 1) % MAX_EVENTS;
  if (nextTail != eventQueueHead_)
  {
    eventQueue_[eventQueueTail_] = event;
    eventQueueTail_ = nextTail;
  }
}

std::string WiFiInputSource::get_effect_name(int effect)
{
  switch (effect)
  {
  case 0:
    return "Rotating Darkness";
  case 1:
    return "Portal Open";
  case 2:
    return "Battery Status";
  case 3:
    return "Random Blink";
  case 4:
    return "WiFi Mode";
  default:
    return "Unknown";
  }
}

std::string WiFiInputSource::get_ip_address()
{
  if (inAPMode_)
  {
    // In AP mode, return the standard AP IP address
    return "192.168.4.1";
  }
  else if (isConnected_)
  {
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK)
    {
      ESP_LOGI(TAG, "IP Address: " IPSTR, IP2STR(&ip_info.ip));
      char ip_str[16];
      snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&ip_info.ip));
      return std::string(ip_str);
    }
  }
  return "Not Connected";
}

void WiFiInputSource::wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
  WiFiInputSource *self = (WiFiInputSource *)arg;

  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    esp_wifi_connect();
  }
  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    self->isConnected_ = false;
    ESP_LOGI(TAG, "WiFi disconnected, reconnecting...");
    esp_wifi_connect();
  }
  else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    self->isConnected_ = true;
    ESP_LOGI(TAG, "WiFi connected! IP: " IPSTR, IP2STR(&event->ip_info.ip));
  }
}

esp_err_t WiFiInputSource::handle_root(httpd_req_t *req)
{
  WiFiInputSource *self = (WiFiInputSource *)req->user_ctx;

  // Use a smaller, more efficient HTML response
  const char *html_template =
      "<!DOCTYPE html>"
      "<html><head><title>Outtatimers Controller</title>"
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
      "<style>"
      "body{font-family:Arial,sans-serif;margin:20px;}"
      ".container{max-width:600px;margin:0 auto;}"
      ".btn{background:#007bff;color:white;border:none;padding:10px 20px;margin:5px;border-radius:5px;cursor:pointer;}"
      ".btn:hover{background:#0056b3;}"
      ".effect-btn{background:#28a745;}"
      ".effect-btn:hover{background:#1e7e34;}"
      ".status{background:#e9ecef;padding:10px;margin:10px 0;border-radius:5px;}"
      "</style></head>"
      "<body><div class=\"container\">"
      "<h1>Outtatimers Controller</h1>"
      "<div class=\"status\"><h3>Status: %s</h3><p><strong>IP:</strong> %s</p><div id=\"batteryStatus\"></div></div>"
      "<h3>Effect Control</h3>"
      "<button class=\"btn effect-btn\" onclick=\"setEffect(0)\">Rotating Darkness</button>"
      "<button class=\"btn effect-btn\" onclick=\"setEffect(1)\">Portal Open</button>"
      "<button class=\"btn effect-btn\" onclick=\"setEffect(2)\">Battery Status</button>"
      "<button class=\"btn effect-btn\" onclick=\"setEffect(3)\">Random Blink</button>"
      "<button class=\"btn effect-btn\" onclick=\"setEffect(4)\">WiFi Mode</button>"
      "<h3>Brightness</h3>"
      "<input type=\"range\" id=\"brightness\" min=\"0\" max=\"255\" value=\"128\" onchange=\"setBrightness(this.value)\">"
      "<span id=\"brightnessValue\">128</span>"
      "<div class=\"status\" id=\"response\"></div>"
      "</div>"
      "<script>"
      "function setEffect(effect){"
      "fetch('/effect?effect='+effect)"
      ".then(response=>response.text())"
      ".then(data=>{document.getElementById('response').innerHTML='<p>Effect: '+data+'</p>';})"
      ".catch(error=>{document.getElementById('response').innerHTML='<p>Error: '+error+'</p>';});"
      "}"
      "function setBrightness(value){"
      "document.getElementById('brightnessValue').textContent=value;"
      "fetch('/brightness?value='+value)"
      ".then(response=>response.text())"
      ".then(data=>{document.getElementById('response').innerHTML='<p>Brightness: '+data+'</p>';})"
      ".catch(error=>{document.getElementById('response').innerHTML='<p>Error: '+error+'</p>';});"
      "}"
      "function updateBatteryStatus(){"
      "fetch('/battery')"
      ".then(response=>response.text())"
      ".then(data=>{document.getElementById('batteryStatus').innerHTML='<p>Battery: '+data.replace(/\\n/g,'<br>')+'</p>';})"
      ".catch(error=>{document.getElementById('batteryStatus').innerHTML='<p>Battery: Error reading status</p>';});"
      "}"
      "setInterval(updateBatteryStatus,5000);" // Update battery status every 5 seconds
      "</script></body></html>";

  // Use a smaller buffer since we optimized the HTML
  char response[4096];
  std::string status = self->inAPMode_ ? "Access Point Mode" : "Connected to WiFi";
  int ret = snprintf(response, sizeof(response), html_template, status.c_str(), self->get_ip_address().c_str());

  if (ret < 0 || ret >= sizeof(response))
  {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);

  return ESP_OK;
}

esp_err_t WiFiInputSource::handle_set_effect(httpd_req_t *req)
{
  WiFiInputSource *self = (WiFiInputSource *)req->user_ctx;

  char buf[128];
  int ret = httpd_req_get_url_query_str(req, buf, sizeof(buf));
  if (ret != ESP_OK)
  {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  char effect_str[5];
  if (httpd_query_key_value(buf, "effect", effect_str, sizeof(effect_str)) != ESP_OK)
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing effect parameter");
    return ESP_FAIL;
  }

  int effect = atoi(effect_str);
  if (effect >= 0 && effect < 5)
  {
    self->queue_event({.inputId = effect,
                       .type = EventType::Pressed,
                       .timestamp = esp_timer_get_time(),
                       .sourceName = "WiFi"});

    std::string response = "Effect set to: " + self->get_effect_name(effect);
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, response.c_str(), HTTPD_RESP_USE_STRLEN);
  }
  else
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid effect number (0-4)");
  }

  return ESP_OK;
}

esp_err_t WiFiInputSource::handle_set_brightness(httpd_req_t *req)
{
  WiFiInputSource *self = (WiFiInputSource *)req->user_ctx;

  char buf[128];
  int ret = httpd_req_get_url_query_str(req, buf, sizeof(buf));
  if (ret != ESP_OK)
  {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  char value_str[5];
  if (httpd_query_key_value(buf, "value", value_str, sizeof(value_str)) != ESP_OK)
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing value parameter");
    return ESP_FAIL;
  }

  int brightness = atoi(value_str);
  if (brightness >= 0 && brightness <= 255)
  {
    self->queue_event({.inputId = -brightness,
                       .type = EventType::Pressed,
                       .timestamp = esp_timer_get_time(),
                       .sourceName = "WiFi"});

    std::string response = "Brightness set to: " + std::to_string(brightness);
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, response.c_str(), HTTPD_RESP_USE_STRLEN);
  }
  else
  {
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid brightness value (0-255)");
  }

  return ESP_OK;
}

esp_err_t WiFiInputSource::handle_status(httpd_req_t *req)
{
  WiFiInputSource *self = (WiFiInputSource *)req->user_ctx;

  std::string status = "Outtatimers Controller Status\n";
  status += "WiFi Connected: " + std::string(self->isConnected_ ? "Yes" : "No") + "\n";
  status += "IP Address: " + self->get_ip_address() + "\n";
  status += "Available Commands:\n";
  status += "  GET / - Web interface\n";
  status += "  GET /effect?effect=0-5 - Change effect\n";
  status += "  GET /brightness?value=0-255 - Set brightness\n";
  status += "  GET /battery - Battery status\n";
  status += "  GET /status - This status\n";

  httpd_resp_set_type(req, "text/plain");
  httpd_resp_send(req, status.c_str(), HTTPD_RESP_USE_STRLEN);

  return ESP_OK;
}

esp_err_t WiFiInputSource::handle_battery(httpd_req_t *req)
{
  WiFiInputSource *self = (WiFiInputSource *)req->user_ctx;

  // TODO: Access actual battery voltage/percentage from main application
  // For now, return feature status
  std::string battery = "Battery Monitoring Active\n";
  battery += "ADC Channel: Configured on GPIO0\n";
  battery += "Voltage Divider: 1.0x ratio (direct reading)\n";
  battery += "Range: 3.0V - 4.2V\n";
  battery += "Note: Connect battery to BAT+ and BAT- pins\n";
  battery += "Status: Monitoring every 5 seconds\n";

  httpd_resp_set_type(req, "text/plain");
  httpd_resp_send(req, battery.c_str(), HTTPD_RESP_USE_STRLEN);

  return ESP_OK;
}

bool WiFiInputSource::init()
{
  ESP_LOGI(TAG, "Initializing WiFi hardware...");

  // Initialize NVS (required for WiFi)
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Initialize TCP/IP adapter
  ESP_ERROR_CHECK(esp_netif_init());

  // Initialize event loop
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // Create default event handlers
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, this));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, this));

  return true;
}

bool WiFiInputSource::startConnection(const char *ssid, const char *password)
{
  ESP_LOGI(TAG, "Starting WiFi connection...");

  // Start WiFi station
  ESP_ERROR_CHECK(start_wifi_station(ssid, password));

  // Setup web server
  ESP_ERROR_CHECK(setup_web_server());

  return true;
}

void WiFiInputSource::stopConnection()
{
  ESP_LOGI(TAG, "Stopping WiFi connection...");

  // Stop web server
  if (server_handle_)
  {
    httpd_stop(server_handle_);
    server_handle_ = nullptr;
    apServerStarted_ = false;
  }

  // Disconnect and stop WiFi
  if (isConnected_)
  {
    esp_wifi_disconnect();
    isConnected_ = false;
  }

  esp_wifi_stop();
  inAPMode_ = false;

  // Reset connection state
  connectionStartTime_ = 0;
  timeoutLogged_ = false;
}

esp_err_t WiFiInputSource::start_wifi_ap()
{
  ESP_LOGI(TAG, "Starting WiFi Access Point mode...");

  // Stop any existing station connection
  if (isConnected_)
  {
    esp_wifi_disconnect();
    isConnected_ = false;
  }

  // Create AP network interface
  esp_netif_create_default_wifi_ap();

  // WiFi configuration for Access Point
  wifi_config_t wifi_config = {};
  strncpy((char *)wifi_config.ap.ssid, "Outtatimers-Controller", sizeof(wifi_config.ap.ssid) - 1);
  wifi_config.ap.ssid[sizeof(wifi_config.ap.ssid) - 1] = '\0'; // Ensure null termination
  wifi_config.ap.ssid_len = 0;                                 // Use null-terminated string length
  wifi_config.ap.channel = 1;
  strncpy((char *)wifi_config.ap.password, "outtatimers123", sizeof(wifi_config.ap.password) - 1);
  wifi_config.ap.password[sizeof(wifi_config.ap.password) - 1] = '\0'; // Ensure null termination
  wifi_config.ap.max_connection = 4;
  wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
  wifi_config.ap.ssid_hidden = 0;

  // Copy password if provided
  if (strlen((char *)wifi_config.ap.password) > 0)
  {
    strncpy((char *)wifi_config.ap.password, "outtatimers123", sizeof(wifi_config.ap.password) - 1);
  }

  // Initialize WiFi in AP mode
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "WiFi Access Point started: SSID='Outtatimers-Controller'");
  ESP_LOGI(TAG, "Connect to access point and navigate to: http://192.168.4.1");

  return ESP_OK;
}