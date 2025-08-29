/* ============================================================================
 * SOURCE FILE: Application/VenCtrl/inc/Ven_ctrl_cfg.h
 * ============================================================================*/
/**
 * @file Ven_ctrl_cfg.h
 * @brief Configuration definitions for the Ven Control (VenCtrl) module.
 *
 * This file declares the structure for individual Ven configurations,
 * allowing control via either direct GPIO or an I/O expander.
 */

#ifndef VEN_CTRL_CFG_H
#define VEN_CTRL_CFG_H

#include <stdint.h> // For uint8_t
#include <stddef.h> // For size_t

/**
 * @brief Enumeration for unique Ven IDs.
 * These IDs are used to reference specific Vens within the Ven Control module.
 */
typedef enum {
    VEN_ID_0 = 0,   ///< First Ven
    VEN_ID_1,   ///< First Ven
    VEN_ID_2,       ///< Second Ven
    VEN_ID_3,       ///< Third Ven
    VEN_ID_4,       ///< Fourth Ven
    VEN_ID_COUNT    ///< Total number of Vens configured
} Ven_ID_t;

#define VEN_ID_ALL (VEN_ID_COUNT)

/**
 * @brief Enumeration for Ven states.
 */
typedef enum {
    VEN_STATE_OFF = 0, ///< Ven is off
    VEN_STATE_ON,      ///< Ven is on
    VEN_STATE_INVALID  ///< Invalid Ven state
} Ven_State_t;

/**
 * @brief Enumeration for the type of control mechanism for a Ven.
 */
typedef enum {
    VEN_CONTROL_TYPE_IO_EXPANDER = 0, ///< Controlled via CH423S I/O expander
    VEN_CONTROL_TYPE_GPIO,            ///< Controlled via direct ESP32 GPIO pin
    VEN_CONTROL_TYPE_COUNT            ///< Total number of control types
} Ven_Control_Type_t;

/**
 * @brief Structure to hold the configuration for a single Ven.
 *
 * This includes the control type and the specific pin number.
 * The interpretation of `pinNum` depends on `control_type`.
 */
typedef struct {
    Ven_ID_t ven_id;                   ///< Unique identifier for the Ven
    Ven_Control_Type_t control_type;   ///< How this Ven is controlled
    uint8_t pinNum;                    ///< The pin number (either CH423S GP pin or direct GPIO pin)
    Ven_State_t initial_state;         ///< Initial state of the Ven (ON/OFF)
} ven_config_item_t;

/**
 * @brief External declaration of the array containing all predefined Ven configurations.
 * This array is defined in Ven_ctrl_cfg.c and accessed by Ven_ctrl.c for initialization.
 */
extern const ven_config_item_t s_ven_configurations[];

/**
 * @brief External declaration of the number of elements in the Ven configurations array.
 */
extern const size_t s_num_ven_configurations;

#endif /* VEN_CTRL_CFG_H */
