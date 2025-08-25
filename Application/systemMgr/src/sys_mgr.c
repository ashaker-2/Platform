\
/* sys_mgr.c - simplified SysMgr with single global thresholds applied to all sensors & averages */

#include "sys_mgr.h"
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
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "Rte.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

#define FLASH_CFG_ID  1

static SysMgr_Config_t g_cfg;
static float g_ema_temp[TEMPHUM_SENSOR_ID_COUNT];
static float g_ema_hum[TEMPHUM_SENSOR_ID_COUNT];
static bool  g_ema_valid[TEMPHUM_SENSOR_ID_COUNT];

static float g_avg_temp;
static float g_avg_hum;
static bool  g_avg_valid;
static bool  g_critical;

static SemaphoreHandle_t g_mtx;
static bool g_initialized = false;
static bool g_save_pending = false;

/* forward */
static void load_defaults(void);
static void compute_any_set_enabled(void);
static void update_ema_and_averages(void);
static void apply_hysteresis_control(void);
static void apply_fail_safe(void);
static bool is_hum_supported(TempHum_Sensor_ID_t id);

Status_t SYS_MGR_Init(void)
{
    if (g_initialized) return E_OK;
    g_mtx = xSemaphoreCreateMutex();
    if (!g_mtx) return E_NOK;

    memset(&g_cfg, 0, sizeof(g_cfg));
    load_defaults();

    for (int i=0;i<TEMPHUM_SENSOR_ID_COUNT;i++)
    { 
        g_ema_valid[i]=false; 
        g_ema_temp[i]=NAN; 
        g_ema_hum[i]=NAN;
    }

    /* try load from flash (FlashMgr_LoadConfig) */
    size_t sz = sizeof(SysMgr_Config_t);
    if (FlashMgr_LoadConfig(FLASH_CFG_ID, &g_cfg, &sz) != E_OK) {
        /* keep defaults and mark save pending */
        g_save_pending = true;
    }
    compute_any_set_enabled();

    /* ensure LEDs off initially */
    LedCtrl_SetState(LED_ID_1, LED_STATE_OFF);
    LedCtrl_SetState(LED_ID_2, LED_STATE_OFF);
    LedCtrl_SetState(LED_ID_3, LED_STATE_OFF);
    LedCtrl_SetState(LED_ID_4, LED_STATE_OFF);
    LedCtrl_SetState(LED_ID_5, LED_STATE_OFF);

    g_initialized = true;
    return E_OK;
}

static void load_defaults(void)
{
    g_cfg.global.temp_min_c = SYS_MGR_DEFAULT_TEMP_MIN_C;
    g_cfg.global.temp_max_c = SYS_MGR_DEFAULT_TEMP_MAX_C;
    g_cfg.global.temp_hyst_c = SYS_MGR_DEFAULT_TEMP_HYST_C;
    g_cfg.global.hum_min_p = SYS_MGR_DEFAULT_HUM_MIN_P;
    g_cfg.global.hum_max_p = SYS_MGR_DEFAULT_HUM_MAX_P;
    g_cfg.global.hum_hyst_p = SYS_MGR_DEFAULT_HUM_HYST_P;
    g_cfg.mode = SYS_MGR_MODE_AUTOMATIC;
    g_cfg.ema_alpha_temp = SYS_MGR_DEFAULT_EMA_ALPHA_T;
    g_cfg.ema_alpha_hum  = SYS_MGR_DEFAULT_EMA_ALPHA_H;
    g_cfg.any_set_enabled = false;
    g_cfg.light_schedule_enabled = false;
}

/* compute whether any set is enabled */
static void compute_any_set_enabled(void)
{
    g_cfg.any_set_enabled = false;
    for (int i=0;i<TEMPHUM_SENSOR_ID_COUNT;i++){
        if (g_cfg.sets[i].enabled) { g_cfg.any_set_enabled = true; break; }
    }
}

/* determine humidity supported */
static bool is_hum_supported(TempHum_Sensor_ID_t id)
{
    if (id == TEMPHUM_SENSOR_ID_NTC_1 || id == TEMPHUM_SENSOR_ID_NTC_2) return false;
    return true;
}

