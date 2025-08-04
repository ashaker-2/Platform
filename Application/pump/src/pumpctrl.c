#include "pumpctrl.h"
#include "pumpctrl_cfg.h"
#include "logger.h"
#include "system_monitor.h"
#include "Rte.h"
#include "mcal_gpio.h"
#include "mcal_adc.h"
#include <string.h>

/**
 * @file pumpctrl.c
 * @brief Implementation for the PumpCtrl (Pump Control) component.
 *
 * This file contains the core logic for controlling water pumps based on the
 * configuration provided in pumpctrl_cfg.c/.h.
 */

// --- Internal State Variables ---
static PumpCtrl_State_t s_commanded_states[PumpCtrl_COUNT];
static PumpCtrl_State_t s_actual_states[PumpCtrl_COUNT];
static bool s_is_initialized = false;

// --- Private Helper Function Prototypes ---
static APP_Status_t PumpCtrl_ApplyControl(const PumpCtrl_Config_t* config, PumpCtrl_State_t state);
static APP_Status_t PumpCtrl_ReadFeedback(const PumpCtrl_Config_t* config, PumpCtrl_State_t* actual_state);

// --- Public Function Implementations ---

APP_Status_t PumpCtrl_Init(void) {
    if (s_is_initialized) {
        return APP_OK;
    }

    // Initialize internal state variables to a known safe state
    memset(s_commanded_states, PumpCtrl_STATE_OFF, sizeof(s_commanded_states));
    memset(s_actual_states, PumpCtrl_STATE_OFF, sizeof(s_actual_states));

    // Initialize hardware for each configured pump
    for (uint32_t i = 0; i < PumpCtrl_COUNT; i++) {
        const PumpCtrl_Config_t* config = &pump_configs[i];
        APP_Status_t status = APP_ERROR;

        // Initialize control interface
        switch (config->type) {
            case PumpCtrl_TYPE_RELAY:
                status = MCAL_GPIO_Init(config->control_details.relay_gpio_pin, GPIO_MODE_OUTPUT);
                break;
            default:
                LOGE("PumpCtrl: Unknown pump type for ID %lu", config->id);
                RTE_Service_SystemMonitor_ReportFault(FAULT_ID_PUMP_INIT_FAILED, SEVERITY_HIGH, config->id);
                return APP_ERROR;
        }

        if (status != APP_OK) {
            LOGE("PumpCtrl: Control interface init failed for ID %lu", config->id);
            RTE_Service_SystemMonitor_ReportFault(FAULT_ID_PUMP_INIT_FAILED, SEVERITY_HIGH, config->id);
            return APP_ERROR;
        }
        
        // Initialize feedback interface (if configured)
        if (config->feedback_type != PumpCtrl_FEEDBACK_TYPE_NONE) {
            status = APP_ERROR;
            switch (config->feedback_type) {
                case PumpCtrl_FEEDBACK_TYPE_FLOW_SENSOR:
                    // Assuming pulse-based flow sensor for this example
                    status = MCAL_GPIO_Init(config->feedback_details.flow_sensor_pulse.gpio_pin, GPIO_MODE_INPUT_IT_RISING);
                    break;
                case PumpCtrl_FEEDBACK_TYPE_CURRENT_SENSOR:
                    status = MCAL_ADC_Init(config->feedback_details.current_sensor.adc_channel);
                    break;
                default:
                    LOGW("PumpCtrl: Unknown feedback type for ID %lu", config->id);
                    break; // Non-fatal, continue initialization
            }
            if (status != APP_OK) {
                LOGW("PumpCtrl: Feedback interface init failed for ID %lu", config->id);
                RTE_Service_SystemMonitor_ReportFault(FAULT_ID_PUMP_FEEDBACK_FAILURE, SEVERITY_MEDIUM, config->id);
            }
        }
    }

    s_is_initialized = true;
    LOGI("PumpCtrl: Module initialized successfully.");
    return APP_OK;
}

APP_Status_t PumpCtrl_SetState(uint32_t actuatorId, PumpCtrl_State_t state) {
    if (!s_is_initialized) {
        LOGW("PumpCtrl: SetState called before initialization.");
        return APP_ERROR;
    }

    if (actuatorId >= PumpCtrl_COUNT) {
        LOGE("PumpCtrl: Invalid actuatorId %lu", actuatorId);
        return APP_ERROR;
    }

    if (state >= PumpCtrl_STATE_COUNT) {
        LOGE("PumpCtrl: Invalid state %u for actuatorId %lu", state, actuatorId);
        return APP_ERROR;
    }
    
    s_commanded_states[actuatorId] = state;

    LOGD("PumpCtrl: Actuator %lu commanded to state %s", actuatorId, (state == PumpCtrl_STATE_ON) ? "ON" : "OFF");
    return APP_OK;
}

