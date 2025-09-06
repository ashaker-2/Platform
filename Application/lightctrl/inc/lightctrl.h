/* ============================================================================
 * SOURCE FILE: Application/lightCtrl/inc/light_ctrl.h
 * ============================================================================*/
/**
 * @file light_ctrl.h
 * @brief Public API for the Light Control (LightCtrl) module.
 *
 * This header provides functions to initialize the light control system and
 * manage the state (ON/OFF) of individual lights.
 */

#ifndef LIGHT_CTRL_H
#define LIGHT_CTRL_H

#include "common.h"        // For Status_t
#include "lightctrl_cfg.h" // For Light_ID_t, Light_State_t

/**
 * @brief Initializes the Light Control (LightCtrl) module.
 *
 * This function configures the necessary GPIO expander pins (via HAL_I2C)
 * or direct GPIO pins (via HAL_GPIO) according to the settings in light_ctrl_cfg.c
 * and sets the initial state of all configured lights.
 *
 * @return E_OK on success, or an error code if initialization fails.
 */
Status_t LightCtrl_Init(void);

/**
 * @brief Sets the state (ON or OFF) of a specific light.
 *
 * @param light_id The unique identifier of the light to control.
 * @param state The desired state for the light (LIGHT_STATE_ON or LIGHT_STATE_OFF).
 * @return E_OK on success, E_INVALID_PARAM if `light_id` is invalid, or
 * an error code from the underlying HAL if pin control fails.
 */
Status_t LightCtrl_SetState(Light_ID_t light_id, Light_State_t state);

/**
 * @brief Gets the current state of a specific light.
 *
 * This function relies on the internally tracked state of the expander outputs
 * or direct GPIO states.
 *
 * @param light_id The unique identifier of the light to query.
 * @param state_out Pointer to a variable where the current state (LIGHT_STATE_ON/OFF)
 * will be stored.
 * @return E_OK on success, E_INVALID_PARAM if `light_id` or `state_out` is invalid.
 */
Status_t LightCtrl_GetState(Light_ID_t light_id, Light_State_t *state_out);

#endif /* LIGHT_CTRL_H */
