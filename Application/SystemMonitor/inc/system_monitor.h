/*
 * SystemMonitor Component Implementation
 * Environmental Monitoring & Control System
 * 
 * This module acts as the central Fault Manager with direct monitoring capabilities
 * for temperature, humidity, CPU load, stack usage, and component feedback.
 */

/* ============================================================================
 * HEADER FILE: Application/SystemMonitor/inc/system_monitor.h
 * ============================================================================ */

#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include "common.h"
#include "system_monitor_cfg.h"
#include <stdint.h>
#include <stdbool.h>

/* System health metrics */
typedef struct 
{
    uint8_t  cpu_load_percent;
    uint32_t total_min_free_stack_bytes;
    uint32_t active_task_count;
    uint32_t system_uptime_ms;
} SystemMonitor_HealthMetrics_t;

typedef enum 
{
    FAULT_Paasive = 0x00,
    FAULT_Active = 0x01,
} SysMon_FaultStatus_t;



/* Fault status for external queries */
typedef struct {
    uint32_t                      total_fault_count;
    SystemMonitor_FaultRecord_t   *fault_records;  /* Pointer to config fault table */
    SystemMonitor_HealthMetrics_t health_metrics;
} SystemMonitor_FaultStatus_t;

/* --- Public Function Prototypes --- */

/**
 * @brief Initializes the SystemMonitor module
 * @return APP_OK on success, APP_ERROR on failure
 */
Status_t SystemMonitor_Init(void);

/**
 * @brief Reports a fault from external component to SystemMonitor
 * @param fault_id The ID of the fault being reported
 * @return APP_OK on success, APP_ERROR on failure
 */
Status_t SysMon_ReportFaultStatus(SystemMonitor_FaultId_t fault_id,SysMon_FaultStatus_t status);

/**
 * @brief Main periodic function for system monitoring
 * Called by SYS_MON_Task every SYSMON_MONITOR_PERIOD_MS
 */
void SysMon_MainFunction(void);

/**
 * @brief Gets current CPU load percentage
 * @return CPU load (0-100%)
 */
uint8_t sysmon_get_core_cpu_load(uint8_t core_id, bool *is_hyperperiod_complete);
uint8_t sysmon_get_system_cpu_load(bool *is_hyperperiod_complete);
uint8_t sysmon_get_core_cpu_load_avg(uint8_t core_id);
uint8_t sysmon_get_system_cpu_load_avg(void);
void sysmon_print_cpu_load_report(void);

/**
 * @brief Gets total minimum free stack across all tasks
 * @return Minimum free stack in bytes
 */
uint32_t SysMon_GetTotalMinFreeStack(void);

/**
 * @brief Gets comprehensive fault status for diagnostic module
 * @param status Pointer to status structure to fill
 * @return APP_OK on success, APP_ERROR on invalid pointer
 */
Status_t SysMon_GetFaultStatus(SystemMonitor_FaultId_t FaultId , SysMon_FaultStatus_t * FaultStatus);

/**
 * @brief Checks if system is in critical fault state
 * @return true if critical faults are active
 */
Status_t SysMon_ClearAllFaults(void);


#endif /* SYSTEM_MONITOR_H */

