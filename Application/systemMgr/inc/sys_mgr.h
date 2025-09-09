/**
 * @file sys_mgr.h
 * @brief Public API for the System Manager
 * @version 2.2
 * @date 2025
 *
 * The System Manager provides:
 *  - Initialization and shutdown
 *  - Central runtime configuration (with persistence support)
 *  - Periodic main function for control
 *  - Access to actuator states
 *
 * Core control logic (sensor reading, hysteresis, mode handling)
 * is delegated to sys_mgr_core for modularity.
 */

#ifndef SYS_MGR_H
#define SYS_MGR_H

#include <stdbool.h>
#include <stdint.h>
#include "common.h"
#include "sys_mgr_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * TYPE DEFINITIONS
 * ========================================================================== */

/**
 * @brief Snapshot of all actuator states for UI and diagnostics.
 */
typedef struct
{
    bool fans_on;
    bool heaters_on;
    bool pumps_on;
    bool vents_on;
    bool lights_on;
} SysMgr_ActuatorStates_t;

/* ============================================================================
 * PUBLIC API
 * ========================================================================== */

/**
 * @brief Initialize the System Manager.
 *
 * - Loads configuration from flash (if available, valid).
 * - Falls back to defaults if invalid or missing.
 * - Initializes core control logic.
 *
 * @return E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_Init(void);

/**
 * @brief Main periodic function (non-blocking).
 *
 * Call this at a fixed rate (e.g. 1Hz or faster) from the main loop
 * or from an RTOS task. It updates sensors, evaluates control logic,
 * and applies actuator commands.
 */
void SYS_MGR_MainFunction(void);

/**
 * @brief Get a copy of the current configuration.
 *
 * Thread-safe via mutex protection.
 *
 * @param[out] out Pointer to config struct.
 * @return E_OK if valid, E_NOK if null.
 */
Status_t SYS_MGR_GetConfig(SysMgr_Config_t *out);

/**
 * @brief Update runtime configuration.
 *
 * Thread-safe, marks configuration as dirty for later save.
 *
 * @param[in] in Pointer to new config struct.
 * @return E_OK if applied, E_NOK if null.
 */
Status_t SYS_MGR_UpdateConfigRuntime(const SysMgr_Config_t *in);

/**
 * @brief Save configuration to flash immediately if dirty.
 *
 * Usually called internally by MainFunction, but can also
 * be called on-demand.
 *
 * @return E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_SaveConfigToFlash(void);

/**
 * @brief Get the current snapshot of actuator states.
 *
 * Useful for UI and monitoring modules.
 *
 * @param[out] states_out Populated state struct.
 * @return E_OK on success, E_NOK if null.
 */
Status_t SYS_MGR_GetActuatorStates(SysMgr_ActuatorStates_t *states_out);

/**
 * @brief Force all actuators OFF.
 *
 * Puts system into safe passive state. Typically used on shutdown
 * or critical fault conditions.
 */
void SYS_MGR_AllOff(void);

#ifdef __cplusplus
}
#endif

#endif /* SYS_MGR_H */
