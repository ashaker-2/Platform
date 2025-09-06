/**
 * @file sys_mgr.h
 * @brief System Manager Core Public API and Type Definitions
 * @version 2.1
 * @date 2025
 *
 * This file defines the public interface and all core type definitions
 * for the System Manager component. It serves as the primary header for
 * other modules to interact with the system's control logic and configuration.
 */

#ifndef SYS_MGR_H
#define SYS_MGR_H

#include <stdbool.h>
#include <stdint.h>
#include <float.h>
#include "common.h"
#include "temphumctrl_cfg.h"
#include "fanctrl_cfg.h"
#include "venctrl_cfg.h"
#include "pumpctrl_cfg.h"
#include "heaterctrl_cfg.h"
#include "lightctrl_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * CONSTANTS AND LIMITS
 * ============================================================================= */

/** @defgroup SysMgr_Constants System Manager Constants
 * @{
 */

/** Maximum number of temperature/humidity sensors */
#define TEMPHUM_SENSOR_ID_COUNT         TEMPHUM_SENSOR_ID_COUNT

/** Default temperature hysteresis value in degrees Celsius */
#define SYS_MGR_DEFAULT_TEMP_HYST_C     1.0f

/** Default humidity hysteresis value in percent relative humidity */
#define SYS_MGR_DEFAULT_HUM_HYST_P      3.0f

/** Default EMA alpha for temperature */
#define SYS_MGR_DEFAULT_EMA_ALPHA_T     0.2f

/** Default EMA alpha for humidity */
#define SYS_MGR_DEFAULT_EMA_ALPHA_H     0.2f

/** Critical temperature threshold in degrees Celsius */
#define SYS_MGR_FIRE_TEMP_THRESHOLD_C   60.0f

/** Configuration timeout in milliseconds (auto-save) */
#define SYS_MGR_CONFIG_TIMEOUT_MS       30000

/** Main function execution period in milliseconds */
#define SYS_MGR_MAIN_PERIOD_MS          100

/** @} */

/* =============================================================================
 * TYPE DEFINITIONS
 * ============================================================================= */

/**
 * @brief System operational modes.
 */
typedef enum {
    SYS_MGR_MODE_AUTOMATIC = 0, /**< Fully automatic control based on thresholds */
    SYS_MGR_MODE_HYBRID,        /**< Combines automatic control with time-based schedules */
    SYS_MGR_MODE_MANUAL,        /**< Control based only on time-based schedules */
    SYS_MGR_MODE_FAILSAFE,      /**< Safety-first mode with overrides */
    SYS_MGR_MODE_COUNT
} SYS_MGR_Mode_t;

/**
 * @brief Actuator types for configuration and control
 */
typedef enum {
    SYS_MGR_ACTUATOR_FANS = 0,
    SYS_MGR_ACTUATOR_HEATERS,
    SYS_MGR_ACTUATOR_PUMPS,
    SYS_MGR_ACTUATOR_VENTS,
    SYS_MGR_ACTUATOR_LIGHTS,
    SYS_MGR_ACTUATOR_COUNT
} SYS_MGR_Actuator_t;

/**
 * @brief Configuration for a single time-based actuator cycle.
 */
typedef struct {
    uint32_t on_time_sec;     /**< ON duration in seconds */
    uint32_t off_time_sec;    /**< OFF duration in seconds */
    bool enabled;             /**< Enable/disable this cycle */
} Actuator_Cycle_t;

/**
 * @brief Configuration for a single per-sensor set.
 * 
 * Implementation note: Use temp_configured/hum_configured to determine
 * if per-sensor values should be used or fall back to global thresholds.
 */
typedef struct {
    bool temp_configured;     /**< True if per-sensor temp thresholds are set */
    bool hum_configured;      /**< True if per-sensor humidity thresholds are set */
    float temp_min_C;         /**< Minimum temperature threshold (°C) */
    float temp_max_C;         /**< Maximum temperature threshold (°C) */
    float hum_min_P;          /**< Minimum humidity threshold (%) */
    float hum_max_P;          /**< Maximum humidity threshold (%) */
} Per_Sensor_Config_t;

/**
 * @brief Light schedule configuration using RTC time.
 */
typedef struct {
    uint8_t on_hour;          /**< Turn ON hour (0-23) */
    uint8_t on_min;           /**< Turn ON minute (0-59) */
    uint8_t off_hour;         /**< Turn OFF hour (0-23) */
    uint8_t off_min;          /**< Turn OFF minute (0-59) */
    bool enabled;             /**< Enable/disable light schedule */
} Light_Schedule_t;

/**
 * @brief System sensor readings structure for monitoring
 */
typedef struct {
    float avg_temp_C;         /**< System average temperature (°C) */
    float avg_hum_P;          /**< System average humidity (%) */
    uint8_t valid_temp_sensors; /**< Number of functioning temperature sensors */
    uint8_t valid_hum_sensors;  /**< Number of functioning humidity sensors */
    bool sensor_data_valid;   /**< True if any sensor data is available */
} SYS_MGR_Readings_t;

/**
 * @brief Actuator states for monitoring
 */
typedef struct {
    bool fans_active;         /**< True if any fan is ON */
    bool heaters_active;      /**< True if any heater is ON */
    bool pumps_active;        /**< True if any pump is ON */
    bool vents_active;        /**< True if any ventilator is ON */
    bool lights_active;       /**< True if lights are ON */
} SYS_MGR_Actuator_States_t;

