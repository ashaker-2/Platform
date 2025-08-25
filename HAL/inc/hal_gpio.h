/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/inc/HAL_GPIO.h
 * ============================================================================*/
/**
 * @file HAL_GPIO.h
 * @brief Public API for interacting with ESP32 direct GPIO pins.
 * This header declares functions for initializing the GPIOs, setting and getting
 * GPIO levels, changing direction, and managing pull-up/down resistors.
 */
#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "driver/gpio.h" // For gpio_num_t, gpio_mode_t
#include "common.h"      // For Status_t

/**
 * @brief Initializes all ESP32 direct GPIO pins according to the configurations
 * defined in `HAL_GPIO_Cfg.c`. This is the main initialization function for the GPIO HAL.
 * @return E_OK if configuration is successful, otherwise an error code.
 */
Status_t HAL_GPIO_Init(void);

/**
 * @brief Sets the output level of a configured GPIO pin.
 * @param gpio_num The GPIO number to control (e.g., from hal_cfg.h).
 * @param level The desired output level (0 for low, 1 for high).
 * @return E_OK on success, or an error code.
 */
Status_t HAL_GPIO_SetLevel(gpio_num_t gpio_num, uint8_t level);

/**
 * @brief Reads the input level of a configured GPIO pin.
 * @param gpio_num The GPIO number to read.
 * @param level_out Pointer to store the read level (0 for low, 1 for high).
 * @return E_OK on success, or an error code.
 */
Status_t HAL_GPIO_GetLevel(gpio_num_t gpio_num, uint8_t *level_out);

/**
 * @brief Configures the direction of a specific GPIO pin at runtime.
 * @param gpio_num The GPIO number to configure.
 * @param mode The desired GPIO mode (e.g., GPIO_MODE_INPUT, GPIO_MODE_OUTPUT).
 * @return E_OK on success, or an error code.
 */
Status_t HAL_GPIO_SetDirection(gpio_num_t gpio_num, gpio_mode_t mode);

/**
 * @brief Enables or disables the internal pull-up resistor for a GPIO pin.
 * @param gpio_num The GPIO number.
 * @param enable True to enable pull-up, false to disable.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_GPIO_SetPullUp(gpio_num_t gpio_num, bool enable);

/**
 * @brief Enables or disables the internal pull-down resistor for a GPIO pin.
 * @param gpio_num The GPIO number.
 * @param enable True to enable pull-down, false to disable.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_GPIO_SetPullDown(gpio_num_t gpio_num, bool enable);

#endif /* HAL_GPIO_H */
