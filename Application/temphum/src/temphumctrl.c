/* ============================================================================
 * SOURCE FILE: Application/tempHumCtrl/src/temphum_ctrl.c
 * ============================================================================*/
/**
 * @file temphum_ctrl.c
 * @brief Implements the Temperature and Humidity Control (TempHumCtrl) module.
 *
 * This file includes the internal logic, calls HAL layer functions,
 * and uses configurations defined in `temphum_ctrl_cfg.c`.
 * Enhanced with averaging functionality and sensor counting.
 */

#include "temphumctrl.h"     // Public API for TempHumCtrl
#include "temphumctrl_cfg.h" // Configuration data declarations (e.g., s_temphum_sensor_configurations)
#include "common.h"          // Common status codes (e.g., E_OK, E_INVALID_PARAM)
#include "logger.h"          // Logging macros (e.g., LOGI, LOGE, LOGW, LOGD)
#include "HAL_ADC.h"         // External HAL for ADC readings (for NTC thermistors)
#include "dht.h"             // External driver for DHT sensors

#include <string.h> // For memset
#include <stdlib.h> // For rand() (used in dummy ADC/DHT drivers)
#include <time.h>   // For time() to seed rand() (used in dummy ADC/DHT drivers)
#include <math.h>   // For mathematical functions (if complex NTC conversion)

static const char *TAG = "TempHumCtrl"; // Logging tag for this module

// --- Configuration Constants for Averaging ---
#define TEMPHUM_AVERAGE_BUFFER_SIZE 10    // Number of samples to keep for averaging
#define TEMPHUM_MAX_SAMPLE_AGE_MS   30000 // Maximum age of samples in milliseconds (30 seconds)

// --- Internal Module Data ---
static bool s_temphum_ctrl_initialized = false;

// Arrays to store the last read temperature and humidity for each sensor ID.
static float s_current_temperatures[TEMPHUM_SENSOR_ID_COUNT];
static float s_current_humidity[TEMPHUM_SENSOR_ID_COUNT];

static float f32AvgSysHumidity = 0;
static float f32AvgSysTemp = 0;

// --- Enhanced Data Structures for Averaging ---
typedef struct {
    float values[TEMPHUM_AVERAGE_BUFFER_SIZE];
    uint32_t timestamps[TEMPHUM_AVERAGE_BUFFER_SIZE];
    uint8_t write_index;
    uint8_t count;
    bool initialized;
} average_buffer_t;

// Buffers for temperature and humidity averaging
static average_buffer_t s_temp_avg_buffers[TEMPHUM_SENSOR_ID_COUNT];
static average_buffer_t s_hum_avg_buffers[TEMPHUM_SENSOR_ID_COUNT];

// --- Helper Functions for Time Management ---
/**
 * @brief Gets current time in milliseconds.
 * In a real system, this would interface with the system tick or RTC.
 * For demonstration, we use a simple counter.
 */
static uint32_t prv_get_current_time_ms(void)
{
    static uint32_t time_counter = 0;
    time_counter += 100; // Simulate 100ms increment per call
    return time_counter;
}

// --- Private Helper Functions for Averaging ---

/**
 * @brief Initializes an averaging buffer.
 * @param buffer Pointer to the buffer to initialize.
 */
static void prv_init_average_buffer(average_buffer_t *buffer)
{
    if (buffer == NULL) return;
    
    memset(buffer->values, 0, sizeof(buffer->values));
    memset(buffer->timestamps, 0, sizeof(buffer->timestamps));
    buffer->write_index = 0;
    buffer->count = 0;
    buffer->initialized = true;
}

/**
 * @brief Adds a new value to the averaging buffer.
 * @param buffer Pointer to the buffer.
 * @param value The value to add.
 * @param timestamp Current timestamp in milliseconds.
 */
static void prv_add_to_average_buffer(average_buffer_t *buffer, float value, uint32_t timestamp)
{
    if (buffer == NULL || !buffer->initialized) return;
    
    buffer->values[buffer->write_index] = value;
    buffer->timestamps[buffer->write_index] = timestamp;
    
    buffer->write_index = (buffer->write_index + 1) % TEMPHUM_AVERAGE_BUFFER_SIZE;
    
    if (buffer->count < TEMPHUM_AVERAGE_BUFFER_SIZE) {
        buffer->count++;
    }
}

/**
 * @brief Calculates the average from the buffer, excluding stale data.
 * @param buffer Pointer to the buffer.
 * @param current_time Current timestamp in milliseconds.
 * @param average_out Pointer to store the calculated average.
 * @return E_OK if average calculated successfully, E_DATA_STALE if no fresh data available.
 */
static Status_t prv_calculate_average_from_buffer(const average_buffer_t *buffer, uint32_t current_time, float *average_out)
{
    if (buffer == NULL || !buffer->initialized || average_out == NULL) {
        return E_INVALID_PARAM;
    }
    
    if (buffer->count == 0) {
        return E_DATA_STALE;
    }
    
    float sum = 0.0f;
    uint16_t valid_count = 0;
    
    // Calculate average using only fresh data
    for (uint8_t i = 0; i < buffer->count; i++) {
        uint32_t age = current_time - buffer->timestamps[i];
        if (age <= TEMPHUM_MAX_SAMPLE_AGE_MS) {
            sum += buffer->values[i];
            valid_count++;
        }
    }
    
    if (valid_count == 0) {
        return E_DATA_STALE;
    }
    
    *average_out = sum / valid_count;
    return E_OK;
}

// --- Private Functions ---

/**
 * @brief Converts a raw NTC ADC value to Celsius.
 * This is a simplified, linear approximation for demonstration.
 * In a real application, this would use the Steinhart-Hart equation or a lookup table.
 * Assumes ADC range 0-4095 (12-bit).
 * Temperature range: approx 0C (high ADC) to 100C (low ADC).
 * @param adc_raw The raw ADC value.
 * @return Temperature in Celsius.
 */
