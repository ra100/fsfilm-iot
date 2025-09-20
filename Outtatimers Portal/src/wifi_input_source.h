#pragma once

#include "input_manager.h"
#include "status_led.h"
#include "config_manager.h"

#ifndef UNIT_TEST
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#else
// Mock classes for unit testing
class ESP8266WebServer
{
public:
  ESP8266WebServer(int port) {}
  void begin() {}
  void handleClient() {}
  void on(const char *path, std::function<void()> handler) {}
  void send(int code, const char *type, const char *content) {}
  bool hasArg(const char *name) { return false; }
  String arg(const char *name) { return ""; }
};
#endif

/**
 * @brief WiFi-based input source for remote control
 *
 * This input source creates a simple web server that accepts HTTP requests
 * to trigger portal commands. This demonstrates how the InputManager system
 * can be extended with new input sources without modifying existing code.
 *
 * @example
 * ```cpp
 * WiFiInputSource wifiInput(80);  // HTTP server on port 80
 * wifiInput.begin("MyPortal", "password123");
 * inputManager.addInputSource(&wifiInput);
 *
 * // Now you can control via HTTP:
 * // http://portal-ip/toggle
 * // http://portal-ip/malfunction
 * // http://portal-ip/fadeout
 * ```
 */
class WiFiInputSource : public IInputSource
{
public:
  /**
   * @brief Construct a new WiFiInputSource
   * @param port HTTP server port (default: 80)
   */
  explicit WiFiInputSource(int port = 80)
      : server_(port), eventQueueHead_(0), eventQueueTail_(0), isConnected_(false) {}

  /**
   * @brief Initialize WiFi and start web server
   * @param ssid WiFi network name
   * @param password WiFi password
   * @return true if initialization successful
   */
  bool begin(const char *ssid, const char *password)
  {
#ifndef UNIT_TEST
    // Initialize status LED
    StatusLED::begin();

    // Start WiFi connection (non-blocking)
    WiFi.begin(ssid, password);
    StatusLED::update(PortalConfig::Hardware::WiFiStatus::CONNECTING_STA, millis());

    // Don't wait for connection - let it happen in the background
    // The update() method will handle connection status and server startup

    // Initialize LittleFS filesystem for serving web assets (modern replacement for SPIFFS with better wear-leveling)
    if (!LittleFS.begin())
    {
      Serial.println(F("LittleFS mount failed - check flash partitioning and available space"));
      StatusLED::update(PortalConfig::Hardware::WiFiStatus::STARTED_NOT_CONNECTED, millis());
      return false;
    }

    Serial.println(F("LittleFS mounted successfully"));

    // Set up web server routes
    server_.on("/", [this]()
               { handleRoot(); });
    server_.on("/toggle", [this]()
               { handleCommand(InputManager::Command::TogglePortal); });
    server_.on("/malfunction", [this]()
               { handleCommand(InputManager::Command::TriggerMalfunction); });
    server_.on("/fadeout", [this]()
               { handleCommand(InputManager::Command::FadeOut); });
    server_.on("/status", [this]()
               { handleStatus(); });
    server_.on("/config", [this]()
               { handleConfig(); });
    server_.on("/set_speed", [this]()
               { handleSetSpeed(); });
    server_.on("/set_brightness", [this]()
               { handleSetBrightness(); });
    server_.on("/set_hue", [this]()
               { handleSetHue(); });
    server_.on("/set_mode", [this]()
               { handleSetMode(); });
    server_.on("/options", HTTP_OPTIONS, [this]()
               {
        server_.sendHeader("Access-Control-Allow-Origin", "*");
        server_.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        server_.sendHeader("Access-Control-Allow-Headers", "*");
        server_.send(200, "text/plain", ""); });
    server_.onNotFound([this]()
                       {
        server_.sendHeader("Access-Control-Allow-Origin", "*");
        server_.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        server_.sendHeader("Access-Control-Allow-Headers", "*");
        server_.send(404, "text/plain", "Not Found"); });

    server_.begin();
#endif

    return true;
  }

