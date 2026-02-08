#pragma once
// Mock GPIO driver for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

// GPIO mode definitions
typedef enum {
    GPIO_MODE_INPUT = 0x01,
    GPIO_MODE_OUTPUT = 0x02,
    GPIO_MODE_OUTPUT_OD = 0x06,
    GPIO_MODE_INPUT_OUTPUT_OD = 0x07,
    GPIO_MODE_INPUT_OUTPUT = 0x03
} gpio_mode_t;

// GPIO number enum
typedef enum {
    GPIO_NUM_0 = 0,
    GPIO_NUM_1 = 1,
    GPIO_NUM_2 = 2,
    GPIO_NUM_3 = 3,
    GPIO_NUM_4 = 4,
    GPIO_NUM_5 = 5,
    GPIO_NUM_6 = 6,
    GPIO_NUM_7 = 7,
    GPIO_NUM_MAX = 8
} gpio_num_t;

// Error codes
#define ESP_OK          0
#define ESP_FAIL        -1
typedef int esp_err_t;

// Mock GPIO functions
static inline esp_err_t gpio_reset_pin(gpio_num_t gpio_num) {
    // Mock: do nothing on host
    return ESP_OK;
}

static inline esp_err_t gpio_set_direction(gpio_num_t gpio_num, gpio_mode_t mode) {
    // Mock: do nothing on host
    return ESP_OK;
}

static inline esp_err_t gpio_set_level(gpio_num_t gpio_num, uint32_t level) {
    // Mock: do nothing on host
    return ESP_OK;
}

static inline int gpio_get_level(gpio_num_t gpio_num) {
    // Mock: always return 0
    return 0;
}

#ifdef __cplusplus
}
#endif