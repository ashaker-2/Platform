/* ============================================================================
 * SOURCE FILE: Application/lightCtrl/src/light_ctrl.c
 * ============================================================================*/
/**
 * @file light_ctrl.c
 * @brief Implements the Light Control (LightCtrl) module.
 *
 * This file provides the logic for initializing and controlling lights,
 * by interfacing with the Hardware Abstraction Layer (HAL),
 * supporting both CH423S expander and direct GPIO control.
 */

#include "lightctrl.h"     // Public API for LightCtrl
#include "lightctrl_cfg.h" // Configuration for light instances
#include "HAL_I2C.h"       // Dependency for controlling CH423S expander
#include "HAL_GPIO.h"      // Dependency for controlling direct GPIO
#include "esp_log.h"       // ESP-IDF logging

static const char *TAG = "LightCtrl"; // Logging tag for this module

// --- Internal Data ---
// Array to store the current state of each light. Initialized during LightCtrl_Init.
static Light_State_t s_current_light_states[LIGHT_ID_COUNT];

static Status_t LightCtrl_SetSingle(Light_ID_t light_id, Light_State_t state);
// --- Public Functions ---

/**
 * @brief Initializes the Light Control (LightCtrl) module.
 *
 * This function iterates through the `s_light_configurations` array,
 * sets up the corresponding pins (either CH423S or direct GPIO)
 * and sets the initial state for each light based on its control type.
 *
 * @return E_OK on successful initialization of all configured lights,
 * or an error code if any light configuration or HAL call fails.
 */
Status_t LightCtrl_Init(void)
{
    ESP_LOGI(TAG, "Initializing Light Control module...");

    for (size_t i = 0; i < s_num_light_configurations; i++)
    {
        const light_config_item_t *light_cfg = &s_light_configurations[i];
        Status_t status = E_OK; // Initialize status to OK

        if (light_cfg->light_id >= LIGHT_ID_COUNT)
        {
            ESP_LOGE(TAG, "Invalid Light ID %d found in configuration.", light_cfg->light_id);
            return E_INVALID_PARAM;
        }

        switch (light_cfg->control_type)
        {
        case LIGHT_CONTROL_TYPE_IO_EXPANDER:
            // Set the initial state of the light via the CH423S expander
            status = HAL_CH423S_SetOutput(light_cfg->pinNum, light_cfg->initial_state);
            if (status != E_OK)
            {
                ESP_LOGE(TAG, "Failed to set initial state for Light ID %d (CH423S pin %d). Status: %d",
                         light_cfg->light_id, light_cfg->pinNum, status);
            }
            break;

        case LIGHT_CONTROL_TYPE_GPIO:
            // Initialize the direct GPIO pin and set its initial state
            status = HAL_GPIO_SetLevel(light_cfg->pinNum, light_cfg->initial_state);
            if (status != E_OK)
            {
                ESP_LOGE(TAG, "Failed to set initial state for Light ID %d (GPIO pin %d). Status: %d",
                         light_cfg->light_id, light_cfg->pinNum, status);
            }
            break;

        default:
            ESP_LOGE(TAG, "Unknown control type %d for Light ID %d.",
                     light_cfg->control_type, light_cfg->light_id);
            return E_INVALID_PARAM; // Error for unknown control type
        }

        if (status != E_OK)
        {
            return status; // Propagate the error from HAL
        }

        // Store the initial state internally
        s_current_light_states[light_cfg->light_id] = light_cfg->initial_state;
    }

    ESP_LOGI(TAG, "Light Control module initialized successfully with %zu lights.", s_num_light_configurations);
    return E_OK;
}

/**
 * @brief Sets the state (ON or OFF) of a specific light.
 *
 * This function looks up the light's configuration and calls the appropriate
 * HAL function (CH423S or direct GPIO) to control it.
 *
 * @param light_id The unique identifier of the light to control.
 * @param state The desired state for the light (LIGHT_STATE_ON or LIGHT_STATE_OFF).
 * @return E_OK on success, E_INVALID_PARAM if `light_id` is invalid, or
 * an error code from the underlying HAL if pin control fails.
 */

