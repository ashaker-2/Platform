/**
 * @file hal_i2c_cfg.c
 * @brief Hardware Abstraction Layer for I2C - Configuration Implementation.
 *
 * This file implements the static configuration parameters for each I2C port.
 * These settings are used by the `hal_i2c.c` implementation to initialize
 * and operate the I2C functionalities.
 *
 * IMPORTANT: Adjust these values according to your specific application's
 * hardware connections and desired I2C bus behavior.
 */

#include "hal_i2c_cfg.h"
#include "hal_i2c.h" // For HAL_I2C_Port_t and other enums

// Define the global array of I2C port configurations
const HAL_I2C_PortConfig_t g_hal_i2c_port_configs[HAL_I2C_PORT_MAX] = {
    [HAL_I2C_PORT_0] = {
        .port_id = HAL_I2C_PORT_0,
        .scl_gpio_num = 22,                     // Example: GPIO 22 for SCL on I2C_NUM_0
        .sda_gpio_num = 21,                     // Example: GPIO 21 for SDA on I2C_NUM_0
        .clk_speed_hz = HAL_I2C_CLK_SPEED_100KHZ, // Default to 100 kHz
        .timeout_ms = 1000,                     // Default timeout of 1 second
        .pullup_en = true,                      // Enable internal pull-ups
    },
    [HAL_I2C_PORT_1] = {
        .port_id = HAL_I2C_PORT_1,
        .scl_gpio_num = 26,                     // Example: GPIO 26 for SCL on I2C_NUM_1
        .sda_gpio_num = 27,                     // Example: GPIO 27 for SDA on I2C_NUM_1
        .clk_speed_hz = HAL_I2C_CLK_SPEED_400KHZ, // Default to 400 kHz
        .timeout_ms = 500,                      // Default timeout of 0.5 seconds
        .pullup_en = true,                      // Enable internal pull-ups
    },
    // Add configurations for more I2C ports if defined in hal_i2c.h
};

