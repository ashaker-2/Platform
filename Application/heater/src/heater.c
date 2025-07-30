// app/src/heater.c

#include "heater.h"
#include "heater_config.h"
#include "ecual_gpio.h" // For ECUAL_GPIO_SetState, ECUAL_GPIO_GetState
#include "esp_log.h"    // For logging

static const char *TAG = "HEATER";

// Array to store the current COMMANDED ON/OFF state of each heater unit
static bool current_heater_commanded_states[HEATER_TOTAL_UNITS];

/**
 * @brief Helper function to get the configuration for a specific heater ID.
 * @param unit_id The HEATER_ID_t of the heater unit.
 * @return Pointer to the configuration struct, or NULL if ID is invalid.
 */
static const HEATER_Config_t* heater_get_config(HEATER_ID_t unit_id) {
    if (unit_id >= HEATER_TOTAL_UNITS) {
        ESP_LOGE(TAG, "Invalid Heater ID: %u", unit_id);
        return NULL;
    }
    return &heater_configurations[unit_id];
}

uint8_t HEATER_Init(void) {
    uint8_t app_ret = APP_OK;

    for (uint32_t i = 0; i < HEATER_NUM_CONFIGURATIONS; i++) {
        const HEATER_Config_t *cfg = &heater_configurations[i];

        // 1. Initialize Control GPIO
        ECUAL_GPIO_State_t initial_control_state = (cfg->active_state == ECUAL_GPIO_HIGH) ? ECUAL_GPIO_LOW : ECUAL_GPIO_HIGH;
        if (ECUAL_GPIO_SetState(cfg->control_gpio_id, initial_control_state) != ECUAL_OK) {
            ESP_LOGE(TAG, "Heater %u: Failed to set initial state for Control GPIO %u.", i, cfg->control_gpio_id);
            app_ret = APP_ERROR; // Mark as error, but try to initialize others
        }
        current_heater_commanded_states[i] = false; // Mark as off initially

        // 2. Log initialization for control GPIO
        ESP_LOGI(TAG, "Heater %u initialized. Control GPIO: %u, Active State: %s (Initial State: OFF)",
                 i, cfg->control_gpio_id, (cfg->active_state == ECUAL_GPIO_HIGH) ? "HIGH" : "LOW");

        // 3. Initialize Feedback GPIO (if configured)
        if (cfg->has_feedback_gpio) {
            // Note: ECUAL_GPIO_Init should already configure this as input.
            // We just log that it's present and its expected active state.
            ESP_LOGI(TAG, "Heater %u has Feedback GPIO: %u, Feedback Active State: %s",
                     i, cfg->feedback_gpio_id, (cfg->feedback_active_state == ECUAL_GPIO_HIGH) ? "HIGH" : "LOW");
        } else {
            ESP_LOGI(TAG, "Heater %u: No Feedback GPIO configured.", i);
        }
    }
    return app_ret;
}

uint8_t HEATER_On(HEATER_ID_t unit_id) {
    const HEATER_Config_t *cfg = heater_get_config(unit_id);
    if (cfg == NULL) { return APP_ERROR; }

    if (ECUAL_GPIO_SetState(cfg->control_gpio_id, cfg->active_state) != ECUAL_OK) {
        ESP_LOGE(TAG, "Heater %u: Failed to turn ON Control GPIO %u.", unit_id, cfg->control_gpio_id);
        return APP_ERROR;
    }
    current_heater_commanded_states[unit_id] = true;
    ESP_LOGI(TAG, "Heater %u commanded ON.", unit_id);
    return APP_OK;
}

uint8_t HEATER_Off(HEATER_ID_t unit_id) {
    const HEATER_Config_t *cfg = heater_get_config(unit_id);
    if (cfg == NULL) { return APP_ERROR; }

    ECUAL_GPIO_State_t inactive_state = (cfg->active_state == ECUAL_GPIO_HIGH) ? ECUAL_GPIO_LOW : ECUAL_GPIO_HIGH;
    if (ECUAL_GPIO_SetState(cfg->control_gpio_id, inactive_state) != ECUAL_OK) {
        ESP_LOGE(TAG, "Heater %u: Failed to turn OFF Control GPIO %u.", unit_id, cfg->control_gpio_id);
        return APP_ERROR;
    }
    current_heater_commanded_states[unit_id] = false;
    ESP_LOGI(TAG, "Heater %u commanded OFF.", unit_id);
    return APP_OK;
}

uint8_t HEATER_GetCommandedState(HEATER_ID_t unit_id, bool *is_on) {
    if (unit_id >= HEATER_TOTAL_UNITS || is_on == NULL) {
        ESP_LOGE(TAG, "Invalid Heater ID or NULL pointer for GetCommandedState.");
        return APP_ERROR;
    }
    *is_on = current_heater_commanded_states[unit_id];
    return APP_OK;
}

uint8_t HEATER_GetFeedbackState(HEATER_ID_t unit_id, bool *is_working) {
    const HEATER_Config_t *cfg = heater_get_config(unit_id);
    if (cfg == NULL || is_working == NULL) {
        if (is_working != NULL) *is_working = false;
        return APP_ERROR;
    }

    if (!cfg->has_feedback_gpio) {
        ESP_LOGW(TAG, "Heater %u: No feedback GPIO configured. Cannot get feedback state.", unit_id);
        *is_working = false; // Default to not working if no feedback exists
        return APP_ERROR;
    }

    ECUAL_GPIO_State_t current_feedback_state;
    if (ECUAL_GPIO_GetState(cfg->feedback_gpio_id, &current_feedback_state) != ECUAL_OK) {
        ESP_LOGE(TAG, "Heater %u: Failed to read feedback GPIO %u.", unit_id, cfg->feedback_gpio_id);
        *is_working = false;
        return APP_ERROR;
    }

    *is_working = (current_feedback_state == cfg->feedback_active_state);
    ESP_LOGD(TAG, "Heater %u: Feedback GPIO %u state: %s, Is Working: %s",
             unit_id, cfg->feedback_gpio_id,
             (current_feedback_state == ECUAL_GPIO_HIGH) ? "HIGH" : "LOW",
             *is_working ? "TRUE" : "FALSE");
    return APP_OK;
}
