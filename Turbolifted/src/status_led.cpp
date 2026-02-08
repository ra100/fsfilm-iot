/**
 * @file status_led.cpp
 * @brief Implementation of status LED manager
 */

#include "status_led.h"

bool StatusLED::initialized_ = false;
unsigned long StatusLED::lastToggleTime_ = 0;
bool StatusLED::ledState_ = false;
TurboliftConfig::Hardware::WiFiStatus StatusLED::currentStatus_ = TurboliftConfig::Hardware::WiFiStatus::STARTED_NOT_CONNECTED;
unsigned long StatusLED::cycleStartTime_ = 0; // For cycle-based blinking

void StatusLED::begin()
{
  if (!initialized_)
  {
    pinMode(TurboliftConfig::Hardware::STATUS_LED_PIN, OUTPUT);
    setLED(false); // Start with LED off
    initialized_ = true;
    currentStatus_ = TurboliftConfig::Hardware::WiFiStatus::STARTED_NOT_CONNECTED;
  }
}

void StatusLED::update(TurboliftConfig::Hardware::WiFiStatus status, unsigned long currentTime)
{
  if (!initialized_)
  {
    begin();
  }

  // If status changed, reset the blink state
  if (status != currentStatus_)
  {
    currentStatus_ = status;
    lastToggleTime_ = currentTime;
    cycleStartTime_ = currentTime;
    ledState_ = false;
    setLED(false);
  }

  // Handle LED based on status
  switch (status)
  {
  case TurboliftConfig::Hardware::WiFiStatus::STARTED_NOT_CONNECTED:
  case TurboliftConfig::Hardware::WiFiStatus::CONNECTING_STA:
  case TurboliftConfig::Hardware::WiFiStatus::AP_MODE:
  {
    // Standard blinking for these states
    unsigned long interval = getBlinkInterval(status);
    if (currentTime - lastToggleTime_ >= interval)
    {
      ledState_ = !ledState_;
      setLED(ledState_);
      lastToggleTime_ = currentTime;
    }
    break;
  }
  case TurboliftConfig::Hardware::WiFiStatus::STA_CONNECTED:
  {
    // STA connected: LED on, short off every 5s
    if (currentTime - cycleStartTime_ >= TurboliftConfig::Timing::STATUS_LED_STA_CONNECTED_CYCLE_MS)
    {
      // Cycle reset: turn on
      ledState_ = true;
      setLED(true);
      cycleStartTime_ = currentTime;
    }
    else if (currentTime - cycleStartTime_ >= (TurboliftConfig::Timing::STATUS_LED_STA_CONNECTED_CYCLE_MS - TurboliftConfig::Timing::STATUS_LED_SHORT_OFF_MS))
    {
      // Short off period
      ledState_ = false;
      setLED(false);
    }
    else if (currentTime - cycleStartTime_ >= TurboliftConfig::Timing::STATUS_LED_SHORT_OFF_MS)
    {
      // Turn on after short off
      ledState_ = true;
      setLED(true);
    }
    break;
  }
  case TurboliftConfig::Hardware::WiFiStatus::STA_CONNECTED_CLIENTS:
  case TurboliftConfig::Hardware::WiFiStatus::AP_WITH_CLIENTS:
  {
    // Clients connected: LED on, short off every 10s
    if (currentTime - cycleStartTime_ >= TurboliftConfig::Timing::STATUS_LED_AP_CLIENTS_CYCLE_MS)
    {
      // Cycle reset: turn on
      ledState_ = true;
      setLED(true);
      cycleStartTime_ = currentTime;
    }
    else if (currentTime - cycleStartTime_ >= (TurboliftConfig::Timing::STATUS_LED_AP_CLIENTS_CYCLE_MS - TurboliftConfig::Timing::STATUS_LED_SHORT_OFF_MS))
    {
      // Short off period
      ledState_ = false;
      setLED(false);
    }
    else if (currentTime - cycleStartTime_ >= TurboliftConfig::Timing::STATUS_LED_SHORT_OFF_MS)
    {
      // Turn on after short off
      ledState_ = true;
      setLED(true);
    }
    break;
  }
  default:
    // Unknown status: LED off
    setLED(false);
    ledState_ = false;
    break;
  }
}

void StatusLED::off()
{
  if (initialized_)
  {
    setLED(false);
    ledState_ = false;
    currentStatus_ = TurboliftConfig::Hardware::WiFiStatus::STARTED_NOT_CONNECTED;
  }
}

void StatusLED::setLED(bool state)
{
  if (TurboliftConfig::Hardware::STATUS_LED_ACTIVE_LOW)
  {
    digitalWrite(TurboliftConfig::Hardware::STATUS_LED_PIN, state ? LOW : HIGH);
  }
  else
  {
    digitalWrite(TurboliftConfig::Hardware::STATUS_LED_PIN, state ? HIGH : LOW);
  }
}

unsigned long StatusLED::getBlinkInterval(TurboliftConfig::Hardware::WiFiStatus status)
{
  switch (status)
  {
  case TurboliftConfig::Hardware::WiFiStatus::CONNECTING_STA:
    return TurboliftConfig::Timing::STATUS_LED_FAST_BLINK_MS;
  case TurboliftConfig::Hardware::WiFiStatus::AP_MODE:
    return TurboliftConfig::Timing::STATUS_LED_AP_BLINK_MS;
  case TurboliftConfig::Hardware::WiFiStatus::STARTED_NOT_CONNECTED:
    return TurboliftConfig::Timing::STATUS_LED_STARTED_BLINK_MS;
  default:
    return TurboliftConfig::Timing::STATUS_LED_BLINK_INTERVAL_MS;
  }
}