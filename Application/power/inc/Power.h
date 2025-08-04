#ifndef POWER_H
#define POWER_H

#include "app_common.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file power.h
 * @brief Public interface for the PowerMgr (Power Management) component.
 *
 * This header defines the public API for the PowerMgr module, which provides
 * control over system power modes and monitors power consumption.
 */

// --- Power Mode Definitions ---
typedef enum {
    POWER_MODE_ON = 0,    // Full power, all peripherals active
    POWER_MODE_SLEEP,     // Low power, essential peripherals active, wake-up sources enabled
    POWER_MODE_OFF,       // Lowest power, effectively shut down, requires external wake-up/power cycle
    POWER_MODE_COUNT
} Power_Mode_t;

// --- Power Consumption Metrics ---
typedef struct {
    float current_mA;    // Current in milli-Amperes
    float voltage_mV;    // Voltage in milli-Volts
    float power_mW;      // Power in milli-Watts
} Power_Consumption_t;

// --- Public Functions ---

/**
 * @brief Initializes the Power module and related hardware (e.g., ADC for monitoring).
 * Sets the initial power mode to ON.
 * @return APP_OK on success, APP_ERROR on failure.
 */
APP_Status_t PowerMgr_Init(void);

/**
 * @brief Commands the system to transition to a specified power mode.
 * This function handles the necessary hardware configurations for the mode change.
 * @param mode The desired power mode (POWER_MODE_ON, POWER_MODE_SLEEP, POWER_MODE_OFF).
 * @return APP_OK on successful transition, APP_ERROR on failure or invalid mode.
 */
APP_Status_t PowerMgr_SetMode(Power_Mode_t mode);

/**
 * @brief Gets the last calculated power consumption metrics.
 * This is a non-blocking getter function.
 * @param consumption Pointer to a Power_Consumption_t structure to fill.
 * @return APP_OK on successful retrieval, APP_ERROR on failure (e.g., NULL pointer).
 */
APP_Status_t PowerMgr_GetConsumption(Power_Consumption_t *consumption);

/**
 * @brief Performs periodic power consumption monitoring and fault detection.
 * This function is intended to be called periodically by an RTE task.
 */
void PowerMgr_MainFunction(void);

#endif // POWER_H