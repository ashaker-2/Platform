/**
 * @file hal_adc.c
 * @brief Implementation for the Hardware Abstraction Layer (HAL) ADC component.
 *
 * This file implements the hardware-independent ADC interface, mapping logical
 * ADC channels to their physical MCU counterparts via the MCAL layer. It handles
 * configuration, reading raw values, and converting to millivolts.
 */

#include "hal_adc.h"        // Public header for HAL_ADC
#include "hal_adc_cfg.h"    // Configuration header for HAL_ADC
// #include "adc.h"       // MCAL layer for direct ADC hardware access
#include "common.h"     // Common application definitions
#include "system_monitor.h" // For reporting faults
#include "logger.h"         // For logging

// --- Private Data Structures ---
/**
 * @brief Runtime state for each logical ADC channel.
 */
typedef struct {
    HAL_ADC_Resolution_t    current_resolution; /**< Current configured resolution. */
    HAL_ADC_Attenuation_t   current_attenuation;/**< Current configured attenuation. */
    bool                    is_configured;      /**< Flag indicating if the channel has been configured. */
} HAL_ADC_ChannelState_t;

// --- Private Variables ---
static HAL_ADC_ChannelState_t s_adc_channel_states[HAL_ADC_CHANNEL_COUNT];
static bool s_hal_adc_initialized = false;


// --- Public Function Implementations ---

/**
 * @brief Initializes the HAL ADC module.
 * This function should be called once during system startup.
 * It prepares the internal data structures and potentially the underlying MCAL.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_ADC_Init(void) 
{
    Status_t ret = E_NOK;
    if (s_hal_adc_initialized) 
    {
        LOGI("HAL_ADC", "Module already initialized.");
        return E_OK;
    }

    // ret = adc1_config_width(ACD1_RESOLUTION);
    if (ret != E_OK) 
    {
        LOGI("HAL_ADC", "ADC Module Fail to initialize.");
        return E_OK;
    }
    
    // Initialize ADC Driver 
    for (uint8_t i = 0; i < HAL_ADC_CHANNEL_COUNT; i++) 
    {

        const HAL_ADC_Config_t *config = &g_hal_adc_configs[i];
        if (HAL_ADC_CHANNEL_COUNT < i) 
        {
            LOGE("HAL_ADC", "Config array mismatch at index %d. Check hal_adc_cfg.c!", i);
            return E_NOK; // Critical configuration error
        }

        // Apply default configurations during initialization
        // ret =  adc1_config_channel_atten(config->mcal_channel, config->attenuation);

        if (ret != E_OK) 
        {
            LOGE("HAL_ADC", "Failed to apply default config for channel %d.", config->logical_id);
        }
    }
    
    // 3. Characterize ADC to get calibration values
    // esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH_BIT, DEFAULT_VREF, &adc_chars);

    s_hal_adc_initialized = true;
    LOGI("HAL_ADC", "Module initialized.");
    return ret;
}


/**
 * @brief Reads the raw analog value from a specific ADC channel.
 * The returned value is the raw digital count based on the configured resolution.
 * @param channel The logical ID of the ADC channel to read.
 * @param raw_value_p Pointer to store the raw ADC value.
 * @return E_OK on success, E_NOK on failure (e.g., channel not configured, MCAL error).
 */
Status_t HAL_ADC_ReadChannel(HAL_ADC_Channel_t channel, uint16_t *raw_value_p) 
{
    if (channel >= HAL_ADC_CHANNEL_COUNT || raw_value_p == NULL) 
    {
        LOGE("HAL_ADC", "Read failed: Invalid channel ID %d, not configured, or NULL raw_value_p.", channel);
        return E_INVALID_PARAM;
    }

    Status_t ret = E_NOK;
    uint32_t adc_reading = 0;
    const HAL_ADC_Config_t *config = &g_hal_adc_configs[channel];
    

    // Multisample for stability
    for (uint8_t i = 0; i < config->SampleRate; i++) {
        // adc_reading += adc1_get_raw(config->mcal_channel);
    }
    adc_reading /= config->SampleRate;
    *raw_value_p = adc_reading;

    // LOGI("HAL_ADC", "Channel %d read raw value: %u", channel, *raw_value_p);
    return E_OK;
}

/**
 * @brief Converts a raw ADC value to millivolts (mV).
 * This function uses the configured resolution and attenuation to estimate the
 * analog voltage corresponding to the raw ADC reading.
 * @param channel The logical ID of the ADC channel (used to get its configuration).
 * @param raw_value The raw ADC value.
 * @param voltage_mv_p Pointer to store the converted voltage in millivolts.
 * @return E_OK on success, E_NOK on failure (e.g., invalid channel, unconfigured).
 */
Status_t HAL_ADC_RawToMillivolts(uint16_t raw_value, uint32_t *voltage_mv_p) 
{
    if (voltage_mv_p == NULL) 
    {
        LOGE("HAL_ADC", "RawToMillivolts failed: NULL voltage_mv_p.");
        return E_INVALID_PARAM;
    }
    Status_t ret = E_OK;
    uint32_t max_raw_value = 0;
    uint8_t u8DefaultResolution = ACD1_RESOLUTION;
    uint64_t calculated_mv = 0;

    // Determine max raw value based on resolution
    switch (u8DefaultResolution) 
    {
        case HAL_ADC_RES_8_BIT:  max_raw_value = (1 << 8) - 1;  break; // 255
        case HAL_ADC_RES_10_BIT: max_raw_value = (1 << 10) - 1; break; // 1023
        case HAL_ADC_RES_12_BIT: max_raw_value = (1 << 12) - 1; break; // 4095
        case HAL_ADC_RES_16_BIT: max_raw_value = (1 << 16) - 1; break; // 65535
        default:
            LOGE("HAL_ADC", "Unknown resolution");
            return E_NOK;
    }

    // Ensure raw_value does not exceed max_raw_value
    if (raw_value > max_raw_value) 
    {
        LOGE("HAL_ADC", "Raw value %u exceeds max %lu. Clamping.", raw_value, max_raw_value);
        ret = E_NOK;
    }

    // Calculate voltage in millivolts
    // Use 64-bit intermediate for precision if needed, then cast to uint32_t
    calculated_mv = (raw_value * ACD1_VREF)/max_raw_value;


    *voltage_mv_p = (uint32_t)calculated_mv;
    // LOGI("HAL_ADC", "Channel %d raw %u converted to %lu mV", channel, raw_value, *voltage_mv_p);
    return E_OK;
}

