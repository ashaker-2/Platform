// ecual/inc/ecual_i2c_config.h

#ifndef ECUAL_I2C_CONFIG_H
#define ECUAL_I2C_CONFIG_H

#include <stdint.h>
#include "ecual_i2c.h" // Include ecual_i2c.h for ECUAL_I2C_Config_t definition

/**
 * @brief Define Symbolic Names for your I2C instances.
 * These map to ECUAL_I2C_UNIT_0, ECUAL_I2C_UNIT_1.
 */
#define ECUAL_I2C_SENSOR_BUS ECUAL_I2C_UNIT_0
#define ECUAL_I2C_DISPLAY_BUS ECUAL_I2C_UNIT_1


// Declare the external constant array for I2C configurations
extern const ECUAL_I2C_Config_t i2c_configurations[];
// Declare the external constant for its size
extern const uint32_t ECUAL_NUM_I2C_CONFIGURATIONS;

#endif /* ECUAL_I2C_CONFIG_H */