static float prv_ntc_adc_to_celsius(uint16_t adc_raw)
{
    // Simplified conversion: Higher ADC generally means lower resistance for NTC, which means higher temp.
    // However, the example here models it inversed for illustrative purposes.
    // For a real NTC, you'd use the Steinhart-Hart equation or a calibrated lookup table.
    float temp = 100.0f - (((float)adc_raw / 4095.0f) * 100.0f);
    return temp;
}

/**
 * @brief Internal function to perform a sensor read using the appropriate HAL/driver.
 * This function calls either HAL_ADC_ReadRaw or dht_read_data based on sensor_type.
 *
 * @param sensor_type The type of sensor (NTC, DHT11, DHT22).
 * @param hw_pin The hardware pin/channel for the sensor.
 * @param temperature_out Pointer to store the read temperature.
 * @param humidity_out Pointer to store the read humidity.
 * @return E_OK on success, or an error code.
 */
static Status_t prv_read_sensor_data_from_hw(Sensor_Type_t sensor_type, uint8_t hw_pin, float *temperature_out, float *humidity_out)
{
    if (temperature_out == NULL || humidity_out == NULL)
    {
        LOGE(TAG, "prv_read_sensor_data_from_hw: NULL output pointers.");
        return E_INVALID_PARAM;
    }

    Status_t status = E_FAIL;
    uint16_t adc_raw_value = 0;
    int16_t temp_raw_value = 0;
    int16_t hum_raw_value = 0;

    switch (sensor_type)
    {
    case SENSOR_TYPE_NTC_THERMISTOR:
        status = HAL_ADC_ReadRaw(hw_pin, &adc_raw_value);
        if (status == E_OK)
        {
            *temperature_out = prv_ntc_adc_to_celsius(adc_raw_value);
            *humidity_out = 0.0f; // NTC does not provide humidity
            LOGD(TAG, "NTC (Pin %d) ADC raw: %u -> Temp: %.2f C", hw_pin, adc_raw_value, *temperature_out);
        }
        else
        {
            LOGE(TAG, "Failed to read NTC from ADC channel %d. Status: %d", hw_pin, status);
        }
        break;

    case SENSOR_TYPE_DHT11:
        status = dht_read_data(SENSOR_TYPE_DHT11, hw_pin, &temp_raw_value, &hum_raw_value);
        if (status == E_OK)
        {
            *temperature_out = (float)temp_raw_value;
            *humidity_out = (float)hum_raw_value;

            LOGD(TAG, "DHT11 (Pin %d) Temp: %.2f C, Hum: %.2f %%", hw_pin, *temperature_out, *humidity_out);
        }
        else
        {
            LOGE(TAG, "Failed to read DHT11 data from GPIO %d. Status: %d", hw_pin, status);
            *temperature_out = -999.0f; // Indicate error
            *humidity_out = -999.0f;    // Indicate error
        }
        break;

    case SENSOR_TYPE_DHT22:
        status = dht_read_data(SENSOR_TYPE_DHT22, hw_pin, &temp_raw_value, &hum_raw_value);
        if (status == E_OK)
        {
            *temperature_out = (float)temp_raw_value;
            *humidity_out = (float)hum_raw_value;
            LOGD(TAG, "DHT22 (Pin %d) Temp: %.2f C, Hum: %.2f %%", hw_pin, *temperature_out, *humidity_out);
        }
        else
        {
            LOGE(TAG, "Failed to read DHT22 data from GPIO %d. Status: %d", hw_pin, status);
            *temperature_out = -999.0f; // Indicate error
            *humidity_out = -999.0f;    // Indicate error
        }
        break;

    default:
        LOGE(TAG, "prv_read_sensor_data_from_hw: Unsupported sensor type %d for pin %d.", sensor_type, hw_pin);
        status = E_NOT_SUPPORTED;
        break;
    }
    return status;
}

// --- Private Helper Functions ---

/**
 * @brief Helper function to get the configuration for a given sensor ID.
 * @param sensor_id The unique identifier of the sensor.
 * @return Pointer to the sensor configuration item, or NULL if not found.
 */
static const temphum_sensor_config_item_t *prv_get_sensor_config(TempHum_Sensor_ID_t sensor_id)
{
    if (sensor_id >= TEMPHUM_SENSOR_ID_COUNT)
    {
        return NULL; // Invalid ID
    }
    for (size_t i = 0; i < TEMPHUM_SENSOR_ID_COUNT; i++)
    { // Loop over the fixed size array
        if (s_temphum_sensor_configurations[i].sensor_id == sensor_id)
        {
            return &s_temphum_sensor_configurations[i];
        }
    }
    return NULL; // Sensor ID not found in configurations
}

/**
 * @brief Internal function to perform a sensor read and update stored values.
 * This is called by TempHumCtrl_MainFunction for periodic updates.
 * Enhanced to also update averaging buffers.
 *
 * @param sensor_id The ID of the sensor to read and update.
 * @return Status_t E_OK if sensor read and update was successful, specific error code otherwise.
 */
