// app/inc/sys_mgr_config.h

#ifndef SYS_MGR_CONFIG_H
#define SYS_MGR_CONFIG_H

#include "sys_mgr.h" // Includes SYS_MGR_Config_t definition

// Define default operational parameters if not set by user
#define SYS_MGR_DEFAULT_MIN_TEMP_C      20.0f
#define SYS_MGR_DEFAULT_MAX_TEMP_C      25.0f
#define SYS_MGR_DEFAULT_MIN_HUMIDITY_P  40.0f
#define SYS_MGR_DEFAULT_MAX_HUMIDITY_P  60.0f

// Define default ventilator schedule (e.g., ON from 8 PM to 6 AM)
#define SYS_MGR_DEFAULT_VENT_ON_HOUR    20
#define SYS_MGR_DEFAULT_VENT_ON_MINUTE  0
#define SYS_MGR_DEFAULT_VENT_OFF_HOUR   6
#define SYS_MGR_DEFAULT_VENT_OFF_MINUTE 0

// Define default light schedule (e.g., ON from 7 PM to 11 PM)
#define SYS_MGR_DEFAULT_LIGHT_ON_HOUR    19
#define SYS_MGR_DEFAULT_LIGHT_ON_MINUTE  0
#define SYS_MGR_DEFAULT_LIGHT_OFF_HOUR   23
#define SYS_MGR_DEFAULT_LIGHT_OFF_MINUTE 0

// Hysteresis for control loops to prevent rapid toggling (chattering)
#define TEMP_HEATER_HYSTERESIS_C      1.0f // If temp drops below min, heater ON. If temp rises above min + hyst, heater OFF.
#define TEMP_FAN_OFF_HYSTERESIS_C     1.0f // If temp rises above max, fans ON. If temp drops below max - hyst, fans OFF.
#define HUMIDITY_PUMP_OFF_HYSTERESIS_P 5.0f // If hum rises above max, pump ON. If hum drops below max - hyst, pump OFF.

// Delay between fan stages when temperature is high (in milliseconds)
#define FAN_STAGE_DELAY_MS 30000 // 30 seconds

// Thresholds for ventilator override (e.g., force ON if temp is 5C above max OR hum is 10% above max)
#define VENT_TEMP_OVERRIDE_OFFSET_C   5.0f
#define VENT_HUM_OVERRIDE_OFFSET_P    10.0f

// Fire Alarm Configuration
#define FIRE_ALARM_DELAY_MS (2 * 60 * 1000) // 2 minutes

#endif /* SYS_MGR_CONFIG_H */
