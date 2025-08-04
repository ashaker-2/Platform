#ifndef LIGHTCTRL_CFG_H
#define LIGHTCTRL_CFG_H

#include "app_common.h"
#include "system_monitor.h" // For fault IDs
#include "mcal_gpio.h"
#include "mcal_pwm.h"
#include "mcal_adc.h"
#include <stdint.h>

/**
 * @file lightctrl_cfg.h
 * @brief Configuration header for the LightCtrl component.
 *
 * This file defines all the necessary data types, macros, and external declarations
 * for configuring the LightCtrl module for a specific application.
 */

// --- Fault ID Definitions (from SystemMonitor) ---
#define FAULT_ID_LIGHT_INIT_FAILED           FAULT_ID_LIGHT_CONTROL_FAILURE
#define FAULT_ID_LIGHT_CONTROL_FAILED        FAULT_ID_LIGHT_CONTROL_FAILURE
#define FAULT_ID_LIGHT_FEEDBACK_FAILURE      FAULT_ID_LIGHT_CONTROL_FAILURE
#define FAULT_ID_LIGHT_FEEDBACK_MISMATCH     FAULT_ID_LIGHT_CONTROL_FAILURE

// --- Light Type Enums ---
typedef enum {
    LightCtrl_TYPE_RELAY,
    LightCtrl_TYPE_PWM,
    LightCtrl_TYPE_COUNT
} LightCtrl_Type_t;

// --- Light Feedback Type Enums ---
typedef enum {
    LightCtrl_FEEDBACK_TYPE_NONE,
    LightCtrl_FEEDBACK_TYPE_CURRENT_SENSOR, // Analog voltage from current sensor
    LightCtrl_FEEDBACK_TYPE_LIGHT_SENSOR,   // Analog voltage from light sensor
    LightCtrl_FEEDBACK_TYPE_COUNT
} LightCtrl_FeedbackType_t;

// --- Light Control Details Union ---
typedef union {
    struct {
        uint8_t relay_gpio_pin;
    } relay;
    struct {
        uint8_t pwm_channel;
    } pwm;
} LightCtrl_ControlDetails_t;

// --- Light Feedback Details Union (Optional) ---
typedef union {
    struct {
        uint8_t adc_channel;
        float voltage_to_brightness_slope;
        float voltage_to_brightness_offset;
    } analog_adc;
} LightCtrl_FeedbackDetails_t;

// --- Light Configuration Structure ---
typedef struct {
    uint32_t id; // Unique ID for this light instance
    LightCtrl_Type_t type;
    LightCtrl_ControlDetails_t control_details;
    LightCtrl_FeedbackType_t feedback_type;
    LightCtrl_FeedbackDetails_t feedback_details; // Optional, only if feedback_type != NONE
} LightCtrl_Config_t;

// --- Light IDs ---
typedef enum {
    LIGHT_ID_STATUS = 0,
    LIGHT_ID_CABINET,
    LIGHT_ID_READING,
    // Add more light IDs as needed
    LightCtrl_COUNT // Total number of configured lights
} LightCtrl_Id_t;

// --- External Declaration of Light Configurations Array ---
extern const LightCtrl_Config_t light_configs[LightCtrl_COUNT];

// --- Periodic Control Settings for LightCtrl_MainFunction() ---
#define LightCtrl_CONTROL_PERIOD_MS             100 // LightCtrl_MainFunction called every 100 ms

// --- Thresholds for feedback mismatch detection ---
#define LightCtrl_MIN_OPERATIONAL_BRIGHTNESS    5 // Minimum brightness % to be considered 'ON'

#endif // LIGHTCTRL_CFG_H