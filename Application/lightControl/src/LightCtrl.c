#include "lightctrl.h"
#include "lightctrl_cfg.h"
#include "logger.h"
#include "system_monitor.h"
// #include "Rte.h"
// #include "hal_gpio.h"
// #include "hal_pwm.h"
// #include "hal_adc.h"
#include <string.h>

/**
 * @file lightctrl.c
 * @brief Implementation for the LightCtrl (Light Control) component.
 *
 * This file contains the core logic for controlling lights based on the
 * configuration provided in lightctrl_cfg.c/.h.
 */

// --- Internal State Variables ---
static LightCtrl_State_t s_commanded_states[LightCtrl_COUNT];
static uint8_t s_commanded_brightness_percent[LightCtrl_COUNT];
static LightCtrl_State_t s_actual_states[LightCtrl_COUNT];
static uint8_t s_actual_brightness_percent[LightCtrl_COUNT];
static bool s_is_initialized = false;

// --- Private Helper Function Prototypes ---
static Status_t LightCtrl_ApplyControl(const LightCtrl_Config_t *config, LightCtrl_State_t state, uint8_t brightness);
static Status_t LightCtrl_ReadFeedback(const LightCtrl_Config_t *config, uint8_t *actual_brightness);
static Status_t LightCtrl_ConvertAnalogToBrightness(uint32_t raw_data, const LightCtrl_FeedbackDetails_t *details, uint8_t *brightness);

// --- Public Function Implementations ---

Status_t LightCtrl_Init(void)
{
    // if (s_is_initialized)
    // {
    //     return E_OK;
    // }

    // Initialize internal state variables to a known safe state
    // memset(s_commanded_states, LightCtrl_STATE_OFF, sizeof(s_commanded_states));
    // memset(s_commanded_brightness_percent, 0, sizeof(s_commanded_brightness_percent));
    // memset(s_actual_states, LightCtrl_STATE_OFF, sizeof(s_actual_states));
    // memset(s_actual_brightness_percent, 0, sizeof(s_actual_brightness_percent));

    // Initialize hardware for each configured light
    // for (uint32_t i = 0; i < LightCtrl_COUNT; i++)
    // {
    //     const LightCtrl_Config_t* config = &light_configs[i];
    //     Status_t status = E_NOK;

    //     // Initialize control interface
    //     switch (config->type) {
    //         case LightCtrl_TYPE_RELAY:
    //             status = MCAL_GPIO_Init(config->control_details.relay.relay_gpio_pin, GPIO_MODE_OUTPUT);
    //             break;
    //         case LightCtrl_TYPE_PWM:
    //             status = MCAL_PWM_Init(config->control_details.pwm.pwm_channel);
    //             break;
    //         default:
    //             LOGE("LightCtrl: Unknown light type for ID %lu", config->id);
    //             RTE_Service_SystemMonitor_ReportFault(FAULT_ID_LIGHT_INIT_FAILED,  config->id);
    //             return E_NOK;
    //     }

    //     if (status != E_OK)
    //     {
    //         LOGE("LightCtrl: Control interface init failed for ID %lu", config->id);
    //         RTE_Service_SystemMonitor_ReportFault(FAULT_ID_LIGHT_INIT_FAILED,  config->id);
    //         return E_NOK;
    //     }

    //     // Initialize feedback interface (if configured)
    //     if (config->feedback_type != LightCtrl_FEEDBACK_TYPE_NONE)
    //     {
    //         status = E_NOK;
    //         switch (config->feedback_type) {
    //             case LightCtrl_FEEDBACK_TYPE_CURRENT_SENSOR:
    //             case LightCtrl_FEEDBACK_TYPE_LIGHT_SENSOR:
    //                 status = MCAL_ADC_Init(config->feedback_details.analog_adc.adc_channel);
    //                 break;
    //             default:
    //                 LOGW("LightCtrl: Unknown feedback type for ID %lu", config->id);
    //                 break; // Non-fatal, continue initialization
    //         }
    //         if (status != E_OK) {
    //             LOGW("LightCtrl: Feedback interface init failed for ID %lu", config->id);
    //             // Report a medium severity fault as the light might still be controllable
    //             RTE_Service_SystemMonitor_ReportFault(FAULT_ID_LIGHT_FEEDBACK_FAILURE,  config->id);
    //         }
    //     }
    // }

    // s_is_initialized = true;
    // LOGI("LightCtrl: Module initialized successfully.");
    return E_OK;
}

