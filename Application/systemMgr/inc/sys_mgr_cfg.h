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




// /**
//  * @brief Structure to map temperature sensors to their corresponding fan and heater actuators.
//  */
// typedef struct
// {
//     TempHum_Sensor_ID_t sensor_id;    ///< The temperature sensor ID
//     Actuator_ID_t fan_actuator_id;    ///< Fan actuator ID for cooling (when temp is HIGH)
//     Actuator_ID_t heater_actuator_id; ///< Heater actuator ID for heating (when temp is LOW)
//     bool fan_enabled;                 ///< Enable/disable fan control for this sensor
//     bool heater_enabled;              ///< Enable/disable heater control for this sensor
// } temp_sensor_actuator_mapping_t;

// /**
//  * @brief Structure to map humidity sensors to their corresponding ventilator and pump actuators.
//  */
// typedef struct
// {
//     TempHum_Sensor_ID_t sensor_id;        ///< The humidity sensor ID (only DHT sensors)
//     Actuator_ID_t ventilator_actuator_id; ///< Ventilator actuator ID for dehumidification (when humidity is HIGH)
//     Actuator_ID_t pump_actuator_id;       ///< Pump actuator ID for humidification (when humidity is LOW)
//     bool ventilator_enabled;              ///< Enable/disable ventilator control for this sensor
//     bool pump_enabled;                    ///< Enable/disable pump control for this sensor
// } hum_sensor_actuator_mapping_t;

// // --- External Configuration Array Declarations ---

// /**
//  * @brief Array containing the mapping of temperature sensors to their actuators.
//  * This maps each temperature sensor to its corresponding fan (for cooling) and heater (for heating).
//  */
// extern const temp_sensor_actuator_mapping_t s_temp_sensor_actuator_mappings[TEMPHUM_SENSOR_ID_COUNT];

// /**
//  * @brief Array containing the mapping of humidity sensors to their actuators.
//  * This maps each humidity-capable sensor to its corresponding ventilator (for dehumidification) 
//  * and pump (for humidification). NTC sensors are not included as they don't measure humidity.
//  */
// extern const hum_sensor_actuator_mapping_t s_hum_sensor_actuator_mappings[TEMPHUM_SENSOR_ID_COUNT - 2]; // -2 because NTC sensors don't have humidity


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