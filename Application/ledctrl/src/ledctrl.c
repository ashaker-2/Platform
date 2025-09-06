/* ============================================================================
 * SOURCE FILE: Application/ledCtrl/src/led_ctrl.c
 * ============================================================================*/
/**
 * @file led_ctrl.c
 * @brief Implements the Led Control (LedCtrl) module.
 *
 * This file provides the logic for initializing and controlling leds,
 * by interfacing with the Hardware Abstraction Layer (HAL),
 * supporting both CH423S expander and direct GPIO control.
 */

#include "ledctrl.h"       // Public API for LedCtrl
#include "ledctrl_cfg.h"   // Configuration for led instances
#include "HAL_I2C.h"        // Dependency for controlling CH423S expander
#include "HAL_GPIO.h"       // Dependency for controlling direct GPIO
#include "esp_log.h"        // ESP-IDF logging

static const char *TAG = "LedCtrl"; // Logging tag for this module

// --- Internal Data ---
// Array to store the current state of each led. Initialized during LedCtrl_Init.
static Led_State_t s_current_led_states[LED_ID_COUNT];

static Status_t LedCtrl_SetSingle(Led_ID_t led_id, Led_State_t state);

// --- Public Functions ---

/**
 * @brief Initializes the Led Control (LedCtrl) module.
 *
 * This function iterates through the `s_led_configurations` array,
 * sets up the corresponding pins (either CH423S or direct GPIO)
 * and sets the initial state for each led based on its control type.
 *
 * @return E_OK on successful initialization of all configured leds,
 * or an error code if any led configuration or HAL call fails.
 */
Status_t LedCtrl_Init(void) {
    ESP_LOGI(TAG, "Initializing Led Control module...");

    for (size_t i = 0; i < s_num_led_configurations; i++) {
        const led_config_item_t *led_cfg = &s_led_configurations[i];
        Status_t status = E_OK; // Initialize status to OK

        if (led_cfg->led_id >= LED_ID_COUNT) {
            ESP_LOGE(TAG, "Invalid Led ID %d found in configuration.", led_cfg->led_id);
            return E_INVALID_PARAM;
        }

        switch (led_cfg->control_type) {
            case LED_CONTROL_TYPE_IO_EXPANDER:
                // Set the initial state of the led via the CH423S expander
                status = HAL_CH423S_SetOutput(led_cfg->pinNum, led_cfg->initial_state);
                if (status != E_OK) 
                {
                    ESP_LOGE(TAG, "Failed to set initial state for Led ID %d (CH423S pin %d). Status: %d",
                             led_cfg->led_id, led_cfg->pinNum, status);
                }
                break;

            case LED_CONTROL_TYPE_GPIO:
                // Initialize the direct GPIO pin and set its initial state
                status = HAL_GPIO_SetLevel(led_cfg->pinNum, led_cfg->initial_state);
                if (status != E_OK) 
                {
                    ESP_LOGE(TAG, "Failed to set initial state for Led ID %d (GPIO pin %d). Status: %d",
                             led_cfg->led_id, led_cfg->pinNum, status);
                }
                break;

            default:
                ESP_LOGE(TAG, "Unknown control type %d for Led ID %d.",
                         led_cfg->control_type, led_cfg->led_id);
                return E_INVALID_PARAM; // Error for unknown control type
        }

        if (status != E_OK) {
            return status; // Propagate the error from HAL
        }

        // Store the initial state internally
        s_current_led_states[led_cfg->led_id] = led_cfg->initial_state;
    }

    ESP_LOGI(TAG, "Led Control module initialized successfully with %zu leds.", s_num_led_configurations);
    return E_OK;
}

/**
 * @brief Sets the state (ON or OFF) of a specific led.
 *
 * This function looks up the led's configuration and calls the appropriate
 * HAL function (CH423S or direct GPIO) to control it.
 *
 * @param led_id The unique identifier of the led to control.
 * @param state The desired state for the led (LED_STATE_ON or LED_STATE_OFF).
 * @return E_OK on success, E_INVALID_PARAM if `led_id` is invalid, or
 * an error code from the underlying HAL if pin control fails.
 */
