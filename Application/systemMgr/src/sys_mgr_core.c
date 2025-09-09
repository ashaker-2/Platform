/**
 * @file sys_mgr_core.c
 * @brief System Manager Core Control Logic
 * @version 1.2
 * @date 2025
 *
 * Core logic for System Manager:
 *  - Reads sensor values (Temp/Hum)
 *  - Applies thresholds and hysteresis
 *  - Runs mode-specific control (AUTO / MANUAL / HYBRID)
 *  - Drives actuators
 *  - Provides status snapshots for UI
 */

#include "sys_mgr_core.h"
#include "temphumctrl.h"
#include "fanctrl.h"
#include "heaterctrl.h"
#include "pumpctrl.h"
#include "venctrl.h"
#include "lightctrl.h"
#include "ledctrl.h"
#include "logger.h"

#include <string.h>
#include <stdbool.h>

static const char *TAG = "SysMgr_Core";

/* ============================================================================
 * PRIVATE GLOBALS
 * ========================================================================== */

static float g_avg_temp = 0.0f;
static float g_avg_hum = 0.0f;
static bool g_avg_valid = false;

static SysMgr_ActuatorStates_t g_actuator_states;

/* Timers for manual cycle control */
static uint32_t g_cycle_timer_ms_fan = 0;
static uint32_t g_cycle_timer_ms_heater = 0;
static uint32_t g_cycle_timer_ms_pump = 0;
static uint32_t g_cycle_timer_ms_vent = 0;

static bool g_cycle_on_fan = false;
static bool g_cycle_on_heater = false;
static bool g_cycle_on_pump = false;
static bool g_cycle_on_vent = false;

/* Tick accumulator */
static uint32_t g_core_tick_acc_ms = 0;

/* ============================================================================
 * PRIVATE HELPERS
 * ========================================================================== */

static void update_averages(void);
static void apply_auto_control(const SysMgr_Config_t *cfg);
static void apply_hybrid_control(const SysMgr_Config_t *cfg);
static void apply_manual_control(const SysMgr_Config_t *cfg);
static void apply_light_schedule(const SysMgr_Config_t *cfg);
static void apply_actuator_cycles(const SysMgr_Config_t *cfg);
static void update_actuator_states(void);

/* ============================================================================
 * PUBLIC API
 * ========================================================================== */

void SYS_MGR_CORE_MainFunction(const SysMgr_Config_t *cfg)
{
    if (!cfg) return;

    g_core_tick_acc_ms += SYS_MGR_MAIN_PERIOD_MS;

    update_averages();

    switch (cfg->mode)
    {
        case SYS_MGR_MODE_AUTOMATIC:
            apply_auto_control(cfg);
            break;

        case SYS_MGR_MODE_HYBRID:
            apply_hybrid_control(cfg);
            break;

        case SYS_MGR_MODE_MANUAL:
            apply_manual_control(cfg);
            break;

        default:
            LOGE(TAG, "Invalid mode %d", cfg->mode);
            break;
    }

    update_actuator_states();
}

Status_t SYS_MGR_CORE_GetAverageReadings(float *avg_temp, float *avg_hum)
{
    if (!avg_temp || !avg_hum) return E_NOK;
    if (!g_avg_valid) return E_NOK;

    *avg_temp = g_avg_temp;
    *avg_hum = g_avg_hum;
    return E_OK;
}

Status_t SYS_MGR_CORE_GetActuatorStates(SysMgr_ActuatorStates_t *states_out)
{
    if (!states_out) return E_NOK;
    *states_out = g_actuator_states;
    return E_OK;
}

void SYS_MGR_CORE_RequestFailsafe(void)
{
    /* Placeholder for fire alarm or absolute failsafe logic */
    HeaterCtrl_SetState(HEATER_ID_COUNT, HEATER_STATE_OFF);
    FanCtrl_SetState(FAN_ID_COUNT, FAN_STATE_ON);
    VenCtrl_SetState(VEN_ID_COUNT, VEN_STATE_ON);
    PumpCtrl_SetState(PUMP_ID_COUNT, PUMP_STATE_OFF);
    LightCtrl_SetState(LIGHT_ID_COUNT, LIGHT_STATE_OFF);
    LedCtrl_SetState(LED_ID_COUNT, LED_STATE_ON);

    LOGW(TAG, "FAILSAFE activated");
}

