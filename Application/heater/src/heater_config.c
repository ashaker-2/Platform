// app/src/heater_config.c

#include "heater_config.h"
#include "ecual_gpio.h" // Ensure ECUAL_GPIO_NONE is accessible

// Definition of the static constant array for Heater configurations
const HEATER_Config_t heater_configurations[] = {
    // HEATER_ROOM (with feedback)
    {
        .control_gpio_id = ECUAL_GPIO_HEATER_ROOM_CONTROL,
        .active_state    = ECUAL_GPIO_HIGH,
        .has_feedback_gpio = true,
        .feedback_gpio_id  = ECUAL_GPIO_HEATER_ROOM_FEEDBACK, // New GPIO for feedback
        .feedback_active_state = ECUAL_GPIO_HIGH              // Assuming HIGH means working
    },
    // HEATER_WATER_TANK (without feedback)
    {
        .control_gpio_id = ECUAL_GPIO_HEATER_WATER_TANK_CONTROL,
        .active_state    = ECUAL_GPIO_LOW,                    // Example: Assuming LOW turns this heater ON
        .has_feedback_gpio = false,                           // No feedback for this heater
        .feedback_gpio_id  = ECUAL_GPIO_NONE,                 // Not applicable
        .feedback_active_state = ECUAL_GPIO_LOW               // Not applicable
    },
    // HEATER_GARAGE (New example, with feedback)
    {
        .control_gpio_id = ECUAL_GPIO_HEATER_GARAGE_CONTROL,
        .active_state    = ECUAL_GPIO_HIGH,
        .has_feedback_gpio = true,
        .feedback_gpio_id  = ECUAL_GPIO_HEATER_GARAGE_FEEDBACK,
        .feedback_active_state = ECUAL_GPIO_LOW             // Example: Assuming LOW means working for this one
    }
};

// Definition of the number of configurations in the array
const uint32_t HEATER_NUM_CONFIGURATIONS = sizeof(heater_configurations) / sizeof(HEATER_Config_t);
