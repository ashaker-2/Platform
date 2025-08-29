/* ============================================================================
 * SOURCE FILE: Application/tempHumCtrl/inc/temphum_ctrl_cfg.h
 * ============================================================================*/
/**
 * @file temphum_ctrl_cfg.h
 * @brief Configuration definitions for the Temperature and Humidity Control (TempHumCtrl) module.
 *
 * This file defines sensor types, IDs, threshold levels, and the structures
 * used for configuring sensors and their thresholds.
 * Enhanced to support 6 sensors with comprehensive configuration options.
 */

#ifndef TEMPHUM_CTRL_CFG_H
#define TEMPHUM_CTRL_CFG_H

#include "common.h" // For Status_t and other common types
#include <stddef.h> // For size_t
#include <stdint.h> // For uint8_t

// --- Sensor Type Definitions ---

/**
 * @brief Enumeration for different types of sensors supported by the HAL.
 */
typedef enum
{
    SENSOR_TYPE_DHT11 = 0,       ///< DHT11 digital sensor (provides temperature and humidity)
    SENSOR_TYPE_DHT22 = 1,       ///< DHT22 digital sensor (provides temperature and humidity, higher precision)
    SENSOR_TYPE_NTC_THERMISTOR,  ///< NTC Thermistor (provides temperature via ADC)
    SENSOR_TYPE_COUNT            ///< Total number of sensor types
} Sensor_Type_t;

// --- Configuration Definitions ---

/**
 * @brief Enumeration for unique Temperature/Humidity Sensor IDs.
 * These IDs are used to reference specific sensors.
 * Enhanced to support 6 sensors for comprehensive system testing.
 */
typedef enum
{
    TEMPHUM_SENSOR_ID_NTC_1 = 0, ///< Temperature sensor 1 (NTC) - High-temp industrial application
    TEMPHUM_SENSOR_ID_NTC_2,     ///< Temperature sensor 2 (NTC) - Medium-temp application
    TEMPHUM_SENSOR_ID_DHT_1,     ///< DHT sensor 1 (DHT11) - Office/Room environment
    TEMPHUM_SENSOR_ID_DHT_2,     ///< DHT sensor 2 (DHT11) - Storage environment
    TEMPHUM_SENSOR_ID_DHT_3,     ///< DHT sensor 3 (DHT22) - Greenhouse/Growing environment
    TEMPHUM_SENSOR_ID_DHT_4,     ///< DHT sensor 4 (DHT22) - Server room environment
    TEMPHUM_SENSOR_ID_COUNT      ///< Total number of configured temperature/humidity sensors
} TempHum_Sensor_ID_t;

#define TEMPHUM_ID_ALL (TEMPHUM_SENSOR_ID_COUNT)

/**
 * @brief Enumeration for the status level of a temperature or humidity reading against its thresholds.
 * This is used to indicate whether a value is below low threshold, normal, or above high threshold.
 */
typedef enum
{
    TEMPHUM_STATUS_LEVEL_LOW = 0, ///< Value is below the configured low threshold
    TEMPHUM_STATUS_LEVEL_NORMAL,  ///< Value is within the normal range
    TEMPHUM_STATUS_LEVEL_HIGH,    ///< Value is above the configured high threshold
    TEMPHUM_STATUS_LEVEL_COUNT    ///< Total number of status levels
} TempHum_Status_Level_t;

/**
 * @brief Structure to hold the configuration for a single temperature/humidity sensor.
 *
 * This includes the sensor ID, its specific type (e.g., NTC, DHT11), and its
 * associated hardware pin/address. Enhanced with better documentation for
 * different sensor applications.
 */
typedef struct
{
    TempHum_Sensor_ID_t sensor_id; ///< Unique identifier for the sensor
    Sensor_Type_t sensor_type;     ///< The type of sensor (e.g., NTC_THERMISTOR, DHT11, DHT22)
    uint8_t hw_pin;                ///< The hardware pin or address the sensor is connected to (ADC channel for NTC, GPIO pin for DHT)
} temphum_sensor_config_item_t;

/**
 * @brief Structure to hold threshold configurations for a single temperature/humidity sensor.
 *
 * This includes thresholds for both temperature (above/below) and humidity (high/low).
 * Enhanced with more descriptive field documentation for different applications.
 */
typedef struct
{
    TempHum_Sensor_ID_t sensor_id; ///< The sensor this threshold applies to
    float hum_threshold_high;      ///< The humidity value (%) above which is considered HIGH (potential issues like mold, condensation)
    float hum_threshold_low;       ///< The humidity value (%) below which is considered LOW (dry air, static electricity risk)
    float temp_threshold_above;    ///< The temperature value (°C) above which is considered HIGH (overheating, comfort issues)
    float temp_threshold_below;    ///< The temperature value (°C) below which is considered LOW (freezing risk, heating needed)
} temphum_threshold_cfg_t;

/**
 * @brief Structure to hold the current status level of temperature and humidity for a sensor.
 * Enhanced with better documentation for status interpretation.
 */
typedef struct
{
    TempHum_Sensor_ID_t sensor_id;      ///< The sensor this status belongs to
    TempHum_Status_Level_t temp_status; ///< Current temperature status (LOW, NORMAL, HIGH)
    TempHum_Status_Level_t hum_status;  ///< Current humidity status (LOW, NORMAL, HIGH) - N/A for NTC sensors
} temphum_status_t;

// --- Configuration Constants ---

/**
 * @brief Maximum number of samples stored for averaging calculations.
 * This defines the size of the circular buffer used for each sensor's averaging.
 */
#define TEMPHUM_AVERAGE_BUFFER_SIZE 10

/**
 * @brief Maximum age of samples in milliseconds for averaging calculations.
 * Samples older than this value are considered stale and excluded from averaging.
 */
#define TEMPHUM_MAX_SAMPLE_AGE_MS   30000  // 30 seconds

// --- External Configuration Array Declarations ---
// These arrays are defined in the corresponding .c file

/**
 * @brief Array containing the configuration for all sensors.
 * This is a const array as sensor hardware configuration typically doesn't change at runtime.
 */
extern const temphum_sensor_config_item_t s_temphum_sensor_configurations[TEMPHUM_SENSOR_ID_COUNT];

/**
 * @brief Array containing the threshold configurations for all sensors.
 * This is a non-const array to allow dynamic threshold updates via the SetThreshold functions.
 */
extern temphum_threshold_cfg_t s_temphum_threshold_configurations[TEMPHUM_SENSOR_ID_COUNT];

/**
 * @brief Array containing the current status of all sensors.
 * This is a non-const array as it's updated continuously based on sensor readings and threshold comparisons.
 */
extern temphum_status_t s_SensorStatus[TEMPHUM_SENSOR_ID_COUNT];

#endif /* TEMPHUM_CTRL_CFG_H */