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
#include "mcal_adc.h"  // For MCAL_ADC_Channel_t, MCAL_ADC_Resolution_t etc. (MCU-specific)

// --- Configuration Structure for each Logical ADC Channel ---
/**
 * @brief Structure to hold the static configuration for each logical ADC channel.
 */
typedef struct {
    HAL_ADC_Channel_t       logical_id;         /**< The logical ID of the ADC channel. */
    MCAL_ADC_Channel_t      mcal_channel;       /**< The corresponding MCAL (physical) ADC channel. */
    HAL_ADC_Resolution_t    default_resolution; /**< Default resolution at initialization. */
    HAL_ADC_Attenuation_t   default_attenuation;/**< Default attenuation at initialization. */
    uint32_t                max_voltage_mv;     /**< Maximum voltage in mV for this channel at full scale. */
    uint32_t                min_voltage_mv;     /**< Minimum voltage in mV for this channel at zero scale. */
    // Add more configuration parameters as needed (e.g., reference voltage)
} HAL_ADC_Config_t;

// --- External Declaration of Configuration Array ---
/**
 * @brief Global array containing the configuration for all logical ADC channels.
 * This array is defined in hal_adc_cfg.c and accessed by the HAL_ADC module.
 * The order of elements in this array must match the HAL_ADC_Channel_t enum.
 */
extern const HAL_ADC_Config_t g_hal_adc_configs[HAL_ADC_CHANNEL_COUNT];

#endif // HAL_ADC_CFG_H
