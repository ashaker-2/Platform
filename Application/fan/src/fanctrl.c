/* ============================================================================
 * SOURCE FILE: Application/fanCtrl/src/fan_ctrl.c
 * ============================================================================*/
/**
 * @file fan_ctrl.c
 * @brief Implements the Fan Control (FanCtrl) module.
 *
 * This file provides the logic for initializing and controlling fans,
 * by interfacing with the Hardware Abstraction Layer (HAL),
 * supporting both CH423S expander and direct GPIO control.
 */

#include "fanctrl.h"       // Public API for FanCtrl
#include "fanctrl_cfg.h"   // Configuration for fan instances
#include "HAL_I2C.h"        // Dependency for controlling CH423S expander
#include "HAL_GPIO.h"       // Dependency for controlling direct GPIO
#include "esp_log.h"        // ESP-IDF logging

static const char *TAG = "FanCtrl"; // Logging tag for this module

// --- Internal Data ---
// Array to store the current state of each fan. Initialized during FanCtrl_Init.
static Fan_State_t s_current_fan_states[FAN_ID_COUNT];

static Status_t FanCtrl_SetSingle(Fan_ID_t fan_id, Fan_State_t state);

// --- Public Functions ---

/**
 * @brief Initializes the Fan Control (FanCtrl) module.
 *
 * This function iterates through the `s_fan_configurations` array,
 * sets up the corresponding pins (either CH423S or direct GPIO)
 * and sets the initial state for each fan based on its control type.
 *
 * @return E_OK on successful initialization of all configured fans,
 * or an error code if any fan configuration or HAL call fails.
 */
Status_t FanCtrl_Init(void) {
    ESP_LOGI(TAG, "Initializing Fan Control module...");

    for (size_t i = 0; i < s_num_fan_configurations; i++) {
        const fan_config_item_t *fan_cfg = &s_fan_configurations[i];
        Status_t status = E_OK; // Initialize status to OK

        if (fan_cfg->fan_id >= FAN_ID_COUNT) {
            ESP_LOGE(TAG, "Invalid Fan ID %d found in configuration.", fan_cfg->fan_id);
            return E_INVALID_PARAM;
        }

        switch (fan_cfg->control_type) {
            case FAN_CONTROL_TYPE_IO_EXPANDER:
                // Set the initial state of the fan via the CH423S expander
                status = HAL_CH423S_SetOutput(fan_cfg->pinNum, fan_cfg->initial_state);
                if (status != E_OK) 
                {
                    ESP_LOGE(TAG, "Failed to set initial state for Fan ID %d (CH423S pin %d). Status: %d",
                             fan_cfg->fan_id, fan_cfg->pinNum, status);
                }
                break;

            case FAN_CONTROL_TYPE_GPIO:
                // Initialize the direct GPIO pin and set its initial state
                status = HAL_GPIO_SetLevel(fan_cfg->pinNum, fan_cfg->initial_state);
                if (status != E_OK) 
                {
                    ESP_LOGE(TAG, "Failed to set initial state for Fan ID %d (GPIO pin %d). Status: %d",
                             fan_cfg->fan_id, fan_cfg->pinNum, status);
                }
                break;

            default:
                ESP_LOGE(TAG, "Unknown control type %d for Fan ID %d.",
                         fan_cfg->control_type, fan_cfg->fan_id);
                return E_INVALID_PARAM; // Error for unknown control type
        }

        if (status != E_OK) {
            return status; // Propagate the error from HAL
        }

        // Store the initial state internally
        s_current_fan_states[fan_cfg->fan_id] = fan_cfg->initial_state;
    }

    ESP_LOGI(TAG, "Fan Control module initialized successfully with %zu fans.", s_num_fan_configurations);
    return E_OK;
}

/**
 * @brief Sets the state (ON or OFF) of a specific fan.
 *
 * This function looks up the fan's configuration and calls the appropriate
 * HAL function (CH423S or direct GPIO) to control it.
 *
 * @param fan_id The unique identifier of the fan to control.
 * @param state The desired state for the fan (FAN_STATE_ON or FAN_STATE_OFF).
 * @return E_OK on success, E_INVALID_PARAM if `fan_id` is invalid, or
 * an error code from the underlying HAL if pin control fails.
 */
