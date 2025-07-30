// app/src/fan.c

#include "fan.h"
#include "fan_config.h"
#include "ecual_pwm.h"
#include "ecual_gpio.h"
#include "esp_log.h"

static const char *TAG = "FAN";

// Static array to store the current commanded speed for each fan
// For ON/OFF fans: 0 = OFF, 100 = ON
static uint8_t current_fan_speeds[FAN_TOTAL_FANS];

// Helper to get fan config by ID
static const FAN_Config_t* fan_get_config(FAN_ID_t fan_id) {
    if (fan_id >= FAN_TOTAL_FANS) {
        ESP_LOGE(TAG, "Invalid Fan ID: %u", fan_id);
        return NULL;
    }
    return &fan_configurations[fan_id];
}

uint8_t FAN_Init(void) {
    uint8_t app_ret = APP_OK;
    uint8_t ecual_ret;

    for (uint32_t i = 0; i < FAN_NUM_CONFIGURATIONS; i++) {
        const FAN_Config_t *cfg = &fan_configurations[i];
        current_fan_speeds[i] = 0; // Initialize commanded speed to 0 (off)

        ESP_LOGI(TAG, "Initializing Fan %u (Type: %s)...", i,
                 (cfg->fan_type == FAN_TYPE_ON_OFF) ? "ON/OFF" : "PWM");

        switch (cfg->fan_type) {
            case FAN_TYPE_ON_OFF:
                // For ON/OFF fans, the control_gpio_id is the primary control.
                // It should already be configured as OUTPUT by ECUAL_GPIO_Init.
                // Set initial state to inactive.
                ecual_ret = ECUAL_GPIO_SetState(cfg->control_gpio_id,
                                                (cfg->on_off_gpio_active_state == ECUAL_GPIO_HIGH) ? ECUAL_GPIO_LOW : ECUAL_GPIO_HIGH);
                if (ecual_ret != ECUAL_OK) {
                    ESP_LOGE(TAG, "Fan %u: Failed to set initial state for ON/OFF GPIO %u.", i, cfg->control_gpio_id);
                    app_ret = APP_ERROR;
                }
                break;

            case FAN_TYPE_PWM_SPEED_CONTROL:
                // Ensure the PWM channel ID is valid
                if (cfg->pwm_channel_id >= ECUAL_PWM_TOTAL_CHANNELS) {
                    ESP_LOGE(TAG, "Fan %u: Invalid PWM Channel ID %u in config.", i, cfg->pwm_channel_id);
                    app_ret = APP_ERROR;
                }

                // If there's an auxiliary enable GPIO for the PWM fan
                if (cfg->has_aux_gpio_control) {
                    // Ensure initial state is off
                    ecual_ret = ECUAL_GPIO_SetState(cfg->control_gpio_id, ECUAL_GPIO_LOW);
                    if (ecual_ret != ECUAL_OK) {
                        ESP_LOGE(TAG, "Fan %u: Failed to set auxiliary enable GPIO %u low.", i, cfg->control_gpio_id);
                        app_ret = APP_ERROR;
                    }
                }

                // Set initial PWM duty to 0 (off)
                ecual_ret = ECUAL_PWM_SetDuty(cfg->pwm_channel_id, 0);
                if (ecual_ret != ECUAL_OK) {
                    ESP_LOGE(TAG, "Fan %u: Failed to set initial PWM duty to 0 for channel %u.", i, cfg->pwm_channel_id);
                    app_ret = APP_ERROR;
                }
                break;

            default:
                ESP_LOGE(TAG, "Fan %u: Unknown fan type %u in config.", i, cfg->fan_type);
                app_ret = APP_ERROR;
                break;
        }
    }

    return app_ret;
}

