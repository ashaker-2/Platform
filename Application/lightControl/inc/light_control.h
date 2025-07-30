// app/inc/light_control.h

#ifndef LIGHT_CONTROL_H
#define LIGHT_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "app_common.h"  // For APP_OK/APP_ERROR
#include "ecual_gpio.h"  // For ECUAL_GPIO_ID_t and ECUAL_GPIO_State_t
#include "ecual_pwm.h"   // For ECUAL_PWM_CHANNEL_t

/**
 * @brief User-friendly IDs for configured Light Control unit instances.
 */
typedef enum LIGHT_ID_t {
    LIGHT_KITCHEN = 0,      ///< Kitchen light (ON/OFF type)
    LIGHT_LIVING_ROOM,      ///< Living room light (Dimmable type)
    LIGHT_BEDROOM,          ///< Bedroom light (ON/OFF type)
    LIGHT_TOTAL_UNITS       ///< Keep this last to get the count
} LIGHT_ID_t;

/**
 * @brief Defines the type of light control.
 */
typedef enum LIGHT_Type_t {
    LIGHT_TYPE_ON_OFF,      ///< Simple ON/OFF control using GPIO.
    LIGHT_TYPE_DIMMABLE     ///< Dimmable control using PWM.
} LIGHT_Type_t;

/**
 * @brief Structure to hold the configuration for a single Light Control unit.
 * Uses a union to store type-specific parameters efficiently.
 */
typedef struct LIGHT_Config_t {
    LIGHT_Type_t    type; ///< Specifies if the light is ON/OFF or Dimmable.
    union {
        // Configuration for ON/OFF type lights
        struct {
            ECUAL_GPIO_ID_t     gpio_id;      ///< The ECUAL GPIO ID connected to the light's control.
            ECUAL_GPIO_State_t  active_state; ///< The GPIO state (HIGH or LOW) that turns the light ON.
        } on_off_cfg;

        // Configuration for Dimmable type lights
        struct {
            ECUAL_PWM_CHANNEL_t pwm_channel; ///< The ECUAL PWM channel for dimming.
            // Frequency and resolution are typically defined per PWM timer in ECUAL_PWM_Config.
        } dimmable_cfg;
    };
} LIGHT_Config_t;


/**
 * @brief Initializes all Light Control units based on their configurations.
 * Sets up underlying GPIOs/PWMs to their initial (OFF/0% brightness) states.
 * @return APP_OK if all lights are initialized successfully, APP_ERROR otherwise.
 */
uint8_t LIGHT_Init(void);

/**
 * @brief Turns a specific Light unit fully ON (100% brightness for dimmable).
 * @param unit_id The LIGHT_ID_t of the light unit to control.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_On(LIGHT_ID_t unit_id);

/**
 * @brief Turns a specific Light unit fully OFF (0% brightness for dimmable).
 * @param unit_id The LIGHT_ID_t of the light unit to control.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_Off(LIGHT_ID_t unit_id);

/**
 * @brief Sets the brightness of a dimmable light.
 * This function is only applicable for lights configured as LIGHT_TYPE_DIMMABLE.
 * For ON/OFF lights, it will return APP_ERROR.
 * @param unit_id The LIGHT_ID_t of the light unit.
 * @param percentage The desired brightness in percentage (0 to 100).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_SetBrightness(LIGHT_ID_t unit_id, uint8_t percentage);

/**
 * @brief Gets the current commanded ON/OFF state of a specific Light unit.
 * For dimmable lights, this is true if brightness > 0%.
 * @param unit_id The LIGHT_ID_t of the light unit.
 * @param is_on Pointer to a boolean variable to store the commanded state (true if commanded ON, false if OFF).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_GetCommandedState(LIGHT_ID_t unit_id, bool *is_on);

/**
 * @brief Gets the current commanded brightness percentage of a specific Light unit.
 * For ON/OFF lights, this will return 0 if OFF, and 100 if ON.
 * @param unit_id The LIGHT_ID_t of the light unit.
 * @param percentage Pointer to a uint8_t variable to store the brightness (0-100).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_GetBrightness(LIGHT_ID_t unit_id, uint8_t *percentage);

#endif /* LIGHT_CONTROL_H */
