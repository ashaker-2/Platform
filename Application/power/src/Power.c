#include "power.h"
#include "power_cfg.h"
#include "logger.h"
#include "Rte.h"
#include "hal_adc.h"
#include "hal_gpio.h"
#include "system_monitor.h"
#include <string.h>

/**
 * @file power.c
 * @brief Implementation for the PowerMgr (Power Management) component.
 *
 * This file contains the core logic for managing power modes and monitoring
 * consumption, as detailed in the PowerMgr Detailed Design Document.
 */

// --- Internal State Variables ---
static Power_Mode_t s_current_power_mode = POWER_MODE_OFF; // Default to OFF until Init
static Power_Consumption_t s_last_consumption = {0};
static bool s_is_initialized = false;

// --- Private Helper Function Prototypes ---
static APP_Status_t power_transition_to_on(void);
static APP_Status_t power_transition_to_sleep(void);
static APP_Status_t power_transition_to_off(void);

// --- Public Function Implementations ---

APP_Status_t Power_Init(void) {
    if (s_is_initialized) {
        return APP_OK;
    }

    s_current_power_mode = POWER_MODE_OFF;
    memset(&s_last_consumption, 0, sizeof(Power_Consumption_t));

    // Initialize the main power rail enable GPIO
    if (MCAL_GPIO_Init(POWER_MAIN_RAIL_ENABLE_GPIO_PIN, GPIO_MODE_OUTPUT) != APP_OK) {
        LOGF("PowerMgr: Main rail GPIO init failed.");
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_POWER_INIT_FAILURE, SEVERITY_CRITICAL, 0);
        return APP_ERROR;
    }

    // Initialize ADC channels for voltage and current sensing
    if (MCAL_ADC_Init(POWER_VOLTAGE_ADC_UNIT) != APP_OK || MCAL_ADC_Init(POWER_CURRENT_ADC_UNIT) != APP_OK) {
        LOGF("PowerMgr: ADC unit init failed.");
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_POWER_INIT_FAILURE, SEVERITY_CRITICAL, 1);
        return APP_ERROR;
    }

    if (MCAL_ADC_ConfigChannel(POWER_VOLTAGE_ADC_UNIT, POWER_VOLTAGE_ADC_CHANNEL) != APP_OK ||
        MCAL_ADC_ConfigChannel(POWER_CURRENT_ADC_UNIT, POWER_CURRENT_ADC_CHANNEL) != APP_OK) {
        LOGF("PowerMgr: ADC channel config failed.");
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_POWER_INIT_FAILURE, SEVERITY_CRITICAL, 2);
        return APP_ERROR;
    }

    // Set initial power mode to ON
    if (power_transition_to_on() != APP_OK) {
        LOGF("PowerMgr: Initial transition to ON mode failed.");
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_POWER_INIT_FAILURE, SEVERITY_CRITICAL, 3);
        return APP_ERROR;
    }

    s_is_initialized = true;
    LOGI("PowerMgr: Module initialized successfully. Current mode: ON");
    return APP_OK;
}

APP_Status_t PowerMgr_SetMode(Power_Mode_t mode) {
    if (!s_is_initialized) {
        return APP_ERROR;
    }

    if (mode >= POWER_MODE_COUNT) {
        LOGE("PowerMgr: Invalid power mode command: %u", mode);
        return APP_ERROR;
    }

    if (mode == s_current_power_mode) {
        return APP_OK; // Already in the requested mode
    }

    APP_Status_t status = APP_ERROR;
    switch (mode) {
        case POWER_MODE_ON:
            status = power_transition_to_on();
            break;
        case POWER_MODE_SLEEP:
            status = power_transition_to_sleep();
            break;
        case POWER_MODE_OFF:
            status = power_transition_to_off();
            break;
        default:
            status = APP_ERROR;
            break;
    }

    if (status == APP_OK) {
        s_current_power_mode = mode;
        LOGI("PowerMgr: Transitioned to mode %u", s_current_power_mode);
    } else {
        LOGE("PowerMgr: Failed to transition to mode %u", mode);
    }

    return status;
}

APP_Status_t PowerMgr_GetConsumption(Power_Consumption_t *consumption) {
    if (consumption == NULL || !s_is_initialized) {
        return APP_ERROR;
    }
    memcpy(consumption, &s_last_consumption, sizeof(Power_Consumption_t));
    return APP_OK;
}

