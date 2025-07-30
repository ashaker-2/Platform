// ecual/src/ecual_common.c

#include "app_common.h"
#include "esp_timer.h" // For ESP32, this is the way to get high-resolution time

// ... existing implementations ...

uint64_t APP_GetUptimeMs(void) {
    // For ESP32, esp_timer_get_time() returns microseconds since boot.
    return esp_timer_get_time() / 1000;
}