static Status_t prv_perform_sensor_read_and_update(TempHum_Sensor_ID_t sensor_id)
{
    const temphum_sensor_config_item_t *sensor_cfg = prv_get_sensor_config(sensor_id);
    if (sensor_cfg == NULL)
    {
        LOGE(TAG, "Sensor ID %d configuration not found for read operation.", sensor_id);
        return E_NOT_FOUND;
    }

    float temp_val = 0.0f;
    float hum_val = 0.0f;

    // Read data from the hardware using the appropriate HAL/driver
    Status_t status = prv_read_sensor_data_from_hw(sensor_cfg->sensor_type, sensor_cfg->hw_pin, &temp_val, &hum_val);

    uint32_t current_time = prv_get_current_time_ms();

    if (status == E_OK)
    {
        s_current_temperatures[sensor_id] = temp_val;
        s_current_humidity[sensor_id] = hum_val;
        
        // Add to averaging buffers
        prv_add_to_average_buffer(&s_temp_avg_buffers[sensor_id], temp_val, current_time);
        
        // Only add humidity to buffer for sensors that support it
        if (sensor_cfg->sensor_type != SENSOR_TYPE_NTC_THERMISTOR) {
            prv_add_to_average_buffer(&s_hum_avg_buffers[sensor_id], hum_val, current_time);
        }
        
        LOGD(TAG, "Sensor ID %d (Type %d, Pin %d): Stored Temp=%.2f C, Hum=%.2f %%",
             sensor_id, sensor_cfg->sensor_type, sensor_cfg->hw_pin, temp_val, hum_val);
    }
    else
    {
        // On failure, store a clear error value (e.g., -999.0f) and log
        s_current_temperatures[sensor_id] = -999.0f;
        s_current_humidity[sensor_id] = -999.0f;
        LOGE(TAG, "Failed to read sensor ID %d (Type %d, Pin %d). Status: %d",
             sensor_id, sensor_cfg->sensor_type, sensor_cfg->hw_pin, status);
    }
    return status;
}

/**
 * @brief Checks the temperature and humidity of a specific sensor against its configured thresholds.
 *
 * This function updates the `s_SensorStatus` array for the given sensor ID.
 *
 * @param sensor_id The unique identifier of the sensor to check.
 * @return E_OK if the check was performed, E_INVALID_PARAM if `sensor_id` is invalid.
 */
static Status_t prv_check_tempHum_thresholds(TempHum_Sensor_ID_t sensor_id)
{
    if (sensor_id >= TEMPHUM_SENSOR_ID_COUNT)
    {
        LOGE(TAG, "Invalid Sensor ID %d for threshold check.", sensor_id);
        return E_INVALID_PARAM;
    }

    float current_temp = s_current_temperatures[sensor_id];
    float current_hum = s_current_humidity[sensor_id];
    const temphum_threshold_cfg_t *threshold_cfg = &s_temphum_threshold_configurations[sensor_id]; // Direct access as it's indexed by ID

    LOGD(TAG, "Checking thresholds for Sensor ID %d. Current Temp: %.2f C, Hum: %.2f %%", sensor_id, current_temp, current_hum);

    // --- Check Temperature Thresholds ---
    if (current_temp == -999.0f)
    {                                                                        // Check for read error
        s_SensorStatus[sensor_id].temp_status = TEMPHUM_STATUS_LEVEL_NORMAL; // Or define an error status
        LOGW(TAG, "Temp for Sensor ID %d has read error. Skipping temp threshold check.", sensor_id);
    }
    else if (current_temp > threshold_cfg->temp_threshold_above)
    {
        s_SensorStatus[sensor_id].temp_status = TEMPHUM_STATUS_LEVEL_HIGH;
        LOGW(TAG, "WARNING! Sensor ID %d (Temp: %.2f C) is above high temp threshold (%.2f C).",
             sensor_id, current_temp, threshold_cfg->temp_threshold_above);
    }
    else if (current_temp < threshold_cfg->temp_threshold_below)
    {
        s_SensorStatus[sensor_id].temp_status = TEMPHUM_STATUS_LEVEL_LOW;
        LOGW(TAG, "WARNING! Sensor ID %d (Temp: %.2f C) is below low temp threshold (%.2f C).",
             sensor_id, current_temp, threshold_cfg->temp_threshold_below);
    }
    else
    {
        s_SensorStatus[sensor_id].temp_status = TEMPHUM_STATUS_LEVEL_NORMAL;
        LOGD(TAG, "Sensor ID %d (Temp: %.2f C) is in Normal temp range.", sensor_id, current_temp);
    }

    // --- Check Humidity Thresholds ---
    const temphum_sensor_config_item_t *sensor_cfg = prv_get_sensor_config(sensor_id);
    if (sensor_cfg->sensor_type == SENSOR_TYPE_NTC_THERMISTOR)
    {
        s_SensorStatus[sensor_id].hum_status = TEMPHUM_STATUS_LEVEL_NORMAL; // NTC has no humidity
        LOGD(TAG, "Sensor ID %d (NTC) does not provide humidity. Skipping humidity threshold check.", sensor_id);
    }
    else if (current_hum == -999.0f)
    {                                                                       // Check for read error
        s_SensorStatus[sensor_id].hum_status = TEMPHUM_STATUS_LEVEL_NORMAL; // Or define an error status
        LOGW(TAG, "Hum for Sensor ID %d has read error. Skipping hum threshold check.", sensor_id);
    }
    else if (current_hum > threshold_cfg->hum_threshold_high)
    {
        s_SensorStatus[sensor_id].hum_status = TEMPHUM_STATUS_LEVEL_HIGH;
        LOGW(TAG, "WARNING! Sensor ID %d (Hum: %.2f %%) is above high hum threshold (%.2f %%).",
             sensor_id, current_hum, threshold_cfg->hum_threshold_high);
    }
    else if (current_hum < threshold_cfg->hum_threshold_low)
    {
        s_SensorStatus[sensor_id].hum_status = TEMPHUM_STATUS_LEVEL_LOW;
        LOGW(TAG, "WARNING! Sensor ID %d (Hum: %.2f %%) is below low hum threshold (%.2f %%).",
             sensor_id, current_hum, threshold_cfg->hum_threshold_low);
    }
    else
    {
        s_SensorStatus[sensor_id].hum_status = TEMPHUM_STATUS_LEVEL_NORMAL;
        LOGD(TAG, "Sensor ID %d (Hum: %.2f %%) is in Normal hum range.", sensor_id, current_hum);
    }
    return E_OK;
}

