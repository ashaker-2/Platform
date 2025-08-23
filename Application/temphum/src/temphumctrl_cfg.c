/* ============================================================================
 * SOURCE FILE: Application/tempHumCtrl/src/temphum_ctrl_cfg.c
 * ============================================================================*/
/**
 * @file temphum_ctrl_cfg.c
 * @brief Static configuration data for the Temperature and Humidity Control (TempHumCtrl) module.
 *
 * This file defines the arrays for sensor configurations and temperature thresholds.
 * Enhanced to support additional sensors for comprehensive testing.
 */

#include "temphumctrl_cfg.h" // Include configuration header for definitions

// --- Sensor Configurations ---
// Enhanced configuration with 6 sensors (2 NTC + 4 DHT sensors)
const temphum_sensor_config_item_t s_temphum_sensor_configurations[TEMPHUM_SENSOR_ID_COUNT] = {
    {
        .sensor_id = TEMPHUM_SENSOR_ID_NTC_1,
        .sensor_type = SENSOR_TYPE_NTC_THERMISTOR,
        .hw_pin = 0, // Assuming ADC channel 0 for NTC 1
    },
    {
        .sensor_id = TEMPHUM_SENSOR_ID_NTC_2,
        .sensor_type = SENSOR_TYPE_NTC_THERMISTOR,
        .hw_pin = 1, // Assuming ADC channel 1 for NTC 2
    },
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_1,
        .sensor_type = SENSOR_TYPE_DHT11,
        .hw_pin = 2, // Assuming GPIO pin 2 for DHT 1
    },
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_2,
        .sensor_type = SENSOR_TYPE_DHT11,
        .hw_pin = 3, // Assuming GPIO pin 3 for DHT 2
    },
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_3,
        .sensor_type = SENSOR_TYPE_DHT22,
        .hw_pin = 4, // Assuming GPIO pin 4 for DHT 3 (DHT22 for higher precision)
    },
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_4,
        .sensor_type = SENSOR_TYPE_DHT22,
        .hw_pin = 5, // Assuming GPIO pin 5 for DHT 4 (DHT22 for higher precision)
    },
};

// --- Temperature & Humidity Threshold Configurations ---
// NOTE: This array is made non-const here to allow dynamic updates via SetTemperatureThreshold/SetHumidityThreshold.
// If you want to keep initial values const and allow runtime changes, you would need
// to copy these values to a separate, non-const array in TempHumCtrl_Init().
// Enhanced with realistic threshold values for different applications
temphum_threshold_cfg_t s_temphum_threshold_configurations[TEMPHUM_SENSOR_ID_COUNT] = {
    // Thresholds for NTC 1 (Temperature only) - High-temperature application
    {
        .sensor_id = TEMPHUM_SENSOR_ID_NTC_1,
        .hum_threshold_high = 0.0f, // NTC does not provide humidity
        .hum_threshold_low = 0.0f,  // NTC does not provide humidity
        .temp_threshold_above = 60.0f,  // High temperature alarm for industrial applications
        .temp_threshold_below = 10.0f,  // Low temperature alarm
    },
    // Thresholds for NTC 2 (Temperature only) - Medium-temperature application
    {
        .sensor_id = TEMPHUM_SENSOR_ID_NTC_2,
        .hum_threshold_high = 0.0f, // NTC does not provide humidity
        .hum_threshold_low = 0.0f,  // NTC does not provide humidity
        .temp_threshold_above = 55.0f,  // Medium-high temperature alarm
        .temp_threshold_below = 5.0f,   // Low temperature alarm
    },
    // Thresholds for DHT 1 - Office/Room environment
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_1,
        .hum_threshold_high = 80.0f,    // High humidity alarm (potential mold risk)
        .hum_threshold_low = 30.0f,     // Low humidity alarm (dry air)
        .temp_threshold_above = 35.0f,  // High temperature for comfort
        .temp_threshold_below = 18.0f,  // Low temperature for comfort
    },
    // Thresholds for DHT 2 - Storage environment
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_2,
        .hum_threshold_high = 75.0f,    // Storage humidity limit
        .hum_threshold_low = 35.0f,     // Minimum humidity for storage
        .temp_threshold_above = 32.0f,  // Maximum storage temperature
        .temp_threshold_below = 20.0f,  // Minimum storage temperature
    },
    // Thresholds for DHT 3 - Greenhouse/Growing environment
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_3,
        .hum_threshold_high = 90.0f,    // Very high humidity (greenhouse conditions)
        .hum_threshold_low = 25.0f,     // Minimum humidity for plants
        .temp_threshold_above = 40.0f,  // Maximum growing temperature
        .temp_threshold_below = 0.0f,   // Freezing protection
    },
    // Thresholds for DHT 4 - Server room environment
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_4,
        .hum_threshold_high = 85.0f,    // High humidity alarm for electronics
        .hum_threshold_low = 40.0f,     // Minimum humidity to prevent static
        .temp_threshold_above = 38.0f,  // High temperature alarm for servers
        .temp_threshold_below = 5.0f,   // Low temperature alarm
    },
};

// --- Sensor Status Array ---
// This array is made non-const to allow updates to the sensor's current status (LOW, NORMAL, HIGH).
// Enhanced initialization with proper sensor ID mapping
temphum_status_t s_SensorStatus[TEMPHUM_SENSOR_ID_COUNT] = {
    { 
        .sensor_id = TEMPHUM_SENSOR_ID_NTC_1, 
        .temp_status = TEMPHUM_STATUS_LEVEL_NORMAL, 
        .hum_status = TEMPHUM_STATUS_LEVEL_NORMAL 
    },
    { 
        .sensor_id = TEMPHUM_SENSOR_ID_NTC_2, 
        .temp_status = TEMPHUM_STATUS_LEVEL_NORMAL, 
        .hum_status = TEMPHUM_STATUS_LEVEL_NORMAL 
    },
    { 
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_1, 
        .temp_status = TEMPHUM_STATUS_LEVEL_NORMAL, 
        .hum_status = TEMPHUM_STATUS_LEVEL_NORMAL 
    },
    { 
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_2, 
        .temp_status = TEMPHUM_STATUS_LEVEL_NORMAL, 
        .hum_status = TEMPHUM_STATUS_LEVEL_NORMAL 
    },
    { 
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_3, 
        .temp_status = TEMPHUM_STATUS_LEVEL_NORMAL, 
        .hum_status = TEMPHUM_STATUS_LEVEL_NORMAL 
    },
    { 
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_4, 
        .temp_status = TEMPHUM_STATUS_LEVEL_NORMAL, 
        .hum_status = TEMPHUM_STATUS_LEVEL_NORMAL 
    },
};