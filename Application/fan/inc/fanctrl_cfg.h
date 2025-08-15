/**
 * @file fanctrl_cfg.h
 * @brief Configuration header for the Fan Control (FAN_CTL) component.
 *
 * This file defines configurable parameters for the Fan Control module,
 * primarily related to the hardware interface (e.g., PWM channel assignments).
 */

#ifndef FAN_CONTROL_CFG_H
#define FAN_CONTROL_CFG_H

#include <stdlib.h>
#include <stdint.h>
// #include "hal_pwm.h" // For HAL_PWM_Channel_t

// Enum definition for fan types
typedef enum {
    FAN_TYPE_PWM = 1,  // PWM Controlled Fan
    FAN_TYPE_ON_OFF = 2,  // ON/OFF Fan
    // Add more fan types as needed
} FAN_Type_t;

// Structure to hold fan configuration
typedef struct {
    FAN_Type_t fan_type;                  // Type of fan: PWM or ON/OFF
    uint32_t control_gpio_id;             // GPIO ID used to control the fan
    uint8_t has_aux_gpio_control;            // Flag for aux GPIO control (only for PWM)
    uint32_t on_off_gpio_active_state;    // Active state for ON/OFF fans (HIGH or LOW)
    uint32_t pwm_channel_id;              // PWM channel for PWM fans
    uint32_t min_speed_duty_percent;      // Minimum PWM duty for PWM fans
    uint32_t max_speed_duty_percent;      // Maximum PWM duty for PWM fans
} FAN_Config_t;

#endif // fanctrl_cfg
