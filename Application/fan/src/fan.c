/**
 * @file fan_control.c
 * @brief Implementation for the Fan Control (FAN_CTL) component.
 *
 * This file contains the logic for controlling the cooling fan
 * via the underlying HAL PWM interface. It handles speed setting
 * and reports errors to the System Monitor.
 */

#include "fan_control.h"
#include "fan_control_cfg.h"
#include "logger.h"
#include "Rte.h" // For reporting faults via RTE
#include "hal_pwm.h" // For controlling the PWM channel

// --- Internal State Variables ---
static uint8_t s_fan_speed_percent = 0;
static bool s_is_initialized = false;

// --- Public Function Implementations ---

APP_Status_t FAN_CONTROL_Init(void) {
    if (s_is_initialized) {
        LOGW("FAN_CTL", "Already initialized.");
        return APP_OK;
    }

    // Initialize the PWM channel for the fan
    // Assuming HAL_PWM_Init configures the channel with default frequency/resolution
    if (HAL_PWM_Init(FAN_CONTROL_CFG_PWM_CHANNEL) != APP_OK) {
        LOGE("FAN_CTL", "Failed to initialize fan PWM channel.");
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_FAN_DRIVER_FAIL, SEVERITY_CRITICAL, "PWM Init Fail");
        return APP_ERROR;
    }

    // Ensure fan is off initially
    if (FAN_CONTROL_SetSpeed(0) != APP_OK) {
        LOGE("FAN_CTL", "Failed to set initial fan speed to 0.");
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_FAN_DRIVER_FAIL, SEVERITY_CRITICAL, "Initial Speed Fail");
        return APP_ERROR;
    }

    s_is_initialized = true;
    LOGI("FAN_CTL", "Fan Control module initialized.");
    return APP_OK;
}

APP_Status_t FAN_CONTROL_SetSpeed(uint8_t speed_percent) {
    if (!s_is_initialized) {
        LOGE("FAN_CTL", "Not initialized, cannot set fan speed.");
        return APP_NOT_INITIALIZED;
    }

    if (speed_percent > 100) {
        LOGW("FAN_CTL", "Invalid fan speed percentage: %d. Clamping to 100.", speed_percent);
        speed_percent = 100;
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_INVALID_PARAMETER, SEVERITY_LOW, "Fan Speed Out of Range");
    }

    // Convert percentage to PWM duty cycle.
    // Assuming HAL_PWM_SetDutyCycle takes a percentage directly or scales it internally.
    // If HAL_PWM expects raw duty cycle counts (e.g., 0-1023 for 10-bit PWM),
    // this conversion would be: duty_cycle = (speed_percent * MAX_PWM_DUTY_CYCLE) / 100;
    if (HAL_PWM_SetDutyCycle(FAN_CONTROL_CFG_PWM_CHANNEL, speed_percent) != APP_OK) {
        LOGE("FAN_CTL", "Failed to set fan PWM duty cycle to %d%%.", speed_percent);
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_FAN_DRIVER_FAIL, SEVERITY_HIGH, "PWM SetDutyCycle Fail");
        return APP_ERROR;
    }

    s_fan_speed_percent = speed_percent;
    LOGD("FAN_CTL", "Fan speed set to: %d%%", speed_percent);
    return APP_OK;
}

APP_Status_t FAN_CONTROL_GetSpeed(uint8_t *speed_percent_ptr) {
    if (!s_is_initialized) {
        LOGE("FAN_CTL", "Not initialized, cannot get fan speed.");
        return APP_NOT_INITIALIZED;
    }
    if (speed_percent_ptr == NULL) {
        LOGE("FAN_CTL", "NULL pointer for speed_percent_ptr.");
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_INVALID_PARAMETER, SEVERITY_LOW, "NULL ptr in GetSpeed");
        return APP_INVALID_ARG;
    }

    *speed_percent_ptr = s_fan_speed_percent;
    return APP_OK;
}
