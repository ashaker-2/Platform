/* ============================================================================
 * SOURCE FILE: Application/tempHumCtrl/src/temphum_ctrl_cfg.c
 * ============================================================================*/
/**
 * @file temphum_ctrl_cfg.c
 * @brief Static configuration data for the Temperature and Humidity Control (TempHumCtrl) module.
 *
 * This file defines the arrays for sensor configurations and temperature thresholds.
 */

#include "temphumctrl_cfg.h" // Include configuration header for definitions

// --- Sensor Configurations ---
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
        .hw_pin = 4, // Assuming GPIO pin 4 for DHT 3
    },
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_4,
        .sensor_type = SENSOR_TYPE_DHT22,
        .hw_pin = 5, // Assuming GPIO pin 5 for DHT 4
    },
};

// --- Temperature & Humidity Threshold Configurations ---
// NOTE: This array is made non-const here to allow dynamic updates via SetTemperatureThreshold/SetHumidityThreshold.
// If you want to keep initial values const and allow runtime changes, you would need
// to copy these values to a separate, non-const array in TempHumCtrl_Init().
temphum_threshold_cfg_t s_temphum_threshold_configurations[TEMPHUM_SENSOR_ID_COUNT] = {
    // Thresholds for NTC 1 (Temperature only)
    {
        .sensor_id = TEMPHUM_SENSOR_ID_NTC_1,
        .hum_threshold_high = 0.0f, // NTC does not provide humidity
        .hum_threshold_low = 0.0f,  // NTC does not provide humidity
        .temp_threshold_above = 60.0f,
        .temp_threshold_below = 10.0f, // Example low threshold
    },
    // Thresholds for NTC 2 (Temperature only)
    {
        .sensor_id = TEMPHUM_SENSOR_ID_NTC_2,
        .hum_threshold_high = 0.0f, // NTC does not provide humidity
        .hum_threshold_low = 0.0f,  // NTC does not provide humidity
        .temp_threshold_above = 55.0f,
        .temp_threshold_below = 5.0f,
    },
    // Thresholds for DHT 1
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_1,
        .hum_threshold_high = 80.0f,
        .hum_threshold_low = 30.0f,
        .temp_threshold_above = 35.0f,
        .temp_threshold_below = 18.0f,
    },
    // Thresholds for DHT 2
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_2,
        .hum_threshold_high = 75.0f,
        .hum_threshold_low = 35.0f,
        .temp_threshold_above = 32.0f,
        .temp_threshold_below = 20.0f,
    },
    // Thresholds for DHT 3
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_3,
        .hum_threshold_high = 90.0f,
        .hum_threshold_low = 25.0f,
        .temp_threshold_above = 40.0f,
        .temp_threshold_below = 0.0f,
    },
    // Thresholds for DHT 4
    {
        .sensor_id = TEMPHUM_SENSOR_ID_DHT_4,
        .hum_threshold_high = 85.0f,
        .hum_threshold_low = 40.0f,
        .temp_threshold_above = 38.0f,
        .temp_threshold_below = 5.0f,
    },
};

// --- Sensor Status Array ---
// This array is made non-const to allow updates to the sensor's current status (LOW, NORMAL, HIGH).
temphum_status_t s_SensorStatus[TEMPHUM_SENSOR_ID_COUNT] = {
    { .sensor_id = TEMPHUM_SENSOR_ID_NTC_1, .temp_status = TEMPHUM_STATUS_LEVEL_NORMAL, .hum_status = TEMPHUM_STATUS_LEVEL_NORMAL },
    { .sensor_id = TEMPHUM_SENSOR_ID_NTC_2, .temp_status = TEMPHUM_STATUS_LEVEL_NORMAL, .hum_status = TEMPHUM_STATUS_LEVEL_NORMAL },
    { .sensor_id = TEMPHUM_SENSOR_ID_DHT_1, .temp_status = TEMPHUM_STATUS_LEVEL_NORMAL, .hum_status = TEMPHUM_STATUS_LEVEL_NORMAL },
    { .sensor_id = TEMPHUM_SENSOR_ID_DHT_2, .temp_status = TEMPHUM_STATUS_LEVEL_NORMAL, .hum_status = TEMPHUM_STATUS_LEVEL_NORMAL },
    { .sensor_id = TEMPHUM_SENSOR_ID_DHT_3, .temp_status = TEMPHUM_STATUS_LEVEL_NORMAL, .hum_status = TEMPHUM_STATUS_LEVEL_NORMAL },
    { .sensor_id = TEMPHUM_SENSOR_ID_DHT_4, .temp_status = TEMPHUM_STATUS_LEVEL_NORMAL, .hum_status = TEMPHUM_STATUS_LEVEL_NORMAL },
};