/**
 * @brief Main system configuration structure.
 *
 * This holds all parameters that can be configured by the user and persisted.
 * Implementation note: Always pass by reference to avoid stack usage issues.
 */
typedef struct {
    uint16_t version;                       /**< Configuration version for migration */
    float global_temp_min;                  /**< Global minimum temperature threshold (°C) */
    float global_temp_max;                  /**< Global maximum temperature threshold (°C) */
    float global_hum_min;                   /**< Global minimum humidity threshold (%) */
    float global_hum_max;                   /**< Global maximum humidity threshold (%) */
    SYS_MGR_Mode_t mode;                    /**< Current system operating mode */
    bool per_sensor_control_enabled;        /**< Flag to use per-sensor thresholds */
    Per_Sensor_Config_t per_sensor[TEMPHUM_SENSOR_ID_COUNT]; /**< Per-sensor threshold sets */
    Actuator_Cycle_t fans_cycle;            /**< Time-based cycle for fans */
    Actuator_Cycle_t heaters_cycle;         /**< Time-based cycle for heaters */
    Actuator_Cycle_t pumps_cycle;           /**< Time-based cycle for pumps */
    Actuator_Cycle_t vents_cycle;           /**< Time-based cycle for ventilators */
    Light_Schedule_t light_schedule;        /**< Light schedule for hybrid/manual modes */
    bool fan_manual_in_hybrid;              /**< Enable manual fan control in hybrid mode */
    bool heater_manual_in_hybrid;           /**< Enable manual heater control in hybrid mode */
    bool pump_manual_in_hybrid;             /**< Enable manual pump control in hybrid mode */
    bool vent_manual_in_hybrid;             /**< Enable manual vent control in hybrid mode */
    bool light_manual_in_hybrid;            /**< Enable manual light control in hybrid mode */
} SysMgr_Config_t;



typedef struct {
    uint8_t hour;    // 0–23
    uint8_t minute;  // 0–59
    uint8_t second;  // 0–59
} SysClock_Time_t;




/* =============================================================================
 * PUBLIC API FUNCTIONS
 * ============================================================================= */

/**
 * @brief Initializes the SysMgr module.
 *
 * This function must be called once at system startup. It creates the
 * necessary resources and loads the system configuration.
 *
 * Implementation notes:
 * - Initialize mutex/semaphores for thread safety
 * - Load configuration from flash, use defaults if invalid
 * - Initialize all controller modules
 * - Set initial system state
 *
 * @return Status_t E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_Init(void);

/**
 * @brief Gets the current system configuration.
 *
 * This function provides a thread-safe way to read the current configuration.
 *
 * @param[out] out A pointer to the SysMgr_Config_t struct to be populated.
 * @return Status_t E_OK on success, E_NOK if the pointer is null.
 */
Status_t SYS_MGR_GetConfig(SysMgr_Config_t *out);

/**
 * @brief Updates the runtime configuration.
 *
 * This function updates the in-memory configuration and marks it for a
 * subsequent save to flash.
 *
 * Implementation notes:
 * - Validate configuration before applying
 * - Use mutex protection for thread safety
 * - Mark configuration as dirty for next flash save
 *
 * @param[in] in A pointer to the new configuration.
 * @return Status_t E_OK on success, E_NOK if the pointer is null or config invalid.
 */
Status_t SYS_MGR_UpdateConfigRuntime(const SysMgr_Config_t *in);

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
 * @brief Saves the current configuration to flash.
 *
 * This function can be called on demand to save the configuration
 * to persistent storage.
 *
 * @return Status_t E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_SaveConfigToFlash(void);

/**
 * @brief The main periodic function for the SysMgr.
 *
 * This function handles all core system logic, including sensor reading,
 * control decisions, and state transitions. It should be called at a
 * regular, fast interval (recommended: every 100ms).
 *
 * Implementation notes:
 * - Read all sensors, handle E_NOT_SUPPORTED for humidity
 * - Calculate system averages (exclude failed sensors)
 * - Execute control logic based on current mode
 * - Handle state transitions and fail-safe conditions
 * - Update actuator states
 * - Execution time should be < 10ms
 *
 * @param void.
 */
void SYS_MGR_MainFunction();

/**
 * @brief Gets the current system operational mode.
 *
 * @param[out] out Pointer to store the current mode.
 * @return Status_t E_OK on success, E_NOK if the pointer is null.
 */
Status_t SYS_MGR_GetMode(SYS_MGR_Mode_t *out);

/**
 * @brief Gets the current actuator states.
 *
 * Provides information about which actuators are currently active.
 *
 * @param[out] states Pointer to store the actuator states structure.
 * @return Status_t E_OK on success, E_NOK if the pointer is null.
 */
Status_t SYS_MGR_GetActuatorStates(SYS_MGR_Actuator_States_t *states);

/**
 * @brief Checks if sensor supports humidity readings.
 *
 * This function can be used by the UI to determine if humidity configuration
 * options should be enabled for a specific sensor.
 *
 * Implementation note: Call TempHumCtrl_GetHumidity() and check for E_NOT_SUPPORTED.
 *
 * @param[in] sensor_id The sensor ID to check.
 * @param[out] supports_humidity True if sensor supports humidity, false otherwise.
 * @return Status_t E_OK on success, E_NOK if sensor_id is invalid.
 */
Status_t SYS_MGR_GetSensorHumiditySupport(uint8_t sensor_id, bool *supports_humidity);

#ifdef __cplusplus
}
#endif

#endif /* SYS_MGR_H */