/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/inc/HAL_I2C_Cfg.h
 * ============================================================================*/
/**
 * @file HAL_I2C_Cfg.h
 * @brief Declarations for external I2C configuration data.
 * This header makes the static I2C configuration array available to other HAL modules,
 * particularly HAL_I2C.c, for initialization. It does not declare any functions.
 */
#ifndef HAL_I2C_CFG_H
#define HAL_I2C_CFG_H

#include "driver/i2c.h"     // For i2c_config_t, i2c_port_t

/**
 * @brief Structure to hold I2C configuration along with its port ID.
 * Defined here for external visibility, used in HAL_I2C_Cfg.c.
 */
typedef struct {
    i2c_port_t port;
    i2c_config_t config;
} i2c_cfg_item_t;

/**
 * @brief External declaration of the array containing all predefined I2C bus configurations.
 * This array is defined in HAL_I2C_Cfg.c and accessed by HAL_I2C.c to perform
 * initial I2C setup.
 */
extern const i2c_cfg_item_t s_i2c_configurations[];

/**
 * @brief External declaration of the number of elements in the I2C configurations array.
 */
extern const size_t s_num_i2c_configurations;

#endif /* HAL_I2C_CFG_H */
