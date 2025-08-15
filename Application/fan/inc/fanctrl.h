/**
 * @file fan_control.h
 * @brief Public header for the Fan Control (FAN_CTL) component.
 *
 * This component provides an interface for controlling the cooling fan
 * of the environmental monitoring system. It abstracts the underlying
 * hardware control (e.g., via PWM for speed control) and provides functions
 * for initialization and setting the fan speed.
 */

#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include "common.h" // For Status_t
#include <stdint.h> // For uint8_t

// --- Public Functions ---

/**
 * @brief Initializes the Fan Control module.
 * This function should be called once during system startup.
 * It configures the necessary hardware (e.g., PWM channel).
 * @return E_OK on success, E_NOK on failure.
 */
Status_t FAN_CONTROL_Init(void);

/**
 * @brief Sets the speed of the fan.
 * The speed is specified as a percentage (0-100).
 * @param speed_percent The desired fan speed in percentage (0 = off, 100 = max speed).
 * @return E_OK on success, E_NOK on failure (e.g., invalid speed, hardware error).
 */
Status_t FAN_CONTROL_SetSpeed(uint8_t speed_percent);

/**
 * @brief Gets the current commanded speed of the fan.
 * @param speed_percent_ptr Pointer to a uint8_t where the current speed will be stored.
 * @return E_OK on success, E_NOK on failure (e.g., NULL pointer).
 */
Status_t FAN_CONTROL_GetSpeed(uint8_t *speed_percent_ptr);

#endif // FAN_CONTROL_H


