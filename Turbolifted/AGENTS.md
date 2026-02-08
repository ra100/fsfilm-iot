# Turbolifted Controller - Project Documentation

## Overview

The Turbolifted Controller is an IoT device designed to create "turbolift" visual effects on a WS2812B addressable LED strip arranged in a linear pattern. It simulates dynamic light effects using gradient-based animations, controllable both locally via physical buttons and remotely through a web interface and HTTP API.

## Purpose

This project simplifies complex LED programming, provides reliable remote control, and enables runtime customization of visual effects without requiring users to write Arduino code.

## Problems Solved

- **Complex LED Programming**: Pre-built, tunable effects without coding
- **Remote Control**: Wireless control from smartphones/computers via web UI
- **Reliable Input Handling**: Debounced button inputs and uniform command processing
- **Customization**: Runtime adjustment of speed, brightness, colors, and modes
- **Diagnostics**: Startup tests and status indication for troubleshooting

## How It Works

### Hardware

- **Microcontroller**: ESP8266 (e.g., Lolin D1 Mini) for WiFi and GPIO control
- **LED Strip**: WS2812B addressable strip arranged in linear pattern
- **Inputs**: 3 physical buttons for local control
- **Status Indicator**: On-board LED for WiFi and system status

### Core Features

- **Turbolift Effects**: Random color gradients that rotate along the strip
- **Malfunction Mode**: Flickering effect for dramatic impact
- **Dual Effect Modes**:
  - **Classic (Mode 0)**: Pre-generated segmented gradients with random colors
  - **Virtual Gradients (Mode 1)**: Real-time sine-wave based dual rotating gradients for smoother animation
- **Controls**:
  - **Buttons**: Toggle effect, trigger malfunction, fade out
  - **Web API**: HTTP endpoints for all commands plus config settings (speed 0-10, brightness 0-255, hue range 0-255, mode 0/1)
  - **Web Interface**: Responsive single-page app served from LittleFS

### User Experience Goals

- **Intuitive**: Simple buttons for local use; clean, mobile-friendly web UI for remote
- **Immediate Feedback**: Visual LED responses to commands, status messages in UI, blinking status LED for WiFi
- **Reliable**: Non-blocking operation for smooth 100 FPS updates; debounced inputs prevent glitches
- **Customizable**: Real-time parameter tweaks with visual hue gradient preview
- **Accessible**: Works offline (buttons only) or with WiFi; easy setup with credential template

## System Architecture

### Core Components

#### LED Driver System

- **ILEDDriver Interface** (`src/led_driver.h`): Abstract interface for LED control, enabling testing with mock drivers
- **FastLEDDriver Template** (`src/led_driver.h`): Production implementation using FastLED library for WS2812B strips
- **Buffer Management**: Static CRGB buffers sized at compile time for memory efficiency

#### Turbolift Effect Engine

- **TurboliftEffectTemplate** (`src/turbolift_effect.h`): Main effect controller with template-based sizing
- **Function Pointer Architecture**: Uses `DriverColorGenerator` typedef for flexible color generation strategies
- **Dual Effect Modes**:
  - Mode 0: Classic pre-generated segmented gradients
  - Mode 1: Virtual gradients with real-time computation

#### Effect Generation System

- **generateTurboliftEffect()**: Core effect generator using function pointers for color strategies
- **DriverColorGenerator**: Function pointer type `CRGB (*)(int driverIndex)` for modular color generation
- **Virtual Gradient Sequences**: Lambda-based color generators implementing "1 color, 2 black" pattern
- **Interpolation Engine**: Linear color interpolation between driver points using `interpolateColor()`

#### Configuration Management

- **ConfigManager** (`src/config_manager.h/.cpp`): Runtime parameter storage with regeneration flags
- **TurboliftConfig Namespace** (`src/config.h`): Compile-time constants for hardware, timing, and effects

#### Input System

- **InputManager** (`src/input_manager.h`): Unified command handling from multiple sources
- **ButtonInputSource**: Physical button debouncing and mapping
- **WiFiInputSource** (`src/wifi_input_source.h`): HTTP API and web interface with automatic AP fallback

### Key Technical Decisions

#### Function Pointer Pattern

The `generateTurboliftEffect()` function uses function pointers to decouple color generation from effect rendering:

```cpp
typedef CRGB (*DriverColorGenerator)(int driverIndex);
void generateTurboliftEffect(CRGB *effectLeds, DriverColorGenerator colorGen = nullptr)
```

#### WiFi Connection Fallback Mechanism

The WiFi input source implements a robust fallback mechanism for network connectivity:

- **Non-blocking Connection**: WiFi connection attempts are non-blocking to maintain responsive button input
- **Connection Timeout**: 10-second timeout (`WIFI_TIMEOUT_MS`) for STA connection attempts
- **Automatic AP Fallback**: Switches to Access Point mode when STA connection fails
- **Captive Portal**: Provides web interface for configuration when in AP mode
- **Status Indication**: Uses status LED to indicate connection state (CONNECTING_STA → AP_MODE)

#### WiFi State Machine

```
WiFi.begin(ssid, password)
    ↓ (10 second timeout)
WiFi.status() == WL_CONNECTED?
    ├── YES → STA_CONNECTED mode
    └── NO  → switchToAPMode()
                ↓
            AP_MODE with web server
                ↓
            User connects and configures
                ↓
            Restart with new credentials
```

#### Virtual Gradient Implementation

- **Driver-based sequence generation**: Uses random driver spacing similar to `generateDriverColors()` function
- **Sparse color pattern**: Only every 3rd driver LED gets color, others are black, creating "1 color, 2 black" breaks
- **Gradient interpolation**: Calculates smooth gradients between driver LEDs using `interpolateColor()` function
- **Dual counter-rotating sequences**: Two independent sequences with different hues rotating in opposite directions
- **Additive color blending**: Combines both sequences additively for brighter, more vibrant effects
- **Half-speed rotation**: Virtual gradients rotate at half speed for smoother wave effects

