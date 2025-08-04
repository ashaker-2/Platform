#ifndef HEATERCTRL_H
#define HEATERCTRL_H

#include "app_common.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file heaterctrl.h
 * @brief Public interface for the HeaterControl (HeaterCtrl) component.
 *
 * This header defines the public API for the HeaterCtrl module, which provides a
 * high-level interface for controlling a heating element via a relay.
 */

// --- Heater State Definitions ---
typedef enum {
    HEATERCTRL_STATE_OFF = 0,
    HEATERCTRL_STATE_ON
} HeaterCtrl_State_t;

// --- Public Functions ---

/**
 * @brief Initializes the HeaterCtrl module and the configured heater hardware.
 * The heater is set to a safe, known state (OFF).
 * @return APP_OK on success, APP_ERROR on failure.
 */
APP_Status_t HeaterCtrl_Init(void);

/**
 * @brief Commands a desired heater state.
 * This function directly applies the commanded state to the hardware.
 * @param state The desired state (HEATERCTRL_STATE_ON or HEATERCTRL_STATE_OFF).
 * @return APP_OK on success, APP_ERROR on failure (e.g., hardware driver error).
 */
APP_Status_t HeaterCtrl_SetState(HeaterCtrl_State_t state);

/**
 * @brief Gets the last commanded state of the heater.
 * This is a non-blocking getter function.
 * @param state Pointer to store the current state.
 * @return APP_OK on success, APP_ERROR on failure (e.g., NULL pointer).
 */
APP_Status_t HeaterCtrl_GetState(HeaterCtrl_State_t *state);

#endif // HEATERCTRL_H