Status_t FanCtrl_SetState(Fan_ID_t fan_id, Fan_State_t state) 
{
    if (fan_id == FAN_ID_COUNT) {
        Status_t overall_status = E_OK;

        for (Fan_ID_t id = 0; id < FAN_ID_COUNT; id++) 
        {
            Status_t s = FanCtrl_SetSingle(id, state);
            if (s != E_OK) 
            {
                overall_status = s; // record last failure
            }
        }

        return overall_status;
    }

    // Single heater case
    return FanCtrl_SetSingle(fan_id, state);
}


// Helper function to set a single heater by ID
static Status_t FanCtrl_SetSingle(Fan_ID_t fan_id, Fan_State_t state) 
{
    if (fan_id > FAN_ID_COUNT) {
        ESP_LOGE(TAG, "Attempted to set state for invalid Fan ID %d.", fan_id);
        return E_INVALID_PARAM;
    }

    // Find the configuration for the specified fan ID
    const fan_config_item_t *fan_cfg = NULL;
    for (size_t i = 0; i < s_num_fan_configurations; i++) {
        if (s_fan_configurations[i].fan_id == fan_id) {
            fan_cfg = &s_fan_configurations[i];
            break;
        }
    }

    if (fan_cfg == NULL) {
        ESP_LOGE(TAG, "Fan ID %d not found in configuration.", fan_id);
        return E_INVALID_PARAM;
    }

    Status_t status = E_OK; // Initialize status to OK

    switch (fan_cfg->control_type) {
        case FAN_CONTROL_TYPE_IO_EXPANDER:
            // Call the HAL to set the output on the CH423S expander
            status = HAL_CH423S_SetOutput(fan_cfg->pinNum, state);
            if (status != E_OK) 
            {
                
                ESP_LOGE(TAG, "Failed to set Fan ID %d (CH423S pin %d) to %s. Status: %d",
                         fan_id, fan_cfg->pinNum,
                         (state == FAN_STATE_ON) ? "ON" : "OFF", status);
            }
            break;

        case FAN_CONTROL_TYPE_GPIO:
            // Call the HAL to set the output on the direct GPIO pin
            status = HAL_GPIO_SetLevel(fan_cfg->pinNum, state);
            if (status == E_OK) {
                ESP_LOGI(TAG, "Fan ID %d (GPIO pin %d) set to %s.",
                         fan_id, fan_cfg->pinNum,
                         (state == FAN_STATE_ON) ? "ON" : "OFF");
            } else {
                ESP_LOGE(TAG, "Failed to set Fan ID %d (GPIO pin %d) to %s. Status: %d",
                         fan_id, fan_cfg->pinNum,
                         (state == FAN_STATE_ON) ? "ON" : "OFF", status);
            }
            break;

        default:
            ESP_LOGE(TAG, "Unknown control type %d for Fan ID %d.",
                     fan_cfg->control_type, fan_id);
            return E_INVALID_PARAM; // Error for unknown control type
    }

    if (status != E_OK) {
        return status;
    }

    // Update the internal state tracking
    s_current_fan_states[fan_id] = state;
    return E_OK;
}

/**
 * @brief Gets the current state of a specific fan.
 *
 * This function retrieves the state from the internally tracked `s_current_fan_states` array.
 *
 * @param fan_id The unique identifier of the fan to query.
 * @param state_out Pointer to a variable where the current state (FAN_STATE_ON/OFF)
 * will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `fan_id` or `state_out` is invalid.
 */
Status_t FanCtrl_GetState(Fan_ID_t fan_id, Fan_State_t *state_out) {
    if (fan_id >= FAN_ID_COUNT || state_out == NULL) {
        ESP_LOGE(TAG, "Invalid Fan ID %d or NULL state_out pointer.", fan_id);
        return E_INVALID_PARAM;
    }

    *state_out = s_current_fan_states[fan_id];
    ESP_LOGD(TAG, "Fan ID %d current state is %s.", fan_id,
             (*state_out == FAN_STATE_ON) ? "ON" : "OFF");
    return E_OK;
}
