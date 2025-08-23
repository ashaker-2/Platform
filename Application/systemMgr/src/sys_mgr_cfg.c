
/**
 * @file sys_mgr_cfg.c
 * @brief Configuration data definitions for the SystemMgr component.
 *
 * This file is for defining any non-volatile or complex configuration data
 * structures. As per the DDD, SystemMgr's configuration is primarily
 * handled via preprocessor macros in the header file, so this file
 * is provided as a placeholder.
 */
#include "sys_mgr_cfg.h"
#include "temphumctrl_cfg.h"
#include "fanctrl_cfg.h"
// // --- Temperature Sensor to Actuator Mapping Configuration ---
// const temp_sensor_actuator_mapping_t s_temp_sensor_actuator_mappings[TEMPHUM_SENSOR_ID_COUNT] = {
//     // NTC_1 - High-temperature industrial application
//     {
//         .sensor_id = TEMPHUM_SENSOR_ID_NTC_1,
//         .fan_actuator_id = ACTUATOR_ID_FAN_1,
//         .heater_actuator_id = ACTUATOR_ID_HEATER_1,
//         .fan_enabled = true,        // Enable cooling for high-temp applications
//         .heater_enabled = true,     // Enable heating for low-temp protection
//     },
//     // NTC_2 - Medium-temperature application
//     {
//         .sensor_id = TEMPHUM_SENSOR_ID_NTC_2,
//         .fan_actuator_id = ACTUATOR_ID_FAN_2,
//         .heater_actuator_id = ACTUATOR_ID_HEATER_2,
//         .fan_enabled = true,
//         .heater_enabled = true,
//     },
//     // DHT_1 - Office/Room environment
//     {
//         .sensor_id = TEMPHUM_SENSOR_ID_DHT_1,
//         .fan_actuator_id = ACTUATOR_ID_FAN_3,
//         .heater_actuator_id = ACTUATOR_ID_HEATER_3,
//         .fan_enabled = true,        // Enable for comfort control
//         .heater_enabled = true,     // Enable for comfort control
//     },
//     // DHT_2 - Storage environment
//     {
     
//         .sensor_id = TEMPHUM_SENSOR_ID_DHT_2,
//         .fan_actuator_id = ACTUATOR_ID_FAN_4,
//         .heater_actuator_id = ACTUATOR_ID_HEATER_4,
//         .fan_enabled = true,        // Critical for storage preservation
//         .heater_enabled = true,     // Prevent storage from getting too cold
//     },
//     // DHT_3 - Greenhouse/Growing environment
//     {
//         .sensor_id = TEMPHUM_SENSOR_ID_DHT_3,
//         .fan_actuator_id = ACTUATOR_ID_FAN_5,
//         .heater_actuator_id = ACTUATOR_ID_HEATER_5,
//         .fan_enabled = true,        // Essential for plant growth control
//         .heater_enabled = true,     // Protect plants from cold
//     },
//     // DHT_4 - Server room environment
//     {
//         .sensor_id = TEMPHUM_SENSOR_ID_DHT_4,
//         .fan_actuator_id = ACTUATOR_ID_FAN_6,
//         .heater_actuator_id = ACTUATOR_ID_HEATER_6,
//         .fan_enabled = true,        // Critical for server cooling
//         .heater_enabled = false,    // Servers usually don't need heating
//     },
// };

// // --- Humidity Sensor to Actuator Mapping Configuration ---
// // Note: Only DHT sensors are included as NTC thermistors don't measure humidity
// const hum_sensor_actuator_mapping_t s_hum_sensor_actuator_mappings[TEMPHUM_SENSOR_ID_COUNT] = {
//     // DHT_1 - Office/Room environment
//     {
//         .sensor_id = TEMPHUM_SENSOR_ID_DHT_1,
//         .ventilator_actuator_id = ACTUATOR_ID_VENTILATOR_1,
//         .pump_actuator_id = ACTUATOR_ID_PUMP_1,
//         .ventilator_enabled = true, // Enable dehumidification for mold prevention
//         .pump_enabled = true,       // Enable humidification for comfort
//     },
//     // DHT_2 - Storage environment
//     {
//         .sensor_id = TEMPHUM_SENSOR_ID_DHT_2,
//         .ventilator_actuator_id = ACTUATOR_ID_VENTILATOR_2,
//         .pump_actuator_id = ACTUATOR_ID_PUMP_2,
//         .ventilator_enabled = true, // Critical for storage preservation
//         .pump_enabled = true,       // Maintain optimal storage humidity
//     },
//     // DHT_3 - Greenhouse/Growing environment
//     {
//         .sensor_id = TEMPHUM_SENSOR_ID_DHT_3,
//         .ventilator_actuator_id = ACTUATOR_ID_VENTILATOR_3,
//         .pump_actuator_id = ACTUATOR_ID_PUMP_3,
//         .ventilator_enabled = true, // Essential for plant health
//         .pump_enabled = true,       // Critical for plant growth
//     },
//     // DHT_4 - Server room environment
//     {
//         .sensor_id = TEMPHUM_SENSOR_ID_DHT_4,
//         .ventilator_actuator_id = ACTUATOR_ID_VENTILATOR_4,
//         .pump_actuator_id = ACTUATOR_ID_PUMP_4,
//         .ventilator_enabled = true, // Prevent condensation on electronics
//         .pump_enabled = true,       // Prevent static electricity buildup
//     },
//     // DHT_4 - Server room environment
//     {
//         .sensor_id = TEMPHUM_SENSOR_ID_DHT_4,
//         .ventilator_actuator_id = ACTUATOR_ID_VENTILATOR_4,
//         .pump_actuator_id = ACTUATOR_ID_PUMP_4,
//         .ventilator_enabled = true, // Prevent condensation on electronics
//         .pump_enabled = true,       // Prevent static electricity buildup
//     },
//     // DHT_4 - Server room environment
//     {
//         .sensor_id = TEMPHUM_SENSOR_ID_DHT_4,
//         .ventilator_actuator_id = ACTUATOR_ID_VENTILATOR_4,
//         .pump_actuator_id = ACTUATOR_ID_PUMP_4,
//         .ventilator_enabled = true, // Prevent condensation on electronics
//         .pump_enabled = true,       // Prevent static electricity buildup
//     },
// };
