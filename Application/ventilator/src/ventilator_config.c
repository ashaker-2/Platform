// app/src/ventilator_config.c

#include "ventilator_config.h"

// Definition of the static constant array for Ventilator configurations
const VENTILATOR_Config_t ventilator_configurations[] = {
    // VENTILATOR_EXHAUST_FAN
    {
        .control_gpio_id = ECUAL_GPIO_VENTILATOR_EXHAUST, // Assign a specific ECUAL_GPIO_ID
        .active_state    = ECUAL_GPIO_HIGH                 // HIGH state means the ventilator is ON
    }
    // Add more ventilator configurations here if you have multiple units:
    // {
    //     .control_gpio_id = ECUAL_GPIO_VENTILATOR_SUPPLY,
    //     .active_state    = ECUAL_GPIO_LOW // Maybe this one is active LOW
    // }
};

// Definition of the number of configurations in the array
const uint32_t VENTILATOR_NUM_CONFIGURATIONS = sizeof(ventilator_configurations) / sizeof(VENTILATOR_Config_t);
