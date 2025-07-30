// app/inc/heater.h

#ifndef HEATER_H
#define HEATER_H

#include <stdint.h>
#include <stdbool.h>
#include "app_common.h"  // For APP_OK/APP_ERROR
#include "ecual_gpio.h"  // For ECUAL_GPIO_ID_t and ECUAL_GPIO_State_t

/**
 * @brief User-friendly IDs for configured Heater unit instances.
 */
typedef enum HEATER_ID_t {
    HEATER_ROOM = 0,         ///< Heater for the main room (with feedback)
    HEATER_WATER_TANK,       ///< Heater for the water tank (without feedback)
    HEATER_GARAGE,           ///< Heater for the garage (with feedback) <--- NEW EXAMPLE
    HEATER_TOTAL_UNITS       ///< Keep this last to get the count
} HEATER_ID_t;

/**
 * @brief Structure to hold the configuration for a single Heater unit.
 */
typedef struct HEATER_Config_t {
    ECUAL_GPIO_ID_t         control_gpio_id; ///< The ECUAL GPIO ID connected to the heater's control (e.g., relay).
    ECUAL_GPIO_State_t      active_state;    ///< The GPIO state (HIGH or LOW) that turns the heater ON.

    bool                    has_feedback_gpio;  ///< True if this heater has a feedback GPIO.
    ECUAL_GPIO_ID_t         feedback_gpio_id;   ///< The ECUAL GPIO ID for the feedback line. Only used if has_feedback_gpio is true.
    ECUAL_GPIO_State_t      feedback_active_state; ///< The GPIO state (HIGH or LOW) that indicates the heater is working.
} HEATER_Config_t;

/**
 * @brief Initializes all Heater units based on their configurations.
 * This sets up the underlying GPIOs to their initial (OFF) states.
 * @return APP_OK if all heaters are initialized successfully, APP_ERROR otherwise.
 */
uint8_t HEATER_Init(void);

/**
 * @brief Turns a specific Heater unit ON.
 * @param unit_id The HEATER_ID_t of the heater unit to control.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t HEATER_On(HEATER_ID_t unit_id);

/**
 * @brief Turns a specific Heater unit OFF.
 * @param unit_id The HEATER_ID_t of the heater unit to control.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t HEATER_Off(HEATER_ID_t unit_id);

/**
 * @brief Gets the current ON/OFF state of a specific Heater unit based on its control command.
 * @param unit_id The HEATER_ID_t of the heater unit.
 * @param is_on Pointer to a boolean variable to store the commanded state (true if commanded ON, false if OFF).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t HEATER_GetCommandedState(HEATER_ID_t unit_id, bool *is_on);

/**
 * @brief Gets the actual working state of a specific Heater unit based on its feedback line.
 * This function should only be called if the heater configuration specifies `has_feedback_gpio = true`.
 * If feedback is not configured, it will return APP_ERROR.
 * @param unit_id The HEATER_ID_t of the heater unit.
 * @param is_working Pointer to a boolean variable to store the feedback state (true if working, false if not).
 * @return APP_OK if feedback was read successfully, APP_ERROR otherwise (e.g., no feedback line configured).
 */
uint8_t HEATER_GetFeedbackState(HEATER_ID_t unit_id, bool *is_working);


#endif /* HEATER_H */
