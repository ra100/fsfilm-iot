#pragma once

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "config.h"
#include <esp_timer.h>
#include <driver/gpio.h>
#include <esp_log.h>

/**
 * @brief Button state enumeration
 */
enum class ButtonState
{
  Released,
  Pressed,
  LongPress
};

/**
 * @brief Button event structure
 */
struct ButtonEvent
{
  uint8_t buttonId; // 0 for button1, 1 for button2
  ButtonState state;
  int64_t timestamp;
};

/**
 * @brief Button handler class for managing GPIO button inputs with debouncing
 */
class ButtonHandler
{
public:
  ButtonHandler();
  void begin();
  bool update(int64_t currentTime);
  bool hasEvents() const { return eventQueueSize_ > 0; }
  ButtonEvent getNextEvent();

  // Button state queries
  bool isButton1Pressed() const { return button1State_; }
  bool isButton2Pressed() const { return button2State_; }

private:
  static constexpr uint8_t MAX_EVENTS = 10;
  static constexpr uint8_t BUTTON1_ID = 0;
  static constexpr uint8_t BUTTON2_ID = 1;

  // Button states
  bool button1State_;
  bool button2State_;
  bool button1LastState_;
  bool button2LastState_;

  // Timing for debouncing and long press detection
  int64_t button1LastChange_;
  int64_t button2LastChange_;
  int64_t button1PressStart_;
  int64_t button2PressStart_;

  // Event queue
  ButtonEvent eventQueue_[MAX_EVENTS];
  uint8_t eventQueueHead_;
  uint8_t eventQueueTail_;
  uint8_t eventQueueSize_;

  // Long press threshold (2 seconds)
  static constexpr int64_t LONG_PRESS_THRESHOLD_US = 2000000;

  void addEvent(uint8_t buttonId, ButtonState state);
  ButtonState getButton1State(int64_t currentTime);
  ButtonState getButton2State(int64_t currentTime);
  bool readButton1();
  bool readButton2();

  static constexpr const char *TAG = "ButtonHandler";
};

#endif // BUTTON_HANDLER_H