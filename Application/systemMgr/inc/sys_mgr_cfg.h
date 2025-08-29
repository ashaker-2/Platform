/**
 * @file sys_mgr_cfg.h
 * @brief Default System Configuration Definitions
 * @version 1.1
 * @date 2025
 *
 * This header file contains constants and the default configuration structure
 * used by the System Manager. It provides a single point of truth for
 * initial values and can be modified to change system behavior at compile time.
 */

#ifndef SYS_MGR_CFG_H
#define SYS_MGR_CFG_H

#include "sys_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * DEFAULT VALUES
 * ============================================================================= */

/** @defgroup SysMgr_Default_Values Default Configuration Values
 * @{
 */

/** Default minimum temperature threshold in degrees Celsius */
#define SYS_MGR_DEFAULT_TEMP_MIN_C    20.0f
/** Default maximum temperature threshold in degrees Celsius */
#define SYS_MGR_DEFAULT_TEMP_MAX_C    25.0f

/** Default minimum humidity threshold in percent */
#define SYS_MGR_DEFAULT_HUM_MIN_P     40.0f
/** Default maximum humidity threshold in percent */
#define SYS_MGR_DEFAULT_HUM_MAX_P     60.0f

/** Default actuator cycle ON time in seconds */
#define SYS_MGR_DEFAULT_CYCLE_ON_SEC  60
/** Default actuator cycle OFF time in seconds */
#define SYS_MGR_DEFAULT_CYCLE_OFF_SEC 30

/** Default light schedule ON time (21:00) */
#define SYS_MGR_DEFAULT_LIGHT_ON_HOUR  21
#define SYS_MGR_DEFAULT_LIGHT_ON_MIN   0
/** Default light schedule OFF time (06:00) */
#define SYS_MGR_DEFAULT_LIGHT_OFF_HOUR 6
#define SYS_MGR_DEFAULT_LIGHT_OFF_MIN  0

/** Configuration version for current structure */
#define SYS_MGR_CONFIG_VERSION        0x0001

/** @} */

/* =============================================================================
 * VALIDATION CONSTANTS
 * ============================================================================= */

/** @defgroup SysMgr_Validation_Limits Configuration Validation Limits
 * @{
 */

/** Minimum allowed temperature threshold */
#define SYS_MGR_TEMP_MIN_LIMIT_C     -40.0f
/** Maximum allowed temperature threshold */
#define SYS_MGR_TEMP_MAX_LIMIT_C      85.0f

/** Minimum allowed humidity threshold */
#define SYS_MGR_HUM_MIN_LIMIT_P       0.0f
/** Maximum allowed humidity threshold */
#define SYS_MGR_HUM_MAX_LIMIT_P     100.0f

/** Minimum actuator cycle time in seconds */
#define SYS_MGR_CYCLE_MIN_SEC         1
/** Maximum actuator cycle time in seconds */
#define SYS_MGR_CYCLE_MAX_SEC      3600

/** @} */

/* =============================================================================
 * PUBLIC VARIABLES
 * ============================================================================= */

/**
 * @brief The default system configuration.
 *
 * This global variable holds the factory-default settings for the system.
 * It is used for initialization when no configuration is available in flash.
 * 
 * Implementation note: Define this in sys_mgr_cfg.c
 */
extern const SysMgr_Config_t g_default_system_configuration;

/* =============================================================================
 * PUBLIC API FUNCTIONS
 * ============================================================================= */

/**
 * @brief Validates a given configuration.
 *
 * This function is used to check if a configuration is valid before
 * it is applied or saved. It is provided as a weak symbol, allowing
 * it to be overridden with a project-specific implementation if needed.
 *
 * Implementation notes:
 * - Check all temperature and humidity ranges against limits
 * - Validate mode is within valid enum range
 * - Check actuator cycle times are reasonable (1s - 1hour)
 * - Validate light schedule times (0-23 hours, 0-59 minutes)
 * - Ensure min < max for all threshold pairs
 *
 * @param[in] cfg A pointer to the configuration to be validated.
 * @return Status_t E_OK if the configuration is valid, E_NOK otherwise.
 */
Status_t SYS_MGR_ValidateConfig(const SysMgr_Config_t *cfg);

/**
 * @brief Gets the default configuration.
 *
 * Helper function to get a copy of the default configuration.
 * Useful for UI reset operations or initialization.
 *
 * @param[out] cfg Pointer to configuration structure to be filled with defaults.
 * @return Status_t E_OK on success, E_NOK if cfg is null.
 */
Status_t SYS_MGR_GetDefaultConfig(SysMgr_Config_t *cfg);

/**
 * @brief Validates individual parameter ranges.
 *
 * Helper functions for parameter validation. These can be used by
 * the UI layer for real-time input validation.
 */

/**
 * @brief Validates temperature range.
 * @param temp_min Minimum temperature threshold
 * @param temp_max Maximum temperature threshold
 * @return Status_t E_OK if valid range, E_NOK otherwise
 */
Status_t SYS_MGR_ValidateTemperatureRange(float temp_min, float temp_max);

/**
 * @brief Validates humidity range.
 * @param hum_min Minimum humidity threshold
 * @param hum_max Maximum humidity threshold
 * @return Status_t E_OK if valid range, E_NOK otherwise
 */
Status_t SYS_MGR_ValidateHumidityRange(float hum_min, float hum_max);

/**
 * @brief Validates actuator cycle timing.
 * @param on_time_sec ON duration in seconds
 * @param off_time_sec OFF duration in seconds
 * @return Status_t E_OK if valid timing, E_NOK otherwise
 */
Status_t SYS_MGR_ValidateActuatorCycle(uint32_t on_time_sec, uint32_t off_time_sec);

/**
 * @brief Validates light schedule times.
 * @param on_hour ON hour (0-23)
 * @param on_min ON minute (0-59)
 * @param off_hour OFF hour (0-23)
 * @param off_min OFF minute (0-59)
 * @return Status_t E_OK if valid schedule, E_NOK otherwise
 */
Status_t SYS_MGR_ValidateLightSchedule(uint8_t on_hour, uint8_t on_min, 
                                       uint8_t off_hour, uint8_t off_min);

#ifdef __cplusplus
}
#endif

#endif /* SYS_MGR_CFG_H */