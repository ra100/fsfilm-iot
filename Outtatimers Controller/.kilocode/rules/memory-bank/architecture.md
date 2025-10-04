# Architecture

## System Architecture

The Outtatimers Controller is a standalone IoT device built on the ESP8266 microcontroller, designed to control LED lighting effects through physical buttons and display them on a hexagonal NeoPixel arrangement.

## Hardware Architecture

- **Microcontroller**: ESP8266 (Wemos D1 board) for Wi-Fi capability and processing
- **LED Array**: NeoPixel 8 ring with 7 LEDs (6 outer + 1 center) for effect display
- **Status LED**: Single RGB LED for additional status indication
- **Input Devices**: Two physical buttons for effect selection and control
- **Power Management**: USB or battery power for standalone operation

## Software Architecture

- **Framework**: Arduino framework for embedded C++ development
- **Build System**: PlatformIO for project management and compilation
- **LED Library**: FastLED for advanced NeoPixel control and effects
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

- **ESP8266 over ESP32**: Sufficient performance with lower power consumption
- **FastLED Library**: Rich feature set for complex LED animations
- **PlatformIO**: Better dependency management than Arduino IDE
- **C++ Classes**: Improved code organization over procedural approach
- **Hexagonal Layout**: Sci-fi aesthetic with efficient LED usage (7/8 pixels)