  bool update(unsigned long currentTime) override
  {
#ifndef UNIT_TEST
    // Check WiFi connection status and start server if connected
    if (!isConnected_)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        // WiFi connected - start the server
        isConnected_ = true;
        StatusLED::update(PortalConfig::Hardware::WiFiStatus::STA_CONNECTED, currentTime);

        // Initialize LittleFS filesystem for serving web assets
        if (!LittleFS.begin())
        {
          Serial.println(F("LittleFS mount failed - check flash partitioning and available space"));
          StatusLED::update(PortalConfig::Hardware::WiFiStatus::STARTED_NOT_CONNECTED, currentTime);
          isConnected_ = false;
          return hasEvents();
        }

        Serial.println(F("LittleFS mounted successfully"));

        // Set up web server routes
        server_.on("/", [this]()
                   { handleRoot(); });
        server_.on("/toggle", [this]()
                   { handleCommand(InputManager::Command::TogglePortal); });
        server_.on("/malfunction", [this]()
                   { handleCommand(InputManager::Command::TriggerMalfunction); });
        server_.on("/fadeout", [this]()
                   { handleCommand(InputManager::Command::FadeOut); });
        server_.on("/status", [this]()
                   { handleStatus(); });
        server_.on("/config", [this]()
                   { handleConfig(); });
        server_.on("/set_speed", [this]()
                   { handleSetSpeed(); });
        server_.on("/set_brightness", [this]()
                   { handleSetBrightness(); });
        server_.on("/set_hue", [this]()
                   { handleSetHue(); });
        server_.on("/set_mode", [this]()
                   { handleSetMode(); });
        server_.on("/options", HTTP_OPTIONS, [this]()
                   {
           server_.sendHeader("Access-Control-Allow-Origin", "*");
           server_.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
           server_.sendHeader("Access-Control-Allow-Headers", "*");
           server_.send(200, "text/plain", ""); });
        server_.onNotFound([this]()
                           {
           server_.sendHeader("Access-Control-Allow-Origin", "*");
           server_.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
           server_.sendHeader("Access-Control-Allow-Headers", "*");
           server_.send(404, "text/plain", "Not Found"); });

        server_.begin();
        Serial.print("WiFi connected! Web interface available at: http://");
        Serial.println(WiFi.localIP().toString().c_str());
        Serial.println("WiFi commands available:");
        Serial.println("  http://[ip]/toggle - Toggle portal effect");
        Serial.println("  http://[ip]/malfunction - Trigger malfunction");
        Serial.println("  http://[ip]/fadeout - Fade out effect");
      }
      else
      {
        // Still connecting - update status LED
        StatusLED::update(PortalConfig::Hardware::WiFiStatus::CONNECTING_STA, currentTime);
      }
    }
    else
    {
      // WiFi is connected - handle web server
      server_.handleClient();

      // Check if there are any connected clients (stations)
      int numClients = WiFi.softAPgetStationNum();
      bool hasClients = numClients > 0;

      // Update status LED based on connection and client status
      if (hasClients)
      {
        StatusLED::update(PortalConfig::Hardware::WiFiStatus::AP_WITH_CLIENTS, currentTime);
      }
      else if (WiFi.getMode() & WIFI_STA)
      {
        // STA mode
        StatusLED::update(PortalConfig::Hardware::WiFiStatus::STA_CONNECTED, currentTime);
      }
      else
      {
        // AP mode without clients
        StatusLED::update(PortalConfig::Hardware::WiFiStatus::AP_MODE, currentTime);
      }
    }
#endif
    return hasEvents();
  }

  bool hasEvents() const override
  {
    return eventQueueHead_ != eventQueueTail_;
  }

  InputEvent getNextEvent() override
  {
    if (!hasEvents())
    {
      return {0, EventType::Released, 0, "none"};
    }

    InputEvent event = eventQueue_[eventQueueHead_];
    eventQueueHead_ = (eventQueueHead_ + 1) % MAX_EVENTS;
    return event;
  }

  const char *getSourceName() const override
  {
    return "WiFiInput";
  }

  /**
   * @brief Get the WiFi IP address
   * @return IP address as string, or "Not Connected" if not connected
   */
  const char *getIPAddress() const
  {
#ifndef UNIT_TEST
    if (isConnected_)
    {
      static String ipStr = WiFi.localIP().toString();
      return ipStr.c_str();
    }
#endif
    return "Not Connected";
  }

  /**
   * @brief Check if WiFi is connected
   * @return true if connected to WiFi
   */
  bool isConnected() const
  {
    return isConnected_;
  }

