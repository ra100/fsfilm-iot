#pragma once
// Mock ESP logging for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

// Log levels
typedef enum {
    ESP_LOG_NONE,
    ESP_LOG_ERROR,
    ESP_LOG_WARN,
    ESP_LOG_INFO,
    ESP_LOG_DEBUG,
    ESP_LOG_VERBOSE
} esp_log_level_t;

// Mock logging macros that print to stdout
#define ESP_LOGE(tag, format, ...) printf("[E][%s] " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) printf("[W][%s] " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGI(tag, format, ...) printf("[I][%s] " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGD(tag, format, ...) printf("[D][%s] " format "\n", tag, ##__VA_ARGS__)
#define ESP_LOGV(tag, format, ...) printf("[V][%s] " format "\n", tag, ##__VA_ARGS__)

// Error checking macro
#define ESP_ERROR_CHECK(x) do { \
    esp_err_t rc = (x); \
    if (rc != ESP_OK) { \
        printf("ESP_ERROR_CHECK failed: %d\n", rc); \
    } \
} while(0)

#ifndef ESP_OK
#define ESP_OK 0
#define ESP_FAIL -1
#endif
typedef int esp_err_t;

#ifdef __cplusplus
}
#endif