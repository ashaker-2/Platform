#include "ventctrl.h"
#include "ventctrl_cfg.h"
#include "logger.h"
#include "system_monitor.h"
// #include "Rte.h"
// #include "hal_gpio.h"
// #include "hal_pwm.h"
// #include "hal_adc.h"
#include <string.h>

/**
 * @file ventctrl.c
 * @brief Implementation for the VentCtrl (Ventilator Control) component.
 *
 * This file contains the core logic for controlling ventilators based on the
 * configuration provided in ventctrl_cfg.c/.h.
 */

// --- Internal State Variables ---
static VentCtrl_State_t s_commanded_states[VentCtrl_COUNT];
static uint8_t s_commanded_speeds_percent[VentCtrl_COUNT];
static VentCtrl_State_t s_actual_states[VentCtrl_COUNT];
static uint8_t s_actual_speeds_percent[VentCtrl_COUNT];
static bool s_is_initialized = false;

// --- Private Helper Function Prototypes ---
static Status_t VentCtrl_ApplyControl(const VentCtrl_Config_t *config, VentCtrl_State_t state, uint8_t speed);
static Status_t VentCtrl_ReadFeedback(const VentCtrl_Config_t *config, uint8_t *actual_speed);
static Status_t VentCtrl_ConvertTachometerToSpeed(uint32_t pulse_count, uint8_t *speed_percent);

// --- Public Function Implementations ---

Status_t VentCtrl_Init(void)
{
    // if (s_is_initialized) {
    //     return E_OK;
    // }

    // // Initialize internal state variables to a known safe state
    // memset(s_commanded_states, VentCtrl_STATE_OFF, sizeof(s_commanded_states));
    // memset(s_commanded_speeds_percent, 0, sizeof(s_commanded_speeds_percent));
    // memset(s_actual_states, VentCtrl_STATE_OFF, sizeof(s_actual_states));
    // memset(s_actual_speeds_percent, 0, sizeof(s_actual_speeds_percent));

    // // Initialize hardware for each configured ventilator
    // for (uint32_t i = 0; i < VentCtrl_COUNT; i++) {
    //     const VentCtrl_Config_t* config = &vent_configs[i];
    //     Status_t status = E_NOK;

    //     // Initialize control interface
    //     switch (config->type) {
    //         case VentCtrl_TYPE_RELAY:
    //             status = MCAL_GPIO_Init(config->control_details.relay.relay_gpio_pin, GPIO_MODE_OUTPUT);
    //             break;
    //         case VentCtrl_TYPE_PWM:
    //             status = MCAL_PWM_Init(config->control_details.pwm.pwm_channel);
    //             break;
    //         default:
    //             LOGE("VentCtrl: Unknown ventilator type for ID %lu", config->id);
    //             RTE_Service_SystemMonitor_ReportFault(FAULT_ID_VENTILATOR_INIT_FAILED,  config->id);
    //             return E_NOK;
    //     }

    //     if (status != E_OK) {
    //         LOGE("VentCtrl: Control interface init failed for ID %lu", config->id);
    //         RTE_Service_SystemMonitor_ReportFault(FAULT_ID_VENTILATOR_INIT_FAILED,  config->id);
    //         return E_NOK;
    //     }

    //     // Initialize feedback interface (if configured)
    //     if (config->feedback_type != VentCtrl_FEEDBACK_TYPE_NONE) {
    //         status = E_NOK;
    //         switch (config->feedback_type) {
    //             case VentCtrl_FEEDBACK_TYPE_TACHOMETER:
    //                 status = MCAL_GPIO_Init(config->feedback_details.tachometer.tachometer_gpio_pin, GPIO_MODE_INPUT_IT_RISING);
    //                 break;
    //             case VentCtrl_FEEDBACK_TYPE_ANALOG_ADC:
    //                 status = MCAL_ADC_Init(config->feedback_details.analog_adc.adc_channel);
    //                 break;
    //             default:
    //                 // LOGW("VentCtrl: Unknown feedback type for ID %lu", config->id);
    //                 break; // Non-fatal, continue initialization
    //         }
    //         if (status != E_OK) {
    //             LOGW("VentCtrl: Feedback interface init failed for ID %lu", config->id);
    //             // Report a medium severity fault as the ventilator might still be controllable
    //             RTE_Service_SystemMonitor_ReportFault(FAULT_ID_VENTILATOR_FEEDBACK_FAILURE,  config->id);
    //         }
    //     }
    // }

    // s_is_initialized = true;
    // LOGI("VentCtrl: Module initialized successfully.");
    return E_OK;
}

