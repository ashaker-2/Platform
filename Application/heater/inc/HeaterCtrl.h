/* ============================================================================
 * SOURCE FILE: Application/heaterCtrl/inc/heater_ctrl.h
 * ============================================================================*/
/**
 * @file heater_ctrl.h
 * @brief Public API for the Heater Control (HeaterCtrl) module.
 *
 * This header provides functions to initialize the heater control system and
 * manage the state (ON/OFF) of individual heaters.
 */

#ifndef HEATER_CTRL_H
#define HEATER_CTRL_H

#include "common.h"  // For Status_t
#include "heaterctrl_cfg.h"   // For Heater_ID_t, Heater_State_t

/**
 * @brief Initializes the Heater Control (HeaterCtrl) module.
 *
 * This function configures the necessary GPIO expander pins (via HAL_I2C)
 * or direct GPIO pins (via HAL_GPIO) according to the settings in heater_ctrl_cfg.c
 * and sets the initial state of all configured heaters.
 *
 * @return E_OK on success, or an error code if initialization fails.
 */
Status_t HeaterCtrl_Init(void);

/**
 * @brief Sets the state (ON or OFF) of a specific heater.
 *
 * @param heater_id The unique identifier of the heater to control.
 * @param state The desired state for the heater (HEATER_STATE_ON or HEATER_STATE_OFF).
 * @return E_OK on success, E_INVALID_PARAM if `heater_id` is invalid, or
 * an error code from the underlying HAL if pin control fails.
 */
Status_t HeaterCtrl_SetState(Heater_ID_t heater_id, Heater_State_t state);

/**
 * @brief Gets the current state of a specific heater.
 *
 * This function relies on the internally tracked state of the expander outputs
 * or direct GPIO states.
 *
 * @param heater_id The unique identifier of the heater to query.
 * @param state_out Pointer to a variable where the current state (HEATER_STATE_ON/OFF)
 * will be stored.
 * @return E_OK on success, E_INVALID_PARAM if `heater_id` or `state_out` is invalid.
 */
Status_t HeaterCtrl_GetState(Heater_ID_t heater_id, Heater_State_t *state_out);

#endif /* HEATER_CTRL_H */
