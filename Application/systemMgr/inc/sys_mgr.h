// app/inc/sys_mgr.h

#ifndef SYS_MGR_H
#define SYS_MGR_H

#include <stdint.h>
#include "app_common.h" // For APP_OK/APP_ERROR

/**
 * @brief Initializes the System Manager component.
 * This function will now create and launch the internal control tasks.
 * @return APP_OK if initialized successfully, APP_ERROR otherwise.
 */
uint8_t SYS_MGR_Init(void);

// The SYS_MGR_Run() function is removed from the public API as it's now split into internal tasks.

/**
 * @brief Sets the desired operational temperature range.
 * @param min_temp_c Minimum desired temperature in Celsius.
 * @param max_temp_c Maximum desired temperature in Celsius.
 * @return APP_OK if successful, APP_ERROR if parameters are invalid (e.g., min > max).
 */
uint8_t SYS_MGR_SetOperationalTemperature(float min_temp_c, float max_temp_c);

/**
 * @brief Sets the desired operational humidity range.
 * Note: Only max_humidity is used for control (pump).
 * @param min_humidity_p Minimum desired humidity in percentage.
 * @param max_humidity_p Maximum desired humidity in percentage.
 * @return APP_OK if successful, APP_ERROR if parameters are invalid (e.g., min > max).
 */
uint8_t SYS_MGR_SetOperationalHumidity(float min_humidity_p, float max_humidity_p);

/**
 * @brief Sets the schedule for the ventilator.
 * The ventilator will be ON between on_hour:on_minute and off_hour:off_minute
 * based on system uptime (simulated 24-hour cycle).
 * @param on_hour Hour (0-23) to turn ventilator ON.
 * @param on_minute Minute (0-59) to turn ventilator ON.
 * @param off_hour Hour (0-23) to turn ventilator OFF.
 * @param off_minute Minute (0-59) to turn ventilator OFF.
 * @return APP_OK if successful, APP_ERROR if parameters are invalid.
 */
uint8_t SYS_MGR_SetVentilatorSchedule(uint8_t on_hour, uint8_t on_minute,
                                      uint8_t off_hour, uint8_t off_minute);

/**
 * @brief Sets the schedule for the main room light.
 * The light will be ON between on_hour:on_minute and off_hour:off_minute
 * based on system uptime (simulated 24-hour cycle).
 * @param on_hour Hour (0-23) to turn light ON.
 * @param on_minute Minute (0-59) to turn light ON.
 * @param off_hour Hour (0-23) to turn light OFF.
 * @param off_minute Minute (0-59) to turn light OFF.
 * @return APP_OK if successful, APP_ERROR if parameters are invalid.
 */
uint8_t SYS_MGR_SetLightSchedule(uint8_t on_hour, uint8_t on_minute,
                                 uint8_t off_hour, uint8_t off_minute);

/**
 * @brief Gets the current simulated time (hour and minute based on system uptime).
 * @param hour Pointer to store the current simulated hour (0-23).
 * @param minute Pointer to store the current simulated minute (0-59).
 */
void SYS_MGR_GetSimulatedTime(uint32_t *hour, uint32_t *minute);


#endif /* SYS_MGR_H */
