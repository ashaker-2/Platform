// ecual/src/ecual_adc_config.c

#include "ecual_adc_config.h" // Includes ecual_adc.h for ECUAL_ADC_Config_t definition

// Definition of the static constant array for ADC channel configurations
const ECUAL_ADC_Config_t adc_channel_configurations[] = {
    // Example configurations for 6 channels, primarily using ADC1
    {ECUAL_ADC_UNIT_1, ECUAL_ADC_CHANNEL_BATTERY_VOLTAGE,    ECUAL_ADC_ATTEN_11DB}, // Max range for battery (GPIO36)
    {ECUAL_ADC_UNIT_1, ECUAL_ADC_CHANNEL_TEMPERATURE_SENSOR, ECUAL_ADC_ATTEN_6DB},  // GPIO39
    {ECUAL_ADC_UNIT_1, ECUAL_ADC_CHANNEL_LIGHT_SENSOR,       ECUAL_ADC_ATTEN_11DB}, // GPIO32
    {ECUAL_ADC_UNIT_1, ECUAL_ADC_CHANNEL_POTENTIOMETER_1,    ECUAL_ADC_ATTEN_11DB}, // GPIO33
    {ECUAL_ADC_UNIT_1, ECUAL_ADC_CHANNEL_POTENTIOMETER_2,    ECUAL_ADC_ATTEN_11DB}, // GPIO34
    {ECUAL_ADC_UNIT_1, ECUAL_ADC_CHANNEL_EXTERNAL_SENSOR,    ECUAL_ADC_ATTEN_11DB}, // GPIO35

    // If you need ADC2, you could add entries like this (be mindful of Wi-Fi conflict)
    // {ECUAL_ADC_UNIT_2, ECUAL_ADC_CHANNEL_0, ECUAL_ADC_ATTEN_11DB}, // GPIO4
};

// Definition of the number of configurations in the array
const uint32_t ECUAL_NUM_ADC_CONFIGURATIONS = sizeof(adc_channel_configurations) / sizeof(ECUAL_ADC_Config_t);
