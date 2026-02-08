#pragma once
// Mock ESP event system for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef ESP_OK
#define ESP_OK          0
#define ESP_FAIL        -1
#endif
typedef int esp_err_t;

// Event base type
typedef const char* esp_event_base_t;

// Event handler function type
typedef void (*esp_event_handler_t)(void* event_handler_arg,
                                   esp_event_base_t event_base,
                                   int32_t event_id,
                                   void* event_data);

// WiFi events
#define WIFI_EVENT ((esp_event_base_t)"WIFI_EVENT")
enum {
    WIFI_EVENT_STA_START = 0,
    WIFI_EVENT_STA_STOP,
    WIFI_EVENT_STA_CONNECTED,
    WIFI_EVENT_STA_DISCONNECTED,
    WIFI_EVENT_AP_START,
    WIFI_EVENT_AP_STOP
};

// IP events
#define IP_EVENT ((esp_event_base_t)"IP_EVENT")
enum {
    IP_EVENT_STA_GOT_IP = 0,
    IP_EVENT_STA_LOST_IP,
    IP_EVENT_AP_STAIPASSIGNED,
    IP_EVENT_GOT_IP6
};

#define ESP_EVENT_ANY_ID (-1)

// IP info structure
typedef struct {
    uint32_t ip;
    uint32_t netmask;
    uint32_t gw;
} esp_netif_ip_info_t;

typedef struct {
    esp_netif_ip_info_t ip_info;
    uint32_t ip_changed;
} ip_event_got_ip_t;

// Mock event functions
static inline esp_err_t esp_event_loop_create_default(void) {
    return ESP_OK;
}

static inline esp_err_t esp_event_handler_register(esp_event_base_t event_base,
                                                   int32_t event_id,
                                                   esp_event_handler_t event_handler,
                                                   void* event_handler_arg) {
    return ESP_OK;
}

static inline esp_err_t esp_event_handler_unregister(esp_event_base_t event_base,
                                                     int32_t event_id,
                                                     esp_event_handler_t event_handler) {
    return ESP_OK;
}

#ifdef __cplusplus
}
#endif