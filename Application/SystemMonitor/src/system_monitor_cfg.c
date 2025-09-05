/* ============================================================================
 * CONFIG FILE: Application/SystemMonitor/cfg/system_monitor_cfg.c
 * ============================================================================ */

#include "system_monitor_cfg.h"

/* Fault configuration table */
SystemMonitor_FaultRecord_t SystemMonitor_FaultTable[SYSMON_MAX_FAULTS] = {
    /* Fault ID,                     Active Status */
    {FAULT_ID_TEMP_OVER_RANGE,         false},
    {FAULT_ID_TEMP_UNDER_RANGE,        false},
    {FAULT_ID_TEMP_SENSOR_DISCONNECTED, false},
    {FAULT_ID_HUM_OVER_RANGE,          false},
    {FAULT_ID_HUM_UNDER_RANGE,         false},
    {FAULT_ID_HUM_SENSOR_DISCONNECTED, false},
    {FAULT_ID_OVERALL_CPU_LOAD,        false},
    {FAULT_ID_CORE_0_CPU_LOAD,         false},
    {FAULT_ID_CORE_1_CPU_LOAD,         false},
    {FAULT_ID_STACK_OVERFLOW,          false},
    {FAULT_ID_STACK_UNDERFLOW,         false},
    {FAULT_ID_FAN_FEEDBACK_ERROR,      false},
    {FAULT_ID_HEATER_FEEDBACK_ERROR,   false},
    {FAULT_ID_PUMP_FEEDBACK_ERROR,     false},
    {FAULT_ID_VENT_FEEDBACK_ERROR,     false},
    {FAULT_ID_LIGHT_FEEDBACK_ERROR,    false},
    {FAULT_ID_MODBUS_CRC_ERROR,        false},
    {FAULT_ID_BT_CONNECTION_FAILURE,   false},
    {FAULT_ID_WIFI_CONN_FAILURE,       false},
    {FAULT_ID_STORAGE_FAILURE,         false},
    {FAULT_ID_POWER_FAILURE,           false},
    {FAULT_ID_SYS_INIT_ERROR,          false}, /* Reserved */
    {FAULT_ID_SYS_INIT_ERROR,          false}  /* Reserved */
};


