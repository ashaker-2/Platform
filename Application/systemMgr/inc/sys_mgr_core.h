/**
 * @file sys_mgr_core.h
 * @brief System Manager Core Logic Public API
 * @version 1.1
 * @date 2025
 *
 * This header defines the public interface for the SysMgr core control
 * logic. This module is responsible for sensor data processing, the main
 * state machine, and actuator control.
 */

#ifndef SYS_MGR_CORE_H
#define SYS_MGR_CORE_H

#include "sys_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 * TYPE DEFINITIONS FOR CORE MODULE
 * ============================================================================= */

/**
 * @brief Internal sensor data structure for core processing
 */
typedef struct {
    float temp_readings[TEMPHUM_SENSOR_ID_COUNT];     /**< Temperature readings per sensor */
    float hum_readings[TEMPHUM_SENSOR_ID_COUNT];      /**< Humidity readings per sensor */
    bool temp_valid[TEMPHUM_SENSOR_ID_COUNT];         /**< Temperature reading validity flags */
    bool hum_valid[TEMPHUM_SENSOR_ID_COUNT];          /**< Humidity reading validity flags */
    bool hum_supported[TEMPHUM_SENSOR_ID_COUNT];      /**< Humidity support flags per sensor */
    uint32_t last_read_time_ms;                       /**< Last sensor reading timestamp */
} SensorData_t;

/**
 * @brief Internal actuator control state
 */
typedef struct {
    bool fans_commanded;          /**< Fans command state */
    bool heaters_commanded;       /**< Heaters command state */
    bool pumps_commanded;         /**< Pumps command state */
    bool vents_commanded;         /**< Vents command state */
    bool lights_commanded;        /**< Lights command state */
    uint32_t cycle_timers[SYS_MGR_ACTUATOR_COUNT]; /**< Cycle timers for each actuator type */
    bool cycle_states[SYS_MGR_ACTUATOR_COUNT];     /**< Current cycle state (ON/OFF) */
} ActuatorControl_t;

/**
 * @brief Core module state machine states
 */
typedef enum {
    SYS_MGR_CORE_STATE_INIT = 0,        /**< Initialization state */
    SYS_MGR_CORE_STATE_SENSOR_READ,     /**< Reading sensors */
    SYS_MGR_CORE_STATE_PROCESS_DATA,    /**< Processing sensor data */
    SYS_MGR_CORE_STATE_CONTROL_LOGIC,   /**< Executing control logic */
    SYS_MGR_CORE_STATE_ACTUATOR_UPDATE, /**< Updating actuator states */
    SYS_MGR_CORE_STATE_FAILSAFE,        /**< Fail-safe mode */
    SYS_MGR_CORE_STATE_COUNT
} SYS_MGR_CORE_State_t;

/* =============================================================================
 * PUBLIC API FUNCTIONS
 * ============================================================================= */

/**
 * @brief Initializes the SysMgr core logic module.
 *
 * This function must be called once during system initialization.
 * It sets up internal state variables and prepares the core logic.
 *
 * Implementation notes:
 * - Initialize sensor data structures
 * - Set initial state machine state
 * - Initialize actuator control states
 * - Set up any required timers
 *
 * @return Status_t E_OK on success, E_NOK on failure.
 */
Status_t SYS_MGR_CORE_Init(void);

