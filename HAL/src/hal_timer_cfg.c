/**
 * @file hal_timer_cfg.c
 * @brief Hardware Abstraction Layer for Timer - Configuration Implementation.
 *
 * This file implements the static configuration parameters for each timer channel.
 * These settings are used by the `hal_timer.c` implementation to initialize
 * and operate the timer functionalities.
 *
 * IMPORTANT: Adjust these values according to your specific application's
 * timing requirements and desired default behavior.
 */

#include "hal_timer_cfg.h"
#include "hal_timer.h" // For HAL_TIMER_Channel_t and other enums

// Define the global array of timer channel configurations
const HAL_TIMER_ChannelConfig_t g_hal_timer_channel_configs[HAL_TIMER_CHANNEL_MAX] = {
    [HAL_TIMER_CHANNEL_0] = {
        .channel_id = HAL_TIMER_CHANNEL_0,
        .mode = HAL_TIMER_MODE_AUTO_RELOAD, // Example: Auto-reload mode
        .direction = HAL_TIMER_DIR_UP,      // Example: Count up
        .prescaler = 80,                    // For 80MHz APB_CLK, 80 prescaler gives 1 us tick
        .default_period_us = 1000000,       // Default period of 1 second (1,000,000 microseconds)
        .interrupt_priority = 1,            // Lowest priority interrupt
        // .default_callback = NULL,           // No default callback
        // .default_user_data = NULL,          // No default user data
    },
    [HAL_TIMER_CHANNEL_1] = {
        .channel_id = HAL_TIMER_CHANNEL_1,
        .mode = HAL_TIMER_MODE_ONE_SHOT, // Example: One-shot mode
        .direction = HAL_TIMER_DIR_UP,   // Example: Count up
        .prescaler = 80,                 // For 80MHz APB_CLK, 80 prescaler gives 1 us tick
        .default_period_us = 500000,     // Default period of 0.5 seconds (500,000 microseconds)
        .interrupt_priority = 2,         // Medium priority interrupt
        // .default_callback = NULL,        // No default callback
        // .default_user_data = NULL,       // No default user data
    },
    // Add configurations for more timer channels if defined in hal_timer.h
};
