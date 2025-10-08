#include "button_handler.h"

ButtonHandler::ButtonHandler()
    : button1State_(false), button2State_(false),
      button1LastState_(true), button2LastState_(true), // Start with true (pulled up)
      button1LastChange_(0), button2LastChange_(0),
      button1PressStart_(0), button2PressStart_(0),
      eventQueueHead_(0), eventQueueTail_(0), eventQueueSize_(0)
{
}

void ButtonHandler::begin()
{
  ESP_LOGI(TAG, "Initializing button handler...");

  // Configure button1 (D5, GPIO23)
  gpio_reset_pin((gpio_num_t)ControllerConfig::Hardware::BUTTON1_PIN);
  gpio_set_direction((gpio_num_t)ControllerConfig::Hardware::BUTTON1_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode((gpio_num_t)ControllerConfig::Hardware::BUTTON1_PIN, GPIO_PULLUP_ONLY);

  // Configure button2 (D6, GPIO16)
  gpio_reset_pin((gpio_num_t)ControllerConfig::Hardware::BUTTON2_PIN);
  gpio_set_direction((gpio_num_t)ControllerConfig::Hardware::BUTTON2_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode((gpio_num_t)ControllerConfig::Hardware::BUTTON2_PIN, GPIO_PULLUP_ONLY);

  // Read initial states
  button1LastState_ = readButton1();
  button2LastState_ = readButton2();

  ESP_LOGI(TAG, "Button handler initialized - Button1 (GPIO%d), Button2 (GPIO%d)",
           ControllerConfig::Hardware::BUTTON1_PIN, ControllerConfig::Hardware::BUTTON2_PIN);
}

bool ButtonHandler::update(int64_t currentTime)
{
  bool eventsGenerated = false;

  // Update button1 state
  ButtonState button1State = getButton1State(currentTime);
  if (button1State != ButtonState::Released)
  {
    addEvent(BUTTON1_ID, button1State);
    eventsGenerated = true;
  }

  // Update button2 state
  ButtonState button2State = getButton2State(currentTime);
  if (button2State != ButtonState::Released)
  {
    addEvent(BUTTON2_ID, button2State);
    eventsGenerated = true;
  }

  return eventsGenerated;
}

ButtonEvent ButtonHandler::getNextEvent()
{
  ButtonEvent event = {0, ButtonState::Released, 0};

  if (eventQueueSize_ > 0)
  {
    event = eventQueue_[eventQueueHead_];
    eventQueueHead_ = (eventQueueHead_ + 1) % MAX_EVENTS;
    eventQueueSize_--;
  }

  return event;
}

void ButtonHandler::addEvent(uint8_t buttonId, ButtonState state)
{
  if (eventQueueSize_ < MAX_EVENTS)
  {
    eventQueue_[eventQueueTail_].buttonId = buttonId;
    eventQueue_[eventQueueTail_].state = state;
    eventQueue_[eventQueueTail_].timestamp = esp_timer_get_time();
    eventQueueTail_ = (eventQueueTail_ + 1) % MAX_EVENTS;
    eventQueueSize_++;
  }
  else
  {
    ESP_LOGW(TAG, "Event queue full, dropping event");
  }
}

ButtonState ButtonHandler::getButton1State(int64_t currentTime)
{
  bool currentState = readButton1();

  // Check for state change
  if (currentState != button1LastState_)
  {
    button1LastChange_ = currentTime;
    button1LastState_ = currentState;

    if (!currentState) // Button pressed (pulled low)
    {
      button1State_ = true;
      button1PressStart_ = currentTime;
      return ButtonState::Pressed;
    }
    else // Button released
    {
      button1State_ = false;
      ButtonState state = (button1PressStart_ > 0 && currentTime - button1PressStart_ > LONG_PRESS_THRESHOLD_US)
                              ? ButtonState::LongPress
                              : ButtonState::Released;
      button1PressStart_ = 0;
      return state;
    }
  }

  // Check for long press while button is held
  if (button1State_ && button1PressStart_ > 0 && currentTime - button1PressStart_ > LONG_PRESS_THRESHOLD_US)
  {
    return ButtonState::LongPress;
  }

  return ButtonState::Released;
}

ButtonState ButtonHandler::getButton2State(int64_t currentTime)
{
  bool currentState = readButton2();

  // Check for state change
  if (currentState != button2LastState_)
  {
    button2LastChange_ = currentTime;
    button2LastState_ = currentState;

    if (!currentState) // Button pressed (pulled low)
    {
      button2State_ = true;
      button2PressStart_ = currentTime;
      return ButtonState::Pressed;
    }
    else // Button released
    {
      button2State_ = false;
      ButtonState state = (button2PressStart_ > 0 && currentTime - button2PressStart_ > LONG_PRESS_THRESHOLD_US)
                              ? ButtonState::LongPress
                              : ButtonState::Released;
      button2PressStart_ = 0;
      return state;
    }
  }

  // Check for long press while button is held
  if (button2State_ && button2PressStart_ > 0 && currentTime - button2PressStart_ > LONG_PRESS_THRESHOLD_US)
  {
    return ButtonState::LongPress;
  }

  return ButtonState::Released;
}

bool ButtonHandler::readButton1()
{
  return gpio_get_level((gpio_num_t)ControllerConfig::Hardware::BUTTON1_PIN);
}

bool ButtonHandler::readButton2()
{
  return gpio_get_level((gpio_num_t)ControllerConfig::Hardware::BUTTON2_PIN);
}