Status_t LightCtrl_SetState(Light_ID_t light_id, Light_State_t state)
{
    if (light_id == LIGHT_ID_COUNT)
    {
        Status_t overall_status = E_OK;

        for (Light_ID_t id = 0; id < LIGHT_ID_COUNT; id++)
        {
            Status_t s = LightCtrl_SetSingle(id, state);
            if (s != E_OK)
            {
                overall_status = s; // record last failure
            }
        }

        return overall_status;
    }

    // Single heater case
    return LightCtrl_SetSingle(light_id, state);
}

static Status_t LightCtrl_SetSingle(Light_ID_t light_id, Light_State_t state)
{
    if (light_id > LIGHT_ID_COUNT)
    {
        ESP_LOGE(TAG, "Attempted to set state for invalid Light ID %d.", light_id);
        return E_INVALID_PARAM;
    }

    // Find the configuration for the specified light ID
    const light_config_item_t *light_cfg = NULL;
    for (size_t i = 0; i < s_num_light_configurations; i++)
    {
        if (s_light_configurations[i].light_id == light_id)
        {
            light_cfg = &s_light_configurations[i];
            break;
        }
    }

    if (light_cfg == NULL)
    {
        ESP_LOGE(TAG, "Light ID %d not found in configuration.", light_id);
        return E_INVALID_PARAM;
    }

    Status_t status = E_OK; // Initialize status to OK

    switch (light_cfg->control_type)
    {
    case LIGHT_CONTROL_TYPE_IO_EXPANDER:
        // Call the HAL to set the output on the CH423S expander
        status = HAL_CH423S_SetOutput(light_cfg->pinNum, state);
        if (status != E_OK)
        {
            ESP_LOGE(TAG, "Failed to set Light ID %d (CH423S pin %d) to %s. Status: %d",
                     light_id, light_cfg->pinNum,
                     (state == LIGHT_STATE_ON) ? "ON" : "OFF", status);
        }
        break;

    case LIGHT_CONTROL_TYPE_GPIO:
        // Call the HAL to set the output on the direct GPIO pin
        status = HAL_GPIO_SetLevel(light_cfg->pinNum, state);
        if (status == E_OK)
        {
            ESP_LOGI(TAG, "Light ID %d (GPIO pin %d) set to %s.",
                     light_id, light_cfg->pinNum,
                     (state == LIGHT_STATE_ON) ? "ON" : "OFF");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to set Light ID %d (GPIO pin %d) to %s. Status: %d",
                     light_id, light_cfg->pinNum,
                     (state == LIGHT_STATE_ON) ? "ON" : "OFF", status);
        }
        break;

    default:
        ESP_LOGE(TAG, "Unknown control type %d for Light ID %d.",
                 light_cfg->control_type, light_id);
        return E_INVALID_PARAM; // Error for unknown control type
    }

    if (status != E_OK)
    {
        return status;
    }

    // Update the internal state tracking
    s_current_light_states[light_id] = state;
    return E_OK;
}

/**
 * @brief Gets the current state of a specific light.
 *
 * This function retrieves the state from the internally tracked `s_current_light_states` array.
 *
 * @param light_id The unique identifier of the light to query.
 * @param state_out Pointer to a variable where the current state (LIGHT_STATE_ON/OFF)
 * will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `light_id` or `state_out` is invalid.
 */
Status_t LightCtrl_GetState(Light_ID_t light_id, Light_State_t *state_out)
{
    if (light_id >= LIGHT_ID_COUNT || state_out == NULL)
    {
        ESP_LOGE(TAG, "Invalid Light ID %d or NULL state_out pointer.", light_id);
        return E_INVALID_PARAM;
    }

    *state_out = s_current_light_states[light_id];
    ESP_LOGD(TAG, "Light ID %d current state is %s.", light_id,
             (*state_out == LIGHT_STATE_ON) ? "ON" : "OFF");
    return E_OK;
}
