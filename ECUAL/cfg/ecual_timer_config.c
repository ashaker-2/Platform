// ecual/src/ecual_timer_config.c

#include "ecual_timer_config.h" // Includes ecual_timer.h for relevant definitions

// Dummy callback function (to avoid NULL pointer issues if not explicitly set)
static void default_timer_callback(void *arg) {
    (void)arg; // Suppress unused argument warning
    // This function does nothing. User should register real callbacks.
}

// Definition of the static constant array for Timer configurations
const ECUAL_TIMER_Config_t timer_configurations[] = {
    // ECUAL_TIMER_APP_UPDATE (Maps to TIMER_GROUP_0, TIMER_0)
    {
        .group            = ECUAL_TIMER_GROUP_0,
        .index            = ECUAL_TIMER_INDEX_0,
        .divider          = 80,                 // 80MHz / 80 = 1MHz clock (1us per tick)
        .count_direction  = ECUAL_TIMER_COUNT_UP,
        .initial_count    = 0,
        .enable_alarm     = true,
        .alarm_value      = 100000,             // 100,000 ticks = 100ms
        .auto_reload      = ECUAL_TIMER_AUTORELOAD_ENABLE,
        .enable_interrupt = true,
        .callback         = default_timer_callback, // Default callback
        .callback_arg     = NULL
    },
    // ECUAL_TIMER_HEARTBEAT (Maps to TIMER_GROUP_0, TIMER_1)
    {
        .group            = ECUAL_TIMER_GROUP_0,
        .index            = ECUAL_TIMER_INDEX_1,
        .divider          = 80000,                // 80MHz / 80000 = 1kHz clock (1ms per tick)
        .count_direction  = ECUAL_TIMER_COUNT_UP,
        .initial_count    = 0,
        .enable_alarm     = true,
        .alarm_value      = 1000,                 // 1,000 ticks = 1 second
        .auto_reload      = ECUAL_TIMER_AUTORELOAD_ENABLE,
        .enable_interrupt = true,
        .callback         = default_timer_callback, // Default callback
        .callback_arg     = NULL
    }
};

// Definition of the number of configurations in the array
const uint32_t ECUAL_NUM_TIMER_CONFIGURATIONS = sizeof(timer_configurations) / sizeof(ECUAL_TIMER_Config_t);

// Internal mapping from ECUAL_TIMER_ID_t to Timer Group/Index
// This array MUST be in the same order as ECUAL_TIMER_ID_t enum
const ECUAL_TIMER_Internal_Map_t timer_id_map[ECUAL_TIMER_TOTAL_TIMERS] = {
    // Corresponding to ECUAL_TIMER_APP_UPDATE (index 0)
    { .group = ECUAL_TIMER_GROUP_0, .index = ECUAL_TIMER_INDEX_0 },
    // Corresponding to ECUAL_TIMER_HEARTBEAT (index 1)
    { .group = ECUAL_TIMER_GROUP_0, .index = ECUAL_TIMER_INDEX_1 }
};
