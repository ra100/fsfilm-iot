#pragma once

#include "config.h"
#include "led_driver.h"

#ifndef UNIT_TEST
#include <Arduino.h>
#else
extern "C" unsigned long millis();
#endif

/**
 * @brief Manages the non-blocking startup sequence to clear and test LEDs on boot
 *
 * This class clears the LEDs on boot, then runs a timed color flash sequence (Red/Green/Blue)
 * to indicate startup. The sequence is non-blocking and progresses via periodic `update()` calls.
 *
 * @example
 * ```cpp
 * StartupSequence startup;
 * startup.begin(ledDriver);
 *
 * void loop() {
 *     if (!startup.isComplete()) {
 *         startup.update(millis());
 *     }
 * }
 * ```
 */
class StartupSequence
{
public:
  /**
   * @brief Startup sequence states
   */
  enum class State
  {
    NotStarted,         ///< Sequence not yet started
    WaitingBeforeFlash, ///< Initial delay before first flash
    FlashRed,           ///< Displaying red color
    FlashGreen,         ///< Displaying green color
    FlashBlue,          ///< Displaying blue color
    Done                ///< Sequence completed
  };

  /**
   * @brief Construct a new StartupSequence object
   */
  StartupSequence() : currentState_(State::NotStarted), stateStartTime_(0), driver_(nullptr) {}

  /**
   * @brief Initialize and start the startup sequence
   * @param driver LED driver to use for the sequence
   */
  void begin(ILEDDriver *driver)
  {
    driver_ = driver;
    if (driver_)
    {
      driver_->setBrightness(TurboliftConfig::Hardware::DEFAULT_BRIGHTNESS);
      driver_->clear();
      driver_->show();
      stateStartTime_ = millis();
      transitionToState(State::WaitingBeforeFlash, millis());
    }
  }

  /**
   * @brief Update the startup sequence state machine
   * @param currentTime Current timestamp in milliseconds
   * @return true if state changed, false otherwise
   */
  bool update(unsigned long currentTime)
  {
    if (!driver_ || currentState_ == State::Done)
    {
      return false;
    }

    bool stateChanged = false;

    switch (currentState_)
    {
    case State::WaitingBeforeFlash:
      if (currentTime - stateStartTime_ >= TurboliftConfig::Timing::STARTUP_INITIAL_DELAY_MS)
      {
        driver_->fillSolid(CRGB::Red);
        transitionToState(State::FlashRed, currentTime);
        stateChanged = true;
      }
      break;

    case State::FlashRed:
      if (currentTime - stateStartTime_ >= TurboliftConfig::Timing::STARTUP_COLOR_DURATION_MS)
      {
        driver_->fillSolid(CRGB::Green);
        transitionToState(State::FlashGreen, currentTime);
        stateChanged = true;
      }
      break;

    case State::FlashGreen:
      if (currentTime - stateStartTime_ >= TurboliftConfig::Timing::STARTUP_COLOR_DURATION_MS)
      {
        driver_->fillSolid(CRGB::Blue);
        transitionToState(State::FlashBlue, currentTime);
        stateChanged = true;
      }
      break;

    case State::FlashBlue:
      if (currentTime - stateStartTime_ >= TurboliftConfig::Timing::STARTUP_COLOR_DURATION_MS)
      {
        driver_->clear();
        driver_->setBrightness(TurboliftConfig::Hardware::DEFAULT_BRIGHTNESS);
        transitionToState(State::Done, currentTime);
        stateChanged = true;
      }
      break;

    default:
      break;
    }

    return stateChanged;
  }

  /**
   * @brief Check if the startup sequence is complete
   * @return true if sequence is done, false otherwise
   */
  bool isComplete() const
  {
    return currentState_ == State::Done;
  }

  /**
   * @brief Get the current state of the startup sequence
   * @return Current state
   */
  State getCurrentState() const
  {
    return currentState_;
  }

  /**
   * @brief Get a string representation of the current state
   * @return State name as string
   */
  const char *getStateString() const
  {
    switch (currentState_)
    {
    case State::NotStarted:
      return "NotStarted";
    case State::WaitingBeforeFlash:
      return "WaitingBeforeFlash";
    case State::FlashRed:
      return "FlashRed";
    case State::FlashGreen:
      return "FlashGreen";
    case State::FlashBlue:
      return "FlashBlue";
    case State::Done:
      return "Done";
    default:
      return "Unknown";
    }
  }

private:
  /**
   * @brief Transition to a new state
   * @param newState State to transition to
   * @param currentTime Current timestamp
   */
  void transitionToState(State newState, unsigned long currentTime)
  {
    currentState_ = newState;
    stateStartTime_ = currentTime;
  }

  State currentState_;           ///< Current state of the sequence
  unsigned long stateStartTime_; ///< Timestamp when current state started
  ILEDDriver *driver_;           ///< LED driver for output
};