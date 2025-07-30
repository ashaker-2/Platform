// app/inc/temp_sensor_config.h

#ifndef TEMP_SENSOR_CONFIG_H
#define TEMP_SENSOR_CONFIG_H

#include <stdint.h>
#include "temp_sensor.h" // Includes TEMP_SENSOR_Config_t definition

// Declare the external constant array defined in temp_sensor_config.c
extern const TEMP_SENSOR_Config_t temp_sensor_configurations[];

// Declare the external constant for the number of configurations
extern const uint32_t TEMP_NUM_SENSOR_CONFIGURATIONS;

#endif /* TEMP_SENSOR_CONFIG_H */
