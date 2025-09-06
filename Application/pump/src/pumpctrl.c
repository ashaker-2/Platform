/* ============================================================================
 * SOURCE FILE: Application/pumpCtrl/src/pump_ctrl.c
 * ============================================================================*/
/**
 * @file pump_ctrl.c
 * @brief Implements the Pump Control (PumpCtrl) module.
 *
 * This file provides the logic for initializing and controlling pumps,
 * by interfacing with the Hardware Abstraction Layer (HAL),
 * supporting both CH423S expander and direct GPIO control.
 */

#include "pumpctrl.h"     // Public API for PumpCtrl
#include "pumpctrl_cfg.h" // Configuration for pump instances
#include "HAL_I2C.h"      // Dependency for controlling CH423S expander
#include "HAL_GPIO.h"     // Dependency for controlling direct GPIO
#include "esp_log.h"      // ESP-IDF logging

static const char *TAG = "PumpCtrl"; // Logging tag for this module

// --- Internal Data ---
// Array to store the current state of each pump. Initialized during PumpCtrl_Init.
static Pump_State_t s_current_pump_states[PUMP_ID_COUNT];

static Status_t PumpCtrl_SetSingle(Pump_ID_t pump_id, Pump_State_t state);

// --- Public Functions ---

/**
 * @brief Initializes the Pump Control (PumpCtrl) module.
 *
 * This function iterates through the `s_pump_configurations` array,
 * sets up the corresponding pins (either CH423S or direct GPIO)
 * and sets the initial state for each pump based on its control type.
 *
 * @return E_OK on successful initialization of all configured pumps,
 * or an error code if any pump configuration or HAL call fails.
 */
Status_t PumpCtrl_Init(void)
{
    ESP_LOGI(TAG, "Initializing Pump Control module...");

    for (size_t i = 0; i < s_num_pump_configurations; i++)
    {
        const pump_config_item_t *pump_cfg = &s_pump_configurations[i];
        Status_t status = E_OK; // Initialize status to OK

        if (pump_cfg->pump_id >= PUMP_ID_COUNT)
        {
            ESP_LOGE(TAG, "Invalid Pump ID %d found in configuration.", pump_cfg->pump_id);
            return E_INVALID_PARAM;
        }

        switch (pump_cfg->control_type)
        {
        case PUMP_CONTROL_TYPE_IO_EXPANDER:
            // Set the initial state of the pump via the CH423S expander
            status = HAL_CH423S_SetOutput(pump_cfg->pinNum, pump_cfg->initial_state);
            if (status != E_OK)
            {
                ESP_LOGE(TAG, "Failed to set initial state for Pump ID %d (CH423S pin %d). Status: %d",
                         pump_cfg->pump_id, pump_cfg->pinNum, status);
            }
            break;

        case PUMP_CONTROL_TYPE_GPIO:
            // Initialize the direct GPIO pin and set its initial state
            status = HAL_GPIO_SetLevel(pump_cfg->pinNum, pump_cfg->initial_state);
            if (status != E_OK)
            {
                ESP_LOGE(TAG, "Failed to set initial state for Pump ID %d (GPIO pin %d). Status: %d",
                         pump_cfg->pump_id, pump_cfg->pinNum, status);
            }
            break;

        default:
            ESP_LOGE(TAG, "Unknown control type %d for Pump ID %d.",
                     pump_cfg->control_type, pump_cfg->pump_id);
            return E_INVALID_PARAM; // Error for unknown control type
        }

        if (status != E_OK)
        {
            return status; // Propagate the error from HAL
        }

        // Store the initial state internally
        s_current_pump_states[pump_cfg->pump_id] = pump_cfg->initial_state;
    }

    ESP_LOGI(TAG, "Pump Control module initialized successfully with %zu pumps.", s_num_pump_configurations);
    return E_OK;
}

/**
 * @brief Sets the state (ON or OFF) of a specific pump.
 *
 * This function looks up the pump's configuration and calls the appropriate
 * HAL function (CH423S or direct GPIO) to control it.
 *
 * @param pump_id The unique identifier of the pump to control.
 * @param state The desired state for the pump (PUMP_STATE_ON or PUMP_STATE_OFF).
 * @return E_OK on success, E_INVALID_PARAM if `pump_id` is invalid, or
 * an error code from the underlying HAL if pin control fails.
 */

