#ifndef SYSTEM_MONITOR_CFG_H
#define SYSTEM_MONITOR_CFG_H

#include <stdint.h>

/**
 * @file system_monitor_cfg.h
 * @brief Configuration header for the SystemMonitor component.
 *
 * This file defines all the necessary configuration macros, thresholds,
 * and fault IDs for the SystemMonitor module.
 */

// --- Fault ID Definitions ---
// The full list of fault IDs for the entire system
typedef enum {
    // General System Faults
    FAULT_ID_NONE = 0,
    FAULT_ID_SYS_INIT_ERROR,
    FAULT_ID_TASK_WATCHDOG_TIMEOUT,
    FAULT_ID_NVM_WRITE_FAILURE,
    FAULT_ID_NVM_READ_FAILURE,

    // Sensor Faults
    FAULT_ID_TEMP_SENSOR_FAILURE,
    FAULT_ID_HUMIDITY_SENSOR_FAILURE,
    FAULT_ID_LIGHT_SENSOR_FAILURE,
    
    // Actuator Faults
    FAULT_ID_FAN_CONTROL_FAILURE,
    FAULT_ID_HEATER_CONTROL_FAILURE,
    FAULT_ID_PUMP_CONTROL_FAILURE,
    FAULT_ID_VENTILATOR_CONTROL_FAILURE,
    FAULT_ID_LIGHT_CONTROL_FAILURE,

    // Communication Faults
    FAULT_ID_MODBUS_COMM_ERROR,
    FAULT_ID_MODBUS_CRC_ERROR,
    FAULT_ID_BLUETOOTH_COMM_ERROR,
    FAULT_ID_WIFI_CONN_FAILED,

    // Power Management Faults
    FAULT_ID_POWER_OVER_CURRENT,
    FAULT_ID_POWER_UNDER_VOLTAGE,

    // Diagnostic/OTA Faults
    FAULT_ID_OTA_DOWNLOAD_FAILED,
    FAULT_ID_OTA_INTEGRITY_CHECK_FAILED,

    // System Health Faults
    FAULT_ID_CPU_OVERLOAD_PERSISTENT,
    FAULT_ID_STACK_OVERFLOW_RISK,
    
    // Total count of defined faults
    SysMon_TOTAL_FAULT_IDS
} SystemMonitor_FaultId_t;

// --- Fault Log Sizes ---
#define SysMon_HISTORY_SIZE          50   // Number of historical fault records
#define SysMon_MAX_ACTIVE_FAULTS     10   // Max concurrent active faults

// --- Monitoring Thresholds ---
#define SysMon_CPU_LOAD_THRESHOLD_PERCENT         85  // % CPU load
#define SysMon_MIN_FREE_STACK_THRESHOLD_BYTES     512 // Bytes

// --- Periodic Task Settings ---
#define SysMon_MONITOR_PERIOD_MS                  5000 // 5 seconds

#endif // SYSTEM_MONITOR_CFG_H