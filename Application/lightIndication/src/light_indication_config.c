// app/src/light_indication_config.c

#include "light_indication_config.h"

// Definition of the static constant array for Light Indication configurations
const LIGHT_INDICATION_Config_t light_indication_configurations[] = {
    // LIGHT_INDICATION_SYSTEM_STATUS
    {
        .gpio_id      = ECUAL_GPIO_LED_SYSTEM_STATUS, // Assign a specific ECUAL GPIO ID
        .active_state = ECUAL_GPIO_HIGH                 // Assuming HIGH turns the LED ON
    },
    // LIGHT_INDICATION_ERROR
    {
        .gpio_id      = ECUAL_GPIO_LED_ERROR,         // Assign another ECUAL GPIO ID
        .active_state = ECUAL_GPIO_LOW                // Example: Assuming LOW turns this LED ON (e.g., common anode)
    },
    // LIGHT_INDICATION_WIFI_STATUS
    {
        .gpio_id      = ECUAL_GPIO_LED_WIFI_STATUS,   // Another ECUAL GPIO ID
        .active_state = ECUAL_GPIO_HIGH
    }
    // Add more LED configurations here as needed
};

// Definition of the number of configurations in the array
const uint32_t LIGHT_INDICATION_NUM_CONFIGURATIONS = sizeof(light_indication_configurations) / sizeof(LIGHT_INDICATION_Config_t);
