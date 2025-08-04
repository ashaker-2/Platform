/**
 * @file hal_gpio.h
 * @brief Public header for the Hardware Abstraction Layer (HAL) GPIO component.
 *
 * This component provides a hardware-independent interface for configuring and
 * controlling General Purpose Input/Output (GPIO) pins. It abstracts the
 * microcontroller-specific register access and provides a clean API for
 * higher-level modules.
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "app_common.h" // For APP_Status_t and HAL_Status_t
#include <stdint.h>     // For uint8_t
#include <stdbool.h>    // For bool

// --- GPIO Pin Definitions (Logical IDs) ---
// These are logical IDs that map to physical pins in hal_gpio_cfg.c
typedef enum
{
    HAL_GPIO_PIN_LED_STATUS = 0,         /**< Logical ID for a system status LED. */
    HAL_GPIO_PIN_BUTTON_MODE,            /**< Logical ID for a mode selection button. */
    HAL_GPIO_PIN_HEATER_RELAY,           /**< Logical ID for the heater control relay. */
    HAL_GPIO_PIN_FAN_PWM,                /**< Logical ID for the fan PWM output. */
    HAL_GPIO_PIN_PUMP_RELAY,             /**< Logical ID for the pump control relay. */
    HAL_GPIO_PIN_VENTILATOR_RELAY,       /**< Logical ID for the ventilator control relay. */
    HAL_GPIO_PIN_LIGHT_RELAY,            /**< Logical ID for the main light control relay. */
    HAL_GPIO_PIN_TEMP_SENSOR_ADC_IN,     /**< Logical ID for the temperature sensor analog input. */
    HAL_GPIO_PIN_HUMIDITY_SENSOR_ADC_IN, /**< Logical ID for the humidity sensor analog input. */
    // Add more logical GPIO pins as needed
    HAL_GPIO_PIN_COUNT /**< Total number of logical GPIO pins. */
} HAL_GPIO_Pin_t;

// --- GPIO Direction ---
typedef enum
{
    HAL_GPIO_DIR_INPUT, /**< Configure pin as input. */
    HAL_GPIO_DIR_OUTPUT /**< Configure pin as output. */
} HAL_GPIO_Direction_t;

// --- GPIO State ---
typedef enum
{
    HAL_GPIO_STATE_LOW = 0, /**< Set output pin to low (0V). */
    HAL_GPIO_STATE_HIGH     /**< Set output pin to high (VCC). */
} HAL_GPIO_State_t;

// --- GPIO Pull Mode (for inputs) ---
typedef enum
{
    HAL_GPIO_PULL_NONE, /**< No pull-up or pull-down resistor. */
    HAL_GPIO_PULL_UP,   /**< Enable internal pull-up resistor. */
    HAL_GPIO_PULL_DOWN  /**< Enable internal pull-down resistor. */
} HAL_GPIO_PullMode_t;

// --- GPIO Interrupt Edge/Level ---
typedef enum
{
    HAL_GPIO_INT_NONE,         /**< No interrupt. */
    HAL_GPIO_INT_RISING_EDGE,  /**< Interrupt on rising edge. */
    HAL_GPIO_INT_FALLING_EDGE, /**< Interrupt on falling edge. */
    HAL_GPIO_INT_BOTH_EDGES,   /**< Interrupt on both rising and falling edges. */
    HAL_GPIO_INT_HIGH_LEVEL,   /**< Interrupt on high level (if supported by MCAL). */
    HAL_GPIO_INT_LOW_LEVEL     /**< Interrupt on low level (if supported by MCAL). */
} HAL_GPIO_InterruptType_t;

// --- Function Pointer for GPIO Interrupt Callback ---
typedef void (*HAL_GPIO_InterruptCallback_t)(HAL_GPIO_Pin_t pin);

// --- Public Functions ---

/**
 * @brief Initializes the HAL GPIO module.
 * This function should be called once during system startup.
 * It prepares the internal data structures and potentially the underlying MCAL.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_GPIO_Init(void);

/**
 * @brief Configures a specific GPIO pin.
 * This sets the direction, initial state (for outputs), and pull mode (for inputs).
 * @param pin The logical ID of the GPIO pin to configure.
 * @param direction The desired direction (input or output).
 * @param initial_state The initial state for output pins (HAL_GPIO_STATE_LOW/HIGH). Ignored for inputs.
 * @param pull_mode The pull mode for input pins (HAL_GPIO_PULL_NONE/UP/DOWN). Ignored for outputs.
 * @return HAL_OK on success, HAL_ERROR on failure (e.g., invalid pin, MCAL error).
 */
HAL_Status_t HAL_GPIO_Config(HAL_GPIO_Pin_t pin, HAL_GPIO_Direction_t direction,
                             HAL_GPIO_State_t initial_state, HAL_GPIO_PullMode_t pull_mode);

/**
 * @brief Sets the state of an output GPIO pin.
 * @param pin The logical ID of the output GPIO pin.
 * @param state The desired state (HAL_GPIO_STATE_LOW or HAL_GPIO_STATE_HIGH).
 * @return HAL_OK on success, HAL_ERROR on failure (e.g., pin not configured as output, MCAL error).
 */
HAL_Status_t HAL_GPIO_Write(HAL_GPIO_Pin_t pin, HAL_GPIO_State_t state);

/**
 * @brief Toggles the state of an output GPIO pin.
 * @param pin The logical ID of the output GPIO pin.
 * @return HAL_OK on success, HAL_ERROR on failure (e.g., pin not configured as output, MCAL error).
 */
HAL_Status_t HAL_GPIO_Toggle(HAL_GPIO_Pin_t pin);

/**
 * @brief Reads the current state of a GPIO pin.
 * @param pin The logical ID of the GPIO pin to read.
 * @param state_p Pointer to store the read state (HAL_GPIO_STATE_LOW or HAL_GPIO_STATE_HIGH).
 * @return HAL_OK on success, HAL_ERROR on failure (e.g., invalid pin, MCAL error).
 */
HAL_Status_t HAL_GPIO_Read(HAL_GPIO_Pin_t pin, HAL_GPIO_State_t *state_p);

/**
 * @brief Configures an interrupt for a specific GPIO pin.
 * @param pin The logical ID of the GPIO pin.
 * @param interrupt_type The type of interrupt (edge or level).
 * @param callback The function to be called when the interrupt occurs.
 * @param debounce_ms Debounce time in milliseconds (0 for no debouncing).
 * @return HAL_OK on success, HAL_ERROR on failure (e.g., invalid pin, MCAL error).
 */
HAL_Status_t HAL_GPIO_ConfigInterrupt(HAL_GPIO_Pin_t pin, HAL_GPIO_InterruptType_t interrupt_type,
                                      HAL_GPIO_InterruptCallback_t callback, uint32_t debounce_ms);

/**
 * @brief Enables the interrupt for a specific GPIO pin.
 * @param pin The logical ID of the GPIO pin.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_GPIO_EnableInterrupt(HAL_GPIO_Pin_t pin);

/**
 * @brief Disables the interrupt for a specific GPIO pin.
 * @param pin The logical ID of the GPIO pin.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_GPIO_DisableInterrupt(HAL_GPIO_Pin_t pin);

#endif // HAL_GPIO_H
