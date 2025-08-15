#ifndef HEATERCTRL_CFG_H
#define HEATERCTRL_CFG_H

#include "common.h"
#include "system_monitor.h" // For fault IDs
#include "hal_gpio.h"
#include <stdint.h>

/**
 * @file heaterctrl_cfg.h
 * @brief Configuration header for the HeaterControl component.
 *
 * This file defines all the necessary data types, macros, and external declarations
 * for configuring the HeaterCtrl module for a specific application.
 */

// --- Fault ID Definitions (from SystemMonitor) ---
#define FAULT_ID_HEATER_CONTROL_FAILED     (SystemMonitor_FaultId_t)250

// --- Heater IDs ---
typedef enum {
    HEATER_ID_MAIN = 0,
    // Add more heater IDs as needed
    HEATER_COUNT // Total number of configured heaters
} HeaterCtrl_Id_t;

// --- Heater Configuration Structure ---
typedef struct {
    uint32_t id;       // Unique ID for this heater instance
    uint32_t gpio_pin; // The GPIO pin controlling the heater relay
} HeaterCtrl_Config_t;

// --- External Declaration of Heater Configurations Array ---
extern const HeaterCtrl_Config_t heater_configs[HEATER_COUNT];

#endif // HEATERCTRL_CFG_H