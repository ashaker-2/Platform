/* ============================================================================
 * SOURCE FILE: Application/venCtrl/src/ven_ctrl.c
 * ============================================================================*/
/**
 * @file ven_ctrl.c
 * @brief Implements the Ven Control (VenCtrl) module.
 *
 * This file provides the logic for initializing and controlling vens,
 * by interfacing with the Hardware Abstraction Layer (HAL),
 * supporting both CH423S expander and direct GPIO control.
 */

#include "venctrl.h"       // Public API for VenCtrl
#include "venctrl_cfg.h"   // Configuration for ven instances
#include "HAL_I2C.h"        // Dependency for controlling CH423S expander
#include "HAL_GPIO.h"       // Dependency for controlling direct GPIO
#include "esp_log.h"        // ESP-IDF logging

static const char *TAG = "VenCtrl"; // Logging tag for this module

// --- Internal Data ---
// Array to store the current state of each ven. Initialized during VenCtrl_Init.
static Ven_State_t s_current_ven_states[VEN_ID_COUNT];

// --- Public Functions ---

/**
 * @brief Initializes the Ven Control (VenCtrl) module.
 *
 * This function iterates through the `s_ven_configurations` array,
 * sets up the corresponding pins (either CH423S or direct GPIO)
 * and sets the initial state for each ven based on its control type.
 *
 * @return E_OK on successful initialization of all configured vens,
 * or an error code if any ven configuration or HAL call fails.
 */
Status_t VenCtrl_Init(void) {
    ESP_LOGI(TAG, "Initializing Ven Control module...");

    for (size_t i = 0; i < s_num_ven_configurations; i++) {
        const ven_config_item_t *ven_cfg = &s_ven_configurations[i];
        Status_t status = E_OK; // Initialize status to OK

        if (ven_cfg->ven_id >= VEN_ID_COUNT) {
            ESP_LOGE(TAG, "Invalid Ven ID %d found in configuration.", ven_cfg->ven_id);
            return E_INVALID_PARAM;
        }

        switch (ven_cfg->control_type) {
            case VEN_CONTROL_TYPE_IO_EXPANDER:
                // Set the initial state of the ven via the CH423S expander
                status = HAL_CH423S_SetOutput(ven_cfg->pinNum, ven_cfg->initial_state);
                if (status == E_OK) 
                {
                    ESP_LOGI(TAG, "Ven ID %d (CH423S pin %d) initialized to %s.",
                             ven_cfg->ven_id, ven_cfg->pinNum,
                             (ven_cfg->initial_state == VEN_STATE_ON) ? "ON" : "OFF");
                } else 
                {
                    ESP_LOGE(TAG, "Failed to set initial state for Ven ID %d (CH423S pin %d). Status: %d",
                             ven_cfg->ven_id, ven_cfg->pinNum, status);
                }
                break;

            case VEN_CONTROL_TYPE_GPIO:
                // Initialize the direct GPIO pin and set its initial state
                status = HAL_GPIO_SetLevel(ven_cfg->pinNum, ven_cfg->initial_state);
                if (status == E_OK) {
                    ESP_LOGI(TAG, "Ven ID %d (GPIO pin %d) initialized to %s.",
                             ven_cfg->ven_id, ven_cfg->pinNum,
                             (ven_cfg->initial_state == VEN_STATE_ON) ? "ON" : "OFF");
                } else {
                    ESP_LOGE(TAG, "Failed to set initial state for Ven ID %d (GPIO pin %d). Status: %d",
                             ven_cfg->ven_id, ven_cfg->pinNum, status);
                }
                break;

            default:
                ESP_LOGE(TAG, "Unknown control type %d for Ven ID %d.",
                         ven_cfg->control_type, ven_cfg->ven_id);
                return E_INVALID_PARAM; // Error for unknown control type
        }

        if (status != E_OK) {
            return status; // Propagate the error from HAL
        }

        // Store the initial state internally
        s_current_ven_states[ven_cfg->ven_id] = ven_cfg->initial_state;
    }

    ESP_LOGI(TAG, "Ven Control module initialized successfully with %zu vens.", s_num_ven_configurations);
    return E_OK;
}

