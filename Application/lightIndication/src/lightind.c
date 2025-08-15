#include "lightind.h"
#include "lightind_cfg.h"
#include "logger.h"
#include "system_monitor.h"
// #include "Rte.h"
#include "hal_gpio.h"
// #include "FreeRTOS.h"
// #include "semphr.h"
#include <string.h>

/**
 * @file lightind.c
 * @brief Implementation for the LightIndication (LightInd) component.
 *
 * This file contains the core logic for controlling light indicators, including
 * handling ON/OFF and blinking states.
 */

// --- Internal State Variables ---
typedef struct {
    LightInd_State_t commanded_state;
    uint32_t on_time_ms;
    uint32_t off_time_ms;
    uint32_t timer_ms;
    bool is_on;
} LightInd_ControlState_t;

static LightInd_ControlState_t s_indicator_states[LightInd_COUNT];
// static SemaphoreHandle_t s_lightind_mutex;
static bool s_is_initialized = false;

// --- Private Helper Function Prototypes ---
static Status_t LightInd_ApplyGpioState(uint32_t gpio_pin, bool is_on);

// --- Public Function Implementations ---

Status_t LightInd_Init(void) 
{
    // if (s_is_initialized) {
    //     return E_OK;
    // }
    
    // s_lightind_mutex = xSemaphoreCreateMutex();
    // if (s_lightind_mutex == NULL) {
    //     LOGF("LightInd: Failed to create state mutex.");
    //     RTE_Service_SystemMonitor_ReportFault(FAULT_ID_LIGHT_IND_INIT_FAILED, SEVERITY_CRITICAL, 0);
    //     return E_NOK;
    // }

    // xSemaphoreTake(s_lightind_mutex, portMAX_DELAY);
    
    // memset(s_indicator_states, 0, sizeof(s_indicator_states));
    
    // for (uint32_t i = 0; i < LightInd_COUNT; i++) 
    // {
    //     const LightInd_Config_t* config = &light_ind_configs[i];
        
    //     if (MCAL_GPIO_Init(config->gpio_pin, GPIO_MODE_OUTPUT) != E_OK) {
    //         LOGF("LightInd: GPIO init failed for indicator ID %lu.", config->id);
    //         RTE_Service_SystemMonitor_ReportFault(FAULT_ID_LIGHT_IND_INIT_FAILED, SEVERITY_CRITICAL, config->id);
    //         xSemaphoreGive(s_lightind_mutex);
    //         return E_NOK;
    //     }
        
    //     // Ensure all indicators start in a safe state (OFF)
    //     LightInd_ApplyGpioState(config->gpio_pin, false);
    // }
    
    // xSemaphoreGive(s_lightind_mutex);
    // s_is_initialized = true;
    // LOGI("LightInd: Module initialized successfully.");
    return E_OK;
}

Status_t LightInd_On(uint32_t indicatorId) 
{
    // if (!s_is_initialized || indicatorId >= LightInd_COUNT) {
    //     return E_NOK;
    // }
    
    // xSemaphoreTake(s_lightind_mutex, portMAX_DELAY);
    // s_indicator_states[indicatorId].commanded_state = LIGHTIND_STATE_ON;
    // s_indicator_states[indicatorId].is_on = true; // For immediate update
    // LightInd_ApplyGpioState(light_ind_configs[indicatorId].gpio_pin, true);
    // xSemaphoreGive(s_lightind_mutex);
    
    return E_OK;
}

Status_t LightInd_Off(uint32_t indicatorId) 
{
    // if (!s_is_initialized || indicatorId >= LightInd_COUNT) {
    //     return E_NOK;
    // }
    
    // xSemaphoreTake(s_lightind_mutex, portMAX_DELAY);
    // s_indicator_states[indicatorId].commanded_state = LIGHTIND_STATE_OFF;
    // s_indicator_states[indicatorId].is_on = false; // For immediate update
    // LightInd_ApplyGpioState(light_ind_configs[indicatorId].gpio_pin, false);
    // xSemaphoreGive(s_lightind_mutex);
    
    return E_OK;
}

Status_t LightInd_Blink(uint32_t indicatorId, uint32_t on_time_ms, uint32_t off_time_ms) 
{
    // if (!s_is_initialized || indicatorId >= LightInd_COUNT || on_time_ms == 0 || off_time_ms == 0) {
    //     return E_NOK;
    // }

    // xSemaphoreTake(s_lightind_mutex, portMAX_DELAY);
    // s_indicator_states[indicatorId].commanded_state = LIGHTIND_STATE_BLINK;
    // s_indicator_states[indicatorId].on_time_ms = on_time_ms;
    // s_indicator_states[indicatorId].off_time_ms = off_time_ms;
    // s_indicator_states[indicatorId].timer_ms = 0; // Reset timer
    // s_indicator_states[indicatorId].is_on = true; // Start in ON state
    // LightInd_ApplyGpioState(light_ind_configs[indicatorId].gpio_pin, true);
    // xSemaphoreGive(s_lightind_mutex);

    return E_OK;
}

void LightInd_MainFunction(void) 
{
    // if (!s_is_initialized) {
    //     return;
    // }
    
    // xSemaphoreTake(s_lightind_mutex, portMAX_DELAY);
    
    // for (uint32_t i = 0; i < LightInd_COUNT; i++) {
    //     if (s_indicator_states[i].commanded_state == LIGHTIND_STATE_BLINK) {
    //         s_indicator_states[i].timer_ms += LIGHTIND_CONTROL_PERIOD_MS;
            
    //         if (s_indicator_states[i].is_on) {
    //             if (s_indicator_states[i].timer_ms >= s_indicator_states[i].on_time_ms) {
    //                 s_indicator_states[i].is_on = false;
    //                 LightInd_ApplyGpioState(light_ind_configs[i].gpio_pin, false);
    //                 s_indicator_states[i].timer_ms = 0;
    //             }
    //         } else { // is OFF
    //             if (s_indicator_states[i].timer_ms >= s_indicator_states[i].off_time_ms) {
    //                 s_indicator_states[i].is_on = true;
    //                 LightInd_ApplyGpioState(light_ind_configs[i].gpio_pin, true);
    //                 s_indicator_states[i].timer_ms = 0;
    //             }
    //         }
    //     }
    // }
    
    // xSemaphoreGive(s_lightind_mutex);
}

// --- Private Helper Function Implementations ---

static Status_t LightInd_ApplyGpioState(uint32_t gpio_pin, bool is_on) 
{
    return E_OK;
    // return MCAL_GPIO_WritePin(gpio_pin, is_on ? GPIO_STATE_HIGH : GPIO_STATE_LOW);
}