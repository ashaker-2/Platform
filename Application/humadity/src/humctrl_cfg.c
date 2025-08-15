#include "humctrl_cfg.h"

/**
 * @file humctrl_cfg.c
 * @brief Configuration data definitions for the HumCtrl component.
 *
 * This file contains the array of structures that defines all configured
 * humidity sensors and their properties.
 */

// --- Array of Sensor Configurations ---
const HumCtrl_Config_t humctrl_sensor_configs[HUMCTRL_SENSOR_COUNT] = {
    {
        .id = HUMCTRL_SENSOR_ID_INT_1,
        .type = HUMCTRL_SENSOR_TYPE_ANALOG_ADC,
        .interface_details.analog_adc = {
            .adc_channel = 5,
            .voltage_scale_factor = 0.805f // Example: 12-bit ADC, 3.3V ref -> 0.805 mV/count
        },
        .calibration = {
            .voltage_to_rh_slope = 0.05f, // Example value (V to %)
            .voltage_to_rh_offset = -20.0f
        }
    },
    {
        .id = HUMCTRL_SENSOR_ID_EXT_1,
        .type = HUMCTRL_SENSOR_TYPE_I2C,
        .interface_details.i2c = {
            .i2c_port = 1,
            .i2c_address = 0x44 // Example address for an SHT3x sensor
        },
        .calibration = {
            .voltage_to_rh_slope = 0.0f, // N/A for I2C
            .voltage_to_rh_offset = 0.0f  // N/A for I2C
        }
    }
};