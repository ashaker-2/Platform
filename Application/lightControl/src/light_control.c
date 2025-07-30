// app/src/light_control.c

#include "light_control.h"
#include "light_control_config.h"
#include "ecual_gpio.h"
#include "ecual_pwm.h"
#include "esp_log.h"

static const char *TAG = "LIGHT_CONTROL";

// Array to store the current commanded brightness for each light (0-100%)
static uint8_t current_light_brightness[LIGHT_TOTAL_UNITS];

/**
 * @brief Helper function to get the configuration for a specific light ID.
 * @param unit_id The LIGHT_ID_t of the light unit.
 * @return Pointer to the configuration struct, or NULL if ID is invalid.
 */
static const LIGHT_Config_t* light_get_config(LIGHT_ID_t unit_id) {
    if (unit_id >= LIGHT_TOTAL_UNITS) {
        ESP_LOGE(TAG, "Invalid Light ID: %u", unit_id);
        return NULL;
    }
    return &light_configurations[unit_id];
}

uint8_t LIGHT_Init(void) {
    uint8_t app_ret = APP_OK;

    for (uint32_t i = 0; i < LIGHT_NUM_CONFIGURATIONS; i++) {
        const LIGHT_Config_t *cfg = &light_configurations[i];
        current_light_brightness[i] = 0; // Initialize all lights to OFF (0% brightness)

        switch (cfg->type) {
            case LIGHT_TYPE_ON_OFF: {
                // For ON/OFF lights, set initial state to inactive (OFF)
                ECUAL_GPIO_State_t initial_state = (cfg->on_off_cfg.active_state == ECUAL_GPIO_HIGH) ? ECUAL_GPIO_LOW : ECUAL_GPIO_HIGH;
                if (ECUAL_GPIO_SetState(cfg->on_off_cfg.gpio_id, initial_state) != ECUAL_OK) {
                    ESP_LOGE(TAG, "Light %u (ON/OFF): Failed to set initial state for GPIO %u.", i, cfg->on_off_cfg.gpio_id);
                    app_ret = APP_ERROR;
                }
                ESP_LOGI(TAG, "Light %u (ON/OFF) initialized. GPIO: %u, Active State: %s (Initial State: OFF)",
                         i, cfg->on_off_cfg.gpio_id, (cfg->on_off_cfg.active_state == ECUAL_GPIO_HIGH) ? "HIGH" : "LOW");
                break;
            }
            case LIGHT_TYPE_DIMMABLE: {
                // For dimmable lights, set initial duty cycle to 0%
                if (ECUAL_PWM_SetDuty(cfg->dimmable_cfg.pwm_channel, 0) != ECUAL_OK) {
                    ESP_LOGE(TAG, "Light %u (Dimmable): Failed to set initial duty for PWM channel %u.", i, cfg->dimmable_cfg.pwm_channel);
                    app_ret = APP_ERROR;
                }
                ESP_LOGI(TAG, "Light %u (Dimmable) initialized. PWM Channel: %u (Initial Brightness: 0%%)",
                         i, cfg->dimmable_cfg.pwm_channel);
                break;
            }
            default:
                ESP_LOGE(TAG, "Light %u: Unknown light type: %d.", i, cfg->type);
                app_ret = APP_ERROR;
                break;
        }
    }
    return app_ret;
}

uint8_t LIGHT_On(LIGHT_ID_t unit_id) {
    return LIGHT_SetBrightness(unit_id, 100); // Set to 100% brightness (or active state for ON/OFF)
}

uint8_t LIGHT_Off(LIGHT_ID_t unit_id) {
    return LIGHT_SetBrightness(unit_id, 0); // Set to 0% brightness (or inactive state for ON/OFF)
}

uint8_t LIGHT_SetBrightness(LIGHT_ID_t unit_id, uint8_t percentage) {
    const LIGHT_Config_t *cfg = light_get_config(unit_id);
    if (cfg == NULL) { return APP_ERROR; }

    if (percentage > 100) { percentage = 100; } // Cap at 100%

    uint8_t app_ret = APP_OK;

    switch (cfg->type) {
        case LIGHT_TYPE_ON_OFF: {
            ECUAL_GPIO_State_t target_state = (percentage > 0) ? cfg->on_off_cfg.active_state :
                                                                 ((cfg->on_off_cfg.active_state == ECUAL_GPIO_HIGH) ? ECUAL_GPIO_LOW : ECUAL_GPIO_HIGH);
            if (ECUAL_GPIO_SetState(cfg->on_off_cfg.gpio_id, target_state) != ECUAL_OK) {
                ESP_LOGE(TAG, "Light %u (ON/OFF): Failed to set GPIO %u to %s for brightness %u%%.",
                         unit_id, cfg->on_off_cfg.gpio_id, (target_state == ECUAL_GPIO_HIGH) ? "HIGH" : "LOW", percentage);
                app_ret = APP_ERROR;
            }
            // For ON/OFF, brightness is either 0 or 100
            current_light_brightness[unit_id] = (percentage > 0) ? 100 : 0;
            ESP_LOGI(TAG, "Light %u (ON/OFF) set to %u%% (GPIO %s).", unit_id, current_light_brightness[unit_id],
                     (target_state == ECUAL_GPIO_HIGH) ? "HIGH" : "LOW");
            break;
        }
        case LIGHT_TYPE_DIMMABLE: {
            if (ECUAL_PWM_SetDuty(cfg->dimmable_cfg.pwm_channel, percentage) != ECUAL_OK) {
                ESP_LOGE(TAG, "Light %u (Dimmable): Failed to set PWM channel %u duty to %u%%.",
                         unit_id, cfg->dimmable_cfg.pwm_channel, percentage);
                app_ret = APP_ERROR;
            }
            current_light_brightness[unit_id] = percentage;
            ESP_LOGI(TAG, "Light %u (Dimmable) set to %u%%.", unit_id, percentage);
            break;
        }
        default:
            ESP_LOGE(TAG, "Light %u: Unknown light type: %d.", unit_id, cfg->type);
            app_ret = APP_ERROR;
            break;
    }
    return app_ret;
}

uint8_t LIGHT_GetCommandedState(LIGHT_ID_t unit_id, bool *is_on) {
    if (unit_id >= LIGHT_TOTAL_UNITS || is_on == NULL) {
        ESP_LOGE(TAG, "Invalid Light ID or NULL pointer for GetCommandedState.");
        return APP_ERROR;
    }
    *is_on = (current_light_brightness[unit_id] > 0);
    return APP_OK;
}

uint8_t LIGHT_GetBrightness(LIGHT_ID_t unit_id, uint8_t *percentage) {
    if (unit_id >= LIGHT_TOTAL_UNITS || percentage == NULL) {
        ESP_LOGE(TAG, "Invalid Light ID or NULL pointer for GetBrightness.");
        return APP_ERROR;
    }
    *percentage = current_light_brightness[unit_id];
    return APP_OK;
}
