#pragma once
// Mock FreeRTOS for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// Basic FreeRTOS types
typedef void* TaskHandle_t;
typedef uint32_t TickType_t;
typedef void (*TaskFunction_t)(void*);

// Basic FreeRTOS constants
#define portMAX_DELAY 0xFFFFFFFF
#define pdTRUE  1
#define pdFALSE 0
#define pdPASS  1
#define pdFAIL  0

// Mock tick conversion
static inline TickType_t pdMS_TO_TICKS(uint32_t ms) {
    return ms;
}

#ifdef __cplusplus
}
#endif