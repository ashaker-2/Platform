// app/inc/Rte_cfg.h
#ifndef RTE_CFG_H
#define RTE_CFG_H


#include "tempctrl.h"



// --- RTE Service Functions (for inter-component calls) ---
// These functions are the only allowed way for modules to interact with each other.


#define RTE_TempCtrl_Init()                            TempCtrl_Init()
#define RTE_TempCtrl_MainFunction()                    TempCtrl_MainFunction()
#define RTE_TempCtrl_GetSensorTemp(x,y)                TempCtrl_GetSensorTemp(x,y)




#define RTE_HumCtrl_MainFunction()                 HumCtrl_MainFunction()


#define RTE_SysMgr_MainFunction()                     SYS_MGR_MainFunction()


// Services for Sensor Readings
#define RTE_Service_TEMP_SENSOR_Read(x)
#define RTE_Service_HUMIDITY_SENSOR_Read(x)


// Services for Actuator Control

#define RTE_Service_FAN_SetSpeed(x)
#define RTE_Service_HEATER_SetState(x)
#define RTE_Service_PUMP_SetState(x)
#define RTE_Service_VENTILATOR_SetState(x)
#define RTE_Service_LIGHT_SetState(x, y)
#define RTE_Service_LIGHT_GetState(x, y)


// Services for Display and Indication
#define RTE_Service_UpdateDisplayAndAlarm(x)
#define RTE_Service_CHARACTER_DISPLAY_Clear(x)
#define RTE_Service_CHARACTER_DISPLAY_PrintString(x, y)
#define RTE_Service_CHARACTER_DISPLAY_SetCursor(a, b, c)
#define RTE_Service_CHARACTER_DISPLAY_BacklightOn(x)
#define RTE_Service_CHARACTER_DISPLAY_BacklightOff(x)
#define RTE_Service_LIGHT_INDICATION_On(x)
#define RTE_Service_LIGHT_INDICATION_Off(x)
#define RTE_Service_LIGHT_INDICATION_Toggle(x)


// Services for System Manager Data Access
#define RTE_Service_GetCurrentSensorReadings(x, y, z)
#define RTE_Service_GetOperationalTemperature(x, y)
#define RTE_Service_GetOperationalHumidity(fx, y)
#define RTE_Service_GetActuatorStates(a , b , c , d)
#define RTE_Service_GetSimulatedTime(x , y)
#define RTE_Service_SetOperationalTemperature(x , y)
#define RTE_Service_SetOperationalHumidity(x , y)
#define RTE_Service_SetVentilatorSchedule(a , b , c , d)
#define RTE_Service_SetLightSchedule(a , b , c , d)


#define RTE_Service_Modbus_WriteHoldingRegister
#define RTE_Service_Modbus_ReadHoldingRegister
#define RTE_Service_Bluetooth_SendData
#define RTE_Service_WiFi_Connect
#define RTE_Service_WiFi_Disconnect
#define RTE_Service_WiFi_IsConnected
#define RTE_Service_WiFi_SendNetworkData


#endif