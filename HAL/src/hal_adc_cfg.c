/**
 * @file hal_adc_cfg.c
 * @brief Configuration definitions for the HAL ADC component.
 *
 * This file contains the actual definitions of the configuration data
 * for the HAL ADC module, mapping logical ADC channels to physical MCU channels
 * and their default settings.
 */

#include "hal_adc_cfg.h" // Include the configuration header
#include "hal_adc.h"     // Include the public header for logical IDs

// Define the global array of HAL ADC configurations.
// The order of elements in this array MUST match the HAL_ADC_Channel_t enum.
const HAL_ADC_Config_t g_hal_adc_configs[HAL_ADC_CHANNEL_COUNT] = {
    [HAL_ADC_CHANNEL_TEMP_SENSOR] = {
        .logical_id         = HAL_ADC_CHANNEL_TEMP_SENSOR,
        .mcal_channel       = MCAL_ADC_CHANNEL_0,  // Example: Physical ADC channel 0
        .default_resolution = HAL_ADC_RES_12_BIT,
        .default_attenuation= HAL_ADC_ATTEN_11DB, // Assuming sensor output is 0-3.3V
        .max_voltage_mv     = 3300, // Max voltage for 11dB attenuation on ESP32 is approx 3300mV
        .min_voltage_mv     = 0
    },
    [HAL_ADC_CHANNEL_HUMIDITY_SENSOR] = {
        .logical_id         = HAL_ADC_CHANNEL_HUMIDITY_SENSOR,
        .mcal_channel       = MCAL_ADC_CHANNEL_1,  // Example: Physical ADC channel 1
        .default_resolution = HAL_ADC_RES_12_BIT,
        .default_attenuation= HAL_ADC_ATTEN_11DB, // Assuming sensor output is 0-3.3V
        .max_voltage_mv     = 3300, // Max voltage for 11dB attenuation on ESP32 is approx 3300mV
        .min_voltage_mv     = 0
    },
    // Add configurations for other logical ADC channels here
};
