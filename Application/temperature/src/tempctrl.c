#include "tempctrl.h"
#include "tempctrl_cfg.h"
#include "common.h"
#include "logger.h"
#include "system_monitor.h"

#include "hal_adc.h"
#include "hal_i2c.h"

#include <math.h>

/**
 * @file tempctrl.c
 * @brief Implementation for the TempCtrl (Temperature Control) component.
 *
 * This file contains the core logic for initializing, reading, and managing
 * temperature sensor data based on the configuration provided in tempctrl_cfg.c/.h.
 */

// --- Internal State Variables ---

// Array to store the latest valid temperature reading for each sensor
static float s_latest_temperatures_c[TempCtrl_SENSOR_COUNT];
// Array to store the timestamp of the last successful read for each sensor
static uint32_t s_last_read_timestamps_ms[TempCtrl_SENSOR_COUNT];
// Module initialization status
static bool s_is_initialized = false;

// --- Private Helper Function Prototypes ---
static Status_t TempCtrl_ReadAndConvertSensor(uint32_t sensorId, float *temperature_c);

// --- Public Function Implementations ---

Status_t TempCtrl_Init(void) 
{
    // if (s_is_initialized) {
    //     return E_OK; // Already initialized
    // }

    // // Initialize internal state variables to a known state
    // for (uint32_t i = 0; i < TempCtrl_SENSOR_COUNT; i++) {
    //     s_latest_temperatures_c[i] = NAN; // No valid data yet
    //     s_last_read_timestamps_ms[i] = 0;
    // }

    // // Initialize communication interfaces for each configured sensor
    // for (uint32_t i = 0; i < TempCtrl_SENSOR_COUNT; i++) {
    //     const TempCtrl_SensorConfig_t* config = &tempctrl_sensor_configs[i];
    //     Status_t status = E_NOK;

    //     switch (config->type) {
    //         case TempCtrl_SENSOR_TYPE_ANALOG_ADC:
    //             status = MCAL_ADC_Init(config->comm_details.analog_adc.adc_channel);
    //             break;
    //         case TempCtrl_SENSOR_TYPE_I2C:
    //             status = MCAL_I2C_Init(config->comm_details.i2c.i2c_port);
    //             break;
    //         case TempCtrl_SENSOR_TYPE_1WIRE_DS18B20:
    //             status = HAL_1WIRE_Init(config->comm_details.one_wire.one_wire_bus_id);
    //             break;
    //         default:
    //             LOGE("TempCtrl: Unknown sensor type for sensor ID %lu", config->id);
    //             SysMon_ReportFault(FAULT_ID_TEMP_SENSOR_INIT_FAILURE, SEVERITY_HIGH, config->id);
    //             return E_NOK;
    //     }

    //     if (status != E_OK) {
    //         LOGE("TempCtrl: Initialization failed for sensor ID %lu", config->id);
    //         SysMon_ReportFault(FAULT_ID_TEMP_SENSOR_INIT_FAILURE, SEVERITY_HIGH, config->id);
    //         return E_NOK;
    //     }
    // }

    // s_is_initialized = true;
    // LOGI("TempCtrl: Module initialized successfully.");
    return E_OK;
}

