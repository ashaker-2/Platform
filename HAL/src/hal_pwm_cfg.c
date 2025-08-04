/**
 * @file hal_pwm_cfg.c
 * @brief Configuration definitions for the HAL PWM component.
 *
 * This file contains the actual definitions of the configuration data
 * for the HAL PWM module, mapping logical PWM channels to physical MCU channels/timers
 * and their default settings.
 */

#include "hal_pwm_cfg.h" // Include the configuration header
#include "hal_pwm.h"     // Include the public header for logical IDs

// Define the global array of HAL PWM configurations.
// The order of elements in this array MUST match the HAL_PWM_Channel_t enum.
const HAL_PWM_Config_t g_hal_pwm_configs[HAL_PWM_CHANNEL_COUNT] = {
    [HAL_PWM_CHANNEL_FAN_CONTROL] = {
        .logical_id                 = HAL_PWM_CHANNEL_FAN_CONTROL,
        .mcal_channel               = MCAL_PWM_CHANNEL_0,  // Example: Physical PWM channel 0
        .mcal_timer                 = MCAL_PWM_TIMER_0,    // Example: Using Timer 0
        .gpio_pin                   = HAL_GPIO_PIN_FAN_PWM, // Associated logical GPIO pin
        .default_frequency_hz       = 25000,               // 25 kHz for quiet fan control
        .default_duty_cycle_percent = 0                    // Fan off by default
    },
    [HAL_PWM_CHANNEL_LIGHT_DIMMING] = {
        .logical_id                 = HAL_PWM_CHANNEL_LIGHT_DIMMING,
        .mcal_channel               = MCAL_PWM_CHANNEL_1,  // Example: Physical PWM channel 1
        .mcal_timer                 = MCAL_PWM_TIMER_1,    // Example: Using Timer 1
        .gpio_pin                   = HAL_GPIO_PIN_LIGHT_RELAY, // Associated logical GPIO pin (if it's PWM capable)
        .default_frequency_hz       = 5000,                // 5 kHz for flicker-free light dimming
        .default_duty_cycle_percent = 0                    // Light off by default
    },
    // Add configurations for other logical PWM channels here
};
