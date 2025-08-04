#include "lightctrl_cfg.h"

/**
 * @file lightctrl_cfg.c
 * @brief Configuration data definitions for the LightCtrl component.
 *
 * This file contains the array of structures that defines all configured
 * lights and their properties.
 */

// --- Array of Light Configurations ---
const LightCtrl_Config_t light_configs[LightCtrl_COUNT] = {
    {
        .id = LIGHT_ID_STATUS,
        .type = LightCtrl_TYPE_RELAY,
        .control_details.relay = {
            .relay_gpio_pin = MCAL_GPIO_PIN_21
        },
        .feedback_type = LightCtrl_FEEDBACK_TYPE_NONE
    },
    {
        .id = LIGHT_ID_CABINET,
        .type = LightCtrl_TYPE_PWM,
        .control_details.pwm = {
            .pwm_channel = MCAL_PWM_CHANNEL_2
        },
        .feedback_type = LightCtrl_FEEDBACK_TYPE_CURRENT_SENSOR,
        .feedback_details.analog_adc = {
            .adc_channel = MCAL_ADC_CHANNEL_4,
            .voltage_to_brightness_slope = 30.0f, // Example value
            .voltage_to_brightness_offset = 0.0f
        }
    },
    {
        .id = LIGHT_ID_READING,
        .type = LightCtrl_TYPE_PWM,
        .control_details.pwm = {
            .pwm_channel = MCAL_PWM_CHANNEL_3
        },
        .feedback_type = LightCtrl_FEEDBACK_TYPE_NONE
    }
};