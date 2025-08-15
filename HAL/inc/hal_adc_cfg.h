/**
 * @file hal_adc_cfg.h
 * @brief Configuration header for the HAL ADC component.
 *
 * This file defines the hardware-specific mapping and default configurations
 * for the logical ADC channels used by the HAL ADC module.
 */

#ifndef HAL_ADC_CFG_H
#define HAL_ADC_CFG_H

#include "hal_adc.h"   // For HAL_ADC_Channel_t, HAL_ADC_Resolution_t, etc.

// --- Configuration Structure for each Logical ADC Channel ---
/**
 * @brief Structure to hold the static configuration for each logical ADC channel.
 */
typedef struct {
    uint8_t    logical_id;         /**< The logical ID of the ADC channel. */
    uint8_t    mcal_channel;       /**< The corresponding MCAL (physical) ADC channel. */
    uint8_t    attenuation;        /**< Default attenuation at initialization. */
    uint8_t    SampleRate;        /**< Default attenuation at initialization. */
    // Add more configuration parameters as needed (e.g., reference voltage)
} HAL_ADC_Config_t;


// Configure ADC Default Resolution
#define ACD1_RESOLUTION      HAL_ADC_RES_12_BIT
#define ACD1_VREF            ((float)3.3)

// --- External Declaration of Configuration Array ---
/**
 * @brief Global array containing the configuration for all logical ADC channels.
 * This array is defined in hal_adc_cfg.c and accessed by the HAL_ADC module.
 * The order of elements in this array must match the HAL_ADC_Channel_t enum.
 */
extern const HAL_ADC_Config_t g_hal_adc_configs[HAL_ADC_CHANNEL_COUNT];

#endif // HAL_ADC_CFG_H
