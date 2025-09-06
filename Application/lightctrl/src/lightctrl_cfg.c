/* ============================================================================
 * SOURCE FILE: Application/lightCtrl/src/light_ctrl_cfg.c
 * ============================================================================*/
/**
 * @file light_ctrl_cfg.c
 * @brief Implements the static array of Light Control configuration settings.
 *
 * This file defines specific GPIO pins or CH423S expander pins used to control
 * each light and their initial states.
 */

#include "lightctrl_cfg.h"   // Header for light configuration types and extern declarations
#include "hal_cfg.h"     // Global hardware definitions for CH423S/GPIO pins
#include <stddef.h>         // For size_t

/**
 * @brief Array containing all predefined light configurations.
 * Each element in this array maps a logical Light_ID to its control type
 * and the specific pin (GPIO or CH423S GP) for control.
 */
const light_config_item_t s_light_configurations[] = {
    {
        .light_id = LIGHT_ID_0,
        .control_type = LIGHT_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_LIGHT, // Controlled by CH423S
        .initial_state = LIGHT_STATE_OFF,
    },
    // Add more light configurations here.
    // Ensure HW_CH423S_GP_LIGHT_X and HW_GPIO_LIGHT_X are defined in HAL_Config.h
};

/**
 * @brief Defines the number of elements in the `s_light_configurations` array.
 */
const size_t s_num_light_configurations = sizeof(s_light_configurations) / sizeof(s_light_configurations[0]);
