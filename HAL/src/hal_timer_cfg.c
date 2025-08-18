/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/src/HAL_Timer_Cfg.c
 * ============================================================================*/
/**
 * @file HAL_Timer_Cfg.c
 * @brief Implements the static array of hardware timer configuration settings.
 * This file defines the initial divider, counter direction, and auto-reload settings for timers.
 * It does not contain any initialization functions; its purpose is purely
 * to hold configuration data.
 */

#include "HAL_Timer_Cfg.h"  // Header for Timer configuration types and extern declarations
#include <stddef.h>         // For size_t

/**
 * @brief Array containing all predefined hardware timer configurations.
 * This is a placeholder; actual timer configurations would be defined here.
 * This array is made `const` and global (`extern` in header) to be accessed by `HAL_Timer.c`.
 */
const timer_cfg_item_t s_timer_configurations[] = {
    {
        .group_id = TIMER_GROUP_0,
        .timer_id = TIMER_0,
        .config = {
            .divider = 80,          // 80MHz APB_CLK / 80 = 1MHz, 1 tick = 1us
            .counter_dir = TIMER_COUNT_UP,
            .counter_en = TIMER_PAUSE,
            .intr_type = TIMER_INTR_LEVEL,
            .auto_reload = true,
        },
        .initial_alarm_value = 1000000, // Example: 1 second (1,000,000 microseconds)
    },
    // Add other timer configurations here if needed
    // Example for another timer:
    // {
    //     .group_id = TIMER_GROUP_0,
    //     .timer_id = TIMER_1,
    //     .config = {
    //         .divider = 16,
    //         .counter_dir = TIMER_COUNT_UP,
    //         .counter_en = TIMER_PAUSE,
    //         .intr_type = TIMER_INTR_LEVEL,
    //         .auto_reload = true,
    //     },
    //     .initial_alarm_value = 500000, // Example: 0.5 second
    // },
};

/**
 * @brief Defines the number of elements in the `s_timer_configurations` array.
 * This variable is made `const` and global (`extern` in header) to be accessed by `HAL_Timer.c`.
 */
const size_t s_num_timer_configurations = sizeof(s_timer_configurations) / sizeof(s_timer_configurations[0]);
