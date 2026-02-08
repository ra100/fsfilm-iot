# AGENTS.md - Outtatimers Controller Project

> This document provides comprehensive context for AI agents working on the Outtatimers Controller project. Read this file at the start of every task to understand the project state and context.

---

## Project Brief

IoT device powered by ESP32-C6 using a linear LED strip (9 LEDs) and 2 buttons to create a sci-fi controller effect system. The device is controlled by 2 buttons to switch between different lighting effects.

---

## Product Description

### Why This Project Exists

This project creates a physical sci-fi controller for LED lighting effects, providing a tangible interface for controlling dynamic visual displays. It addresses the need for an interactive, standalone device that can manage complex lighting patterns without relying on software applications or network connectivity with possibility to control it remotely over Wi-Fi.

### Problems It Solves

- **Lack of Physical Control**: Many LED effect systems require app-based or computer-based control, limiting accessibility and creating dependency on digital devices.
- **Limited Interactive Options**: Standard LED controllers often lack intuitive physical interfaces for switching between multiple effects.
- **Aesthetic Integration**: Need for a controller that matches the sci-fi aesthetic of the lighting setup, enhancing the overall visual experience.
- **Standalone Operation**: Desire for a self-contained device that operates independently without external power sources or connectivity.
- **Freedom of Control**: Need for more freedom with control over Wi-Fi through simple web app, to be able to modify effects on the fly without need of programming a flashing.

### How It Should Work

The controller uses two buttons to cycle through different lighting effects on a linear LED strip using the first 9 LEDs. Two buttons (D5 for effect cycling, D6 for intensity modification) provide intuitive control over the lighting effects. Button presses trigger smooth transitions between modes, with visual feedback confirming the current selection.

### User Experience Goals

- **Intuitive Interaction**: Users should be able to understand and operate the controller without instructions.
- **Responsive Feedback**: Immediate visual response to button presses enhances user confidence.
- **Seamless Transitions**: Smooth effect changes create a polished, professional feel.
- **Reliable Performance**: Consistent operation across different environments and usage patterns.
- **Aesthetic Appeal**: The linear LED strip arrangement should evoke a sci-fi aesthetic that complements the lighting effects.

---

## Architecture

### System Architecture

The Outtatimers Controller is a standalone IoT device built on the ESP32-C6 microcontroller, designed to control LED lighting effects through physical buttons and display them on a linear LED strip using the first 9 LEDs.

### Hardware Architecture

- **Microcontroller**: ESP32-C6 (Seeed Xiao ESP32-C6 board) for Wi-Fi 6 capability and processing
- **LED Array**: Standard LED strip using first 9 LEDs (D4 pin) for linear effect display
- **Input Devices**: Two physical buttons on D5 (effect cycling) and D6 (intensity modification) for effect selection and control
- **Power Management**: USB or battery power for standalone operation

### Software Architecture

- **Framework**: ESP-IDF framework for production-ready embedded applications
- **Build System**: PlatformIO with ESP-IDF support for project management and compilation
- **LED Library**: FastLED for advanced NeoPixel control and effects (ESP-IDF compatible)
- **Structure**: Modular C++ classes for effects, input handling, and LED management

### Key Components

| Component     | Description                                         | Source Files                                                                                       |
| ------------- | --------------------------------------------------- | -------------------------------------------------------------------------------------------------- |
| EffectManager | Handles different lighting effects and transitions  | [`src/effect_manager.cpp`](src/effect_manager.cpp), [`src/effect_manager.h`](src/effect_manager.h) |
| ButtonHandler | Manages button press detection and debouncing       | [`src/button_handler.cpp`](src/button_handler.cpp), [`src/button_handler.h`](src/button_handler.h) |
| LedController | Controls NeoPixel and RGB LED outputs               | [`src/led_driver.cpp`](src/led_driver.cpp), [`src/led_driver.h`](src/led_driver.h)                 |
| WiFiManager   | Provides web interface for remote control (planned) | -                                                                                                  |

### Design Patterns

- **State Pattern**: For managing different LED effects and modes
- **Observer Pattern**: For button events triggering effect changes
- **Singleton Pattern**: For shared resources like LED controllers

### Critical Implementation Paths

1. **Effect Switching**: Button press → Debounce → State change → LED update
2. **LED Rendering**: Effect calculation → Color mapping → FastLED output
3. **Wi-Fi Integration**: HTTP server → WebSocket for real-time control (future)
4. **Power Management**: Sleep modes for battery conservation (future)

### Component Relationships

```
ButtonHandler → EffectManager → LedController → FastLED
                     ↓
                WiFiManager (planned)
```

### Technical Decisions

