/* ============================================================================
 * SOURCE FILE: Application/fanCtrl/src/fan_ctrl_cfg.c
 * ============================================================================*/
/**
 * @file fan_ctrl_cfg.c
 * @brief Implements the static array of Fan Control configuration settings.
 *
 * This file defines specific GPIO pins or CH423S expander pins used to control
 * each fan and their initial states.
 */

#include "fanctrl_cfg.h"   // Header for fan configuration types and extern declarations
#include "hal_cfg.h"     // Global hardware definitions for CH423S/GPIO pins
#include <stddef.h>         // For size_t

/**
 * @brief Array containing all predefined fan configurations.
 * Each element in this array maps a logical Fan_ID to its control type
 * and the specific pin (GPIO or CH423S GP) for control.
 */
const fan_config_item_t s_fan_configurations[] = {
    {
        .fan_id = FAN_ID_0,
        .control_type = FAN_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_FAN_0, // Controlled by CH423S
        .initial_state = FAN_STATE_OFF,
    },
    {
        .fan_id = FAN_ID_1,
        .control_type = FAN_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_FAN_1, // Controlled by CH423S
        .initial_state = FAN_STATE_OFF,
    },
    {
        .fan_id = FAN_ID_2,
        .control_type = FAN_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_FAN_2, // Controlled by CH423S
        .initial_state = FAN_STATE_OFF,
    },
    {
        .fan_id = FAN_ID_3,
        .control_type = FAN_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_FAN_3,         // Controlled by direct ESP32 GPIO
        .initial_state = FAN_STATE_OFF,
    },
    // Add more fan configurations here.
    // Ensure HW_CH423S_GP_FAN_X and HW_GPIO_FAN_X are defined in HAL_Config.h
};

/**
 * @brief Defines the number of elements in the `s_fan_configurations` array.
 */
const size_t s_num_fan_configurations = sizeof(s_fan_configurations) / sizeof(s_fan_configurations[0]);
