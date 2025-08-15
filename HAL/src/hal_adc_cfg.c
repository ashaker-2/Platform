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
const HAL_ADC_Config_t g_hal_adc_configs[HAL_ADC_CHANNEL_COUNT] = 
{
    [HAL_ADC1_CHANNEL_0] = {
        .logical_id          =(uint8_t) HAL_ADC1_CHANNEL_0,
        .mcal_channel        =(uint8_t) 36,  // GPIO36
        .attenuation         =(uint8_t) HAL_ADC_ATTEN_11DB, // 0–3.3V range
        .SampleRate          = 64
    },
    [HAL_ADC1_CHANNEL_1] = {
        .logical_id          =(uint8_t) HAL_ADC1_CHANNEL_1,
        .mcal_channel        =(uint8_t) 37,  // GPIO37
        .attenuation         =(uint8_t) HAL_ADC_ATTEN_11DB, // 0–3.3V range
        .SampleRate          = 64
    },
    [HAL_ADC1_CHANNEL_2] = {
        .logical_id          =(uint8_t)HAL_ADC1_CHANNEL_2,
        .mcal_channel        =(uint8_t)38,  // GPIO38
        .attenuation         =(uint8_t) HAL_ADC_ATTEN_11DB, // 0–3.3V range
        .SampleRate          = 64
    },
    [HAL_ADC1_CHANNEL_3] = {
        .logical_id          =(uint8_t)HAL_ADC1_CHANNEL_3,
        .mcal_channel        =(uint8_t)39,  // GPIO39
        .attenuation         =(uint8_t) HAL_ADC_ATTEN_11DB, // 0–3.3V range
        .SampleRate          = 64
    },
    [HAL_ADC1_CHANNEL_4] = {
        .logical_id          =(uint8_t)HAL_ADC1_CHANNEL_4,
        .mcal_channel        =(uint8_t)32,  // GPIO32
        .attenuation         =(uint8_t) HAL_ADC_ATTEN_11DB, // 0–3.3V range
        .SampleRate          = 64
    },
    [HAL_ADC1_CHANNEL_5] = {
        .logical_id          =(uint8_t)HAL_ADC1_CHANNEL_5,
        .mcal_channel        =(uint8_t)33,  // GPIO33
        .attenuation         =(uint8_t) HAL_ADC_ATTEN_11DB, // 0–3.3V range
        .SampleRate          = 64
    },
    [HAL_ADC1_CHANNEL_6] = {
        .logical_id          =(uint8_t)HAL_ADC1_CHANNEL_6,
        .mcal_channel        =(uint8_t)34,  // GPIO34
        .attenuation         =(uint8_t) HAL_ADC_ATTEN_11DB, // 0–3.3V range
        .SampleRate          = 64
    },
    [HAL_ADC1_CHANNEL_7] = {
        .logical_id          =(uint8_t) HAL_ADC1_CHANNEL_7,
        .mcal_channel        =(uint8_t) 35,  // GPIO35
        .attenuation         =(uint8_t) HAL_ADC_ATTEN_11DB, // 0–3.3V range
        .SampleRate          = 64
    }
};

