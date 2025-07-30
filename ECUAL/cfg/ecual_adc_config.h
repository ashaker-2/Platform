// ecual/inc/ecual_adc_config.h

#ifndef ECUAL_ADC_CONFIG_H
#define ECUAL_ADC_CONFIG_H

#include <stdint.h>
#include "ecual_adc.h" // Includes ecual_adc.h for ECUAL_ADC_Config_t definition

/**
 * @brief Define Symbolic Names for your 6 ADC Channels.
 * These map to hardware channels (e.g., ADC1_CHANNEL_0).
 * Important: Ensure these map to physical GPIOs (e.g., ADC1_CHANNEL0 is GPIO36)
 *
 * ADC1 Channels and their default GPIOs:
 * CH0: GPIO36, CH1: GPIO37, CH2: GPIO38, CH3: GPIO39
 * CH4: GPIO32, CH5: GPIO33, CH6: GPIO34, CH7: GPIO35
 */
#define ECUAL_ADC_CHANNEL_BATTERY_VOLTAGE    ECUAL_ADC_CHANNEL_0 // GPIO36
#define ECUAL_ADC_CHANNEL_TEMPERATURE_SENSOR ECUAL_ADC_CHANNEL_3 // GPIO39
#define ECUAL_ADC_CHANNEL_LIGHT_SENSOR       ECUAL_ADC_CHANNEL_4 // GPIO32
#define ECUAL_ADC_CHANNEL_POTENTIOMETER_1    ECUAL_ADC_CHANNEL_5 // GPIO33
#define ECUAL_ADC_CHANNEL_POTENTIOMETER_2    ECUAL_ADC_CHANNEL_6 // GPIO34
#define ECUAL_ADC_CHANNEL_EXTERNAL_SENSOR    ECUAL_ADC_CHANNEL_7 // GPIO35


// Declare the external constant array for ADC configurations
extern const ECUAL_ADC_Config_t adc_channel_configurations[];
// Declare the external constant for its size
extern const uint32_t ECUAL_NUM_ADC_CONFIGURATIONS;

#endif /* ECUAL_ADC_CONFIG_H */
