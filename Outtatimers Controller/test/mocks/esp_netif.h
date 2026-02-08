#pragma once
// Mock ESP network interface for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef ESP_OK
#define ESP_OK          0
#define ESP_FAIL        -1
#endif
typedef int esp_err_t;

// Network interface type
typedef void* esp_netif_t;

// IP info structure
typedef struct {
    uint32_t ip;
    uint32_t netmask;
    uint32_t gw;
} esp_netif_ip_info_t;

// IP string formatting macros
#define IP2STR(ipaddr) ((uint8_t*)ipaddr)[0], ((uint8_t*)ipaddr)[1], ((uint8_t*)ipaddr)[2], ((uint8_t*)ipaddr)[3]
#define IPSTR "%d.%d.%d.%d"

// Mock netif functions
static inline esp_err_t esp_netif_init(void) {
    return ESP_OK;
}

static inline esp_netif_t* esp_netif_create_default_wifi_sta(void) {
    return (esp_netif_t*)1; // Fake handle
}

static inline esp_netif_t* esp_netif_create_default_wifi_ap(void) {
    return (esp_netif_t*)1; // Fake handle
}

static inline esp_netif_t* esp_netif_get_handle_from_ifkey(const char* if_key) {
    return (esp_netif_t*)1; // Fake handle
}

static inline esp_err_t esp_netif_get_ip_info(esp_netif_t *esp_netif, esp_netif_ip_info_t *ip_info) {
    // Mock: return fake IP address 192.168.1.100
    if (ip_info) {
        ip_info->ip = 0x6401A8C0; // 192.168.1.100 in network byte order
        ip_info->netmask = 0x00FFFFFF; // 255.255.255.0
        ip_info->gw = 0x0101A8C0; // 192.168.1.1
    }
    return ESP_OK;
}

#ifdef __cplusplus
}
#endif