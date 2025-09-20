# System Architecture

## Core Components

### LED Driver System

- **ILEDDriver Interface** (`src/led_driver.h`): Abstract interface for LED control, enabling testing with mock drivers
- **FastLEDDriver Template** (`src/led_driver.h`): Production implementation using FastLED library for WS2812B strips
- **Buffer Management**: Static CRGB buffers sized at compile time for memory efficiency

### Portal Effect Engine

- **PortalEffectTemplate** (`src/portal_effect.h`): Main effect controller with template-based sizing
- **Function Pointer Architecture**: Uses `DriverColorGenerator` typedef for flexible color generation strategies
- **Dual Effect Modes**:
  - Mode 0: Classic pre-generated segmented gradients
  - Mode 1: Virtual gradients with real-time computation

### Effect Generation System

- **generatePortalEffect()**: Core effect generator using function pointers for color strategies
- **DriverColorGenerator**: Function pointer type `CRGB (*)(int driverIndex)` for modular color generation
- **Virtual Gradient Sequences**: Lambda-based color generators implementing "1 color, 2 black" pattern
- **Interpolation Engine**: Linear color interpolation between driver points using `interpolateColor()`

### Configuration Management

- **ConfigManager** (`src/config_manager.h/.cpp`): Runtime parameter storage with regeneration flags
- **PortalConfig Namespace** (`src/config.h`): Compile-time constants for hardware, timing, and effects

### Input System

- **InputManager** (`src/input_manager.h`): Unified command handling from multiple sources
- **ButtonInputSource**: Physical button debouncing and mapping
- **WiFiInputSource** (`src/wifi_input_source.h`): HTTP API and web interface

## Key Technical Decisions

### Function Pointer Pattern

The `generatePortalEffect()` function uses function pointers to decouple color generation from effect rendering:

```cpp
typedef CRGB (*DriverColorGenerator)(int driverIndex);
void generatePortalEffect(CRGB *effectLeds, DriverColorGenerator colorGen = nullptr)
```

### Virtual Gradient Implementation

- **Driver-based sequence generation**: Uses random driver spacing similar to `generateDriverColors()` function
- **Sparse color pattern**: Only every 3rd driver LED gets color, others are black, creating "1 color, 2 black" breaks
- **Gradient interpolation**: Calculates smooth gradients between driver LEDs using `interpolateColor()` function
- **Dual counter-rotating sequences**: Two independent sequences with different hues rotating in opposite directions
- **Additive color blending**: Combines both sequences additively for brighter, more vibrant effects
- **Half-speed rotation**: Virtual gradients rotate at half speed for smoother wave effects

### Memory Management

- Template-based static allocation avoids dynamic memory
- Instance storage eliminates shared buffer bugs
- Compile-time sizing for predictable memory usage

## Component Relationships

```
main.cpp
├── PortalEffectTemplate (portal_effect.h)
│   ├── FastLEDDriver (led_driver.h)
│   ├── ConfigManager (config_manager.h)
│   └── Effect Generation Functions
├── InputManager (input_manager.h)
│   ├── ButtonInputSource
│   └── WiFiInputSource (wifi_input_source.h)
└── StartupSequence & StatusLED
```

## Critical Implementation Paths

### Effect Update Loop

1. `main.cpp:loop()` → `portal.update(now)`
2. Speed calculation from `ConfigManager::getRotationSpeed()`
3. Position updates: `gradientPos1/2` for virtual mode
4. Effect rendering: `virtualGradientEffect()` or `portalEffect()`

### Virtual Gradient Pipeline

1. `generateVirtualGradients()` creates driver-based sequences with random spacing
2. Driver LEDs placed at random distances using `minDist + random(maxDist - minDist + 1)`
3. Only every 3rd driver gets color, others are black for "1 color, 2 black" pattern
4. `interpolateColor()` calculates smooth gradients between driver LEDs
5. Runtime rotation and LED value sampling from sequences
6. Additive color blending combines both sequences for vibrant output

### Configuration Flow

1. Web/button input → `ConfigManager` setters
2. Regeneration flag set → `needsEffectRegeneration()`
3. Next update cycle → effect regeneration with new parameters
4. Flag cleared → normal animation continues
