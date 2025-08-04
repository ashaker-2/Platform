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
#include "mcal_adc.h"       // MCAL layer for direct ADC hardware access
#include "app_common.h"     // Common application definitions
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

// --- Private Function Prototypes ---
static MCAL_ADC_Resolution_t HAL_ADC_MapResolutionToMcal(HAL_ADC_Resolution_t hal_res);
static MCAL_ADC_Attenuation_t HAL_ADC_MapAttenuationToMcal(HAL_ADC_Attenuation_t hal_atten);

// --- Public Function Implementations ---

/**
 * @brief Initializes the HAL ADC module.
 * This function should be called once during system startup.
 * It prepares the internal data structures and potentially the underlying MCAL.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_ADC_Init(void) {
    if (s_hal_adc_initialized) {
        LOG_WARNING("HAL_ADC", "Module already initialized.");
        return HAL_OK;
    }

    // Initialize MCAL ADC (if MCAL_ADC_Init requires it)
    // Assuming MCAL_ADC_Init is called once at the MCAL layer initialization
    // For this consolidated file, we'll call a mock MCAL_ADC_Init here.
    // In a real project, this would be part of sys_startup.c's MCAL init phase.
    // if (MCAL_ADC_Init() != MCAL_OK) { // Assuming MCAL_OK is the success status for MCAL
    //     LOG_ERROR("HAL_ADC", "Failed to initialize MCAL ADC.");
    //     SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_ADC_ERROR, 0); // Use a generic HAL ADC error
    //     return HAL_ERROR;
    // }

    // Initialize internal state for all logical channels
    for (uint8_t i = 0; i < HAL_ADC_CHANNEL_COUNT; i++) {
        s_adc_channel_states[i].current_resolution = HAL_ADC_RES_12_BIT; // Default
        s_adc_channel_states[i].current_attenuation = HAL_ADC_ATTEN_11DB; // Default
        s_adc_channel_states[i].is_configured = false;

        const HAL_ADC_Config_t *config = &g_hal_adc_configs[i];
        if (config->logical_id != (HAL_ADC_Channel_t)i) {
            LOG_ERROR("HAL_ADC", "Config array mismatch at index %d. Check hal_adc_cfg.c!", i);
            SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_ADC_ERROR, (uint32_t)i);
            return HAL_ERROR; // Critical configuration error
        }

        // Apply default configurations during initialization
        HAL_Status_t status = HAL_ADC_ConfigChannel(config->logical_id,
                                                    config->default_resolution,
                                                    config->default_attenuation);
        if (status != HAL_OK) {
            LOG_ERROR("HAL_ADC", "Failed to apply default config for channel %d.", config->logical_id);
            SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_ADC_ERROR, (uint32_t)config->logical_id);
            // For critical channels, this might lead to HAL_ERROR return
        }
    }

    s_hal_adc_initialized = true;
    LOG_INFO("HAL_ADC", "Module initialized.");
    return HAL_OK;
}

/**
 * @brief Configures a specific ADC channel.
 * This sets the resolution, attenuation, and any other channel-specific settings.
 * @param channel The logical ID of the ADC channel to configure.
 * @param resolution The desired ADC resolution.
 * @param attenuation The desired input attenuation (voltage range).
 * @return HAL_OK on success, HAL_ERROR on failure (e.g., invalid channel, MCAL error).
 */
HAL_Status_t HAL_ADC_ConfigChannel(HAL_ADC_Channel_t channel,
                                   HAL_ADC_Resolution_t resolution,
                                   HAL_ADC_Attenuation_t attenuation) {
    if (channel >= HAL_ADC_CHANNEL_COUNT) {
        LOG_ERROR("HAL_ADC", "Invalid channel ID %d for configuration.", channel);
        return HAL_INVALID_PARAM;
    }

    const HAL_ADC_Config_t *config = &g_hal_adc_configs[channel];
    MCAL_ADC_Resolution_t mcal_res = HAL_ADC_MapResolutionToMcal(resolution);
    MCAL_ADC_Attenuation_t mcal_atten = HAL_ADC_MapAttenuationToMcal(attenuation);

    if (mcal_res == MCAL_ADC_RES_INVALID || mcal_atten == MCAL_ADC_ATTEN_INVALID) {
        LOG_ERROR("HAL_ADC", "Invalid resolution or attenuation mapping for channel %d.", channel);
        return HAL_INVALID_PARAM;
    }

    // Call MCAL to configure the channel
    if (MCAL_ADC_ConfigChannel(config->mcal_channel, mcal_res, mcal_atten) != APP_OK) {
        LOG_ERROR("HAL_ADC", "MCAL config failed for channel %d (MCAL %d).", channel, config->mcal_channel);
        SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_ADC_ERROR, (uint32_t)channel);
        return HAL_ERROR;
    }

    // Update internal state
    s_adc_channel_states[channel].current_resolution = resolution;
    s_adc_channel_states[channel].current_attenuation = attenuation;
    s_adc_channel_states[channel].is_configured = true;
    LOG_DEBUG("HAL_ADC", "Channel %d configured: Res=%d, Atten=%d", channel, resolution, attenuation);
    return HAL_OK;
}

/**
 * @brief Reads the raw analog value from a specific ADC channel.
 * The returned value is the raw digital count based on the configured resolution.
 * @param channel The logical ID of the ADC channel to read.
 * @param raw_value_p Pointer to store the raw ADC value.
 * @return HAL_OK on success, HAL_ERROR on failure (e.g., channel not configured, MCAL error).
 */
