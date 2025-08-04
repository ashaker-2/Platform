/**
 * @file hal_timer_cfg.h
 * @brief Hardware Abstraction Layer for Timer - Configuration Header.
 *
 * This header defines the compile-time configuration parameters for each
 * timer channel. These settings are typically defined in `hal_timer_cfg.c`
 * and are used during the initialization of the Timer HAL.
 */

#ifndef HAL_TIMER_CFG_H
#define HAL_TIMER_CFG_H

#include <stdint.h>
#include "hal_timer.h" // Include the main Timer HAL interface for enums and types

/**
 * @brief Structure to hold compile-time configuration for a single Timer channel.
 *
 * This structure defines various parameters that configure the timer's behavior,
 * such as its mode, counting direction, prescaler, and default period.
 */
typedef struct
{
    HAL_TIMER_Channel_t channel_id;     /**< The logical ID of the timer channel. */
    HAL_TIMER_Mode_t mode;              /**< Timer operation mode (One-shot or Auto-reload). */
    HAL_TIMER_Direction_t direction;    /**< Timer counting direction (Up or Down). */
    uint32_t prescaler;                 /**< Timer prescaler value (e.g., 80 for 1 us tick with 80MHz APB_CLK). */
    uint64_t default_period_us;         /**< Default period in microseconds for the timer. */
    uint8_t interrupt_priority;         /**< Interrupt priority for the timer (1 to 3 on ESP32, 1 being lowest). */
    HAL_TIMER_Callback_t default_callback; /**< Default callback function. Can be NULL. */
    void *default_user_data;            /**< Default user data for the callback. Can be NULL. */
} HAL_TIMER_ChannelConfig_t;

/**
 * @brief External declaration for the global array of timer channel configurations.
 *
 * This array is defined in `hal_timer_cfg.c` and provides the static
 * configuration for all available timer channels.
 */
extern const HAL_TIMER_ChannelConfig_t g_hal_timer_channel_configs[HAL_TIMER_CHANNEL_MAX];

#endif /* HAL_TIMER_CFG_H */