- **ESP32-C6 over ESP8266**: Enhanced performance with Wi-Fi 6 and Bluetooth 5 LE support
- **ESP-IDF Framework**: Production-ready framework for robust embedded applications
- **FastLED Library**: Rich feature set for complex LED animations (ESP-IDF compatible)
- **PlatformIO**: Better dependency management and ESP-IDF support
- **C++17 Standard**: Modern C++ features for embedded development
- **FreeRTOS Integration**: Real-time task scheduling for responsive operation
- **Linear LED Strip**: Practical hardware choice using first 9 LEDs for effects
- **Button Pin Configuration**: D5 for effect cycling, D6 for intensity modification

---

## Technology Stack

### Technologies Used

| Category        | Technology                      | Purpose                                                               |
| --------------- | ------------------------------- | --------------------------------------------------------------------- |
| Microcontroller | ESP32-C6 (Seeed Xiao ESP32-C6)  | High-performance Wi-Fi 6 + Bluetooth 5 LE microcontroller             |
| Framework       | ESP-IDF                         | Espressif IoT Development Framework for production-ready applications |
| Build System    | PlatformIO with ESP-IDF support | Advanced project management and compilation tool                      |
| LED Library     | FastLED                         | High-performance library for addressable LEDs (ESP-IDF compatible)    |
| Language        | C++17                           | Modern C++ for embedded systems                                       |
| RTOS            | FreeRTOS                        | Real-time operating system for embedded applications                  |

### Development Setup

- **IDE**: VS Code with PlatformIO extension for seamless development workflow
- **Board Configuration**: Seeed Xiao ESP32-C6 with Wi-Fi 6 and Bluetooth 5 LE support
- **Upload Speed**: 115200 baud for firmware flashing
- **Monitor Speed**: 115200 baud for serial communication and debugging
- **Filesystem**: LittleFS for SPIFFS-like file system on ESP32-C6

### Technical Constraints

- **Memory Limitations**: ESP32-C6 has 512KB RAM and 8MB flash
- **Power Consumption**: Designed for battery operation, power management is critical
- **Processing Power**: 160MHz clock speed enables complex real-time calculations
- **Network Stack**: Wi-Fi 6 connectivity with improved performance and efficiency
- **GPIO Limitations**: More digital and analog pins available for external components

### Dependencies

- **ESP-IDF v5.3**: Official Espressif IoT Development Framework
  - Provides comprehensive APIs for WiFi, HTTP server, GPIO, and more
  - Includes FreeRTOS for task management and timing
  - Supports ESP32-C6 with WiFi 6 and Bluetooth 5 LE
- **FastLED v3.10.2**: Core library for LED strip control and lighting effects
  - ESP-IDF compatible version for advanced LED control
  - Provides advanced color management and animation functions
  - Optimized for ESP32-C6 performance with linear LED strip
  - Supports various LED chipsets including WS2812B

### Tool Usage Patterns

- **Compilation**: PlatformIO handles automatic dependency resolution and building
- **Testing**: Unit tests for individual components, integration tests for hardware
- **Debugging**: Serial monitor for runtime debugging and LED status feedback
- **Version Control**: Git for source code management and collaboration
- **Deployment**: OTA updates via Wi-Fi for remote firmware updates (planned)

### Current Implementation Details

- **Hardware Configuration**: D2 for LED strip data, D5/D6 for button inputs with pull-up resistors
- **LED Setup**: 9 LEDs from standard strip, utilizing FastLED NEOPIXEL driver
- **Button Handling**: Debounced inputs with 200ms delay for reliable operation
- **Effect System**: 4 distinct lighting effects with smooth transitions and visual feedback
- **Brightness Control**: 4-level intensity adjustment (0, 64, 128, 192) with visual confirmation

### Performance Considerations

- **LED Refresh Rate**: FastLED optimized for 30-60 FPS animations
- **Memory Management**: Careful allocation to avoid fragmentation on limited RAM
- **Power Optimization**: Sleep modes, LED brightness control for battery life
- **Wi-Fi Management**: Connection handling to minimize power draw when not in use

---

## Current Context

### Current Work Focus

**COMPLETED (2025-10-11)**: Complete LED effects ecosystem with 4 distinct effects, power-saving deep sleep, and comprehensive color system for production-ready sci-fi controller with all WiFi functionality removed to eliminate power consumption issues.

### Project Status

| Area               | Status                                                                      |
| ------------------ | --------------------------------------------------------------------------- |
| Code Quality       | ✅ Complete effects ecosystem with 4 distinct LED effects                   |
| Build System       | ✅ All features compile successfully with no errors                         |
| Effect Count       | ✅ 4 effects (Rotating Darkness, Portal Open, Battery Status, Random Blink) |
| User Experience    | ✅ Intuitive button layout with dramatic default effect                     |
| Battery Monitoring | ✅ Visual battery status integrated with LED system                         |
| Power Management   | ✅ Light sleep with 3-second hold activation                                |
| LED Configuration  | ✅ Flexible 8-LED setup with LED mirroring (8 and 9 behave as one)          |

### Recent Changes (2025-10-08)

#### LED Effects System Enhancement

