#pragma once
// Mock ESP WiFi driver for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

// Error codes
#ifndef ESP_OK
#define ESP_OK          0
#define ESP_FAIL        -1
#endif
typedef int esp_err_t;

// WiFi mode
typedef enum {
    WIFI_MODE_NULL = 0,
    WIFI_MODE_STA,
    WIFI_MODE_AP,
    WIFI_MODE_APSTA,
    WIFI_MODE_MAX
} wifi_mode_t;

// WiFi interface
typedef enum {
    WIFI_IF_STA = 0,
    WIFI_IF_AP,
    WIFI_IF_MAX
} wifi_interface_t;

// WiFi auth mode
typedef enum {
    WIFI_AUTH_OPEN = 0,
    WIFI_AUTH_WEP,
    WIFI_AUTH_WPA_PSK,
    WIFI_AUTH_WPA2_PSK,
    WIFI_AUTH_WPA_WPA2_PSK,
    WIFI_AUTH_MAX
} wifi_auth_mode_t;

// WiFi configuration structures
typedef struct {
    uint8_t ssid[32];
    uint8_t password[64];
    struct {
        wifi_auth_mode_t authmode;
    } threshold;
} wifi_sta_config_t;

typedef struct {
    uint8_t ssid[32];
    uint8_t password[64];
} wifi_ap_config_t;

typedef union {
    wifi_sta_config_t sta;
    wifi_ap_config_t ap;
} wifi_config_t;

typedef struct {
    // Simplified init config
    int dummy;
} wifi_init_config_t;

#define WIFI_INIT_CONFIG_DEFAULT() { .dummy = 0 }

// Mock WiFi functions
static inline esp_err_t esp_wifi_init(const wifi_init_config_t *config) {
    return ESP_OK;
}

static inline esp_err_t esp_wifi_set_mode(wifi_mode_t mode) {
    return ESP_OK;
}

static inline esp_err_t esp_wifi_set_config(wifi_interface_t interface, wifi_config_t *conf) {
    return ESP_OK;
}

static inline esp_err_t esp_wifi_start(void) {
    return ESP_OK;
}

static inline esp_err_t esp_wifi_stop(void) {
    return ESP_OK;
}

static inline esp_err_t esp_wifi_connect(void) {
    return ESP_OK;
}

static inline esp_err_t esp_wifi_disconnect(void) {
    return ESP_OK;
}

#ifdef __cplusplus
}
#endif