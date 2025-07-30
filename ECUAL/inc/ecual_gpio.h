// ecual/inc/ecual_gpio.h

#ifndef ECUAL_GPIO_H
#define ECUAL_GPIO_H

#include <stdint.h> // For uint8_t
#include "ecual_gpio_config.h" // Include pin definitions

/**
 * @brief Possible states for a GPIO pin (High or Low).
 */
typedef enum {
    ECUAL_GPIO_STATE_LOW  = 0,
    ECUAL_GPIO_STATE_HIGH = 1
} ECUAL_GPIO_State_t;

/**
 * @brief Possible directions for a GPIO pin (Input or Output).
 */
typedef enum {
    ECUAL_GPIO_DIR_INPUT  = 0,
    ECUAL_GPIO_DIR_OUTPUT = 1
} ECUAL_GPIO_Direction_t;

/**
 * @brief Possible pull configurations for a GPIO pin.
 */
typedef enum {
    ECUAL_GPIO_PULL_NONE = 0,
    ECUAL_GPIO_PULL_UP   = 1,
    ECUAL_GPIO_PULL_DOWN = 2
} ECUAL_GPIO_Pull_t;

/**
 * @brief Initializes the GPIO peripheral.
 * This function performs any necessary global GPIO setup (e.g., enabling clocks).
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_GPIO_Init(void);

/**
 * @brief Configures the direction of a specified GPIO pin.
 * @param gpio_num The GPIO pin number (0-39 for ESP32).
 * @param direction The desired direction (ECUAL_GPIO_DIR_INPUT or ECUAL_GPIO_DIR_OUTPUT).
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_GPIO_SetDirection(uint8_t gpio_num, ECUAL_GPIO_Direction_t direction);

/**
 * @brief Sets the output state of a specified GPIO pin (only if configured as output).
 * @param gpio_num The GPIO pin number (0-39).
 * @param state The desired state (ECUAL_GPIO_STATE_LOW or ECUAL_GPIO_STATE_HIGH).
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_GPIO_SetState(uint8_t gpio_num, ECUAL_GPIO_State_t state);

/**
 * @brief Reads the input state of a specified GPIO pin (only if configured as input).
 * @param gpio_num The GPIO pin number (0-39).
 * @return The current state of the pin (ECUAL_GPIO_STATE_LOW or ECUAL_GPIO_STATE_HIGH).
 */
ECUAL_GPIO_State_t ECUAL_GPIO_GetState(uint8_t gpio_num);

/**
 * @brief Configures the pull-up/pull-down resistors for a specified GPIO pin.
 * @param gpio_num The GPIO pin number (0-39).
 * @param pull The desired pull configuration (ECUAL_GPIO_PULL_NONE, ECUAL_GPIO_PULL_UP, ECUAL_GPIO_PULL_DOWN).
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_GPIO_SetPull(uint8_t gpio_num, ECUAL_GPIO_Pull_t pull);

#endif /* ECUAL_GPIO_H */
