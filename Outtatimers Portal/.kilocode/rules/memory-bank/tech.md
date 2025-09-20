# Technologies

## Technologies Used

- **Microcontroller**: ESP8266 (e.g., Lolin D1 Mini) for WiFi and GPIO control.
- **LED Library**: FastLED for WS2812B addressable LED strip control.
- **Web Server**: ESP8266WebServer for HTTP API and serving static files.
- **Filesystem**: LittleFS for storing web assets (index.html).

## Development Setup

- **Build System**: PlatformIO with Arduino framework for ESP8266.
- **Environments**: [env:d1] for device build, [env:test_native] for host unit testing.
- **Dependencies**: FastLED@^3.10.2.
- **Upload/Monitor**: 115200 baud rate.

## Technical Constraints

- **Memory**: Static buffers for LED data (800 CRGB ~ 2.4KB), careful with ESP8266 RAM limits.
- **Timing**: Non-blocking updates at 10ms intervals for ~100 FPS, no delays in loop.
- **WiFi**: STA mode for connection, optional AP fallback not implemented.
- **LED Strip**: WS2812B protocol requires precise timing, FastLED handles interrupts.

## Dependencies

- FastLED: LED control and color utilities (CHSV, CRGB, nscale8).
- ESP8266WiFi, ESP8266WebServer, LittleFS: Built-in Arduino libraries for WiFi and web.

## Tool Usage Patterns

- **Unit Testing**: Unity framework for C/C++ tests on debounce, effects; native C++ mains for quick checks.
- **Mocking**: ILEDDriver interface for testable LED interactions.
- **Configuration**: wifi_credentials.h (git-ignored) for SSID/password.
- **Build Flags**: -DUNIT_TEST for host compilation, disabling FastLED.
- **Scripts**: run_tests.sh for executing tests, setup_wifi.sh for credentials.

## Git Commit Conventions

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
  - ✨ feat: add new portal effect mode
  - 🐛 fix: resolve LED flickering issue
  - 📝 docs: update architecture documentation
  - ♻️ refactor: improve effect generation performance