HAL_Status_t HAL_ADC_ReadChannel(HAL_ADC_Channel_t channel, uint16_t *raw_value_p) {
    if (channel >= HAL_ADC_CHANNEL_COUNT || !s_adc_channel_states[channel].is_configured || raw_value_p == NULL) {
        LOG_ERROR("HAL_ADC", "Read failed: Invalid channel ID %d, not configured, or NULL raw_value_p.", channel);
        return HAL_INVALID_PARAM;
    }

    const HAL_ADC_Config_t *config = &g_hal_adc_configs[channel];
    MCAL_Status_t mcal_status = MCAL_ADC_ReadChannel(config->mcal_channel, raw_value_p);

    if (mcal_status != APP_OK) {
        LOG_ERROR("HAL_ADC", "MCAL read failed for channel %d (MCAL %d).", channel, config->mcal_channel);
        SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_ADC_ERROR, (uint32_t)channel);
        return HAL_ERROR;
    }
    LOG_VERBOSE("HAL_ADC", "Channel %d read raw value: %u", channel, *raw_value_p);
    return HAL_OK;
}

/**
 * @brief Converts a raw ADC value to millivolts (mV).
 * This function uses the configured resolution and attenuation to estimate the
 * analog voltage corresponding to the raw ADC reading.
 * @param channel The logical ID of the ADC channel (used to get its configuration).
 * @param raw_value The raw ADC value.
 * @param voltage_mv_p Pointer to store the converted voltage in millivolts.
 * @return HAL_OK on success, HAL_ERROR on failure (e.g., invalid channel, unconfigured).
 */
HAL_Status_t HAL_ADC_RawToMillivolts(HAL_ADC_Channel_t channel, uint16_t raw_value, uint32_t *voltage_mv_p) {
    if (channel >= HAL_ADC_CHANNEL_COUNT || !s_adc_channel_states[channel].is_configured || voltage_mv_p == NULL) {
        LOG_ERROR("HAL_ADC", "RawToMillivolts failed: Invalid channel ID %d, not configured, or NULL voltage_mv_p.", channel);
        return HAL_INVALID_PARAM;
    }

    const HAL_ADC_Config_t *config = &g_hal_adc_configs[channel];
    uint32_t max_raw_value;

    // Determine max raw value based on resolution
    switch (s_adc_channel_states[channel].current_resolution) {
        case HAL_ADC_RES_8_BIT:  max_raw_value = (1 << 8) - 1;  break; // 255
        case HAL_ADC_RES_10_BIT: max_raw_value = (1 << 10) - 1; break; // 1023
        case HAL_ADC_RES_12_BIT: max_raw_value = (1 << 12) - 1; break; // 4095
        case HAL_ADC_RES_16_BIT: max_raw_value = (1 << 16) - 1; break; // 65535
        default:
            LOG_ERROR("HAL_ADC", "Unknown resolution %d for channel %d.", s_adc_channel_states[channel].current_resolution, channel);
            SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_ADC_ERROR, (uint32_t)channel);
            return HAL_ERROR;
    }

    // Ensure raw_value does not exceed max_raw_value
    if (raw_value > max_raw_value) {
        raw_value = max_raw_value;
        LOG_WARNING("HAL_ADC", "Raw value %u exceeds max %lu for channel %d. Clamping.", raw_value, max_raw_value, channel);
    }

    // Calculate voltage in millivolts
    // (raw_value / max_raw_value) * (max_voltage_mv - min_voltage_mv) + min_voltage_mv
    // Use 64-bit intermediate for precision if needed, then cast to uint32_t
    uint64_t calculated_mv = (uint66_t)raw_value * (config->max_voltage_mv - config->min_voltage_mv);
    calculated_mv = (calculated_mv + (max_raw_value / 2)) / max_raw_value; // Add half for rounding
    calculated_mv += config->min_voltage_mv;

    *voltage_mv_p = (uint32_t)calculated_mv;
    LOG_VERBOSE("HAL_ADC", "Channel %d raw %u converted to %lu mV", channel, raw_value, *voltage_mv_p);
    return HAL_OK;
}

// --- Private Function Implementations ---

/**
 * @brief Maps HAL_ADC_Resolution_t to MCAL_ADC_Resolution_t.
 * @param hal_res The HAL resolution enum.
 * @return The corresponding MCAL resolution enum, or MCAL_ADC_RES_INVALID if no mapping.
 */
static MCAL_ADC_Resolution_t HAL_ADC_MapResolutionToMcal(HAL_ADC_Resolution_t hal_res) {
    switch (hal_res) {
        case HAL_ADC_RES_8_BIT:  return MCAL_ADC_RES_8_BIT;
        case HAL_ADC_RES_10_BIT: return MCAL_ADC_RES_10_BIT;
        case HAL_ADC_RES_12_BIT: return MCAL_ADC_RES_12_BIT;
        case HAL_ADC_RES_16_BIT: return MCAL_ADC_RES_16_BIT;
        default: return MCAL_ADC_RES_INVALID; // Or a default MCAL error value
    }
}

/**
 * @brief Maps HAL_ADC_Attenuation_t to MCAL_ADC_Attenuation_t.
 * @param hal_atten The HAL attenuation enum.
 * @return The corresponding MCAL attenuation enum, or MCAL_ADC_ATTEN_INVALID if no mapping.
 */
static MCAL_ADC_Attenuation_t HAL_ADC_MapAttenuationToMcal(HAL_ADC_Attenuation_t hal_atten) {
    switch (hal_atten) {
        case HAL_ADC_ATTEN_0DB:   return MCAL_ADC_ATTEN_0DB;
        case HAL_ADC_ATTEN_2_5DB: return MCAL_ADC_ATTEN_2_5DB;
        case HAL_ADC_ATTEN_6DB:   return MCAL_ADC_ATTEN_6DB;
        case HAL_ADC_ATTEN_11DB:  return MCAL_ADC_ATTEN_11DB;
        default: return MCAL_ADC_ATTEN_INVALID; // Or a default MCAL error value
    }
}