// --- Public Functions ---

/**
 * @brief Initializes the Temperature and Humidity Control (TempHumCtrl) module.
 *
 * This function initializes module's internal state variables and copies
 * static threshold configurations into the modifiable array.
 * Enhanced to initialize averaging buffers.
 * Note: It assumes underlying HALs (like ADC, GPIO for DHT) are initialized elsewhere.
 *
 * @return E_OK on successful initialization, or an error code otherwise.
 */
Status_t TempHumCtrl_Init(void)
{
    LOGI(TAG, "Initializing Temperature and Humidity Control module...");

    // Initialize current temperatures and humidity to 0.0f
    memset(s_current_temperatures, 0, sizeof(s_current_temperatures));
    memset(s_current_humidity, 0, sizeof(s_current_humidity));

    // Initialize averaging buffers
    for (size_t i = 0; i < TEMPHUM_SENSOR_ID_COUNT; i++) {
        prv_init_average_buffer(&s_temp_avg_buffers[i]);
        prv_init_average_buffer(&s_hum_avg_buffers[i]);
    }

    // Initialize current status levels for all sensors to NORMAL
    for (size_t i = 0; i < TEMPHUM_SENSOR_ID_COUNT; i++)
    {
        s_SensorStatus[i].sensor_id = (TempHum_Sensor_ID_t)i;
        s_SensorStatus[i].temp_status = TEMPHUM_STATUS_LEVEL_NORMAL;
        s_SensorStatus[i].hum_status = TEMPHUM_STATUS_LEVEL_NORMAL;
    }

    // Since s_temphum_threshold_configurations is now modifiable directly,
    // we don't need a separate s_modifiable_temphum_threshold_configurations array
    // unless we want to preserve the 'const' nature of the initial config.
    // For now, it's modified directly in temphum_ctrl_cfg.c.

    s_temphum_ctrl_initialized = true;
    LOGI(TAG, "Temperature and Humidity Control module initialized successfully.");
    return E_OK;
}

/**
 * @brief Main function for the Temperature and Humidity Control module.
 *
 * This function should be called periodically (e.g., in a main loop or RTOS task).
 * It iterates through all configured sensors, reads their data using the internal HAL,
 * updates their stored values, and then checks for temperature and humidity threshold breaches.
 */
void TempHumCtrl_MainFunction(void)
{
    if (!s_temphum_ctrl_initialized)
    {
        LOGE(TAG, "TempHumCtrl_MainFunction: Module not initialized. Skipping operations.");
        return;
    }

    LOGI(TAG, "TempHumCtrl_MainFunction: Performing periodic sensor reads and threshold checks for all sensors.");

    for (TempHum_Sensor_ID_t id = 0; id < TEMPHUM_SENSOR_ID_COUNT; id++)
    {
        // Step 1: Read sensor data and update internal stored values
        Status_t read_status = prv_perform_sensor_read_and_update(id);
        if (read_status != E_OK)
        {
            LOGW(TAG, "MainFunction: Failed to read and update data for Sensor ID %d. Status: %d", id, read_status);
            // Continue to next sensor even if one fails
        }

        // Step 2: Check temperature and humidity thresholds for the just-read sensor
        Status_t check_status = prv_check_tempHum_thresholds(id);
        if (check_status != E_OK)
        {
            LOGW(TAG, "MainFunction: Failed to check thresholds for Sensor ID %d. Status: %d", id, check_status);
        }
    }
}

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
Status_t TempHumCtrl_GetTemperature(TempHum_Sensor_ID_t sensor_id, float *temperature_out)
{
    if (!s_temphum_ctrl_initialized)
    {
        LOGE(TAG, "TempHumCtrl not initialized.");
        return E_NOT_INITIALIZED;
    }
    if (sensor_id >= TEMPHUM_SENSOR_ID_COUNT || temperature_out == NULL)
    {
        LOGE(TAG, "Invalid parameter for GetTemperature: Sensor ID %d, Out pointer %p",
             sensor_id, (void *)temperature_out);
        return E_INVALID_PARAM;
    }
    *temperature_out = s_current_temperatures[sensor_id];
    LOGD(TAG, "Retrieved temperature for Sensor ID %d: %.2f C", sensor_id, *temperature_out);
    return E_OK;
}

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
 * E_NOT_SUPPORTED if the sensor type does not provide humidity (e.g., NTC).
 */
Status_t TempHumCtrl_GetHumidity(TempHum_Sensor_ID_t sensor_id, float *humidity_out)
{
    if (!s_temphum_ctrl_initialized)
    {
        LOGE(TAG, "TempHumCtrl not initialized.");
        return E_NOT_INITIALIZED;
    }
    if (sensor_id >= TEMPHUM_SENSOR_ID_COUNT || humidity_out == NULL)
    {
        LOGE(TAG, "Invalid parameter for GetHumidity: Sensor ID %d, Out pointer %p",
             sensor_id, (void *)humidity_out);
        return E_INVALID_PARAM;
    }

    const temphum_sensor_config_item_t *sensor_cfg = prv_get_sensor_config(sensor_id);
    if (sensor_cfg == NULL)
    {
        LOGE(TAG, "Sensor ID %d not found in configuration.", sensor_id);
        return E_NOT_FOUND;
    }

    if (sensor_cfg->sensor_type == SENSOR_TYPE_NTC_THERMISTOR)
    {
        LOGW(TAG, "Sensor ID %d (NTC Thermistor) does not provide humidity data.", sensor_id);
        *humidity_out = 0.0f; // Indicate no humidity data
        return E_NOT_SUPPORTED;
    }

    *humidity_out = s_current_humidity[sensor_id];
    LOGD(TAG, "Retrieved humidity for Sensor ID %d: %.2f %%", sensor_id, *humidity_out);
    return E_OK;
}

