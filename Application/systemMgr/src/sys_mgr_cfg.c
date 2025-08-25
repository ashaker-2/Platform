#include "sys_mgr_cfg.h"

/* Default initial user config (persisted copy is read on boot if present) */
SysMgr_UserConfig_t g_default_user_config = {
    .global_temp_min = SYS_MGR_DEFAULT_TEMP_MIN_C,
    .global_temp_max = SYS_MGR_DEFAULT_TEMP_MAX_C,
    .global_hum_min  = SYS_MGR_DEFAULT_HUM_MIN_P,
    .global_hum_max  = SYS_MGR_DEFAULT_HUM_MAX_P,
    .mode = SYS_MGR_MODE_AUTOMATIC,
    .sets_enabled = false,
    /* per_sensor defaults: all not configured */
    .per_sensor = { [0 ... (TEMPHUM_SENSOR_ID_COUNT-1)] = { .temp_configured = false, .hum_configured = false } },
    /* cycles default: disabled */
    .fans_cycle = { .on_time_sec = 0, .off_time_sec = 0, .enabled = false },
    .heaters_cycle = { .on_time_sec = 0, .off_time_sec = 0, .enabled = false },
    .pumps_cycle = { .on_time_sec = 0, .off_time_sec = 0, .enabled = false },
    .vents_cycle = { .on_time_sec = 0, .off_time_sec = 0, .enabled = false },
    .light_schedule = { .on_hour = 21, .on_min = 0, .off_hour = 6, .off_min = 0, .enabled = false },
    .fan_manual_in_hybrid = false,
    .heater_manual_in_hybrid = false,
    .pump_manual_in_hybrid = false,
    .vent_manual_in_hybrid = false,
    .light_manual_in_hybrid = false
};

/* Validate function default (weak) - platform may override or implement stronger checks */
Status_t SysMgr_ValidateUserConfig(const SysMgr_UserConfig_t *cfg)
{
    if (!cfg) return E_NOK;
    if (cfg->global_temp_min < 0.0f || cfg->global_temp_max > 99.0f || cfg->global_temp_min > cfg->global_temp_max) return E_NOK;
    if (cfg->global_hum_min < 0.0f || cfg->global_hum_max > 99.0f || cfg->global_hum_min > cfg->global_hum_max) return E_NOK;
    return E_OK;
}

/* Storage hooks left unimplemented here; platform should provide implementations. */
