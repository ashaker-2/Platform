#include "humctrl.h"
#include "humctrl_cfg.h"
#include "logger.h"
#include "system_monitor.h"
// #include "Rte.h"
// #include "hal_adc.h"
// #include "hal_i2c.h"
#include <string.h>
#include <math.h> // For NAN

/**
 * @file humctrl.c
 * @brief Implementation for the HumCtrl (Humidity Control) component.
 *
 * This file contains the core logic for managing multiple humidity sensors,
 * including periodic reading, data validation, and conversion.
 */

// --- Internal State Variables ---
static float s_latest_humidities_rh[HUMCTRL_SENSOR_COUNT];
static bool s_is_initialized = false;

// --- Private Helper Function Prototypes ---
static Status_t HumCtrl_ReadSensor(const HumCtrl_Config_t* config, float* humidity_rh);
static Status_t HumCtrl_ReadAnalogSensor(const HumCtrl_Config_t* config, float* humidity_rh);

// --- Public Function Implementations ---

Status_t HumCtrl_Init(void) 
{
    // if (s_is_initialized) 
    // {
    //     return E_OK;
    // }

    // // Initialize internal state variables to a known safe state
    // for (uint32_t i = 0; i < HUMCTRL_SENSOR_COUNT; i++) {
    //     s_latest_humidities_rh[i] = NAN; // No valid data yet
    // }

    // // Initialize hardware for each configured sensor
    // for (uint32_t i = 0; i < HUMCTRL_SENSOR_COUNT; i++) {
    //     const HumCtrl_Config_t* config = &humctrl_sensor_configs[i];
    //     Status_t status = E_NOK;

    //     switch (config->type) {
    //         case HUMCTRL_SENSOR_TYPE_ANALOG_ADC:
    //             status = MCAL_ADC_Init(config->interface_details.analog_adc.adc_channel);
    //             break;
    //         case HUMCTRL_SENSOR_TYPE_I2C:
    //             status = MCAL_I2C_Init(config->interface_details.i2c.i2c_port);
    //             break;
    //         default:
    //             LOGE("HumCtrl: Unknown sensor type for ID %lu", config->id);
    //             RTE_Service_SystemMonitor_ReportFault(FAULT_ID_HUMCTRL_SENSOR_INIT_FAILED, SEVERITY_HIGH, config->id);
    //             return E_NOK;
    //     }

    //     if (status != E_OK) {
    //         LOGE("HumCtrl: Sensor interface init failed for ID %lu", config->id);
    //         RTE_Service_SystemMonitor_ReportFault(FAULT_ID_HUMCTRL_SENSOR_INIT_FAILED, SEVERITY_HIGH, config->id);
    //         return E_NOK;
    //     }
    // }

    // s_is_initialized = true;
    // LOGI("HumCtrl: Module initialized successfully.");
    return E_OK;
}

Status_t HumCtrl_GetSensorHumidity(uint32_t sensorId, float *humidity_rh) 
{
    // if (!s_is_initialized || humidity_rh == NULL) 
    // {
    //     return E_NOK;
    // }
    // if (sensorId >= HUMCTRL_SENSOR_COUNT)
    // {
    //     LOGE("HumCtrl: Invalid sensorId %lu", sensorId);
    //     return E_NOK;
    // }
    // if (isnan(s_latest_humidities_rh[sensorId])) {
    //     LOGW("HumCtrl: No valid data available for sensor %lu", sensorId);
    //     return E_NOK;
    // }
    
    // *humidity_rh = s_latest_humidities_rh[sensorId];

    return E_OK;
}

void HumCtrl_MainFunction(void) 
{
    // if (!s_is_initialized) 
    // {
    //     return;
    // }
    
    // for (uint32_t i = 0; i < HUMCTRL_SENSOR_COUNT; i++) {
    //     const HumCtrl_Config_t* config = &humctrl_sensor_configs[i];
    //     float current_humidity = NAN;
        
    //     // Read from sensor and convert
    //     if (HumCtrl_ReadSensor(config, &current_humidity) != E_OK) {
    //         LOGW("HumCtrl: Failed to read sensor ID %lu", config->id);
    //         // Fault reporting for persistent failures should be handled by a retry mechanism
    //         // For now, we'll just report a medium-severity fault on any read failure
    //         RTE_Service_SystemMonitor_ReportFault(FAULT_ID_HUMCTRL_SENSOR_READ_FAILURE, SEVERITY_MEDIUM, config->id);
    //     } else {
    //         // Validation: check for plausible humidity range (0-100%)
    //         if (current_humidity >= 0.0f && current_humidity <= 100.0f) {
    //             s_latest_humidities_rh[i] = current_humidity;
    //             LOGD("HumCtrl: Sensor %lu read: %.1f%% RH", config->id, current_humidity);
    //         } else {
    //             LOGE("HumCtrl: Invalid humidity value %.1f%% RH from sensor ID %lu", current_humidity, config->id);
    //             RTE_Service_SystemMonitor_ReportFault(FAULT_ID_HUMCTRL_SENSOR_DATA_INVALID, SEVERITY_HIGH, config->id);
    //         }
    //     }
    // }
}

// --- Private Helper Function Implementations ---

static Status_t HumCtrl_ReadSensor(const HumCtrl_Config_t* config, float* humidity_rh) 
{
    // switch (config->type) 
    // {
    //     case HUMCTRL_SENSOR_TYPE_ANALOG_ADC:
    //         return HumCtrl_ReadAnalogSensor(config, humidity_rh);
    //     case HUMCTRL_SENSOR_TYPE_I2C:
    //         // Placeholder for a real I2C sensor read
    //         LOGW("HumCtrl: I2C sensor read not implemented for ID %lu", config->id);
    //         return E_NOK;
    //     default:
    //         return E_NOK;
    // }
}

static Status_t HumCtrl_ReadAnalogSensor(const HumCtrl_Config_t* config, float* humidity_rh) 
{
    // uint32_t raw_data;
    // if (MCAL_ADC_Read(config->interface_details.analog_adc.adc_channel, &raw_data) != E_OK) 
    // {
    //     return E_NOK;
    // }

    // // Simplified conversion from raw ADC to RH based on the configuration
    // float voltage = (float)raw_data * config->interface_details.analog_adc.voltage_scale_factor;
    // *humidity_rh = (voltage * config->calibration.voltage_to_rh_slope) + config->calibration.voltage_to_rh_offset;

    return E_OK;
}