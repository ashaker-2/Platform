/**
 * @file hal_adc.h
 * @brief Public API for the ADC Hardware Abstraction Layer.
 * @version 1.3
 * @date 2025
 *
 * This header defines the public interface for the ADC HAL. It provides
 * functions for initializing the ADC and reading a calibrated voltage value
 * from a specific channel.
 */

#ifndef HAL_ADC_H
#define HAL_ADC_H

#include <stdint.h>
#include "common.h"
#include "esp_adc/adc_oneshot.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * PUBLIC API FUNCTIONS
 * ============================================================================= */

/**
 * @brief Initializes the ADC peripheral and all configured channels.
 *
 * This function sets up the ADC unit and then iterates through all channels
 * defined in the configuration, initializing and calibrating each one.
 *
 * @return Status_t E_OK on successful initialization, E_NOK otherwise.
 */
Status_t HAL_ADC_Init(void);

/**
 * @brief Reads a calibrated voltage from a specific ADC channel.
 *
 * This function performs a one-shot read on the configured ADC channel
 * and converts the raw value to a calibrated voltage in millivolts (mV).
 *
 * @param channel_id The ID of the channel to read from.
 * @param voltage_mv Pointer to store the read voltage in millivolts.
 * @return Status_t E_OK on success, E_NOK on failure.
 */
Status_t HAL_ADC_ReadRaw(uint8_t channel_id, uint16_t *voltage_mv);

#ifdef __cplusplus
}
#endif

#endif /* HAL_ADC_H */