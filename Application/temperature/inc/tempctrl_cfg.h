#ifndef TEMPCTRL_CFG_H
#define TEMPCTRL_CFG_H

#include "hal_adc.h"
#include "hal_i2c.h"

#include <stdint.h>

/**
 * @file tempctrl_cfg.h
 * @brief Configuration header for the TempCtrl component.
 *
 * This file defines all the necessary data types, macros, and external declarations
 * for configuring the TempCtrl module for a specific application.
 */

// --- Fault ID Definitions (from SystemMonitor) ---
// These are included here for direct use in the TempCtrl module
#define FAULT_ID_TEMP_SENSOR_INIT_FAILURE       FAULT_ID_TEMP_SENSOR_FAILURE
#define FAULT_ID_TEMP_SENSOR_COMM_ERROR         FAULT_ID_TEMP_SENSOR_FAILURE
#define FAULT_ID_TEMP_SENSOR_OUT_OF_RANGE       FAULT_ID_TEMP_SENSOR_FAILURE

// --- Sensor Type Enums ---
typedef enum {
    TempCtrl_SENSOR_TYPE_ANALOG_ADC,
    TempCtrl_SENSOR_TYPE_I2C,
    TempCtrl_SENSOR_TYPE_1WIRE_DS18B20
    // Add more types as needed
} TempCtrl_SensorType_t;

// --- Sensor Communication Details Union ---
/**
 * @brief A union to store specific communication parameters for each sensor type.
 */
typedef union {
    struct {
        uint8_t adc_channel;
    } analog_adc;
    struct {
        uint8_t i2c_port;
        uint8_t i2c_address;
    } i2c;
    struct {
        uint8_t one_wire_bus_id;
        uint8_t ds18b20_rom_address[8];
    } one_wire;
    // Add more structs for other sensor types
} TempCtrl_SensorCommDetails_t;

// --- Sensor Configuration Structure ---
/**
 * @brief Structure to hold the configuration for a single temperature sensor.
 */
typedef struct {
    uint32_t id;
    TempCtrl_SensorType_t type;
    TempCtrl_SensorCommDetails_t comm_details;
    float calibration_slope;
    float calibration_offset;
    uint32_t read_delay_ms;
} TempCtrl_SensorConfig_t;

// --- Sensor IDs ---
typedef enum {
    TempCtrl_SENSOR_ID_ROOM = 0,
    TempCtrl_SENSOR_ID_OUTDOOR,
    TempCtrl_SENSOR_ID_HEATER_EXHAUST,
    // Add more sensor IDs as needed
    TempCtrl_SENSOR_COUNT // Total number of configured sensors
} TempCtrl_SensorId_t;

// --- External Declaration of Sensor Configurations Array ---
// This array is defined in tempctrl_cfg.c and holds all sensor configurations.
extern const TempCtrl_SensorConfig_t tempctrl_sensor_configs[TempCtrl_SENSOR_COUNT];

// --- Periodic Read Settings for TempCtrl_MainFunction() ---
#define TempCtrl_READ_PERIOD_MS             1000 // TempCtrl_MainFunction called every 1 second
#define TempCtrl_READ_RETRIES               3    // Number of times to retry a read on failure per sensor
#define TempCtrl_READ_RETRY_DELAY_MS        100  // Delay between retries (if sensor allows)

// --- Valid temperature range for sanity checks ---
#define TempCtrl_MIN_VALID_C                -20.0f
#define TempCtrl_MAX_VALID_C                100.0f

#endif // TEMPCTRL_CFG_H