/**
 * @brief Retrieves the current temperature status (LOW, NORMAL, HIGH) for a specific sensor.
 * @param sensor_id The unique identifier of the sensor.
 * @param temp_status_out Pointer to a TempHum_Status_Level_t where the status will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or `temp_status_out` is NULL.
 */
Status_t TempHumCtrl_GetTemperatureStatus(TempHum_Sensor_ID_t sensor_id, TempHum_Status_Level_t *temp_status_out)
{
    if (!s_temphum_ctrl_initialized)
    {
        LOGE(TAG, "TempHumCtrl not initialized.");
        return E_NOT_INITIALIZED;
    }
    if (sensor_id >= TEMPHUM_SENSOR_ID_COUNT || temp_status_out == NULL)
    {
        LOGE(TAG, "Invalid parameter for GetTemperatureStatus: Sensor ID %d, Out pointer %p",
             sensor_id, (void *)temp_status_out);
        return E_INVALID_PARAM;
    }
    *temp_status_out = s_SensorStatus[sensor_id].temp_status;
    LOGD(TAG, "Retrieved temp status for Sensor ID %d: %d", sensor_id, *temp_status_out);
    return E_OK;
}

/**
 * @brief Retrieves the current humidity status (LOW, NORMAL, HIGH) for a specific sensor.
 * @param sensor_id The unique identifier of the sensor.
 * @param hum_status_out Pointer to a TempHum_Status_Level_t where the status will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or `hum_status_out` is NULL,
 * E_NOT_SUPPORTED if the sensor type does not provide humidity.
 */
Status_t TempHumCtrl_GetHumidityStatus(TempHum_Sensor_ID_t sensor_id, TempHum_Status_Level_t *hum_status_out)
{
    if (!s_temphum_ctrl_initialized)
    {
        LOGE(TAG, "TempHumCtrl not initialized.");
        return E_NOT_INITIALIZED;
    }
    if (sensor_id >= TEMPHUM_SENSOR_ID_COUNT || hum_status_out == NULL)
    {
        LOGE(TAG, "Invalid parameter for GetHumidityStatus: Sensor ID %d, Out pointer %p",
             sensor_id, (void *)hum_status_out);
        return E_INVALID_PARAM;
    }

    const temphum_sensor_config_item_t *sensor_cfg = prv_get_sensor_config(sensor_id);
    if (sensor_cfg == NULL)
    {
        LOGE(TAG, "Sensor ID %d not found in configuration for GetHumidityStatus.", sensor_id);
        return E_NOT_FOUND;
    }
    if (sensor_cfg->sensor_type == SENSOR_TYPE_NTC_THERMISTOR)
    {
        LOGW(TAG, "Sensor ID %d (NTC Thermistor) does not provide humidity data for status.", sensor_id);
        *hum_status_out = TEMPHUM_STATUS_LEVEL_NORMAL; // Default status for NTC humidity
        return E_NOT_SUPPORTED;
    }

    *hum_status_out = s_SensorStatus[sensor_id].hum_status;
    LOGD(TAG, "Retrieved hum status for Sensor ID %d: %d", sensor_id, *hum_status_out);
    return E_OK;
}

/**
 * @brief Retrieves the configured temperature threshold values (above and below) for a specific sensor.
 *
 * @param sensor_id The unique identifier of the sensor.
 * @param above_threshold_out Pointer to a float where the above temperature threshold will be stored. Must not be NULL.
 * @param below_threshold_out Pointer to a float where the below temperature threshold will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or output pointers are NULL.
 */
Status_t TempHumCtrl_GetTemperatureThreshold(TempHum_Sensor_ID_t sensor_id, float *above_threshold_out, float *below_threshold_out)
{
    if (sensor_id >= TEMPHUM_SENSOR_ID_COUNT || above_threshold_out == NULL || below_threshold_out == NULL)
    {
        LOGE(TAG, "Invalid parameter for GetTemperatureThreshold: Sensor ID %d, Above Out %p, Below Out %p",
             sensor_id, (void *)above_threshold_out, (void *)below_threshold_out);
        return E_INVALID_PARAM;
    }

    *above_threshold_out = s_temphum_threshold_configurations[sensor_id].temp_threshold_above;
    *below_threshold_out = s_temphum_threshold_configurations[sensor_id].temp_threshold_below;

    LOGD(TAG, "Retrieved Temp Thresholds for Sensor ID %d: Above=%.2f C, Below=%.2f C",
         sensor_id, *above_threshold_out, *below_threshold_out);
    return E_OK;
}

/**
 * @brief Retrieves the configured humidity threshold values (high and low) for a specific sensor.
 *
 * @param sensor_id The unique identifier of the sensor.
 * @param high_threshold_out Pointer to a float where the high humidity threshold will be stored. Must not be NULL.
 * @param low_threshold_out Pointer to a float where the low humidity threshold will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or output pointers are NULL.
 * E_NOT_SUPPORTED if the sensor type does not provide humidity.
 */
