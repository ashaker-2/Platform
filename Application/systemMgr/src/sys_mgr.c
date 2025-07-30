// app/src/sys_mgr.c

#include "sys_mgr.h"
#include "sys_mgr_config.h"
#include "logger.h"             // For logging
#include "ecual_common.h"       // For ECUAL_GetUptimeMs()

// FreeRTOS includes for tasks and mutexes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// Application module includes
#include "temp_sensor.h"
#include "humidity_sensor.h"
#include "fan.h"
#include "heater.h"
#include "pump.h"
#include "ventilator.h"
#include "character_display.h"
#include "light_control.h"      // For scheduling lights
#include "light_indication.h"   // For new LED indications

#include <stdio.h> // For snprintf
#include <string.h> // For strlen

static const char *TAG = "SYS_MGR";

// Internal state of the System Manager
typedef struct {
    float operational_temp_min;
    float operational_temp_max;
    float operational_humidity_min;
    float operational_humidity_max;

    uint8_t vent_on_hour;
    uint8_t vent_on_minute;
    uint8_t vent_off_hour;
    uint8_t vent_off_minute;

    uint8_t light_on_hour;
    uint8_t light_on_minute;
    uint8_t light_off_hour;
    uint8_t light_off_minute;

    // Runtime measured values (updated by sensor task, read by control/display tasks)
    float current_room_temp_c;
    float current_room_humidity_p;
    float current_heatsink_temp_c;

    // Runtime actuator states (updated by control task, read by display/alarm tasks)
    uint8_t current_fan_stage; // 0: All OFF, 1: Fan1 ON, 2: Fan1+2 ON, 3: Fan1+2+3 ON
    uint64_t last_fan_stage_time_ms; // Uptime when fan stage was last changed or started
    bool heater_is_working;
    bool pump_is_working;
    bool ventilator_is_working;
    bool fan_any_active; // True if any room fan is running

    // Fire Alarm state variables
    bool temp_alarm_condition_active;
    uint64_t temp_alarm_start_time_ms;
    bool hum_alarm_condition_active;
    uint64_t hum_alarm_start_time_ms;
    bool fire_alarm_triggered;

} sys_mgr_state_t;

static sys_mgr_state_t sys_mgr_state;
static bool is_initialized = false;
static SemaphoreHandle_t sys_mgr_state_mutex; // Mutex to protect sys_mgr_state_t

// --- Internal Task Prototypes ---
static void sys_mgr_sensor_read_task(void *pvParameters);
static void sys_mgr_actuator_control_task(void *pvParameters);
static void sys_mgr_display_alarm_task(void *pvParameters);


// Helper to get current simulated time based on uptime
void SYS_MGR_GetSimulatedTime(uint32_t *hour, uint32_t *minute) {
    uint64_t uptime_ms = ECUAL_GetUptimeMs();
    // Simulate a 24-hour cycle where 1 second of real time = 1 minute of simulated time.
    // So, 60 seconds real time = 1 hour simulated time.
    // 24 * 60 seconds = 1440 seconds real time for a full simulated day.
    uint64_t simulated_minutes_total = uptime_ms / 1000; // 1 real second = 1 simulated minute
    
    *hour = (simulated_minutes_total / 60) % 24;
    *minute = simulated_minutes_total % 60;
}

static bool is_time_between(uint32_t current_hour, uint32_t current_minute,
                            uint8_t on_hour, uint8_t on_minute,
                            uint8_t off_hour, uint8_t off_minute) {
    uint32_t current_total_minutes = current_hour * 60 + current_minute;
    uint32_t on_total_minutes = on_hour * 60 + on_minute;
    uint32_t off_total_minutes = off_hour * 60 + off_minute;

    if (on_total_minutes < off_total_minutes) {
        // Normal schedule within the same day (e.g., 10:00 to 18:00)
        return (current_total_minutes >= on_total_minutes && current_total_minutes < off_total_minutes);
    } else if (on_total_minutes > off_total_minutes) {
        // Overnight schedule (e.g., 22:00 to 06:00)
        return (current_total_minutes >= on_total_minutes || current_total_minutes < off_total_minutes);
    } else {
        // on_total_minutes == off_total_minutes
        return false; // Assumes 0-hour duration if start and end are identical
    }
}

