#pragma once
// Mock ESP timer for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/time.h>

#ifndef ESP_OK
#define ESP_OK          0
#define ESP_FAIL        -1
#endif
typedef int esp_err_t;

// Mock timer function that returns microseconds since epoch
static inline int64_t esp_timer_get_time(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t)tv.tv_sec * 1000000LL + (int64_t)tv.tv_usec;
}

#ifdef __cplusplus
}
#endif