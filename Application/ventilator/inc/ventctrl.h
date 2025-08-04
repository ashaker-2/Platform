#ifndef VENTCTRL_H
#define VENTCTRL_H

#include "app_common.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file ventctrl.h
 * @brief Public interface for the VentCtrl (Ventilator Control) component.
 *
 * This header defines the public API for the VentCtrl module, which provides a
 * high-level interface for controlling and monitoring ventilation systems.
 */

// --- Ventilator State/Speed Definitions ---
typedef enum {
    VentCtrl_STATE_OFF = 0,
    VentCtrl_STATE_ON,
} VentCtrl_State_t;

// For PWM ventilators, speed is typically 0-100%
#define VentCtrl_MIN_PWM_SPEED_PERCENT   0
#define VentCtrl_MAX_PWM_SPEED_PERCENT   100

// --- Public Functions ---

/**
 * @brief Initializes the VentCtrl module and all configured ventilator control hardware.
 * All module-internal variables and ventilator states are initialized to a safe,
 * known state (e.g., OFF or 0% speed).
 * @return APP_OK on success, APP_ERROR on failure.
 */
APP_Status_t VentCtrl_Init(void);

/**
 * @brief Commands a desired ventilator state (ON/OFF) or speed (for PWM ventilators).
 * This function only updates the internal commanded state. The actual hardware control
 * is performed periodically by VentCtrl_MainFunction.
 * @param actuatorId The unique ID of the ventilator to control.
 * @param state The desired state (VentCtrl_STATE_ON or VentCtrl_STATE_OFF).
 * @param speed_percent For PWM ventilators: desired speed in percentage (0-100).
 * For relay ventilators: this parameter is ignored.
 * @return APP_OK on successful command update, APP_ERROR if the actuatorId is invalid
 * or the state/speed_percent is out of range.
 */
APP_Status_t VentCtrl_SetState(uint32_t actuatorId, VentCtrl_State_t state, uint8_t speed_percent);

/**
 * @brief Gets the last commanded state/speed, or the actual measured state/speed if feedback is implemented.
 * This is a non-blocking getter function.
 * @param actuatorId The unique ID of the ventilator to retrieve data from.
 * @param state Pointer to store the current ON/OFF state.
 * @param speed_percent Pointer to store the current speed in percentage (0-100), relevant for PWM.
 * @return APP_OK on successful retrieval, APP_ERROR if the actuatorId is invalid,
 * or any pointer is NULL.
 */
APP_Status_t VentCtrl_GetState(uint32_t actuatorId, VentCtrl_State_t *state, uint8_t *speed_percent);

// --- Internal Periodic Runnable Prototype (called by RTE) ---
// This function is declared here so RTE can call it.
/**
 * @brief Performs the periodic ventilator control, applying the commanded state to hardware,
 * and optionally reading feedback. This function is intended to be called periodically by an RTE task.
 */
void VentCtrl_MainFunction(void);

#endif // VENTCTRL_H