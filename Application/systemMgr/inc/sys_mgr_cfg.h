#ifndef SYS_MGR_CFG_H
#define SYS_MGR_CFG_H

#include "common.h"
#include "system_monitor.h" // For fault IDs
#include <stdint.h>

/**
 * @file sys_mgr_cfg.h
 * @brief Configuration header for the SystemMgr component.
 *
 * This file defines all the necessary configuration macros for the
 * SystemMgr module, including default operational parameters and thresholds.
 */

// --- Internal Configuration Types (Persisted to storage) ---
typedef struct {
    float operational_temp_min_c;
    float operational_temp_max_c;
    float operational_humidity_min_p;
    float operational_humidity_max_p;
    uint8_t fan_stage_threshold_temp_c[3];
} SystemOperationalParams_t;

// --- Fault ID Definitions (from SystemMonitor) ---
// These are defined here for direct use in the SystemMgr module
#define FAULT_ID_SYS_MGR_FIRE_ALARM         (SystemMonitor_FaultId_t)200 // Custom ID for specific fault

// --- Default Operational Parameters (used if storage read fails) ---
#define SYS_MGR_DEFAULT_TEMP_MIN_C          20.0f
#define SYS_MGR_DEFAULT_TEMP_MAX_C          25.0f
#define SYS_MGR_DEFAULT_HUMIDITY_MIN_P      40.0f
#define SYS_MGR_DEFAULT_HUMIDITY_MAX_P      60.0f

// --- Fan Staging Thresholds (example for 3 stages) ---
#define SYS_MGR_FAN_STAGE_1_TEMP_C          26.0f
#define SYS_MGR_FAN_STAGE_2_TEMP_C          28.0f
#define SYS_MGR_FAN_STAGE_3_TEMP_C          30.0f
#define SYS_MGR_FAN_STAGE_THRESHOLDS_C {SYS_MGR_FAN_STAGE_1_TEMP_C, SYS_MGR_FAN_STAGE_2_TEMP_C, SYS_MGR_FAN_STAGE_3_TEMP_C}

// --- Fire Alarm Threshold ---
#define SYS_MGR_FIRE_TEMP_THRESHOLD_C       60.0f

#endif // SYS_MGR_CFG_H