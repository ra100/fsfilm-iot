# Architecture

## System Architecture

The Outtatimers Controller is a standalone IoT device built on the ESP32-C6 microcontroller, designed to control LED lighting effects through physical buttons and display them on a linear LED strip using the first 9 LEDs.

## Hardware Architecture

- **Microcontroller**: ESP32-C6 (Seeed Xiao ESP32-C6 board) for Wi-Fi 6 capability and processing
- **LED Array**: Standard LED strip using first 9 LEDs (D4 pin) for linear effect display
- **Input Devices**: Two physical buttons on D5 (effect cycling) and D6 (intensity modification) for effect selection and control
- **Power Management**: USB or battery power for standalone operation

## Software Architecture

- **Framework**: ESP-IDF framework for production-ready embedded applications
- **Build System**: PlatformIO with ESP-IDF support for project management and compilation
- **LED Library**: FastLED for advanced NeoPixel control and effects (ESP-IDF compatible)
- **Structure**: Modular C++ classes for effects, input handling, and LED management

## Key Components

- **EffectManager**: Handles different lighting effects and transitions
- **ButtonHandler**: Manages button press detection and debouncing
- **LedController**: Controls NeoPixel and RGB LED outputs
- **WiFiManager**: Provides web interface for remote control (planned)

## Design Patterns

- **State Pattern**: For managing different LED effects and modes
- **Observer Pattern**: For button events triggering effect changes
- **Singleton Pattern**: For shared resources like LED controllers

## Critical Implementation Paths

1. **Effect Switching**: Button press → Debounce → State change → LED update
2. **LED Rendering**: Effect calculation → Color mapping → FastLED output
3. **Wi-Fi Integration**: HTTP server → WebSocket for real-time control (future)
4. **Power Management**: Sleep modes for battery conservation (future)

## Component Relationships

```
ButtonHandler → EffectManager → LedController → FastLED
                     ↓
                WiFiManager (planned)
```

## Technical Decisions

- **ESP32-C6 over ESP8266**: Enhanced performance with Wi-Fi 6 and Bluetooth 5 LE support
- **ESP-IDF Framework**: Production-ready framework for robust embedded applications
- **FastLED Library**: Rich feature set for complex LED animations (ESP-IDF compatible)
- **PlatformIO**: Better dependency management and ESP-IDF support
- **C++17 Standard**: Modern C++ features for embedded development
- **FreeRTOS Integration**: Real-time task scheduling for responsive operation
- **Linear LED Strip**: Practical hardware choice using first 9 LEDs for effects
- **Button Pin Configuration**: D5 for effect cycling, D6 for intensity modification