Status_t TempHumCtrl_GetHumidityThreshold(TempHum_Sensor_ID_t sensor_id, float *high_threshold_out, float *low_threshold_out)
{
    if (sensor_id >= TEMPHUM_SENSOR_ID_COUNT || high_threshold_out == NULL || low_threshold_out == NULL)
    {
        LOGE(TAG, "Invalid parameter for GetHumidityThreshold: Sensor ID %d, High Out %p, Low Out %p",
             sensor_id, (void *)high_threshold_out, (void *)low_threshold_out);
        return E_INVALID_PARAM;
    }

    const temphum_sensor_config_item_t *sensor_cfg = prv_get_sensor_config(sensor_id);
    if (sensor_cfg == NULL)
    {
        LOGE(TAG, "Sensor ID %d not found in configuration for GetHumidityThreshold.", sensor_id);
        return E_NOT_FOUND;
    }
    if (sensor_cfg->sensor_type == SENSOR_TYPE_NTC_THERMISTOR)
    {
        LOGW(TAG, "Sensor ID %d (NTC Thermistor) does not provide humidity thresholds.", sensor_id);
        *high_threshold_out = 0.0f; // Indicate N/A
        *low_threshold_out = 0.0f;  // Indicate N/A
        return E_NOT_SUPPORTED;
    }

    *high_threshold_out = s_temphum_threshold_configurations[sensor_id].hum_threshold_high;
    *low_threshold_out = s_temphum_threshold_configurations[sensor_id].hum_threshold_low;

    LOGD(TAG, "Retrieved Hum Thresholds for Sensor ID %d: High=%.2f %%, Low=%.2f %%",
         sensor_id, *high_threshold_out, *low_threshold_out);
    return E_OK;
}

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
 * @return E_OK on success, E_INVALID_PARAM if `sensor_id` is invalid or threshold values are illogical (e.g., above <= below).
 */
Status_t TempHumCtrl_SetTemperatureThreshold(TempHum_Sensor_ID_t sensor_id, float new_above_threshold_celsius, float new_below_threshold_celsius)
{
    if (sensor_id >= TEMPHUM_SENSOR_ID_COUNT)
    {
        LOGE(TAG, "Invalid Sensor ID %d for SetTemperatureThreshold.", sensor_id);
        return E_INVALID_PARAM;
    }
    // Basic validation: above threshold should be greater than below threshold
    if (new_above_threshold_celsius <= new_below_threshold_celsius)
    {
        LOGE(TAG, "Invalid temperature thresholds for Sensor ID %d: Above (%.2f) must be greater than Below (%.2f).",
             sensor_id, new_above_threshold_celsius, new_below_threshold_celsius);
        return E_INVALID_PARAM;
    }

    float old_above = s_temphum_threshold_configurations[sensor_id].temp_threshold_above;
    float old_below = s_temphum_threshold_configurations[sensor_id].temp_threshold_below;

    s_temphum_threshold_configurations[sensor_id].temp_threshold_above = new_above_threshold_celsius;
    s_temphum_threshold_configurations[sensor_id].temp_threshold_below = new_below_threshold_celsius;

    LOGI(TAG, "Temp Thresholds for Sensor ID %d updated: Above (%.2f->%.2f C), Below (%.2f->%.2f C)",
         sensor_id, old_above, new_above_threshold_celsius, old_below, new_below_threshold_celsius);
    return E_OK;
}

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
Status_t TempHumCtrl_SetHumidityThreshold(TempHum_Sensor_ID_t sensor_id, float new_high_threshold_humidity, float new_low_threshold_humidity)
{
    if (sensor_id >= TEMPHUM_SENSOR_ID_COUNT)
    {
        LOGE(TAG, "Invalid Sensor ID %d for SetHumidityThreshold.", sensor_id);
        return E_INVALID_PARAM;
    }

    const temphum_sensor_config_item_t *sensor_cfg = prv_get_sensor_config(sensor_id);
    if (sensor_cfg == NULL)
    {
        LOGE(TAG, "Sensor ID %d not found in configuration for SetHumidityThreshold.", sensor_id);
        return E_NOT_FOUND;
    }
    if (sensor_cfg->sensor_type == SENSOR_TYPE_NTC_THERMISTOR)
    {
        LOGW(TAG, "Sensor ID %d (NTC Thermistor) does not support humidity thresholds.", sensor_id);
        return E_NOT_SUPPORTED;
    }

    // Basic validation: high threshold should be greater than low threshold
    if (new_high_threshold_humidity <= new_low_threshold_humidity)
    {
        LOGE(TAG, "Invalid humidity thresholds for Sensor ID %d: High (%.2f) must be greater than Low (%.2f).",
             sensor_id, new_high_threshold_humidity, new_low_threshold_humidity);
        return E_INVALID_PARAM;
    }
    // Also, humidity values should typically be between 0 and 100
    if (new_high_threshold_humidity > 100.0f || new_low_threshold_humidity < 0.0f)
    {
        LOGE(TAG, "Humidity thresholds for Sensor ID %d out of valid range (0-100%%): High=%.2f, Low=%.2f.",
             sensor_id, new_high_threshold_humidity, new_low_threshold_humidity);
        return E_INVALID_PARAM;
    }

    float old_high = s_temphum_threshold_configurations[sensor_id].hum_threshold_high;
    float old_low = s_temphum_threshold_configurations[sensor_id].hum_threshold_low;

    s_temphum_threshold_configurations[sensor_id].hum_threshold_high = new_high_threshold_humidity;
    s_temphum_threshold_configurations[sensor_id].hum_threshold_low = new_low_threshold_humidity;

    LOGI(TAG, "Hum Thresholds for Sensor ID %d updated: High (%.2f->%.2f %%), Low (%.2f->%.2f %%)",
         sensor_id, old_high, new_high_threshold_humidity, old_low, new_low_threshold_humidity);
    return E_OK;
}

/**
 * @brief Retrieves the calculated average temperature value for a specific sensor.
 * The 'Status_t' return indicates the validity/freshness of the returned average.
 * @param sensor_id The ID of the sensor.
 * @param average_temperature_out Pointer to a float variable where the average temperature (in Celsius) will be stored.
 * @return Status_t E_OK if average is valid, E_DATA_STALE if data used for average is stale,
 * E_INVALID_PARAM if sensor_id is invalid, E_NOT_INITIALIZED or E_NULL_POINTER.
 */