APP_Status_t PumpCtrl_GetState(uint32_t actuatorId, PumpCtrl_State_t *state) {
    if (!s_is_initialized || state == NULL) {
        LOGE("PumpCtrl: GetState called with NULL pointer or before initialization.");
        return APP_ERROR;
    }
    if (actuatorId >= PumpCtrl_COUNT) {
        LOGE("PumpCtrl: Invalid actuatorId %lu", actuatorId);
        return APP_ERROR;
    }

    *state = s_actual_states[actuatorId];

    return APP_OK;
}

void PumpCtrl_MainFunction(void) {
    if (!s_is_initialized) {
        return;
    }
    
    for (uint32_t i = 0; i < PumpCtrl_COUNT; i++) {
        const PumpCtrl_Config_t* config = &pump_configs[i];
        
        // 1. Apply commanded state to hardware
        if (PumpCtrl_ApplyControl(config, s_commanded_states[i]) != APP_OK) {
            RTE_Service_SystemMonitor_ReportFault(FAULT_ID_PUMP_CONTROL_FAILED, SEVERITY_HIGH, config->id);
            LOGE("PumpCtrl: Control failed for actuator ID %lu", config->id);
            // On failure, set actual state to off
            s_actual_states[i] = PumpCtrl_STATE_OFF;
            continue; // Move to next pump
        }
        
        // 2. Read feedback (if configured)
        PumpCtrl_State_t actual_state_feedback = PumpCtrl_STATE_OFF;
        if (config->feedback_type != PumpCtrl_FEEDBACK_TYPE_NONE) {
            if (PumpCtrl_ReadFeedback(config, &actual_state_feedback) != APP_OK) {
                RTE_Service_SystemMonitor_ReportFault(FAULT_ID_PUMP_FEEDBACK_FAILURE, SEVERITY_MEDIUM, config->id);
                LOGW("PumpCtrl: Feedback read failed for actuator ID %lu", config->id);
            } else {
                // 3. Compare commanded vs. actual
                if (s_commanded_states[i] != actual_state_feedback) {
                    RTE_Service_SystemMonitor_ReportFault(FAULT_ID_PUMP_FEEDBACK_MISMATCH, SEVERITY_MEDIUM, config->id);
                    LOGW("PumpCtrl: Feedback mismatch for actuator ID %lu. Commanded: %s, Actual: %s",
                         config->id,
                         (s_commanded_states[i] == PumpCtrl_STATE_ON) ? "ON" : "OFF",
                         (actual_state_feedback == PumpCtrl_STATE_ON) ? "ON" : "OFF");
                }
                s_actual_states[i] = actual_state_feedback;
            }
        } else {
             // If no feedback, assume commanded state is the actual state
             s_actual_states[i] = s_commanded_states[i];
        }
    }
}

// --- Private Helper Function Implementations ---

static APP_Status_t PumpCtrl_ApplyControl(const PumpCtrl_Config_t* config, PumpCtrl_State_t state) {
    switch (config->type) {
        case PumpCtrl_TYPE_RELAY:
            return MCAL_GPIO_WritePin(config->control_details.relay_gpio_pin, (state == PumpCtrl_STATE_ON) ? GPIO_STATE_HIGH : GPIO_STATE_LOW);
        default:
            return APP_ERROR;
    }
}

static APP_Status_t PumpCtrl_ReadFeedback(const PumpCtrl_Config_t* config, PumpCtrl_State_t* actual_state) {
    uint32_t raw_data = 0;
    APP_Status_t status = APP_ERROR;
    float current_flow = 0.0f;
    float current_current = 0.0f;

    switch (config->feedback_type) {
        case PumpCtrl_FEEDBACK_TYPE_FLOW_SENSOR:
            // This is a simplified placeholder for reading a pulse-based flow sensor.
            // A real implementation would involve a pulse-counting ISR.
            status = APP_OK;
            // Simplified logic: If commanded on, assume it's on.
            if (s_commanded_states[config->id] == PumpCtrl_STATE_ON) {
                *actual_state = PumpCtrl_STATE_ON;
            } else {
                *actual_state = PumpCtrl_STATE_OFF;
            }
            break;
        case PumpCtrl_FEEDBACK_TYPE_CURRENT_SENSOR:
            status = MCAL_ADC_Read(config->feedback_details.current_sensor.adc_channel, &raw_data);
            if (status == APP_OK) {
                // Simplified conversion from raw ADC to current
                current_current = (float)raw_data * 0.01f; // Example conversion factor
                if (current_current >= config->feedback_details.current_sensor.current_threshold_on) {
                    *actual_state = PumpCtrl_STATE_ON;
                } else {
                    *actual_state = PumpCtrl_STATE_OFF;
                }
            }
            break;
        default:
            return APP_ERROR;
    }
    return status;
}