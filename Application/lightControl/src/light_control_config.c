// app/src/light_control_config.c

#include "light_control_config.h"

// Definition of the static constant array for Light configurations
const LIGHT_Config_t light_configurations[] = {
    // LIGHT_KITCHEN (ON/OFF type)
    {
        .type = LIGHT_TYPE_ON_OFF,
        .on_off_cfg = {
            .gpio_id      = ECUAL_GPIO_LIGHT_KITCHEN_CONTROL,
            .active_state = ECUAL_GPIO_HIGH // Assuming HIGH turns the light ON
        }
    },
    // LIGHT_LIVING_ROOM (Dimmable type)
    {
        .type = LIGHT_TYPE_DIMMABLE,
        .dimmable_cfg = {
            .pwm_channel = ECUAL_PWM_CHANNEL_LIVING_ROOM // Assign a specific ECUAL PWM channel
        }
    },
    // LIGHT_BEDROOM (ON/OFF type)
    {
        .type = LIGHT_TYPE_ON_OFF,
        .on_off_cfg = {
            .gpio_id      = ECUAL_GPIO_LIGHT_BEDROOM_CONTROL,
            .active_state = ECUAL_GPIO_LOW // Example: Assuming LOW turns this light ON
        }
    }
};

const uint32_t LIGHT_NUM_CONFIGURATIONS = sizeof(light_configurations) / sizeof(LIGHT_Config_t);
