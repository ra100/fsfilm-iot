# OuttatimersController

ESP-IDF based LED controller with WiFi connectivity for the Outtatimers IoT project.

## Overview

OuttatimersController is an ESP32-C6 based LED controller that provides:
- WiFi connectivity for remote control via HTTP
- Web-based interface for effect control
- Support for multiple LED effects
- Brightness control
- Host-based testing infrastructure for development without hardware

## Hardware

- **Board**: Seeed XIAO ESP32-C6
- **LED Type**: WS2812B addressable LEDs
- **LED Count**: 9 LEDs
- **GPIO Pin**: GPIO4 for LED data

## Project Structure

```
OuttatimersController/
├── CMakeLists.txt          # Main build configuration
├── src/                    # Source code
│   ├── main.cpp           # Application entry point
│   ├── config.h           # Configuration constants
│   ├── wifi_input_source.h # WiFi and HTTP server implementation
│   └── led_driver.h       # NeoPixel LED driver
├── test/                   # Host-based tests
│   ├── CMakeLists.txt     # Test build configuration
│   ├── test_main.cpp      # Unity test cases
│   └── mocks/             # Mock headers for hardware dependencies
├── run_host_tests.sh      # Script to run host-based tests
└── README.md              # This file
```

## Requirements

### For Hardware Build
- ESP-IDF v5.0 or later
- Seeed XIAO ESP32-C6 board
- USB-C cable for flashing

### For Host-Based Testing
- ESP-IDF v5.0 or later (with Linux support)
- Linux development environment (Ubuntu, Debian, or similar)
- GCC toolchain for x86_64-linux-gnu
- CMake 3.16 or later

## Building for Hardware

### 1. Set Up ESP-IDF Environment

```bash
# Source ESP-IDF environment
. $HOME/esp/esp-idf/export.sh
```

### 2. Configure WiFi Credentials

Copy the template and edit with your credentials:

```bash
cp wifi_credentials.h.template wifi_credentials.h
# Edit wifi_credentials.h with your WiFi SSID and password
```

### 3. Build and Flash

```bash
# Set target to ESP32-C6
idf.py set-target esp32c6

# Build the project
idf.py build

# Flash to device
idf.py flash

# Monitor serial output
idf.py monitor
```

## Host-Based Testing

Host-based testing allows you to run tests on your development machine without ESP32 hardware. This is useful for:
- Testing business logic without hardware dependencies
- Continuous integration/automated testing
- Rapid development iteration
- Validating configuration and constants

### Running Host Tests

```bash
# Make sure ESP-IDF is sourced
. $HOME/esp/esp-idf/export.sh

# Run the test script
./run_host_tests.sh
```

To clean and rebuild:

```bash
./run_host_tests.sh clean
```

### Expected Test Output

```
==========================================
OuttatimersController Host-Based Testing
==========================================

Using ESP-IDF from: /home/user/esp/esp-idf

Setting IDF_TARGET to linux...
Configuring build for host testing...
Building tests...
==========================================
Running tests...
==========================================

Running: ./OuttatimersController.elf

test_hardware_config_constants ... OK
test_timing_config_constants ... OK
test_effects_config_constants ... OK
test_wifi_config_constants ... OK
test_config_value_ranges ... OK
test_basic_arithmetic ... OK
test_string_operations ... OK

-----------------------
7 Tests 0 Failures 0 Ignored
OK

==========================================
Tests completed successfully!
==========================================
```

### What Can Be Tested

The current test infrastructure includes:
- Configuration constant validation
- Value range verification
- Basic arithmetic and string operations
- Hardware configuration validation (GPIO pins, LED counts, etc.)

### Limitations

Host-based tests cannot:
- Test actual hardware interactions (GPIO, WiFi, LEDs)
- Verify timing-critical code
- Test FreeRTOS task scheduling
- Validate real WiFi connectivity

For these aspects, hardware testing is still required.

## Mock System

The test infrastructure uses mock headers to simulate ESP-IDF APIs:
- **GPIO**: Mock GPIO driver for LED control
- **WiFi**: Mock WiFi stack
- **HTTP Server**: Mock HTTP server
- **FreeRTOS**: Mock task and timing functions
- **ESP Timer**: Mock high-resolution timer
- **NVS**: Mock non-volatile storage

Mock headers are located in `test/mocks/` and are automatically included during host builds.

## Troubleshooting

### Host Build Issues

**Error: `IDF_PATH is not set`**
- Solution: Source the ESP-IDF environment: `. $HOME/esp/esp-idf/export.sh`

**Error: `IDF_TARGET=linux not supported`**
- Solution: Ensure you're using ESP-IDF v5.0 or later with Linux host support

**Error: Missing unity component**
- Solution: Unity test framework should be included with ESP-IDF. Check your ESP-IDF installation.

**Compilation errors in mocks**
- Solution: Verify that `test/mocks/` is in the include path. Check `test/CMakeLists.txt`.

### Hardware Build Issues

**Error: `Target 'esp32c6' not found`**
- Solution: Update ESP-IDF to v5.0 or later which includes ESP32-C6 support

**WiFi connection fails**
- Solution: Check WiFi credentials in `wifi_credentials.h`
- Verify WiFi network is 2.4GHz (ESP32-C6 doesn't support 5GHz)

**LEDs don't light up**
- Solution: Check LED_PIN configuration in `config.h`
- Verify LED power supply and data line connections

## Adding New Tests

To add new tests to the host-based testing infrastructure:

1. Add test functions to `test/test_main.cpp`:

```cpp
void test_my_feature(void) {
    TEST_ASSERT_EQUAL(expected, actual);
}
```

2. Register the test in `app_main()`:

```cpp
extern "C" void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_my_feature);
    UNITY_END();
}
```

3. Run the tests: `./run_host_tests.sh`

## Contributing

When contributing to this project:
1. Add tests for new features when possible
2. Ensure host tests pass before submitting changes
3. Update documentation for new features
4. Follow ESP-IDF coding style guidelines

## License

See LICENSE file in the root of the repository.

## Additional Resources

- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [ESP-IDF Host Testing Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/host-apps.html)
- [Unity Test Framework](http://www.throwtheswitch.org/unity)
- [Seeed XIAO ESP32-C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/)