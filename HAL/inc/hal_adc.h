/**
 * @file hal_adc.h
 * @brief Public header for the Hardware Abstraction Layer (HAL) ADC component.
 *
 * This component provides a hardware-independent interface for configuring and
 * reading Analog-to-Digital Converter (ADC) channels. It abstracts the
 * microcontroller-specific register access and provides a clean API for
 * higher-level modules, such as sensor drivers.
 */

#ifndef HAL_ADC_H
#define HAL_ADC_H

#include "app_common.h" // For APP_Status_t and HAL_Status_t
#include <stdint.h>     // For uint16_t, uint32_t

// --- ADC Channel Definitions (Logical IDs) ---
// These are logical IDs that map to physical ADC channels in hal_adc_cfg.c
typedef enum
{
    HAL_ADC_CHANNEL_TEMP_SENSOR = 0, /**< Logical ID for the temperature sensor ADC channel. */
    HAL_ADC_CHANNEL_HUMIDITY_SENSOR, /**< Logical ID for the humidity sensor ADC channel. */
    // Add more logical ADC channels as needed
    HAL_ADC_CHANNEL_COUNT /**< Total number of logical ADC channels. */
} HAL_ADC_Channel_t;

// --- ADC Resolution ---
typedef enum
{
    HAL_ADC_RES_8_BIT,  /**< 8-bit resolution. */
    HAL_ADC_RES_10_BIT, /**< 10-bit resolution. */
    HAL_ADC_RES_12_BIT, /**< 12-bit resolution. */
    HAL_ADC_RES_16_BIT  /**< 16-bit resolution (if supported). */
} HAL_ADC_Resolution_t;

// --- ADC Attenuation (for ESP32, controls input voltage range) ---
typedef enum
{
    HAL_ADC_ATTEN_0DB,   /**< No attenuation, input voltage range 0-1.1V (example). */
    HAL_ADC_ATTEN_2_5DB, /**< 2.5dB attenuation, input voltage range 0-1.5V (example). */
    HAL_ADC_ATTEN_6DB,   /**< 6dB attenuation, input voltage range 0-2.2V (example). */
    HAL_ADC_ATTEN_11DB   /**< 11dB attenuation, input voltage range 0-3.9V (example). */
} HAL_ADC_Attenuation_t;

// --- Public Functions ---

/**
 * @brief Initializes the HAL ADC module.
 * This function should be called once during system startup.
 * It prepares the internal data structures and potentially the underlying MCAL.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_ADC_Init(void);

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
                                   HAL_ADC_Attenuation_t attenuation);

/**
 * @brief Reads the raw analog value from a specific ADC channel.
 * The returned value is the raw digital count based on the configured resolution.
 * @param channel The logical ID of the ADC channel to read.
 * @param raw_value_p Pointer to store the raw ADC value.
 * @return HAL_OK on success, HAL_ERROR on failure (e.g., channel not configured, MCAL error).
 */
HAL_Status_t HAL_ADC_ReadChannel(HAL_ADC_Channel_t channel, uint16_t *raw_value_p);

/**
 * @brief Converts a raw ADC value to millivolts (mV).
 * This function uses the configured resolution and attenuation to estimate the
 * analog voltage corresponding to the raw ADC reading.
 * @param channel The logical ID of the ADC channel (used to get its configuration).
 * @param raw_value The raw ADC value.
 * @param voltage_mv_p Pointer to store the converted voltage in millivolts.
 * @return HAL_OK on success, HAL_ERROR on failure (e.g., invalid channel, unconfigured).
 */
HAL_Status_t HAL_ADC_RawToMillivolts(HAL_ADC_Channel_t channel, uint16_t raw_value, uint32_t *voltage_mv_p);

#endif // HAL_ADC_H
