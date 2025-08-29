/* ============================================================================
 * SOURCE FILE: Application/lightCtrl/inc/light_ctrl_cfg.h
 * ============================================================================*/
/**
 * @file light_ctrl_cfg.h
 * @brief Configuration definitions for the Light Control (LightCtrl) module.
 *
 * This file declares the structure for individual light configurations,
 * allowing control via either direct GPIO or an I/O expander.
 */

#ifndef LIGHT_CTRL_CFG_H
#define LIGHT_CTRL_CFG_H

#include <stdint.h> // For uint8_t
#include <stddef.h> // For size_t

/**
 * @brief Enumeration for unique Light IDs.
 * These IDs are used to reference specific lights within the Light Control module.
 */
typedef enum {
    LIGHT_ID_0 = 0,   ///< First light
    LIGHT_ID_1,   ///< First light
    LIGHT_ID_COUNT    ///< Total number of lights configured
} Light_ID_t;

#define Light_ID_ALL (LIGHT_ID_COUNT)


/**
 * @brief Enumeration for light states.
 */
typedef enum {
    LIGHT_STATE_OFF = 0, ///< Light is off
    LIGHT_STATE_ON,      ///< Light is on
    LIGHT_STATE_INVALID  ///< Invalid light state
} Light_State_t;

/**
 * @brief Enumeration for the type of control mechanism for a light.
 */
typedef enum {
    LIGHT_CONTROL_TYPE_IO_EXPANDER = 0, ///< Controlled via CH423S I/O expander
    LIGHT_CONTROL_TYPE_GPIO,            ///< Controlled via direct ESP32 GPIO pin
    LIGHT_CONTROL_TYPE_COUNT            ///< Total number of control types
} Light_Control_Type_t;

/**
 * @brief Structure to hold the configuration for a single light.
 *
 * This includes the control type and the specific pin number.
 * The interpretation of `pinNum` depends on `control_type`.
 */
typedef struct {
    Light_ID_t light_id;                   ///< Unique identifier for the light
    Light_Control_Type_t control_type;   ///< How this light is controlled
    uint8_t pinNum;                    ///< The pin number (either CH423S GP pin or direct GPIO pin)
    Light_State_t initial_state;         ///< Initial state of the light (ON/OFF)
} light_config_item_t;

/**
 * @brief External declaration of the array containing all predefined light configurations.
 * This array is defined in light_ctrl_cfg.c and accessed by light_ctrl.c for initialization.
 */
extern const light_config_item_t s_light_configurations[];

/**
 * @brief External declaration of the number of elements in the light configurations array.
 */
extern const size_t s_num_light_configurations;

#endif /* LIGHT_CTRL_CFG_H */
