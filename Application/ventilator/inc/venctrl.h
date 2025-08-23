/* ============================================================================
 * SOURCE FILE: Application/VenCtrl/inc/Ven_ctrl.h
 * ============================================================================*/
/**
 * @file Ven_ctrl.h
 * @brief Public API for the Ven Control (VenCtrl) module.
 *
 * This header provides functions to initialize the Ven control system and
 * manage the state (ON/OFF) of individual Vens.
 */

#ifndef VEN_CTRL_H
#define VEN_CTRL_H

#include "common.h"  // For Status_t
#include "Venctrl_cfg.h"   // For Ven_ID_t, Ven_State_t

/**
 * @brief Initializes the Ven Control (VenCtrl) module.
 *
 * This function configures the necessary GPIO expander pins (via HAL_I2C)
 * or direct GPIO pins (via HAL_GPIO) according to the settings in Ven_ctrl_cfg.c
 * and sets the initial state of all configured Vens.
 *
 * @return E_OK on success, or an error code if initialization fails.
 */
Status_t VenCtrl_Init(void);

/**
 * @brief Sets the state (ON or OFF) of a specific Ven.
 *
 * @param Ven_id The unique identifier of the Ven to control.
 * @param state The desired state for the Ven (VEN_STATE_ON or VEN_STATE_OFF).
 * @return E_OK on success, E_INVALID_PARAM if `Ven_id` is invalid, or
 * an error code from the underlying HAL if pin control fails.
 */
Status_t VenCtrl_SetState(Ven_ID_t Ven_id, Ven_State_t state);

/**
 * @brief Gets the current state of a specific Ven.
 *
 * This function relies on the internally tracked state of the expander outputs
 * or direct GPIO states.
 *
 * @param Ven_id The unique identifier of the Ven to query.
 * @param state_out Pointer to a variable where the current state (VEN_STATE_ON/OFF)
 * will be stored.
 * @return E_OK on success, E_INVALID_PARAM if `Ven_id` or `state_out` is invalid.
 */
Status_t VenCtrl_GetState(Ven_ID_t Ven_id, Ven_State_t *state_out);

#endif /* VEN_CTRL_H */


Display :
HAL_CharDisplay_WriteChar
HAL_CharDisplay_WriteString
HAL_CharDisplay_SetCursor
HAL_CharDisplay_ClearDisplay
HAL_CharDisplay_Home
HAL_CharDisplay_EnableCursor

Fan:
Status_t FanCtrl_SetState(Fan_ID_t fan_id, Fan_State_t state);
Status_t FanCtrl_GetState(Fan_ID_t fan_id, Fan_State_t *state_out);

Heater
Status_t HeaterCtrl_SetState(Heater_ID_t heater_id, Heater_State_t state);
Status_t HeaterCtrl_GetState(Heater_ID_t heater_id, Heater_State_t *state_out);


LED:
Status_t LedCtrl_SetState(Led_ID_t led_id, Led_State_t state);
Status_t LedCtrl_GetState(Led_ID_t led_id, Led_State_t *state_out);

light:
Status_t LightCtrl_SetState(Led_ID_t led_id, Led_State_t state); 
Status_t LightCtrl_GetState(Led_ID_t led_id, Led_State_t *state_out); 

Pump:
Status_t PumpCtrl_SetState(Pump_ID_t pump_id, Pump_State_t state);
Status_t PumpCtrl_GetState(Pump_ID_t pump_id, Pump_State_t *state_out);

SystemMonitor:
Status_t SysMon_ReportFaultStatus(SystemMonitor_FaultId_t fault_id,SysMon_FaultStatus_t status);

Temp&Hum
Status_t TempHumCtrl_GetHumidity(TempHum_Sensor_ID_t sensor_id, float *humidity_out);
Status_t TempHumCtrl_GetTemperature(TempHum_Sensor_ID_t sensor_id, float *temperature_out);
Status_t TempHumCtrl_GetAverageTemperature(TempHum_Sensor_ID_t sensor_id, float *average_temperature_out);
Status_t TempHumCtrl_GetAverageHumidity(TempHum_Sensor_ID_t sensor_id, float *average_humidity_out);
Status_t TempHumCtrl_GetSystemAverageTemperature(float *system_average_temperature_out);
Status_t TempHumCtrl_GetSystemAverageHumidity(float *system_average_humidity_out);

Vent
Status_t VenCtrl_SetState(Ven_ID_t Ven_id, Ven_State_t state);
Status_t VenCtrl_GetState(Ven_ID_t Ven_id, Ven_State_t *state_out);












