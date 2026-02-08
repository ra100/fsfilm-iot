# Turbolifted LED Controller

A comprehensive LED controller for ESP8266 with turbolift effects, featuring an extensible input system that supports both physical buttons and WiFi control.

## Features

- **Turbolift Effects**: Stunning linear LED effects with fade in/out and malfunction modes
- **Button Control**: Physical buttons with proper debouncing
- **WiFi Control**: Optional web interface for remote control
- **Extensible Input System**: Easy to add new input sources (MQTT, Serial, etc.)
- **Non-blocking Architecture**: Smooth operation without delays
- **Comprehensive Configuration**: Centralized configuration system
- **Runtime Configuration**: Adjust effects via WiFi without code changes

## Hardware Requirements

- ESP8266 (Wemos D1 Mini or similar)
- WS2812B LED strip (up to 800 LEDs)
- 3 push buttons (optional)
- Power supply appropriate for your LED count

## Pin Configuration

- **LED Strip**: GPIO4 (D2)
- **Button 1** (Turbolift Toggle): GPIO14 (D5)
- **Button 2** (Malfunction): GPIO12 (D6)
- **Button 3** (Fade Out): GPIO13 (D7)

## Quick Start

1. **Configure your setup** in `src/config.h`:

   ```cpp
   // Hardware Configuration
   constexpr int NUM_LEDS = 800;           // Your LED count
   constexpr int LED_PIN = 4;              // Your LED pin
   ```

2. **Build and upload**:

   ```bash
   pio run -e d1 -t upload
   ```

3. **Control via buttons**:
   - Button 1: Toggle turbolift effect
   - Button 2: Trigger malfunction effect
   - Button 3: Fade out current effect

## WiFi Control (Optional)

To enable WiFi control with web interface:

1. **Enable WiFi** in `src/config.h`:

   ```cpp
   #define ENABLE_WIFI_CONTROL 1  // Change from 0 to 1
   ```

2. **Configure your WiFi** in `src/config.h`:

   ```cpp
   constexpr const char* DEFAULT_SSID = "YourWiFiNetwork";
   constexpr const char* DEFAULT_PASSWORD = "YourWiFiPassword";
   ```

3. **Build and upload**:

   ```bash
   pio run -e d1 -t upload
   ```

4. **Access web interface**:
   - Check serial monitor for IP address
   - Open `http://[device-ip]` in your browser
   - Use the web interface to control the turbolift

### WiFi API Endpoints

- `GET /` - Web interface
- `GET /toggle` - Toggle turbolift effect
- `GET /malfunction` - Trigger malfunction
- `GET /fadeout` - Fade out effect
- `GET /status` - System status
- `GET /config` - View current configuration
- `GET /set_speed?speed=0-10` - Set rotation speed
- `GET /set_brightness?brightness=0-255` - Set max brightness
- `GET /set_hue?min=0-255&max=0-255` - Set color hue range

## Configuration

All configuration is centralized in `src/config.h`:

### Hardware Settings

```cpp
constexpr int NUM_LEDS = 800;               // LED count
constexpr int LED_PIN = 4;                  // LED data pin
constexpr uint8_t DEFAULT_BRIGHTNESS = 255; // Max brightness
```

### Effect Parameters

```cpp
constexpr int GRADIENT_STEP_DEFAULT = 10;   // Color generation density
constexpr int GRADIENT_MOVE_DEFAULT = 2;    // Animation speed
constexpr unsigned long FADE_IN_DURATION_MS = 3000;  // Fade in time
```

### WiFi Settings

WiFi credentials are stored securely in `wifi_credentials.h` (git-ignored):

```cpp
#define WIFI_SSID "YourNetwork"      // Your WiFi network name
#define WIFI_PASSWORD "YourPassword" // Your WiFi password
#define AP_SSID "TurboliftedController"   // Fallback access point name
#define AP_PASSWORD "turbolift123"      // Fallback access point password
```

Other WiFi settings in `src/config.h`:

```cpp
constexpr int HTTP_PORT = 80;                    // Web server port
constexpr unsigned long WIFI_TIMEOUT_MS = 10000; // Connection timeout
```

## Runtime Configuration

The system now supports runtime configuration via WiFi:

### Available Configuration Parameters

- **Rotation Speed**: Controls the animation speed (0-10)
- **Max Brightness**: Adjusts the overall brightness (0-255)
- **Color Hue Range**: Sets the color palette range (0-255)

### Usage Examples

1. **View Current Configuration**:

   ```
   GET /config
   ```

2. **Set Rotation Speed**:

   ```
   GET /set_speed?speed=5
   ```

3. **Set Max Brightness**:

   ```
   GET /set_brightness?brightness=128
   ```

4. **Set Color Hue Range**:
   ```
   GET /set_hue?min=180&max=220
   ```

### Web Interface

The web interface now includes a configuration section that shows current settings and provides controls to adjust them.

## Architecture

The system uses a clean, extensible architecture:

- **InputManager**: Coordinates multiple input sources
- **ButtonInputSource**: Handles physical buttons with debouncing
- **WiFiInputSource**: Provides web interface and HTTP API
- **TurboliftEffect**: Manages LED effects and animations
- **StartupSequence**: Handles system initialization
- **Configuration**: Centralized parameter management
- **ConfigManager**: Runtime configuration system

## Adding New Input Sources

The system is designed for easy extension. To add a new input source:

1. **Implement IInputSource interface**:

   ```cpp
   class MyInputSource : public IInputSource {
       // Implement required methods
   };
   ```

2. **Add to InputManager**:

   ```cpp
   MyInputSource myInput;
   inputManager.addInputSource(&myInput);
   ```

3. **Commands are automatically handled** by the existing callback system

## Development

### Building

```bash
# Build for ESP8266
pio run -e d1

# Upload to device
pio run -e d1 -t upload

# Monitor serial output
pio device monitor
```

### Testing

#### Easy Test Runner (Recommended)

```bash
# Run all tests with a single command
./run_tests.sh
```

This script will:

- ✅ Run all available tests automatically
- ✅ Show clear pass/fail status with colors
- ✅ Clean up temporary files
- ✅ Provide a summary of test results

#### Manual Testing

```bash
# Run unit tests (PlatformIO method)
pio test -e test_native

# Or run individual tests manually
g++ -std=c++17 -I src -I .pio/libdeps/d1/FastLED/src test/native_test.cpp src/effects.cpp -o native_test && ./native_test
```

## Memory Usage

Current memory usage with WiFi enabled:

- **RAM**: 42.4% (34,768 bytes)
- **Flash**: 26.4% (275,735 bytes)

## Troubleshooting

### WiFi Connection Issues

- Check SSID and password in `wifi_credentials.h`
- Ensure 2.4GHz network (ESP8266 doesn't support 5GHz)
- Check serial monitor for connection status
- Verify `wifi_credentials.h` exists and has correct format

### LED Issues

- Verify power supply capacity
- Check data pin connection
- Ensure proper ground connection between ESP8266 and LED strip

### Button Issues

- Buttons use internal pull-up resistors
- Connect buttons between GPIO pin and ground
- Check debounce settings if buttons are too sensitive

## License

This project is open source. Feel free to modify and distribute.

## Contributing

Contributions are welcome! The modular architecture makes it easy to add new features:

- New effect algorithms
- Additional input sources (MQTT, Serial, IR remote, etc.)
- Enhanced web interface
- Mobile app integration
- Additional runtime configuration options