void PowerMgr_MainFunction(void) {
    if (!s_is_initialized) {
        return;
    }

    // Power monitoring is only active in ON mode
    if (s_current_power_mode != POWER_MODE_ON) {
        return;
    }

    uint32_t raw_voltage, raw_current;
    APP_Status_t status = APP_OK;

    // Read raw ADC values
    if (MCAL_ADC_ReadChannel(POWER_VOLTAGE_ADC_UNIT, POWER_VOLTAGE_ADC_CHANNEL, &raw_voltage) != APP_OK) {
        LOGE("PowerMgr: Failed to read voltage ADC.");
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_POWER_ADC_READ_FAILURE, SEVERITY_MEDIUM, POWER_VOLTAGE_ADC_CHANNEL);
        status = APP_ERROR;
    }
    if (MCAL_ADC_ReadChannel(POWER_CURRENT_ADC_UNIT, POWER_CURRENT_ADC_CHANNEL, &raw_current) != APP_OK) {
        LOGE("PowerMgr: Failed to read current ADC.");
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_POWER_ADC_READ_FAILURE, SEVERITY_MEDIUM, POWER_CURRENT_ADC_CHANNEL);
        status = APP_ERROR;
    }

    if (status != APP_OK) {
        // If ADC reads failed, clear consumption data and return
        memset(&s_last_consumption, 0, sizeof(Power_Consumption_t));
        return;
    }

    // Convert to physical units
    s_last_consumption.voltage_mV = (float)raw_voltage * POWER_VOLTAGE_SCALE_FACTOR;
    s_last_consumption.current_mA = (float)raw_current * POWER_CURRENT_SCALE_FACTOR;
    s_last_consumption.power_mW = s_last_consumption.voltage_mV * s_last_consumption.current_mA / 1000.0f;

    // Report to systemMgr
    RTE_Service_SYS_MGR_UpdatePowerConsumption(s_last_consumption.current_mA,
                                             s_last_consumption.voltage_mV,
                                             s_last_consumption.power_mW);

    // Threshold Check & Fault Reporting
    if (s_last_consumption.current_mA > POWER_OVERCURRENT_THRESHOLD_MA) {
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_POWER_OVER_CURRENT, SEVERITY_HIGH, (uint32_t)s_last_consumption.current_mA);
        LOGW("PowerMgr: Overcurrent detected: %.1f mA", s_last_consumption.current_mA);
    }
    if (s_last_consumption.voltage_mV < POWER_UNDERVOLTAGE_THRESHOLD_MV) {
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_POWER_UNDER_VOLTAGE, SEVERITY_HIGH, (uint32_t)s_last_consumption.voltage_mV);
        LOGW("PowerMgr: Undervoltage detected: %.1f mV", s_last_consumption.voltage_mV);
    }
    if (s_last_consumption.power_mW > POWER_OVERPOWER_THRESHOLD_MW) {
        RTE_Service_SystemMonitor_ReportFault(FAULT_ID_POWER_OVER_POWER, SEVERITY_HIGH, (uint32_t)s_last_consumption.power_mW);
        LOGW("PowerMgr: Overpower detected: %.1f mW", s_last_consumption.power_mW);
    }

    LOGD("Power: V:%.1f mV, I:%.1f mA, P:%.1f mW",
         s_last_consumption.voltage_mV, s_last_consumption.current_mA, s_last_consumption.power_mW);
}

// --- Private Helper Function Implementations ---

static APP_Status_t power_transition_to_on(void) {
    // Enable the main power rail
    if (MCAL_GPIO_WritePin(POWER_MAIN_RAIL_ENABLE_GPIO_PIN, GPIO_STATE_HIGH) != APP_OK) {
        return APP_ERROR;
    }
    // Disable any wake-up sources
    MCAL_GPIO_Deinit(POWER_WAKEUP_BUTTON_GPIO_PIN); // Assuming a simple de-init for now
    
    // In a real system, more peripherals would be enabled here
    return APP_OK;
}

static APP_Status_t power_transition_to_sleep(void) {
    // Configure wake-up sources
    if (MCAL_GPIO_Init(POWER_WAKEUP_BUTTON_GPIO_PIN, GPIO_MODE_INPUT_IT_RISING) != APP_OK) {
        return APP_ERROR;
    }
    
    // Disable non-essential peripherals/power rails here
    // ...
    
    // Enter low-power sleep mode
    // Note: This function would typically not return until a wake-up event occurs
    // A placeholder call is used here for conceptual purposes
    // MCAL_MCU_EnterSleepMode();
    
    LOGW("PowerMgr: System entering Sleep mode.");
    return APP_OK;
}

static APP_Status_t power_transition_to_off(void) {
    // Disable all power rails
    MCAL_GPIO_WritePin(POWER_MAIN_RAIL_ENABLE_GPIO_PIN, GPIO_STATE_LOW);
    
    // In a real system, you might save state to NVM here before shutting down
    // RTE_Service_Nvm_WriteParam(...);
    
    // Call low-level MCU shutdown function (this function likely won't return)
    // MCAL_MCU_Shutdown();
    
    LOGW("PowerMgr: System entering OFF mode. Halting execution.");
    while(1) {} // The MCU is effectively off, but for simulation, we'll halt
    return APP_OK;
}