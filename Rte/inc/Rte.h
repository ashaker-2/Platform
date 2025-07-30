// app/inc/Rte.h

#ifndef RTE_H
#define RTE_H

#include <stdint.h>
#include "app_common.h"     // For APP_OK/APP_ERROR
#include "freertos/FreeRTOS.h" // For TaskFunction_t (FreeRTOS task prototype)

/**
 * @brief Master initialization function for the Runtime Environment.
 * This function is called once from app_main. It is responsible for creating
 * the initial hardware initialization task (INIT_TASKS_HwInitTask).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t RTE_Init(void);

/**
 * @brief FreeRTOS Task for reading environmental sensors.
 * Periodicity: 20ms
 * This task calls SYS_MGR_ProcessSensorReadings internally.
 * @param pvParameters Standard FreeRTOS task parameter (unused).
 */
void RTE_SensorReadTask(void *pvParameters);

/**
 * @brief FreeRTOS Task for controlling actuators (Heater, Fans, Pump, Ventilator, Lights)
 * and updating their LED indications.
 * Periodicity: 100ms
 * This task calls SYS_MGR_ControlActuators internally.
 * @param pvParameters Standard FreeRTOS task parameter (unused).
 */
void RTE_ActuatorControlTask(void *pvParameters);

/**
 * @brief FreeRTOS Task for managing display updates and fire alarm logic.
 * Periodicity: 1000ms (1 second)
 * This task calls SYS_MGR_UpdateDisplayAndAlarm internally.
 * @param pvParameters Standard FreeRTOS task parameter (unused).
 */
void RTE_DisplayAlarmTask(void *pvParameters);

/**
 * @brief FreeRTOS Task for general main loop operations (e.g., heartbeat LED, non-critical periodic checks).
 * This replaces the original `while(1)` loop content from `app_main`.
 * Periodicity: 5000ms (5 seconds)
 * @param pvParameters Standard FreeRTOS task parameter (unused).
 */
void RTE_MainLoopTask(void *pvParameters);

/**
 * @brief Starts all the permanent application tasks.
 * This function is called by INIT_TASKS_AppInitTask after all modules are initialized.
 * It contains all xTaskCreate calls for the ongoing application tasks.
 * @return APP_OK if all tasks are created successfully, APP_ERROR otherwise.
 */
uint8_t RTE_StartAllPermanentTasks(void);


#endif /* RTE_H */