uint8_t SYS_MGR_Init(void) {
    if (is_initialized) {
        LOGW(TAG, "System Manager already initialized.");
        return APP_OK;
    }

    sys_mgr_state_mutex = xSemaphoreCreateMutex();
    if (sys_mgr_state_mutex == NULL) {
        LOGE(TAG, "Failed to create sys_mgr_state_mutex!");
        return APP_ERROR;
    }

    xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY); // Acquire mutex for initialization

    // Set initial operational parameters from config defaults
    sys_mgr_state.operational_temp_min = SYS_MGR_DEFAULT_MIN_TEMP_C;
    sys_mgr_state.operational_temp_max = SYS_MGR_DEFAULT_MAX_TEMP_C;
    sys_mgr_state.operational_humidity_min = SYS_MGR_DEFAULT_MIN_HUMIDITY_P;
    sys_mgr_state.operational_humidity_max = SYS_MGR_DEFAULT_MAX_HUMIDITY_P;

    sys_mgr_state.vent_on_hour = SYS_MGR_DEFAULT_VENT_ON_HOUR;
    sys_mgr_state.vent_on_minute = SYS_MGR_DEFAULT_VENT_ON_MINUTE;
    sys_mgr_state.vent_off_hour = SYS_MGR_DEFAULT_VENT_OFF_HOUR;
    sys_mgr_state.vent_off_minute = SYS_MGR_DEFAULT_VENT_OFF_MINUTE;

    sys_mgr_state.light_on_hour = SYS_MGR_DEFAULT_LIGHT_ON_HOUR;
    sys_mgr_state.light_on_minute = SYS_MGR_DEFAULT_LIGHT_ON_MINUTE;
    sys_mgr_state.light_off_hour = SYS_MGR_DEFAULT_LIGHT_OFF_HOUR;
    sys_mgr_state.light_off_minute = SYS_MGR_DEFAULT_LIGHT_OFF_MINUTE;

    sys_mgr_state.current_fan_stage = 0;
    sys_mgr_state.last_fan_stage_time_ms = 0;
    sys_mgr_state.heater_is_working = false;
    sys_mgr_state.pump_is_working = false;
    sys_mgr_state.ventilator_is_working = false;
    sys_mgr_state.fan_any_active = false;

    sys_mgr_state.temp_alarm_condition_active = false;
    sys_mgr_state.temp_alarm_start_time_ms = 0;
    sys_mgr_state.hum_alarm_condition_active = false;
    sys_mgr_state.hum_alarm_start_time_ms = 0;
    sys_mgr_state.fire_alarm_triggered = false;

    sys_mgr_state.current_room_temp_c = 0.0f;
    sys_mgr_state.current_room_humidity_p = 0.0f;
    sys_mgr_state.current_heatsink_temp_c = 0.0f;


    xSemaphoreGive(sys_mgr_state_mutex); // Release mutex

    // Create tasks
    if (xTaskCreate(sys_mgr_sensor_read_task, "SensorReadTask", 2048, NULL, 5, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create SensorReadTask!");
        return APP_ERROR;
    }
    if (xTaskCreate(sys_mgr_actuator_control_task, "ActuatorControlTask", 4096, NULL, 4, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create ActuatorControlTask!");
        return APP_ERROR;
    }
    if (xTaskCreate(sys_mgr_display_alarm_task, "DisplayAlarmTask", 3072, NULL, 3, NULL) != pdPASS) {
        LOGE(TAG, "Failed to create DisplayAlarmTask!");
        return APP_ERROR;
    }

    is_initialized = true;
    LOGI(TAG, "System Manager initialized and tasks created.");
    LOGI(TAG, "Operational Temp: %.1fC - %.1fC", sys_mgr_state.operational_temp_min, sys_mgr_state.operational_temp_max);
    LOGI(TAG, "Operational Humidity: %.1f%% - %.1f%%", sys_mgr_state.operational_humidity_min, sys_mgr_state.operational_humidity_max);
    LOGI(TAG, "Ventilator Schedule: %02u:%02u - %02u:%02u (simulated time)",
         sys_mgr_state.vent_on_hour, sys_mgr_state.vent_on_minute,
         sys_mgr_state.vent_off_hour, sys_mgr_state.vent_off_minute);
    LOGI(TAG, "Light Schedule: %02u:%02u - %02u:%02u (simulated time)",
         sys_mgr_state.light_on_hour, sys_mgr_state.light_on_minute,
         sys_mgr_state.light_off_hour, sys_mgr_state.light_off_minute);

    return APP_OK;
}

uint8_t SYS_MGR_SetOperationalTemperature(float min_temp_c, float max_temp_c) {
    if (min_temp_c >= max_temp_c) {
        LOGE(TAG, "Invalid temperature range: Min (%.1fC) must be less than Max (%.1fC).", min_temp_c, max_temp_c);
        return APP_ERROR;
    }
    if (xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY) == pdTRUE) {
        sys_mgr_state.operational_temp_min = min_temp_c;
        sys_mgr_state.operational_temp_max = max_temp_c;
        xSemaphoreGive(sys_mgr_state_mutex);
        LOGI(TAG, "Operational Temperature set to: %.1fC - %.1fC", min_temp_c, max_temp_c);
        return APP_OK;
    }
    LOGE(TAG, "Failed to acquire mutex for setting temperature.");
    return APP_ERROR;
}