Status_t LedCtrl_SetState(Led_ID_t led_id, Led_State_t state) 
{
    if (led_id == LED_ID_COUNT) {
        Status_t overall_status = E_OK;

        for (Led_ID_t id = 0; id < LED_ID_COUNT; id++) 
        {
            Status_t s = LedCtrl_SetSingle(id, state);
            if (s != E_OK) 
            {
                overall_status = s; // record last failure
            }
        }

        return overall_status;
    }

    // Single heater case
    return LedCtrl_SetSingle(led_id, state);
}


// Helper function to set a single heater by ID
static Status_t LedCtrl_SetSingle(Led_ID_t led_id, Led_State_t state)  
{
    if (led_id >= LED_ID_COUNT) {
        ESP_LOGE(TAG, "Attempted to set state for invalid Led ID %d.", led_id);
        return E_INVALID_PARAM;
    }

    // Find the configuration for the specified led ID
    const led_config_item_t *led_cfg = NULL;
    for (size_t i = 0; i < s_num_led_configurations; i++) {
        if (s_led_configurations[i].led_id == led_id) {
            led_cfg = &s_led_configurations[i];
            break;
        }
    }

    if (led_cfg == NULL) {
        ESP_LOGE(TAG, "Led ID %d not found in configuration.", led_id);
        return E_INVALID_PARAM;
    }

    Status_t status = E_OK; // Initialize status to OK

    switch (led_cfg->control_type) {
        case LED_CONTROL_TYPE_IO_EXPANDER:
            // Call the HAL to set the output on the CH423S expander
            status = HAL_CH423S_SetOutput(led_cfg->pinNum, state);
            if (status != E_OK) 
            {
                ESP_LOGE(TAG, "Failed to set Led ID %d (CH423S pin %d) to %s. Status: %d",
                         led_id, led_cfg->pinNum,
                         (state == LED_STATE_ON) ? "ON" : "OFF", status);
            }
            break;

        case LED_CONTROL_TYPE_GPIO:
            // Call the HAL to set the output on the direct GPIO pin
            status = HAL_GPIO_SetLevel(led_cfg->pinNum, state);
            if (status == E_OK) {
                ESP_LOGI(TAG, "Led ID %d (GPIO pin %d) set to %s.",
                         led_id, led_cfg->pinNum,
                         (state == LED_STATE_ON) ? "ON" : "OFF");
            } else {
                ESP_LOGE(TAG, "Failed to set Led ID %d (GPIO pin %d) to %s. Status: %d",
                         led_id, led_cfg->pinNum,
                         (state == LED_STATE_ON) ? "ON" : "OFF", status);
            }
            break;

        default:
            ESP_LOGE(TAG, "Unknown control type %d for Led ID %d.",
                     led_cfg->control_type, led_id);
            return E_INVALID_PARAM; // Error for unknown control type
    }

    if (status != E_OK) {
        return status;
    }

    // Update the internal state tracking
    s_current_led_states[led_id] = state;
    return E_OK;
}

/**
 * @brief Gets the current state of a specific led.
 *
 * This function retrieves the state from the internally tracked `s_current_led_states` array.
 *
 * @param led_id The unique identifier of the led to query.
 * @param state_out Pointer to a variable where the current state (LED_STATE_ON/OFF)
 * will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `led_id` or `state_out` is invalid.
 */
Status_t LedCtrl_GetState(Led_ID_t led_id, Led_State_t *state_out) {
    if (led_id >= LED_ID_COUNT || state_out == NULL) {
        ESP_LOGE(TAG, "Invalid Led ID %d or NULL state_out pointer.", led_id);
        return E_INVALID_PARAM;
    }

    *state_out = s_current_led_states[led_id];
    ESP_LOGD(TAG, "Led ID %d current state is %s.", led_id,
             (*state_out == LED_STATE_ON) ? "ON" : "OFF");
    return E_OK;
}
