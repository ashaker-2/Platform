/**
 * @file hal_adc_cfg.h
 * @brief Configuration definitions for the ADC HAL.
 * @version 1.3
 * @date 2025
 *
 * This header file contains the configuration data structures and extern
 * declarations for the ADC channels.
 */

#ifndef HAL_ADC_CFG_H
#define HAL_ADC_CFG_H

#include <stdint.h>
#include "esp_adc/adc_oneshot.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * PUBLIC DEFINITIONS
 * ============================================================================= */

/**
 * @brief The total number of ADC channels configured.
 */
#define ADC_CFG_MAX_CHANNELS    2

/**
 * @brief The single ADC unit to be used for all configured channels.
 */
#define ADC_CFG_UNIT            ADC_UNIT_1

/* =============================================================================
 * TYPE DEFINITIONS
 * ============================================================================= */

/**
 * @brief Structure to hold the configuration for a single ADC channel.
 */
typedef struct {
    adc_channel_t channel;
    adc_oneshot_chan_cfg_t chan_cfg;
} ADC_Channel_Config_t;

/* =============================================================================
 * PUBLIC VARIABLES
 * ============================================================================= */

/**
 * @brief ADC unit initialization configuration.
 */
extern const adc_oneshot_unit_init_cfg_t g_adc_unit_init_cfg;

/**
 * @brief Array of ADC channel configurations.
 *
 * This array contains the specific settings for each ADC channel used by the system.
 */
extern ADC_Channel_Config_t g_adc_channel_configs[ADC_CFG_MAX_CHANNELS];

#ifdef __cplusplus
}
#endif

#endif /* HAL_ADC_CFG_H */