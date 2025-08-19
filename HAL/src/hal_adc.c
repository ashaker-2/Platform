/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/src/HAL_ADC.c
 * ============================================================================*/
/**
 * @file HAL_ADC.c
 * @brief Implements the public API functions for ADC operations,
 * including the module's initialization function.
 * These functions wrap the ESP-IDF ADC driver calls with a common status return.
 */

#include "hal_adc.h"        // Header for HAL_ADC functions
#include "hal_adc_cfg.h"    // To access ADC configuration array and parameters
#include "esp_log.h"        // ESP-IDF logging library
#include "driver/adc.h"     // ESP-IDF ADC driver
#include "esp_err.h"        // For ESP_OK, ESP_FAIL, etc.

static const char *TAG = "HAL_ADC";

/**
 * @brief Initializes the ADC peripheral (ADC1 unit) with its specific configuration
 * and configures the channels based on the internal `s_adc_channel_attenuations` array
 * from `hal_adc_cfg.c`.
 *
 * @return E_OK if initialization is successful, otherwise an error code.
 */
Status_t HAL_ADC_Init(void) {
    esp_err_t ret;

    ESP_LOGI(TAG, "Applying ADC configurations from hal_adc_cfg.c...");

    // Configure ADC1 unit for 12-bit resolution (ADC_WIDTH_BITS is from hal_adc_cfg.h)
    ret = adc1_config_width(ADC_WIDTH_BITS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC1 width config failed: %s", esp_err_to_name(ret));
        return E_ERROR;
    }
    ESP_LOGD(TAG, "ADC1 configured for %d-bit resolution.", ADC_WIDTH_BITS);

    // Configure NTC Temperature Sensor ADC1 channels with attenuation from array
    for (size_t i = 0; i < s_num_adc_channel_attenuations; i++) {
        const adc_channel_atten_cfg_t *channel_cfg = &s_adc_channel_attenuations[i];
        ret = adc1_config_channel_atten(channel_cfg->channel, channel_cfg->attenuation);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "ADC1 channel %d config failed: %s", channel_cfg->channel, esp_err_to_name(ret));
            return E_ERROR;
        }
        ESP_LOGD(TAG, "ADC1 channel %d configured with attenuation %d.",
                 channel_cfg->channel, channel_cfg->attenuation);
    }

    ESP_LOGI(TAG, "ADC1 configurations applied successfully.");
    return E_OK;
}

/**
 * @brief Reads a raw ADC value from a specified ADC1 channel.
 * @param channel The ADC1 channel to read.
 * @param raw_value_out Pointer to store the raw 12-bit ADC value (0-4095).
 * @return E_OK on success, or an error code.
 */
Status_t HAL_ADC_ReadRaw(adc1_channel_t channel, int *raw_value_out) 
{
    if (raw_value_out == NULL) {
        ESP_LOGE(TAG, "HAL_ADC1_ReadRaw: raw_value_out pointer is NULL for channel %d.", channel);
        return E_INVALID_PARAM;
    }
    if (channel >= ADC1_CHANNEL_MAX) {
        ESP_LOGE(TAG, "HAL_ADC1_ReadRaw: Invalid ADC1 channel: %d.", channel);
        return E_INVALID_PARAM;
    }

    int raw = adc1_get_raw(channel);
    if (raw == -1) { // adc1_get_raw returns -1 on error
        ESP_LOGE(TAG, "Failed to get raw ADC reading for channel %d.", channel);
        return E_ERROR;
    }
    *raw_value_out = raw;
    return E_OK;
}
