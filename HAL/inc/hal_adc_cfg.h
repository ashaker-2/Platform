/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/inc/hal_adc_cfg.h
 * ============================================================================*/
/**
 * @file hal_adc_cfg.h
 * @brief Declarations for external ADC configuration data.
 * This header makes the static ADC configuration array and parameters available
 * to HAL_ADC.c for initialization. It does not declare any functions.
 */
#ifndef hal_adc_cfg_H
#define hal_adc_cfg_H

#include "driver/adc.h"     // For ADC_WIDTH_BIT_x, ADC_ATTEN_DB_x, adc1_channel_t
#include <stddef.h>         // For size_t

// ADC Configuration Parameters
#define ADC_WIDTH_BITS              ADC_WIDTH_BIT_12 // 12-bit resolution
#define ADC_NTC_ATTENUATION         ADC_ATTEN_DB_11  // Approx. 0-3.3V input range for NTC

/**
 * @brief Structure to hold an ADC channel and its desired attenuation.
 * Defined here for external visibility, used in hal_adc_cfg.c.
 */
typedef struct {
    adc1_channel_t channel;
    adc_atten_t attenuation;
} adc_channel_atten_cfg_t;

/**
 * @brief External declaration of the array containing all predefined ADC channel configurations.
 * This array is defined in hal_adc_cfg.c and accessed by HAL_ADC.c to perform
 * initial ADC setup.
 */
extern const adc_channel_atten_cfg_t s_adc_channel_attenuations[];

/**
 * @brief External declaration of the number of elements in the ADC channel configurations array.
 */
extern const size_t s_num_adc_channel_attenuations;

#endif /* hal_adc_cfg_H */
