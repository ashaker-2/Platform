#include "pumpctrl_cfg.h"

/**
 * @file pumpctrl_cfg.c
 * @brief Configuration data definitions for the PumpCtrl component.
 *
 * This file contains the array of structures that defines all configured
 * pumps and their properties.
 */

// --- Array of Pump Configurations ---
const PumpCtrl_Config_t pump_configs[PumpCtrl_COUNT] = {
    {
        .id = PUMP_ID_HUMIDITY,
        .type = PumpCtrl_TYPE_RELAY,
        .control_details.relay_gpio_pin = 13,
        .feedback_type = PumpCtrl_FEEDBACK_TYPE_FLOW_SENSOR, // Example: flow sensor
        .feedback_details.flow_sensor_pulse = {
            .gpio_pin = 14,
            .pulses_per_liter = 450.0f, // Example value
            .flow_threshold_on = 0.1f    // Liters/second
        }
    },
    {
        .id = PUMP_ID_DRAIN,
        .type = PumpCtrl_TYPE_RELAY,
        .control_details.relay_gpio_pin = 15,
        .feedback_type = PumpCtrl_FEEDBACK_TYPE_NONE // No feedback for this pump
    },
    // Add more pump configurations here
};