- **CREATED**: PortalOpenEffect - Sequential activation of first 6 LEDs, last LED turns on, then all off after 1 second
- **MODIFIED**: PortalOpenEffect - Now runs once and stops after completing the sequence
- **CREATED**: BatteryStatusEffect - Real-time battery level indicator using 1-7 green LEDs
- **CREATED**: RotatingDarknessEffect - All LEDs on except one rotating "dark" spot
- **CREATED**: RandomBlinkEffect - Random red blinking with 15-second auto-stop

#### Hardware Configuration & Control

- **CONFIGURED**: Flexible LED selection system using ACTIVE_LEDS array
  - Currently using LEDs: 1,2,3,5,6,7,8,9 (8 LEDs, with 8 and 9 behaving as one)
- **RECONFIGURED**: Button control scheme
  - Button1 (D5): Effect cycling through 4 available effects
  - Button2 (D6): LED on/off toggle or deep sleep activation
- **SET**: PortalOpen effect as default startup effect

#### Color System Improvements

- **REFACTORED**: Color definitions using packed GRB format for better performance
- **IMPLEMENTED**: Clean color constants (COLOR_PURPLE_GRB, COLOR_GREEN_GRB)
- **OPTIMIZED**: Single 32-bit color values for memory efficiency

#### Power Management System

- **IMPLEMENTED**: Light sleep functionality with 3-second button hold
  - Button2 hold for 3+ seconds activates light sleep mode
  - Button wake-up only (timer wake-up removed)
  - System restart on wake-up with all settings preserved
- **REMOVED**: 10-minute autosleep when LEDs are off

### LED Strip Color Mapping

**Rainbow Debug Effect Results (2025-10-08):**

| Logical Index | Physical LED | Color  | GRB Value                       |
| ------------- | ------------ | ------ | ------------------------------- |
| 0             | LED1         | Red    | 0x80000000                      |
| 1             | LED2         | Orange | 0xFFFF8000                      |
| 2             | LED3         | Yellow | 0xFF800000                      |
| 3             | LED5         | Green  | 0x00FF0000                      |
| 4             | LED6         | Blue   | 0xFF0000FF                      |
| 5             | LED7         | Indigo | 0xFF0080FF                      |
| 6             | LED8         | Violet | 0xFF00FF80                      |
| 7             | LED9         | Violet | 0xFF00FF80 (mirrored with LED8) |

### Previous Work

- Converted main.cpp from Arduino framework to ESP-IDF with FreeRTOS task-based architecture
- Updated WiFi input source to use ESP-IDF WiFi and HTTP server APIs
- Updated CMakeLists.txt for proper ESP-IDF component structure
- Maintained LED control functionality with WS2812B support
- Updated hardware configuration to use ESP32-C6 with Wi-Fi 6 capabilities
- **COMPLETED (2025-10-07)**: ESP-IDF compatibility fixes and host-based testing infrastructure

### Next Steps

1. **Install ESP-IDF**: Set up toolchain to execute host-based tests
2. **Run Tests**: Execute `./run_host_tests.sh` to validate all components
3. **Hardware Deployment**: Flash to xiao_esp32c6 and test complete 4-effect system
4. **User Testing**: Validate button responsiveness and effect visual impact
5. **Battery Testing**: Verify battery status effect accuracy across charge levels
6. **Deep Sleep Testing**: Test 3-second hold activation and wake-up
7. **Future Enhancement**: Consider RMT peripheral for precise WS2812B timing
8. **Production**: Deploy with OTA update capability for field updates

---

## Future Technical Enhancements

- **ESP32 Migration**: For increased processing power and memory (if needed)
- **Web Interface**: HTML/CSS/JavaScript for remote control via Wi-Fi
- **MQTT Integration**: For IoT connectivity and smart home integration
- **Advanced Effects**: Shader-like effects with more complex animations

---

## Project Structure

```
OuttatimersController/
├── src/
│   ├── main.cpp              # Main entry point
│   ├── config.h              # Configuration constants
│   ├── button_handler.cpp/h  # Button input handling
│   ├── effect_manager.cpp/h  # LED effect management
│   └── led_driver.cpp/h      # LED hardware control
├── test/
│   ├── test_main.cpp         # Test entry point
│   └── mocks/                # ESP-IDF mock implementations
├── include/                  # Public headers
├── lib/                      # External libraries
├── platformio.ini            # PlatformIO configuration
├── CMakeLists.txt            # CMake configuration
├── Makefile                  # Build automation
└── run_host_tests.sh         # Host-based test runner
```

---

## Coding Guidelines

### Commit Messages

Follow gitmoji convention with imperative mood:

```
✨ Add new feature
🐛 Fix bug
📝 Update documentation
♻️ Refactor code
```

Keep subject lines under 80 characters. Focus on what/why, not how.

### Code Style

- Use C++17 features where appropriate
- Follow ESP-IDF coding conventions
- Use meaningful variable and function names
- Document complex algorithms and hardware-specific code

---

_Last updated: 2025-10-11_
