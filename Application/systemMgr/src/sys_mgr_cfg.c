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

