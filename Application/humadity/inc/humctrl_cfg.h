#ifndef HUMCTRL_CFG_H
#define HUMCTRL_CFG_H

#include "common.h"
#include "system_monitor.h" // For fault IDs
#include "hal_adc.h"
#include "hal_i2c.h"
#include <stdint.h>

/**
 * @file humctrl_cfg.h
 * @brief Configuration header for the HumCtrl component.
 *
 * This file defines all the necessary data types, macros, and external declarations
 * for configuring the HumCtrl module for a specific application.
 */

// --- Fault ID Definitions (from SystemMonitor) ---
#define FAULT_ID_HUMCTRL_SENSOR_INIT_FAILED     (SystemMonitor_FaultId_t)310
#define FAULT_ID_HUMCTRL_SENSOR_READ_FAILURE    (SystemMonitor_FaultId_t)311
#define FAULT_ID_HUMCTRL_SENSOR_DATA_INVALID    (SystemMonitor_FaultId_t)312

// --- Humidity Sensor Type Enums ---
typedef enum {
    HUMCTRL_SENSOR_TYPE_ANALOG_ADC,
    HUMCTRL_SENSOR_TYPE_I2C,
    HUMCTRL_SENSOR_TYPE_COUNT
} HumCtrl_SensorType_t;

// --- Sensor Interface Details Union ---
typedef union {
    struct {
        uint8_t adc_channel;
        float voltage_scale_factor; // mV per ADC count
    } analog_adc;
    struct {
        uint8_t i2c_port;
        uint8_t i2c_address;
    } i2c;
} HumCtrl_InterfaceDetails_t;

// --- Sensor Calibration Structure ---
typedef struct {
    float voltage_to_rh_slope;
    float voltage_to_rh_offset;
} HumCtrl_Calibration_t;

// --- Sensor Configuration Structure ---
typedef struct {
    uint32_t id; // Unique ID for this sensor instance
    HumCtrl_SensorType_t type;
    HumCtrl_InterfaceDetails_t interface_details;
    HumCtrl_Calibration_t calibration;
} HumCtrl_Config_t;

// --- Sensor IDs ---
typedef enum {
    HUMCTRL_SENSOR_ID_INT_1 = 0,
    HUMCTRL_SENSOR_ID_EXT_1,
    // Add more sensor IDs as needed
    HUMCTRL_SENSOR_COUNT // Total number of configured sensors
} HumCtrl_Id_t;

// --- External Declaration of Sensor Configurations Array ---
extern const HumCtrl_Config_t humctrl_sensor_configs[HUMCTRL_SENSOR_COUNT];

// --- Periodic Control Settings for HumCtrl_MainFunction() ---
#define HUMCTRL_READ_PERIOD_MS             1000 // HumCtrl_MainFunction called every 1 second

#endif // HUMCTRL_CFG_H