/* ============================================================================
 * SOURCE FILE: Application/ledCtrl/inc/led_ctrl_cfg.h
 * ============================================================================*/
/**
 * @file led_ctrl_cfg.h
 * @brief Configuration definitions for the Led Control (LedCtrl) module.
 *
 * This file declares the structure for individual led configurations,
 * allowing control via either direct GPIO or an I/O expander.
 */

#ifndef LED_CTRL_CFG_H
#define LED_CTRL_CFG_H

#include <stdint.h> // For uint8_t
#include <stddef.h> // For size_t

/**
 * @brief Enumeration for unique Led IDs.
 * These IDs are used to reference specific leds within the Led Control module.
 */
typedef enum {
    LED_ID_1 = 0,   ///< First led
    LED_ID_2,       ///< Second led
    LED_ID_3,       ///< Third led
    LED_ID_4,       ///< Fourth led
    LED_ID_5,       ///< Fourth led
    LED_ID_COUNT    ///< Total number of leds configured
} Led_ID_t;

/**
 * @brief Enumeration for led states.
 */
typedef enum {
    LED_STATE_OFF = 0, ///< Led is off
    LED_STATE_ON,      ///< Led is on
    LED_STATE_INVALID  ///< Invalid led state
} Led_State_t;

/**
 * @brief Enumeration for the type of control mechanism for a led.
 */
typedef enum {
    LED_CONTROL_TYPE_IO_EXPANDER = 0, ///< Controlled via CH423S I/O expander
    LED_CONTROL_TYPE_GPIO,            ///< Controlled via direct ESP32 GPIO pin
    LED_CONTROL_TYPE_COUNT            ///< Total number of control types
} Led_Control_Type_t;

/**
 * @brief Structure to hold the configuration for a single led.
 *
 * This includes the control type and the specific pin number.
 * The interpretation of `pinNum` depends on `control_type`.
 */
typedef struct {
    Led_ID_t led_id;                   ///< Unique identifier for the led
    Led_Control_Type_t control_type;   ///< How this led is controlled
    uint8_t pinNum;                    ///< The pin number (either CH423S GP pin or direct GPIO pin)
    Led_State_t initial_state;         ///< Initial state of the led (ON/OFF)
} led_config_item_t;

/**
 * @brief External declaration of the array containing all predefined led configurations.
 * This array is defined in led_ctrl_cfg.c and accessed by led_ctrl.c for initialization.
 */
extern const led_config_item_t s_led_configurations[];

/**
 * @brief External declaration of the number of elements in the led configurations array.
 */
extern const size_t s_num_led_configurations;

#endif /* LED_CTRL_CFG_H */
