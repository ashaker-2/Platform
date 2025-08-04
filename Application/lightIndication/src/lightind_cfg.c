#include "lightind_cfg.h"

/**
 * @file lightind_cfg.c
 * @brief Configuration data definitions for the LightIndication component.
 *
 * This file contains the array of structures that defines all configured
 * light indicators and their properties.
 */

// --- Array of Light Indicator Configurations ---
const LightInd_Config_t light_ind_configs[LightInd_COUNT] = {
    {
        .id = LIGHT_INDICATION_ID_STATUS,
        .gpio_pin = MCAL_GPIO_PIN_18
    },
    {
        .id = LIGHT_INDICATION_ID_CRITICAL_ALARM,
        .gpio_pin = MCAL_GPIO_PIN_19
    },
    {
        .id = LIGHT_INDICATION_ID_WARNING,
        .gpio_pin = MCAL_GPIO_PIN_20
    }
    // Add more indicator configurations here
};