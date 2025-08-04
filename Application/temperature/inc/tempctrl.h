#ifndef TEMPCTRL_H
#define TEMPCTRL_H

#include "app_common.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file tempctrl.h
 * @brief Public interface for the TempCtrl (Temperature Control) component.
 *
 * This header defines the public API for the TempCtrl module, which provides a high-level
 * interface for acquiring temperature data from multiple sensors.
 */

// --- Public Functions ---

/**
 * @brief Initializes the TempCtrl module and all configured temperature sensor hardware.
 *
 * All module-internal variables and sensor data storage are initialized to a safe,
 * known state (e.g., zeroes or NaN).
 *
 * @return APP_OK on success, APP_ERROR on failure.
 */
APP_Status_t TempCtrl_Init(void);

/**
 * @brief Gets the last successfully read and stored temperature value for a specific sensor.
 *
 * This is a non-blocking getter function. The actual sensor reading for all sensors is
 * performed periodically by the internal TempCtrl_MainFunction.
 *
 * @param sensorId The unique ID of the sensor to retrieve data from.
 * @param temperature_c Pointer to store the latest temperature value in Celsius.
 * @return APP_OK on successful retrieval, APP_ERROR if the sensorId is invalid,
 * the pointer is NULL, or no valid data is available for that sensor.
 */
APP_Status_t TempCtrl_GetSensorTemp(uint32_t sensorId, float *temperature_c);

// --- Internal Periodic Runnable Prototype (called by RTE) ---

/**
 * @brief Performs the periodic temperature sensor reading, validation, and internal data update
 * for all configured sensors. This function is intended to be called periodically by an RTE task.
 */
void TempCtrl_MainFunction(void);

#endif // TEMPCTRL_H