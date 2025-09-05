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
    FAULT_ID_OVERALL_CPU_LOAD = 0x0030,
    FAULT_ID_CORE_0_CPU_LOAD = 0x00031,
    FAULT_ID_CORE_1_CPU_LOAD = 0x00032,
    FAULT_ID_STACK_OVERFLOW = 0x00034,
    FAULT_ID_STACK_UNDERFLOW = 0x00035,
    
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

    FAULT_ID_SYS_INIT_ERROR = 0x0020,
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

#define SYSMON_QUEUE_LENGTH   16
#define SYSMON_QUEUE_WAIT_MS  0   // non-blocking enqueue


/**
 * @brief Per-core hyperperiod-aware CPU load calculation for ESP32 SMP FreeRTOS
 *
 * System configuration:
 * - Core 0 tasks periods: 20ms, 100ms, 150ms, 200ms -> Hyperperiod = 600ms
 * - Core 1 tasks periods: 50ms -> Hyperperiod = 50ms  
 * - System hyperperiod: LCM(20, 50, 100, 150, 200) = 600ms
 */

/* System hyperperiod configuration */
#define SYSTEM_HYPERPERIOD_MS 600    // LCM(20, 50, 100, 150, 200)
#define CORE0_HYPERPERIOD_MS 600     // LCM(20, 100, 150, 200)  
#define CORE1_HYPERPERIOD_MS 50      // LCM(50)
#define MIN_SAMPLES_PER_CORE 5       // Minimum samples before valid reading
#define NUMBER_OF_ALL_PERIODS 5


/* Fault table extern declaration */
extern SystemMonitor_FaultRecord_t SystemMonitor_FaultTable[SYSMON_MAX_FAULTS];
/* System hyperperiod is LCM of all periods */
extern uint32_t all_periods[NUMBER_OF_ALL_PERIODS];
#endif /* SYSTEM_MONITOR_CFG_H */

