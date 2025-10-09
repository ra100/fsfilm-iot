# Tech

## Technologies Used

- **Microcontroller**: ESP32-C6 (Seeed Xiao ESP32-C6) - High-performance Wi-Fi 6 + Bluetooth 5 LE microcontroller
- **Programming Framework**: ESP-IDF - Espressif IoT Development Framework for production-ready applications
- **Build System**: PlatformIO with ESP-IDF support - Advanced project management and compilation tool
- **LED Control Library**: FastLED - High-performance library for addressable LEDs (ESP-IDF compatible)
- **Programming Language**: C++17 - Modern C++ for embedded systems
- **Real-Time Operating System**: FreeRTOS - Real-time operating system for embedded applications

## Development Setup

- **IDE**: VS Code with PlatformIO extension for seamless development workflow
- **Board Configuration**: Seeed Xiao ESP32-C6 with Wi-Fi 6 and Bluetooth 5 LE support
- **Upload Speed**: 115200 baud for firmware flashing
- **Monitor Speed**: 115200 baud for serial communication and debugging
- **Filesystem**: LittleFS for SPIFFS-like file system on ESP32-C6

## Technical Constraints

- **Memory Limitations**: ESP32-C6 has 512KB RAM and 8MB flash, providing more resources for complex applications
- **Power Consumption**: Designed for battery operation, power management is critical
- **Processing Power**: 160MHz clock speed enables more complex real-time calculations
- **Network Stack**: Wi-Fi 6 connectivity with improved performance and efficiency
- **GPIO Limitations**: More digital and analog pins available for external components

## Dependencies

- **ESP-IDF v5.3**: Official Espressif IoT Development Framework
  - Provides comprehensive APIs for WiFi, HTTP server, GPIO, and more
  - Includes FreeRTOS for task management and timing
  - Supports ESP32-C6 with WiFi 6 and Bluetooth 5 LE
- **FastLED v3.10.2**: Core library for LED strip control and lighting effects
  - ESP-IDF compatible version for advanced LED control
  - Provides advanced color management and animation functions
  - Optimized for ESP32-C6 performance with linear LED strip
  - Supports various LED chipsets including WS2812B
  - Enables 6 distinct lighting effects adapted for linear arrangement

## Tool Usage Patterns

- **Compilation**: PlatformIO handles automatic dependency resolution and building
- **Testing**: Unit tests for individual components, integration tests for hardware
- **Debugging**: Serial monitor for runtime debugging and LED status feedback
- **Version Control**: Git for source code management and collaboration
- **Deployment**: OTA updates via Wi-Fi for remote firmware updates (planned)

## Current Implementation Details

- **Hardware Configuration**: D2 for LED strip data, D5/D6 for button inputs with pull-up resistors
- **LED Setup**: 9 LEDs from standard strip, utilizing FastLED NEOPIXEL driver
- **Button Handling**: Debounced inputs with 200ms delay for reliable operation
- **Effect System**: 6 distinct lighting effects with smooth transitions and visual feedback
- **Brightness Control**: 4-level intensity adjustment (0, 64, 128, 192) with visual confirmation

## Performance Considerations

- **LED Refresh Rate**: FastLED optimized for 30-60 FPS animations
- **Memory Management**: Careful allocation to avoid fragmentation on limited RAM
- **Power Optimization**: Sleep modes, LED brightness control, and 10-minute autosleep when LEDs off for battery life
- **Wi-Fi Management**: Connection handling to minimize power draw when not in use

## Future Technical Enhancements

- **ESP32 Migration**: For increased processing power and memory (if needed)
- **Web Interface**: HTML/CSS/JavaScript for remote control via Wi-Fi
- **MQTT Integration**: For IoT connectivity and smart home integration
- **Advanced Effects**: Shader-like effects with more complex animations
