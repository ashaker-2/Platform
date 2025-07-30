// app/src/fan.c

#include "fan.h" // Updated include
#include "fan_config.h" // Updated include
#include "ecual_pwm.h"
#include "ecual_gpio.h"
#include "esp_log.h"

static const char *TAG = "FAN"; // Renamed log tag

// Static array to store the current commanded speed for each fan
static uint8_t current_fan_speeds[FAN_TOTAL_FANS]; // Renamed enum

// Helper to get fan config by ID
static const FAN_Config_t* fan_get_config(FAN_ID_t fan_id) { // Renamed helper function and struct
    if (fan_id >= FAN_TOTAL_FANS) { // Renamed enum
        ESP_LOGE(TAG, "Invalid Fan ID: %u", fan_id);
        return NULL;
    }
    return &fan_configurations[fan_id];
}

uint8_t FAN_Init(void) { // Renamed function
    uint8_t app_ret = APP_OK;
    uint8_t ecual_ret;

    // First, ensure the underlying ECUAL modules are initialized if they haven't been already
    // (This is usually done in main(), but good to ensure dependencies)
    // ecual_ret = ECUAL_PWM_Init();
    // if (ecual_ret != ECUAL_OK) {
    //     ESP_LOGE(TAG, "Failed to initialize ECUAL PWM.");
    //     return APP_ERROR;
    // }
    // ecual_ret = ECUAL_GPIO_Init();
    // if (ecual_ret != ECUAL_OK) {
    //     ESP_LOGE(TAG, "Failed to initialize ECUAL GPIO.");
    //     return APP_ERROR;
    // }

    for (uint32_t i = 0; i < FAN_NUM_CONFIGURATIONS; i++) { // Renamed constant
        const FAN_Config_t *cfg = &fan_configurations[i]; // Renamed struct
        current_fan_speeds[i] = 0; // Initialize commanded speed to 0 (off)

        // Ensure the PWM channel is ready
        // No explicit PWM channel init here, as it's part of ECUAL_PWM_Init.
        // We just ensure we have a valid channel ID.
        if (cfg->pwm_channel_id >= ECUAL_PWM_TOTAL_CHANNELS) {
            ESP_LOGE(TAG, "Fan %u: Invalid PWM Channel ID %u in config.", i, cfg->pwm_channel_id);
            app_ret = APP_ERROR;
            continue;
        }

        // Configure enable GPIO if present
        if (cfg->has_enable_gpio) {
            // ECUAL_GPIO_SetDirection(cfg->enable_gpio_id, ECUAL_GPIO_OUTPUT); // Direction handled by ECUAL_GPIO_Init
            // Ensure initial state is off
            ecual_ret = ECUAL_GPIO_SetState(cfg->enable_gpio_id, ECUAL_GPIO_LOW);
            if (ecual_ret != ECUAL_OK) {
                ESP_LOGE(TAG, "Fan %u: Failed to set enable GPIO %u low.", i, cfg->enable_gpio_id);
                app_ret = APP_ERROR;
            }
        }

        // Set initial PWM duty to 0 (off)
        ecual_ret = ECUAL_PWM_SetDuty(cfg->pwm_channel_id, 0);
        if (ecual_ret != ECUAL_OK) {
            ESP_LOGE(TAG, "Fan %u: Failed to set initial PWM duty to 0 for channel %u.", i, cfg->pwm_channel_id);
            app_ret = APP_ERROR;
        }

        ESP_LOGI(TAG, "Fan %u initialized. PWM Channel: %u, Enable GPIO: %s%u, Min/Max Speed: %lu/%lu%%",
                 i, cfg->pwm_channel_id, (cfg->has_enable_gpio ? "" : "N/A:"), cfg->enable_gpio_id,
                 cfg->min_speed_duty_percent, cfg->max_speed_duty_percent);
    }

    return app_ret;
}

uint8_t FAN_SetSpeed(FAN_ID_t fan_id, uint8_t speed_percent) { // Renamed function and enum
    const FAN_Config_t *cfg = fan_get_config(fan_id); // Renamed struct and helper
    if (cfg == NULL) { return APP_ERROR; }

    uint8_t app_ret = APP_OK;
    uint8_t ecual_ret;

    // Clip speed_percent to valid range (0-100)
    if (speed_percent > 100) speed_percent = 100;

    uint32_t actual_duty_percent;
    if (speed_percent == 0) {
        actual_duty_percent = 0; // Always allow 0% to turn off
    } else {
        // Scale the 1-100 input to the effective min-max range
        actual_duty_percent = cfg->min_speed_duty_percent +
                                  (speed_percent * (cfg->max_speed_duty_percent - cfg->min_speed_duty_percent) / 100);
        if (actual_duty_percent > cfg->max_speed_duty_percent) actual_duty_percent = cfg->max_speed_duty_percent;
        if (actual_duty_percent < cfg->min_speed_duty_percent) actual_duty_percent = cfg->min_speed_duty_percent;
    }


    // Control enable GPIO if present
    if (cfg->has_enable_gpio) {
        ECUAL_GPIO_State_t enable_state = (actual_duty_percent > 0) ? ECUAL_GPIO_HIGH : ECUAL_GPIO_LOW;
        ecual_ret = ECUAL_GPIO_SetState(cfg->enable_gpio_id, enable_state);
        if (ecual_ret != ECUAL_OK) {
            ESP_LOGE(TAG, "Fan %u: Failed to set enable GPIO %u state.", fan_id, cfg->enable_gpio_id);
            app_ret = APP_ERROR;
        }
    }

    // Set PWM duty cycle
    ecual_ret = ECUAL_PWM_SetDuty(cfg->pwm_channel_id, actual_duty_percent);
    if (ecual_ret != ECUAL_OK) {
        ESP_LOGE(TAG, "Fan %u: Failed to set PWM duty to %lu for channel %u.", fan_id, actual_duty_percent, cfg->pwm_channel_id);
        app_ret = APP_ERROR;
    } else {
        current_fan_speeds[fan_id] = speed_percent; // Only update if PWM set was successful
        ESP_LOGI(TAG, "Fan %u speed set to %u%% (actual PWM duty %lu%%)", fan_id, speed_percent, actual_duty_percent);
    }

    return app_ret;
}

uint8_t FAN_Start(FAN_ID_t fan_id) { // Renamed function and enum
    const FAN_Config_t *cfg = fan_get_config(fan_id); // Renamed struct and helper
    if (cfg == NULL) { return APP_ERROR; }

    ESP_LOGI(TAG, "Starting Fan %u...", fan_id);
    // Start at min speed, which ensures it spins up
    return FAN_SetSpeed(fan_id, cfg->min_speed_duty_percent); // Renamed function
}

uint8_t FAN_Stop(FAN_ID_t fan_id) { // Renamed function and enum
    ESP_LOGI(TAG, "Stopping Fan %u...", fan_id);
    return FAN_SetSpeed(fan_id, 0); // Renamed function
}

uint8_t FAN_GetSpeed(FAN_ID_t fan_id, uint8_t *speed_percent) { // Renamed function and enum
    if (fan_id >= FAN_TOTAL_FANS || speed_percent == NULL) { // Renamed enum
        ESP_LOGE(TAG, "Invalid Fan ID or NULL pointer for GetSpeed.");
        return APP_ERROR;
    }
    *speed_percent = current_fan_speeds[fan_id];
    return APP_OK;
}
