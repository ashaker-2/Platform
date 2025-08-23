/* ============================================================================
 * SOURCE FILE: Application/venCtrl/src/ven_ctrl_cfg.c
 * ============================================================================*/
/**
 * @file ven_ctrl_cfg.c
 * @brief Implements the static array of Ven Control configuration settings.
 *
 * This file defines specific GPIO pins or CH423S expander pins used to control
 * each ven and their initial states.
 */

#include "venctrl_cfg.h"   // Header for ven configuration types and extern declarations
#include "hal_cfg.h"     // Global hardware definitions for CH423S/GPIO pins
#include <stddef.h>         // For size_t

/**
 * @brief Array containing all predefined ven configurations.
 * Each element in this array maps a logical Ven_ID to its control type
 * and the specific pin (GPIO or CH423S GP) for control.
 */
const ven_config_item_t s_ven_configurations[] = {
    {
        .ven_id = VEN_ID_1,
        .control_type = VEN_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_VEN_1, // Controlled by CH423S
        .initial_state = VEN_STATE_OFF,
    },
    {
        .ven_id = VEN_ID_2,
        .control_type = VEN_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_VEN_2, // Controlled by CH423S
        .initial_state = VEN_STATE_OFF,
    },
    {
        .ven_id = VEN_ID_3,
        .control_type = VEN_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_VEN_3,         // Controlled by direct ESP32 GPIO
        .initial_state = VEN_STATE_OFF,
    },
    {
        .ven_id = VEN_ID_4,
        .control_type = VEN_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_VEN_4,         // Controlled by direct ESP32 GPIO
        .initial_state = VEN_STATE_OFF,
    },
    // Add more ven configurations here.
    // Ensure HW_CH423S_GP_VEN_X and HW_GPIO_VEN_X are defined in HAL_Config.h
};

/**
 * @brief Defines the number of elements in the `s_ven_configurations` array.
 */
const size_t s_num_ven_configurations = sizeof(s_ven_configurations) / sizeof(s_ven_configurations[0]);
