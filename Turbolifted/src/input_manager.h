#pragma once

#include "config.h"
#include "debounce.h"
#include <functional>

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" unsigned long millis();
extern "C" int digitalRead(int pin);
extern "C" void pinMode(int pin, int mode);
constexpr int INPUT_PULLUP = 2;
#endif

/**
 * @brief Abstract base class for input sources
 *
 * This interface allows different input sources (physical buttons, WiFi commands,
 * serial commands, etc.) to be handled uniformly by the input manager.
 */
class IInputSource
{
public:
  /**
   * @brief Input event types
   */
  enum class EventType
  {
    Pressed,  ///< Input was activated (button pressed, command received)
    Released, ///< Input was deactivated (button released)
    LongPress ///< Input held for extended period
  };

  /**
   * @brief Input event data
   */
  struct InputEvent
  {
    int inputId;             ///< Unique identifier for the input
    EventType type;          ///< Type of event
    unsigned long timestamp; ///< When the event occurred
    const char *sourceName;  ///< Name of the input source (for debugging)
  };

  virtual ~IInputSource() = default;

  /**
   * @brief Update the input source and check for events
   * @param currentTime Current timestamp in milliseconds
   * @return true if events are available, false otherwise
   */
  virtual bool update(unsigned long currentTime) = 0;

  /**
   * @brief Check if events are available
   * @return true if events are pending
   */
  virtual bool hasEvents() const = 0;

  /**
   * @brief Get the next available event
   * @return Next input event, or nullptr if no events available
   */
  virtual InputEvent getNextEvent() = 0;

  /**
   * @brief Get the name of this input source
   * @return Source name for debugging/logging
   */
  virtual const char *getSourceName() const = 0;
};

/**
 * @brief Physical button input source using debouncing
 *
 * Handles physical buttons connected to GPIO pins with proper debouncing
 * and edge detection for reliable input processing.
 */
class ButtonInputSource : public IInputSource
{
public:
  /**
   * @brief Button configuration
   */
  struct ButtonConfig
  {
    int pin;                  ///< GPIO pin number
    int inputId;              ///< Unique identifier for this button
    bool activeLow;           ///< true if button is active low (default)
    unsigned long debounceMs; ///< Debounce interval in milliseconds
    const char *name;         ///< Human-readable name for debugging
  };

  /**
   * @brief Construct a new ButtonInputSource
   * @param buttons Array of button configurations
   * @param buttonCount Number of buttons in the array
   */
  ButtonInputSource(const ButtonConfig *buttons, int buttonCount)
      : buttons_(buttons), buttonCount_(buttonCount), eventQueueHead_(0), eventQueueTail_(0)
  {

    // Initialize GPIO pins and debounce objects
    for (int i = 0; i < buttonCount_; i++)
    {
      pinMode(buttons_[i].pin, INPUT_PULLUP);
      debouncers_[i] = Debounce(buttons_[i].debounceMs);
      lastStates_[i] = buttons_[i].activeLow ? static_cast<int>(TurboliftConfig::PinState::High) : static_cast<int>(TurboliftConfig::PinState::Low);
    }
  }

  bool update(unsigned long currentTime) override
  {
    bool hasNewEvents = false;

    for (int i = 0; i < buttonCount_; i++)
    {
      const ButtonConfig &config = buttons_[i];

      // Read raw pin state
      int rawState = digitalRead(config.pin);

      // Apply debouncing
      bool stateChanged = debouncers_[i].sample(rawState, currentTime);
      int currentState = debouncers_[i].getState();

      if (stateChanged)
      {
        // Determine event type based on active low/high configuration
        bool isPressed = config.activeLow ? (currentState == static_cast<int>(TurboliftConfig::PinState::Low)) : (currentState == static_cast<int>(TurboliftConfig::PinState::High));

        EventType eventType = isPressed ? EventType::Pressed : EventType::Released;

        // Queue the event
        queueEvent({.inputId = config.inputId,
                    .type = eventType,
                    .timestamp = currentTime,
                    .sourceName = config.name});

        hasNewEvents = true;
      }

      lastStates_[i] = currentState;
    }

    return hasNewEvents;
  }

  bool hasEvents() const override
  {
    return eventQueueHead_ != eventQueueTail_;
  }

  InputEvent getNextEvent() override
  {
    if (!hasEvents())
    {
      // Return empty event if no events available
      return {0, EventType::Released, 0, "none"};
    }

    InputEvent event = eventQueue_[eventQueueHead_];
    eventQueueHead_ = (eventQueueHead_ + 1) % MAX_EVENTS;
    return event;
  }

