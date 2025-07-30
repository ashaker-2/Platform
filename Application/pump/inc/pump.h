// app/inc/pump.h

#ifndef PUMP_H
#define PUMP_H

#include <stdint.h>
#include <stdbool.h>
#include "app_common.h"  // For APP_OK/APP_ERROR
#include "ecual_gpio.h"  // For ECUAL_GPIO_ID_t and ECUAL_GPIO_State_t

/**
 * @brief User-friendly IDs for configured Pump unit instances.
 */
typedef enum PUMP_ID_t {
    PUMP_WATER_CIRCULATION = 0, ///< Pump for water circulation (with feedback)
    PUMP_DRAINAGE,              ///< Pump for drainage (without feedback)
    PUMP_IRRIGATION,            ///< Pump for irrigation (with feedback)
    PUMP_TOTAL_UNITS            ///< Keep this last to get the count
} PUMP_ID_t;

/**
 * @brief Structure to hold the configuration for a single Pump unit.
 */
typedef struct PUMP_Config_t {
    ECUAL_GPIO_ID_t         control_gpio_id; ///< The ECUAL GPIO ID connected to the pump's control (e.g., relay).
    ECUAL_GPIO_State_t      active_state;    ///< The GPIO state (HIGH or LOW) that turns the pump ON.

    bool                    has_feedback_gpio;   ///< True if this pump has a feedback GPIO.
    ECUAL_GPIO_ID_t         feedback_gpio_id;    ///< The ECUAL GPIO ID for the feedback line. Only used if has_feedback_gpio is true.
    ECUAL_GPIO_State_t      feedback_active_state; ///< The GPIO state (HIGH or LOW) that indicates the pump is working.
} PUMP_Config_t;

/**
 * @brief Initializes all Pump units based on their configurations.
 * This sets up the underlying GPIOs to their initial (OFF) states.
 * @return APP_OK if all pumps are initialized successfully, APP_ERROR otherwise.
 */
uint8_t PUMP_Init(void);

/**
 * @brief Turns a specific Pump unit ON.
 * @param unit_id The PUMP_ID_t of the pump unit to control.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t PUMP_On(PUMP_ID_t unit_id);

/**
 * @brief Turns a specific Pump unit OFF.
 * @param unit_id The PUMP_ID_t of the pump unit to control.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t PUMP_Off(PUMP_ID_t unit_id);

/**
 * @brief Gets the current ON/OFF state of a specific Pump unit based on its control command.
 * @param unit_id The PUMP_ID_t of the pump unit.
 * @param is_on Pointer to a boolean variable to store the commanded state (true if commanded ON, false if OFF).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t PUMP_GetCommandedState(PUMP_ID_t unit_id, bool *is_on);

/**
 * @brief Gets the actual working state of a specific Pump unit based on its feedback line.
 * This function should only be called if the pump configuration specifies `has_feedback_gpio = true`.
 * If feedback is not configured, it will return APP_ERROR.
 * @param unit_id The PUMP_ID_t of the pump unit.
 * @param is_working Pointer to a boolean variable to store the feedback state (true if working, false if not).
 * @return APP_OK if feedback was read successfully, APP_ERROR otherwise (e.g., no feedback line configured).
 */
uint8_t PUMP_GetFeedbackState(PUMP_ID_t unit_id, bool *is_working);

#endif /* PUMP_H */
