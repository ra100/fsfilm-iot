# Tech

## Technologies Used

- **Microcontroller**: ESP8266 (Wemos D1 Mini) - Low-power Wi-Fi enabled microcontroller
- **Programming Framework**: Arduino - Simplified embedded C++ development
- **Build System**: PlatformIO - Advanced project management and compilation tool
- **LED Control Library**: FastLED - High-performance library for addressable LEDs
- **Programming Language**: C++ - Object-oriented programming for embedded systems

## Development Setup

- **IDE**: VS Code with PlatformIO extension for seamless development workflow
- **Board Configuration**: Wemos D1 (ESP8266) with 4MB flash and 80MHz clock speed
- **Upload Speed**: 115200 baud for firmware flashing
- **Monitor Speed**: 115200 baud for serial communication and debugging
- **Filesystem**: LittleFS for SPIFFS-like file system on ESP8266

## Technical Constraints

- **Memory Limitations**: ESP8266 has 80KB RAM and 4MB flash, requiring efficient code
- **Power Consumption**: Designed for battery operation, power management is critical
- **Processing Power**: 80MHz clock speed limits complex real-time calculations
- **Network Stack**: Wi-Fi connectivity impacts available memory and power usage
- **GPIO Limitations**: Limited digital and analog pins for external components

## Dependencies

- **FastLED v3.10.2**: Core library for LED strip control and lighting effects
  - Provides advanced color management and animation functions
  - Optimized for ESP8266 performance with linear LED strip
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
- **Power Optimization**: Sleep modes and LED brightness control for battery life
- **Wi-Fi Management**: Connection handling to minimize power draw when not in use

## Future Technical Enhancements

- **ESP32 Migration**: For increased processing power and memory (if needed)
- **Web Interface**: HTML/CSS/JavaScript for remote control via Wi-Fi
- **MQTT Integration**: For IoT connectivity and smart home integration
- **Advanced Effects**: Shader-like effects with more complex animations
