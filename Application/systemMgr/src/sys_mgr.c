#include "sys_mgr.h"
#include "sys_mgr_cfg.h"
#include "common.h"
#include "logger.h"
#include "system_monitor.h"
// #include "Rte.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/semphr.h"
#include <string.h>

/**
 * @file sys_mgr.c
 * @brief Implementation for the SystemMgr (System Manager) component.
 *
 * This file contains the core logic for the central control and state management,
 * as detailed in the SystemMgr Detailed Design Document.
 */

// --- Internal State Variables ---
typedef struct
{
    SYS_MGR_Mode_t current_mode;
    bool is_fail_safe_active;
    float current_room_temp_c;
    float current_room_humidity_p;
    SYS_MGR_ActuatorState_t actuator_states;
    bool critical_alarm_active;
    // Persisted operational parameters
    SystemOperationalParams_t operational_params;
} SYS_MGR_State_t;

static SYS_MGR_State_t sys_mgr_state;
// static SemaphoreHandle_t sys_mgr_state_mutex;
static bool s_is_initialized = false;

// --- Private Helper Function Prototypes ---
static void SYS_MGR_ApplyAutomaticControl(void);
static void SYS_MGR_ApplyFailSafeControl(void);
static void SYS_MGR_UpdateSensorReadings(void);
static void SYS_MGR_UpdateDisplayAndAlarm(void);

// --- Public Function Implementations ---

Status_t SYS_MGR_Init(void)
{
    // if (s_is_initialized)
    // {
    //     return E_OK;
    // }

    // sys_mgr_state_mutex = xSemaphoreCreateMutex();
    // if (sys_mgr_state_mutex == NULL)
    // {
    //     // LOGF("SystemMgr", "Failed to create state mutex.");
    //     return E_NOK;
    // }

    // xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY);

    // // Initialize state with default values
    // memset(&sys_mgr_state, 0, sizeof(SYS_MGR_State_t));
    // sys_mgr_state.current_mode = SYS_MGR_MODE_AUTOMATIC;
    // sys_mgr_state.operational_params.operational_temp_min_c = SYS_MGR_DEFAULT_TEMP_MIN_C;
    // sys_mgr_state.operational_params.operational_temp_max_c = SYS_MGR_DEFAULT_TEMP_MAX_C;
    // sys_mgr_state.operational_params.operational_humidity_min_p = SYS_MGR_DEFAULT_HUMIDITY_MIN_P;
    // sys_mgr_state.operational_params.operational_humidity_max_p = SYS_MGR_DEFAULT_HUMIDITY_MAX_P;
    // memcpy(sys_mgr_state.operational_params.fan_stage_threshold_temp_c, SYS_MGR_FAN_STAGE_THRESHOLDS_C, sizeof(SYS_MGR_FAN_STAGE_THRESHOLDS_C));

    // Try to load operational parameters from storage
    // Status_t status = RTE_Service_STORAGE_ReadConfig(STORAGE_CONFIG_ID_SYSTEM_OPERATIONAL_PARAMS,
    //                                                      &sys_mgr_state.operational_params,
    //                                                      sizeof(SystemOperationalParams_t));

    // if (status != E_OK)
    // {
    //     LOGW("SystemMgr", "Failed to load operational parameters from storage. Using defaults.");
    //     SysMon_ReportFault(FAULT_ID_NVM_READ_FAILURE, SEVERITY_MEDIUM, 0);
    // }

    // xSemaphoreGive(sys_mgr_state_mutex);
    // s_is_initialized = true;
    // LOGI("SystemMgr", "Module initialized successfully.");
    return E_OK;
}

