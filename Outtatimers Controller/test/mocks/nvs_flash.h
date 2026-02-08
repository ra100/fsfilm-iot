#pragma once
// Mock NVS flash for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef ESP_OK
#define ESP_OK          0
#define ESP_FAIL        -1
#define ESP_ERR_NVS_NO_FREE_PAGES -6
#define ESP_ERR_NVS_NEW_VERSION_FOUND -7
#endif
typedef int esp_err_t;

// Mock NVS functions
static inline esp_err_t nvs_flash_init(void) {
    // Mock: always succeed
    return ESP_OK;
}

static inline esp_err_t nvs_flash_erase(void) {
    // Mock: always succeed
    return ESP_OK;
}

#ifdef __cplusplus
}
#endif