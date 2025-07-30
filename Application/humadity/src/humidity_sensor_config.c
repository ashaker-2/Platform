// app/src/humidity_sensor_config.c
#include "humidity_sensor_config.h"

const HUMIDITY_SENSOR_Config_t humidity_sensor_configurations[] = {
    {   // This DHT22 uses the same physical sensor as TEMP_SENSOR_OUTSIDE
        .sensor_type = HUMIDITY_TYPE_DHT22,
        .gpio_pin_id = ECUAL_GPIO_DHT22_DATA,
        .params.dht22 = {}
    },
    {
        .sensor_type = HUMIDITY_TYPE_DHT22,
        .gpio_pin_id = ECUAL_GPIO_DHT22_BATHROOM_DATA,
        .params.dht22 = {}
    },
    {
        .sensor_type = HUMIDITY_TYPE_SHT3X,
        .gpio_pin_id = ECUAL_GPIO_NONE, // Not applicable for I2C
        .params.sht3x = {
            .i2c_bus_id  = ECUAL_I2C_MASTER_0,
            .i2c_address = 0x44, // Common address for SHT3x
        }
    }
};

const uint32_t HUMIDITY_NUM_SENSOR_CONFIGURATIONS = sizeof(humidity_sensor_configurations) / sizeof(HUMIDITY_SENSOR_Config_t);
