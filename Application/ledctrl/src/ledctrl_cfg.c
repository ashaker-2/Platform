/* ============================================================================
 * SOURCE FILE: Application/ledCtrl/src/led_ctrl_cfg.c
 * ============================================================================*/
/**
 * @file led_ctrl_cfg.c
 * @brief Implements the static array of Led Control configuration settings.
 *
 * This file defines specific GPIO pins or CH423S expander pins used to control
 * each led and their initial states.
 */

#include "ledctrl_cfg.h"   // Header for led configuration types and extern declarations
#include "hal_cfg.h"     // Global hardware definitions for CH423S/GPIO pins
#include <stddef.h>         // For size_t

/**
 * @brief Array containing all predefined led configurations.
 * Each element in this array maps a logical Led_ID to its control type
 * and the specific pin (GPIO or CH423S GP) for control.
 */
const led_config_item_t s_led_configurations[] = {
    {
        .led_id = LED_ID_1,
        .control_type = LED_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_FAN_LED, // Controlled by CH423S
        .initial_state = LED_STATE_OFF,
    },
    {
        .led_id = LED_ID_2,
        .control_type = LED_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_VEN_LED, // Controlled by CH423S
        .initial_state = LED_STATE_OFF,
    },
    {
        .led_id = LED_ID_3,
        .control_type = LED_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_LIGHT_LED,         // Controlled by direct ESP32 GPIO
        .initial_state = LED_STATE_OFF,
    },
    {
        .led_id = LED_ID_4,
        .control_type = LED_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_PUMP_LED,         // Controlled by direct ESP32 GPIO
        .initial_state = LED_STATE_OFF,
    },
    {
        .led_id = LED_ID_5,
        .control_type = LED_CONTROL_TYPE_IO_EXPANDER,
        .pinNum = HW_CH423S_GP_HEATER_LED,         // Controlled by direct ESP32 GPIO
        .initial_state = LED_STATE_OFF,
    },
    // Add more led configurations here.
    // Ensure HW_CH423S_GP_LED_X and HW_GPIO_LED_X are defined in HAL_Config.h
};

/**
 * @brief Defines the number of elements in the `s_led_configurations` array.
 */
const size_t s_num_led_configurations = sizeof(s_led_configurations) / sizeof(s_led_configurations[0]);