Status_t VentCtrl_SetState(uint32_t actuatorId, VentCtrl_State_t state, uint8_t speed_percent)
{
    // if (!s_is_initialized) {
    //     LOGW("VentCtrl: SetState called before initialization.");
    //     return E_NOK;
    // }

    // if (actuatorId >= VentCtrl_COUNT) {
    //     LOGE("VentCtrl: Invalid actuatorId %lu", actuatorId);
    //     return E_NOK;
    // }

    // if (state >= VentCtrl_STATE_COUNT) {
    //     LOGE("VentCtrl: Invalid state %u for actuatorId %lu", state, actuatorId);
    //     return E_NOK;
    // }

    // if (speed_percent > VentCtrl_MAX_PWM_SPEED_PERCENT) {
    //     speed_percent = VentCtrl_MAX_PWM_SPEED_PERCENT;
    //     LOGW("VentCtrl: Clamping speed_percent to max for actuatorId %lu", actuatorId);
    // }

    // s_commanded_states[actuatorId] = state;
    // s_commanded_speeds_percent[actuatorId] = speed_percent;

    // LOGD("VentCtrl: Actuator %lu commanded to state %u, speed %u%%", actuatorId, state, speed_percent);
    return E_OK;
}

Status_t VentCtrl_GetState(uint32_t actuatorId, VentCtrl_State_t *state, uint8_t *speed_percent)
{
    // if (!s_is_initialized || state == NULL || speed_percent == NULL) {
    //     LOGE("VentCtrl: GetState called with NULL pointer or before initialization.");
    //     return E_NOK;
    // }
    // if (actuatorId >= VentCtrl_COUNT) {
    //     LOGE("VentCtrl: Invalid actuatorId %lu", actuatorId);
    //     return E_NOK;
    // }

    // *state = s_actual_states[actuatorId];
    // *speed_percent = s_actual_speeds_percent[actuatorId];

    return E_OK;
}

void VentCtrl_MainFunction(void)
{
    // if (!s_is_initialized) {
    //     return;
    // }

    // for (uint32_t i = 0; i < VentCtrl_COUNT; i++) {
    //     const VentCtrl_Config_t* config = &vent_configs[i];

    //     // 1. Apply commanded state/speed to hardware
    //     if (VentCtrl_ApplyControl(config, s_commanded_states[i], s_commanded_speeds_percent[i]) != E_OK) {
    //         RTE_Service_SystemMonitor_ReportFault(FAULT_ID_VENTILATOR_CONTROL_FAILED,  config->id);
    //         LOGE("VentCtrl: Control failed for actuator ID %lu", config->id);
    //         // On failure, set actual state to off
    //         s_actual_states[i] = VentCtrl_STATE_OFF;
    //         s_actual_speeds_percent[i] = 0;
    //         continue; // Move to next ventilator
    //     }

    //     // 2. Read feedback (if configured)
    //     uint8_t actual_speed = 0;
    //     if (config->feedback_type != VentCtrl_FEEDBACK_TYPE_NONE) {
    //         if (VentCtrl_ReadFeedback(config, &actual_speed) != E_OK) {
    //             RTE_Service_SystemMonitor_ReportFault(FAULT_ID_VENTILATOR_FEEDBACK_FAILURE,  config->id);
    //             LOGW("VentCtrl: Feedback read failed for actuator ID %lu", config->id);
    //         } else {
    //             // 3. Compare commanded vs. actual (only if feedback is available)
    //             if (s_commanded_states[i] == VentCtrl_STATE_ON && actual_speed < VentCtrl_MIN_OPERATIONAL_SPEED) {
    //                 RTE_Service_SystemMonitor_ReportFault(FAULT_ID_VENTILATOR_FEEDBACK_MISMATCH,  config->id);
    //                 LOGW("VentCtrl: Feedback mismatch for actuator ID %lu. Commanded ON, but actual speed is %u%%", config->id, actual_speed);
    //                 s_actual_states[i] = VentCtrl_STATE_OFF;
    //             } else if (s_commanded_states[i] == VentCtrl_STATE_OFF && actual_speed > VentCtrl_MIN_OPERATIONAL_SPEED) {
    //                 RTE_Service_SystemMonitor_ReportFault(FAULT_ID_VENTILATOR_FEEDBACK_MISMATCH,  config->id);
    //                 LOGW("VentCtrl: Feedback mismatch for actuator ID %lu. Commanded OFF, but actual speed is %u%%", config->id, actual_speed);
    //                 s_actual_states[i] = VentCtrl_STATE_ON;
    //             } else {
    //                 s_actual_states[i] = s_commanded_states[i];
    //             }
    //             s_actual_speeds_percent[i] = actual_speed;
    //         }
    //     } else {
    //          // If no feedback, assume commanded state is the actual state
    //          s_actual_states[i] = s_commanded_states[i];
    //          s_actual_speeds_percent[i] = s_commanded_speeds_percent[i];
    //     }
    // }
}

