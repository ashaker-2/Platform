/**
 * @file sys_mgr_cfg.h
 * @brief Configuration structures and defaults for System Manager.
 * @version 1.0
 * @date 2025
 *
 * This file defines the configuration model for SysMgr:
 *  - Temperature/Humidity thresholds (global + per-sensor)
 *  - Actuator operating modes (AUTO / MANUAL with ON/OFF cycles)
 *  - Lighting schedule
 *  - System-wide defaults and limits
 *
 * All configuration here is runtime-editable via UI Manager and
 * can be persisted in non-volatile memory by SysMgr.
 */

#ifndef SYS_MGR_CFG_H
#define SYS_MGR_CFG_H

#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "temphum_ctrl.h"

/* ============================================================================
 * ENUMS & CONSTANTS
 * ========================================================================== */

/**
 * @brief System operation modes.
 */
typedef enum {
    SYS_MGR_MODE_AUTOMATIC = 0,  /**< Fully automatic control */
    SYS_MGR_MODE_MANUAL,         /**< Manual ON/OFF cycles */
    SYS_MGR_MODE_HYBRID,         /**< Mixed: some manual, some automatic */
    SYS_MGR_MODE_FAILSAFE        /**< Emergency safe state (future) */
} SysMgr_Mode_t;

/**
 * @brief Actuator control mode.
 */
typedef enum {
    ACTUATOR_MODE_AUTOMATIC = 0, /**< Controlled by thresholds/system logic */
    ACTUATOR_MODE_MANUAL         /**< Manual cycle (on/off periods) */
} Actuator_Mode_t;

/* ---------------- CONFIG LIMITS ---------------- */
#define SYS_TEMP_MIN_LIMIT_C     0.0f
#define SYS_TEMP_MAX_LIMIT_C     60.0f
#define SYS_HUM_MIN_LIMIT_PCT    20.0f
#define SYS_HUM_MAX_LIMIT_PCT    100.0f

#define SYS_ACTUATOR_MAX_ON_SEC  (12 * 3600)  /**< Max ON period: 12 hours */
#define SYS_ACTUATOR_MAX_OFF_SEC (12 * 3600)  /**< Max OFF period: 12 hours */

#define SYS_LIGHT_HOUR_MAX       23
#define SYS_LIGHT_MINUTE_MAX     59

/* ============================================================================
 * STRUCTS
 * ========================================================================== */

/**
 * @brief Per-sensor configuration thresholds.
 */
typedef struct {
    float temp_min_c;  /**< Minimum allowed temperature (Celsius) */
    float temp_max_c;  /**< Maximum allowed temperature (Celsius) */
    float hum_min_pct; /**< Minimum allowed humidity (%) */
    float hum_max_pct; /**< Maximum allowed humidity (%) */
} SysMgr_SensorCfg_t;

/**
 * @brief Manual cycle configuration for actuators (used in MANUAL mode).
 */
typedef struct {
    Actuator_Mode_t mode;   /**< AUTO or MANUAL */
    uint32_t on_period_s;   /**< ON period in seconds (when MANUAL) */
    uint32_t off_period_s;  /**< OFF period in seconds (when MANUAL) */
} SysMgr_ActuatorCfg_t;

/**
 * @brief Lighting schedule configuration.
 */
typedef struct {
    uint8_t on_hour;    /**< ON time (hour, 0–23) */
    uint8_t on_minute;  /**< ON time (minute, 0–59) */
    uint8_t off_hour;   /**< OFF time (hour, 0–23) */
    uint8_t off_minute; /**< OFF time (minute, 0–59) */
} SysMgr_LightCfg_t;

/**
 * @brief Complete working configuration for SysMgr.
 */
typedef struct {
    SysMgr_Mode_t mode; /**< Current system mode */

    /* Per-sensor thresholds */
    SysMgr_SensorCfg_t sensor_cfg[TEMPHUM_SENSOR_ID_COUNT];

    /* Actuator configs (arrays sized per actuator type) */
    SysMgr_ActuatorCfg_t fans[FAN_ID_COUNT];
    SysMgr_ActuatorCfg_t vents[VEN_ID_COUNT];
    SysMgr_ActuatorCfg_t pumps[PUMP_ID_COUNT];
    SysMgr_ActuatorCfg_t heaters[HEATER_ID_COUNT];
    SysMgr_ActuatorCfg_t leds[LED_ID_COUNT];

    /* Light schedule (single config for lights group) */
    SysMgr_LightCfg_t lights[LIGHT_ID_COUNT];

} SysMgr_Config_t;

/* ============================================================================
 * DEFAULTS
 * ========================================================================== */

/**
 * @brief Provides a default configuration (used at startup or reset).
 * Defined in sys_mgr_cfg.c
 */
extern const SysMgr_Config_t SYS_MGR_DEFAULT_CONFIG;

#endif /* SYS_MGR_CFG_H */