private:
  static constexpr int MAX_EVENTS = 8;

  ESP8266WebServer server_;
  InputEvent eventQueue_[MAX_EVENTS];
  int eventQueueHead_;
  int eventQueueTail_;
  bool isConnected_;

  /**
   * @brief Send CORS headers for all responses
   */
  void sendCORSHeaders()
  {
    server_.sendHeader("Access-Control-Allow-Origin", "*");
    server_.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    server_.sendHeader("Access-Control-Allow-Headers", "*");
  }

  /**
   * @brief Read file content from LittleFS
   * @param path File path to read
   * @return File content as String
   */
  String readFile(const char *path)
  {
#ifndef UNIT_TEST
    File file = LittleFS.open(path, "r");
    if (!file)
    {
      return "File not found";
    }

    String content = "";
    while (file.available())
    {
      content += (char)file.read();
    }
    file.close();
    return content;
#else
    return "File reading not supported in unit test mode";
#endif
  }

  /**
   * @brief Handle root web page request
   */
  void handleRoot()
  {
#ifndef UNIT_TEST
    // Serve the HTML file from the data directory
    sendCORSHeaders();
    server_.send(200, "text/html", readFile("/index.html"));
#else
    // In unit test mode, return a simple response
    sendCORSHeaders();
    server_.send(200, "text/plain", "Web interface not available in unit test mode");
#endif
  }

  /**
   * @brief Handle command requests
   * @param command Command to execute
   */
  void handleCommand(InputManager::Command command)
  {
    // Queue the event
    queueEvent({.inputId = static_cast<int>(command),
                .type = EventType::Pressed,
                .timestamp = millis(),
                .sourceName = "WiFi"});

    // Send response
    String response = "Command executed: ";
    response += InputManager::getCommandName(command);

    sendCORSHeaders();
    server_.send(200, "text/plain", response);
  }

  /**
   * @brief Handle status request
   */
  void handleStatus()
  {
    String status = "Portal Controller Status\n";
    status += "WiFi Connected: Yes\n";
    status += "IP Address: ";
    status += getIPAddress();
    status += "\n";
    status += "Available Commands:\n";
    status += "  /toggle - Toggle portal effect\n";
    status += "  /malfunction - Trigger malfunction\n";
    status += "  /fadeout - Fade out effect\n";
    status += "  /config - View current configuration\n";
    status += "  /set_speed?speed=0-10 - Set rotation speed\n";
    status += "  /set_brightness?brightness=0-255 - Set max brightness\n";
    status += "  /set_hue?min=0-255&max=0-255 - Set color hue range\n";

    sendCORSHeaders();
    server_.send(200, "text/plain", status);
  }

  /**
   * @brief Handle configuration request
   */
  void handleConfig()
  {
    String json = "{";
    json += "\"speed\":" + String(ConfigManager::getRotationSpeed()) + ",";
    json += "\"brightness\":" + String(ConfigManager::getMaxBrightness()) + ",";
    json += "\"hueMin\":" + String(ConfigManager::getHueMin()) + ",";
    json += "\"hueMax\":" + String(ConfigManager::getHueMax()) + ",";
    json += "\"mode\":" + String(ConfigManager::getPortalMode());
    json += "}";

    sendCORSHeaders();
    server_.send(200, "application/json", json);
  }

  /**
   * @brief Handle set speed request
   */
  void handleSetSpeed()
  {
    if (server_.hasArg("speed"))
    {
      int speed = server_.arg("speed").toInt();
      ConfigManager::setRotationSpeed(speed);
      String response = "Rotation speed set to: " + String(speed) + " (0-10)";
      sendCORSHeaders();
      server_.send(200, "text/plain", response);
    }
    else
    {
      sendCORSHeaders();
      server_.send(400, "text/plain", "Missing speed parameter");
    }
  }

  /**
   * @brief Handle set brightness request
   */
  void handleSetBrightness()
  {
    if (server_.hasArg("brightness"))
    {
      int brightness = server_.arg("brightness").toInt();
      ConfigManager::setMaxBrightness(brightness);
      String response = "Max brightness set to: " + String(brightness) + " (0-255)";
      sendCORSHeaders();
      server_.send(200, "text/plain", response);
    }
    else
    {
      sendCORSHeaders();
      server_.send(400, "text/plain", "Missing brightness parameter");
    }
  }

  /**
   * @brief Handle set hue range request
   */
  void handleSetHue()
  {
    if (server_.hasArg("min") && server_.hasArg("max"))
    {
      int minHue = server_.arg("min").toInt();
      int maxHue = server_.arg("max").toInt();
      ConfigManager::setHueMin(minHue);
      ConfigManager::setHueMax(maxHue);
      String response = "Color hue range set to: " + String(minHue) + " - " + String(maxHue) + " (0-255)";
      sendCORSHeaders();
      server_.send(200, "text/plain", response);
    }
    else
    {
      sendCORSHeaders();
      server_.send(400, "text/plain", "Missing min or max parameter");
    }
  }

  /**
   * @brief Add event to queue
   * @param event Event to queue
   */
  /**
   * @brief Handle set mode request
   */
  void handleSetMode()
  {
    if (server_.hasArg("mode"))
    {
      int mode = server_.arg("mode").toInt();
      ConfigManager::setPortalMode(mode);
      String response = "Portal mode set to: " + String(mode == 0 ? "Classic" : "Virtual Gradients");
      sendCORSHeaders();
      server_.send(200, "text/plain", response);
    }
    else
    {
      sendCORSHeaders();
      server_.send(400, "text/plain", "Missing mode parameter");
    }
  }

  /**
   * @brief Add event to queue
   * @param event Event to queue
   */
  void queueEvent(const InputEvent &event)
  {
    int nextTail = (eventQueueTail_ + 1) % MAX_EVENTS;
    if (nextTail != eventQueueHead_)
    { // Don't overflow
      eventQueue_[eventQueueTail_] = event;
      eventQueueTail_ = nextTail;
    }
  }
};