#ifndef LIGHTIND_CFG_H
#define LIGHTIND_CFG_H

#include "app_common.h"
#include "system_monitor.h" // For fault IDs
#include "mcal_gpio.h"
#include <stdint.h>

/**
 * @file lightind_cfg.h
 * @brief Configuration header for the LightIndication component.
 *
 * This file defines all the necessary data types, macros, and external declarations
 * for configuring the LightInd module for a specific application.
 */

// --- Fault ID Definitions (from SystemMonitor) ---
#define FAULT_ID_LIGHT_IND_INIT_FAILED      FAULT_ID_LIGHT_CONTROL_FAILURE

// --- Light Indicator IDs ---
typedef enum {
    LIGHT_INDICATION_ID_STATUS = 0,
    LIGHT_INDICATION_ID_CRITICAL_ALARM,
    LIGHT_INDICATION_ID_WARNING,
    // Add more indicator IDs as needed
    LightInd_COUNT // Total number of configured indicators
} LightInd_Id_t;

// --- Light Indicator Configuration Structure ---
typedef struct {
    uint32_t id;       // Unique ID for this indicator instance
    uint32_t gpio_pin; // The GPIO pin controlling this indicator
} LightInd_Config_t;

// --- External Declaration of Indicator Configurations Array ---
extern const LightInd_Config_t light_ind_configs[LightInd_COUNT];

// --- Periodic Control Settings for LightInd_MainFunction() ---
#define LIGHTIND_CONTROL_PERIOD_MS         50 // LightInd_MainFunction called every 50 ms

#endif // LIGHTIND_CFG_H