// app/src/light_indication.c

#include "light_indication.h"
#include "light_indication_config.h"
#include "ecual_gpio.h" // For ECUAL_GPIO_SetState, ECUAL_GPIO_GetState
#include "esp_log.h"    // For logging

static const char *TAG = "LIGHT_INDICATION";

// Array to store the current ON/OFF state of each LED unit
static bool current_led_states[LIGHT_INDICATION_TOTAL_UNITS];

/**
 * @brief Helper function to get the configuration for a specific LED ID.
 * @param unit_id The LIGHT_INDICATION_ID_t of the LED unit.
 * @return Pointer to the configuration struct, or NULL if ID is invalid.
 */
static const LIGHT_INDICATION_Config_t* light_indication_get_config(LIGHT_INDICATION_ID_t unit_id) {
    if (unit_id >= LIGHT_INDICATION_TOTAL_UNITS) {
        ESP_LOGE(TAG, "Invalid Light Indication ID: %u", unit_id);
        return NULL;
    }
    return &light_indication_configurations[unit_id];
}

uint8_t LIGHT_INDICATION_Init(void) {
    uint8_t app_ret = APP_OK;

    for (uint32_t i = 0; i < LIGHT_INDICATION_NUM_CONFIGURATIONS; i++) {
        const LIGHT_INDICATION_Config_t *cfg = &light_indication_configurations[i];

        // Determine the initial (OFF) state for the LED GPIO
        ECUAL_GPIO_State_t initial_state = (cfg->active_state == ECUAL_GPIO_HIGH) ? ECUAL_GPIO_LOW : ECUAL_GPIO_HIGH;

        // Set the GPIO to its initial OFF state
        if (ECUAL_GPIO_SetState(cfg->gpio_id, initial_state) != ECUAL_OK) {
            ESP_LOGE(TAG, "Light Indication %u: Failed to set initial state for GPIO %u.", i, cfg->gpio_id);
            app_ret = APP_ERROR;
            continue; // Continue to initialize other LEDs
        }
        current_led_states[i] = false; // Mark as off initially

        ESP_LOGI(TAG, "Light Indication %u initialized. GPIO: %u, Active State: %s (Initial State: OFF)",
                 i, cfg->gpio_id, (cfg->active_state == ECUAL_GPIO_HIGH) ? "HIGH" : "LOW");
    }
    return app_ret;
}

uint8_t LIGHT_INDICATION_On(LIGHT_INDICATION_ID_t unit_id) {
    const LIGHT_INDICATION_Config_t *cfg = light_indication_get_config(unit_id);
    if (cfg == NULL) {
        return APP_ERROR;
    }

    if (ECUAL_GPIO_SetState(cfg->gpio_id, cfg->active_state) != ECUAL_OK) {
        ESP_LOGE(TAG, "Light Indication %u: Failed to turn ON GPIO %u.", unit_id, cfg->gpio_id);
        return APP_ERROR;
    }
    current_led_states[unit_id] = true;
    ESP_LOGI(TAG, "Light Indication %u turned ON.", unit_id);
    return APP_OK;
}

uint8_t LIGHT_INDICATION_Off(LIGHT_INDICATION_ID_t unit_id) {
    const LIGHT_INDICATION_Config_t *cfg = light_indication_get_config(unit_id);
    if (cfg == NULL) {
        return APP_ERROR;
    }

    // Determine the inactive (OFF) state for the LED GPIO
    ECUAL_GPIO_State_t inactive_state = (cfg->active_state == ECUAL_GPIO_HIGH) ? ECUAL_GPIO_LOW : ECUAL_GPIO_HIGH;

    if (ECUAL_GPIO_SetState(cfg->gpio_id, inactive_state) != ECUAL_OK) {
        ESP_LOGE(TAG, "Light Indication %u: Failed to turn OFF GPIO %u.", unit_id, cfg->gpio_id);
        return APP_ERROR;
    }
    current_led_states[unit_id] = false;
    ESP_LOGI(TAG, "Light Indication %u turned OFF.", unit_id);
    return APP_OK;
}

uint8_t LIGHT_INDICATION_Toggle(LIGHT_INDICATION_ID_t unit_id) {
    bool current_state;
    if (LIGHT_INDICATION_GetState(unit_id, &current_state) != APP_OK) {
        return APP_ERROR; // Failed to get current state
    }

    if (current_state) {
        return LIGHT_INDICATION_Off(unit_id);
    } else {
        return LIGHT_INDICATION_On(unit_id);
    }
}

uint8_t LIGHT_INDICATION_GetState(LIGHT_INDICATION_ID_t unit_id, bool *is_on) {
    if (unit_id >= LIGHT_INDICATION_TOTAL_UNITS || is_on == NULL) {
        ESP_LOGE(TAG, "Invalid Light Indication ID or NULL pointer for GetState.");
        return APP_ERROR;
    }
    *is_on = current_led_states[unit_id];
    return APP_OK;
}
