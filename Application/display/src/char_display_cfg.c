/* ============================================================================
 * SOURCE FILE: char_display_cfg.c
 * ============================================================================*/
/**
 * @file char_display_cfg.c
 * @brief Implements the character display configuration.
 *
 * This file defines the specific GPIO pin assignments for the character LCD
 * display, which are used by the char_display driver.
 */

#include "char_display_cfg.h" // Include the configuration header

/**
 * @brief Global constant holding the GPIO pin assignments for the character display.
 *
 * These pins are configured for 4-bit mode operation.
 *
 * NOTE: GPIO14 is a strapping pin. Its state during boot affects the boot mode.
 * Ensure proper external pull-up/down if its state needs to be high/low at boot
 * for your application's specific boot requirements.
 */
const char_display_cfg_t s_char_display_config = {
    .rs_pin = GPIO_NUM_25,
    .e_pin = GPIO_NUM_26,
    .db4_pin = GPIO_NUM_27,
    .db5_pin = GPIO_NUM_32,
    .db6_pin = GPIO_NUM_33,
    .db7_pin = GPIO_NUM_14, // Strapping pin! See comments for details.
};
