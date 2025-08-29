/* ============================================================================
 * SOURCE FILE: Application/fanCtrl/inc/fan_ctrl_cfg.h
 * ============================================================================*/
/**
 * @file fan_ctrl_cfg.h
 * @brief Configuration definitions for the Fan Control (FanCtrl) module.
 *
 * This file declares the structure for individual fan configurations,
 * allowing control via either direct GPIO or an I/O expander.
 */

#ifndef FAN_CTRL_CFG_H
#define FAN_CTRL_CFG_H

#include <stdint.h> // For uint8_t
#include <stddef.h> // For size_t

/**
 * @brief Enumeration for unique Fan IDs.
 * These IDs are used to reference specific fans within the Fan Control module.
 */
typedef enum {
    FAN_ID_0,       ///< Second fan
    FAN_ID_1,       ///< Second fan
    FAN_ID_2,       ///< Third fan
    FAN_ID_3,       ///< Fourth fan
    FAN_ID_4,       ///< Fourth fan
    FAN_ID_COUNT    ///< Total number of fans configured
} Fan_ID_t;

#define FAN_ID_ALL (FAN_ID_COUNT)

/**
 * @brief Enumeration for fan states.
 */
typedef enum {
    FAN_STATE_OFF = 0, ///< Fan is off
    FAN_STATE_ON,      ///< Fan is on
    FAN_STATE_INVALID  ///< Invalid fan state
} Fan_State_t;

/**
 * @brief Enumeration for the type of control mechanism for a fan.
 */
typedef enum {
    FAN_CONTROL_TYPE_IO_EXPANDER = 0, ///< Controlled via CH423S I/O expander
    FAN_CONTROL_TYPE_GPIO,            ///< Controlled via direct ESP32 GPIO pin
    FAN_CONTROL_TYPE_COUNT            ///< Total number of control types
} Fan_Control_Type_t;

/**
 * @brief Structure to hold the configuration for a single fan.
 *
 * This includes the control type and the specific pin number.
 * The interpretation of `pinNum` depends on `control_type`.
 */
typedef struct {
    Fan_ID_t fan_id;                   ///< Unique identifier for the fan
    Fan_Control_Type_t control_type;   ///< How this fan is controlled
    uint8_t pinNum;                    ///< The pin number (either CH423S GP pin or direct GPIO pin)
    Fan_State_t initial_state;         ///< Initial state of the fan (ON/OFF)
} fan_config_item_t;

/**
 * @brief External declaration of the array containing all predefined fan configurations.
 * This array is defined in fan_ctrl_cfg.c and accessed by fan_ctrl.c for initialization.
 */
extern const fan_config_item_t s_fan_configurations[];

/**
 * @brief External declaration of the number of elements in the fan configurations array.
 */
extern const size_t s_num_fan_configurations;

#endif /* FAN_CTRL_CFG_H */