Status_t TempHumCtrl_GetAverageTemperature(TempHum_Sensor_ID_t sensor_id, float *average_temperature_out)
{
    if (!s_temphum_ctrl_initialized)
    {
        LOGE(TAG, "TempHumCtrl not initialized.");
        return E_NOT_INITIALIZED;
    }
    
    if (sensor_id >= TEMPHUM_SENSOR_ID_COUNT)
    {
        LOGE(TAG, "Invalid sensor ID %d for GetAverageTemperature.", sensor_id);
        return E_INVALID_PARAM;
    }
    
    if (average_temperature_out == NULL)
    {
        LOGE(TAG, "NULL pointer provided for average_temperature_out.");
        return E_NULL_POINTER;
    }

    uint32_t current_time = prv_get_current_time_ms();
    Status_t status = prv_calculate_average_from_buffer(&s_temp_avg_buffers[sensor_id], current_time, average_temperature_out);
    
    if (status == E_OK)
    {
        LOGD(TAG, "Retrieved average temperature for Sensor ID %d: %.2f C", sensor_id, *average_temperature_out);
    }
    else if (status == E_DATA_STALE)
    {
        LOGW(TAG, "Average temperature data for Sensor ID %d is stale or insufficient.", sensor_id);
        *average_temperature_out = 0.0f; // Set to a default value
    }
    
    return status;
}

/**
 * @brief Retrieves the calculated average humidity value for a specific sensor.
 * The 'Status_t' return indicates the validity/freshness of the returned average.
 * @param sensor_id The ID of the sensor.
 * @param average_humidity_out Pointer to a float variable where the average humidity (in percentage) will be stored.
 * @return Status_t E_OK if average is valid, E_DATA_STALE if data used for average is stale,
 * E_INVALID_PARAM if sensor_id is invalid, E_NOT_SUPPORTED if sensor type does not provide humidity,
 * E_NOT_INITIALIZED or E_NULL_POINTER.
 */
Status_t TempHumCtrl_GetAverageHumidity(TempHum_Sensor_ID_t sensor_id, float *average_humidity_out)
{
    if (!s_temphum_ctrl_initialized)
    {
        LOGE(TAG, "TempHumCtrl not initialized.");
        return E_NOT_INITIALIZED;
    }
    
    if (sensor_id >= TEMPHUM_SENSOR_ID_COUNT)
    {
        LOGE(TAG, "Invalid sensor ID %d for GetAverageHumidity.", sensor_id);
        return E_INVALID_PARAM;
    }
    
    if (average_humidity_out == NULL)
    {
        LOGE(TAG, "NULL pointer provided for average_humidity_out.");
        return E_NULL_POINTER;
    }

    const temphum_sensor_config_item_t *sensor_cfg = prv_get_sensor_config(sensor_id);
    if (sensor_cfg == NULL)
    {
        LOGE(TAG, "Sensor ID %d not found in configuration for GetAverageHumidity.", sensor_id);
        return E_NOT_FOUND;
    }

    if (sensor_cfg->sensor_type == SENSOR_TYPE_NTC_THERMISTOR)
    {
        LOGW(TAG, "Sensor ID %d (NTC Thermistor) does not provide humidity data for averaging.", sensor_id);
        *average_humidity_out = 0.0f;
        return E_NOT_SUPPORTED;
    }

    uint32_t current_time = prv_get_current_time_ms();
    Status_t status = prv_calculate_average_from_buffer(&s_hum_avg_buffers[sensor_id], current_time, average_humidity_out);
    
    if (status == E_OK)
    {
        LOGD(TAG, "Retrieved average humidity for Sensor ID %d: %.2f %%", sensor_id, *average_humidity_out);
    }
    else if (status == E_DATA_STALE)
    {
        LOGW(TAG, "Average humidity data for Sensor ID %d is stale or insufficient.", sensor_id);
        *average_humidity_out = 0.0f; // Set to a default value
    }
    
    return status;
}

/**
 * @brief Retrieves the total number of configured sensors in the system.
 * This function provides information about how many sensors are configured,
 * which can be useful for system diagnostics and iterating through all sensors.
 * 
 * @param sensor_count_out Pointer to a uint8_t variable where the sensor count will be stored.
 * @return Status_t E_OK on success, E_NULL_POINTER if output pointer is NULL, 
 * E_NOT_INITIALIZED if module is not initialized.
 */
Status_t TempHumCtrl_GetConfiguredSensorCount(uint8_t *sensor_count_out)
{
    if (!s_temphum_ctrl_initialized)
    {
        LOGE(TAG, "TempHumCtrl not initialized.");
        return E_NOT_INITIALIZED;
    }
    
    if (sensor_count_out == NULL)
    {
        LOGE(TAG, "NULL pointer provided for sensor_count_out.");
        return E_NULL_POINTER;
    }

    *sensor_count_out = TEMPHUM_SENSOR_ID_COUNT;
    LOGD(TAG, "Retrieved configured sensor count: %d", *sensor_count_out);
    return E_OK;
}



/**
 * @brief Retrieves the calculated average temperature across all configured sensors in the system.
 * 
 * This function calculates the system-wide average temperature by reading the current temperature
 * from all configured sensors and dividing by the total number of sensors that provide valid temperature readings.
 * Sensors with invalid readings (-999.0f) are excluded from the calculation.
 * 
 * @param system_average_temperature_out Pointer to a float variable where the system average temperature (in Celsius) will be stored.
 * @return Status_t:
 *         - E_OK if average is calculated successfully with at least one valid sensor reading
 *         - E_DATA_STALE if no sensors have valid temperature readings
 *         - E_NULL_POINTER if system_average_temperature_out is NULL
 *         - E_NOT_INITIALIZED if module is not initialized
 */
