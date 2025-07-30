// ecual/src/ecual_adc.c

#include "ecual_adc.h"
#include "ecual_adc_config.h"
#include "ecual_common.h" // For ECUAL_OK and ECUAL_ERROR

// No local definitions for ECUAL_OK/ERROR needed here anymore

uint8_t ECUAL_ADC_Init(void) {
    uint32_t i;
    uint8_t ecual_ret = ECUAL_OK;
    esp_err_t mcal_ret;

    // Configure ADC1 width (common for all ADC1 channels). Set to 12-bit (range 0-4095)
    // The driver internally manages the RTC_IO setup when adc1_config_channel is called.
    if (adc1_config_width(ADC_WIDTH_BIT_12) != ESP_OK) {
        return ECUAL_ERROR; // ADC1 width config failed
    }

    // Initialize each configured ADC channel
    for (i = 0; i < ECUAL_NUM_ADC_CONFIGURATIONS; i++) {
        const ECUAL_ADC_Config_t *cfg = &adc_channel_configurations[i];
        
        if (cfg->unit == ECUAL_ADC_UNIT_1) {
            // Configure ADC1 channel with specified attenuation - Direct cast possible
            mcal_ret = adc1_config_channel((adc1_channel_t)cfg->channel, (adc_atten_t)cfg->attenuation);
            if (mcal_ret != ESP_OK) {
                ecual_ret = ECUAL_ERROR;
                // Optionally log error
                continue;
            }
        } else { // ECUAL_ADC_UNIT_2
            // ADC2 requires explicit RTC init if not done by full IDF context
            // and is prone to Wi-Fi conflict.
            // For simplicity in a bare-metal context without Wi-Fi, we'll configure it.
            adc2_rtc_init(); // This is typically a one-time call for ADC2

            // Configure ADC2 channel with specified attenuation - Direct cast possible
            mcal_ret = adc2_config_channel((adc2_channel_t)cfg->channel, (adc_atten_t)cfg->attenuation);
            if (mcal_ret != ESP_OK) {
                ecual_ret = ECUAL_ERROR;
                // Optionally log error
                continue;
            }
        }
    }

    return ecual_ret;
}

uint16_t ECUAL_ADC_ReadRaw(ECUAL_ADC_Unit_t unit, ECUAL_ADC_Channel_t channel) {
    uint16_t raw_value = 0;
    esp_err_t mcal_ret;

    if (unit == ECUAL_ADC_UNIT_1) {
        // adc1_get_raw returns int. -1 indicates error, otherwise raw value.
        int val = adc1_get_raw((adc1_channel_t)channel);
        if (val == -1) { // -1 is common return for error/invalid channel
            return 0; // Return 0 or specific error value for ECUAL if read fails
        }
        raw_value = (uint16_t)val;
    } else { // ECUAL_ADC_UNIT_2
        // For ADC2, adc2_get_raw uses a pointer for value and returns esp_err_t
        int val;
        mcal_ret = adc2_get_raw((adc2_channel_t)channel, &val);
        if (mcal_ret != ESP_OK) {
            return 0; // Error reading ADC2
        }
        raw_value = (uint16_t)val;
    }

    return raw_value;
}