// --- Private Helper Function Implementations ---

static Status_t VentCtrl_ApplyControl(const VentCtrl_Config_t *config, VentCtrl_State_t state, uint8_t speed)
{
    // if (state == VentCtrl_STATE_OFF) {
    //     speed = 0;
    // }

    // switch (config->type) {
    //     case VentCtrl_TYPE_RELAY:
    //         // For relay, ON/OFF is sufficient
    //         return MCAL_GPIO_WritePin(config->control_details.relay.relay_gpio_pin, (state == VentCtrl_STATE_ON) ? GPIO_STATE_HIGH : GPIO_STATE_LOW);
    //     case VentCtrl_TYPE_PWM:
    //         return MCAL_PWM_SetDutyCycle(config->control_details.pwm.pwm_channel, speed);
    //     default:
    //         return E_NOK;
    // }
    return E_NOK;
}

static Status_t VentCtrl_ReadFeedback(const VentCtrl_Config_t *config, uint8_t *actual_speed)
{
    // uint32_t raw_data = 0;
    Status_t status = E_NOK;

    // switch (config->feedback_type) {
    //     case VentCtrl_FEEDBACK_TYPE_TACHOMETER:
    //         // This would involve reading a pulse counter, which is a complex topic.
    //         // Placeholder: Assume a global tachometer counter is available.
    //         // For this example, we'll assume a successful read means it's on.
    //         // A more robust implementation would read a counter and calculate RPM.
    //         status = E_OK;
    //         // Simplified logic: If commanded on, assume it's on.
    //         if (s_commanded_states[config->id] == VentCtrl_STATE_ON) {
    //             *actual_speed = s_commanded_speeds_percent[config->id];
    //         } else {
    //             *actual_speed = 0;
    //         }
    //         break;
    //     case VentCtrl_FEEDBACK_TYPE_ANALOG_ADC:
    //         status = MCAL_ADC_Read(config->feedback_details.analog_adc.adc_channel, &raw_data);
    //         if (status == E_OK) {
    //             float voltage = (raw_data / 4096.0f) * 3.3f; // Example for 12-bit ADC, 3.3V ref
    //             float converted_speed = (voltage * config->feedback_details.analog_adc.voltage_to_speed_slope) + config->feedback_details.analog_adc.voltage_to_speed_offset;
    //             *actual_speed = (uint8_t)converted_speed;
    //             // Clamp the speed to 0-100
    //             if (*actual_speed > 100) *actual_speed = 100;
    //             if (*actual_speed < 0) *actual_speed = 0;
    //         }
    //         break;
    //     default:
    //         return E_NOK; // Should not be reached if feedback_type != NONE
    // }
    return status;
}