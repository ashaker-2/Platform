/**
 * @file sys_mgr_core.c
 * @brief System Manager Core Control Logic
 * @version 1.1
 * @date 2025
 *
 * This file contains the core control logic for the SysMgr system,
 * including the state machine for managing operational modes, sensor
 * data processing, and actuator control. This is a new module created
 * as part of the refactoring effort to separate the core logic from
 * the main SysMgr interface.
 */

#include "sys_mgr_core.h"
#include "sys_mgr_cfg.h"
#include "temphumctrl.h"
#include "fanctrl.h"
#include "heaterctrl.h"
#include "pumpctrl.h"
#include "venctrl.h"
#include "lightctrl.h"
#include "ledctrl.h"
#include "system_monitor.h"
#include "logger.h"
#include "temphumctrl_cfg.h"
#include "logger.h" 
#include <string.h>
#include <math.h>
#include <time.h>   // if RTC or system time is available


/* =============================================================================
 * PRIVATE GLOBAL VARIABLES
 * ============================================================================= */

static float g_ema_temperature_data[TEMPHUM_SENSOR_ID_COUNT];
static float g_ema_humidity_data[TEMPHUM_SENSOR_ID_COUNT];
static bool g_ema_valid[TEMPHUM_SENSOR_ID_COUNT];
static float g_average_temperature;
static float g_average_humidity;
static bool g_average_valid;
static bool g_is_critical_condition;
static uint32_t g_fan_cycle_timer_ms = 0;
static bool g_fan_is_on = false;
static uint32_t g_core_tick_acc_ms = 0;



static const char *TAG = "SysMgr_Core";
/* =============================================================================
 * PRIVATE FUNCTION PROTOTYPES
 * ============================================================================= */
static void update_sensor_data_averages(void);
static void check_critical_conditions(void);
static void apply_temperature_and_humidity_control(const SysMgr_Config_t *cfg);
static void apply_light_and_cycle_control(const SysMgr_Config_t *cfg);
static void enter_failsafe_mode(void);

/* =============================================================================
 * PUBLIC FUNCTIONS
 * ============================================================================= */

/**
 * @brief The main periodic function for the SysMgr core logic.
 *
 * This function is called by the `sys_mgr.c` component at a regular
 * interval. It updates sensor data and executes the appropriate control
 * logic based on the current system mode.
 *
 * @param tick_ms The time in milliseconds since the last call.
 * @param cfg The current system configuration.
 */
void SYS_MGR_CORE_MainFunction(const SysMgr_Config_t *cfg)
{
    g_core_tick_acc_ms += SYS_MGR_MAIN_PERIOD_MS; /* increment by constant tick */

    update_sensor_data_averages();
    check_critical_conditions();

    if (g_is_critical_condition) {
        enter_failsafe_mode();
        return;
    }

    switch (cfg->mode) {
        case SYS_MGR_MODE_AUTOMATIC:
            apply_temperature_and_humidity_control(cfg);
            break;
        case SYS_MGR_MODE_HYBRID:
            apply_temperature_and_humidity_control(cfg);
            apply_light_and_cycle_control(cfg);
            break;
        case SYS_MGR_MODE_MANUAL:
            apply_light_and_cycle_control(cfg);
            break;
        default:
            LOGE(TAG,"Invalid system mode: %d", cfg->mode);
            enter_failsafe_mode();
            break;
    }
}

/**
 * @brief Gets the average temperature and humidity readings.
 *
 * Provides the system-wide average sensor readings.
 *
 * @param[out] avg_temp Pointer to store the average temperature in Celsius.
 * @param[out] avg_hum Pointer to store the average humidity in percent.
 * @return Status_t E_OK on success, E_NOK if no valid readings are available.
 */
Status_t SYS_MGR_CORE_GetAverageReadings(float *avg_temp, float *avg_hum)
{
    if (!avg_temp || !avg_hum) return E_NOK;
    if (!g_average_valid) return E_NOK;

    *avg_temp = g_average_temperature;
    *avg_hum = g_average_humidity;

    return E_OK;
}

/* =============================================================================
 * PRIVATE FUNCTIONS
 * ============================================================================= */

/**
 * @brief Updates EMA and average values for all temperature/humidity sensors.
 */
