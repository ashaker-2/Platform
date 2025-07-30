// app/inc/ventilator.h

#ifndef VENTILATOR_H
#define VENTILATOR_H

#include <stdint.h>
#include <stdbool.h>
#include "app_common.h"     // For APP_OK/APP_ERROR
#include "ecual_gpio.h"     // For ECUAL_GPIO_ID_t

/**
 * @brief User-friendly IDs for configured Ventilator instances.
 * Add an entry here for each physical ventilator you configure.
 */
typedef enum VENTILATOR_ID_t {
    VENTILATOR_EXHAUST_FAN = 0, ///< Primary Exhaust Ventilator
    // VENTILATOR_SUPPLY_FAN,   // Uncomment if you have more ventilators
    VENTILATOR_TOTAL_UNITS      ///< Keep this last to get the count
} VENTILATOR_ID_t;

/**
 * @brief Structure to hold the configuration for a single Ventilator.
 */
typedef struct VENTILATOR_Config_t {
    ECUAL_GPIO_ID_t         control_gpio_id;       ///< The ECUAL GPIO ID controlling this ventilator.
    ECUAL_GPIO_State_t      active_state;          ///< The GPIO state (HIGH/LOW) that turns the ventilator ON.
} VENTILATOR_Config_t;

/**
 * @brief Initializes all Ventilator modules based on their configurations.
 * This sets up the underlying ECUAL GPIO channels.
 * @return APP_OK if all ventilators are initialized successfully, APP_ERROR otherwise.
 */
uint8_t VENTILATOR_Init(void);

/**
 * @brief Turns a specific ventilator ON.
 * @param unit_id The VENTILATOR_ID_t of the ventilator to turn ON.
 * @return APP_OK if successful, APP_ERROR otherwise (e.g., invalid unit_id).
 */
uint8_t VENTILATOR_On(VENTILATOR_ID_t unit_id);

/**
 * @brief Turns a specific ventilator OFF.
 * @param unit_id The VENTILATOR_ID_t of the ventilator to turn OFF.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t VENTILATOR_Off(VENTILATOR_ID_t unit_id);

/**
 * @brief Gets the current commanded state of a specific ventilator.
 * @param unit_id The VENTILATOR_ID_t of the ventilator.
 * @param is_on Pointer to a boolean variable to store the state (true if ON, false if OFF).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t VENTILATOR_GetState(VENTILATOR_ID_t unit_id, bool *is_on);

#endif /* VENTILATOR_H */
