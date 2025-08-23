/* ============================================================================
 * SOURCE FILE: Application/ledCtrl/inc/led_ctrl.h
 * ============================================================================*/
/**
 * @file led_ctrl.h
 * @brief Public API for the Led Control (LedCtrl) module.
 *
 * This header provides functions to initialize the led control system and
 * manage the state (ON/OFF) of individual leds.
 */

#ifndef LED_CTRL_H
#define LED_CTRL_H

#include "common.h"  // For Status_t
#include "ledctrl_cfg.h"   // For Led_ID_t, Led_State_t

/**
 * @brief Initializes the Led Control (LedCtrl) module.
 *
 * This function configures the necessary GPIO expander pins (via HAL_I2C)
 * or direct GPIO pins (via HAL_GPIO) according to the settings in led_ctrl_cfg.c
 * and sets the initial state of all configured leds.
 *
 * @return E_OK on success, or an error code if initialization fails.
 */
Status_t LedCtrl_Init(void);

/**
 * @brief Sets the state (ON or OFF) of a specific led.
 *
 * @param led_id The unique identifier of the led to control.
 * @param state The desired state for the led (LED_STATE_ON or LED_STATE_OFF).
 * @return E_OK on success, E_INVALID_PARAM if `led_id` is invalid, or
 * an error code from the underlying HAL if pin control fails.
 */
Status_t LedCtrl_SetState(Led_ID_t led_id, Led_State_t state);

/**
 * @brief Gets the current state of a specific led.
 *
 * This function relies on the internally tracked state of the expander outputs
 * or direct GPIO states.
 *
 * @param led_id The unique identifier of the led to query.
 * @param state_out Pointer to a variable where the current state (LED_STATE_ON/OFF)
 * will be stored.
 * @return E_OK on success, E_INVALID_PARAM if `led_id` or `state_out` is invalid.
 */
Status_t LedCtrl_GetState(Led_ID_t led_id, Led_State_t *state_out);

#endif /* LED_CTRL_H */