SysClock_Time_t SysMgr_GetCurrentTime(void)
{
    SysClock_Time_t t = {0};
    /* TODO: integrate RTC/FreeRTOS time */
    return t;
}

/* ============================================================================
 * PRIVATE FUNCTIONS
 * ========================================================================== */

/**
 * @brief Update global average values from TempHumCtrl
 */
static void update_averages(void)
{
    if (TempHumCtrl_GetSystemAverageTemperature(&g_avg_temp) == E_OK &&
        TempHumCtrl_GetSystemAverageHumidity(&g_avg_hum) == E_OK)
    {
        g_avg_valid = true;
    }
    else
    {
        g_avg_valid = false;
    }
}

/**
 * @brief Automatic mode – thresholds per-sensor or global
 */
static void apply_auto_control(const SysMgr_Config_t *cfg)
{
    uint8_t count = TEMPHUM_SENSOR_ID_COUNT;

    if (cfg->per_sensor_control_enabled)
    {
        for (uint8_t i = 0; i < count; i++)
        {
            TempHum_Status_Level_t st_temp, st_hum;

            if (TempHumCtrl_GetTemperatureStatus((TempHum_Sensor_ID_t)i, &st_temp) == E_OK)
            {
                if (st_temp == TEMPHUM_STATUS_HIGH)
                {
                    FanCtrl_SetState((Fan_ID_t)i, FAN_STATE_ON);
                    HeaterCtrl_SetState((Heater_ID_t)i, HEATER_STATE_OFF);
                }
                else if (st_temp == TEMPHUM_STATUS_LOW)
                {
                    HeaterCtrl_SetState((Heater_ID_t)i, HEATER_STATE_ON);
                    FanCtrl_SetState((Fan_ID_t)i, FAN_STATE_OFF);
                }
            }

            if (TempHumCtrl_GetHumidityStatus((TempHum_Sensor_ID_t)i, &st_hum) == E_OK)
            {
                if (st_hum == TEMPHUM_STATUS_HIGH)
                {
                    VenCtrl_SetState((Ven_ID_t)i, VEN_STATE_ON);
                    PumpCtrl_SetState((Pump_ID_t)i, PUMP_STATE_OFF);
                }
                else if (st_hum == TEMPHUM_STATUS_LOW)
                {
                    PumpCtrl_SetState((Pump_ID_t)i, PUMP_STATE_ON);
                    VenCtrl_SetState((Ven_ID_t)i, VEN_STATE_OFF);
                }
            }
        }
    }
    else if (g_avg_valid)
    {
        /* Global thresholds */
        if (g_avg_temp >= cfg->global_temp_max)
        {
            FanCtrl_SetState(FAN_ID_COUNT, FAN_STATE_ON);
            HeaterCtrl_SetState(HEATER_ID_COUNT, HEATER_STATE_OFF);
        }
        else if (g_avg_temp <= cfg->global_temp_min)
        {
            HeaterCtrl_SetState(HEATER_ID_COUNT, HEATER_STATE_ON);
            FanCtrl_SetState(FAN_ID_COUNT, FAN_STATE_OFF);
        }

        if (g_avg_hum >= cfg->global_hum_max)
        {
            VenCtrl_SetState(VEN_ID_COUNT, VEN_STATE_ON);
            PumpCtrl_SetState(PUMP_ID_COUNT, PUMP_STATE_OFF);
        }
        else if (g_avg_hum <= cfg->global_hum_min)
        {
            PumpCtrl_SetState(PUMP_ID_COUNT, PUMP_STATE_ON);
            VenCtrl_SetState(VEN_ID_COUNT, VEN_STATE_OFF);
        }
    }
}

/**
 * @brief Hybrid mode – light schedule + manual cycles override
 */
static void apply_hybrid_control(const SysMgr_Config_t *cfg)
{
    apply_auto_control(cfg);
    apply_light_schedule(cfg);
    apply_actuator_cycles(cfg);
}

