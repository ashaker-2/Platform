// app/src/temp_sensor_config.c

#include "temp_sensor_config.h"

// Definition of the static constant array for Temperature Sensor configurations
const TEMP_SENSOR_Config_t temp_sensor_configurations[] = {
    // TEMP_SENSOR_ROOM (DS18B20)
    {
        .sensor_type = TEMP_TYPE_DS18B20,
        .gpio_pin_id = ECUAL_GPIO_DS18B20_DATA, // ECUAL_GPIO_ID for DS18B20 data line (needs pull-up resistor)
        .params.ds18b20 = {
            // No specific parameters needed for DS18B20 in this example, component auto-detects
        }
    },
    // TEMP_SENSOR_OUTSIDE (DHT22)
    {
        .sensor_type = TEMP_TYPE_DHT22,
        .gpio_pin_id = ECUAL_GPIO_DHT22_DATA, // ECUAL_GPIO_ID for DHT22 data line (needs pull-up resistor)
        .params.dht22 = {
            // No specific parameters needed for DHT22
        }
    },
    // TEMP_SENSOR_HEATSINK (LM35 Analog)
    {
        .sensor_type = TEMP_TYPE_LM35_ANALOG,
        .gpio_pin_id = ECUAL_GPIO_ADC1_CH4, // The GPIO associated with ADC1_CHANNEL_4
        .params.lm35 = {
            .adc_channel_id  = ECUAL_ADC_CHANNEL_4, // ECUAL_ADC_Channel_ID for LM35
            .voltage_reference = 3300.0f,          // Assuming 3.3V ADC reference (in mV)
        }
    }
};

// Definition of the number of configurations in the array
const uint32_t TEMP_NUM_SENSOR_CONFIGURATIONS = sizeof(temp_sensor_configurations) / sizeof(TEMP_SENSOR_Config_t);
