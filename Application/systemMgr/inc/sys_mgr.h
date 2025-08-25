\
/* sys_mgr.h
 *
 * System Manager public API (single global thresholds applied to sensors & averages).
 */

#ifndef SYS_MGR_H
#define SYS_MGR_H

#include <stdbool.h>
#include <stdint.h>
#include "common.h"
#include "system_monitor.h"
#include "temphumctrl_cfg.h" /* TEMPHUM_SENSOR_ID_COUNT */
#include "fanctrl_cfg.h"
#include "venctrl_cfg.h"
#include "pumpctrl_cfg.h"
#include "heaterctrl_cfg.h"
#include "ledctrl.h"
#include "flash_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SYS_MGR_MODE_AUTOMATIC = 0,
    SYS_MGR_MODE_HYBRID,
    SYS_MGR_MODE_MANUAL,
    SYS_MGR_MODE_FAIL_SAFE,
    SYS_MGR_MODE_COUNT
} SYS_MGR_Mode_t;

typedef struct {
    float temp_min_c;
    float temp_max_c;
    float temp_hyst_c;
    float hum_min_p;
    float hum_max_p;
    float hum_hyst_p;
} SysMgr_GlobalThresholds_t;

typedef struct {
    bool enabled; /* set enabled or not */
    TempHum_Sensor_ID_t temp_sensor_id;
    TempHum_Sensor_ID_t hum_sensor_id;
    Fan_ID_t fan_id;
    Heater_ID_t heater_id;
    Ven_ID_t ven_id;
    Pump_ID_t pump_id;
} SysMgr_Set_t;

typedef struct {
    SysMgr_GlobalThresholds_t global;
    SysMgr_Set_t sets[TEMPHUM_SENSOR_ID_COUNT]; /* mapping per potential set */
    bool any_set_enabled;
    SYS_MGR_Mode_t mode;
    float ema_alpha_temp;
    float ema_alpha_hum;
    /* schedules (seconds) */
    uint32_t fan_on_s;
    uint32_t fan_off_s;
    uint32_t heater_on_s;
    uint32_t heater_off_s;
    uint32_t pump_on_s;
    uint32_t pump_off_s;
    uint32_t ven_on_s;
    uint32_t ven_off_s;
    /* light schedule (hh:mm) */
    uint8_t light_on_hour;
    uint8_t light_on_min;
    uint8_t light_off_hour;
    uint8_t light_off_min;
    bool light_schedule_enabled;
} SysMgr_Config_t;

typedef struct {
    bool fan_any_on;
    bool ven_any_on;
    bool heater_on;
    bool pump_on;
    bool light_on;
} SYS_MGR_ActuatorState_t;

/* Initialize SysMgr */
Status_t SYS_MGR_Init(void);

/* Periodic main (tick_ms elapsed since last call) */
void SYS_MGR_MainFunction(uint32_t tick_ms);

/* Config access */
Status_t SYS_MGR_GetConfig(SysMgr_Config_t *out);
Status_t SYS_MGR_UpdateConfigRuntime(const SysMgr_Config_t *in);
Status_t SYS_MGR_SaveConfigToFlash(void);

/* Status access */
Status_t SYS_MGR_GetActuatorStates(SYS_MGR_ActuatorState_t *out);
Status_t SYS_MGR_GetCriticalAlarmStatus(bool *active);

#ifdef __cplusplus
}
#endif

#endif /* SYS_MGR_H */
