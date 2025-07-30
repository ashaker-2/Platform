// app/inc/temp_sensor.h

#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>         // For NAN (Not-a-Number)
#include "app_common.h"   // For APP_OK/APP_ERROR
#include "ecual_gpio.h"   // For ECUAL_GPIO_ID_t
#include "ecual_adc.h"    // For ECUAL_ADC_Channel_ID_t

/**
 * @brief User-friendly IDs for configured Temperature Sensor instances.
 */
typedef enum TEMP_SENSOR_ID_t {
    TEMP_SENSOR_ROOM = 0,    ///< DS18B20 for room temperature
    TEMP_SENSOR_OUTSIDE,     ///< DHT22 for outside temp/humidity
    TEMP_SENSOR_HEATSINK,    ///< LM35 for heatsink temperature
    TEMP_TOTAL_SENSORS       ///< Keep this last to get the count
} TEMP_SENSOR_ID_t;

/**
 * @brief Enum to define the type of temperature sensor.
 */
typedef enum TEMP_SENSOR_Type_t {
    TEMP_TYPE_DS18B20 = 0,    ///< Dallas DS18B20 using 1-Wire protocol
    TEMP_TYPE_DHT22,          ///< DHT22 temperature and humidity sensor
    TEMP_TYPE_LM35_ANALOG     ///< LM35 analog temperature sensor
} TEMP_SENSOR_Type_t;

/**
 * @brief Structure to hold the configuration for a single Temperature Sensor.
 * Uses a union to store type-specific parameters efficiently.
 */
typedef struct TEMP_SENSOR_Config_t {
    TEMP_SENSOR_Type_t      sensor_type;    ///< Type of the sensor (DS18B20, DHT22, LM35).
    ECUAL_GPIO_ID_t         gpio_pin_id;    ///< GPIO pin ID for the sensor's data line (DS18B20, DHT22)
                                            ///< Or the GPIO for ADC pin for LM35 (though ADC channel ID is primary).

    // Union for type-specific parameters
    union {
        // Parameters for DS18B20
        struct {
            // For DS18B20, the component typically auto-detects addresses.
            // If you need to specify a fixed address, you'd add it here (e.g., uint64_t address;)
        } ds18b20;

        // Parameters for DHT22
        struct {
            // No extra config needed, just the GPIO pin
        } dht22;

        // Parameters for LM35 (Analog)
        struct {
            ECUAL_ADC_Channel_ID_t  adc_channel_id;     ///< The ECUAL ADC channel ID connected to the LM35.
            float                   voltage_reference;  ///< ADC reference voltage in mV (e.g., 3300 for 3.3V).
                                                        ///< Crucial for accurate analog conversions.
        } lm35;
    } params;
} TEMP_SENSOR_Config_t;

/**
 * @brief Initializes all Temperature Sensor modules based on their configurations.
 * This includes setting up GPIOs, ADC channels, and underlying sensor drivers.
 * @return APP_OK if all sensors are initialized successfully, APP_ERROR otherwise.
 */
uint8_t TEMP_SENSOR_Init(void);

/**
 * @brief Reads the temperature from a specific sensor.
 * @param sensor_id The TEMP_SENSOR_ID_t of the sensor to read.
 * @param temperature_celsius Pointer to a float variable to store the temperature in Celsius.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t TEMP_SENSOR_ReadTemperature(TEMP_SENSOR_ID_t sensor_id, float *temperature_celsius);

/**
 * @brief Reads the humidity from a specific sensor (if supported, e.g., DHT22).
 * @param sensor_id The TEMP_SENSOR_ID_t of the sensor to read.
 * @param humidity_percent Pointer to a float variable to store the humidity in percentage (0-100%).
 * @return APP_OK if successful, APP_ERROR otherwise. Returns APP_ERROR if sensor does not support humidity.
 */
uint8_t TEMP_SENSOR_ReadHumidity(TEMP_SENSOR_ID_t sensor_id, float *humidity_percent);

#endif /* TEMP_SENSOR_H */
