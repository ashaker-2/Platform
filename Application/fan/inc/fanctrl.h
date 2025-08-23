/* ============================================================================
 * SOURCE FILE: Application/fanCtrl/inc/fan_ctrl.h
 * ============================================================================*/
/**
 * @file fan_ctrl.h
 * @brief Public API for the Fan Control (FanCtrl) module.
 *
 * This header provides functions to initialize the fan control system and
 * manage the state (ON/OFF) of individual fans.
 */

#ifndef FAN_CTRL_H
#define FAN_CTRL_H

#include "common.h"  // For Status_t
#include "fanctrl_cfg.h"   // For Fan_ID_t, Fan_State_t

/**
 * @brief Initializes the Fan Control (FanCtrl) module.
 *
 * This function configures the necessary GPIO expander pins (via HAL_I2C)
 * or direct GPIO pins (via HAL_GPIO) according to the settings in fan_ctrl_cfg.c
 * and sets the initial state of all configured fans.
 *
 * @return E_OK on success, or an error code if initialization fails.
 */
Status_t FanCtrl_Init(void);

/**
 * @brief Sets the state (ON or OFF) of a specific fan.
 *
 * @param fan_id The unique identifier of the fan to control.
 * @param state The desired state for the fan (FAN_STATE_ON or FAN_STATE_OFF).
 * @return E_OK on success, E_INVALID_PARAM if `fan_id` is invalid, or
 * an error code from the underlying HAL if pin control fails.
 */
Status_t FanCtrl_SetState(Fan_ID_t fan_id, Fan_State_t state);

/**
 * @brief Gets the current state of a specific fan.
 *
 * This function relies on the internally tracked state of the expander outputs
 * or direct GPIO states.
 *
 * @param fan_id The unique identifier of the fan to query.
 * @param state_out Pointer to a variable where the current state (FAN_STATE_ON/OFF)
 * will be stored.
 * @return E_OK on success, E_INVALID_PARAM if `fan_id` or `state_out` is invalid.
 */
Status_t FanCtrl_GetState(Fan_ID_t fan_id, Fan_State_t *state_out);

#endif /* FAN_CTRL_H */
