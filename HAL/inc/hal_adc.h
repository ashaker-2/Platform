/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/inc/HAL_ADC.h
 * ============================================================================*/
/**
 * @file HAL_ADC.h
 * @brief Public API for interacting with the ADC peripheral.
 * This header declares functions for initializing the ADC and reading raw ADC values.
 */
#ifndef HAL_ADC_H
#define HAL_ADC_H

#include "driver/adc.h"     // For adc1_channel_t
#include "common.h" // For Status_t

/**
 * @brief Initializes the ADC peripheral (ADC1 unit) with its specific configuration
 * and configures the channels based on the data in `HAL_ADC_Cfg.c`.
 * This is the main initialization function for the ADC HAL.
 * @return E_OK if initialization is successful, otherwise an error code.
 */
Status_t HAL_ADC_Init(void);

/**
 * @brief Reads a raw ADC value from a specified ADC1 channel.
 * @param channel The ADC1 channel to read (e.g., HW_ADC1_NTC_TEMP0_CHANNEL from HAL_Config.h).
 * @param raw_value_out Pointer to store the raw 12-bit ADC value (0-4095).
 * @return E_OK on success, or an error code.
 */
Status_t HAL_ADC_ReadRaw(adc1_channel_t channel, int *raw_value_out);

// Note: For advanced usage like ADC calibration, refer to ESP-IDF's esp_adc_cal functions
// as this typically lives at a higher layer than the raw HAL.

#endif /* HAL_ADC_H */
