/* ============================================================================
 * SOURCE FILE: char_display_cfg.h
 * ============================================================================*/
/**
 * @file char_display_cfg.h
 * @brief Configuration definitions for the character display module.
 *
 * This file declares the structure and external configuration constant
 * for the character LCD display's GPIO pin assignments.
 */

#ifndef CHAR_DISPLAY_CFG_H
#define CHAR_DISPLAY_CFG_H

#include "driver/gpio.h" // Required for gpio_num_t type

/**
 * @brief Structure to hold the GPIO pin assignments for the character display.
 */
typedef struct {
    gpio_num_t rs_pin;  ///< Register Select pin
    gpio_num_t e_pin;   ///< Enable pin
    gpio_num_t db4_pin; ///< Data Bus 4 pin
    gpio_num_t db5_pin; ///< Data Bus 5 pin
    gpio_num_t db6_pin; ///< Data Bus 6 pin
    gpio_num_t db7_pin; ///< Data Bus 7 pin
} char_display_cfg_t;

/**
 * @brief External declaration of the character display configuration.
 * This constant is defined in char_display_cfg.c and holds the specific
 * GPIO pin assignments for your display.
 */
extern const char_display_cfg_t s_char_display_config;

#endif /* CHAR_DISPLAY_CFG_H */