/**
 * @brief Manual mode – only light schedule + cycles
 */
static void apply_manual_control(const SysMgr_Config_t *cfg)
{
    apply_light_schedule(cfg);
    apply_actuator_cycles(cfg);
}

/**
 * @brief Apply light on/off schedule
 */
static void apply_light_schedule(const SysMgr_Config_t *cfg)
{
    if (!cfg->light_schedule.enabled) return;

    SysClock_Time_t now = SysMgr_GetCurrentTime();

    uint32_t now_s = now.hour * 3600U + now.minute * 60U + now.second;
    uint32_t on_s = cfg->light_schedule.on_hour * 3600U + cfg->light_schedule.on_min * 60U;
    uint32_t off_s = cfg->light_schedule.off_hour * 3600U + cfg->light_schedule.off_min * 60U;

    bool active = (on_s < off_s) ?
                  (now_s >= on_s && now_s < off_s) :
                  (now_s >= on_s || now_s < off_s);

    for (uint8_t i = 0; i < LIGHT_ID_COUNT; i++)
    {
        LightCtrl_SetState((Light_ID_t)i, active ? LIGHT_STATE_ON : LIGHT_STATE_OFF);
    }
}

/**
 * @brief Apply time-based cycles for actuators
 */
static void apply_actuator_cycles(const SysMgr_Config_t *cfg)
{
    /* Example: Fans */
    if (cfg->fans_cycle.enabled)
    {
        uint32_t on_ms = cfg->fans_cycle.on_time_sec * 1000U;
        uint32_t off_ms = cfg->fans_cycle.off_time_sec * 1000U;

        g_cycle_timer_ms_fan += SYS_MGR_MAIN_PERIOD_MS;

        if (g_cycle_on_fan)
        {
            if (g_cycle_timer_ms_fan >= on_ms)
            {
                FanCtrl_SetState(FAN_ID_COUNT, FAN_STATE_OFF);
                g_cycle_on_fan = false;
                g_cycle_timer_ms_fan = 0;
            }
        }
        else
        {
            if (g_cycle_timer_ms_fan >= off_ms)
            {
                FanCtrl_SetState(FAN_ID_COUNT, FAN_STATE_ON);
                g_cycle_on_fan = true;
                g_cycle_timer_ms_fan = 0;
            }
        }
    }

    /* Same pattern can be extended for heaters, pumps, vents if desired */
}

/**
 * @brief Snapshot actuator states into g_actuator_states
 */
static void update_actuator_states(void)
{
    memset(&g_actuator_states, 0, sizeof(g_actuator_states));

    /* For simplicity: if ANY device of type ON → mark as ON */
    for (uint8_t i = 0; i < FAN_ID_COUNT; i++)
    {
        Fan_State_t st;
        if (FanCtrl_GetState((Fan_ID_t)i, &st) == E_OK && st == FAN_STATE_ON)
        {
            g_actuator_states.fans_on = true;
        }
    }

    for (uint8_t i = 0; i < HEATER_ID_COUNT; i++)
    {
        Heater_State_t st;
        if (HeaterCtrl_GetState((Heater_ID_t)i, &st) == E_OK && st == HEATER_STATE_ON)
        {
            g_actuator_states.heaters_on = true;
        }
    }

    for (uint8_t i = 0; i < PUMP_ID_COUNT; i++)
    {
        Pump_State_t st;
        if (PumpCtrl_GetState((Pump_ID_t)i, &st) == E_OK && st == PUMP_STATE_ON)
        {
            g_actuator_states.pumps_on = true;
        }
    }

    for (uint8_t i = 0; i < VEN_ID_COUNT; i++)
    {
        Ven_State_t st;
        if (VenCtrl_GetState((Ven_ID_t)i, &st) == E_OK && st == VEN_STATE_ON)
        {
            g_actuator_states.vents_on = true;
        }
    }

    for (uint8_t i = 0; i < LIGHT_ID_COUNT; i++)
    {
        Light_State_t st;
        if (LightCtrl_GetState((Light_ID_t)i, &st) == E_OK && st == LIGHT_STATE_ON)
        {
            g_actuator_states.lights_on = true;
        }
    }
}