static void update_sensor_data_averages(void)
{
    uint8_t valid_temp_count = 0;
    uint8_t valid_hum_count = 0;
    float rdg = 0;
    Status_t status = E_OK;
    for (int i = 0; i < TEMPHUM_SENSOR_ID_COUNT; i++) 
    {    
        status = TempHumCtrl_GetAverageTemperature((TempHum_Sensor_ID_t)i, &rdg);
        if (status == E_OK) 
        {
            g_ema_temperature_data[i] = rdg;
            valid_temp_count++;
        }
        status = TempHumCtrl_GetAverageHumidity((TempHum_Sensor_ID_t)i, &rdg);
        if (status == E_OK) 
        {
            g_ema_humidity_data[i] = rdg;
            valid_hum_count++;
        }
    }

    g_average_valid = (valid_temp_count > 0 || valid_hum_count > 0);
    TempHumCtrl_GetSystemAverageTemperature(&g_average_temperature);
    TempHumCtrl_GetSystemAverageHumidity(&g_average_humidity);
}

/**
 * @brief Checks for critical system conditions, e.g., fire alarm.
 */
static void check_critical_conditions(void)
{
    // Check for fire/critical temperature
    g_is_critical_condition = (g_average_temperature >= SYS_MGR_FIRE_TEMP_THRESHOLD_C);
}

/**
 * @brief Applies temperature and humidity control based on current configuration.
 *
 * This function implements the hysteresis control logic for fans, heaters,
 * pumps, and ventilators. It uses either per-sensor thresholds or global
 * averages based on the configuration.
 *
 * @param cfg The current system configuration.
 */
static void apply_temperature_and_humidity_control(const SysMgr_Config_t *cfg)
{
    if (cfg->per_sensor_control_enabled) {
        for (int i = 0; i < TEMPHUM_SENSOR_ID_COUNT; i++) {
            if (cfg->per_sensor[i].temp_configured) {
                float temp_reading = g_ema_temperature_data[i];
                float min_temp = cfg->per_sensor[i].temp_min_C;
                float max_temp = cfg->per_sensor[i].temp_max_C;
                float hyst = SYS_MGR_DEFAULT_TEMP_HYST_C;

                if (temp_reading >= (max_temp + hyst)) {
                    HeaterCtrl_SetState((Heater_ID_t)i, HEATER_STATE_OFF);
                    FanCtrl_SetState((Fan_ID_t)i, FAN_STATE_ON);
                } else if (temp_reading <= (min_temp - hyst)) {
                    HeaterCtrl_SetState((Heater_ID_t)i, HEATER_STATE_ON);
                    FanCtrl_SetState((Fan_ID_t)i, FAN_STATE_OFF);
                }
            }
            if (cfg->per_sensor[i].hum_configured) {
                float hum_reading = g_ema_humidity_data[i];
                float min_hum = cfg->per_sensor[i].hum_min_P;
                float max_hum = cfg->per_sensor[i].hum_max_P;
                float hyst = SYS_MGR_DEFAULT_HUM_HYST_P;

                if (hum_reading >= (max_hum + hyst)) {
                    VenCtrl_SetState((Ven_ID_t)i, VEN_STATE_ON);
                    PumpCtrl_SetState((Pump_ID_t)i, PUMP_STATE_OFF);
                } else if (hum_reading <= (min_hum - hyst)) {
                    VenCtrl_SetState((Ven_ID_t)i, VEN_STATE_OFF);
                    PumpCtrl_SetState((Pump_ID_t)i, PUMP_STATE_ON);
                }
            }
        }
    } else {
        if (g_average_valid) {
            float min_temp = cfg->global_temp_min;
            float max_temp = cfg->global_temp_max;
            float min_hum = cfg->global_hum_min;
            float max_hum = cfg->global_hum_max;
            float temp_hyst = SYS_MGR_DEFAULT_TEMP_HYST_C;
            float hum_hyst = SYS_MGR_DEFAULT_HUM_HYST_P;

            if (g_average_temperature >= (max_temp + temp_hyst)) 
            {
                HeaterCtrl_SetState(HEATER_ID_COUNT, HEATER_STATE_OFF);
                FanCtrl_SetState(FAN_ID_COUNT, FAN_STATE_ON);
            } 
            else if (g_average_temperature <= (min_temp - temp_hyst)) 
            {
                HeaterCtrl_SetState(HEATER_ID_COUNT, HEATER_STATE_ON);
                FanCtrl_SetState(FAN_ID_COUNT, FAN_STATE_OFF);
            }

            if (g_average_humidity >= (max_hum + hum_hyst)) 
            {
                VenCtrl_SetState(VEN_ID_COUNT, VEN_STATE_ON);
                PumpCtrl_SetState(PUMP_ID_COUNT, PUMP_STATE_OFF);
            } 
            else if (g_average_humidity <= (min_hum - hum_hyst)) 
            {
                VenCtrl_SetState(VEN_ID_COUNT, VEN_STATE_OFF);
                PumpCtrl_SetState(PUMP_ID_COUNT, PUMP_STATE_ON);
            }
        }
    }
}

