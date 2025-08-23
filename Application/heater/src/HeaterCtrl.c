/* ============================================================================
 * SOURCE FILE: Application/heaterCtrl/src/heater_ctrl.c
 * ============================================================================*/
/**
 * @file heater_ctrl.c
 * @brief Implements the Heater Control (HeaterCtrl) module.
 *
 * This file provides the logic for initializing and controlling heaters,
 * by interfacing with the Hardware Abstraction Layer (HAL),
 * supporting both CH423S expander and direct GPIO control.
 */

#include "heaterctrl.h"       // Public API for HeaterCtrl
#include "heaterctrl_cfg.h"   // Configuration for heater instances
#include "HAL_I2C.h"        // Dependency for controlling CH423S expander
#include "HAL_GPIO.h"       // Dependency for controlling direct GPIO
#include "esp_log.h"        // ESP-IDF logging

static const char *TAG = "HeaterCtrl"; // Logging tag for this module

// --- Internal Data ---
// Array to store the current state of each heater. Initialized during HeaterCtrl_Init.
static Heater_State_t s_current_heater_states[HEATER_ID_COUNT];

// --- Public Functions ---

/**
 * @brief Initializes the Heater Control (HeaterCtrl) module.
 *
 * This function iterates through the `s_heater_configurations` array,
 * sets up the corresponding pins (either CH423S or direct GPIO)
 * and sets the initial state for each heater based on its control type.
 *
 * @return E_OK on successful initialization of all configured heaters,
 * or an error code if any heater configuration or HAL call fails.
 */
Status_t HeaterCtrl_Init(void) {
    ESP_LOGI(TAG, "Initializing Heater Control module...");

    for (size_t i = 0; i < s_num_heater_configurations; i++) {
        const heater_config_item_t *heater_cfg = &s_heater_configurations[i];
        Status_t status = E_OK; // Initialize status to OK

        if (heater_cfg->heater_id >= HEATER_ID_COUNT) {
            ESP_LOGE(TAG, "Invalid Heater ID %d found in configuration.", heater_cfg->heater_id);
            return E_INVALID_PARAM;
        }

        switch (heater_cfg->control_type) {
            case HEATER_CONTROL_TYPE_IO_EXPANDER:
                // Set the initial state of the heater via the CH423S expander
                status = HAL_CH423S_SetOutput(heater_cfg->pinNum, heater_cfg->initial_state);
                if (status == E_OK) 
                {
                    ESP_LOGI(TAG, "Heater ID %d (CH423S pin %d) initialized to %s.",
                             heater_cfg->heater_id, heater_cfg->pinNum,
                             (heater_cfg->initial_state == HEATER_STATE_ON) ? "ON" : "OFF");
                } else 
                {
                    ESP_LOGE(TAG, "Failed to set initial state for Heater ID %d (CH423S pin %d). Status: %d",
                             heater_cfg->heater_id, heater_cfg->pinNum, status);
                }
                break;

            case HEATER_CONTROL_TYPE_GPIO:
                // Initialize the direct GPIO pin and set its initial state
                status = HAL_GPIO_SetLevel(heater_cfg->pinNum, heater_cfg->initial_state);
                if (status == E_OK) {
                    ESP_LOGI(TAG, "Heater ID %d (GPIO pin %d) initialized to %s.",
                             heater_cfg->heater_id, heater_cfg->pinNum,
                             (heater_cfg->initial_state == HEATER_STATE_ON) ? "ON" : "OFF");
                } else {
                    ESP_LOGE(TAG, "Failed to set initial state for Heater ID %d (GPIO pin %d). Status: %d",
                             heater_cfg->heater_id, heater_cfg->pinNum, status);
                }
                break;

            default:
                ESP_LOGE(TAG, "Unknown control type %d for Heater ID %d.",
                         heater_cfg->control_type, heater_cfg->heater_id);
                return E_INVALID_PARAM; // Error for unknown control type
        }

        if (status != E_OK) {
            return status; // Propagate the error from HAL
        }

        // Store the initial state internally
        s_current_heater_states[heater_cfg->heater_id] = heater_cfg->initial_state;
    }

    ESP_LOGI(TAG, "Heater Control module initialized successfully with %zu heaters.", s_num_heater_configurations);
    return E_OK;
}

