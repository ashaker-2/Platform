/* ============================================================================
 * SOURCE FILE: Application/pumpCtrl/inc/pump_ctrl_cfg.h
 * ============================================================================*/
/**
 * @file pump_ctrl_cfg.h
 * @brief Configuration definitions for the Pump Control (PumpCtrl) module.
 *
 * This file declares the structure for individual pump configurations,
 * allowing control via either direct GPIO or an I/O expander.
 */

#ifndef PUMP_CTRL_CFG_H
#define PUMP_CTRL_CFG_H

#include <stdint.h> // For uint8_t
#include <stddef.h> // For size_t

/**
 * @brief Enumeration for unique Pump IDs.
 * These IDs are used to reference specific pumps within the Pump Control module.
 */
typedef enum
{
    PUMP_ID_0 = 0, ///< First pump
    PUMP_ID_COUNT  ///< Total number of pumps configured
} Pump_ID_t;

#define PUMP_ID_ALL (PUMP_ID_COUNT)

/**
 * @brief Enumeration for pump states.
 */
typedef enum
{
    PUMP_STATE_OFF = 0, ///< Pump is off
    PUMP_STATE_ON,      ///< Pump is on
    PUMP_STATE_INVALID  ///< Invalid pump state
} Pump_State_t;

/**
 * @brief Enumeration for the type of control mechanism for a pump.
 */
typedef enum
{
    PUMP_CONTROL_TYPE_IO_EXPANDER = 0, ///< Controlled via CH423S I/O expander
    PUMP_CONTROL_TYPE_GPIO,            ///< Controlled via direct ESP32 GPIO pin
    PUMP_CONTROL_TYPE_COUNT            ///< Total number of control types
} Pump_Control_Type_t;

/**
 * @brief Structure to hold the configuration for a single pump.
 *
 * This includes the control type and the specific pin number.
 * The interpretation of `pinNum` depends on `control_type`.
 */
typedef struct
{
    Pump_ID_t pump_id;                ///< Unique identifier for the pump
    Pump_Control_Type_t control_type; ///< How this pump is controlled
    uint8_t pinNum;                   ///< The pin number (either CH423S GP pin or direct GPIO pin)
    Pump_State_t initial_state;       ///< Initial state of the pump (ON/OFF)
} pump_config_item_t;

/**
 * @brief External declaration of the array containing all predefined pump configurations.
 * This array is defined in pump_ctrl_cfg.c and accessed by pump_ctrl.c for initialization.
 */
extern const pump_config_item_t s_pump_configurations[];

/**
 * @brief External declaration of the number of elements in the pump configurations array.
 */
extern const size_t s_num_pump_configurations;

#endif /* PUMP_CTRL_CFG_H */