void SYS_MGR_MainFunction(void)
{
    // if (!s_is_initialized)
    // {
    //     return;
    // }

    // xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY);

    // SYS_MGR_UpdateSensorReadings();

    // if (sys_mgr_state.is_fail_safe_active || sys_mgr_state.critical_alarm_active)
    // {
    //     SYS_MGR_ApplyFailSafeControl();
    // }
    // else
    // {
    //     switch (sys_mgr_state.current_mode)
    //     {
    //     case SYS_MGR_MODE_AUTOMATIC:
    //         SYS_MGR_ApplyAutomaticControl();
    //         break;
    //     case SYS_MGR_MODE_MANUAL:
    //         // No automatic control logic in manual mode
    //         break;
    //     case SYS_MGR_MODE_HYBRID:
    //         // Hybrid control logic would go here
    //         break;
    //     default:
    //         break;
    //     }
    // }

    // SYS_MGR_UpdateDisplayAndAlarm();

    // xSemaphoreGive(sys_mgr_state_mutex);
}

Status_t SYS_MGR_SetOperationalTemperature(float min_temp, float max_temp)
{
    // if (!s_is_initialized)
    // {
    //     return E_NOK;
    // }
    // xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY);

    // if (min_temp > max_temp || min_temp < -50.0f || max_temp > 100.0f)
    // {
    //     LOGE("SystemMgr", "Invalid operational temperature range provided.");
    //     SysMon_ReportFault(FAULT_ID_SYS_INIT_ERROR, SEVERITY_MEDIUM, 0);
    //     xSemaphoreGive(sys_mgr_state_mutex);
    //     return E_NOK;
    // }

    // sys_mgr_state.operational_params.operational_temp_min_c = min_temp;
    // sys_mgr_state.operational_params.operational_temp_max_c = max_temp;

    // Status_t status = RTE_Service_STORAGE_WriteConfig(STORAGE_CONFIG_ID_SYSTEM_OPERATIONAL_PARAMS,
    //                                                       &sys_mgr_state.operational_params,
    //                                                       sizeof(SystemOperationalParams_t));
    // if (status != E_OK)
    // {
    //     SysMon_ReportFault(FAULT_ID_NVM_WRITE_FAILURE, SEVERITY_MEDIUM, 0);
    // }

    // xSemaphoreGive(sys_mgr_state_mutex);
    return E_NOK;
}

Status_t SYS_MGR_GetOperationalTemperature(float *min_temp, float *max_temp)
{
    // if (min_temp == NULL || max_temp == NULL || !s_is_initialized)
    // {
    //     return E_NOK;
    // }
    // xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY);
    // *min_temp = sys_mgr_state.operational_params.operational_temp_min_c;
    // *max_temp = sys_mgr_state.operational_params.operational_temp_max_c;
    // xSemaphoreGive(sys_mgr_state_mutex);
    return E_OK;
}

Status_t SYS_MGR_SetMode(SYS_MGR_Mode_t mode)
{
    // if (!s_is_initialized)
    // {
    //     return E_NOK;
    // }
    // if (mode >= SYS_MGR_MODE_COUNT)
    // {
    //     return E_NOK;
    // }

    // xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY);
    // sys_mgr_state.current_mode = mode;
    // xSemaphoreGive(sys_mgr_state_mutex);
    // LOGI("SystemMgr", "System mode set to %u", mode);
    return E_OK;
}

Status_t SYS_MGR_GetMode(SYS_MGR_Mode_t *mode)
{
    // if (mode == NULL || !s_is_initialized)
    // {
    //     return E_NOK;
    // }
    // xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY);
    // *mode = sys_mgr_state.current_mode;
    // xSemaphoreGive(sys_mgr_state_mutex);
    return E_OK;
}

Status_t SYS_MGR_SetFailSafeMode(bool enable)
{
    // if (!s_is_initialized)
    // {
    //     return E_NOK;
    // }
    // xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY);
    // sys_mgr_state.is_fail_safe_active = enable;
    // if (enable)
    // {
    //     LOGW("SystemMgr", "FAIL-SAFE mode activated by SystemMonitor.");
    //     SYS_MGR_ApplyFailSafeControl();
    // }
    // else
    // {
    //     LOGI("SystemMgr", "FAIL-SAFE mode deactivated.");
    // }
    // xSemaphoreGive(sys_mgr_state_mutex);
    return E_OK;
}