Status_t LightCtrl_SetState(uint32_t actuatorId, LightCtrl_State_t state, uint8_t brightness_percent)
{
    // if (!s_is_initialized) {
    //     LOGW("LightCtrl: SetState called before initialization.");
    //     return E_NOK;
    // }

    // if (actuatorId >= LightCtrl_COUNT) {
    //     LOGE("LightCtrl: Invalid actuatorId %lu", actuatorId);
    //     return E_NOK;
    // }

    // if (state >= LightCtrl_STATE_COUNT) {
    //     LOGE("LightCtrl: Invalid state %u for actuatorId %lu", state, actuatorId);
    //     return E_NOK;
    // }

    // if (brightness_percent > LightCtrl_MAX_BRIGHTNESS_PERCENT) {
    //     brightness_percent = LightCtrl_MAX_BRIGHTNESS_PERCENT;
    //     LOGW("LightCtrl: Clamping brightness_percent to max for actuatorId %lu", actuatorId);
    // }

    // s_commanded_states[actuatorId] = state;
    // s_commanded_brightness_percent[actuatorId] = brightness_percent;

    // LOGD("LightCtrl: Actuator %lu commanded to state %u, brightness %u%%", actuatorId, state, brightness_percent);
    return E_OK;
}

Status_t LightCtrl_GetState(uint32_t actuatorId, LightCtrl_State_t *state, uint8_t *brightness_percent)
{
    // if (!s_is_initialized || state == NULL || brightness_percent == NULL) {
    //     LOGE("LightCtrl: GetState called with NULL pointer or before initialization.");
    //     return E_NOK;
    // }
    // if (actuatorId >= LightCtrl_COUNT) {
    //     LOGE("LightCtrl: Invalid actuatorId %lu", actuatorId);
    //     return E_NOK;
    // }

    // *state = s_actual_states[actuatorId];
    // *brightness_percent = s_actual_brightness_percent[actuatorId];

    return E_OK;
}

void LightCtrl_MainFunction(void)
{
    // if (!s_is_initialized) {
    //     return;
    // }

    // for (uint32_t i = 0; i < LightCtrl_COUNT; i++) {
    //     const LightCtrl_Config_t* config = &light_configs[i];

    //     // 1. Apply commanded state/brightness to hardware
    //     if (LightCtrl_ApplyControl(config, s_commanded_states[i], s_commanded_brightness_percent[i]) != E_OK) {
    //         RTE_Service_SystemMonitor_ReportFault(FAULT_ID_LIGHT_CONTROL_FAILED,  config->id);
    //         LOGE("LightCtrl: Control failed for actuator ID %lu", config->id);
    //         // On failure, set actual state to off
    //         s_actual_states[i] = LightCtrl_STATE_OFF;
    //         s_actual_brightness_percent[i] = 0;
    //         continue; // Move to next light
    //     }

    //     // 2. Read feedback (if configured)
    //     uint8_t actual_brightness = 0;
    //     if (config->feedback_type != LightCtrl_FEEDBACK_TYPE_NONE) {
    //         if (LightCtrl_ReadFeedback(config, &actual_brightness) != E_OK) {
    //             RTE_Service_SystemMonitor_ReportFault(FAULT_ID_LIGHT_FEEDBACK_FAILURE,  config->id);
    //             LOGW("LightCtrl: Feedback read failed for actuator ID %lu", config->id);
    //         } else {
    //             // 3. Compare commanded vs. actual (only if feedback is available)
    //             if (s_commanded_states[i] == LightCtrl_STATE_ON && actual_brightness < LightCtrl_MIN_OPERATIONAL_BRIGHTNESS) {
    //                 RTE_Service_SystemMonitor_ReportFault(FAULT_ID_LIGHT_FEEDBACK_MISMATCH,  config->id);
    //                 LOGW("LightCtrl: Feedback mismatch for actuator ID %lu. Commanded ON, but actual brightness is %u%%", config->id, actual_brightness);
    //                 s_actual_states[i] = LightCtrl_STATE_OFF;
    //             } else if (s_commanded_states[i] == LightCtrl_STATE_OFF && actual_brightness >= LightCtrl_MIN_OPERATIONAL_BRIGHTNESS) {
    //                 RTE_Service_SystemMonitor_ReportFault(FAULT_ID_LIGHT_FEEDBACK_MISMATCH,  config->id);
    //                 LOGW("LightCtrl: Feedback mismatch for actuator ID %lu. Commanded OFF, but actual brightness is %u%%", config->id, actual_brightness);
    //                 s_actual_states[i] = LightCtrl_STATE_ON;
    //             } else {
    //                 s_actual_states[i] = s_commanded_states[i];
    //             }
    //             s_actual_brightness_percent[i] = actual_brightness;
    //         }
    //     } else {
    //          // If no feedback, assume commanded state is the actual state
    //          s_actual_states[i] = s_commanded_states[i];
    //          s_actual_brightness_percent[i] = s_commanded_brightness_percent[i];
    //     }
    // }
}