/**
 * @brief Sets the state (ON or OFF) of a specific heater.
 *
 * This function looks up the heater's configuration and calls the appropriate
 * HAL function (CH423S or direct GPIO) to control it.
 *
 * @param heater_id The unique identifier of the heater to control.
 * @param state The desired state for the heater (HEATER_STATE_ON or HEATER_STATE_OFF).
 * @return E_OK on success, E_INVALID_PARAM if `heater_id` is invalid, or
 * an error code from the underlying HAL if pin control fails.
 */
Status_t HeaterCtrl_SetState(Heater_ID_t heater_id, Heater_State_t state) {
    if (heater_id >= HEATER_ID_COUNT) {
        ESP_LOGE(TAG, "Attempted to set state for invalid Heater ID %d.", heater_id);
        return E_INVALID_PARAM;
    }

    // Find the configuration for the specified heater ID
    const heater_config_item_t *heater_cfg = NULL;
    for (size_t i = 0; i < s_num_heater_configurations; i++) {
        if (s_heater_configurations[i].heater_id == heater_id) {
            heater_cfg = &s_heater_configurations[i];
            break;
        }
    }

    if (heater_cfg == NULL) {
        ESP_LOGE(TAG, "Heater ID %d not found in configuration.", heater_id);
        return E_INVALID_PARAM;
    }

    Status_t status = E_OK; // Initialize status to OK

    switch (heater_cfg->control_type) {
        case HEATER_CONTROL_TYPE_IO_EXPANDER:
            // Call the HAL to set the output on the CH423S expander
            status = HAL_CH423S_SetOutput(heater_cfg->pinNum, state);
            if (status == E_OK) {
                ESP_LOGI(TAG, "Heater ID %d (CH423S pin %d) set to %s.",
                         heater_id, heater_cfg->pinNum,
                         (state == HEATER_STATE_ON) ? "ON" : "OFF");
            } else {
                ESP_LOGE(TAG, "Failed to set Heater ID %d (CH423S pin %d) to %s. Status: %d",
                         heater_id, heater_cfg->pinNum,
                         (state == HEATER_STATE_ON) ? "ON" : "OFF", status);
            }
            break;

        case HEATER_CONTROL_TYPE_GPIO:
            // Call the HAL to set the output on the direct GPIO pin
            status = HAL_GPIO_SetLevel(heater_cfg->pinNum, state);
            if (status == E_OK) {
                ESP_LOGI(TAG, "Heater ID %d (GPIO pin %d) set to %s.",
                         heater_id, heater_cfg->pinNum,
                         (state == HEATER_STATE_ON) ? "ON" : "OFF");
            } else {
                ESP_LOGE(TAG, "Failed to set Heater ID %d (GPIO pin %d) to %s. Status: %d",
                         heater_id, heater_cfg->pinNum,
                         (state == HEATER_STATE_ON) ? "ON" : "OFF", status);
            }
            break;

        default:
            ESP_LOGE(TAG, "Unknown control type %d for Heater ID %d.",
                     heater_cfg->control_type, heater_id);
            return E_INVALID_PARAM; // Error for unknown control type
    }

    if (status != E_OK) {
        return status;
    }

    // Update the internal state tracking
    s_current_heater_states[heater_id] = state;
    return E_OK;
}

/**
 * @brief Gets the current state of a specific heater.
 *
 * This function retrieves the state from the internally tracked `s_current_heater_states` array.
 *
 * @param heater_id The unique identifier of the heater to query.
 * @param state_out Pointer to a variable where the current state (HEATER_STATE_ON/OFF)
 * will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `heater_id` or `state_out` is invalid.
 */
Status_t HeaterCtrl_GetState(Heater_ID_t heater_id, Heater_State_t *state_out) {
    if (heater_id >= HEATER_ID_COUNT || state_out == NULL) {
        ESP_LOGE(TAG, "Invalid Heater ID %d or NULL state_out pointer.", heater_id);
        return E_INVALID_PARAM;
    }

    *state_out = s_current_heater_states[heater_id];
    ESP_LOGD(TAG, "Heater ID %d current state is %s.", heater_id,
             (*state_out == HEATER_STATE_ON) ? "ON" : "OFF");
    return E_OK;
}