/* update EMA and averages */
static void update_ema_and_averages(void)
{
    float sum_t=0.0f; int cnt_t=0;
    float sum_h=0.0f; int cnt_h=0;
    for (int i=0;i<TEMPHUM_SENSOR_ID_COUNT;i++){
        float tmp;
        if (TempHumCtrl_GetTemperature((TempHum_Sensor_ID_t)i, &tmp) == E_OK){
            float a = g_cfg.ema_alpha_temp;
            if (!g_ema_valid[i]) { g_ema_temp[i]=tmp; g_ema_valid[i]=true; }
            else g_ema_temp[i] = a*tmp + (1.0f-a)*g_ema_temp[i];
            sum_t += g_ema_temp[i]; cnt_t++;
            /* critical fire detection */
            if (g_ema_temp[i] >= SYS_MGR_FIRE_TEMP_THRESHOLD_C) { g_critical = true; SysMon_ReportFaultStatus(FAULT_ID_SYS_MGR_FIRE_ALARM, SYS_MON_FAULT_ACTIVE); }
        }
        float hum;
        if (is_hum_supported((TempHum_Sensor_ID_t)i) && TempHumCtrl_GetHumidity((TempHum_Sensor_ID_t)i, &hum) == E_OK){
            float a = g_cfg.ema_alpha_hum;
            if (!g_ema_valid[i]) { g_ema_hum[i]=hum; g_ema_valid[i]=true; }
            else g_ema_hum[i] = a*hum + (1.0f-a)*g_ema_hum[i];
            sum_h += g_ema_hum[i]; cnt_h++;
        }
    }
    g_avg_valid = (cnt_t>0);
    g_avg_temp = (cnt_t>0)?(sum_t/cnt_t):NAN;
    g_avg_hum  = (cnt_h>0)?(sum_h/cnt_h):NAN;
}

/* helper hysteresis decisions */
static bool decision_above_max(float val, float max, float hyst, bool last_on)
{
    if (last_on) return (val > (max - hyst));
    return (val > max);
}
static bool decision_below_min(float val, float min, float hyst, bool last_on)
{
    if (last_on) return (val < (min + hyst));
    return (val < min);
}

