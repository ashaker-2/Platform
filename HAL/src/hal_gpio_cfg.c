/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/src/HAL_GPIO_Cfg.c
 * ============================================================================*/
/**
 * @file HAL_GPIO_Cfg.c
 * @brief Implements the static array of GPIO configuration settings.
 * This file defines the specific initial modes, pull-ups/downs, and interrupt types
 * for all direct ESP32 GPIOs used by the hardware. It does not contain any
 * initialization functions; its purpose is purely to hold configuration data.
 */

#include "HAL_GPIO_Cfg.h"   // Header for GPIO configuration types and extern declarations
#include "HAL_Config.h"     // Global hardware pin definitions (e.g., HW_DISPLAY_RS_GPIO)
#include <stddef.h>         // For size_t

/**
 * @brief Array containing all predefined GPIO configurations.
 * Each element in this array configures a specific group of GPIO pins
 * according to their role (e.g., display outputs, keypad inputs).
 * This array dictates the initial state and mode of each GPIO.
 * This array is made `const` and global (`extern` in header) to be accessed by `HAL_GPIO.c`.
 */
const gpio_config_item_t s_gpio_configurations[] = {
    // Configuration for Display GPIOs (Outputs)
    {
        .pin_bit_mask = (1ULL << HW_DISPLAY_RS_GPIO) | (1ULL << HW_DISPLAY_E_GPIO) |
                        (1ULL << HW_DISPLAY_DB4_GPIO) | (1ULL << HW_DISPLAY_DB5_GPIO) |
                        (1ULL << HW_DISPLAY_DB6_GPIO) | (1ULL << HW_DISPLAY_DB7_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    },
    // Configuration for Keypad Row GPIOs (Outputs)
    {
        .pin_bit_mask = (1ULL << HW_KEYPAD_ROW1_GPIO) | (1ULL << HW_KEYPAD_ROW2_GPIO) |
                        (1ULL << HW_KEYPAD_ROW3_GPIO) | (1ULL << HW_KEYPAD_ROW4_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE, // Specific pull behavior handled by application scanning
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    },
    // Configuration for Keypad Column GPIOs (Inputs with Pull-up)
    {
        .pin_bit_mask = (1ULL << HW_KEYPAD_COL1_GPIO) | (1ULL << HW_KEYPAD_COL2_GPIO) |
                        (1ULL << HW_KEYPAD_COL3_GPIO) | (1ULL << HW_KEYPAD_COL4_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, // Internal pull-up to detect open switch
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE, // Or GPIO_INTR_ANYEDGE for interrupt-driven scanning
    },
    // Configuration for Digital Temp/Humidity Sensor GPIOs (Open-Drain, Pull-up for 1-wire)
    {
        .pin_bit_mask = (1ULL << HW_TEMP_HUM_0_GPIO) | (1ULL << HW_TEMP_HUM_1_GPIO) |
                        (1ULL << HW_TEMP_HUM_2_GPIO) | (1ULL << HW_TEMP_HUM_3_GPIO),
        .mode = GPIO_MODE_INPUT_OUTPUT_OD, // Open-drain output for 1-wire, default input
        .pull_up_en = GPIO_PULLUP_ENABLE,  // Essential for 1-wire protocol
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    },
    // Configuration for Alarm GPIO (Output)
    {
        .pin_bit_mask = (1ULL << HW_ALARM_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    },
};

/**
 * @brief Defines the number of elements in the `s_gpio_configurations` array.
 * This variable is made `const` and global (`extern` in header) to be accessed by `HAL_GPIO.c`.
 */
const size_t s_num_gpio_configurations = sizeof(s_gpio_configurations) / sizeof(s_gpio_configurations[0]);
