/**
 * @file sys_mgr_cfg.c
 * @brief Default configuration for System Manager
 * @version 1.0
 * @date 2025
 *
 * This file defines the default runtime configuration for SysMgr.
 * The structure is initialized at startup and can later be modified
 * via UI or other modules. SysMgr is responsible for persisting
 * any changes to non-volatile storage (e.g., Flash).
 */

#include "sys_mgr_cfg.h"

/* ============================================================================
 * DEFAULT CONFIGURATION VALUES
 * ========================================================================== */

/* Default thresholds for all sensors */
#define DEFAULT_TEMP_MIN_C   20.0f
#define DEFAULT_TEMP_MAX_C   30.0f
#define DEFAULT_HUM_MIN_PCT  40.0f
#define DEFAULT_HUM_MAX_PCT  60.0f

/* Default actuator cycle periods (used if manual mode is selected later) */
#define DEFAULT_ACTUATOR_ON_PERIOD_S   30    /* 30 seconds */
#define DEFAULT_ACTUATOR_OFF_PERIOD_S  60    /* 60 seconds */

/* Default light schedule (OFF by default) */
#define DEFAULT_LIGHT_ON_HOUR     8
#define DEFAULT_LIGHT_ON_MINUTE   0
#define DEFAULT_LIGHT_OFF_HOUR    20
#define DEFAULT_LIGHT_OFF_MINUTE  0

/* ============================================================================
 * GLOBAL DEFAULT CONFIG INSTANCE
 * ========================================================================== */

const SysMgr_Config_t SYS_MGR_DEFAULT_CONFIG = {
    .mode = SYS_MGR_MODE_AUTOMATIC,

    /* Per-sensor thresholds */
    .sensor_cfg = {
        [0 ... (TEMPHUM_SENSOR_ID_COUNT - 1)] = {
            .temp_min_c = DEFAULT_TEMP_MIN_C,
            .temp_max_c = DEFAULT_TEMP_MAX_C,
            .hum_min_pct = DEFAULT_HUM_MIN_PCT,
            .hum_max_pct = DEFAULT_HUM_MAX_PCT,
        }
    },

    /* Fans */
    .fans = {
        [0 ... (FAN_ID_COUNT - 1)] = {
            .mode = ACTUATOR_MODE_AUTOMATIC,
            .on_period_s = DEFAULT_ACTUATOR_ON_PERIOD_S,
            .off_period_s = DEFAULT_ACTUATOR_OFF_PERIOD_S,
        }
    },

    /* Vents */
    .vents = {
        [0 ... (VEN_ID_COUNT - 1)] = {
            .mode = ACTUATOR_MODE_AUTOMATIC,
            .on_period_s = DEFAULT_ACTUATOR_ON_PERIOD_S,
            .off_period_s = DEFAULT_ACTUATOR_OFF_PERIOD_S,
        }
    },

    /* Pumps */
    .pumps = {
        [0 ... (PUMP_ID_COUNT - 1)] = {
            .mode = ACTUATOR_MODE_AUTOMATIC,
            .on_period_s = DEFAULT_ACTUATOR_ON_PERIOD_S,
            .off_period_s = DEFAULT_ACTUATOR_OFF_PERIOD_S,
        }
    },

    /* Heaters */
    .heaters = {
        [0 ... (HEATER_ID_COUNT - 1)] = {
            .mode = ACTUATOR_MODE_AUTOMATIC,
            .on_period_s = DEFAULT_ACTUATOR_ON_PERIOD_S,
            .off_period_s = DEFAULT_ACTUATOR_OFF_PERIOD_S,
        }
    },

    /* LEDs */
    .leds = {
        [0 ... (LED_ID_COUNT - 1)] = {
            .mode = ACTUATOR_MODE_AUTOMATIC,
            .on_period_s = DEFAULT_ACTUATOR_ON_PERIOD_S,
            .off_period_s = DEFAULT_ACTUATOR_OFF_PERIOD_S,
        }
    },

    /* Lights */
    .lights = {
        [0 ... (LIGHT_ID_COUNT - 1)] = {
            .on_hour = DEFAULT_LIGHT_ON_HOUR,
            .on_minute = DEFAULT_LIGHT_ON_MINUTE,
            .off_hour = DEFAULT_LIGHT_OFF_HOUR,
            .off_minute = DEFAULT_LIGHT_OFF_MINUTE,
        }
    }
};