/**
 * @brief The main periodic function for the SysMgr core logic.
 *
 * This function handles all core system logic, including sensor reading,
 * control decisions, and state transitions. It should be called at a
 * regular, fast interval by the main SysMgr component.
 *
 * Implementation notes:
 * - Implement state machine with states: SENSOR_READ -> PROCESS_DATA -> CONTROL_LOGIC -> ACTUATOR_UPDATE
 * - In SENSOR_READ: Read all sensors, handle E_NOT_SUPPORTED for humidity
 * - In PROCESS_DATA: Calculate averages, exclude failed sensors, check fail-safe conditions
 * - In CONTROL_LOGIC: Apply thresholds (per-sensor or global), manage time-based cycles
 * - In ACTUATOR_UPDATE: Send commands to actuator controllers, update LEDs
 * - Handle mode-specific logic (AUTO, HYBRID, MANUAL, FAILSAFE)
 * - If all temperature sensors fail, request mode change to MANUAL
 * - Fail-safe trigger: Temperature > SYS_MGR_FIRE_TEMP_THRESHOLD_C
 *
 * @param tick_ms The time in milliseconds since the last call.
 * @param cfg A pointer to the current system configuration.
 */
void SYS_MGR_CORE_MainFunction(const SysMgr_Config_t *cfg);

/**
 * @brief Gets the current processed sensor readings.
 *
 * This function provides access to the processed sensor data including
 * validity information and sensor counts.
 *
 * Implementation notes:
 * - Return averaged values from last processing cycle
 * - Include count of valid sensors for diagnostics
 * - Mark data as invalid if no sensors are working
 *
 * @param[out] readings Pointer to store the sensor readings structure.
 * @return Status_t E_OK on success, E_NOK if no valid readings are available.
 */
Status_t SYS_MGR_CORE_GetReadings(SYS_MGR_Readings_t *readings);

/**
 * @brief Gets the average temperature and humidity readings.
 *
 * This function provides access to the processed, system-wide average
 * sensor readings. This is a simplified interface for backwards compatibility.
 *
 * @param[out] avg_temp Pointer to store the average temperature in Celsius.
 * @param[out] avg_hum Pointer to store the average humidity in percent.
 * @return Status_t E_OK on success, E_NOK if no valid readings are available.
 */
Status_t SYS_MGR_CORE_GetAverageReadings(float *avg_temp, float *avg_hum);

/**
 * @brief Gets the current actuator states.
 *
 * Provides the current commanded states of all actuators.
 *
 * @param[out] states Pointer to store the actuator states structure.
 * @return Status_t E_OK on success, E_NOK if the pointer is null.
 */
Status_t SYS_MGR_CORE_GetActuatorStates(SYS_MGR_Actuator_States_t *states);

/**
 * @brief Forces a specific mode change request.
 *
 * This function can be called by the core logic to request a mode change
 * (e.g., when all sensors fail and system should switch to manual mode).
 * The actual mode change will be handled by the main SysMgr module.
 *
 * Implementation note: This should set a flag that the main SysMgr checks.
 *
 * @param requested_mode The mode to request
 * @return Status_t E_OK on success, E_NOK if mode is invalid
 */
Status_t SYS_MGR_CORE_RequestModeChange(SYS_MGR_Mode_t requested_mode);

/**
 * @brief Checks if a mode change has been requested by core logic.
 *
 * The main SysMgr module should call this to check for mode change requests
 * from the core logic (e.g., sensor failures).
 *
 * @param[out] requested_mode Pointer to store the requested mode
 * @return Status_t E_OK if mode change requested, E_NOK if no request pending
 */
Status_t SYS_MGR_CORE_GetModeChangeRequest(SYS_MGR_Mode_t *requested_mode);

/**
 * @brief Clears any pending mode change request.
 *
 * Should be called by main SysMgr after processing a mode change request.
 *
 * @return Status_t E_OK on success
 */
Status_t SYS_MGR_CORE_ClearModeChangeRequest(void);

/**
 * @brief Gets the current core state machine state.
 *
 * Useful for debugging and diagnostics.
 *
 * @param[out] state Pointer to store the current core state
 * @return Status_t E_OK on success, E_NOK if pointer is null
 */
Status_t SYS_MGR_CORE_GetState(SYS_MGR_CORE_State_t *state);


SysClock_Time_t SysMgr_GetCurrentTime(void);

#ifdef __cplusplus
}
#endif

#endif /* SYS_MGR_CORE_H */