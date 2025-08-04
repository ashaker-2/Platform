/**
 * @file hal_gpio_cfg.h
 * @brief Configuration header for the HAL GPIO component.
 *
 * This file defines the hardware-specific mapping and default configurations
 * for the logical GPIO pins used by the HAL GPIO module.
 */

#ifndef HAL_GPIO_CFG_H
#define HAL_GPIO_CFG_H

#include "hal_gpio.h" // For HAL_GPIO_Pin_t, HAL_GPIO_Direction_t, etc.
#include "mcal_gpio.h" // For MCAL_GPIO_Pin_t, MCAL_GPIO_InterruptType_t etc. (MCU-specific)

// --- Configuration Structure for each Logical GPIO Pin ---
/**
 * @brief Structure to hold the static configuration for each logical GPIO pin.
 */
typedef struct {
    HAL_GPIO_Pin_t          logical_id;     /**< The logical ID of the GPIO pin. */
    MCAL_GPIO_Pin_t         mcal_pin;       /**< The corresponding MCAL (physical) pin number. */
    HAL_GPIO_Direction_t    default_direction; /**< Default direction at initialization. */
    HAL_GPIO_State_t        default_initial_state; /**< Default initial state for outputs. */
    HAL_GPIO_PullMode_t     default_pull_mode; /**< Default pull mode for inputs. */
    bool                    is_interrupt_capable; /**< True if this pin can generate interrupts. */
    // Add more configuration parameters as needed (e.g., drive strength, speed)
} HAL_GPIO_Config_t;

// --- External Declaration of Configuration Array ---
/**
 * @brief Global array containing the configuration for all logical GPIO pins.
 * This array is defined in hal_gpio_cfg.c and accessed by the HAL_GPIO module.
 * The order of elements in this array must match the HAL_GPIO_Pin_t enum.
 */
extern const HAL_GPIO_Config_t g_hal_gpio_configs[HAL_GPIO_PIN_COUNT];

#endif // HAL_GPIO_CFG_H
