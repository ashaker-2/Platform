/**
 * @file sys_mgr_cfg.c
 * @brief Default System Manager Configuration
 * @version 1.0
 * @date 2025
 *
 * This file contains the default, compile-time configuration for the
 * System Manager. This configuration is used at system startup if no
 * persistent configuration is found in non-volatile memory.
 */

#include "sys_mgr_cfg.h"

/* Define default configuration that matches declaration in header */
const SysMgr_Config_t g_default_system_configuration = {
    .version = SYS_MGR_CONFIG_VERSION,
    .global_temp_min = SYS_MGR_DEFAULT_TEMP_MIN_C,
    .global_temp_max = SYS_MGR_DEFAULT_TEMP_MAX_C,
    .global_hum_min = SYS_MGR_DEFAULT_HUM_MIN_P,
    .global_hum_max = SYS_MGR_DEFAULT_HUM_MAX_P,
    .mode = SYS_MGR_MODE_AUTOMATIC,
    .per_sensor_control_enabled = false,
    .per_sensor = {{0}},
    .fans_cycle = {.enabled = false, .on_time_sec = SYS_MGR_DEFAULT_CYCLE_ON_SEC, .off_time_sec = SYS_MGR_DEFAULT_CYCLE_OFF_SEC},
    .heaters_cycle = {.enabled = false, .on_time_sec = SYS_MGR_DEFAULT_CYCLE_ON_SEC, .off_time_sec = SYS_MGR_DEFAULT_CYCLE_OFF_SEC},
    .pumps_cycle = {.enabled = false, .on_time_sec = SYS_MGR_DEFAULT_CYCLE_ON_SEC, .off_time_sec = SYS_MGR_DEFAULT_CYCLE_OFF_SEC},
    .vents_cycle = {.enabled = false, .on_time_sec = SYS_MGR_DEFAULT_CYCLE_ON_SEC, .off_time_sec = SYS_MGR_DEFAULT_CYCLE_OFF_SEC},
    .light_schedule = {.on_hour = SYS_MGR_DEFAULT_LIGHT_ON_HOUR, .on_min = SYS_MGR_DEFAULT_LIGHT_ON_MIN,
                       .off_hour = SYS_MGR_DEFAULT_LIGHT_OFF_HOUR, .off_min = SYS_MGR_DEFAULT_LIGHT_OFF_MIN,
                       .enabled = true},
    .fan_manual_in_hybrid = false,
    .heater_manual_in_hybrid = false,
    .pump_manual_in_hybrid = false,
    .vent_manual_in_hybrid = false,
    .light_manual_in_hybrid = false
};

/* Strong validation implementation */
Status_t SYS_MGR_ValidateConfig(const SysMgr_Config_t *cfg)
{
    if (!cfg) return E_NOK;

    /* basic sanity checks */
    if (cfg->global_temp_min < SYS_MGR_TEMP_MIN_LIMIT_C ||
        cfg->global_temp_max > SYS_MGR_TEMP_MAX_LIMIT_C ||
        cfg->global_temp_min >= cfg->global_temp_max) {
        return E_NOK;
    }
    if (cfg->global_hum_min < SYS_MGR_HUM_MIN_LIMIT_P ||
        cfg->global_hum_max > SYS_MGR_HUM_MAX_LIMIT_P ||
        cfg->global_hum_min >= cfg->global_hum_max) {
        return E_NOK;
    }

    /* validate per-sensor entries if configured */
    if (cfg->per_sensor_control_enabled) {
        for (int i = 0; i < TEMPHUM_SENSOR_ID_COUNT; ++i) {
            if (cfg->per_sensor[i].temp_configured) {
                if (cfg->per_sensor[i].temp_min_C < SYS_MGR_TEMP_MIN_LIMIT_C ||
                    cfg->per_sensor[i].temp_max_C > SYS_MGR_TEMP_MAX_LIMIT_C ||
                    cfg->per_sensor[i].temp_min_C >= cfg->per_sensor[i].temp_max_C) {
                    return E_NOK;
                }
            }
            if (cfg->per_sensor[i].hum_configured) {
                if (cfg->per_sensor[i].hum_min_P < SYS_MGR_HUM_MIN_LIMIT_P ||
                    cfg->per_sensor[i].hum_max_P > SYS_MGR_HUM_MAX_LIMIT_P ||
                    cfg->per_sensor[i].hum_min_P >= cfg->per_sensor[i].hum_max_P) {
                    return E_NOK;
                }
            }
        }
    }

    /* validate actuator cycles */
    Actuator_Cycle_t cycles[] = { cfg->fans_cycle, cfg->heaters_cycle, cfg->pumps_cycle, cfg->vents_cycle };
    for (size_t i = 0; i < sizeof(cycles)/sizeof(cycles[0]); ++i) {
        if (cycles[i].enabled) {
            if (cycles[i].on_time_sec < SYS_MGR_CYCLE_MIN_SEC || cycles[i].on_time_sec > SYS_MGR_CYCLE_MAX_SEC ||
                cycles[i].off_time_sec < SYS_MGR_CYCLE_MIN_SEC || cycles[i].off_time_sec > SYS_MGR_CYCLE_MAX_SEC) {
                return E_NOK;
            }
        }
    }

    /* validate light schedule */
    if (cfg->light_schedule.enabled) {
        if (cfg->light_schedule.on_hour > 23 || cfg->light_schedule.off_hour > 23 ||
            cfg->light_schedule.on_min > 59 || cfg->light_schedule.off_min > 59) {
            return E_NOK;
        }
    }

    return E_OK;
}