Status_t prv_GetSystemAverageTemperature(void)
{
    if (!s_temphum_ctrl_initialized)
    {
        LOGE(TAG, "TempHumCtrl not initialized.");
        return E_NOT_INITIALIZED;
    }

    float temperature_sum = 0.0f;
    uint8_t valid_sensor_count = 0;
    
    // Iterate through all configured sensors
    for (TempHum_Sensor_ID_t sensor_id = 0; sensor_id < TEMPHUM_SENSOR_ID_COUNT; sensor_id++)
    {
        float current_temp = s_current_temperatures[sensor_id];
        
        // Check if the temperature reading is valid (not an error value)
        if (current_temp != -999.0f)
        {
            temperature_sum += current_temp;
            valid_sensor_count++;
            LOGD(TAG, "System Avg Temp: Including Sensor ID %d (Temp: %.2f C)", sensor_id, current_temp);
        }
        else
        {
            LOGW(TAG, "System Avg Temp: Excluding Sensor ID %d due to invalid reading", sensor_id);
        }
    }
    
    // Check if we have at least one valid reading
    if (valid_sensor_count == 0)
    {
        LOGE(TAG, "System Avg Temp: No valid temperature readings available from any sensor");
        f32AvgSysTemp = 0.0f;
        return E_DATA_STALE;
    }
    
    // Calculate the average
    f32AvgSysTemp = temperature_sum / valid_sensor_count;
    
    LOGI(TAG, "System Average Temperature: %.2f C (from %d valid sensors out of %d total)", 
         f32AvgSysTemp, valid_sensor_count, TEMPHUM_SENSOR_ID_COUNT);
    
    return E_OK;
}


/**
 * @brief Retrieves the calculated average humidity across all configured sensors that support humidity.
 * 
 * This function calculates the system-wide average humidity by reading the current humidity
 * from all configured sensors that support humidity (excludes NTC thermistors) and dividing by 
 * the total number of humidity-capable sensors with valid readings.
 * Sensors with invalid readings (-999.0f) are excluded from the calculation.
 * 
 * @param system_average_humidity_out Pointer to a float variable where the system average humidity (in percentage) will be stored.
 * @return Status_t:
 *         - E_OK if average is calculated successfully with at least one valid humidity sensor reading
 *         - E_DATA_STALE if no humidity sensors have valid readings
 *         - E_NOT_SUPPORTED if no sensors in the system support humidity measurements
 *         - E_NULL_POINTER if system_average_humidity_out is NULL
 *         - E_NOT_INITIALIZED if module is not initialized
 */

Status_t TempHumCtrl_GetSystemAverageTemperature(float *system_average_temperature_out)
{
    if (!s_temphum_ctrl_initialized)
    {
        LOGE(TAG, "TempHumCtrl not initialized.");
        return E_NOT_INITIALIZED;
    }
    
    if (system_average_temperature_out == NULL)
    {
        LOGE(TAG, "NULL pointer provided for system_average_humidity_out.");
        return E_NULL_POINTER;
    }
    *system_average_temperature_out = f32AvgSysTemp;
    return E_OK;
}


Status_t TempHumCtrl_GetSystemAverageHumidity(float *system_average_humidity_out)
{
    if (!s_temphum_ctrl_initialized)
    {
        LOGE(TAG, "TempHumCtrl not initialized.");
        return E_NOT_INITIALIZED;
    }
    
    if (system_average_humidity_out == NULL)
    {
        LOGE(TAG, "NULL pointer provided for system_average_humidity_out.");
        return E_NULL_POINTER;
    }
    *system_average_humidity_out = f32AvgSysHumidity;
    return E_OK;
}



Status_t prv_GetSystemAverageHumidity(void)
{


    float humidity_sum = 0.0f;
    uint8_t valid_humidity_sensor_count = 0;
    uint8_t total_humidity_capable_sensors = 0;
    
    // Iterate through all configured sensors
    for (TempHum_Sensor_ID_t sensor_id = 0; sensor_id < TEMPHUM_SENSOR_ID_COUNT; sensor_id++)
    {
        const temphum_sensor_config_item_t *sensor_cfg = prv_get_sensor_config(sensor_id);
        if (sensor_cfg == NULL)
        {
            LOGW(TAG, "System Avg Humidity: Sensor ID %d configuration not found", sensor_id);
            continue;
        }
        
        // Skip sensors that don't support humidity (NTC thermistors)
        if (sensor_cfg->sensor_type == SENSOR_TYPE_NTC_THERMISTOR)
        {
            LOGD(TAG, "System Avg Humidity: Skipping Sensor ID %d (NTC - no humidity)", sensor_id);
            continue;
        }
        
        total_humidity_capable_sensors++;
        float current_hum = s_current_humidity[sensor_id];
        
        // Check if the humidity reading is valid (not an error value)
        if (current_hum != -999.0f)
        {
            humidity_sum += current_hum;
            valid_humidity_sensor_count++;
            LOGD(TAG, "System Avg Humidity: Including Sensor ID %d (Humidity: %.2f %%)", sensor_id, current_hum);
        }
        else
        {
            LOGW(TAG, "System Avg Humidity: Excluding Sensor ID %d due to invalid reading", sensor_id);
        }
    }
    
    // Check if the system has any humidity-capable sensors
    if (total_humidity_capable_sensors == 0)
    {
        LOGW(TAG, "System Avg Humidity: No humidity-capable sensors found in system configuration");
        f32AvgSysHumidity = 0.0f;
        return E_NOT_SUPPORTED;
    }
    
    // Check if we have at least one valid humidity reading
    if (valid_humidity_sensor_count == 0)
    {
        LOGE(TAG, "System Avg Humidity: No valid humidity readings available from any humidity sensor");
        f32AvgSysHumidity = 0.0f;
        return E_DATA_STALE;
    }
    
    // Calculate the average
    f32AvgSysHumidity = humidity_sum / valid_humidity_sensor_count;
    
    LOGI(TAG, "System Average Humidity: %.2f %% (from %d valid sensors out of %d humidity-capable sensors)", 
         f32AvgSysHumidity, valid_humidity_sensor_count, total_humidity_capable_sensors);
    
    return E_OK;
}