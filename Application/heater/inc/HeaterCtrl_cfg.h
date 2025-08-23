/* ============================================================================
 * SOURCE FILE: Application/heaterCtrl/inc/heater_ctrl_cfg.h
 * ============================================================================*/
/**
 * @file heater_ctrl_cfg.h
 * @brief Configuration definitions for the Heater Control (HeaterCtrl) module.
 *
 * This file declares the structure for individual heater configurations,
 * allowing control via either direct GPIO or an I/O expander.
 */

#ifndef HEATER_CTRL_CFG_H
#define HEATER_CTRL_CFG_H

#include <stdint.h> // For uint8_t
#include <stddef.h> // For size_t

/**
 * @brief Enumeration for unique Heater IDs.
 * These IDs are used to reference specific heaters within the Heater Control module.
 */
typedef enum {
    HEATER_ID_1 = 0,   ///< First heater
    HEATER_ID_COUNT    ///< Total number of heaters configured
} Heater_ID_t;

/**
 * @brief Enumeration for heater states.
 */
typedef enum {
    HEATER_STATE_OFF = 0, ///< Heater is off
    HEATER_STATE_ON,      ///< Heater is on
    HEATER_STATE_INVALID  ///< Invalid heater state
} Heater_State_t;

/**
 * @brief Enumeration for the type of control mechanism for a heater.
 */
typedef enum {
    HEATER_CONTROL_TYPE_IO_EXPANDER = 0, ///< Controlled via CH423S I/O expander
    HEATER_CONTROL_TYPE_GPIO,            ///< Controlled via direct ESP32 GPIO pin
    HEATER_CONTROL_TYPE_COUNT            ///< Total number of control types
} Heater_Control_Type_t;

/**
 * @brief Structure to hold the configuration for a single heater.
 *
 * This includes the control type and the specific pin number.
 * The interpretation of `pinNum` depends on `control_type`.
 */
typedef struct {
    Heater_ID_t heater_id;                   ///< Unique identifier for the heater
    Heater_Control_Type_t control_type;   ///< How this heater is controlled
    uint8_t pinNum;                    ///< The pin number (either CH423S GP pin or direct GPIO pin)
    Heater_State_t initial_state;         ///< Initial state of the heater (ON/OFF)
} heater_config_item_t;

/**
 * @brief External declaration of the array containing all predefined heater configurations.
 * This array is defined in heater_ctrl_cfg.c and accessed by heater_ctrl.c for initialization.
 */
extern const heater_config_item_t s_heater_configurations[];

/**
 * @brief External declaration of the number of elements in the heater configurations array.
 */
extern const size_t s_num_heater_configurations;

#endif /* HEATER_CTRL_CFG_H */
