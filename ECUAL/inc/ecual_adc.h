// ecual/inc/ecual_adc.h

#ifndef ECUAL_ADC_H
#define ECUAL_ADC_H

#include <stdint.h> // For uint16_t, uint8_t

// IMPORTANT: Directly include the MCAL driver header to align enum values.
// This ties the ECUAL enum values directly to the specific MCU's driver,
// simplifying ecual_adc.c at the cost of less strict ECUAL enum portability.
#include "driver/adc.h" // Provides ADC_UNIT_1, ADC_CHANNEL_0, ADC_ATTEN_DB0, etc.
// No need for esp_err.h here directly, as ECUAL_OK/ECUAL_ERROR are now via ecual_common.h

/**
 * @brief Defines the ADC unit (ADC1 or ADC2).
 * Values are directly aligned with ESP-IDF's adc_unit_t.
 */
typedef enum {
    ECUAL_ADC_UNIT_1 = ADC_UNIT_1, ///< ADC Unit 1 (Matches ESP-IDF's adc_unit_t)
    ECUAL_ADC_UNIT_2 = ADC_UNIT_2  ///< ADC Unit 2 (Matches ESP-IDF's adc_unit_t)
} ECUAL_ADC_Unit_t;

/**
 * @brief Defines the ADC channel numbers for both ADC units.
 * Values are directly aligned with ESP-IDF's adc_channel_t.
 */
typedef enum {
    ECUAL_ADC_CHANNEL_0 = ADC_CHANNEL_0,
    ECUAL_ADC_CHANNEL_1 = ADC_CHANNEL_1,
    ECUAL_ADC_CHANNEL_2 = ADC_CHANNEL_2,
    ECUAL_ADC_CHANNEL_3 = ADC_CHANNEL_3,
    ECUAL_ADC_CHANNEL_4 = ADC_CHANNEL_4,
    ECUAL_ADC_CHANNEL_5 = ADC_CHANNEL_5,
    ECUAL_ADC_CHANNEL_6 = ADC_CHANNEL_6,
    ECUAL_ADC_CHANNEL_7 = ADC_CHANNEL_7,
    ECUAL_ADC_CHANNEL_8 = ADC_CHANNEL_8,  // ADC2 only
    ECUAL_ADC_CHANNEL_9 = ADC_CHANNEL_9   // ADC2 only
} ECUAL_ADC_Channel_t;

/**
 * @brief Defines the ADC attenuation levels.
 * Values are directly aligned with ESP-IDF's adc_atten_t.
 */
typedef enum {
    ECUAL_ADC_ATTEN_0DB   = ADC_ATTEN_DB0,  ///< 0 dB attenuation (range 0-1.1V)
    ECUAL_ADC_ATTEN_2_5DB = ADC_ATTEN_DB2_5, ///< 2.5 dB attenuation (range 0-1.5V)
    ECUAL_ADC_ATTEN_6DB   = ADC_ATTEN_DB6,  ///< 6 dB attenuation (range 0-2.2V)
    ECUAL_ADC_ATTEN_11DB  = ADC_ATTEN_DB11 ///< 11 dB attenuation (range 0-3.9V)
} ECUAL_ADC_Attenuation_t;

/**
 * @brief Structure to hold the configuration for a single ADC channel.
 */
typedef struct {
    ECUAL_ADC_Unit_t        unit;       ///< ADC unit (ADC1 or ADC2)
    ECUAL_ADC_Channel_t     channel;    ///< ADC channel number
    ECUAL_ADC_Attenuation_t attenuation; ///< Attenuation level for this channel
} ECUAL_ADC_Config_t;

/**
 * @brief Initializes the ADC peripheral and configures specified channels.
 * This function iterates through the `adc_channel_configurations` array
 * and applies the settings for each configured channel.
 * @return ECUAL_OK if successful, ECUAL_ERROR otherwise.
 */
uint8_t ECUAL_ADC_Init(void);

/**
 * @brief Reads the raw 12-bit value from a specified ADC channel.
 * @param unit The ADC unit (ECUAL_ADC_UNIT_1 or ECUAL_ADC_UNIT_2).
 * @param channel The ADC channel number.
 * @return The raw 12-bit ADC value (0-4095) if successful, 0 if an error occurred.
 */
uint16_t ECUAL_ADC_ReadRaw(ECUAL_ADC_Unit_t unit, ECUAL_ADC_Channel_t channel);

#endif /* ECUAL_ADC_H */
