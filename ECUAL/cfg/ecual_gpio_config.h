// ecual/inc/ecual_gpio_config.h

#ifndef ECUAL_GPIO_CONFIG_H
#define ECUAL_GPIO_CONFIG_H

#include <stdint.h> // For uint8_t

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

#endif /* ECUAL_GPIO_CONFIG_H */
