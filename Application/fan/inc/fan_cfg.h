/**
 * @file fan_control_cfg.h
 * @brief Configuration header for the Fan Control (FAN_CTL) component.
 *
 * This file defines configurable parameters for the Fan Control module,
 * primarily related to the hardware interface (e.g., PWM channel assignments).
 */

#ifndef FAN_CONTROL_CFG_H
#define FAN_CONTROL_CFG_H

#include "HAL/inc/hal_pwm.h" // For HAL_PWM_Channel_t

// --- Fan Hardware Configuration ---
// Define the PWM channel used to control the fan speed.
#define FAN_CONTROL_CFG_PWM_CHANNEL     HAL_PWM_CHANNEL_0 /**< PWM channel for fan speed control. */

// Define the PWM frequency and resolution if needed, or assume default from HAL.
// #define FAN_CONTROL_CFG_PWM_FREQUENCY_HZ    25000 // Example: 25 kHz
// #define FAN_CONTROL_CFG_PWM_RESOLUTION_BITS 10    // Example: 10-bit resolution (0-1023 duty cycle)

// --- Safety Configuration (Optional) ---
// #define FAN_CONTROL_CFG_ENABLE_TACHOMETER_FEEDBACK (1) // Enable if fan provides tachometer feedback
// #define FAN_CONTROL_CFG_MIN_RPM_THRESHOLD          (500) // Minimum RPM to detect stalled fan
// #define FAN_CONTROL_CFG_MAX_CURRENT_A              (1.5f) // Max allowed current for fan

#endif // FAN_CONTROL_CFG_H