uint8_t SYS_MGR_SetOperationalHumidity(float min_humidity_p, float max_humidity_p) {
    if (min_humidity_p >= max_humidity_p || min_humidity_p < 0 || max_humidity_p > 100) {
        LOGE(TAG, "Invalid humidity range: Min (%.1f%%) must be less than Max (%.1f%%) and within 0-100%%.", min_humidity_p, max_humidity_p);
        return APP_ERROR;
    }
    if (xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY) == pdTRUE) {
        sys_mgr_state.operational_humidity_min = min_humidity_p;
        sys_mgr_state.operational_humidity_max = max_humidity_p;
        xSemaphoreGive(sys_mgr_state_mutex);
        LOGI(TAG, "Operational Humidity set to: %.1f%% - %.1f%%", min_humidity_p, max_humidity_p);
        return APP_OK;
    }
    LOGE(TAG, "Failed to acquire mutex for setting humidity.");
    return APP_ERROR;
}

uint8_t SYS_MGR_SetVentilatorSchedule(uint8_t on_hour, uint8_t on_minute,
                                      uint8_t off_hour, uint8_t off_minute) {
    if (on_hour > 23 || on_minute > 59 || off_hour > 23 || off_minute > 59) {
        LOGE(TAG, "Invalid ventilator schedule time. Hours (0-23), Minutes (0-59).");
        return APP_ERROR;
    }
    if (xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY) == pdTRUE) {
        sys_mgr_state.vent_on_hour = on_hour;
        sys_mgr_state.vent_on_minute = on_minute;
        sys_mgr_state.vent_off_hour = off_hour;
        sys_mgr_state.vent_off_minute = off_minute;
        xSemaphoreGive(sys_mgr_state_mutex);
        LOGI(TAG, "Ventilator Schedule set to: %02u:%02u - %02u:%02u (simulated time)",
             on_hour, on_minute, off_hour, off_minute);
        return APP_OK;
    }
    LOGE(TAG, "Failed to acquire mutex for setting ventilator schedule.");
    return APP_ERROR;
}

uint8_t SYS_MGR_SetLightSchedule(uint8_t on_hour, uint8_t on_minute,
                                 uint8_t off_hour, uint8_t off_minute) {
    if (on_hour > 23 || on_minute > 59 || off_hour > 23 || off_minute > 59) {
        LOGE(TAG, "Invalid light schedule time. Hours (0-23), Minutes (0-59).");
        return APP_ERROR;
    }
    if (xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY) == pdTRUE) {
        sys_mgr_state.light_on_hour = on_hour;
        sys_mgr_state.light_on_minute = on_minute;
        sys_mgr_state.light_off_hour = off_hour;
        sys_mgr_state.light_off_minute = off_minute;
        xSemaphoreGive(sys_mgr_state_mutex);
        LOGI(TAG, "Light Schedule set to: %02u:%02u - %02u:%02u (simulated time)",
             on_hour, on_minute, off_hour, off_minute);
        return APP_OK;
    }
    LOGE(TAG, "Failed to acquire mutex for setting light schedule.");
    return APP_ERROR;
}


// --- TASK IMPLEMENTATIONS ---

/**
 * @brief FreeRTOS Task for reading environmental sensors.
 * Periodicity: 20ms
 */
