/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/inc/HAL_GPIO_Cfg.h
 * ============================================================================*/
/**
 * @file HAL_GPIO_Cfg.h
 * @brief Declarations for external GPIO configuration data.
 * This header makes the static GPIO configuration array available to other HAL modules,
 * particularly HAL_GPIO.c, for initialization. It does not declare any functions.
 */
#ifndef HAL_GPIO_CFG_H
#define HAL_GPIO_CFG_H

#include "driver/gpio.h"    // For gpio_config_t

// Structure to hold individual GPIO configuration.
// Defined here for external visibility, used in HAL_GPIO_Cfg.c.
typedef struct {
    uint64_t pin_bit_mask;
    gpio_mode_t mode;
    gpio_pulldown_t pull_down_en;
    gpio_pullup_t pull_up_en;
    gpio_int_type_t intr_type;
} gpio_config_item_t;

/**
 * @brief External declaration of the array containing all predefined GPIO configurations.
 * This array is defined in HAL_GPIO_Cfg.c and accessed by HAL_GPIO.c to perform
 * initial GPIO setup.
 */
extern const gpio_config_item_t s_gpio_configurations[];

/**
 * @brief External declaration of the number of elements in the GPIO configurations array.
 */
extern const size_t s_num_gpio_configurations;

#endif /* HAL_GPIO_CFG_H */
