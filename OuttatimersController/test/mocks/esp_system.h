#pragma once
// Mock ESP system functions for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef ESP_OK
#define ESP_OK          0
#define ESP_FAIL        -1
#define ESP_ERR_INVALID_ARG -2
#define ESP_ERR_NO_MEM -3
#define ESP_ERR_INVALID_STATE -4
#define ESP_ERR_NOT_FOUND -5
#define ESP_ERR_NVS_NO_FREE_PAGES -6
#define ESP_ERR_NVS_NEW_VERSION_FOUND -7
#endif
typedef int esp_err_t;

// Mock system functions
static inline void esp_restart(void) {
    // Mock: do nothing on host
}

#ifdef __cplusplus
}
#endif