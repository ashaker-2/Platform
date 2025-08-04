#include "ventctrl_cfg.h"

/**
 * @file ventctrl_cfg.c
 * @brief Configuration data definitions for the VentCtrl component.
 *
 * This file contains the array of structures that defines all configured
 * ventilators and their properties.
 */

// --- Array of Ventilator Configurations ---
const VentCtrl_Config_t vent_configs[VentCtrl_COUNT] = {
    {
        .id = VENT_ID_MAIN_EXHAUST,
        .type = VentCtrl_TYPE_PWM,
        .control_details.pwm = {
            .pwm_channel = MCAL_PWM_CHANNEL_1
        },
        .feedback_type = VentCtrl_FEEDBACK_TYPE_TACHOMETER,
        .feedback_details.tachometer = {
            .tachometer_gpio_pin = MCAL_GPIO_PIN_16
        }
    },
    {
        .id = VENT_ID_INTAKE_FAN,
        .type = VentCtrl_TYPE_RELAY,
        .control_details.relay = {
            .relay_gpio_pin = MCAL_GPIO_PIN_17
        },
        .feedback_type = VentCtrl_FEEDBACK_TYPE_NONE
    }
};