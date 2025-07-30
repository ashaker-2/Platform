// app/src/character_display_config.c

#include "character_display_config.h"

// Definition of the static constant array for Character Display configurations
const CHARACTER_DISPLAY_Config_t character_display_configurations[] = {
    // CHARACTER_DISPLAY_MAIN_STATUS (Example: 16x2 LCD)
    {
        .i2c_id                = ECUAL_I2C_MASTER_0,   // Assuming your I2C master is ECUAL_I2C_MASTER_0
        .i2c_address           = 0x27,                 // Common PCF8574 address, verify for your module
        .num_columns           = 16,
        .num_rows              = 2,
        .has_backlight         = true,
        .backlight_active_high = true                  // Common for PCF8574 backpack
    },
    // CHARACTER_DISPLAY_ALARM_PANEL (Example: 20x4 LCD)
    {
        .i2c_id                = ECUAL_I2C_MASTER_0,   // Can be on the same bus or a different one
        .i2c_address           = 0x3F,                 // Another common PCF8574 address
        .num_columns           = 20,
        .num_rows              = 4,
        .has_backlight         = true,
        .backlight_active_high = true
    }
};

const uint32_t CHARACTER_DISPLAY_NUM_CONFIGURATIONS = sizeof(character_display_configurations) / sizeof(CHARACTER_DISPLAY_Config_t);
