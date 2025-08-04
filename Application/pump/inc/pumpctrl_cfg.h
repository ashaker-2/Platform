#ifndef PUMPCTRL_CFG_H
#define PUMPCTRL_CFG_H

#include "app_common.h"
#include "system_monitor.h" // For fault IDs
#include "mcal_gpio.h"
#include "mcal_adc.h"
#include <stdint.h>

/**
 * @file pumpctrl_cfg.h
 * @brief Configuration header for the PumpCtrl component.
 *
 * This file defines all the necessary data types, macros, and external declarations
 * for configuring the PumpCtrl module for a specific application.
 */

// --- Fault ID Definitions (from SystemMonitor) ---
#define FAULT_ID_PUMP_INIT_FAILED               FAULT_ID_PUMP_CONTROL_FAILURE
#define FAULT_ID_PUMP_CONTROL_FAILED            FAULT_ID_PUMP_CONTROL_FAILURE
#define FAULT_ID_PUMP_FEEDBACK_FAILURE          FAULT_ID_PUMP_CONTROL_FAILURE
#define FAULT_ID_PUMP_FEEDBACK_MISMATCH         FAULT_ID_PUMP_CONTROL_FAILURE

// --- Pump Type Enums ---
typedef enum {
    PumpCtrl_TYPE_RELAY,
    // Add more types as needed (e.g., PWM for variable speed pumps)
    PumpCtrl_TYPE_COUNT
} PumpCtrl_Type_t;

// --- Pump Feedback Type Enums ---
typedef enum {
    PumpCtrl_FEEDBACK_TYPE_NONE,
    PumpCtrl_FEEDBACK_TYPE_FLOW_SENSOR,    // Pulse or analog flow sensor
    PumpCtrl_FEEDBACK_TYPE_CURRENT_SENSOR, // Analog current sensor feedback
    PumpCtrl_FEEDBACK_TYPE_COUNT
} PumpCtrl_FeedbackType_t;

// --- Pump Control Details Structure ---
typedef struct {
    uint8_t relay_gpio_pin;
} PumpCtrl_ControlDetails_t;

// --- Pump Feedback Details Union (Optional) ---
typedef union {
    struct {
        uint8_t gpio_pin; // For pulse-based flow sensors
        float pulses_per_liter; // Conversion factor
        float flow_threshold_on; // Min flow to consider pump "ON"
    } flow_sensor_pulse;
    struct {
        uint8_t adc_channel; // For analog flow/current sensors
        float analog_to_flow_slope;
        float analog_to_flow_offset;
        float flow_threshold_on; // Min flow to consider pump "ON"
    } flow_sensor_analog;
    struct {
        uint8_t adc_channel;
        float current_threshold_on;  // Minimum current to consider pump "ON"
        float current_threshold_off; // Maximum current to consider pump "OFF"
    } current_sensor;
} PumpCtrl_FeedbackDetails_t;

// --- Pump Configuration Structure ---
typedef struct {
    uint32_t id; // Unique ID for this pump instance
    PumpCtrl_Type_t type;
    PumpCtrl_ControlDetails_t control_details;
    PumpCtrl_FeedbackType_t feedback_type;
    PumpCtrl_FeedbackDetails_t feedback_details; // Optional, only if feedback_type != NONE
} PumpCtrl_Config_t;

// --- Pump IDs ---
typedef enum {
    PUMP_ID_HUMIDITY = 0,
    PUMP_ID_DRAIN,
    // Add more pump IDs as needed
    PumpCtrl_COUNT // Total number of configured pumps
} PumpCtrl_Id_t;

// --- External Declaration of Pump Configurations Array ---
extern const PumpCtrl_Config_t pump_configs[PumpCtrl_COUNT];

// --- Periodic Control Settings for PumpCtrl_MainFunction() ---
#define PumpCtrl_CONTROL_PERIOD_MS             100 // PumpCtrl_MainFunction called every 100 ms

#endif // PUMPCTRL_CFG_H