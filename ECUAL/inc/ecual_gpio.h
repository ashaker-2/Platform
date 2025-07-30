// ecual/inc/ecual_gpio.h

#ifndef ECUAL_GPIO_H
#define ECUAL_GPIO_H

#include <stdint.h> // For uint8_t, uint32_t
#include "esp_err.h"

// IMPORTANT: Directly include the MCAL driver header to align enum values.
// This ties the ECUAL enum values directly to the specific MCU's driver,
// simplifying ecual_gpio.c at the cost of less strict ECUAL enum portability.
#include "driver/gpio.h" // Provides GPIO_MODE_INPUT, GPIO_MODE_OUTPUT, GPIO_PULLUP_ONLY, etc.
// No need for esp_err.h here directly, as ECUAL_OK/ECUAL_ERROR are now via ecual_common.h


/**
 * @brief Common ECUAL status codes.
 * ECUAL_OK is directly aliased to ESP_OK (which is 0).
 * ECUAL_ERROR is a generic non-zero error code.
 */
#define ECUAL_OK    ESP_OK    ///< Operation successful (equivalent to 0)
#define ECUAL_ERROR 1         ///< Generic operation failed (non-zero)


/**
 * @brief Defines the GPIO pin direction.
 * Values are directly aligned with ESP-IDF's gpio_mode_t for simplification.
 */
typedef enum {
    ECUAL_GPIO_DIR_INPUT  = GPIO_MODE_INPUT,  ///< Pin configured as input
    ECUAL_GPIO_DIR_OUTPUT = GPIO_MODE_OUTPUT  ///< Pin configured as output
    // Note: ESP-IDF has more modes (GPIO_MODE_INPUT_OUTPUT, GPIO_MODE_OUTPUT_OD),
    // but we only expose basic input/output here for simplicity.
} ECUAL_GPIO_Direction_t;

/**
 * @brief Defines the GPIO pin state (for output pins).
 * Values are directly aligned with typical 0/1 for low/high.
 */
typedef enum {
    ECUAL_GPIO_STATE_LOW  = 0, ///< Logic low (0V)
    ECUAL_GPIO_STATE_HIGH = 1, ///< Logic high (3.3V)
    ECUAL_GPIO_STATE_DEFAULT = 2 // Used for input pins where initial state doesn't apply
} ECUAL_GPIO_State_t;

/**
 * @brief Defines the GPIO pull-up/pull-down resistor configuration.
 * Values are directly aligned with ESP-IDF's gpio_pull_mode_t for simplification.
 */
typedef enum {
    ECUAL_GPIO_PULL_NONE = GPIO_FLOATING,       ///< No pull-up or pull-down resistor
    ECUAL_GPIO_PULL_UP   = GPIO_PULLUP_ONLY,    ///< Enable internal pull-up resistor
    ECUAL_GPIO_PULL_DOWN = GPIO_PULLDOWN_ONLY   ///< Enable internal pull-down resistor
} ECUAL_GPIO_Pull_t;

/**
 * @brief Structure to hold the configuration for a single GPIO pin.
 */
typedef struct {
    uint8_t                gpio_num;       ///< GPIO pin number
    ECUAL_GPIO_Direction_t direction;      ///< Pin direction (input/output)
    ECUAL_GPIO_Pull_t      pull;           ///< Pull-up/pull-down configuration
    ECUAL_GPIO_State_t     initial_state;  ///< Initial state for output pins (LOW/HIGH)
} ECUAL_GPIO_Config_t;

/**
 * @brief Initializes all GPIO pins based on the configurations defined in ecual_gpio_config.h.
 * @return ECUAL_OK if all pins are initialized successfully, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_GPIO_Init(void);

/**
 * @brief Sets the direction of a specified GPIO pin.
 * @param gpio_num The GPIO pin number.
 * @param direction The desired direction (ECUAL_GPIO_DIR_INPUT or ECUAL_GPIO_DIR_OUTPUT).
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_GPIO_SetDirection(uint8_t gpio_num, ECUAL_GPIO_Direction_t direction);

/**
 * @brief Sets the state (HIGH/LOW) of a specified GPIO output pin.
 * @param gpio_num The GPIO pin number.
 * @param state The desired state (ECUAL_GPIO_STATE_HIGH or ECUAL_GPIO_STATE_LOW).
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_GPIO_SetState(uint8_t gpio_num, ECUAL_GPIO_State_t state);

/**
 * @brief Reads the current state of a specified GPIO input pin.
 * @param gpio_num The GPIO pin number.
 * @return The current state of the pin (ECUAL_GPIO_STATE_HIGH or ECUAL_GPIO_STATE_LOW).
 */
ECUAL_GPIO_State_t ECUAL_GPIO_GetState(uint8_t gpio_num);

/**
 * @brief Sets the pull-up/pull-down configuration for a specified GPIO pin.
 * @param gpio_num The GPIO pin number.
 * @param pull The desired pull configuration (ECUAL_GPIO_PULL_UP, ECUAL_GPIO_PULL_DOWN, ECUAL_GPIO_PULL_NONE).
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_GPIO_SetPull(uint8_t gpio_num, ECUAL_GPIO_Pull_t pull);

#endif /* ECUAL_GPIO_H */