// --- Private Helper Function Implementations ---

static Status_t LightCtrl_ApplyControl(const LightCtrl_Config_t *config, LightCtrl_State_t state, uint8_t brightness)
{
    // if (state == LightCtrl_STATE_OFF) {
    //     brightness = 0;
    // }

    // switch (config->type) {
    //     case LightCtrl_TYPE_RELAY:
    //         // For relay, ON/OFF is sufficient
    //         return MCAL_GPIO_WritePin(config->control_details.relay.relay_gpio_pin, (state == LightCtrl_STATE_ON) ? GPIO_STATE_HIGH : GPIO_STATE_LOW);
    //     case LightCtrl_TYPE_PWM:
    //         return MCAL_PWM_SetDutyCycle(config->control_details.pwm.pwm_channel, brightness);
    //     default:
    //         return E_NOK;
    // }
    return E_NOK;
}

static Status_t LightCtrl_ReadFeedback(const LightCtrl_Config_t *config, uint8_t *actual_brightness)
{
    // uint32_t raw_data = 0;
    // Status_t status = E_NOK;

    // switch (config->feedback_type) {
    //     case LightCtrl_FEEDBACK_TYPE_CURRENT_SENSOR:
    //     case LightCtrl_FEEDBACK_TYPE_LIGHT_SENSOR:
    //         status = MCAL_ADC_Read(config->feedback_details.analog_adc.adc_channel, &raw_data);
    //         if (status == E_OK) {
    //             status = LightCtrl_ConvertAnalogToBrightness(raw_data, &config->feedback_details, actual_brightness);
    //         }
    //         break;
    //     default:
    //         return E_NOK; // Should not be reached if feedback_type != NONE
    // }
    return E_NOK;
}

static Status_t LightCtrl_ConvertAnalogToBrightness(uint32_t raw_data, const LightCtrl_FeedbackDetails_t *details, uint8_t *brightness)
{
    // // This is a simplified example. A real implementation would use calibration data.
    // float voltage = (raw_data / 4096.0f) * 3.3f; // Example for 12-bit ADC, 3.3V ref

    // // Use linear conversion from voltage to brightness
    // float converted_brightness = (voltage * details->analog_adc.voltage_to_brightness_slope) + details->analog_adc.voltage_to_brightness_offset;

    // *brightness = (uint8_t)converted_brightness;

    // // Clamp the brightness to 0-100
    // if (*brightness > 100) *brightness = 100;
    // if (*brightness < 0) *brightness = 0;

    return E_OK;
}