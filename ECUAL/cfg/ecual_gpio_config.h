// ecual/inc/ecual_gpio_config.h

#ifndef ECUAL_GPIO_CONFIG_H
#define ECUAL_GPIO_CONFIG_H

#include <stdint.h>
#include "ecual_gpio.h" // Include ecual_gpio.h for ECUAL_GPIO_Config_t definition

/**
 * @brief Define GPIO Pin Numbers for your application outputs.
 * These correspond to the physical GPIO pins on the ESP32.
 */
#define ECUAL_GPIO_OUT_PIN_1        (2)
#define ECUAL_GPIO_OUT_PIN_2        (4)
#define ECUAL_GPIO_OUT_PIN_3        (16)
#define ECUAL_GPIO_OUT_PIN_4        (17)
#define ECUAL_GPIO_OUT_PIN_5        (18)
#define ECUAL_GPIO_OUT_PIN_6        (19)
#define ECUAL_GPIO_OUT_PIN_7        (21)
#define ECUAL_GPIO_OUT_PIN_8        (22)
#define ECUAL_GPIO_OUT_PIN_9        (23)
#define ECUAL_GPIO_OUT_PIN_10       (25)

#define ECUAL_GPIO_ALARM_PIN        (5)

#define ECUAL_GPIO_KEYPAD_ROW_1     (12)
#define ECUAL_GPIO_KEYPAD_ROW_2     (13)
#define ECUAL_GPIO_KEYPAD_COL_1     (14)
#define ECUAL_GPIO_KEYPAD_COL_2     (27)

/**
 * @brief ECUAL_GPIO_MAX_PIN_NUM
 * Maximum valid GPIO number for validation (if used in ECUAL_GPIO).
 */
#define ECUAL_GPIO_MAX_PIN_NUM      (39) // Max GPIO on ESP32 is 39

// Declare the external constant array defined in ecual_gpio_config.c
extern const ECUAL_GPIO_Config_t gpio_pin_configurations[];
// Declare the external constant for its size
extern const uint32_t ECUAL_NUM_GPIO_CONFIGURATIONS;

#endif /* ECUAL_GPIO_CONFIG_H */
