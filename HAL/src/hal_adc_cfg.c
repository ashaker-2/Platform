/**
 * @file hal_adc_cfg.c
 * @brief ADC HAL channel configuration implementation.
 * @version 1.3
 * @date 2025
 *
 * This file contains the initialized array of ADC channel configurations
 * and the ADC unit initialization configuration.
 */

#include "hal_adc_cfg.h"

/* =============================================================================
 * PUBLIC VARIABLES
 * ============================================================================= */

/**
 * @brief ADC unit initialization configuration.
 */
const adc_oneshot_unit_init_cfg_t g_adc_unit_init_cfg = {
    .unit_id = ADC_CFG_UNIT,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
};

/**
 * @brief Array of ADC channel configurations.
 */
ADC_Channel_Config_t g_adc_channel_configs[ADC_CFG_MAX_CHANNELS] = {
    // Channel 0: NTC Sensor
    {
        .channel = ADC_CHANNEL_6,     // Corresponds to GPIO34 on ESP32
        .chan_cfg = {
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .atten = ADC_ATTEN_DB_11,     // Full-scale voltage 3.9V
        }
    },
    // Channel 1: LDR (Light Dependent Resistor)
    {
        .channel = ADC_CHANNEL_7,     // Corresponds to GPIO35 on ESP32
        .chan_cfg = {
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .atten = ADC_ATTEN_DB_11,     // Full-scale voltage 3.9V
        }
    }
};