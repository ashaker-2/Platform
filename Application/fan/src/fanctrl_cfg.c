// app/src/fanctrl_cfg.c

#include "fanctrl_cfg.h"



// Fan configurations array definition
const FAN_Config_t fan_configurations[] = {
    // FAN_MOTOR_1 (PWM Controlled Fan)
    {
        .fan_type               = FAN_TYPE_PWM,
        .control_gpio_id        = 0,     // Not used as primary control for this type
        .has_aux_gpio_control   = 0, // No separate enable GPIO for this PWM fan
        .on_off_gpio_active_state = 1,    // Not applicable for PWM type (can be ignored)
        .pwm_channel_id         = 1,     // This should correspond to an ID defined in ecual_pwm_config.h
        .min_speed_duty_percent = 20,    // Fan starts spinning reliably at 20% PWM duty
        .max_speed_duty_percent = 90     // Max desired speed at 90% PWM duty (to reduce noise perhaps)
    },
    // FAN_COOLING_FAN (ON/OFF Fan)
    {
        .fan_type               = FAN_TYPE_ON_OFF,
        .control_gpio_id        = 1,     // GPIO ID for the ON/OFF control
        .has_aux_gpio_control   = 0, // No aux GPIO control for this type
        .on_off_gpio_active_state = 1,    // HIGH state means fan is ON
        .pwm_channel_id         = 0,     // Not applicable for ON/OFF type
        .min_speed_duty_percent = 0,     // Not applicable for ON/OFF type
        .max_speed_duty_percent = 0      // Not applicable for ON/OFF type
    }
    // Add more fan configurations here if needed
};

// Number of fan configurations
const uint32_t FAN_NUM_CONFIGURATIONS = sizeof(fan_configurations) / sizeof(fan_configurations[0]);

