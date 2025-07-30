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
    ECUAL_GPIO_STATE_HIGH = 1,
    ECUAL_GPIO_STATE_DEFAULT = 2 // For inputs where initial state doesn't matter
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
 * @brief Structure to hold the configuration for a single GPIO pin.
 */
typedef struct {
    uint8_t             gpio_num;    ///< The GPIO pin number
    ECUAL_GPIO_Direction_t direction; ///< Direction (Input or Output)
    ECUAL_GPIO_Pull_t      pull;      ///< Pull resistor configuration
    ECUAL_GPIO_State_t     initial_state; ///< Initial state for output pins (LOW/HIGH), or ignored for inputs
} ECUAL_GPIO_Config_t;

// Function prototypes remain the same
uint8_t ECUAL_GPIO_Init(void);
uint8_t ECUAL_GPIO_SetDirection(uint8_t gpio_num, ECUAL_GPIO_Direction_t direction);
uint8_t ECUAL_GPIO_SetState(uint8_t gpio_num, ECUAL_GPIO_State_t state);
ECUAL_GPIO_State_t ECUAL_GPIO_GetState(uint8_t gpio_num);
uint8_t ECUAL_GPIO_SetPull(uint8_t gpio_num, ECUAL_GPIO_Pull_t pull);

#endif /* ECUAL_GPIO_H */
