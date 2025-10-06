#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "led_driver.h"

// Create FastLED driver instance using the working Portal pattern
static FastLEDDriver<ControllerConfig::Hardware::NUM_LEDS> ledDriver(ControllerConfig::Hardware::LED_PIN);

// Effect Management
enum EffectMode
{
  SOLID_COLOR,
  RAINBOW_CYCLE,
  PULSE,
  CHASE,
  TWINKLE,
  FIRE,
  EFFECT_COUNT
};

EffectMode currentEffect = SOLID_COLOR;
uint8_t effectIntensity = 128; // 0-255 brightness
uint8_t hue = 0;               // Color hue for effects
unsigned long lastEffectChange = 0;
unsigned long lastButtonPress = 0;
const unsigned long DEBOUNCE_DELAY = ControllerConfig::Timing::DEBOUNCE_DELAY; // Debounce time in ms

// Button state tracking
bool button1Pressed = false;
bool button2Pressed = false;

// Function declarations
void nextEffect();
void modifyEffect();
void updateLEDs();
void showEffectSolidColor();
void showEffectRainbowCycle();
void showEffectPulse();
void showEffectChase();
void showEffectTwinkle();
void showEffectFire();

void setup()
{
  // Initialize serial for debugging
  Serial.begin(115200);
  Serial.println("Outtatimers Controller Starting...");

  // Initialize buttons with pull-up resistors
  pinMode(ControllerConfig::Hardware::BUTTON1_PIN, INPUT_PULLUP);
  pinMode(ControllerConfig::Hardware::BUTTON2_PIN, INPUT_PULLUP);

  // Initialize FastLED driver using Portal pattern
  ledDriver.begin();

  // Initial LED test - light up first LED to show ready state
  ledDriver.setPixel(0, CRGB::Green);
  ledDriver.show();
  delay(1000);

  // Clear LEDs and start with first effect
  ledDriver.clear();
  ledDriver.show();

  Serial.println("Setup complete");
}

void loop()
{
  unsigned long currentTime = millis();

  // Handle button inputs with debouncing
  bool button1State = digitalRead(ControllerConfig::Hardware::BUTTON1_PIN) == LOW;
  bool button2State = digitalRead(ControllerConfig::Hardware::BUTTON2_PIN) == LOW;

  // Button 1: Cycle through effects
  if (button1State && !button1Pressed && (currentTime - lastButtonPress) > DEBOUNCE_DELAY)
  {
    nextEffect();
    button1Pressed = true;
    lastButtonPress = currentTime;
    Serial.print("Effect changed to: ");
    Serial.println(currentEffect);
  }

  // Button 2: Modify current effect (intensity/hue)
  if (button2State && !button2Pressed && (currentTime - lastButtonPress) > DEBOUNCE_DELAY)
  {
    modifyEffect();
    button2Pressed = true;
    lastButtonPress = currentTime;
    Serial.print("Effect modified - Intensity: ");
    Serial.println(effectIntensity);
  }

  // Reset button states when released
  if (!button1State)
    button1Pressed = false;
  if (!button2State)
    button2Pressed = false;

  // Update LEDs based on current effect
  updateLEDs();

  // Small delay for smooth operation
  delay(ControllerConfig::Timing::EFFECT_UPDATE_INTERVAL);
}

void nextEffect()
{
  currentEffect = static_cast<EffectMode>((currentEffect + 1) % EFFECT_COUNT);

  // Visual feedback - flash first LED white, then return to effect
  ledDriver.setPixel(0, CRGB::White);
  ledDriver.show();
  delay(100);
}

void modifyEffect()
{
  // Cycle through intensity levels
  effectIntensity = (effectIntensity + ControllerConfig::Effects::BRIGHTNESS_STEP) % 256;
  if (effectIntensity == 0)
    effectIntensity = ControllerConfig::Effects::MIN_BRIGHTNESS; // Minimum brightness

  ledDriver.setBrightness(effectIntensity);

  // Visual feedback - flash first LED yellow
  ledDriver.setPixel(0, CRGB::Yellow);
  ledDriver.show();
  delay(100);
}

void updateLEDs()
{
  // Update hue for animated effects
  hue++;

  switch (currentEffect)
  {
  case SOLID_COLOR:
    showEffectSolidColor();
    break;
  case RAINBOW_CYCLE:
    showEffectRainbowCycle();
    break;
  case PULSE:
    showEffectPulse();
    break;
  case CHASE:
    showEffectChase();
    break;
  case TWINKLE:
    showEffectTwinkle();
    break;
  case FIRE:
    showEffectFire();
    break;
  default:
    // Handle any unexpected effect modes
    showEffectSolidColor();
    break;
  }

  FastLED.show();
}

void showEffectSolidColor()
{
  CRGB color = CHSV(hue, 255, 255);
  ledDriver.fillSolid(color);
}

void showEffectRainbowCycle()
{
  // Create linear rainbow effect across the strip
  for (int i = 0; i < ControllerConfig::Hardware::NUM_LEDS; i++)
  {
    ledDriver.setPixel(i, CHSV(hue + (i * 28), 255, 255)); // 28 degree steps for 9 LEDs
  }
}

void showEffectPulse()
{
  uint8_t brightness = beatsin8(30, 32, 255); // Pulse at 30 BPM
  CRGB color = CHSV(hue, 255, brightness);
  ledDriver.fillSolid(color);
}

void showEffectChase()
{
  static uint8_t chasePos = 0;
  ledDriver.fillSolid(CRGB::Black);

  // Create a linear chase effect with 3 LEDs lit at a time
  for (int i = 0; i < 3; i++)
  {
    int pos = (chasePos + i) % ControllerConfig::Hardware::NUM_LEDS;
    ledDriver.setPixel(pos, CHSV(hue, 255, 255));
  }

  if (hue % 10 == 0)
  {
    chasePos = (chasePos + 1) % ControllerConfig::Hardware::NUM_LEDS;
  }
}

void showEffectTwinkle()
{
  static unsigned long lastTwinkle = 0;
  static bool twinkleState = false;

  if (millis() - lastTwinkle > 200)
  {
    twinkleState = !twinkleState;
    lastTwinkle = millis();

    if (twinkleState)
    {
      // Randomly light some LEDs in the strip
      for (int i = 0; i < ControllerConfig::Hardware::NUM_LEDS; i++)
      {
        if (random(4) == 0)
        { // 1 in 4 chance for more selective twinkling
          ledDriver.setPixel(i, CHSV(random(256), 255, 255));
        }
        else
        {
          ledDriver.setPixel(i, CRGB::Black);
        }
      }
    }
    else
    {
      ledDriver.fillSolid(CRGB::Black);
    }
  }
}

void showEffectFire()
{
  // Simulate fire effect with warm colors across the strip
  for (int i = 0; i < ControllerConfig::Hardware::NUM_LEDS; i++)
  {
    uint8_t fireHue = beatsin8(20, 10, 40); // Orange to red range
    uint8_t fireBrightness = beatsin8(30, 128, 255);
    ledDriver.setPixel(i, CHSV(fireHue, 255, fireBrightness));
  }
}

// Note: setAllLEDs function removed - using ledDriver.fillSolid() instead
