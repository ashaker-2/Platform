/* ============================================================================
 * SOURCE FILE: Application/tempHumCtrl/inc/temphum_ctrl_cfg.h
 * ============================================================================*/
/**
 * @file temphum_ctrl_cfg.h
 * @brief Configuration definitions for the Temperature and Humidity Control (TempHumCtrl) module.
 *
 * This file defines sensor types, IDs, threshold levels, and the structures
 * used for configuring sensors and their thresholds.
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
    SENSOR_TYPE_DHT11 = 0,              ///< DHT11 digital sensor (provides temperature and humidity)
    SENSOR_TYPE_DHT22 = 1,              ///< DHT22 digital sensor (provides temperature and humidity, higher precision)
    SENSOR_TYPE_NTC_THERMISTOR, ///< NTC Thermistor (provides temperature via ADC)
    SENSOR_TYPE_COUNT               ///< Total number of sensor types
} Sensor_Type_t;

// --- Configuration Definitions ---

/**
 * @brief Enumeration for unique Temperature/Humidity Sensor IDs.
 * These IDs are used to reference specific sensors.
 */
typedef enum
{
    TEMPHUM_SENSOR_ID_NTC_1 = 0, ///< Temperature sensor 1 (NTC)
    TEMPHUM_SENSOR_ID_NTC_2,     ///< Temperature sensor 2 (NTC)
    TEMPHUM_SENSOR_ID_DHT_1,     ///< DHT sensor 1
    TEMPHUM_SENSOR_ID_DHT_2,     ///< DHT sensor 2
    TEMPHUM_SENSOR_ID_DHT_3,     ///< DHT sensor 3
    TEMPHUM_SENSOR_ID_DHT_4,     ///< DHT sensor 4
    TEMPHUM_SENSOR_ID_COUNT      ///< Total number of configured temperature/humidity sensors
} TempHum_Sensor_ID_t;

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
 * associated hardware pin/address.
 */
typedef struct
{
    TempHum_Sensor_ID_t sensor_id; ///< Unique identifier for the sensor
    Sensor_Type_t sensor_type;     ///< The type of sensor (e.g., NTC_THERMISTOR, DHT11)
    uint8_t hw_pin;                ///< The hardware pin or address the sensor is connected to (ADC channel or GPIO number)
} temphum_sensor_config_item_t;

/**
 * @brief Structure to hold threshold configurations for a single temperature/humidity sensor.
 *
 * This includes thresholds for both temperature (above/below) and humidity.
 */
typedef struct
{
    TempHum_Sensor_ID_t sensor_id; ///< The sensor this threshold applies to
    float hum_threshold_high;      ///< The humidity value above which is considered HIGH
    float hum_threshold_low;       ///< The humidity value below which is considered LOW
    float temp_threshold_above;    ///< The temperature value above which is considered HIGH
    float temp_threshold_below;    ///< The temperature value below which is considered LOW

} temphum_threshold_cfg_t;

/**
 * @brief Structure to hold the current status level of temperature and humidity for a sensor.
 */
typedef struct
{
    TempHum_Sensor_ID_t sensor_id;      ///< The sensor this status belongs to
    TempHum_Status_Level_t temp_status; ///< Current temperature status (LOW, NORMAL, HIGH)
    TempHum_Status_Level_t hum_status;  ///< Current humidity status (LOW, NORMAL, HIGH)
} temphum_status_t;

// External declarations for configuration arrays (defined in .c file)
extern const temphum_sensor_config_item_t s_temphum_sensor_configurations[TEMPHUM_SENSOR_ID_COUNT];
extern temphum_threshold_cfg_t s_temphum_threshold_configurations[TEMPHUM_SENSOR_ID_COUNT]; // Made non-const for dynamic updates
extern temphum_status_t s_SensorStatus[TEMPHUM_SENSOR_ID_COUNT];                            // Made non-const for dynamic updates

#endif /* TEMPHUM_CTRL_CFG_H */
