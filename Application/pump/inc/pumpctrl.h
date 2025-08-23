/* ============================================================================
 * SOURCE FILE: Application/pumpCtrl/inc/pump_ctrl.h
 * ============================================================================*/
/**
 * @file pump_ctrl.h
 * @brief Public API for the Pump Control (PumpCtrl) module.
 *
 * This header provides functions to initialize the pump control system and
 * manage the state (ON/OFF) of individual pumps.
 */

#ifndef PUMP_CTRL_H
#define PUMP_CTRL_H

#include "common.h"  // For Status_t
#include "pumpctrl_cfg.h"   // For Pump_ID_t, Pump_State_t

/**
 * @brief Initializes the Pump Control (PumpCtrl) module.
 *
 * This function configures the necessary GPIO expander pins (via HAL_I2C)
 * or direct GPIO pins (via HAL_GPIO) according to the settings in pump_ctrl_cfg.c
 * and sets the initial state of all configured pumps.
 *
 * @return E_OK on success, or an error code if initialization fails.
 */
Status_t PumpCtrl_Init(void);

/**
 * @brief Sets the state (ON or OFF) of a specific pump.
 *
 * @param pump_id The unique identifier of the pump to control.
 * @param state The desired state for the pump (PUMP_STATE_ON or PUMP_STATE_OFF).
 * @return E_OK on success, E_INVALID_PARAM if `pump_id` is invalid, or
 * an error code from the underlying HAL if pin control fails.
 */
Status_t PumpCtrl_SetState(Pump_ID_t pump_id, Pump_State_t state);

/**
 * @brief Gets the current state of a specific pump.
 *
 * This function relies on the internally tracked state of the expander outputs
 * or direct GPIO states.
 *
 * @param pump_id The unique identifier of the pump to query.
 * @param state_out Pointer to a variable where the current state (PUMP_STATE_ON/OFF)
 * will be stored.
 * @return E_OK on success, E_INVALID_PARAM if `pump_id` or `state_out` is invalid.
 */
Status_t PumpCtrl_GetState(Pump_ID_t pump_id, Pump_State_t *state_out);

#endif /* PUMP_CTRL_H */