void TempCtrl_MainFunction(void) 
{
    // if (!s_is_initialized) {
    //     return;
    // }

    // for (uint32_t i = 0; i < TempCtrl_SENSOR_COUNT; i++) {
    //     // const TempCtrl_SensorConfig_t* config = &tempctrl_sensor_configs[i];
    //     float temperature_c = NAN;
    //     bool read_success = false;

    //     for (uint32_t retry = 0; retry < TempCtrl_READ_RETRIES; retry++) {
    //         if (TempCtrl_ReadAndConvertSensor(i, &temperature_c) == E_OK) {
    //             // If the read was successful, validate and update the internal state
    //             if (isfinite(temperature_c) && temperature_c >= TempCtrl_MIN_VALID_C && temperature_c <= TempCtrl_MAX_VALID_C) {
    //                 s_latest_temperatures_c[i] = temperature_c;
    //                 s_last_read_timestamps_ms[i] = APP_GetUptimeMs();
    //                 read_success = true;
    //                 LOGD("TempCtrl sensor ID %lu read success: %.2f C", config->id, temperature_c);
    //                 break; // Exit retry loop on success
    //             } else {
    //                 LOGW("TempCtrl sensor ID %lu read success but value %.2f C is out of range", config->id, temperature_c);
    //                 SysMon_ReportFault(FAULT_ID_TEMP_SENSOR_OUT_OF_RANGE, SEVERITY_MEDIUM, config->id);
    //             }
    //         } else {
    //             LOGW("TempCtrl sensor ID %lu read failed, retry %lu/%lu", config->id, retry + 1, TempCtrl_READ_RETRIES);
    //             // Delay between retries
    //             if (TempCtrl_READ_RETRY_DELAY_MS > 0) {
    //                 SERVICE_OS_DelayMs(TempCtrl_READ_RETRY_DELAY_MS);
    //             }
    //         }
    //     }

    //     if (!read_success) {
    //         // All retries failed
    //         SysMon_ReportFault(FAULT_ID_TEMP_SENSOR_COMM_ERROR, SEVERITY_HIGH, config->id);
    //         s_latest_temperatures_c[i] = NAN; // Invalidate the stored data
    //         LOGE("TempCtrl sensor ID %lu failed to read after all retries.", config->id);
    //     }
    // }
}

Status_t TempCtrl_GetSensorTemp(uint32_t sensorId, float *temperature_c) 
{
    // if (temperature_c == NULL) {
    //     LOGE("TempCtrl_GetSensorTemp: NULL pointer provided.");
    //     return E_NOK;
    // }

    // if (sensorId >= TempCtrl_SENSOR_COUNT) {
    //     LOGE("TempCtrl_GetSensorTemp: Invalid sensorId %lu", sensorId);
    //     return E_NOK;
    // }

    // // Check if the stored value is valid
    // if (isnan(s_latest_temperatures_c[sensorId])) {
    //     return E_NOK;
    // }

    // *temperature_c = s_latest_temperatures_c[sensorId];
    return E_OK;
}

// --- Private Helper Function Implementations ---

/**
 * @brief Reads the raw value from a single sensor based on its configuration and applies calibration.
 *
 * @param sensorId The index of the sensor to read.
 * @param temperature_c Pointer to store the converted temperature reading.
 * @return E_OK on success, E_NOK on failure.
 */
static Status_t TempCtrl_ReadAndConvertSensor(uint32_t sensorId, float *temperature_c) 
{
    // const TempCtrl_SensorConfig_t* config = &tempctrl_sensor_configs[sensorId];
    // uint32_t raw_data = 0; // Or other raw data type
    // Status_t status = E_NOK;
    
    // // Perform sensor-specific reading
    // switch (config->type) {
    //     case TempCtrl_SENSOR_TYPE_ANALOG_ADC:
    //         status = MCAL_ADC_Read(config->comm_details.analog_adc.adc_channel, &raw_data);
    //         break;
    //     case TempCtrl_SENSOR_TYPE_I2C:
    //         // I2C read logic would be more complex, likely a master read/write sequence
    //         // This is a simplified placeholder
    //         status = MCAL_I2C_MasterRead(config->comm_details.i2c.i2c_port, config->comm_details.i2c.i2c_address, (uint8_t*)&raw_data, sizeof(raw_data));
    //         break;
    //     case TempCtrl_SENSOR_TYPE_1WIRE_DS18B20:
    //         // 1-Wire read logic would involve a series of commands
    //         // This is a simplified placeholder
    //         status = HAL_1WIRE_ReadDevice(config->comm_details.one_wire.one_wire_bus_id, config->comm_details.one_wire.ds18b20_rom_address, (uint8_t*)&raw_data);
    //         break;
    //     default:
    //         LOGE("TempCtrl: ReadAndConvertSensor failed, unknown sensor type for ID %lu", config->id);
    //         return E_NOK;
    // }

    // if (status != E_OK) {
    //     return status;
    // }
    
    // // Apply calibration and conversion
    // *temperature_c = (raw_data * config->calibration_slope) + config->calibration_offset;
    
    return E_OK;
}