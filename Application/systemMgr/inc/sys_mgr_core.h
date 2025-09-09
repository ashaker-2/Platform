/**
 * @file sys_mgr_core.h
 * @brief Core logic for System Manager (internal module)
 * @version 1.0
 * @date 2025
 *
 * This file declares the internal System Manager core functions that
 * handle actuator control, state updates, and threshold enforcement.
 * It is not meant to be used directly by UI or other modules — only
 * the top-level `sys_mgr.c` should expose public APIs.
 */

#ifndef SYS_MGR_CORE_H
#define SYS_MGR_CORE_H

#include "common.h"
#include "sys_mgr_cfg.h"
#include "temphumctrl.h"

/* ============================================================================
 * ENUMS AND STRUCTURES
 * ========================================================================== */

/**
 * @brief Actuator run mode
 */
typedef enum {
    ACTUATOR_MODE_AUTOMATIC = 0,   /**< Controlled by SysMgr logic */
    ACTUATOR_MODE_MANUAL,          /**< Runs based on ON/OFF timers */
    ACTUATOR_MODE_DISABLED         /**< Permanently off (ignore control) */
} Actuator_Mode_t;

/**
 * @brief Actuator runtime state
 */
typedef struct {
    bool is_on;               /**< Current state (true = ON, false = OFF) */
    uint32_t last_toggle_ms;  /**< Last toggle timestamp (manual mode use) */
} Actuator_Runtime_t;

/**
 * @brief Snapshot of all actuator runtime states
 */
typedef struct {
    Actuator_Runtime_t fans[FAN_ID_COUNT];
    Actuator_Runtime_t vents[VEN_ID_COUNT];
    Actuator_Runtime_t pumps[PUMP_ID_COUNT];
    Actuator_Runtime_t heaters[HEATER_ID_COUNT];
    Actuator_Runtime_t leds[LED_ID_COUNT];
    Actuator_Runtime_t lights[LIGHT_ID_COUNT];
} SysMgr_ActuatorStates_t;

/* ============================================================================
 * PUBLIC FUNCTIONS (called by sys_mgr.c)
 * ========================================================================== */

/**
 * @brief Initialize the System Manager core runtime structures.
 *
 * This sets all actuators OFF and resets internal timers.
 *
 * @param cfg Pointer to the current configuration (validated).
 */
void SYS_MGR_CORE_Init(const SysMgr_Config_t *cfg);

/**
 * @brief Periodic update function (called from SysMgr_MainFunction).
 *
 * This performs:
 *  - Read sensor values and statuses
 *  - Apply thresholds
 *  - Control actuators based on mode (AUTO / MANUAL / DISABLED)
 *  - Update runtime states
 *
 * @param cfg Pointer to the current configuration (validated).
 */
void SYS_MGR_CORE_MainFunction(const SysMgr_Config_t *cfg);

/**
 * @brief Retrieve current actuator runtime states.
 *
 * Useful for UI display or diagnostics.
 *
 * @param[out] states_out Pointer to structure to fill.
 * @return E_OK on success, E_NOK if pointer is NULL.
 */
Status_t SYS_MGR_CORE_GetActuatorStates(SysMgr_ActuatorStates_t *states_out);

/**
 * @brief Force all actuators OFF (e.g., for shutdown or failsafe).
 */
void SYS_MGR_CORE_AllOff(void);

#endif /* SYS_MGR_CORE_H */
