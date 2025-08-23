/* ============================================================================
 * SOURCE FILE: Application/tempHumCtrl/inc/temphum_ctrl.h
 * ============================================================================*/
/**
 * @file temphum_ctrl.h
 * @brief Public API for the Temperature and Humidity Control (TempHumCtrl) module.
 */

#ifndef TEMPHUM_CTRL_H
#define TEMPHUM_CTRL_H

#include "common.h"           // For Status_t
#include "temphumctrl_cfg.h" // For sensor IDs, types, and configuration structures

// --- Public API Functions ---

/**
 * @brief Initializes the Temperature and Humidity Control (TempHumCtrl) module.
 *
 * This function prepares the module for sensor readings and monitoring.
 * Note: It is assumed that underlying HALs (like ADC, GPIO for DHT) are initialized separately
 * by a global HAL_Init() or component-specific initialization functions.
 *
 * @return E_OK on successful initialization, or an error code otherwise.
 */
Status_t TempHumCtrl_Init(void);

/**
 * @brief Main function for the Temperature and Humidity Control module.
 *
 * This function should be called periodically (e.g., in a main loop or RTOS task).
 * It iterates through all configured sensors, reads their data,
 * updates their stored values, and then checks for threshold breaches.
 */
void TempHumCtrl_MainFunction(void);

/**
 * @brief Retrieves the last known current temperature for a specific sensor.
 *
 * This function does NOT initiate a new sensor read; it returns the value
 * last updated by `TempHumCtrl_MainFunction`.
 *
 * @param sensor_id The unique identifier of the sensor.
 * @param temperature_out Pointer to a float where the current temperature in Celsius
 * will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or `temperature_out` is NULL.
 */
Status_t TempHumCtrl_GetTemperature(TempHum_Sensor_ID_t sensor_id, float *temperature_out);

/**
 * @brief Retrieves the last known current humidity for a specific sensor.
 *
 * This function does NOT initiate a new sensor read; it returns the value
 * last updated by `TempHumCtrl_MainFunction`.
 *
 * @param sensor_id The unique identifier of the sensor.
 * @param humidity_out Pointer to a float where the current humidity in percentage (%)
 * will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or `humidity_out` is NULL,
 * E_NOT_SUPPORTED if the sensor type does not provide humidity.
 */
Status_t TempHumCtrl_GetHumidity(TempHum_Sensor_ID_t sensor_id, float *humidity_out);

/**
 * @brief Retrieves the current temperature status (LOW, NORMAL, HIGH) for a specific sensor.
 * @param sensor_id The unique identifier of the sensor.
 * @param temp_status_out Pointer to a TempHum_Status_Level_t where the status will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or `temp_status_out` is NULL.
 */
Status_t TempHumCtrl_GetTemperatureStatus(TempHum_Sensor_ID_t sensor_id, TempHum_Status_Level_t *temp_status_out);

/**
 * @brief Retrieves the current humidity status (LOW, NORMAL, HIGH) for a specific sensor.
 * @param sensor_id The unique identifier of the sensor.
 * @param hum_status_out Pointer to a TempHum_Status_Level_t where the status will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or `hum_status_out` is NULL,
 * E_NOT_SUPPORTED if the sensor type does not provide humidity.
 */
Status_t TempHumCtrl_GetHumidityStatus(TempHum_Sensor_ID_t sensor_id, TempHum_Status_Level_t *hum_status_out);

/**
 * @brief Retrieves the configured temperature threshold values (above and below) for a specific sensor.
 *
 * @param sensor_id The unique identifier of the sensor.
 * @param above_threshold_out Pointer to a float where the above temperature threshold will be stored. Must not be NULL.
 * @param below_threshold_out Pointer to a float where the below temperature threshold will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or output pointers are NULL.
 */
Status_t TempHumCtrl_GetTemperatureThreshold(TempHum_Sensor_ID_t sensor_id, float *above_threshold_out, float *below_threshold_out);

/**
 * @brief Retrieves the configured humidity threshold values (high and low) for a specific sensor.
 *
 * @param sensor_id The unique identifier of the sensor.
 * @param high_threshold_out Pointer to a float where the high humidity threshold will be stored. Must not be NULL.
 * @param low_threshold_out Pointer to a float where the low humidity threshold will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or output pointers are NULL.
 * E_NOT_SUPPORTED if the sensor type does not provide humidity.
 */
Status_t TempHumCtrl_GetHumidityThreshold(TempHum_Sensor_ID_t sensor_id, float *high_threshold_out, float *low_threshold_out);

/**
 * @brief Dynamically sets new temperature threshold values (above and below) for a specific sensor.
 *
 * This function updates the threshold values in the internal configuration.
 * Note that this is an in-memory change and typically does not persist across reboots
 * unless explicitly saved to non-volatile memory elsewhere.
 *
 * @param sensor_id The unique identifier of the sensor.
 * @param new_above_threshold_celsius The new temperature value above which is considered HIGH.
 * @param new_below_threshold_celsius The new temperature value below which is considered LOW.
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or threshold values are illogical.
 */
Status_t TempHumCtrl_SetTemperatureThreshold(TempHum_Sensor_ID_t sensor_id, float new_above_threshold_celsius, float new_below_threshold_celsius);

/**
 * @brief Dynamically sets new humidity threshold values (high and low) for a specific sensor.
 *
 * This function updates the threshold values in the internal configuration.
 * Note that this is an in-memory change and typically does not persist across reboots
 * unless explicitly saved to non-volatile memory elsewhere.
 *
 * @param sensor_id The unique identifier of the sensor.
 * @param new_high_threshold_humidity The new humidity value above which is considered HIGH.
 * @param new_low_threshold_humidity The new humidity value below which is considered LOW.
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or threshold values are illogical.
 * E_NOT_SUPPORTED if the sensor type does not provide humidity.
 */
Status_t TempHumCtrl_SetHumidityThreshold(TempHum_Sensor_ID_t sensor_id, float new_high_threshold_humidity, float new_low_threshold_humidity);

#endif /* TEMPHUM_CTRL_H */