/**
 * @brief Sets the state (ON or OFF) of a specific ven.
 *
 * This function looks up the ven's configuration and calls the appropriate
 * HAL function (CH423S or direct GPIO) to control it.
 *
 * @param ven_id The unique identifier of the ven to control.
 * @param state The desired state for the ven (VEN_STATE_ON or VEN_STATE_OFF).
 * @return E_OK on success, E_INVALID_PARAM if `ven_id` is invalid, or
 * an error code from the underlying HAL if pin control fails.
 */
Status_t VenCtrl_SetState(Ven_ID_t ven_id, Ven_State_t state) {
    if (ven_id >= VEN_ID_COUNT) {
        ESP_LOGE(TAG, "Attempted to set state for invalid Ven ID %d.", ven_id);
        return E_INVALID_PARAM;
    }

    // Find the configuration for the specified ven ID
    const ven_config_item_t *ven_cfg = NULL;
    for (size_t i = 0; i < s_num_ven_configurations; i++) {
        if (s_ven_configurations[i].ven_id == ven_id) {
            ven_cfg = &s_ven_configurations[i];
            break;
        }
    }

    if (ven_cfg == NULL) {
        ESP_LOGE(TAG, "Ven ID %d not found in configuration.", ven_id);
        return E_INVALID_PARAM;
    }

    Status_t status = E_OK; // Initialize status to OK

    switch (ven_cfg->control_type) {
        case VEN_CONTROL_TYPE_IO_EXPANDER:
            // Call the HAL to set the output on the CH423S expander
            status = HAL_CH423S_SetOutput(ven_cfg->pinNum, state);
            if (status == E_OK) {
                ESP_LOGI(TAG, "Ven ID %d (CH423S pin %d) set to %s.",
                         ven_id, ven_cfg->pinNum,
                         (state == VEN_STATE_ON) ? "ON" : "OFF");
            } else {
                ESP_LOGE(TAG, "Failed to set Ven ID %d (CH423S pin %d) to %s. Status: %d",
                         ven_id, ven_cfg->pinNum,
                         (state == VEN_STATE_ON) ? "ON" : "OFF", status);
            }
            break;

        case VEN_CONTROL_TYPE_GPIO:
            // Call the HAL to set the output on the direct GPIO pin
            status = HAL_GPIO_SetLevel(ven_cfg->pinNum, state);
            if (status == E_OK) {
                ESP_LOGI(TAG, "Ven ID %d (GPIO pin %d) set to %s.",
                         ven_id, ven_cfg->pinNum,
                         (state == VEN_STATE_ON) ? "ON" : "OFF");
            } else {
                ESP_LOGE(TAG, "Failed to set Ven ID %d (GPIO pin %d) to %s. Status: %d",
                         ven_id, ven_cfg->pinNum,
                         (state == VEN_STATE_ON) ? "ON" : "OFF", status);
            }
            break;

        default:
            ESP_LOGE(TAG, "Unknown control type %d for Ven ID %d.",
                     ven_cfg->control_type, ven_id);
            return E_INVALID_PARAM; // Error for unknown control type
    }

    if (status != E_OK) {
        return status;
    }

    // Update the internal state tracking
    s_current_ven_states[ven_id] = state;
    return E_OK;
}

/**
 * @brief Gets the current state of a specific ven.
 *
 * This function retrieves the state from the internally tracked `s_current_ven_states` array.
 *
 * @param ven_id The unique identifier of the ven to query.
 * @param state_out Pointer to a variable where the current state (VEN_STATE_ON/OFF)
 * will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `ven_id` or `state_out` is invalid.
 */
Status_t VenCtrl_GetState(Ven_ID_t ven_id, Ven_State_t *state_out) {
    if (ven_id >= VEN_ID_COUNT || state_out == NULL) {
        ESP_LOGE(TAG, "Invalid Ven ID %d or NULL state_out pointer.", ven_id);
        return E_INVALID_PARAM;
    }

    *state_out = s_current_ven_states[ven_id];
    ESP_LOGD(TAG, "Ven ID %d current state is %s.", ven_id,
             (*state_out == VEN_STATE_ON) ? "ON" : "OFF");
    return E_OK;
}