/**
 * @brief Applies light and time-based cycle control.
 *
 * This function handles the time-based control of actuators,
 * used in Hybrid and Manual modes.
 *
 * @param cfg The current system configuration.
 * @param tick_ms The time in milliseconds since the last call.
 */
static void apply_light_and_cycle_control(const SysMgr_Config_t *cfg)
{
    
    if (cfg->fans_cycle.enabled) 
    {
        uint32_t on_ms = cfg->fans_cycle.on_time_sec * 1000U;
        uint32_t off_ms = cfg->fans_cycle.off_time_sec * 1000U;
        g_fan_cycle_timer_ms += SYS_MGR_MAIN_PERIOD_MS;

        if (g_fan_is_on) 
        {
            if (g_fan_cycle_timer_ms >= on_ms) {
                FanCtrl_SetState(FAN_ID_ALL, FAN_STATE_OFF); /* define FAN_ID_ALL sentinel in fanctrl_cfg.h */
                g_fan_is_on = false;
                g_fan_cycle_timer_ms = 0;
            }
        } 
        else 
        {
            if (g_fan_cycle_timer_ms >= off_ms) 
            {
                FanCtrl_SetState(FAN_ID_COUNT, FAN_STATE_ON);
                g_fan_is_on = true;
                g_fan_cycle_timer_ms = 0;
            }
        }
    }

    if (cfg->light_schedule.enabled) 
    {
        Status_t status = E_OK;
        SysClock_Time_t current= SysMgr_GetCurrentTime();
        if (status == E_OK) 
        {
            uint32_t now_s = current.hour * 3600U + current.minute * 60U + current.second;
            uint32_t on_s = cfg->light_schedule.on_hour * 3600U + cfg->light_schedule.on_min * 60U;
            uint32_t off_s = cfg->light_schedule.off_hour * 3600U + cfg->light_schedule.off_min * 60U;
            bool is_on_period = false;
            if (on_s < off_s) 
            {
                is_on_period = (now_s >= on_s && now_s < off_s);
            } 
            else 
            { /* crosses midnight */
                is_on_period = (now_s >= on_s || now_s < off_s);
            }

            if (is_on_period)
            { 
                for(uint8_t i =0; i < Light_ID_ALL ; i++)
                {
                    LightCtrl_SetState( (Light_ID_t)i, LIGHT_STATE_ON);
                }
            }
            else
            { 
                for(uint8_t i =0; i < Light_ID_ALL ; i++)
                {
                    LightCtrl_SetState( (Light_ID_t)i, LIGHT_STATE_OFF);
                }                
            }
        }
}


}

/**
 * @brief Enters the Fail-Safe operational mode.
 *
 * This function sets all actuators to a safe state and activates
 * the alarm LED, overriding all other control logic.
 */
static void enter_failsafe_mode(void)
{
    HeaterCtrl_SetState(HEATER_ID_COUNT, HEATER_STATE_OFF);
    LightCtrl_SetState(LIGHT_ID_COUNT, LIGHT_STATE_OFF);
    VenCtrl_SetState(VEN_ID_COUNT, VEN_STATE_ON);
    FanCtrl_SetState(FAN_ID_COUNT, FAN_STATE_ON);
    PumpCtrl_SetState(PUMP_ID_COUNT, PUMP_STATE_OFF);
    LedCtrl_SetState(LED_ID_COUNT, LED_STATE_ON);
    
    /* report and request mode change */
    // SysMon_ReportFaultStatus(SYS_MON_FAULT_CRITICAL_TEMP, SYS_MON_FAULT_STATUS_ACTIVE);
    // SYS_MGR_CORE_RequestModeChange(SYS_MGR_MODE_FAILSAFE);
}


SysClock_Time_t SysMgr_GetCurrentTime(void)
{
    SysClock_Time_t t;

    // RTC_TimeTypeDef rtc_time;
    // HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);

    // t.hour   = rtc_time.Hours;
    // t.minute = rtc_time.Minutes;
    // t.second = rtc_time.Seconds;

    t.hour   = 0;
    t.minute = 0;
    t.second = 0;

    return t;
}