uint8_t FAN_SetSpeed(FAN_ID_t fan_id, uint8_t speed_percent) {
    const FAN_Config_t *cfg = fan_get_config(fan_id);
    if (cfg == NULL) { return APP_ERROR; }

    uint8_t app_ret = APP_OK;
    uint8_t ecual_ret;

    // Clip speed_percent to valid range (0-100)
    if (speed_percent > 100) speed_percent = 100;

    switch (cfg->fan_type) {
        case FAN_TYPE_ON_OFF: {
            ECUAL_GPIO_State_t desired_state;
            if (speed_percent > 0) {
                // Fan ON
                desired_state = cfg->on_off_gpio_active_state;
                current_fan_speeds[fan_id] = 100; // For ON/OFF, report 100% when ON
                ESP_LOGI(TAG, "Fan %u (ON/OFF) set to ON (speed_percent: %u)", fan_id, speed_percent);
            } else {
                // Fan OFF
                desired_state = (cfg->on_off_gpio_active_state == ECUAL_GPIO_HIGH) ? ECUAL_GPIO_LOW : ECUAL_GPIO_HIGH;
                current_fan_speeds[fan_id] = 0; // For ON/OFF, report 0% when OFF
                ESP_LOGI(TAG, "Fan %u (ON/OFF) set to OFF (speed_percent: %u)", fan_id, speed_percent);
            }
            ecual_ret = ECUAL_GPIO_SetState(cfg->control_gpio_id, desired_state);
            if (ecual_ret != ECUAL_OK) {
                ESP_LOGE(TAG, "Fan %u: Failed to set ON/OFF GPIO %u state.", fan_id, cfg->control_gpio_id);
                app_ret = APP_ERROR;
            }
            break;
        }

        case FAN_TYPE_PWM_SPEED_CONTROL: {
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

            // Control auxiliary enable GPIO if present
            if (cfg->has_aux_gpio_control) {
                ECUAL_GPIO_State_t enable_state = (actual_duty_percent > 0) ? ECUAL_GPIO_HIGH : ECUAL_GPIO_LOW;
                ecual_ret = ECUAL_GPIO_SetState(cfg->control_gpio_id, enable_state);
                if (ecual_ret != ECUAL_OK) {
                    ESP_LOGE(TAG, "Fan %u: Failed to set auxiliary enable GPIO %u state.", fan_id, cfg->control_gpio_id);
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
                ESP_LOGI(TAG, "Fan %u (PWM) speed set to %u%% (actual PWM duty %lu%%)", fan_id, speed_percent, actual_duty_percent);
            }
            break;
        }

        default:
            ESP_LOGE(TAG, "Fan %u: Unhandled fan type %u for SetSpeed.", fan_id, cfg->fan_type);
            app_ret = APP_ERROR;
            break;
    }

    return app_ret;
}

uint8_t FAN_Start(FAN_ID_t fan_id) {
    const FAN_Config_t *cfg = fan_get_config(fan_id);
    if (cfg == NULL) { return APP_ERROR; }

    ESP_LOGI(TAG, "Starting Fan %u...", fan_id);
    switch (cfg->fan_type) {
        case FAN_TYPE_ON_OFF:
            return FAN_SetSpeed(fan_id, 1); // Any non-zero speed turns it ON
        case FAN_TYPE_PWM_SPEED_CONTROL:
            return FAN_SetSpeed(fan_id, cfg->min_speed_duty_percent); // Start at min speed for PWM
        default:
            return APP_ERROR;
    }
}

uint8_t FAN_Stop(FAN_ID_t fan_id) {
    ESP_LOGI(TAG, "Stopping Fan %u...", fan_id);
    return FAN_SetSpeed(fan_id, 0); // Set speed to 0% for both types
}

uint8_t FAN_GetSpeed(FAN_ID_t fan_id, uint8_t *speed_percent) {
    if (fan_id >= FAN_TOTAL_FANS || speed_percent == NULL) {
        ESP_LOGE(TAG, "Invalid Fan ID or NULL pointer for GetSpeed.");
        return APP_ERROR;
    }
    *speed_percent = current_fan_speeds[fan_id];
    return APP_OK;
}