Status_t SYS_MGR_GetActuatorStates(SYS_MGR_ActuatorState_t *state)
{
    // if (state == NULL || !s_is_initialized)
    // {
    //     return E_NOK;
    // }
    // xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY);
    // memcpy(state, &sys_mgr_state.actuator_states, sizeof(SYS_MGR_ActuatorState_t));
    // xSemaphoreGive(sys_mgr_state_mutex);
    return E_OK;
}

Status_t SYS_MGR_GetCriticalAlarmStatus(bool *active)
{
    // if (active == NULL || !s_is_initialized)
    // {
    //     return E_NOK;
    // }
    // xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY);
    // *active = sys_mgr_state.critical_alarm_active;
    // xSemaphoreGive(sys_mgr_state_mutex);
    return E_OK;
}

// --- Private Helper Function Implementations ---

static void SYS_MGR_UpdateSensorReadings(void)
{
    // // Reading multiple sensors (assuming a single instance for now for simplicity)
    // RTE_Service_TEMP_SENSOR_Read(TEMP_SENSOR_ID_ROOM, &sys_mgr_state.current_room_temp_c);
    // RTE_Service_HUMIDITY_SENSOR_Read(HUMIDITY_SENSOR_ID_ROOM, &sys_mgr_state.current_room_humidity_p);

    // // Check for critical alarm condition
    // if (sys_mgr_state.current_room_temp_c >= SYS_MGR_FIRE_TEMP_THRESHOLD_C)
    // {
    //     sys_mgr_state.critical_alarm_active = true;
    //     LOGW("SystemMgr", "Fire alarm triggered! Temperature: %.2fC", sys_mgr_state.current_room_temp_c);
    //     SysMon_ReportFault(FAULT_ID_SYS_MGR_FIRE_ALARM, SEVERITY_CRITICAL, (uint32_t)(sys_mgr_state.current_room_temp_c * 100));
    // }
    // else
    // {
    //     sys_mgr_state.critical_alarm_active = false;
    // }
}

static void SYS_MGR_ApplyAutomaticControl(void)
{
    //     // Temperature Control (Fan/Heater)
    //     float temp_c = sys_mgr_state.current_room_temp_c;
    //     float min_temp = sys_mgr_state.operational_params.operational_temp_min_c;
    //     float max_temp = sys_mgr_state.operational_params.operational_temp_max_c;

    //     if (temp_c > max_temp)
    //     {
    //         // Activate fan
    //         uint8_t fan_speed = 0;
    //         if (temp_c > sys_mgr_state.operational_params.fan_stage_threshold_temp_c[2])
    //         {
    //             fan_speed = 100;
    //         }
    //         else if (temp_c > sys_mgr_state.operational_params.fan_stage_threshold_temp_c[1])
    //         {
    //             fan_speed = 75;
    //         }
    //         else if (temp_c > sys_mgr_state.operational_params.fan_stage_threshold_temp_c[0])
    //         {
    //             fan_speed = 50;
    //         }
    //         RTE_Service_FAN_SetSpeed(FAN_ID_ROOM, fan_speed);
    //         RTE_Service_HEATER_SetState(HEATER_ID_ROOM, false);
    //         sys_mgr_state.actuator_states.fan_speed_percent = fan_speed;
    //         sys_mgr_state.actuator_states.heater_is_on = false;
    //     }
    //     else if (temp_c < min_temp)
    //     {
    //         // Activate heater
    //         RTE_Service_HEATER_SetState(HEATER_ID_ROOM, true);
    //         RTE_Service_FAN_SetSpeed(FAN_ID_ROOM, 0);
    //         sys_mgr_state.actuator_states.heater_is_on = true;
    //         sys_mgr_state.actuator_states.fan_speed_percent = 0;
    //     }
    //     else
    //     {
    //         // Within range, turn off both
    //         RTE_Service_HEATER_SetState(HEATER_ID_ROOM, false);
    //         RTE_Service_FAN_SetSpeed(FAN_ID_ROOM, 0);
    //         sys_mgr_state.actuator_states.heater_is_on = false;
    //         sys_mgr_state.actuator_states.fan_speed_percent = 0;
    //     }

    //     // Humidity Control (Pump/Ventilator)
    //     float humidity_p = sys_mgr_state.current_room_humidity_p;
    //     float min_humidity = sys_mgr_state.operational_params.operational_humidity_min_p;
    //     float max_humidity = sys_mgr_state.operational_params.operational_humidity_max_p;

    //     if (humidity_p > max_humidity)
    //     {
    //         RTE_Service_PUMP_SetState(PUMP_ID_DEHUMIDIFIER, true);
    //         RTE_Service_VENTILATOR_SetState(VENTILATOR_ID_ROOM, true);
    //         sys_mgr_state.actuator_states.pump_is_on = true;
    //         sys_mgr_state.actuator_states.ventilator_is_on = true;
    //     }
    //     else if (humidity_p < min_humidity)
    //     {
    //         RTE_Service_PUMP_SetState(PUMP_ID_DEHUMIDIFIER, false);
    //         RTE_Service_VENTILATOR_SetState(VENTILATOR_ID_ROOM, false);
    //         sys_mgr_state.actuator_states.pump_is_on = false;
    //         sys_mgr_state.actuator_states.ventilator_is_on = false;
    //     }
    //     else
    //     {
    //         RTE_Service_PUMP_SetState(PUMP_ID_DEHUMIDIFIER, false);
    //         RTE_Service_VENTILATOR_SetState(VENTILATOR_ID_ROOM, false);
    //         sys_mgr_state.actuator_states.pump_is_on = false;
    //         sys_mgr_state.actuator_states.ventilator_is_on = false;
    //     }

    //     // Light Control (placeholder for automatic logic)
    //     // For now, we will just turn it off in automatic mode.
    //     RTE_Service_LIGHT_SetState(LIGHT_ID_ROOM, false);
    //     sys_mgr_state.actuator_states.light_is_on = false;
}

