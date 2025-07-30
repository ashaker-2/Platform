// app/src/ventilator.c

#include "ventilator.h"
#include "ventilator_config.h"
#include "ecual_gpio.h"     // To control ventilator GPIO
#include "esp_log.h"        // For logging

static const char *TAG = "VENTILATOR";

// Static array to store the current commanded state for each ventilator
static bool current_ventilator_states[VENTILATOR_TOTAL_UNITS];

// Helper to get ventilator config by ID
static const VENTILATOR_Config_t* ventilator_get_config(VENTILATOR_ID_t unit_id) {
    if (unit_id >= VENTILATOR_TOTAL_UNITS) {
        ESP_LOGE(TAG, "Invalid Ventilator ID: %u", unit_id);
        return NULL;
    }
    return &ventilator_configurations[unit_id];
}

uint8_t VENTILATOR_Init(void) {
    uint8_t app_ret = APP_OK;
    uint8_t ecual_ret;

    for (uint32_t i = 0; i < VENTILATOR_NUM_CONFIGURATIONS; i++) {
        const VENTILATOR_Config_t *cfg = &ventilator_configurations[i];
        current_ventilator_states[i] = false; // Initialize commanded state to OFF

        // The GPIO should already be configured as OUTPUT by ECUAL_GPIO_Init.
        // Set initial state to inactive (OFF).
        ECUAL_GPIO_State_t initial_state = (cfg->active_state == ECUAL_GPIO_HIGH) ? ECUAL_GPIO_LOW : ECUAL_GPIO_HIGH;
        ecual_ret = ECUAL_GPIO_SetState(cfg->control_gpio_id, initial_state);
        if (ecual_ret != ECUAL_OK) {
            ESP_LOGE(TAG, "Ventilator %u: Failed to set initial state for GPIO %u.", i, cfg->control_gpio_id);
            app_ret = APP_ERROR;
            continue;
        }

        ESP_LOGI(TAG, "Ventilator %u initialized. Control GPIO: %u, Active State: %s",
                 i, cfg->control_gpio_id, (cfg->active_state == ECUAL_GPIO_HIGH) ? "HIGH" : "LOW");
    }

    return app_ret;
}

uint8_t VENTILATOR_On(VENTILATOR_ID_t unit_id) {
    const VENTILATOR_Config_t *cfg = ventilator_get_config(unit_id);
    if (cfg == NULL) { return APP_ERROR; }

    uint8_t ecual_ret = ECUAL_GPIO_SetState(cfg->control_gpio_id, cfg->active_state);
    if (ecual_ret != ECUAL_OK) {
        ESP_LOGE(TAG, "Ventilator %u: Failed to turn ON GPIO %u.", unit_id, cfg->control_gpio_id);
        return APP_ERROR;
    }

    current_ventilator_states[unit_id] = true;
    ESP_LOGI(TAG, "Ventilator %u turned ON.", unit_id);
    return APP_OK;
}

uint8_t VENTILATOR_Off(VENTILATOR_ID_t unit_id) {
    const VENTILATOR_Config_t *cfg = ventilator_get_config(unit_id);
    if (cfg == NULL) { return APP_ERROR; }

    ECUAL_GPIO_State_t inactive_state = (cfg->active_state == ECUAL_GPIO_HIGH) ? ECUAL_GPIO_LOW : ECUAL_GPIO_HIGH;
    uint8_t ecual_ret = ECUAL_GPIO_SetState(cfg->control_gpio_id, inactive_state);
    if (ecual_ret != ECUAL_OK) {
        ESP_LOGE(TAG, "Ventilator %u: Failed to turn OFF GPIO %u.", unit_id, cfg->control_gpio_id);
        return APP_ERROR;
    }

    current_ventilator_states[unit_id] = false;
    ESP_LOGI(TAG, "Ventilator %u turned OFF.", unit_id);
    return APP_OK;
}

uint8_t VENTILATOR_GetState(VENTILATOR_ID_t unit_id, bool *is_on) {
    if (unit_id >= VENTILATOR_TOTAL_UNITS || is_on == NULL) {
        ESP_LOGE(TAG, "Invalid Ventilator ID or NULL pointer for GetState.");
        return APP_ERROR;
    }
    *is_on = current_ventilator_states[unit_id];
    return APP_OK;
}