/* main automatic control: when sets enabled use mapping per set (global thresholds),
   else use averages applied to all actuators.
*/
static void apply_hysteresis_control(void)
{
    /* per-set control */
    if (g_cfg.any_set_enabled){
        for (int i=0;i<TEMPHUM_SENSOR_ID_COUNT;i++){
            if (!g_cfg.sets[i].enabled) continue;
            SysMgr_Set_t *s = &g_cfg.sets[i];
            /* temp control */
            float t = g_ema_temp[s->temp_sensor_id];
            /* fan */
            Fan_State_t fstate; FanCtrl_GetState(s->fan_id, &fstate);
            bool fan_last = (fstate==FAN_STATE_ON);
            bool fan_should = decision_above_max(t, g_cfg.global.temp_max_c, g_cfg.global.temp_hyst_c, fan_last);
            FanCtrl_SetState(s->fan_id, fan_should?FAN_STATE_ON:FAN_STATE_OFF);
            /* heater */
            Heater_State_t hstate; HeaterCtrl_GetState(s->heater_id, &hstate);
            bool heater_last = (hstate==HEATER_STATE_ON);
            bool heater_should = decision_below_min(t, g_cfg.global.temp_min_c, g_cfg.global.temp_hyst_c, heater_last);
            HeaterCtrl_SetState(s->heater_id, heater_should?HEATER_STATE_ON:HEATER_STATE_OFF);
            /* humidity -> ven/pump if supported */
            if (is_hum_supported(s->hum_sensor_id)){
                float h = g_ema_hum[s->hum_sensor_id];
                Ven_State_t vstate; VenCtrl_GetState(s->ven_id, &vstate);
                bool ven_last = (vstate==VEN_STATE_ON);
                bool ven_should = decision_above_max(h, g_cfg.global.hum_max_p, g_cfg.global.hum_hyst_p, ven_last);
                VenCtrl_SetState(s->ven_id, ven_should?VEN_STATE_ON:VEN_STATE_OFF);
                Pump_State_t pstate; PumpCtrl_GetState(s->pump_id, &pstate);
                bool pump_last = (pstate==PUMP_STATE_ON);
                bool pump_should = decision_below_min(h, g_cfg.global.hum_min_p, g_cfg.global.hum_hyst_p, pump_last);
                PumpCtrl_SetState(s->pump_id, pump_should?PUMP_STATE_ON:PUMP_STATE_OFF);
            }
        }
        /* unmapped actuators follow averages */
    }

    /* apply averages to any unmapped actuators */
    /* Fans */
    for (int fid=1; fid < FAN_ID_COUNT; fid++){
        bool mapped=false;
        for (int s=0;s<TEMPHUM_SENSOR_ID_COUNT;s++){
            if (g_cfg.sets[s].enabled && g_cfg.sets[s].fan_id == (Fan_ID_t)fid) { mapped=true; break; }
        }
        Fan_State_t last; FanCtrl_GetState((Fan_ID_t)fid, &last);
        bool was_on = (last==FAN_STATE_ON);
        bool should = decision_above_max(g_avg_temp, g_cfg.global.temp_max_c, g_cfg.global.temp_hyst_c, was_on);
        if (!mapped) FanCtrl_SetState((Fan_ID_t)fid, should?FAN_STATE_ON:FAN_STATE_OFF);
    }

    /* Heater (single) */
    bool mapped_heater=false;
    for (int s=0;s<TEMPHUM_SENSOR_ID_COUNT;s++){
        if (g_cfg.sets[s].enabled && g_cfg.sets[s].heater_id==HEATER_ID_1){ mapped_heater=true; break; }
    }
    Heater_State_t hlast; HeaterCtrl_GetState(HEATER_ID_1, &hlast);
    bool hwas = (hlast==HEATER_STATE_ON);
    bool hshould = decision_below_min(g_avg_temp, g_cfg.global.temp_min_c, g_cfg.global.temp_hyst_c, hwas);
    if (!mapped_heater) HeaterCtrl_SetState(HEATER_ID_1, hshould?HEATER_STATE_ON:HEATER_STATE_OFF);

    /* Vens */
    for (int vid=0; vid < VEN_ID_COUNT; vid++){
        bool mapped=false;
        for (int s=0;s<TEMPHUM_SENSOR_ID_COUNT;s++){
            if (g_cfg.sets[s].enabled && g_cfg.sets[s].ven_id == (Ven_ID_t)vid) { mapped=true; break; }
        }
        Ven_State_t last; VenCtrl_GetState((Ven_ID_t)vid, &last);
        bool was_on = (last==VEN_STATE_ON);
        bool should = decision_above_max(g_avg_hum, g_cfg.global.hum_max_p, g_cfg.global.hum_hyst_p, was_on);
        if (!mapped) VenCtrl_SetState((Ven_ID_t)vid, should?VEN_STATE_ON:VEN_STATE_OFF);
    }

    /* Pump */
    bool pump_mapped=false;
    for (int s=0;s<TEMPHUM_SENSOR_ID_COUNT;s++){
        if (g_cfg.sets[s].enabled && g_cfg.sets[s].pump_id==PUMP_ID_1) { pump_mapped=true; break; }
    }
    Pump_State_t plast; PumpCtrl_GetState(PUMP_ID_1,&plast);
    bool pwas = (plast==PUMP_STATE_ON);
    bool pshould = decision_below_min(g_avg_hum, g_cfg.global.hum_min_p, g_cfg.global.hum_hyst_p, pwas);
    if (!pump_mapped) PumpCtrl_SetState(PUMP_ID_1, pshould?PUMP_STATE_ON:PUMP_STATE_OFF);
}

/* fail-safe behavior */
static void apply_fail_safe(void)
{
    for (int fid=1; fid<FAN_ID_COUNT; fid++) FanCtrl_SetState((Fan_ID_t)fid, FAN_STATE_ON);
    for (int v=0; v<VEN_ID_COUNT; v++) VenCtrl_SetState((Ven_ID_t)v, VEN_STATE_ON);
    PumpCtrl_SetState(PUMP_ID_1, PUMP_STATE_ON);
    HeaterCtrl_SetState(HEATER_ID_1, HEATER_STATE_OFF);
    LightCtrl_SetState(LIGHT_ID_1, LIGHT_STATE_ON);
    SysMon_ReportFaultStatus(FAULT_ID_SYS_MGR_FIRE_ALARM, SYS_MON_FAULT_ACTIVE);
}