Status_t PumpCtrl_SetState(Pump_ID_t pump_id, Pump_State_t state)
{
    if (pump_id == PUMP_ID_COUNT)
    {
        Status_t overall_status = E_OK;

        for (Pump_ID_t id = 0; id < PUMP_ID_COUNT; id++)
        {
            Status_t s = PumpCtrl_SetSingle(id, state);
            if (s != E_OK)
            {
                overall_status = s; // record last failure
            }
        }

        return overall_status;
    }

    // Single heater case
    return PumpCtrl_SetSingle(pump_id, state);
}

static Status_t PumpCtrl_SetSingle(Pump_ID_t pump_id, Pump_State_t state)
{
    if (pump_id > PUMP_ID_COUNT)
    {
        ESP_LOGE(TAG, "Attempted to set state for invalid Pump ID %d.", pump_id);
        return E_INVALID_PARAM;
    }

    // Find the configuration for the specified pump ID
    const pump_config_item_t *pump_cfg = NULL;
    for (size_t i = 0; i < s_num_pump_configurations; i++)
    {
        if (s_pump_configurations[i].pump_id == pump_id)
        {
            pump_cfg = &s_pump_configurations[i];
            break;
        }
    }

    if (pump_cfg == NULL)
    {
        ESP_LOGE(TAG, "Pump ID %d not found in configuration.", pump_id);
        return E_INVALID_PARAM;
    }

    Status_t status = E_OK; // Initialize status to OK

    switch (pump_cfg->control_type)
    {
    case PUMP_CONTROL_TYPE_IO_EXPANDER:
        // Call the HAL to set the output on the CH423S expander
        status = HAL_CH423S_SetOutput(pump_cfg->pinNum, state);
        if (status != E_OK)
        {
            ESP_LOGE(TAG, "Failed to set Pump ID %d (CH423S pin %d) to %s. Status: %d",
                     pump_id, pump_cfg->pinNum,
                     (state == PUMP_STATE_ON) ? "ON" : "OFF", status);
        }
        break;

    case PUMP_CONTROL_TYPE_GPIO:
        // Call the HAL to set the output on the direct GPIO pin
        status = HAL_GPIO_SetLevel(pump_cfg->pinNum, state);
        if (status == E_OK)
        {
            ESP_LOGI(TAG, "Pump ID %d (GPIO pin %d) set to %s.",
                     pump_id, pump_cfg->pinNum,
                     (state == PUMP_STATE_ON) ? "ON" : "OFF");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to set Pump ID %d (GPIO pin %d) to %s. Status: %d",
                     pump_id, pump_cfg->pinNum,
                     (state == PUMP_STATE_ON) ? "ON" : "OFF", status);
        }
        break;

    default:
        ESP_LOGE(TAG, "Unknown control type %d for Pump ID %d.",
                 pump_cfg->control_type, pump_id);
        return E_INVALID_PARAM; // Error for unknown control type
    }

    if (status != E_OK)
    {
        return status;
    }

    // Update the internal state tracking
    s_current_pump_states[pump_id] = state;
    return E_OK;
}

/**
 * @brief Gets the current state of a specific pump.
 *
 * This function retrieves the state from the internally tracked `s_current_pump_states` array.
 *
 * @param pump_id The unique identifier of the pump to query.
 * @param state_out Pointer to a variable where the current state (PUMP_STATE_ON/OFF)
 * will be stored. Must not be NULL.
 * @return E_OK on success, E_INVALID_PARAM if `pump_id` or `state_out` is invalid.
 */
Status_t PumpCtrl_GetState(Pump_ID_t pump_id, Pump_State_t *state_out)
{
    if (pump_id >= PUMP_ID_COUNT || state_out == NULL)
    {
        ESP_LOGE(TAG, "Invalid Pump ID %d or NULL state_out pointer.", pump_id);
        return E_INVALID_PARAM;
    }

    *state_out = s_current_pump_states[pump_id];
    ESP_LOGD(TAG, "Pump ID %d current state is %s.", pump_id,
             (*state_out == PUMP_STATE_ON) ? "ON" : "OFF");
    return E_OK;
}
