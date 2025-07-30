// In your main application file (e.g., main/main.c)
#include "ecual_gpio.h"
#include "ecual_pwm.h"
#include "ecual_adc.h"
#include "ecual_i2c.h"
#include "ecual_uart.h"
#include "ecual_common.h"

#include "fan.h"
#include "temp_sensor.h"
#include "ventilator.h"
#include "humidity_sensor.h"
#include "heater.h"
#include "pump.h"
#include "light_control.h"
#include "light_indication.h"
#include "character_display.h"
#include "logger.h" // <--- UPDATED INCLUDE

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char *APP_TAG = "MAIN"; // Using a shorter tag for consistency

void app_main(void) {
    // Initialize ECUAL UART first, as LOGGER depends on it
    // Using LOGI here directly assuming UART is initialized enough for this first message
    // even if LOGGER is not fully set up.
    // For a truly robust system, initial critical messages might go to a direct UART print.
    // But for demo purposes, this is acceptable after ECUAL_UART_Init.
    // Original esp_log calls would be removed if you strictly don't want any ESP-IDF logging.
    // If you need to debug ECUAL_UART_Init, you might temporarily use ESP_LOGI or a direct printf.
    
    // NOTE: You must have ecual_uart_config.c set up for ECUAL_UART_0 (or your chosen UART ID)
    // and its Init function (ECUAL_UART_Init()) should handle the underlying UART driver setup.
    if (ECUAL_UART_Init() != ECUAL_OK) { 
        // Fallback for critical error if even UART can't init
        printf("CRITICAL ERROR: UART ECUAL Init failed! Cannot log.\r\n");
        return; 
    }
    
    // Now initialize the generic logger
    if (LOGGER_Init() != APP_OK) { // <--- UPDATED CALL
        LOGE(APP_TAG, "Logger Init failed! Serial logging may not work reliably."); // <--- UPDATED LOG MACRO
        return; 
    }
    
    // From this point forward, use LOGx macros
    LOGI(APP_TAG, "Initializing ECUAL modules..."); // <--- UPDATED LOG MACRO
    if (ECUAL_GPIO_Init() != ECUAL_OK) { LOGE(APP_TAG, "GPIO ECUAL Init failed!"); return; } // <--- UPDATED LOG MACRO
    if (ECUAL_PWM_Init() != ECUAL_OK) { LOGE(APP_TAG, "PWM ECUAL Init failed!"); return; }
    if (ECUAL_ADC_Init() != ECUAL_OK) { LOGE(APP_TAG, "ADC ECUAL Init failed!"); return; }
    if (ECUAL_I2C_Init() != ECUAL_OK) { LOGE(APP_TAG, "I2C ECUAL Init failed!"); return; }

    LOGI(APP_TAG, "Initializing Application modules...");
    if (FAN_Init() != APP_OK) { LOGE(APP_TAG, "Fan APP Init failed!"); return; }
    if (TEMP_SENSOR_Init() != APP_OK) { LOGE(APP_TAG, "Temperature Sensor APP Init failed!"); return; }
    if (VENTILATOR_Init() != APP_OK) { LOGE(APP_TAG, "Ventilator APP Init failed!"); return; }
    if (HUMIDITY_SENSOR_Init() != APP_OK) { LOGE(APP_TAG, "Humidity Sensor APP Init failed!"); return; }
    if (HEATER_Init() != APP_OK) { LOGE(APP_TAG, "Heater APP Init failed!"); return; }
    if (PUMP_Init() != APP_OK) { LOGE(APP_TAG, "Pump APP Init failed!"); return; }
    if (LIGHT_Init() != APP_OK) { LOGE(APP_TAG, "LightControl APP Init failed!"); return; }
    if (LIGHT_INDICATION_Init() != APP_OK) { LOGE(APP_TAG, "LightIndication APP Init failed!"); return; }
    if (CHARACTER_DISPLAY_Init() != APP_OK) { LOGE(APP_TAG, "CharacterDisplay APP Init failed!"); return; }

    LOGI(APP_TAG, "Application initialized. Starting main loop.");

    float room_temp_c, heatsink_temp_c;
    float room_humidity_p, bathroom_humidity_p, server_rack_humidity_p;
    bool vent_is_on;
    bool room_heater_commanded_on, water_heater_commanded_on, garage_heater_commanded_on;
    bool room_heater_is_working, garage_heater_is_working;
    bool water_pump_commanded_on, drainage_pump_commanded_on, irrigation_pump_commanded_on;
    bool water_pump_is_working, irrigation_pump_is_working;
    bool kitchen_light_on, bedroom_light_on;
    uint8_t living_room_brightness, kitchen_light_brightness;
    bool sys_status_led_on, error_led_on, wifi_led_on;
    uint8_t fan_speed_percent;

    char display_buffer[32]; // Buffer for display text

    // Initial display messages
    CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_MAIN_STATUS);
    CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_MAIN_STATUS, "System Online");
    CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_MAIN_STATUS, 0, 1);
    CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_MAIN_STATUS, "Init OK!");

    CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_ALARM_PANEL);
    CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "ALARM STATUS");
    CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_ALARM_PANEL, 0, 1);
    CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "No Issues.");


    while (1) {
        // --- Temperature Readings & Heater/Fan Control ---
        if (TEMP_SENSOR_ReadTemperature(TEMP_SENSOR_ROOM, &room_temp_c) == APP_OK) {
            // Fan control
            if (room_temp_c > 28.0f) { FAN_SetSpeed(FAN_MOTOR_1, 75); }
            else if (room_temp_c < 25.0f) { FAN_Stop(FAN_MOTOR_1); }
            FAN_GetSpeed(FAN_MOTOR_1, &fan_speed_percent);
            LOGI(APP_TAG, "FAN_MOTOR_1 Current Speed: %u%%", fan_speed_percent); // <--- UPDATED LOG MACRO

            // Room Heater control
            if (room_temp_c < 20.0f) {
                if (HEATER_GetCommandedState(HEATER_ROOM, &room_heater_commanded_on) == APP_OK && !room_heater_commanded_on) {
                    LOGI(APP_TAG, "Room temp low (%.2fC), commanding Room Heater ON.", room_temp_c);
                    HEATER_On(HEATER_ROOM);
                }
            } else if (room_temp_c > 22.0f) {
                if (HEATER_GetCommandedState(HEATER_ROOM, &room_heater_commanded_on) == APP_OK && room_heater_commanded_on) {
                    LOGI(APP_TAG, "Room temp good (%.2fC), commanding Room Heater OFF.", room_temp_c);
                    HEATER_Off(HEATER_ROOM);
                }
            }
            // Read and log Room Heater feedback
            if (HEATER_GetFeedbackState(HEATER_ROOM, &room_heater_is_working) == APP_OK) {
                LOGI(APP_TAG, "Room Heater (Commanded: %s, Working: %s)",
                         room_heater_commanded_on ? "ON" : "OFF", room_heater_is_working ? "YES" : "NO");
            } else {
                LOGW(APP_TAG, "Room Heater feedback not available or failed to read.");
            }
        } else { LOGE(APP_TAG, "Could not read room temperature."); }

        if (TEMP_SENSOR_ReadTemperature(TEMP_SENSOR_HEATSINK, &heatsink_temp_c) == APP_OK) {
            if (heatsink_temp_c > 60.0f) { FAN_Start(FAN_COOLING_FAN); }
            else if (heatsink_temp_c < 55.0f) { FAN_Stop(FAN_COOLING_FAN); }
        } else { LOGE(APP_TAG, "Could not read heatsink temperature."); }

        // --- Humidity Readings & Ventilator Control ---
        if (HUMIDITY_SENSOR_ReadHumidity(HUMIDITY_SENSOR_ROOM, &room_humidity_p) == APP_OK) {
            if (room_humidity_p > 70.0f) {
                if (VENTILATOR_GetState(VENTILATOR_EXHAUST_FAN, &vent_is_on) == APP_OK && !vent_is_on) { VENTILATOR_On(VENTILATOR_EXHAUST_FAN); }
            } else if (room_humidity_p < 60.0f) {
                if (VENTILATOR_GetState(VENTILATOR_EXHAUST_FAN, &vent_is_on) == APP_OK && vent_is_on) { VENTILATOR_Off(VENTILATOR_EXHAUST_FAN); }
            }
            LOGI(APP_TAG, "Room Humidity: %.2f %%", room_humidity_p);
        } else { LOGE(APP_TAG, "Could not read room humidity."); }

        if (HUMIDITY_SENSOR_ReadHumidity(HUMIDITY_SENSOR_BATHROOM, &bathroom_humidity_p) == APP_OK) {
            LOGI(APP_TAG, "Bathroom Humidity: %.2f %%", bathroom_humidity_p);
        } else { LOGE(APP_TAG, "Could not read bathroom humidity."); }

        if (HUMIDITY_SENSOR_ReadHumidity(HUMIDITY_SENSOR_SERVER_RACK, &server_rack_humidity_p) == APP_OK) {
            LOGI(APP_TAG, "Server Rack Humidity (SHT3X): %.2f %%", server_rack_humidity_p);
        } else { LOGE(APP_TAG, "Could not read server rack humidity."); }

        // --- Water Tank Heater Control (no feedback) ---
        static uint8_t water_heater_toggle_count = 0;
        water_heater_toggle_count++;
        if (water_heater_toggle_count % 3 == 0) {
            if (HEATER_GetCommandedState(HEATER_WATER_TANK, &water_heater_commanded_on) == APP_OK) {
                if (water_heater_commanded_on) { HEATER_Off(HEATER_WATER_TANK); }
                else { HEATER_On(HEATER_WATER_TANK); }
                LOGI(APP_TAG, "Water Tank Heater (Commanded: %s)", water_heater_commanded_on ? "ON" : "OFF");
            } else { LOGE(APP_TAG, "Could not get water tank heater commanded state."); }
        }

        // --- Garage Heater Control (with feedback, always commanded ON for demo) ---
        HEATER_On(HEATER_GARAGE);
        if (HEATER_GetFeedbackState(HEATER_GARAGE, &garage_heater_is_working) == APP_OK) {
            LOGI(APP_TAG, "Garage Heater (Commanded: ON, Working: %s)", garage_heater_is_working ? "YES" : "NO");
        } else {
            LOGW(APP_TAG, "Garage Heater feedback not available or failed to read.");
        }

        // --- Pump Controls ---
        // Water Circulation Pump (with feedback) - Toggle every 10 seconds
        static uint8_t water_pump_toggle_count = 0;
        water_pump_toggle_count++;
        if (water_pump_toggle_count % 2 == 0) {
            if (PUMP_GetCommandedState(PUMP_WATER_CIRCULATION, &water_pump_commanded_on) == APP_OK) {
                if (water_pump_commanded_on) { PUMP_Off(PUMP_WATER_CIRCULATION); }
                else { PUMP_On(PUMP_WATER_CIRCULATION); }
                LOGI(APP_TAG, "Water Circulation Pump (Commanded: %s)", water_pump_commanded_on ? "ON" : "OFF");
            } else { LOGE(APP_TAG, "Could not get water pump commanded state."); }

            if (PUMP_GetFeedbackState(PUMP_WATER_CIRCULATION, &water_pump_is_working) == APP_OK) {
                LOGI(APP_TAG, "Water Circulation Pump (Working: %s)", water_pump_is_working ? "YES" : "NO");
            } else {
                LOGW(APP_TAG, "Water Circulation Pump feedback not available or failed to read.");
            }
        }

        // Drainage Pump (without feedback) - Command ON every 20 seconds
        static uint8_t drainage_pump_toggle_count = 0;
        drainage_pump_toggle_count++;
        if (drainage_pump_toggle_count % 4 == 0) {
            LOGI(APP_TAG, "Commanding Drainage Pump ON (demo).");
            PUMP_On(PUMP_DRAINAGE);
        } else if (drainage_pump_toggle_count % 4 == 1) {
            LOGI(APP_TAG, "Commanding Drainage Pump OFF (demo).");
            PUMP_Off(PUMP_DRAINAGE);
        }
        if (PUMP_GetCommandedState(PUMP_DRAINAGE, &drainage_pump_commanded_on) == APP_OK) {
            LOGI(APP_TAG, "Drainage Pump (Commanded: %s)", drainage_pump_commanded_on ? "ON" : "OFF");
        }

        // Irrigation Pump (with feedback) - Command ON if room temp is high (for demo)
        if (room_temp_c > 27.0f) {
            if (PUMP_GetCommandedState(PUMP_IRRIGATION, &irrigation_pump_commanded_on) == APP_OK && !irrigation_pump_commanded_on) {
                LOGI(APP_TAG, "Room temp high (%.2fC), commanding Irrigation Pump ON.", room_temp_c);
                PUMP_On(PUMP_IRRIGATION);
            }
        } else if (room_temp_c < 24.0f) {
            if (PUMP_GetCommandedState(PUMP_IRRIGATION, &irrigation_pump_commanded_on) == APP_OK && irrigation_pump_commanded_on) {
                LOGI(APP_TAG, "Room temp low (%.2fC), commanding Irrigation Pump OFF.", room_temp_c);
                PUMP_Off(PUMP_IRRIGATION);
            }
        }
        if (PUMP_GetFeedbackState(PUMP_IRRIGATION, &irrigation_pump_is_working) == APP_OK) {
            LOGI(APP_TAG, "Irrigation Pump (Commanded: %s, Working: %s)",
                     irrigation_pump_commanded_on ? "ON" : "OFF", irrigation_pump_is_working ? "YES" : "NO");
        } else {
            LOGW(APP_TAG, "Irrigation Pump feedback not available or failed to read.");
        }

        // --- Light Controls ---
        // Kitchen Light (ON/OFF) - Toggle every 10 seconds
        static uint8_t kitchen_light_toggle_count = 0;
        kitchen_light_toggle_count++;
        if (kitchen_light_toggle_count % 2 == 0) {
            if (LIGHT_GetCommandedState(LIGHT_KITCHEN, &kitchen_light_on) == APP_OK) {
                if (kitchen_light_on) { LIGHT_Off(LIGHT_KITCHEN); }
                else { LIGHT_On(LIGHT_KITCHEN); }
                LOGI(APP_TAG, "Kitchen Light (Commanded: %s)", kitchen_light_on ? "OFF" : "ON");
            } else { LOGE(APP_TAG, "Could not get Kitchen Light commanded state."); }
        }
        if (LIGHT_GetBrightness(LIGHT_KITCHEN, &kitchen_light_brightness) == APP_OK) {
            LOGI(APP_TAG, "Kitchen Light brightness: %u%%", kitchen_light_brightness);
        }

        // Living Room Light (Dimmable) - Cycle brightness every 5 seconds
        static uint8_t living_room_dim_level = 0;
        living_room_dim_level += 20;
        if (living_room_dim_level > 100) { living_room_dim_level = 0; }
        if (LIGHT_SetBrightness(LIGHT_LIVING_ROOM, living_room_dim_level) == APP_OK) {
            LOGI(APP_TAG, "Living Room Light set to %u%%.", living_room_dim_level);
        } else {
            LOGE(APP_TAG, "Failed to set Living Room Light brightness.");
        }
        if (LIGHT_GetBrightness(LIGHT_LIVING_ROOM, &living_room_brightness) == APP_OK) {
            LOGI(APP_TAG, "Living Room Light current brightness: %u%%", living_room_brightness);
        }

        // Bedroom Light (ON/OFF) - Turn ON if it's past 6 PM (demo based on 24-hr cycle)
        static bool bedroom_light_managed = false;
        if (!bedroom_light_managed) {
            static uint32_t startup_time_s = 0;
            if (xTaskGetTickCount() * portTICK_PERIOD_MS / 1000 > 20 && !bedroom_light_on) {
                LIGHT_On(LIGHT_BEDROOM);
                LOGI(APP_TAG, "Bedroom Light turned ON (demo time check).");
                bedroom_light_on = true;
            }
            if (xTaskGetTickCount() * portTICK_PERIOD_MS / 1000 > 40 && bedroom_light_on) {
                LIGHT_Off(LIGHT_BEDROOM);
                LOGI(APP_TAG, "Bedroom Light turned OFF (demo time check).");
                bedroom_light_on = false;
                bedroom_light_managed = true;
            }
        }
        if (LIGHT_GetCommandedState(LIGHT_BEDROOM, &bedroom_light_on) == APP_OK) {
            LOGI(APP_TAG, "Bedroom Light (Commanded: %s)", bedroom_light_on ? "ON" : "OFF");
        }

        // --- Light Indication Controls ---
        // System Status LED - Toggle every second (heartbeat)
        static uint8_t sys_status_toggle_count = 0;
        sys_status_toggle_count++;
        if (sys_status_toggle_count % 1 == 0) {
            LIGHT_INDICATION_Toggle(LIGHT_INDICATION_SYSTEM_STATUS);
            if (LIGHT_INDICATION_GetState(LIGHT_INDICATION_SYSTEM_STATUS, &sys_status_led_on) == APP_OK) {
                LOGI(APP_TAG, "System Status LED (State: %s)", sys_status_led_on ? "ON" : "OFF");
            }
        }

        // Error LED - Turn ON if room humidity is very high, OFF otherwise
        if (room_humidity_p > 85.0f) {
            LIGHT_INDICATION_On(LIGHT_INDICATION_ERROR);
            LOGW(APP_TAG, "ERROR! Room Humidity %.2f%%. Error LED ON.", room_humidity_p);
        } else {
            LIGHT_INDICATION_Off(LIGHT_INDICATION_ERROR);
        }
        if (LIGHT_INDICATION_GetState(LIGHT_INDICATION_ERROR, &error_led_on) == APP_OK) {
            LOGI(APP_TAG, "Error LED (State: %s)", error_led_on ? "ON" : "OFF");
        }

        // WiFi Status LED - Always ON for this demo
        LIGHT_INDICATION_On(LIGHT_INDICATION_WIFI_STATUS);
        if (LIGHT_INDICATION_GetState(LIGHT_INDICATION_WIFI_STATUS, &wifi_led_on) == APP_OK) {
            LOGI(APP_TAG, "WiFi Status LED (State: %s)", wifi_led_on ? "ON" : "OFF");
        }

        // --- Character Display Updates ---
        // Update Main Status Display (e.g., Room Temp & Humidity)
        CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_MAIN_STATUS);
        snprintf(display_buffer, sizeof(display_buffer), "Temp: %.1f C", room_temp_c);
        CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_MAIN_STATUS, display_buffer);
        CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_MAIN_STATUS, 0, 1);
        snprintf(display_buffer, sizeof(display_buffer), "Hum: %.1f %%", room_humidity_p);
        CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_MAIN_STATUS, display_buffer);

        // Update Alarm Panel Display (e.g., Heater Status & System Status)
        CHARACTER_DISPLAY_Clear(CHARACTER_DISPLAY_ALARM_PANEL);
        CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "Sys Status: ");
        if (LIGHT_INDICATION_GetState(LIGHT_INDICATION_SYSTEM_STATUS, &sys_status_led_on) == APP_OK) {
            CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, sys_status_led_on ? "OK" : "STANDBY");
        } else {
            CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "N/A");
        }

        CHARACTER_DISPLAY_SetCursor(CHARACTER_DISPLAY_ALARM_PANEL, 0, 1);
        CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "R.Heater: ");
        if (HEATER_GetCommandedState(HEATER_ROOM, &room_heater_commanded_on) == APP_OK &&
            HEATER_GetFeedbackState(HEATER_ROOM, &room_heater_is_working) == APP_OK) {
            CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, room_heater_is_working ? "ON" : "OFF");
        } else {
            CHARACTER_DISPLAY_PrintString(CHARACTER_DISPLAY_ALARM_PANEL, "N/A");
        }
        
        // Toggle backlight for Alarm Panel every 15 seconds
        static uint8_t backlight_toggle_count = 0;
        backlight_toggle_count++;
        if (backlight_toggle_count % 3 == 0) {
            static bool backlight_currently_on = true;
            if(backlight_currently_on) {
                CHARACTER_DISPLAY_BacklightOff(CHARACTER_DISPLAY_ALARM_PANEL);
            } else {
                CHARACTER_DISPLAY_BacklightOn(CHARACTER_DISPLAY_ALARM_PANEL);
            }
            backlight_currently_on = !backlight_currently_on;
            LOGI(APP_TAG, "Alarm Panel Backlight toggled.");
        }


        LOGD(APP_TAG, "Main loop iteration complete."); // <--- UPDATED LOG MACRO
        vTaskDelay(pdMS_TO_TICKS(5000)); // Loop every 5 seconds
    }
}
