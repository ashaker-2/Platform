#ifndef VENTCTRL_CFG_H
#define VENTCTRL_CFG_H

#include "app_common.h"
#include "system_monitor.h" // For fault IDs
#include "mcal_gpio.h"
#include "mcal_pwm.h"
#include "mcal_adc.h"
#include <stdint.h>

/**
 * @file ventctrl_cfg.h
 * @brief Configuration header for the VentCtrl component.
 *
 * This file defines all the necessary data types, macros, and external declarations
 * for configuring the VentCtrl module for a specific application.
 */

// --- Fault ID Definitions (from SystemMonitor) ---
#define FAULT_ID_VENTILATOR_INIT_FAILED         FAULT_ID_FAN_CONTROL_FAILURE
#define FAULT_ID_VENTILATOR_CONTROL_FAILED      FAULT_ID_FAN_CONTROL_FAILURE
#define FAULT_ID_VENTILATOR_FEEDBACK_FAILURE    FAULT_ID_FAN_CONTROL_FAILURE
#define FAULT_ID_VENTILATOR_FEEDBACK_MISMATCH   FAULT_ID_FAN_CONTROL_FAILURE

// --- Ventilator Type Enums ---
typedef enum {
    VentCtrl_TYPE_RELAY,
    VentCtrl_TYPE_PWM,
    // Add more types as needed
} VentCtrl_Type_t;

// --- Ventilator Feedback Type Enums ---
typedef enum {
    VentCtrl_FEEDBACK_TYPE_NONE,
    VentCtrl_FEEDBACK_TYPE_TACHOMETER, // Pulse counting
    VentCtrl_FEEDBACK_TYPE_ANALOG_ADC  // Analog voltage proportional to speed/current
    // Add more types as needed
} VentCtrl_FeedbackType_t;

// --- Ventilator Control Details Union ---
typedef union {
    struct {
        uint8_t relay_gpio_pin;
    } relay;
    struct {
        uint8_t pwm_channel;
    } pwm;
} VentCtrl_ControlDetails_t;

// --- Ventilator Feedback Details Union (Optional) ---
typedef union {
    struct {
        uint8_t tachometer_gpio_pin;
        // Add parameters for pulse-to-RPM conversion
    } tachometer;
    struct {
        uint8_t adc_channel;
        float voltage_to_speed_slope;
        float voltage_to_speed_offset;
    } analog_adc;
} VentCtrl_FeedbackDetails_t;

// --- Ventilator Configuration Structure ---
typedef struct {
    uint32_t id; // Unique ID for this ventilator instance
    VentCtrl_Type_t type;
    VentCtrl_ControlDetails_t control_details;
    VentCtrl_FeedbackType_t feedback_type;
    VentCtrl_FeedbackDetails_t feedback_details; // Optional, only if feedback_type != NONE
} VentCtrl_Config_t;

// --- Ventilator IDs ---
typedef enum {
    VENT_ID_MAIN_EXHAUST = 0,
    VENT_ID_INTAKE_FAN,
    // Add more ventilator IDs as needed
    VentCtrl_COUNT // Total number of configured ventilators
} VentCtrl_Id_t;

// --- External Declaration of Ventilator Configurations Array ---
extern const VentCtrl_Config_t vent_configs[VentCtrl_COUNT];

// --- Periodic Control Settings for VentCtrl_MainFunction() ---
#define VentCtrl_CONTROL_PERIOD_MS             100 // VentCtrl_MainFunction called every 100 ms

// --- Thresholds for feedback mismatch detection ---
#define VentCtrl_MIN_OPERATIONAL_SPEED         5 // Minimum speed % to be considered 'ON'

#endif // VENTCTRL_CFG_H