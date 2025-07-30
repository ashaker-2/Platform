// app/src/fan_config.c

#include "fan_config.h" // Updated include

// Definition of the static constant array for Fan configurations
const FAN_Config_t fan_configurations[] = { // Renamed struct
    // FAN_MOTOR_1
    {
        .pwm_channel_id         = ECUAL_PWM_MOTOR_LEFT, // This should correspond to an ID defined in ecual_pwm_config.h
        .has_enable_gpio        = false, // Assuming fan speed is solely controlled by PWM duty cycle
        .has_enable_gpio        = false, // Assuming fan speed is solely controlled by PWM duty cycle
        .enable_gpio_id         = 0,     // Not used if has_enable_gpio is false
        .min_speed_duty_percent = 20,    // Fan starts spinning reliably at 20% PWM duty
        .max_speed_duty_percent = 90     // Max desired speed at 90% PWM duty (to reduce noise perhaps)
    }
    // Add more fan configurations here if you have multiple fans:
    // {
    //     .pwm_channel_id         = ECUAL_PWM_MOTOR_RIGHT,
    //     .has_enable_gpio        = true,
    //     .enable_gpio_id         = ECUAL_GPIO_FAN_ENABLE_2, // An ECUAL GPIO ID for a separate enable pin
    //     .min_speed_duty_percent = 15,
    //     .max_speed_duty_percent = 100
    // }
};

// Definition of the number of configurations in the array
const uint32_t FAN_NUM_CONFIGURATIONS = sizeof(fan_configurations) / sizeof(FAN_Config_t); // Renamed constant and struct
