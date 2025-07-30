// app/src/heater.c

#include "heater.h"
#include "heater_config.h"
#include "ecual_gpio.h" // For ECUAL_GPIO_SetState
#include "esp_log.h"    // For logging

static const char *TAG = "HEATER";

// Array to store the current ON/OFF state of each heater unit
static bool current_heater_states[HEATER_TOTAL_UNITS];

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

        // Determine the initial (OFF) state for the control GPIO
        ECUAL_GPIO_State_t initial_state = (cfg->active_state == ECUAL_GPIO_HIGH) ? ECUAL_GPIO_LOW : ECUAL_GPIO_HIGH;

        // Set the GPIO to its initial OFF state
        if (ECUAL_GPIO_SetState(cfg->control_gpio_id, initial_state) != ECUAL_OK) {
            ESP_LOGE(TAG, "Heater %u: Failed to set initial state for GPIO %u.", i, cfg->control_gpio_id);
            app_ret = APP_ERROR;
            continue; // Continue to initialize other heaters
        }
        current_heater_states[i] = false; // Mark as off initially

        ESP_LOGI(TAG, "Heater %u initialized. Control GPIO: %u, Active State: %s (Initial State: OFF)",
                 i, cfg->control_gpio_id, (cfg->active_state == ECUAL_GPIO_HIGH) ? "HIGH" : "LOW");
    }
    return app_ret;
}

uint8_t HEATER_On(HEATER_ID_t unit_id) {
    const HEATER_Config_t *cfg = heater_get_config(unit_id);
    if (cfg == NULL) {
        return APP_ERROR;
    }

    if (ECUAL_GPIO_SetState(cfg->control_gpio_id, cfg->active_state) != ECUAL_OK) {
        ESP_LOGE(TAG, "Heater %u: Failed to turn ON GPIO %u.", unit_id, cfg->control_gpio_id);
        return APP_ERROR;
    }
    current_heater_states[unit_id] = true;
    ESP_LOGI(TAG, "Heater %u turned ON.", unit_id);
    return APP_OK;
}

uint8_t HEATER_Off(HEATER_ID_t unit_id) {
    const HEATER_Config_t *cfg = heater_get_config(unit_id);
    if (cfg == NULL) {
        return APP_ERROR;
    }

    // Determine the inactive (OFF) state for the control GPIO
    ECUAL_GPIO_State_t inactive_state = (cfg->active_state == ECUAL_GPIO_HIGH) ? ECUAL_GPIO_LOW : ECUAL_GPIO_HIGH;

    if (ECUAL_GPIO_SetState(cfg->control_gpio_id, inactive_state) != ECUAL_OK) {
        ESP_LOGE(TAG, "Heater %u: Failed to turn OFF GPIO %u.", unit_id, cfg->control_gpio_id);
        return APP_ERROR;
    }
    current_heater_states[unit_id] = false;
    ESP_LOGI(TAG, "Heater %u turned OFF.", unit_id);
    return APP_OK;
}

uint8_t HEATER_GetState(HEATER_ID_t unit_id, bool *is_on) {
    if (unit_id >= HEATER_TOTAL_UNITS || is_on == NULL) {
        ESP_LOGE(TAG, "Invalid Heater ID or NULL pointer for GetState.");
        return APP_ERROR;
    }
    *is_on = current_heater_states[unit_id];
    return APP_OK;
}
