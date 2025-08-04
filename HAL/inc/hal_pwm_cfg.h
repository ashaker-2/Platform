/**
 * @file hal_pwm_cfg.h
 * @brief Configuration header for the HAL PWM component.
 *
 * This file defines the hardware-specific mapping and default configurations
 * for the logical PWM channels used by the HAL PWM module.
 */

#ifndef HAL_PWM_CFG_H
#define HAL_PWM_CFG_H

#include "hal_pwm.h"   // For HAL_PWM_Channel_t
#include "mcal_pwm.h"  // For MCAL_PWM_Channel_t, MCAL_PWM_Timer_t etc. (MCU-specific)
#include "hal_gpio.h"  // For HAL_GPIO_Pin_t (if PWM output is on a GPIO pin)

// --- Configuration Structure for each Logical PWM Channel ---
/**
 * @brief Structure to hold the static configuration for each logical PWM channel.
 */
typedef struct {
    HAL_PWM_Channel_t       logical_id;         /**< The logical ID of the PWM channel. */
    MCAL_PWM_Channel_t      mcal_channel;       /**< The corresponding MCAL (physical) PWM channel. */
    MCAL_PWM_Timer_t        mcal_timer;         /**< The MCAL timer associated with this PWM channel. */
    HAL_GPIO_Pin_t          gpio_pin;           /**< The logical GPIO pin associated with this PWM output. */
    uint32_t                default_frequency_hz; /**< Default PWM frequency in Hertz. */
    uint8_t                 default_duty_cycle_percent; /**< Default initial duty cycle (0-100%). */
    // Add more configuration parameters as needed (e.g., polarity, resolution bits)
} HAL_PWM_Config_t;

// --- External Declaration of Configuration Array ---
/**
 * @brief Global array containing the configuration for all logical PWM channels.
 * This array is defined in hal_pwm_cfg.c and accessed by the HAL_PWM module.
 * The order of elements in this array must match the HAL_PWM_Channel_t enum.
 */
extern const HAL_PWM_Config_t g_hal_pwm_configs[HAL_PWM_CHANNEL_COUNT];

#endif // HAL_PWM_CFG_H
