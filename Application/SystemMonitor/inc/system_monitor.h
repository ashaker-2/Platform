#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include "app_common.h"
#include "system_monitor_cfg.h" // For SysMon_MAX_ACTIVE_FAULTS
#include <stdint.h>
#include <stdbool.h>

/**
 * @file system_monitor.h
 * @brief Public interface for the SystemMonitor (Fault Manager) component.
 *
 * This header defines the public API for the SystemMonitor module, which is
 * responsible for managing fault reports and monitoring system health.
 */

// --- Fault Severity Definitions ---

/**
 * @brief Severity levels for reported faults.
 */
typedef enum {
    SEVERITY_NONE = 0,
    SEVERITY_LOW,      // Minor issue, informational
    SEVERITY_MEDIUM,   // Non-critical, but requires attention
    SEVERITY_HIGH,     // Critical, requires immediate action, might trigger fail-safe
    SEVERITY_CRITICAL  // System-impacting, likely requires reboot or immediate shutdown
} SystemMonitor_FaultSeverity_t;

/**
 * @brief Structure for a single fault record.
 */
typedef struct {
    uint32_t                    id;
    SystemMonitor_FaultSeverity_t severity;
    uint32_t                    timestamp_ms;
    uint32_t                    data;         // Optional fault-specific data
    bool                        is_active;
} SystemMonitor_FaultRecord_t;

/**
 * @brief Structure for overall fault status.
 */
typedef struct {
    uint32_t                      active_fault_count;
    uint32_t                      historical_fault_count;
    SystemMonitor_FaultRecord_t   active_faults[SysMon_MAX_ACTIVE_FAULTS];
} SystemMonitor_FaultStatus_t;


// --- Public Functions ---

/**
 * @brief Initializes the SystemMonitor module.
 *
 * Clears fault logs, resets system health metrics, and creates the internal mutex.
 *
 * @return APP_OK on success, APP_ERROR on failure (e.g., mutex creation failed).
 */
APP_Status_t SysMon_Init(void);

/**
 * @brief Reports a fault to the SystemMonitor.
 *
 * This function is called by other modules when they detect an error. It is
 * a thread-safe, non-blocking function.
 *
 * @param fault_id The ID of the fault being reported (from the configured list).
 * @param severity The severity level of the fault.
 * @param data Optional data related to the fault (e.g., sensor ID, error code).
 * @return APP_OK on success, APP_ERROR if the fault could not be recorded.
 */
APP_Status_t SysMon_ReportFault(uint32_t fault_id,
                                SystemMonitor_FaultSeverity_t severity,
                                uint32_t data);

/**
 * @brief Performs periodic system health monitoring and fault evaluation.
 *
 * This function is intended to be called periodically by the SYS_MON_Task.
 * It checks for CPU load, stack usage, and evaluates active faults.
 */
void SysMon_MainFunction(void);

/**
 * @brief Retrieves the current CPU load percentage.
 * @return Current CPU load in percentage (0-100).
 */
uint8_t SysMon_GetCPULoad(void);

/**
 * @brief Retrieves the total minimum free stack (High Water Mark) across all tasks.
 * @return Total minimum free stack in bytes.
 */
uint32_t SysMon_GetTotalMinFreeStack(void);

/**
 * @brief Retrieves the current fault status (active faults).
 *
 * This function is primarily for the Diagnostic module.
 *
 * @param status Pointer to a SystemMonitor_FaultStatus_t structure to fill.
 * @return APP_OK on success, APP_ERROR on failure (e.g., NULL pointer).
 */
APP_Status_t SysMon_GetFaultStatus(SystemMonitor_FaultStatus_t *status);

#endif // SYSTEM_MONITOR_H