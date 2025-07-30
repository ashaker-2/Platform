// ecual/inc/ecual_i2c_config.h
#ifndef ECUAL_I2C_CONFIG_H
#define ECUAL_I2C_CONFIG_H

#include <stdint.h>
#include "ecual_i2c.h"
#include "ecual_gpio.h"

typedef struct ECUAL_I2C_Config_t {
    ECUAL_I2C_ID_t      i2c_id;
    ECUAL_GPIO_ID_t     sda_gpio_id;
    ECUAL_GPIO_ID_t     scl_gpio_id;
    uint32_t            clk_speed_hz;
} ECUAL_I2C_Config_t;

extern const ECUAL_I2C_Config_t i2c_bus_configurations[];
extern const uint32_t ECUAL_I2C_NUM_CONFIGURATIONS;

#endif /* ECUAL_I2C_CONFIG_H */
