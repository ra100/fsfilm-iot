/**
 * @file test_main.cpp
 * @brief Host-based tests for OuttatimersController
 *
 * This file contains Unity test cases that can run on a Linux host
 * without requiring ESP32 hardware. It uses mock headers to simulate
 * hardware dependencies.
 */

#include <unity.h>
#include <string.h>
#include "config.h"

// For host builds, we use mocked headers
#ifdef HOST_BUILD
// The mocks are already in the include path via CMakeLists.txt
#endif

/**
 * Unity setUp function - called before each test
 */
void setUp(void)
{
    // This is run before each test
}

/**
 * Unity tearDown function - called after each test
 */
void tearDown(void)
{
    // This is run after each test
}

/**
 * Test: Verify hardware configuration constants
 */
void test_hardware_config_constants(void)
{
    TEST_ASSERT_EQUAL(4, ControllerConfig::Hardware::LED_PIN);
    TEST_ASSERT_EQUAL(9, ControllerConfig::Hardware::NUM_LEDS);
    TEST_ASSERT_EQUAL(255, ControllerConfig::Hardware::DEFAULT_BRIGHTNESS);
    TEST_ASSERT_EQUAL(5, ControllerConfig::Hardware::BUTTON1_PIN);
    TEST_ASSERT_EQUAL(6, ControllerConfig::Hardware::BUTTON2_PIN);
}

/**
 * Test: Verify effects configuration constants
 */
void test_effects_config_constants(void)
{
    TEST_ASSERT_EQUAL(64, ControllerConfig::Effects::MIN_BRIGHTNESS);
    TEST_ASSERT_EQUAL(64, ControllerConfig::Effects::BRIGHTNESS_STEP);
}

/**
 * Test: Verify WiFi configuration constants
 */
void test_wifi_config_constants(void)
{
    TEST_ASSERT_EQUAL(10000, ControllerConfig::WiFi::CONNECTION_TIMEOUT_MS);
    TEST_ASSERT_EQUAL(80, ControllerConfig::WiFi::HTTP_PORT);
    TEST_ASSERT_EQUAL(3, ControllerConfig::WiFi::MAX_CONNECTION_ATTEMPTS);
    TEST_ASSERT_EQUAL(1000, ControllerConfig::WiFi::CONNECTION_RETRY_DELAY_MS);
}

/**
 * Test: Basic arithmetic to verify test framework works
 */
void test_basic_arithmetic(void)
{
    TEST_ASSERT_EQUAL(4, 2 + 2);
    TEST_ASSERT_TRUE(5 > 3);
    TEST_ASSERT_FALSE(2 > 10);
}

/**
 * Test: String operations
 */
void test_string_operations(void)
{
    const char *test_str = "OuttatimersController";
    TEST_ASSERT_EQUAL(21, strlen(test_str));
    TEST_ASSERT_EQUAL_STRING("OuttatimersController", test_str);
}

/**
 * Test: Configuration value ranges
 */
void test_config_value_ranges(void)
{
    // Verify LED pin is in valid GPIO range
    TEST_ASSERT_GREATER_OR_EQUAL(0, ControllerConfig::Hardware::LED_PIN);
    TEST_ASSERT_LESS_THAN(32, ControllerConfig::Hardware::LED_PIN);

    // Verify brightness is in valid range
    TEST_ASSERT_GREATER_OR_EQUAL(0, ControllerConfig::Hardware::DEFAULT_BRIGHTNESS);
    TEST_ASSERT_LESS_OR_EQUAL(255, ControllerConfig::Hardware::DEFAULT_BRIGHTNESS);

    // Verify timing values are reasonable
    TEST_ASSERT_GREATER_THAN(0, ControllerConfig::Timing::EFFECT_UPDATE_INTERVAL);
}

/**
 * Main function - runs all tests
 */
extern "C" void app_main(void)
{
    UNITY_BEGIN();

    // Configuration tests
    RUN_TEST(test_hardware_config_constants);
    RUN_TEST(test_effects_config_constants);
    RUN_TEST(test_wifi_config_constants);
    RUN_TEST(test_config_value_ranges);

    // Basic functionality tests
    RUN_TEST(test_basic_arithmetic);
    RUN_TEST(test_string_operations);

    UNITY_END();
}