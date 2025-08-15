#include "heaterctrl_cfg.h"

/**
 * @file heaterctrl_cfg.c
 * @brief Configuration data definitions for the HeaterControl component.
 *
 * This file contains the array of structures that defines all configured
 * heaters and their properties.
 */

// --- Array of Heater Configurations ---
const HeaterCtrl_Config_t heater_configs[HEATER_COUNT] = {
    {
        .id = HEATER_ID_MAIN,
        .gpio_pin = 22
    }
    // Add more heater configurations here
};