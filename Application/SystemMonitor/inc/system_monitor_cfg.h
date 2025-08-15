/* ============================================================================
 * CONFIG FILE: Application/SystemMonitor/cfg/system_monitor_cfg.h
 * ============================================================================ */

#ifndef SYSTEM_MONITOR_CFG_H
#define SYSTEM_MONITOR_CFG_H

#include <stdint.h>
#include <stdbool.h>

/* --- Fault Definitions --- */
typedef enum 
{
    FAULT_ID_NONE = 0,
    
    /* Temperature Faults */
    FAULT_ID_TEMP_OVER_RANGE = 0x0001,
    FAULT_ID_TEMP_UNDER_RANGE = 0x0002,
    FAULT_ID_TEMP_SENSOR_DISCONNECTED = 0x0003,
    
    /* Humidity Faults */
    FAULT_ID_HUM_OVER_RANGE = 0x0004,
    FAULT_ID_HUM_UNDER_RANGE = 0x0005,
    FAULT_ID_HUM_SENSOR_DISCONNECTED = 0x0006,
    
    /* System Health Faults */
    FAULT_ID_CPU_OVERLOAD = 0x0007,
    FAULT_ID_STACK_OVERFLOW_RISK = 0x0008,
    
    /* Component Feedback Faults */
    FAULT_ID_FAN_FEEDBACK_ERROR = 0x0009,
    FAULT_ID_HEATER_FEEDBACK_ERROR = 0x000A,
    FAULT_ID_PUMP_FEEDBACK_ERROR = 0x000B,
    FAULT_ID_VENT_FEEDBACK_ERROR = 0x000C,
    FAULT_ID_LIGHT_FEEDBACK_ERROR = 0x000D,
    
    /* External Component Faults (received from other modules) */
    FAULT_ID_MODBUS_CRC_ERROR = 0x000E,
    FAULT_ID_BT_CONNECTION_FAILURE = 0x000F,
    FAULT_ID_WIFI_CONN_FAILURE = 0x0010,
    FAULT_ID_STORAGE_FAILURE = 0x0011,
    FAULT_ID_POWER_FAILURE = 0x0012,
    
    FAULT_ID_MAX
} SystemMonitor_FaultId_t;


/* Simplified fault record structure */
typedef struct {
    uint32_t    u32FaultId;    /* Fault ID */
    bool        is_active;     /* Active status */
} SystemMonitor_FaultRecord_t;


/* --- Configuration Parameters --- */

// The maximum number of faults the System Monitor can track.
// This defines the size of the SystemMonitor_FaultTable.
#define SYSMON_MAX_FAULTS                       (20) // Number of FAULT_ID_MAX - 1 (excluding FAULT_ID_NONE)

// Threshold for CPU load percentage above which a fault is reported
#define SYSMON_CPU_LOAD_THRESHOLD_PERCENT       (85) // %

// Threshold for minimum free stack space (bytes) below which a fault is reported
#define SYSMON_MIN_FREE_STACK_THRESHOLD_BYTES   (1024) // 1KB

// Interval for periodic system health logging (in seconds).
#define SYSMON_HEALTH_LOG_INTERVAL_SEC          (10) // Log every 10 seconds

// Polling interval for SysMon_MainFunction (in milliseconds).
// This value is used for calculating the logging frequency.
#define SYSMON_POLLING_INTERVAL_MS              (1000) // 1 second


/* Fault table extern declaration */
extern SystemMonitor_FaultRecord_t SystemMonitor_FaultTable[SYSMON_MAX_FAULTS];

#endif /* SYSTEM_MONITOR_CFG_H */

