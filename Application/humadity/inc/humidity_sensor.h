// app/inc/humidity_sensor.h

#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>         // For NAN (Not-a-Number)
#include "app_common.h"   // For APP_OK/APP_ERROR
#include "ecual_gpio.h"   // For ECUAL_GPIO_ID_t
#include "ecual_i2c.h"    // For ECUAL_I2C_ID_t <--- NEW

/**
 * @brief User-friendly IDs for configured Humidity Sensor instances.
 */
typedef enum HUMIDITY_SENSOR_ID_t {
    HUMIDITY_SENSOR_ROOM = 0,    ///< DHT22 for room humidity
    HUMIDITY_SENSOR_BATHROOM,    ///< Another DHT22 for bathroom humidity
    HUMIDITY_SENSOR_SERVER_RACK, ///< SHT3X for server rack humidity <--- NEW
    HUMIDITY_TOTAL_SENSORS       ///< Keep this last to get the count
} HUMIDITY_SENSOR_ID_t;

/**
 * @brief Enum to define the type of humidity sensor.
 */
typedef enum HUMIDITY_SENSOR_Type_t {
    HUMIDITY_TYPE_DHT22 = 0,      ///< DHT22 temperature and humidity sensor
    HUMIDITY_TYPE_SHT3X           ///< SHT3x I2C temperature and humidity sensor <--- NEW
} HUMIDITY_SENSOR_Type_t;

/**
 * @brief Structure to hold the configuration for a single Humidity Sensor.
 * Uses a union to store type-specific parameters efficiently.
 */
typedef struct HUMIDITY_SENSOR_Config_t {
    HUMIDITY_SENSOR_Type_t      sensor_type;    ///< Type of the sensor.
    ECUAL_GPIO_ID_t             gpio_pin_id;    ///< GPIO pin ID for DHT22 data line. Not used for I2C sensors.
                                                ///< For I2C, this might be used for power enable if needed.

    // Union for type-specific parameters
    union {
        // Parameters for DHT22
        struct {
            // No extra config needed, just the GPIO pin_id
        } dht22;

        // Parameters for SHT3X (I2C)
        struct {
            ECUAL_I2C_ID_t      i2c_bus_id;      ///< The ECUAL I2C bus ID the sensor is connected to.
            uint8_t             i2c_address;     ///< The I2C address of the SHT3X sensor (e.g., 0x44 or 0x45).
            // Future: Add other SHT3X specific settings like measurement mode, repeatability.
        } sht3x;
    } params;
} HUMIDITY_SENSOR_Config_t;

/**
 * @brief Initializes all Humidity Sensor modules based on their configurations.
 * This sets up the underlying sensor drivers (e.g., DHT, SHT3x components).
 * @return APP_OK if all sensors are initialized successfully, APP_ERROR otherwise.
 */
uint8_t HUMIDITY_SENSOR_Init(void);

/**
 * @brief Reads the humidity from a specific sensor.
 * @param sensor_id The HUMIDITY_SENSOR_ID_t of the sensor to read.
 * @param humidity_percent Pointer to a float variable to store the humidity in percentage (0-100%).
 * @return APP_OK if successful, APP_ERROR otherwise. Returns APP_ERROR if the sensor fails to read.
 */
uint8_t HUMIDITY_SENSOR_ReadHumidity(HUMIDITY_SENSOR_ID_t sensor_id, float *humidity_percent);

#endif /* HUMIDITY_SENSOR_H */
