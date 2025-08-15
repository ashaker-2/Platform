/**
 * @file hal_gpio_cfg.c
 * @brief Configuration definitions for the HAL GPIO component.
 *
 * This file contains the actual definitions of the configuration data
 * for the HAL GPIO module, mapping logical GPIO IDs to physical MCU pins
 * and their default settings.
 */

#include "hal_gpio_cfg.h" // Include the configuration header
#include "hal_gpio.h"     // Include the public header for logical IDs

// Define the global array of HAL GPIO configurations.
// The order of elements in this array MUST match the HAL_GPIO_Pin_t enum.
const HAL_GPIO_Config_t g_hal_gpio_configs[HAL_GPIO_PIN_COUNT] = {
    [HAL_GPIO_PIN_LED_STATUS] = {
        .logical_id             = HAL_GPIO_PIN_LED_STATUS,
        .mcal_pin               = 14,  // Example: Physical pin 14
        .default_direction      = HAL_GPIO_DIR_OUTPUT,
        .default_initial_state  = HAL_GPIO_STATE_LOW, // LED off by default
        .default_pull_mode      = HAL_GPIO_PULL_NONE,
        .is_interrupt_capable   = false
    },
    [HAL_GPIO_PIN_BUTTON_MODE] = {
        .logical_id             = HAL_GPIO_PIN_BUTTON_MODE,
        .mcal_pin               = 20,  // Example: Physical pin 0
        .default_direction      = HAL_GPIO_DIR_INPUT,
        .default_initial_state  = HAL_GPIO_STATE_LOW,
        .default_pull_mode      = HAL_GPIO_PULL_UP, // Button with internal pull-up
        .is_interrupt_capable   = true
    },
    [HAL_GPIO_PIN_HEATER_RELAY] = {
        .logical_id             = HAL_GPIO_PIN_HEATER_RELAY,
        .mcal_pin               = 4,  // Example: Physical pin 4
        .default_direction      = HAL_GPIO_DIR_OUTPUT,
        .default_initial_state  = HAL_GPIO_STATE_LOW, // Heater off by default
        .default_pull_mode      = HAL_GPIO_PULL_NONE,
        .is_interrupt_capable   = false
    },
    [HAL_GPIO_PIN_FAN_PWM] = {
        .logical_id             = HAL_GPIO_PIN_FAN_PWM,
        .mcal_pin               = 5,  // Example: Physical pin 5 (often used for PWM)
        .default_direction      = HAL_GPIO_DIR_OUTPUT,
        .default_initial_state  = HAL_GPIO_STATE_LOW, // Fan off by default
        .default_pull_mode      = HAL_GPIO_PULL_NONE,
        .is_interrupt_capable   = false
    },
    [HAL_GPIO_PIN_PUMP_RELAY] = {
        .logical_id             = HAL_GPIO_PIN_PUMP_RELAY,
        .mcal_pin               = 6,  // Example: Physical pin 6
        .default_direction      = HAL_GPIO_DIR_OUTPUT,
        .default_initial_state  = HAL_GPIO_STATE_LOW, // Pump off by default
        .default_pull_mode      = HAL_GPIO_PULL_NONE,
        .is_interrupt_capable   = false
    },
    [HAL_GPIO_PIN_VENTILATOR_RELAY] = {
        .logical_id             = HAL_GPIO_PIN_VENTILATOR_RELAY,
        .mcal_pin               = 7,  // Example: Physical pin 7
        .default_direction      = HAL_GPIO_DIR_OUTPUT,
        .default_initial_state  = HAL_GPIO_STATE_LOW, // Ventilator off by default
        .default_pull_mode      = HAL_GPIO_PULL_NONE,
        .is_interrupt_capable   = false
    },
    [HAL_GPIO_PIN_LIGHT_RELAY] = {
        .logical_id             = HAL_GPIO_PIN_LIGHT_RELAY,
        .mcal_pin               = 8,  // Example: Physical pin 8
        .default_direction      = HAL_GPIO_DIR_OUTPUT,
        .default_initial_state  = HAL_GPIO_STATE_LOW, // Light off by default
        .default_pull_mode      = HAL_GPIO_PULL_NONE,
        .is_interrupt_capable   = false
    },
    [HAL_GPIO_PIN_TEMP_SENSOR_ADC_IN] = {
        .logical_id             = HAL_GPIO_PIN_TEMP_SENSOR_ADC_IN,
        .mcal_pin               = 36, // Example: Analog pin 36 (ESP32 ADC1_CH0)
        .default_direction      = HAL_GPIO_DIR_INPUT, // Configured as input for ADC
        .default_initial_state  = HAL_GPIO_STATE_LOW,
        .default_pull_mode      = HAL_GPIO_PULL_NONE,
        .is_interrupt_capable   = false // ADC pins usually not interrupt capable
    },
    [HAL_GPIO_PIN_HUMIDITY_SENSOR_ADC_IN] = {
        .logical_id             = HAL_GPIO_PIN_HUMIDITY_SENSOR_ADC_IN,
        .mcal_pin               = 37, // Example: Analog pin 37 (ESP32 ADC1_CH1)
        .default_direction      = HAL_GPIO_DIR_INPUT, // Configured as input for ADC
        .default_initial_state  = HAL_GPIO_STATE_LOW,
        .default_pull_mode      = HAL_GPIO_PULL_NONE,
        .is_interrupt_capable   = false // ADC pins usually not interrupt capable
    },
    // Add configurations for other logical GPIO pins here
};
