// app/src/fan_config.c

#include "fan_config.h"

// Definition of the static constant array for Fan configurations
const FAN_Config_t fan_configurations[] = {
    // FAN_MOTOR_1 (PWM Controlled Fan)
    {
        .fan_type               = FAN_TYPE_PWM_SPEED_CONTROL,
        .control_gpio_id        = 0,     // Not used as primary control for this type
        .has_aux_gpio_control   = false, // No separate enable GPIO for this PWM fan
        .on_off_gpio_active_state = ECUAL_GPIO_HIGH, // Not applicable for PWM type
        .pwm_channel_id         = ECUAL_PWM_MOTOR_LEFT, // This should correspond to an ID defined in ecual_pwm_config.h
        .min_speed_duty_percent = 20,    // Fan starts spinning reliably at 20% PWM duty
        .max_speed_duty_percent = 90     // Max desired speed at 90% PWM duty (to reduce noise perhaps)
    },
    // FAN_COOLING_FAN (ON/OFF Fan)
    {
        .fan_type               = FAN_TYPE_ON_OFF,
        .control_gpio_id        = ECUAL_GPIO_FAN_COOLING_ENABLE, // Assuming this ECUAL_GPIO_ID is defined
        .has_aux_gpio_control   = false, // Primary control is via this GPIO, not auxiliary
        .on_off_gpio_active_state = ECUAL_GPIO_HIGH, // HIGH state means fan is ON
        .pwm_channel_id         = 0,     // Not applicable for ON/OFF type
        .min_speed_duty_percent = 0,     // Not applicable for ON/OFF type
        .max_speed_duty_percent = 0      // Not applicable for ON/OFF type
    }
    // Add more fan configurations here if you have multiple fans:
};

// Definition of the number of configurations in the array
const uint32_t FAN_NUM_CONFIGURATIONS = sizeof(fan_configurations) / sizeof(FAN_Config_t);
