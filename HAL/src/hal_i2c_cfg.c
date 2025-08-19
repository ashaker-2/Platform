/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/src/HAL_I2C_Cfg.c
 * ============================================================================*/
/**
 * @file HAL_I2C_Cfg.c
 * @brief Implements the static array of I2C bus configuration settings.
 * This file defines the specific initial modes, clock speeds, and pins for
 * all I2C buses used by the hardware. It does not contain any initialization
 * functions; its purpose is purely to hold configuration data.
 */

#include "HAL_I2C_Cfg.h" // Header for I2C configuration types and extern declarations
#include "hal_cfg.h"     // Global hardware pin definitions (e.g., HW_I2C_EXPANDER_PORT)
#include <stddef.h>      // For size_t

/**
 * @brief Array containing all predefined I2C bus configurations.
 * Currently, it includes the configuration for the I2C expander bus.
 * This array is made `const` and global (`extern` in header) to be accessed by `HAL_I2C.c`.
 */
const i2c_cfg_item_t s_i2c_configurations[] = {
    // Configuration for the I2C bus connected to the CH423S expander
    {
        .port = HW_I2C_EXPANDER_PORT,
        .config = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = HW_I2C_EXPANDER_SDA_GPIO,
            .scl_io_num = HW_I2C_EXPANDER_SCL_GPIO,
            .sda_pullup_en = GPIO_PULLUP_ENABLE, // Internal pull-ups, though external are preferred for robust I2C
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = (400 * 1000), // 400 KHz
            .clk_flags = 0,                   // No specific flags
        },
    },
    // Add other I2C bus configurations here if needed for future expansion
};

/**
 * @brief Defines the number of elements in the `s_i2c_configurations` array.
 * This variable is made `const` and global (`extern` in header) to be accessed by `HAL_I2C.c`.
 */
const size_t s_num_i2c_configurations = sizeof(s_i2c_configurations) / sizeof(s_i2c_configurations[0]);