static void sys_mgr_sensor_read_task(void *pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(20); // 20ms periodicity

    xLastWakeTime = xTaskGetTickCount();

    while (1) {
        float room_temp, room_humidity, heatsink_temp;

        // Read sensor data
        if (TEMP_SENSOR_ReadTemperature(TEMP_SENSOR_ROOM, &room_temp) != APP_OK) {
            LOGW(TAG, "Failed to read room temperature.");
            room_temp = 0.0f; 
        }
        if (HUMIDITY_SENSOR_ReadHumidity(HUMIDITY_SENSOR_ROOM, &room_humidity) != APP_OK) {
            LOGW(TAG, "Failed to read room humidity.");
            room_humidity = 0.0f;
        }
        if (TEMP_SENSOR_ReadTemperature(TEMP_SENSOR_HEATSINK, &heatsink_temp) != APP_OK) {
            LOGW(TAG, "Failed to read heatsink temperature.");
            heatsink_temp = 0.0f;
        }

        // Update shared state
        if (xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY) == pdTRUE) {
            sys_mgr_state.current_room_temp_c = room_temp;
            sys_mgr_state.current_room_humidity_p = room_humidity;
            sys_mgr_state.current_heatsink_temp_c = heatsink_temp;
            xSemaphoreGive(sys_mgr_state_mutex);
        } else {
            LOGE(TAG, "Sensor task failed to acquire mutex!");
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief FreeRTOS Task for controlling actuators (Heater, Fans, Pump, Ventilator, Lights)
 * and updating their LED indications.
 * Periodicity: 100ms
 */
static void sys_mgr_actuator_control_task(void *pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 100ms periodicity

    xLastWakeTime = xTaskGetTickCount();

    while (1) {
        // Local copies of state variables for consistent logic within this iteration
        // Acquire mutex once, copy necessary data, then release.
        float local_room_temp_c, local_room_humidity_p, local_heatsink_temp_c;
        float op_temp_min, op_temp_max, op_hum_min, op_hum_max;
        uint8_t vent_on_h, vent_on_m, vent_off_h, vent_off_m;
        uint8_t light_on_h, light_on_m, light_off_h, light_off_m;
        uint8_t local_current_fan_stage;
        uint64_t local_last_fan_stage_time_ms;
        bool local_heater_is_working;
        bool local_pump_is_working;
        bool local_ventilator_is_working;
        
        if (xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY) == pdTRUE) {
            local_room_temp_c = sys_mgr_state.current_room_temp_c;
            local_room_humidity_p = sys_mgr_state.current_room_humidity_p;
            local_heatsink_temp_c = sys_mgr_state.current_heatsink_temp_c;

            op_temp_min = sys_mgr_state.operational_temp_min;
            op_temp_max = sys_mgr_state.operational_temp_max;
            op_hum_min = sys_mgr_state.operational_humidity_min;
            op_hum_max = sys_mgr_state.operational_humidity_max;

            vent_on_h = sys_mgr_state.vent_on_hour;
            vent_on_m = sys_mgr_state.vent_on_minute;
            vent_off_h = sys_mgr_state.vent_off_hour;
            vent_off_m = sys_mgr_state.vent_off_minute;

            light_on_h = sys_mgr_state.light_on_hour;
            light_on_m = sys_mgr_state.light_on_minute;
            light_off_h = sys_mgr_state.light_off_hour;
            light_off_m = sys_mgr_state.light_off_minute;
            
            local_current_fan_stage = sys_mgr_state.current_fan_stage;
            local_last_fan_stage_time_ms = sys_mgr_state.last_fan_stage_time_ms;
            local_heater_is_working = sys_mgr_state.heater_is_working; // Read current state for logic
            local_pump_is_working = sys_mgr_state.pump_is_working;
            local_ventilator_is_working = sys_mgr_state.ventilator_is_working;

            xSemaphoreGive(sys_mgr_state_mutex);
        } else {
            LOGE(TAG, "Actuator control task failed to acquire mutex!");
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
            continue; // Skip this iteration if mutex not acquired
        }

        uint64_t current_time_ms = ECUAL_GetUptimeMs();
        uint32_t current_simulated_hour, current_simulated_minute;
        SYS_MGR_GetSimulatedTime(&current_simulated_hour, &current_simulated_minute);

        // --- Heater Control (Priority 1) ---
        bool heater_commanded_on = false; // Check commanded state, not just feedback
        HEATER_GetCommandedState(HEATER_ROOM, &heater_commanded_on);

        if (local_room_temp_c < op_temp_min) {
            if (!heater_commanded_on) {
                HEATER_On(HEATER_ROOM);
                LOGI(TAG, "Temp %.2fC below min %.1fC. Commanding Heater ON.", local_room_temp_c, op_temp_min);
            }
        } else if (local_room_temp_c >= (op_temp_min + TEMP_HEATER_HYSTERESIS_C)) {
            if (heater_commanded_on) {
                HEATER_Off(HEATER_ROOM);
                LOGI(TAG, "Temp %.2fC back to range. Commanding Heater OFF.", local_room_temp_c);
            }
        }
        HEATER_GetFeedbackState(HEATER_ROOM, &local_heater_is_working); // Update actual feedback state

        // --- Fan Control (Priority 2: Fans only if Heater is OFF) ---
        bool fan_main_active = false; // For LED indication
        if (local_heater_is_working) {
            if (local_current_fan_stage > 0) {
                FAN_Stop(FAN_MOTOR_1);
                FAN_Stop(FAN_MOTOR_2);
                FAN_Stop(FAN_MOTOR_3);
                local_current_fan_stage = 0;
                LOGI(TAG, "Heater active. All room fans forced OFF.");
            }
            local_last_fan_stage_time_ms = current_time_ms; // Reset timer for future use
        } else {
            // Fan staging logic
            if (local_room_temp_c > op_temp_max) {
                if (local_current_fan_stage == 0) {
                    FAN_SetSpeed(FAN_MOTOR_1, 50);
                    local_current_fan_stage = 1;
                    local_last_fan_stage_time_ms = current_time_ms;
                    LOGI(TAG, "Temp %.2fC above max %.1fC. Fan Stage 1 (FAN_MOTOR_1 ON).", local_room_temp_c, op_temp_max);
                } else if (local_current_fan_stage == 1 && (current_time_ms - local_last_fan_stage_time_ms) >= FAN_STAGE_DELAY_MS) {
                    FAN_SetSpeed(FAN_MOTOR_2, 75);
                    local_current_fan_stage = 2;
                    local_last_fan_stage_time_ms = current_time_ms;
                    LOGI(TAG, "Temp still high. Fan Stage 2 (FAN_MOTOR_2 ON).", local_room_temp_c);
                } else if (local_current_fan_stage == 2 && (current_time_ms - local_last_fan_stage_time_ms) >= FAN_STAGE_DELAY_MS) {
                    FAN_SetSpeed(FAN_MOTOR_3, 100);
                    local_current_fan_stage = 3;
                    local_last_fan_stage_time_ms = current_time_ms;
                    LOGI(TAG, "Temp still high. Fan Stage 3 (FAN_MOTOR_3 ON).", local_room_temp_c);
                }
            } else if (local_room_temp_c <= (op_temp_max - TEMP_FAN_OFF_HYSTERESIS_C)) {
                if (local_current_fan_stage > 0) {
                    FAN_Stop(FAN_MOTOR_1);
                    FAN_Stop(FAN_MOTOR_2);
                    FAN_Stop(FAN_MOTOR_3);
                    local_current_fan_stage = 0;
                    LOGI(TAG, "Temp %.2fC back to range. All room fans OFF.", local_room_temp_c);
                }
            }
        }
        fan_main_active = (local_current_fan_stage > 0); // Determine state for LED

        // Handle Cooling Fan (Heatsink)
        if (local_heatsink_temp_c > 60.0f) { FAN_Start(FAN_COOLING_FAN); }
        else if (local_heatsink_temp_c < 55.0f) { FAN_Stop(FAN_COOLING_FAN); }

        // --- Pump Control ---
        bool pump_commanded_on = false;
        PUMP_GetCommandedState(PUMP_WATER_CIRCULATION, &pump_commanded_on);

        if (local_room_humidity_p > op_hum_max) {
            if (!pump_commanded_on) {
                PUMP_On(PUMP_WATER_CIRCULATION);
                LOGI(TAG, "Humidity %.2f%% above max %.1f%%. Pump ON.", local_room_humidity_p, op_hum_max);
            }
        } else if (local_room_humidity_p <= (op_hum_max - HUMIDITY_PUMP_OFF_HYSTERESIS_P)) {
            if (pump_commanded_on) {
                PUMP_Off(PUMP_WATER_CIRCULATION);
                LOGI(TAG, "Humidity %.2f%% back to range. Pump OFF.", local_room_humidity_p);
            }
        }
        PUMP_GetFeedbackState(PUMP_WATER_CIRCULATION, &local_pump_is_working); // Update actual feedback state

        // --- Ventilator Control ---
        bool ventilator_scheduled_on = is_time_between(current_simulated_hour, current_simulated_minute,
                                                      vent_on_h, vent_on_m,
                                                      vent_off_h, vent_off_m);

        bool ventilator_forced_on_by_conditions = false;
        if (local_room_temp_c > (op_temp_max + VENT_TEMP_OVERRIDE_OFFSET_C) ||
            local_room_humidity_p > (op_hum_max + VENT_HUM_OVERRIDE_OFFSET_P)) {
            ventilator_forced_on_by_conditions = true;
            LOGW(TAG, "Ventilator override: Room Temp %.2fC (thr %.1fC) or Humidity %.2f%% (thr %.1f%%) too high!",
                 local_room_temp_c, (op_temp_max + VENT_TEMP_OVERRIDE_OFFSET_C),
                 local_room_humidity_p, (op_hum_max + VENT_HUM_OVERRIDE_OFFSET_P));
        }

        bool ventilator_commanded_on = false;
        VENTILATOR_GetState(VENTILATOR_EXHAUST_FAN, &ventilator_commanded_on);

        if (ventilator_scheduled_on || ventilator_forced_on_by_conditions) {
            if (!ventilator_commanded_on) {
                VENTILATOR_On(VENTILATOR_EXHAUST_FAN);
                LOGI(TAG, "Ventilator ON (Scheduled: %s, Forced: %s)",
                     ventilator_scheduled_on ? "Yes" : "No", ventilator_forced_on_by_conditions ? "Yes" : "No");
            }
        } else {
            if (ventilator_commanded_on) {
                VENTILATOR_Off(VENTILATOR_EXHAUST_FAN);
                LOGI(TAG, "Ventilator OFF (Scheduled: %s, Forced: %s)",
                     ventilator_scheduled_on ? "Yes" : "No", ventilator_forced_on_by_conditions ? "Yes" : "No");
            }
        }
        VENTILATOR_GetState(VENTILATOR_EXHAUST_FAN, &local_ventilator_is_working); // Update actual state


        // --- Light Control ---
        bool light_scheduled_on = is_time_between(current_simulated_hour, current_simulated_minute,
                                                  light_on_h, light_on_m,
                                                  light_off_h, light_off_m);

        bool light_living_room_is_on = false;
        LIGHT_GetCommandedState(LIGHT_LIVING_ROOM, &light_living_room_is_on);

        if (light_scheduled_on) {
            if (!light_living_room_is_on) {
                LIGHT_On(LIGHT_LIVING_ROOM);
                LOGI(TAG, "Living Room Light ON (Scheduled).");
            }
        } else {
            if (light_living_room_is_on) {
                LIGHT_Off(LIGHT_LIVING_ROOM);
                LOGI(TAG, "Living Room Light OFF (Scheduled).");
            }
        }

        // --- LED Indications for Actuators ---
        if (local_heater_is_working) { LIGHT_INDICATION_On(LIGHT_INDICATION_HEATER_ACTIVE); } else { LIGHT_INDICATION_Off(LIGHT_INDICATION_HEATER_ACTIVE); }
        if (fan_main_active)        { LIGHT_INDICATION_On(LIGHT_INDICATION_FAN_ACTIVE); } else { LIGHT_INDICATION_Off(LIGHT_INDICATION_FAN_ACTIVE); }
        if (local_pump_is_working)  { LIGHT_INDICATION_On(LIGHT_INDICATION_PUMP_ACTIVE); } else { LIGHT_INDICATION_Off(LIGHT_INDICATION_PUMP_ACTIVE); }
        if (local_ventilator_is_working) { LIGHT_INDICATION_On(LIGHT_INDICATION_VENTILATOR_ACTIVE); } else { LIGHT_INDICATION_Off(LIGHT_INDICATION_VENTILATOR_ACTIVE); }


        // --- Update shared state with current actuator statuses ---
        if (xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY) == pdTRUE) {
            sys_mgr_state.current_fan_stage = local_current_fan_stage;
            sys_mgr_state.last_fan_stage_time_ms = local_last_fan_stage_time_ms;
            sys_mgr_state.heater_is_working = local_heater_is_working;
            sys_mgr_state.pump_is_working = local_pump_is_working;
            sys_mgr_state.ventilator_is_working = local_ventilator_is_working;
            sys_mgr_state.fan_any_active = fan_main_active; // The combined fan status
            xSemaphoreGive(sys_mgr_state_mutex);
        } else {
            LOGE(TAG, "Actuator control task failed to acquire mutex for updating state!");
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief FreeRTOS Task for managing display updates and fire alarm logic.
 * Periodicity: 1000ms (1 second)
 */
static void sys_mgr_display_alarm_task(void *pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(1000); // 1 second periodicity

    xLastWakeTime = xTaskGetTickCount();

    while (1) {
        // Local copies of state variables
        float local_room_temp_c, local_room_humidity_p;
        float op_temp_min, op_temp_max, op_hum_min, op_hum_max;
        bool local_heater_is_working, local_pump_is_working, local_ventilator_is_working, local_fan_any_active;
        bool local_temp_alarm_condition_active, local_hum_alarm_condition_active;
        uint64_t local_temp_alarm_start_time_ms, local_hum_alarm_start_time_ms;
        bool local_fire_alarm_triggered;

        if (xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY) == pdTRUE) {
            local_room_temp_c = sys_mgr_state.current_room_temp_c;
            local_room_humidity_p = sys_mgr_state.current_room_humidity_p;
            op_temp_min = sys_mgr_state.operational_temp_min;
            op_temp_max = sys_mgr_state.operational_temp_max;
            op_hum_min = sys_mgr_state.operational_humidity_min;
            op_hum_max = sys_mgr_state.operational_humidity_max;
            local_heater_is_working = sys_mgr_state.heater_is_working;
            local_pump_is_working = sys_mgr_state.pump_is_working;
            local_ventilator_is_working = sys_mgr_state.ventilator_is_working;
            local_fan_any_active = sys_mgr_state.fan_any_active;
            local_temp_alarm_condition_active = sys_mgr_state.temp_alarm_condition_active;
            local_temp_alarm_start_time_ms = sys_mgr_state.temp_alarm_start_time_ms;
            local_hum_alarm_condition_active = sys_mgr_state.hum_alarm_condition_active;
            local_hum_alarm_start_time_ms = sys_mgr_state.hum_alarm_start_time_ms;
            local_fire_alarm_triggered = sys_mgr_state.fire_alarm_triggered;
            xSemaphoreGive(sys_mgr_state_mutex);
        } else {
            LOGE(TAG, "Display/Alarm task failed to acquire mutex!");
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
            continue; // Skip this iteration
        }

        uint64_t current_time_ms = ECUAL_GetUptimeMs();

        // --- Fire Alarm Logic ---
        bool current_temp_out_of_range = (local_room_temp_c < op_temp_min) || (local_room_temp_c > op_temp_max);
        bool current_hum_out_of_range = (local_room_humidity_p < op_hum_min) || (local_room_humidity_p > op_hum_max);

        // Update shared alarm state variables inside mutex
        if (xSemaphoreTake(sys_mgr_state_mutex, portMAX_DELAY) == pdTRUE) {
            // Temperature Alarm Check
            if (current_temp_out_of_range && (local_heater_is_working || local_fan_any_active || local_ventilator_is_working)) {
                if (!sys_mgr_state.temp_alarm_condition_active) {
                    sys_mgr_state.temp_alarm_condition_active = true;
                    sys_mgr_state.temp_alarm_start_time_ms = current_time_ms;
                    LOGW(TAG, "Temp alarm condition started (%.2fC out of range). Actuator working.", local_room_temp_c);
                } else if ((current_time_ms - sys_mgr_state.temp_alarm_start_time_ms) >= FIRE_ALARM_DELAY_MS) {
                    if (!sys_mgr_state.fire_alarm_triggered) {
                        sys_mgr_state.fire_alarm_triggered = true;
                        LOGE(TAG, "FIRE ALARM! Temp (%.2fC) critical for >2 min! Actuator(s) working: Heater(%s) Fan(%s) Vent(%s)",
                                 local_room_temp_c, local_heater_is_working?"ON":"OFF", local_fan_any_active?"ON":"OFF", local_ventilator_is_working?"ON":"OFF");
                        LIGHT_INDICATION_On(LIGHT_INDICATION_ERROR);
                    }
                }
            } else {
                if (sys_mgr_state.temp_alarm_condition_active) {
                    LOGI(TAG, "Temp alarm condition cleared. (%.2fC in range or no actuator working).", local_room_temp_c);
                }
                sys_mgr_state.temp_alarm_condition_active = false;
                sys_mgr_state.temp_alarm_start_time_ms = 0;
            }

            // Humidity Alarm Check
            if (current_hum_out_of_range && (local_pump_is_working || local_ventilator_is_working)) {
                if (!sys_mgr_state.hum_alarm_condition_active) {
                    sys_mgr_state.hum_alarm_condition_active = true;
                    sys_mgr_state.hum_alarm_start_time_ms = current_time_ms;
                    LOGW(TAG, "Hum alarm condition started (%.2f%% out of range). Actuator working.", local_room_humidity_p);
                } else if ((current_time_ms - sys_mgr_state.hum_alarm_start_time_ms) >= FIRE_ALARM_DELAY_MS) {
                    if (!sys_mgr_state.fire_alarm_triggered) {
                        sys_mgr_state.fire_alarm_triggered = true;
                        LOGE(TAG, "FIRE ALARM! Humidity (%.2f%%) critical for >2 min! Actuator(s) working: Pump(%s) Vent(%s)",
                                 local_room_humidity_p, local_pump_is_working?"ON":"OFF", local_ventilator_is_working?"ON":"OFF");
                        LIGHT_INDICATION_On(LIGHT_INDICATION_ERROR);
                    }
                }
            } else {
                if (sys_mgr_state.hum_alarm_condition_active) {
                    LOGI(TAG, "Hum alarm condition cleared. (%.2f%% in range or no actuator working).", local_room_humidity_p);
                }
                sys_mgr_state.hum_alarm_condition_active = false;
                sys_mgr_state.hum_alarm_start_time_ms = 0;
            }

            // If both alarm conditions clear, and previously triggered, clear the main alarm LED
            if (!sys_mgr_state.temp_alarm_condition_active && !sys_mgr_state.hum_alarm_condition_active && sys_mgr_state.fire_alarm_triggered) {
                sys_mgr_state.fire_alarm_triggered = false;
                LIGHT_INDICATION_Off(LIGHT_INDICATION_ERROR);
                LOGI(TAG, "Fire Alarm cleared.");
            }
            // Re-fetch current alarm status for display
            local_fire_alarm_triggered = sys_mgr_state.fire_alarm_triggered;
            xSemaphoreGive(sys_mgr_state_mutex);
        } else {
            LOGE(TAG, "Display/Alarm task failed to acquire mutex for alarm update!");
        }


        // --- Display Temperature and Humidity on Character Screen ---
        char display_buffer[32]; // Max 16 chars per line for typical 16x2 LCD
        CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_MAIN_STATUS);
        
        // Line 1: Temperature
        snprintf(display_buffer, sizeof(display_buffer), "Temp:%.1fC", local_room_temp_c);
        CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_MAIN_STATUS, display_buffer);
        CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_MAIN_STATUS, 0, 1); // Move to second line
        snprintf(display_buffer, sizeof(display_buffer), " Hum:%.1f%%", local_room_humidity_p);
        CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_MAIN_STATUS, display_buffer);

        // Update Alarm Panel display with System Status (Alarm, Actuators)
        CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_ALARM_PANEL);
        
        // Line 1: Alarm Status
        if (local_fire_alarm_triggered) {
            CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "!!! FIRE ALARM !!!");
        } else {
            CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "STATUS: NORMAL");
        }

        // Line 2: Actuator Status (Heater/Fan/Pump/Vent)
        CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_ALARM_PANEL, 0, 1);
        char actuator_status[17]; // 16 chars + null
        snprintf(actuator_status, sizeof(actuator_status), "H%s F%s P%s V%s",
                 local_heater_is_working ? "ON" : "OF",
                 local_fan_any_active ? "ON" : "OF", // Displaying "OF" for "OFF" to fit
                 local_pump_is_working ? "ON" : "OF",
                 local_ventilator_is_working ? "ON" : "OF");
        CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, actuator_status);


        LOGD(TAG, "SYS_MGR Display/Alarm Task iteration complete.");

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
