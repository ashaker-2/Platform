#ifndef LIGHTIND_H
#define LIGHTIND_H

#include "common.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file lightind.h
 * @brief Public interface for the LightIndication (LightInd) component.
 *
 * This header defines the public API for the LightInd module, which provides
 * a high-level interface for controlling various types of light indicators.
 */

// --- Light Indicator State Definitions ---
typedef enum
{
    LIGHTIND_STATE_OFF = 0,
    LIGHTIND_STATE_ON,
    LIGHTIND_STATE_BLINK,
    LIGHTIND_STATE_COUNT
} LightInd_State_t;

// --- Public Functions ---

/**
 * @brief Initializes the LightInd module and all configured indicator hardware.
 * All indicators are set to a safe, known state (e.g., OFF).
 * @return E_OK on success, E_NOK on failure.
 */
Status_t LightInd_Init(void);

/**
 * @brief Turns a specific light indicator ON.
 * @param indicatorId The unique ID of the indicator to control.
 * @return E_OK on success, E_NOK on failure (e.g., invalid ID).
 */
Status_t LightInd_On(uint32_t indicatorId);

/**
 * @brief Turns a specific light indicator OFF.
 * @param indicatorId The unique ID of the indicator to control.
 * @return E_OK on success, E_NOK on failure (e.g., invalid ID).
 */
Status_t LightInd_Off(uint32_t indicatorId);

/**
 * @brief Sets a specific light indicator to a blinking state.
 * @param indicatorId The unique ID of the indicator to control.
 * @param on_time_ms The duration of the 'ON' phase in milliseconds.
 * @param off_time_ms The duration of the 'OFF' phase in milliseconds.
 * @return E_OK on success, E_NOK on failure (e.g., invalid ID).
 */
Status_t LightInd_Blink(uint32_t indicatorId, uint32_t on_time_ms, uint32_t off_time_ms);

// --- Internal Periodic Runnable Prototype (called by RTE) ---
/**
 * @brief Performs the periodic light indication control, handling blinking logic.
 * This function is intended to be called periodically by an RTE task.
 */
void LightInd_MainFunction(void);

#endif // LIGHTIND_H