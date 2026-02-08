#include <FastLED.h>
#include "effects.h"
#include "turbolift_effect.h"
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

// Static driver and turbolift effect using template-based class
static FastLEDDriver<TurboliftConfig::Hardware::NUM_LEDS> fastDriver(TurboliftConfig::Hardware::LED_PIN);
static TurboliftEffectTemplate<TurboliftConfig::Hardware::NUM_LEDS, TurboliftConfig::Effects::GRADIENT_STEP_DEFAULT, TurboliftConfig::Effects::GRADIENT_MOVE_DEFAULT> turbolift(&fastDriver);
// Application state
bool turboliftRunning = false;

// System components
StartupSequence startupSequence;
InputManager inputManager;
ButtonInputSource buttonInput(nullptr, 0); // Will be initialized in setup()

#if ENABLE_WIFI_CONTROL
WiFiInputSource wifiInput(TurboliftConfig::WiFi::HTTP_PORT);
#endif

// Button configuration
const ButtonInputSource::ButtonConfig buttonConfigs[] = {
    {.pin = TurboliftConfig::Hardware::BUTTON1_PIN,
     .inputId = static_cast<int>(InputManager::Command::ToggleTurbolift),
     .activeLow = true,
     .debounceMs = TurboliftConfig::Timing::DEBOUNCE_INTERVAL_MS,
     .name = "Button1_Turbolift"},
    {.pin = TurboliftConfig::Hardware::BUTTON2_PIN,
     .inputId = static_cast<int>(InputManager::Command::TriggerMalfunction),
     .activeLow = true,
     .debounceMs = TurboliftConfig::Timing::DEBOUNCE_INTERVAL_MS,
     .name = "Button2_Malfunction"},
    {.pin = TurboliftConfig::Hardware::BUTTON3_PIN,
     .inputId = static_cast<int>(InputManager::Command::FadeOut),
     .activeLow = true,
     .debounceMs = TurboliftConfig::Timing::DEBOUNCE_INTERVAL_MS,
     .name = "Button3_FadeOut"}};

// TurboliftEffect encapsulates malfunction and gradient logic now.

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
  case InputManager::Command::ToggleTurbolift:
    turboliftRunning = !turboliftRunning;
    if (turboliftRunning)
    {
      turbolift.start();
      Serial.println("Animation STARTED - Turbolift effect active (fade in)");
    }
    else
    {
      turbolift.stop();
      Serial.println("Animation STOPPED");
    }
    break;

  case InputManager::Command::TriggerMalfunction:
    Serial.println("Turbolift MALFUNCTION triggered!");
    turbolift.triggerMalfunction();
    break;

  case InputManager::Command::FadeOut:
    Serial.println("Fade out triggered");
    turbolift.triggerFadeOut();
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

  // Initialize turbolift effect (which initializes LEDs)
  turbolift.begin();

  // Initialize startup sequence
  startupSequence.begin(&fastDriver);

  // Initialize configuration manager
  ConfigManager::begin();

  // Initialize input system
  buttonInput = ButtonInputSource(buttonConfigs, 3);
  inputManager.addInputSource(&buttonInput);

#if ENABLE_WIFI_CONTROL
  // Initialize WiFi input source (non-blocking)
  wifiInput.begin(TurboliftConfig::WiFi::DEFAULT_SSID, TurboliftConfig::WiFi::DEFAULT_PASSWORD);
  inputManager.addInputSource(&wifiInput);
  Serial.println("WiFi input source initialized - attempting connection in background");
  Serial.println("WiFi commands available:");
  Serial.println("  http://[ip]/toggle - Toggle turbolift effect");
  Serial.println("  http://[ip]/malfunction - Trigger malfunction");
  Serial.println("  http://[ip]/fadeout - Fade out effect");
  Serial.println("  http://[ip]/status - View status");
  Serial.println("  http://[ip]/config - View configuration");
#endif

  inputManager.setInputCallback(handleInputCommand);

  Serial.println("Setup started; running non-blocking startup diagnostics...");
  Serial.println("Button commands available:");
  Serial.println("  Button 1: Toggle turbolift effect");
  Serial.println("  Button 2: Trigger malfunction");
  Serial.println("  Button 3: Fade out");
  Serial.print("Total LEDs: ");
  Serial.println(TurboliftConfig::Hardware::NUM_LEDS);
  Serial.print("Circle radius: ");
  Serial.print(TurboliftConfig::Hardware::NUM_LEDS / (2.0 * TurboliftConfig::Math::PI_F));
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
  turbolift.update(now);
}
