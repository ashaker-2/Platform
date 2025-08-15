#ifndef SYS_MGR_H
#define SYS_MGR_H

#include "common.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "system_monitor.h"

/**
 * @file sys_mgr.h
 * @brief Public interface for the SystemMgr (System Manager) component.
 *
 * This header defines the public API for the SystemMgr module, which is the
 * central control logic and state manager for the entire system.
 */

// --- System/SW/App Modes ---
typedef enum
{
    SYS_MGR_MODE_AUTOMATIC = 0, // Fully automated control based on sensors
    SYS_MGR_MODE_MANUAL,        // Actuators controlled directly by external commands
    SYS_MGR_MODE_HYBRID,        // Mixed control (e.g., fans auto, lights manual)
    SYS_MGR_MODE_FAIL_SAFE,     // Emergency mode, overrides all other control
    SYS_MGR_MODE_COUNT
} SYS_MGR_Mode_t;

// --- Actuator State Structure ---
typedef struct
{
    bool heater_is_on;
    uint8_t fan_speed_percent;
    bool pump_is_on;
    bool ventilator_is_on;
    bool light_is_on;
} SYS_MGR_ActuatorState_t;

// --- Public Functions ---

/**
 * @brief Initializes the System Manager module.
 * Loads operational parameters from storage and sets initial system mode.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_Init(void);

/**
 * @brief Performs periodic sensor reading, control logic, and state management.
 * This is the main periodic runnable for the System Manager task.
 */
void SYS_MGR_MainFunction(void);

/**
 * @brief Sets the operational temperature range.
 * @param min_temp The minimum desired temperature in Celsius.
 * @param max_temp The maximum desired temperature in Celsius.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_SetOperationalTemperature(float min_temp, float max_temp);

/**
 * @brief Gets the current operational temperature range.
 * @param min_temp Pointer to store the minimum desired temperature.
 * @param max_temp Pointer to store the maximum desired temperature.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_GetOperationalTemperature(float *min_temp, float *max_temp);

/**
 * @brief Sets the system's operational mode (Automatic, Manual, Hybrid, Fail-Safe).
 * @param mode The desired system mode.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_SetMode(SYS_MGR_Mode_t mode);

/**
 * @brief Gets the current system's operational mode.
 * @param mode Pointer to store the current system mode.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_GetMode(SYS_MGR_Mode_t *mode);

/**
 * @brief Activates or deactivates the fail-safe mode.
 * This function is primarily called by SystemMonitor in response to critical faults.
 * @param enable True to activate fail-safe, false to deactivate.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_SetFailSafeMode(bool enable);

/**
 * @brief Gets the current states of all managed actuators.
 * @param state Pointer to a SYS_MGR_ActuatorState_t structure to fill.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_GetActuatorStates(SYS_MGR_ActuatorState_t *state);

/**
 * @brief Checks if a critical alarm (e.g., fire) is active.
 * @param active Pointer to a boolean to store the status.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_GetCriticalAlarmStatus(bool *active);

#endif // SYS_MGR_H