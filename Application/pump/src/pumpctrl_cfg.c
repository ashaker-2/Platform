/* ============================================================================
 * SOURCE FILE: Application/pumpCtrl/src/pump_ctrl_cfg.c
 * ============================================================================*/
/**
 * @file pump_ctrl_cfg.c
 * @brief Implements the static array of Pump Control configuration settings.
 *
 * This file defines specific GPIO pins or CH423S expander pins used to control
 * each pump and their initial states.
 */

#include "pumpctrl_cfg.h"   // Header for pump configuration types and extern declarations
#include "hal_cfg.h"     // Global hardware definitions for CH423S/GPIO pins
#include <stddef.h>         // For size_t

/**
 * @brief Array containing all predefined pump configurations.
 * Each element in this array maps a logical Pump_ID to its control type
 * and the specific pin (GPIO or CH423S GP) for control.
 */
const pump_config_item_t s_pump_configurations[] = {
    {
        .pump_id = PUMP_ID_0,
        .control_type = PUMP_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_PUMP_0, // Controlled by CH423S
        .initial_state = PUMP_STATE_OFF,
    }
    // Add more pump configurations here.
    // Ensure HW_CH423S_GP_PUMP_X and HW_GPIO_PUMP_X are defined in HAL_Config.h
};

/**
 * @brief Defines the number of elements in the `s_pump_configurations` array.
 */
const size_t s_num_pump_configurations = sizeof(s_pump_configurations) / sizeof(s_pump_configurations[0]);
