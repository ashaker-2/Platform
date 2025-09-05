/**
 * @file hal_adc.c
 * @brief ADC Hardware Abstraction Layer (HAL) implementation.
 * @version 1.3
 * @date 2025
 *
 * This file provides the implementation for the ADC HAL using the
 * modern ESP-IDF ADC one-shot driver. It uses the channel-specific
 * configuration from hal_adc_cfg.c and includes robust error checking.
 */

#include "hal_adc.h"
#include "hal_adc_cfg.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

/* =============================================================================
 * PRIVATE GLOBAL VARIABLES
 * ============================================================================= */

static const char *TAG = "ADC_HAL";
static bool g_is_initialized = false;
static adc_oneshot_unit_handle_t g_adc_handle = NULL;
static adc_cali_handle_t g_adc_cali_handle[ADC_CFG_MAX_CHANNELS] = {NULL};

/* =============================================================================
 * PRIVATE FUNCTION PROTOTYPES
 * ============================================================================= */
static Status_t adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void adc_calibration_deinit(adc_cali_handle_t handle);

/* =============================================================================
 * PUBLIC API FUNCTIONS
 * ============================================================================= */

Status_t HAL_ADC_Init(void)
{
    if (g_is_initialized) {
        ESP_LOGW(TAG, "ADC HAL already initialized.");
        return E_OK;
    }
    
    // ADC unit initialization (done once for the entire unit)
    esp_err_t err = adc_oneshot_new_unit(&g_adc_unit_init_cfg, &g_adc_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create ADC unit: %s", esp_err_to_name(err));
        return E_NOK;
    }
    
    // Initialize and calibrate all configured channels
    for (uint8_t i = 0; i < ADC_CFG_MAX_CHANNELS; i++) {
        const ADC_Channel_Config_t *cfg = &g_adc_channel_configs[i];
        
        err = adc_oneshot_config_channel(g_adc_handle, cfg->channel, &cfg->chan_cfg);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to configure ADC channel %d: %s", i, esp_err_to_name(err));
            return E_NOK;
        }

        if (adc_calibration_init(ADC_CFG_UNIT, cfg->chan_cfg.atten, &g_adc_cali_handle[i]) != E_OK) {
            ESP_LOGE(TAG, "ADC calibration for channel %d failed.", i);
            return E_NOK;
        }
    }

    g_is_initialized = true;
    ESP_LOGI(TAG, "ADC HAL initialized successfully for all %d channels.", ADC_CFG_MAX_CHANNELS);
    return E_OK;
}

Status_t HAL_ADC_ReadRaw(uint8_t channel_id, uint16_t *voltage_mv)
{
    if (!g_is_initialized) {
        ESP_LOGE(TAG, "ADC not initialized. Call HAL_ADC_Init() first.");
        return E_NOK;
    }
    
    if (channel_id >= ADC_CFG_MAX_CHANNELS) {
        ESP_LOGE(TAG, "Invalid channel ID: %d", channel_id);
        return E_NOK;
    }

    if (voltage_mv == NULL) {
        ESP_LOGE(TAG, "Output pointer is NULL.");
        return E_NOK;
    }
    
    const ADC_Channel_Config_t *cfg = &g_adc_channel_configs[channel_id];

    int raw_adc;
    esp_err_t err = adc_oneshot_read(g_adc_handle, cfg->channel, &raw_adc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read from ADC channel %d: %s", channel_id, esp_err_to_name(err));
        return E_NOK;
    }

    int voltage;
    err = adc_cali_raw_to_voltage(g_adc_cali_handle[channel_id], raw_adc, &voltage);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Calibration conversion failed for channel %d: %s", channel_id, esp_err_to_name(err));
        return E_NOK;
    }

    *voltage_mv = (uint32_t)voltage;
    ESP_LOGD(TAG, "Channel %d ADC raw: %d, voltage: %d mV", channel_id, raw_adc, *voltage_mv);

    return E_OK;
}

/* =============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================= */

static Status_t adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    // ESP_LOGI(TAG, "Starting ADC calibration...");
    // adc_cali_handle_t handle = NULL;
    // esp_err_t ret = ESP_FAIL;
    
    // if (esp_adc_cali_check_efuse(ADC_CALI_SCHEME_CURVE_FITTING) == ESP_OK) {
    //     adc_cali_curve_fitting_config_t cali_config = {
    //         .unit_id = unit,
    //         .atten = atten,
    //         .bitwidth = ADC_BITWIDTH_DEFAULT,
    //     };
    //     ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
    // } else {
    //     ESP_LOGW(TAG, "eFuse calibration not available. Using default calibration curve.");
    //     adc_cali_curve_fitting_config_t cali_config = {
    //         .unit_id = unit,
    //         .atten = atten,
    //         .bitwidth = ADC_BITWIDTH_DEFAULT,
    //     };
    //     ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
    // }
    
    // if (ret != ESP_OK) {
    //     ESP_LOGE(TAG, "Calibration failed: %s", esp_err_to_name(ret));
    //     return E_NOK;
    // }
    
    // *out_handle = handle;
    // ESP_LOGI(TAG, "Calibration successful.");
    return E_OK;
}

static void adc_calibration_deinit(adc_cali_handle_t handle)
{
    // if (handle) {
    //     esp_err_t err = adc_cali_delete_scheme_curve_fitting(handle);
    //     if (err != ESP_OK) {
    //         ESP_LOGE(TAG, "Failed to de-initialize calibration: %s", esp_err_to_name(err));
    //     }
    // }
}