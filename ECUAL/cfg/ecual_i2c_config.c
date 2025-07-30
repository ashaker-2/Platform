// ecual/src/ecual_i2c_config.c

#include "ecual_i2c_config.h" // Includes ecual_i2c.h for ECUAL_I2C_Config_t definition
#include "driver/gpio.h"      // For GPIO_NUM_X definitions

// Definition of the static constant array for I2C configurations
const ECUAL_I2C_Config_t i2c_configurations[] = {
    // I2C Unit 0 for Sensors (e.g., IMU, pressure sensor)
    {
        .unit = ECUAL_I2C_SENSOR_BUS,
        .mode = ECUAL_I2C_MODE_MASTER,
        .scl_pin = GPIO_NUM_22, // Typical SCL pin for I2C0
        .sda_pin = GPIO_NUM_21, // Typical SDA pin for I2C0
        .scl_pullup_en = true,  // Enable internal pull-ups (external recommended for robust bus)
        .sda_pullup_en = true,
        .master_freq_hz = 400000, // Fast Mode I2C
    },
    // I2C Unit 1 for Display (e.g., OLED)
    {
        .unit = ECUAL_I2C_DISPLAY_BUS,
        .mode = ECUAL_I2C_MODE_MASTER,
        .scl_pin = GPIO_NUM_25, // Example custom SCL pin for I2C1
        .sda_pin = GPIO_NUM_26, // Example custom SDA pin for I2C1
        .scl_pullup_en = true,
        .sda_pullup_en = true,
        .master_freq_hz = 100000, // Standard Mode I2C
    }
};

// Definition of the number of configurations in the array
const uint32_t ECUAL_NUM_I2C_CONFIGURATIONS = sizeof(i2c_configurations) / sizeof(ECUAL_I2C_Config_t);
