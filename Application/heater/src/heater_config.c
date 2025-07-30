// app/src/heater_config.c

#include "heater_config.h"

// Definition of the static constant array for Heater configurations
const HEATER_Config_t heater_configurations[] = {
    // HEATER_ROOM
    {
        .control_gpio_id = ECUAL_GPIO_HEATER_ROOM_CONTROL, // Assign a specific ECUAL GPIO ID
        .active_state    = ECUAL_GPIO_HIGH                 // Assuming HIGH turns the heater ON
    },
    // HEATER_WATER_TANK
    {
        .control_gpio_id = ECUAL_GPIO_HEATER_WATER_TANK_CONTROL, // Assign another ECUAL GPIO ID
        .active_state    = ECUAL_GPIO_LOW                      // Example: Assuming LOW turns this heater ON
    }
    // Add more heater configurations here as needed
};

// Definition of the number of configurations in the array
const uint32_t HEATER_NUM_CONFIGURATIONS = sizeof(heater_configurations) / sizeof(HEATER_Config_t);
