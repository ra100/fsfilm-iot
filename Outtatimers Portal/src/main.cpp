#include <FastLED.h>
#include "effects.h"
#include "portal_effect.h"
#include "led_driver.h"
#include "debounce.h"
#include "config.h"
#include "startup_sequence.h"
#include "input_manager.h"
#include "status_led.h"
#include "config_manager.h"
#if ENABLE_WIFI_CONTROL
#include "wifi_input_source.h"
#endif

// LED Strip Configuration - using config constants
#define LED_TYPE WS2812B

// Static driver and portal effect using template-based class
static FastLEDDriver<PortalConfig::Hardware::NUM_LEDS> fastDriver(PortalConfig::Hardware::LED_PIN);
static PortalEffectTemplate<PortalConfig::Hardware::NUM_LEDS, PortalConfig::Effects::GRADIENT_STEP_DEFAULT, PortalConfig::Effects::GRADIENT_MOVE_DEFAULT> portal(&fastDriver);
// Application state
bool portalRunning = false;

// System components
StartupSequence startupSequence;
InputManager inputManager;
ButtonInputSource buttonInput(nullptr, 0); // Will be initialized in setup()

#if ENABLE_WIFI_CONTROL
WiFiInputSource wifiInput(PortalConfig::WiFi::HTTP_PORT);
#endif

// Button configuration
const ButtonInputSource::ButtonConfig buttonConfigs[] = {
    {.pin = PortalConfig::Hardware::BUTTON1_PIN,
     .inputId = static_cast<int>(InputManager::Command::TogglePortal),
     .activeLow = true,
     .debounceMs = PortalConfig::Timing::DEBOUNCE_INTERVAL_MS,
     .name = "Button1_Portal"},
    {.pin = PortalConfig::Hardware::BUTTON2_PIN,
     .inputId = static_cast<int>(InputManager::Command::TriggerMalfunction),
     .activeLow = true,
     .debounceMs = PortalConfig::Timing::DEBOUNCE_INTERVAL_MS,
     .name = "Button2_Malfunction"},
    {.pin = PortalConfig::Hardware::BUTTON3_PIN,
     .inputId = static_cast<int>(InputManager::Command::FadeOut),
     .activeLow = true,
     .debounceMs = PortalConfig::Timing::DEBOUNCE_INTERVAL_MS,
     .name = "Button3_FadeOut"}};

// PortalEffect encapsulates malfunction and gradient logic now.

/**
 * @brief Handle input commands from any source (buttons, WiFi, etc.)
 * @param command The logical command to execute
 * @param source Name of the input source for logging
 */
void handleInputCommand(InputManager::Command command, const char *source)
{
  Serial.print("Input from ");
  Serial.print(source);
  Serial.print(": ");
  Serial.println(InputManager::getCommandName(command));

  switch (command)
  {
  case InputManager::Command::TogglePortal:
    portalRunning = !portalRunning;
    if (portalRunning)
    {
      portal.start();
      Serial.println("Animation STARTED - Portal effect active (fade in)");
    }
    else
    {
      portal.stop();
      Serial.println("Animation STOPPED");
    }
    break;

  case InputManager::Command::TriggerMalfunction:
    Serial.println("Portal MALFUNCTION triggered!");
    portal.triggerMalfunction();
    break;

  case InputManager::Command::FadeOut:
    Serial.println("Fade out triggered");
    portal.triggerFadeOut();
    break;

  default:
    Serial.print("Unknown command: ");
    Serial.println(static_cast<int>(command));
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  srand(millis()); // Seed random for uniform distribution
  Serial.println("WS2812 Traveling Light Test Starting...");

  // Initialize status LED
  StatusLED::begin();

  // Initialize portal effect (which initializes LEDs)
  portal.begin();

  // Initialize startup sequence
  startupSequence.begin(&fastDriver);

  // Initialize configuration manager
  ConfigManager::begin();

  // Initialize input system
  buttonInput = ButtonInputSource(buttonConfigs, 3);
  inputManager.addInputSource(&buttonInput);

#if ENABLE_WIFI_CONTROL
  // Initialize WiFi input source (non-blocking)
  wifiInput.begin(PortalConfig::WiFi::DEFAULT_SSID, PortalConfig::WiFi::DEFAULT_PASSWORD);
  inputManager.addInputSource(&wifiInput);
  Serial.println("WiFi input source initialized - attempting connection in background");
  Serial.println("WiFi commands available:");
  Serial.println("  http://[ip]/toggle - Toggle portal effect");
  Serial.println("  http://[ip]/malfunction - Trigger malfunction");
  Serial.println("  http://[ip]/fadeout - Fade out effect");
  Serial.println("  http://[ip]/status - View status");
  Serial.println("  http://[ip]/config - View configuration");
#endif

  inputManager.setInputCallback(handleInputCommand);

  Serial.println("Setup started; running non-blocking startup diagnostics...");
  Serial.println("Button commands available:");
  Serial.println("  Button 1: Toggle portal effect");
  Serial.println("  Button 2: Trigger malfunction");
  Serial.println("  Button 3: Fade out");
  Serial.print("Total LEDs: ");
  Serial.println(PortalConfig::Hardware::NUM_LEDS);
  Serial.print("Circle radius: ");
  Serial.print(PortalConfig::Hardware::NUM_LEDS / (2.0 * PortalConfig::Math::PI_F));
  Serial.println(" LEDs");
}

void loop()
{
  unsigned long now = millis();

  // Handle non-blocking startup diagnostics
  if (!startupSequence.isComplete())
  {
    if (startupSequence.update(now))
    {
      // State changed - log it
      Serial.print("Startup: ");
      Serial.println(startupSequence.getStateString());

      if (startupSequence.isComplete())
      {
        Serial.println("Setup complete.");
      }
    }
    return; // Don't process inputs during startup
  }

  // Process all input sources (buttons, WiFi, etc.)
  inputManager.update(now);

  // Run effects
  portal.update(now);
}
