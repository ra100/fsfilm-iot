#pragma once
// Mock FreeRTOS task API for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include <unistd.h>

// Mock task creation - just returns success without creating a task
static inline int xTaskCreate(TaskFunction_t pvTaskCode,
                              const char * const pcName,
                              uint32_t usStackDepth,
                              void *pvParameters,
                              uint32_t uxPriority,
                              TaskHandle_t *pxCreatedTask) {
    // Mock: do nothing on host, just return success
    if (pxCreatedTask) {
        *pxCreatedTask = (TaskHandle_t)1; // Fake handle
    }
    return pdPASS;
}

// Mock task delay using standard POSIX sleep
static inline void vTaskDelay(TickType_t xTicksToDelay) {
    // Convert ticks to milliseconds and sleep
    usleep(xTicksToDelay * 1000);
}

// Mock task delete
static inline void vTaskDelete(TaskHandle_t xTask) {
    // Mock: do nothing on host
}

#ifdef __cplusplus
}
#endif