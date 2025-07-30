// ecual/inc/ecual_gpio_config.h

#ifndef ECUAL_GPIO_CONFIG_H
#define ECUAL_GPIO_CONFIG_H

#include <stdint.h>
#include "ecual_gpio.h" // Include ecual_gpio.h to use ECUAL_GPIO_Config_t

/**
 * @brief Define GPIO Pin Numbers for your application outputs.
 * These correspond to the physical GPIO pins on the ESP32.
 */
#define ECUAL_GPIO_OUT_PIN_1        (2)   // Example: GPIO2
#define ECUAL_GPIO_OUT_PIN_2        (4)   // Example: GPIO4
#define ECUAL_GPIO_OUT_PIN_3        (16)  // Example: GPIO16
#define ECUAL_GPIO_OUT_PIN_4        (17)  // Example: GPIO17
#define ECUAL_GPIO_OUT_PIN_5        (18)  // Example: GPIO18
#define ECUAL_GPIO_OUT_PIN_6        (19)  // Example: GPIO19
#define ECUAL_GPIO_OUT_PIN_7        (21)  // Example: GPIO21
#define ECUAL_GPIO_OUT_PIN_8        (22)  // Example: GPIO22
#define ECUAL_GPIO_OUT_PIN_9        (23)  // Example: GPIO23
#define ECUAL_GPIO_OUT_PIN_10       (25)  // Example: GPIO25 (adjust based on your board)

/**
 * @brief Example: Define a pin for an alarm output.
 */
#define ECUAL_GPIO_ALARM_PIN        (5)   // Example: GPIO5 for alarm output

/**
 * @brief Example: Define pins for a simple keypad (2x2 matrix as a start, extend as needed)
 * Row pins are outputs, Column pins are inputs with pull-ups.
 */
#define ECUAL_GPIO_KEYPAD_ROW_1     (12)
#define ECUAL_GPIO_KEYPAD_ROW_2     (13)
#define ECUAL_GPIO_KEYPAD_COL_1     (14)
#define ECUAL_GPIO_KEYPAD_COL_2     (27)

/**
 * @brief ECUAL_GPIO_MAX_PIN_NUM
 * Highest GPIO number used in your configuration for efficient internal arrays.
 * Set to 39 for ESP32, as it has GPIO0-39.
 */
#define ECUAL_GPIO_MAX_PIN_NUM      (39) // Max GPIO on ESP32 is 39


/**
 * @brief Array of all GPIO pin configurations for initial setup.
 * This array defines the initial direction, pull, and state for each application pin.
 */
static const ECUAL_GPIO_Config_t gpio_pin_configurations[] = {
    // 10 Output Pins
    {ECUAL_GPIO_OUT_PIN_1,  ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW},
    {ECUAL_GPIO_OUT_PIN_2,  ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW},
    {ECUAL_GPIO_OUT_PIN_3,  ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW},
    {ECUAL_GPIO_OUT_PIN_4,  ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW},
    {ECUAL_GPIO_OUT_PIN_5,  ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW},
    {ECUAL_GPIO_OUT_PIN_6,  ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW},
    {ECUAL_GPIO_OUT_PIN_7,  ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW},
    {ECUAL_GPIO_OUT_PIN_8,  ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW},
    {ECUAL_GPIO_OUT_PIN_9,  ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW},
    {ECUAL_GPIO_OUT_PIN_10, ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW},

    // Alarm Pin
    {ECUAL_GPIO_ALARM_PIN,  ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW},

    // Keypad Pins
    {ECUAL_GPIO_KEYPAD_ROW_1, ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW}, // Rows are outputs
    {ECUAL_GPIO_KEYPAD_ROW_2, ECUAL_GPIO_DIR_OUTPUT, ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_STATE_LOW},
    {ECUAL_GPIO_KEYPAD_COL_1, ECUAL_GPIO_DIR_INPUT,  ECUAL_GPIO_PULL_UP,   ECUAL_GPIO_STATE_DEFAULT}, // Columns are inputs with pull-ups
    {ECUAL_GPIO_KEYPAD_COL_2, ECUAL_GPIO_DIR_INPUT,  ECUAL_GPIO_PULL_UP,   ECUAL_GPIO_STATE_DEFAULT},

    // Add other pins here as you define them (e.g., Display, UART, ADC)
    // For Display/UART/ADC, you would typically init their *modules* separately,
    // but if any of their *control pins* are general GPIO, they'd go here.
};

// Define the number of configurations in the array
#define ECUAL_NUM_GPIO_CONFIGURATIONS (sizeof(gpio_pin_configurations) / sizeof(ECUAL_GPIO_Config_t))

#endif /* ECUAL_GPIO_CONFIG_H */
