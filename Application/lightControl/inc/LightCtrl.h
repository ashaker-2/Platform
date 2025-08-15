#ifndef LIGHTCTRL_H
#define LIGHTCTRL_H

#include "common.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file lightctrl.h
 * @brief Public interface for the LightCtrl (Light Control) component.
 *
 * This header defines the public API for the LightCtrl module, which provides a
 * high-level interface for controlling and monitoring lighting elements.
 */

// --- Light State/Brightness Definitions ---
typedef enum {
    LightCtrl_STATE_OFF = 0,
    LightCtrl_STATE_ON,
    LightCtrl_STATE_COUNT
} LightCtrl_State_t;

// For PWM lights, brightness is typically 0-100%
#define LightCtrl_MIN_BRIGHTNESS_PERCENT   0
#define LightCtrl_MAX_BRIGHTNESS_PERCENT   100

// --- Public Functions ---

/**
 * @brief Initializes the LightCtrl module and all configured light control hardware.
 * All module-internal variables and light states are initialized to a safe,
 * known state (e.g., OFF or 0% brightness).
 * @return E_OK on success, E_NOK on failure.
 */
Status_t LightCtrl_Init(void);

/**
 * @brief Commands a desired light state (ON/OFF) or brightness (for PWM lights).
 * This function only updates the internal commanded state. The actual hardware control
 * is performed periodically by LightCtrl_MainFunction.
 * @param actuatorId The unique ID of the light to control.
 * @param state The desired state (LightCtrl_STATE_ON or LightCtrl_STATE_OFF).
 * @param brightness_percent For PWM lights: desired brightness in percentage (0-100).
 * For relay lights: this parameter is ignored.
 * @return E_OK on successful command update, E_NOK if the actuatorId is invalid
 * or the state/brightness_percent is out of range.
 */
Status_t LightCtrl_SetState(uint32_t actuatorId, LightCtrl_State_t state, uint8_t brightness_percent);

/**
 * @brief Gets the last commanded state/brightness, or the actual measured state/brightness if feedback is implemented.
 * This is a non-blocking getter function.
 * @param actuatorId The unique ID of the light to retrieve data from.
 * @param state Pointer to store the current ON/OFF state.
 * @param brightness_percent Pointer to store the current brightness in percentage (0-100), relevant for PWM.
 * @return E_OK on successful retrieval, E_NOK if the actuatorId is invalid,
 * or any pointer is NULL.
 */
Status_t LightCtrl_GetState(uint32_t actuatorId, LightCtrl_State_t *state, uint8_t *brightness_percent);

// --- Internal Periodic Runnable Prototype (called by RTE) ---
/**
 * @brief Performs the periodic light control, applying the commanded state to hardware,
 * and optionally reading feedback. This function is intended to be called periodically by an RTE task.
 */
void LightCtrl_MainFunction(void);

#endif // LIGHTCTRL_H