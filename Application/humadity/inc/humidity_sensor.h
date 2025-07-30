// app/inc/humidity_sensor.h
#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "app_common.h"
#include "ecual_gpio.h"
#include "ecual_i2c.h"

typedef enum HUMIDITY_SENSOR_ID_t {
    HUMIDITY_SENSOR_ROOM = 0,    // DHT22 (shares physical sensor with TEMP_SENSOR_OUTSIDE)
    HUMIDITY_SENSOR_BATHROOM,    // Another DHT22
    HUMIDITY_SENSOR_SERVER_RACK, // SHT3x
    HUMIDITY_TOTAL_SENSORS
} HUMIDITY_SENSOR_ID_t;

typedef enum HUMIDITY_SENSOR_Type_t {
    HUMIDITY_TYPE_DHT22 = 0,
    HUMIDITY_TYPE_SHT3X
} HUMIDITY_SENSOR_Type_t;

typedef struct HUMIDITY_SENSOR_Config_t {
    HUMIDITY_SENSOR_Type_t      sensor_type;
    ECUAL_GPIO_ID_t             gpio_pin_id; // For DHT22. Use ECUAL_GPIO_NONE for I2C sensors.

    union {
        struct { /* For DHT22 */ } dht22;
        struct {
            ECUAL_I2C_ID_t      i2c_bus_id;
            uint8_t             i2c_address;
        } sht3x;
    } params;
} HUMIDITY_SENSOR_Config_t;

uint8_t HUMIDITY_SENSOR_Init(void);
uint8_t HUMIDITY_SENSOR_ReadHumidity(HUMIDITY_SENSOR_ID_t sensor_id, float *humidity_percent);

#endif /* HUMIDITY_SENSOR_H */
