/**
 * @file hal_pwm.h
 * @brief Public header for the Hardware Abstraction Layer (HAL) PWM component.
 *
 * This component provides a hardware-independent interface for configuring and
 * controlling Pulse Width Modulation (PWM) outputs. It abstracts the
 * microcontroller-specific timer/PWM module access and provides a clean API for
 * higher-level modules, such as fan control or light dimming.
 */

#ifndef HAL_PWM_H
#define HAL_PWM_H

#include "common.h" // For Status_t and Status_t
#include <stdint.h>   // For uint8_t, uint32_t

// --- PWM Channel Definitions (Logical IDs) ---
// These are logical IDs that map to physical PWM channels/timers in hal_pwm_cfg.c
typedef enum {
    HAL_PWM_CHANNEL_FAN_CONTROL = 0,    /**< Logical ID for the fan speed control PWM. */
    HAL_PWM_CHANNEL_LIGHT_DIMMING,      /**< Logical ID for light dimming PWM. */
    // Add more logical PWM channels as needed
    HAL_PWM_CHANNEL_COUNT               /**< Total number of logical PWM channels. */
} HAL_PWM_Channel_t;

// --- Public Functions ---

/**
 * @brief Initializes the HAL PWM module.
 * This function should be called once during system startup.
 * It prepares the internal data structures and potentially the underlying MCAL.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_PWM_Init(void);

/**
 * @brief Configures a specific PWM channel.
 * This sets the frequency and initial duty cycle for the PWM output.
 * The duty cycle is typically a percentage (0-100) or a raw value (0-max_resolution).
 * @param channel The logical ID of the PWM channel to configure.
 * @param frequency_hz The desired PWM frequency in Hertz.
 * @param initial_duty_cycle The initial duty cycle (0-100 for percentage).
 * @return E_OK on success, E_NOK on failure (e.g., invalid channel, MCAL error).
 */
Status_t HAL_PWM_ConfigChannel(HAL_PWM_Channel_t channel,
                                   uint32_t frequency_hz,
                                   uint8_t initial_duty_cycle_percent);

/**
 * @brief Sets the duty cycle for a specific PWM channel.
 * @param channel The logical ID of the PWM channel.
 * @param duty_cycle The desired duty cycle (0-100 for percentage).
 * @return E_OK on success, E_NOK on failure (e.g., channel not configured, invalid duty cycle, MCAL error).
 */
Status_t HAL_PWM_SetDutyCycle(HAL_PWM_Channel_t channel, uint8_t duty_cycle_percent);

/**
 * @brief Starts the PWM output on a specific channel.
 * @param channel The logical ID of the PWM channel to start.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_PWM_Start(HAL_PWM_Channel_t channel);

/**
 * @brief Stops the PWM output on a specific channel.
 * The output pin will typically be set to a defined low state.
 * @param channel The logical ID of the PWM channel to stop.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_PWM_Stop(HAL_PWM_Channel_t channel);

#endif // HAL_PWM_H


