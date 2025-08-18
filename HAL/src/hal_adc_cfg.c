/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/src/HAL_ADC_Cfg.c
 * ============================================================================*/
/**
 * @file HAL_ADC_Cfg.c
 * @brief Implements the static array of ADC channel configuration settings.
 * This file defines the specific initial resolution and channel attenuations.
 * It does not contain any initialization functions; its purpose is purely
 * to hold configuration data.
 */

#include "HAL_ADC_Cfg.h"    // Header for ADC configuration types and extern declarations
#include "HAL_Config.h.h"     // Global hardware definitions (ADC channels like HW_ADC1_NTC_TEMP0_CHANNEL)
#include <stddef.h>         // For size_t

/**
 * @brief Array containing all predefined ADC channel configurations.
 * Currently configured for NTC temperature sensors.
 * This array is made `const` and global (`extern` in header) to be accessed by `HAL_ADC.c`.
 */
const adc_channel_atten_cfg_t s_adc_channel_attenuations[] = {
    {
        .channel = HW_ADC1_NTC_TEMP0_CHANNEL,
        .attenuation = ADC_NTC_ATTENUATION,
    },
    {
        .channel = HW_ADC1_NTC_TEMP1_CHANNEL,
        .attenuation = ADC_NTC_ATTENUATION,
    },
    // Add other ADC1 channel configurations here if needed
};

/**
 * @brief Defines the number of elements in the `s_adc_channel_attenuations` array.
 * This variable is made `const` and global (`extern` in header) to be accessed by `HAL_ADC.c`.
 */
const size_t s_num_adc_channel_attenuations = sizeof(s_adc_channel_attenuations) / sizeof(s_adc_channel_attenuations[0]);
