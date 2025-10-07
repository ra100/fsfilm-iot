#pragma once
// Mock ESP ROM system functions for host-based testing

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>

// Mock delay function using standard POSIX usleep
static inline void esp_rom_delay_us(uint32_t us) {
    usleep(us);
}

#ifdef __cplusplus
}
#endif