#### Memory Management

- Template-based static allocation avoids dynamic memory
- Instance storage eliminates shared buffer bugs
- Compile-time sizing for predictable memory usage

### Component Relationships

```
main.cpp
├── TurboliftEffectTemplate (turbolift_effect.h)
│   ├── FastLEDDriver (led_driver.h)
│   ├── ConfigManager (config_manager.h)
│   └── Effect Generation Functions
├── InputManager (input_manager.h)
│   ├── ButtonInputSource
│   └── WiFiInputSource (wifi_input_source.h)
└── StartupSequence & StatusLED
```

### Critical Implementation Paths

#### Effect Update Loop

1. `main.cpp:loop()` → `turbolift.update(now)`
2. Speed calculation from `ConfigManager::getRotationSpeed()`
3. Position updates: `gradientPos1/2` for virtual mode
4. Effect rendering: `virtualGradientEffect()` or `turboliftEffect()`

#### Virtual Gradient Pipeline

1. `generateVirtualGradients()` creates driver-based sequences with random spacing
2. Driver LEDs placed at random distances using `minDist + random(maxDist - minDist + 1)`
3. Only every 3rd driver gets color, others are black for "1 color, 2 black" pattern
4. `interpolateColor()` calculates smooth gradients between driver LEDs
5. Runtime rotation and LED value sampling from sequences
6. Additive color blending combines both sequences for vibrant output

#### Configuration Flow

1. Web/button input → `ConfigManager` setters
2. Regeneration flag set → `needsEffectRegeneration()`
3. Next update cycle → effect regeneration with new parameters
4. Flag cleared → normal animation continues

## Technologies

### Technologies Used

- **Microcontroller**: ESP8266 (e.g., Lolin D1 Mini) for WiFi and GPIO control.
- **LED Library**: FastLED for WS2812B addressable LED strip control.
- **Web Server**: ESP8266WebServer for HTTP API and serving static files.
- **Filesystem**: LittleFS for storing web assets (index.html).

### Development Setup

- **Build System**: PlatformIO with Arduino framework for ESP8266.
- **Environments**: [env:d1] for device build, [env:test_native] for host unit testing.
- **Dependencies**: FastLED@^3.10.2.
- **Upload/Monitor**: 115200 baud rate.

### Technical Constraints

- **Memory**: Static buffers for LED data (800 CRGB ~ 2.4KB), careful with ESP8266 RAM limits.
- **Timing**: Non-blocking updates at 10ms intervals for ~100 FPS, no delays in loop.
- **WiFi**: STA mode for connection with automatic AP fallback on connection failure, 10-second timeout.
- **LED Strip**: WS2812B protocol requires precise timing, FastLED handles interrupts.

### Dependencies

- FastLED: LED control and color utilities (CHSV, CRGB, nscale8).
- ESP8266WiFi, ESP8266WebServer, LittleFS: Built-in Arduino libraries for WiFi and web.

### Tool Usage Patterns

- **Unit Testing**: Unity framework for C/C++ tests on debounce, effects; native C++ mains for quick checks.
- **Mocking**: ILEDDriver interface for testable LED interactions.
- **Configuration**: wifi_credentials.h (git-ignored) for SSID/password.
- **Build Flags**: -DUNIT_TEST for host compilation, disabling FastLED.
- **Scripts**: run_tests.sh for executing tests, setup_wifi.sh for credentials.

### Git Commit Conventions

- **Gitmoji Usage**: When creating commit messages, always use appropriate Gitmoji emojis to categorize commits:
  - ✨ `:sparkles:` - New features or enhancements
  - 🐛 `:bug:` - Bug fixes
  - 📝 `:memo:` - Documentation updates
  - ♻️ `:recycle:` - Refactoring or code improvements
  - 🧪 `:test_tube:` - Adding or updating tests
  - 🔧 `:wrench:` - Configuration changes or build system updates
  - 🎨 `:art:` - Code style improvements or formatting
  - 🚀 `:rocket:` - Performance improvements
  - 📦 `:package:` - Dependency updates
  - 🔒 `:lock:` - Security improvements
- **Commit Format**: Use conventional commits format with Gitmoji: `emoji scope: description`
- **Examples**:
  - ✨ feat: add new turbolift effect mode
  - 🐛 fix: resolve LED flickering issue
  - 📝 docs: update architecture documentation
  - ♻️ refactor: improve effect generation performance

## Current Context

### Current Work Focus

- Development and refinement of core components: LED driver (src/led_driver.h), main application logic (src/main.cpp), and WiFi input source (src/wifi_input_source.h).
- Initializing the memory bank to establish comprehensive project documentation.

### Recent Changes

- **Project creation**: Copied from Outtatimers Portal project and renamed to Turbolifted.
- **Namespace update**: Changed from PortalConfig to TurboliftConfig.
- **Class rename**: Changed PortalEffectTemplate to TurboliftEffectTemplate.
- **File renames**: Renamed portal_effect.h/cpp to turbolift_effect.h/cpp.

### Next Steps

- **Effect customization**: Modify the turbolift effect logic for the specific Turbolifted visual style.
- **WebUI customization**: Update the web interface to reflect Turbolifted branding.
- **Hardware validation**: Test the effects on actual ESP8266 hardware.
- **Performance optimization**: Monitor memory usage and frame rates.

## Target Audience

This product targets LED art enthusiasts, makers, and installers seeking programmable ambient lighting without deep coding knowledge.

---

_Last updated: 2026-02-08_
