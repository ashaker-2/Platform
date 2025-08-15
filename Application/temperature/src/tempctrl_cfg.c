#include "tempctrl_cfg.h"

/**
 * @file tempctrl_cfg.c
 * @brief Configuration data definitions for the TempCtrl component.
 *
 * This file contains the array of structures that defines all configured temperature
 * sensors and their properties.
 */

// --- Array of Sensor Configurations ---
// This is the main configuration array for all temperature sensors.
const TempCtrl_SensorConfig_t tempctrl_sensor_configs[TempCtrl_SENSOR_COUNT] = {
    {
        .id = TempCtrl_SENSOR_ID_ROOM,
        .type = TempCtrl_SENSOR_TYPE_1WIRE_DS18B20,
        .comm_details.one_wire = {
            .one_wire_bus_id = 0,
            .ds18b20_rom_address = {0x28, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x01}},
        .calibration_slope = 1.0f,
        .calibration_offset = 0.0f,
        .read_delay_ms = 750 // Max conversion time for DS18B20
    },
    {
        .id = TempCtrl_SENSOR_ID_OUTDOOR, .type = TempCtrl_SENSOR_TYPE_I2C, .comm_details.i2c = {
                                                                                .i2c_port = 0,
                                                                                .i2c_address = 0x48 // Example for TMP102
                                                                            },
        .calibration_slope = 1.0f,
        .calibration_offset = 0.0f,
        .read_delay_ms = 100 // Example read delay for I2C sensor
    },
    {
        .id = TempCtrl_SENSOR_ID_HEATER_EXHAUST,
        .type = TempCtrl_SENSOR_TYPE_ANALOG_ADC, 
        .comm_details.analog_adc = {.adc_channel = 5}, 
        .calibration_slope = 0.5f, 
        .calibration_offset = -10.0f, 
        .read_delay_ms = 10},
    // Add more sensor configurations here as defined in the DDD example.
};
