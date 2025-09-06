/* ============================================================================
 * SOURCE FILE: Application/heaterCtrl/src/heater_ctrl_cfg.c
 * ============================================================================*/
/**
 * @file heater_ctrl_cfg.c
 * @brief Implements the static array of Heater Control configuration settings.
 *
 * This file defines specific GPIO pins or CH423S expander pins used to control
 * each heater and their initial states.
 */

#include "heaterctrl_cfg.h" // Header for heater configuration types and extern declarations
#include "hal_cfg.h"        // Global hardware definitions for CH423S/GPIO pins
#include <stddef.h>         // For size_t

/**
 * @brief Array containing all predefined heater configurations.
 * Each element in this array maps a logical Heater_ID to its control type
 * and the specific pin (GPIO or CH423S GP) for control.
 */
const heater_config_item_t s_heater_configurations[] = {
    {
        .heater_id = HEATER_ID_0,
        .control_type = HEATER_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_HEATER_0, // Controlled by CH423S
        .initial_state = HEATER_STATE_OFF,
    },
    // Add more heater configurations here.
    // Ensure HW_CH423S_GP_HEATER_X and HW_GPIO_HEATER_X are defined in HAL_Config.h
};

/**
 * @brief Defines the number of elements in the `s_heater_configurations` array.
 */
const size_t s_num_heater_configurations = sizeof(s_heater_configurations) / sizeof(s_heater_configurations[0]);