/* main periodic function */
void SYS_MGR_MainFunction(uint32_t tick_ms)
{
    if (!g_initialized) return;
    xSemaphoreTake(g_mtx, portMAX_DELAY);

    update_ema_and_averages();

    /* critical check */
    if (g_avg_valid && g_avg_temp >= SYS_MGR_FIRE_TEMP_THRESHOLD_C) g_critical=true;

    if (g_critical || g_cfg.mode == SYS_MGR_MODE_FAIL_SAFE) {
        apply_fail_safe();
        xSemaphoreGive(g_mtx);
        return;
    }

    switch (g_cfg.mode) {
        case SYS_MGR_MODE_AUTOMATIC:
            apply_hysteresis_control();
            break;
        case SYS_MGR_MODE_HYBRID:
        case SYS_MGR_MODE_MANUAL:
            /* both fallback to hysteresis control for now */
            apply_hysteresis_control();
            break;
        default:
            break;
    }

    /* update LEDs from actuator snapshot */
    SYS_MGR_ActuatorState_t st;
    memset(&st,0,sizeof(st));
    for (int fid=1; fid<FAN_ID_COUNT; fid++){
        Fan_State_t fs; if (FanCtrl_GetState((Fan_ID_t)fid,&fs)==E_OK && fs==FAN_STATE_ON) st.fan_any_on=true;
    }
    for (int v=0; v<VEN_ID_COUNT; v++){
        Ven_State_t vs; if (VenCtrl_GetState((Ven_ID_t)v,&vs)==E_OK && vs==VEN_STATE_ON) st.ven_any_on=true;
    }
    Heater_State_t hs; if (HeaterCtrl_GetState(HEATER_ID_1,&hs)==E_OK && hs==HEATER_STATE_ON) st.heater_on=true;
    Pump_State_t ps; if (PumpCtrl_GetState(PUMP_ID_1,&ps)==E_OK && ps==PUMP_STATE_ON) st.pump_on=true;
    LedCtrl_SetState(LED_ID_1, st.fan_any_on?LED_STATE_ON:LED_STATE_OFF);
    LedCtrl_SetState(LED_ID_2, st.ven_any_on?LED_STATE_ON:LED_STATE_OFF);
    LedCtrl_SetState(LED_ID_3, st.heater_on?LED_STATE_ON:LED_STATE_OFF);
    LedCtrl_SetState(LED_ID_4, st.pump_on?LED_STATE_ON:LED_STATE_OFF);
    LedCtrl_SetState(LED_ID_5, st.light_on?LED_STATE_ON:LED_STATE_OFF);

    xSemaphoreGive(g_mtx);

    /* save pending config */
    if (g_save_pending) {
        size_t sz = sizeof(SysMgr_Config_t);
        FlashMgr_SaveConfig(FLASH_CFG_ID, &g_cfg, sz);
        g_save_pending = false;
    }
}

/* Config accessors */
Status_t SYS_MGR_GetConfig(SysMgr_Config_t *out)
{
    if (!out) return E_NOK;
    xSemaphoreTake(g_mtx, portMAX_DELAY);
    *out = g_cfg;
    xSemaphoreGive(g_mtx);
    return E_OK;
}

Status_t SYS_MGR_UpdateConfigRuntime(const SysMgr_Config_t *in)
{
    if (!in) return E_NOK;
    xSemaphoreTake(g_mtx, portMAX_DELAY);
    g_cfg = *in;
    compute_any_set_enabled();
    g_save_pending = true;
    xSemaphoreGive(g_mtx);
    return E_OK;
}

Status_t SYS_MGR_SaveConfigToFlash(void)
{
    size_t sz = sizeof(SysMgr_Config_t);
    if (FlashMgr_SaveConfig(FLASH_CFG_ID, &g_cfg, sz) == E_OK) return E_OK;
    return E_NOK;
}

Status_t SYS_MGR_GetActuatorStates(SYS_MGR_ActuatorState_t *out)
{
    if (!out) return E_NOK;
    memset(out,0,sizeof(*out));
    for (int fid=1; fid<FAN_ID_COUNT; fid++){
        Fan_State_t fs; if (FanCtrl_GetState((Fan_ID_t)fid,&fs)==E_OK && fs==FAN_STATE_ON) out->fan_any_on=true;
    }
    for (int v=0; v<VEN_ID_COUNT; v++){ Ven_State_t vs; if (VenCtrl_GetState((Ven_ID_t)v,&vs)==E_OK && vs==VEN_STATE_ON) out->ven_any_on=true;}
    Heater_State_t hs; if (HeaterCtrl_GetState(HEATER_ID_1,&hs)==E_OK && hs==HEATER_STATE_ON) out->heater_on=true;
    Pump_State_t ps; if (PumpCtrl_GetState(PUMP_ID_1,&ps)==E_OK && ps==PUMP_STATE_ON) out->pump_on=true;
    Led_State_t ls; if (LightCtrl_GetState(LIGHT_ID_1,&ls)==E_OK && ls==LED_STATE_ON) out->light_on=true;
    return E_OK;
}

Status_t SYS_MGR_GetCriticalAlarmStatus(bool *active)
{
    if (!active) return E_NOK;
    *active = g_critical;
    return E_OK;
}