  const char *getSourceName() const override
  {
    return "ButtonInput";
  }

private:
  static constexpr int MAX_BUTTONS = 8;
  static constexpr int MAX_EVENTS = 16;

  const ButtonConfig *buttons_;
  int buttonCount_;
  Debounce debouncers_[MAX_BUTTONS];
  int lastStates_[MAX_BUTTONS];

  // Event queue for handling multiple rapid events
  InputEvent eventQueue_[MAX_EVENTS];
  int eventQueueHead_;
  int eventQueueTail_;

  void queueEvent(const InputEvent &event)
  {
    int nextTail = (eventQueueTail_ + 1) % MAX_EVENTS;
    if (nextTail != eventQueueHead_)
    { // Don't overflow the queue
      eventQueue_[eventQueueTail_] = event;
      eventQueueTail_ = nextTail;
    }
    // If queue is full, oldest events are dropped (could add logging here)
  }
};

/**
 * @brief Input manager that coordinates multiple input sources
 *
 * This class manages different input sources (buttons, WiFi, serial, etc.)
 * and provides a unified interface for handling input events with callbacks.
 * Designed for easy extension with new input sources.
 */
class InputManager
{
public:
  /**
   * @brief Input command identifiers
   *
   * These IDs are used consistently across all input sources
   * to identify the logical function being requested.
   */
  enum class Command
  {
    ToggleTurbolift = 1,       ///< Start/stop turbolift effect
    TriggerMalfunction = 2, ///< Trigger malfunction effect
    FadeOut = 3,            ///< Fade out current effect
                            // Future commands can be added here
                            // SetBrightness = 4,
                            // ChangeColor = 5,
                            // etc.
  };

  /**
   * @brief Callback function type for input events
   * @param command The logical command being executed
   * @param source Name of the input source that triggered the command
   */
  using InputCallback = std::function<void(Command command, const char *source)>;

  /**
   * @brief Construct a new InputManager
   */
  InputManager() : callbackSet_(false) {}

  /**
   * @brief Set the callback function for input events
   * @param callback Function to call when input events occur
   */
  void setInputCallback(InputCallback callback)
  {
    callback_ = callback;
    callbackSet_ = true;
  }

  /**
   * @brief Add an input source to the manager
   * @param source Pointer to input source (must remain valid)
   */
  void addInputSource(IInputSource *source)
  {
    if (sourceCount_ < MAX_SOURCES)
    {
      sources_[sourceCount_++] = source;
    }
  }

  /**
   * @brief Update all input sources and process events
   * @param currentTime Current timestamp in milliseconds
   */
  void update(unsigned long currentTime)
  {
    // Update all input sources
    for (int i = 0; i < sourceCount_; i++)
    {
      sources_[i]->update(currentTime);
    }

    // Process events from all sources
    processEvents();
  }

  /**
   * @brief Map input ID to logical command
   * @param inputId Raw input identifier
   * @return Corresponding logical command
   */
  static Command mapInputToCommand(int inputId)
  {
    switch (inputId)
    {
    case 1:
      return Command::ToggleTurbolift;
    case 2:
      return Command::TriggerMalfunction;
    case 3:
      return Command::FadeOut;
    default:
      return Command::ToggleTurbolift; // Default fallback
    }
  }

  /**
   * @brief Get string representation of command
   * @param command Command to convert
   * @return Human-readable command name
   */
  static const char *getCommandName(Command command)
  {
    switch (command)
    {
    case Command::ToggleTurbolift:
      return "ToggleTurbolift";
    case Command::TriggerMalfunction:
      return "TriggerMalfunction";
    case Command::FadeOut:
      return "FadeOut";
    default:
      return "Unknown";
    }
  }

private:
  static constexpr int MAX_SOURCES = 4;

  IInputSource *sources_[MAX_SOURCES];
  int sourceCount_ = 0;
  InputCallback callback_;
  bool callbackSet_;

  void processEvents()
  {
    if (!callbackSet_)
      return;

    for (int i = 0; i < sourceCount_; i++)
    {
      IInputSource *source = sources_[i];

      while (source->hasEvents())
      {
        IInputSource::InputEvent event = source->getNextEvent();

        // Only process press events (ignore releases for now)
        if (event.type == IInputSource::EventType::Pressed)
        {
          Command command = mapInputToCommand(event.inputId);
          callback_(command, event.sourceName);
        }
      }
    }
  }
};