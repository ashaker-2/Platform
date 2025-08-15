#ifndef PUMPCTRL_H
#define PUMPCTRL_H

#include "common.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file pumpctrl.h
 * @brief Public interface for the PumpCtrl (Pump Control) component.
 *
 * This header defines the public API for the PumpCtrl module, which provides a
 * high-level interface for controlling and monitoring water pumps within the system.
 */

// --- Pump State Definitions ---
typedef enum {
    PumpCtrl_STATE_OFF = 0,
    PumpCtrl_STATE_ON,
    PumpCtrl_STATE_COUNT
} PumpCtrl_State_t;

// --- Public Functions ---

/**
 * @brief Initializes the PumpCtrl module and all configured pump control hardware.
 * All module-internal variables and pump states are initialized to a safe,
 * known state (e.g., OFF).
 * @return E_OK on success, E_NOK on failure.
 */
Status_t PumpCtrl_Init(void);

/**
 * @brief Commands a desired pump state (ON/OFF).
 * This function only updates the internal commanded state. The actual hardware control
 * is performed periodically by PumpCtrl_MainFunction.
 * @param actuatorId The unique ID of the pump to control.
 * @param state The desired state (PumpCtrl_STATE_ON or PumpCtrl_STATE_OFF).
 * @return E_OK on successful command update, E_NOK if the actuatorId is invalid
 * or the state is invalid.
 */
Status_t PumpCtrl_SetState(uint32_t actuatorId, PumpCtrl_State_t state);

/**
 * @brief Gets the last commanded state, or the actual measured state if feedback is implemented.
 * This is a non-blocking getter function.
 * @param actuatorId The unique ID of the pump to retrieve data from.
 * @param state Pointer to store the current ON/OFF state.
 * @return E_OK on successful retrieval, E_NOK if the actuatorId is invalid,
 * or the pointer is NULL.
 */
Status_t PumpCtrl_GetState(uint32_t actuatorId, PumpCtrl_State_t *state);

// --- Internal Periodic Runnable Prototype (called by RTE) ---
/**
 * @brief Performs the periodic pump control, applying the commanded state to hardware,
 * and optionally reading feedback. This function is intended to be called periodically by an RTE task.
 */
void PumpCtrl_MainFunction(void);

#endif // PUMPCTRL_H