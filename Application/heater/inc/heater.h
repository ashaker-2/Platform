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
    HEATER_ROOM = 0,         ///< Heater for the main room
    HEATER_WATER_TANK,       ///< Heater for the water tank
    HEATER_TOTAL_UNITS       ///< Keep this last to get the count
} HEATER_ID_t;

/**
 * @brief Structure to hold the configuration for a single Heater unit.
 */
typedef struct HEATER_Config_t {
    ECUAL_GPIO_ID_t         control_gpio_id; ///< The ECUAL GPIO ID connected to the heater's control (e.g., relay).
    ECUAL_GPIO_State_t      active_state;    ///< The GPIO state (HIGH or LOW) that turns the heater ON.
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
 * @brief Gets the current ON/OFF state of a specific Heater unit.
 * @param unit_id The HEATER_ID_t of the heater unit.
 * @param is_on Pointer to a boolean variable to store the state (true if ON, false if OFF).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t HEATER_GetState(HEATER_ID_t unit_id, bool *is_on);

#endif /* HEATER_H */