static void SYS_MGR_ApplyFailSafeControl(void)
{
    // LOGW("SystemMgr", "Applying fail-safe control logic.");
    // RTE_Service_HEATER_SetState(HEATER_ID_ROOM, false);
    // RTE_Service_FAN_SetSpeed(FAN_ID_ROOM, 100);
    // RTE_Service_PUMP_SetState(PUMP_ID_DEHUMIDIFIER, true);
    // RTE_Service_VENTILATOR_SetState(VENTILATOR_ID_ROOM, true);
    // RTE_Service_LIGHT_SetState(LIGHT_ID_ROOM, true);
    // RTE_Service_LIGHT_INDICATION_On(LIGHT_INDICATION_ID_CRITICAL_ALARM);

    // // Update internal state to reflect fail-safe actions
    // sys_mgr_state.actuator_states.heater_is_on = false;
    // sys_mgr_state.actuator_states.fan_speed_percent = 100;
    // sys_mgr_state.actuator_states.pump_is_on = true;
    // sys_mgr_state.actuator_states.ventilator_is_on = true;
    // sys_mgr_state.actuator_states.light_is_on = true;
}

static void SYS_MGR_UpdateDisplayAndAlarm(void)
{
    // char temp_str[16], humidity_str[16];

    // snprintf(temp_str, sizeof(temp_str), "Temp: %.1f C", sys_mgr_state.current_room_temp_c);
    // snprintf(humidity_str, sizeof(humidity_str), "Humidity: %.1f %%", sys_mgr_state.current_room_humidity_p);

    // RTE_Service_DISPLAY_UpdateLine(0, temp_str);
    // RTE_Service_DISPLAY_UpdateLine(1, humidity_str);

    // if (sys_mgr_state.critical_alarm_active)
    // {
    //     RTE_Service_LIGHT_INDICATION_On(LIGHT_INDICATION_ID_CRITICAL_ALARM);
    // }
    // else
    // {
    //     RTE_Service_LIGHT_INDICATION_Off(LIGHT_INDICATION_ID_CRITICAL_ALARM);
    // }
}