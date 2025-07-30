// app/src/pump_config.c

#include "pump_config.h"
#include "ecual_gpio.h" // Ensure ECUAL_GPIO_NONE is accessible

// Definition of the static constant array for Pump configurations
const PUMP_Config_t pump_configurations[] = {
    // PUMP_WATER_CIRCULATION (with feedback)
    {
        .control_gpio_id = ECUAL_GPIO_PUMP_WATER_CIRCULATION_CONTROL,
        .active_state    = ECUAL_GPIO_HIGH,
        .has_feedback_gpio = true,
        .feedback_gpio_id  = ECUAL_GPIO_PUMP_WATER_CIRCULATION_FEEDBACK,
        .feedback_active_state = ECUAL_GPIO_HIGH // Assuming HIGH means working
    },
    // PUMP_DRAINAGE (without feedback)
    {
        .control_gpio_id = ECUAL_GPIO_PUMP_DRAINAGE_CONTROL,
        .active_state    = ECUAL_GPIO_LOW, // Example: Assuming LOW turns this pump ON
        .has_feedback_gpio = false,
        .feedback_gpio_id  = ECUAL_GPIO_NONE,
        .feedback_active_state = ECUAL_GPIO_LOW // Not applicable
    },
    // PUMP_IRRIGATION (with feedback)
    {
        .control_gpio_id = ECUAL_GPIO_PUMP_IRRIGATION_CONTROL,
        .active_state    = ECUAL_GPIO_HIGH,
        .has_feedback_gpio = true,
        .feedback_gpio_id  = ECUAL_GPIO_PUMP_IRRIGATION_FEEDBACK,
        .feedback_active_state = ECUAL_GPIO_HIGH
    }
};

const uint32_t PUMP_NUM_CONFIGURATIONS = sizeof(pump_configurations) / sizeof(PUMP_Config_t);
