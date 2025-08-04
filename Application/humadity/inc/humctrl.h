#ifndef HUMCTRL_H
#define HUMCTRL_H

#include "app_common.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file humctrl.h
 * @brief Public interface for the HumCtrl (Humidity Control) component.
 *
 * This header defines the public API for the HumCtrl module, which provides a
 * high-level interface for reading and managing multiple humidity sensors.
 */

// --- Public Functions ---

/**
 * @brief Initializes the HumCtrl module and all configured humidity sensor hardware.
 * All module-internal variables and sensor data storage are initialized to a safe,
 * known state (e.g., zeroes or NaN).
 * @return APP_OK on success, APP_ERROR on failure.
 */
APP_Status_t HumCtrl_Init(void);

/**
 * @brief Gets the last successfully read and stored humidity value for a specific sensor.
 * This is a non-blocking getter function. The actual sensor reading for all sensors is
 * performed periodically by the internal HumCtrl_MainFunction.
 * @param sensorId The unique ID of the sensor to retrieve data from.
 * @param humidity_rh Pointer to store the latest humidity value in Relative Humidity (%).
 * @return APP_OK on successful retrieval, APP_ERROR if the sensorId is invalid,
 * the pointer is NULL, or no valid data is available for that sensor.
 */
APP_Status_t HumCtrl_GetSensorHumidity(uint32_t sensorId, float *humidity_rh);

// --- Internal Periodic Runnable Prototype (called by RTE) ---
/**
 * @brief Performs the periodic humidity sensor reading, validation, and internal data update
 * for all configured sensors. This function is intended to be called periodically by an RTE task.
 */
void HumCtrl_MainFunction(void);

#endif // HUMCTRL_H