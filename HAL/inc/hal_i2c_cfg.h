/**
 * @file hal_i2c_cfg.h
 * @brief Hardware Abstraction Layer for I2C - Configuration Header.
 *
 * This header defines the compile-time configuration parameters for each
 * I2C port. These settings are typically defined in `hal_i2c_cfg.c`
 * and are used during the initialization of the I2C HAL.
 */

#ifndef HAL_I2C_CFG_H
#define HAL_I2C_CFG_H

#include <stdint.h>
#include "hal_i2c.h" // Include the main I2C HAL interface for enums and types

/**
 * @brief Structure to hold compile-time configuration for a single I2C port.
 *
 * This structure defines various parameters that configure the I2C port's behavior
 * when operating in master mode, such as pin assignments, clock speed, and timeout.
 */
typedef struct
{
    HAL_I2C_Port_t port_id;             /**< The logical ID of the I2C port. */
    int scl_gpio_num;                   /**< GPIO pin number for SCL. */
    int sda_gpio_num;                   /**< GPIO pin number for SDA. */
    HAL_I2C_ClockSpeed_t clk_speed_hz;  /**< Master clock speed in Hz. */
    uint32_t timeout_ms;                /**< Default bus timeout in milliseconds. */
    bool pullup_en;                     /**< Enable internal pull-up resistors on SDA/SCL. */
} HAL_I2C_PortConfig_t;

/**
 * @brief External declaration for the global array of I2C port configurations.
 *
 * This array is defined in `hal_i2c_cfg.c` and provides the static
 * configuration for all available I2C ports.
 */
extern const HAL_I2C_PortConfig_t g_hal_i2c_port_configs[HAL_I2C_PORT_MAX];

#endif /* HAL_I2C_CFG_H */
