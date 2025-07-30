// app/inc/fan.h

#ifndef FAN_H
#define FAN_H

#include <stdint.h>
#include <stdbool.h>
#include "app_common.h"     // For APP_OK/APP_ERROR
#include "ecual_pwm.h"      // For ECUAL_PWM_Channel_ID_t (if fan uses PWM)
#include "ecual_gpio.h"     // For ECUAL_GPIO_ID_t (for ON/OFF or enable pin)

/**
 * @brief User-friendly IDs for configured Fan instances.
 * Add an entry here for each physical fan you configure.
 */
typedef enum FAN_ID_t {
    FAN_MOTOR_1 = 0,    ///< First Fan Motor (PWM Controlled)
    FAN_COOLING_FAN,    ///< A simple ON/OFF Cooling Fan
    FAN_TOTAL_FANS      ///< Keep this last to get the count
} FAN_ID_t;

/**
 * @brief Enum to define the type of fan control.
 */
typedef enum FAN_Type_t {
    FAN_TYPE_ON_OFF = 0,          ///< Fan is controlled purely by turning a GPIO ON/OFF.
    FAN_TYPE_PWM_SPEED_CONTROL    ///< Fan speed is controlled via a PWM signal.
} FAN_Type_t;

/**
 * @brief Structure to hold the configuration for a single Fan.
 */
typedef struct FAN_Config_t {
    FAN_Type_t                fan_type;                 ///< The type of control for this fan.

    // Common to both types (or serves as primary control for ON/OFF)
    ECUAL_GPIO_ID_t           control_gpio_id;          ///< The GPIO ID used for ON/OFF control (FAN_TYPE_ON_OFF)
                                                        ///< OR the enable/disable GPIO for PWM fans (FAN_TYPE_PWM_SPEED_CONTROL)
    bool                      has_aux_gpio_control;     ///< True if control_gpio_id is used as an auxiliary enable for PWM fans.
                                                        ///< (For FAN_TYPE_ON_OFF, this is implicitly true for primary control).

    // Parameters specific to FAN_TYPE_ON_OFF
    ECUAL_GPIO_State_t        on_off_gpio_active_state; ///< What state (HIGH/LOW) means "ON" for ON/OFF GPIO fans.

    // Parameters specific to FAN_TYPE_PWM_SPEED_CONTROL
    ECUAL_PWM_Channel_ID_t    pwm_channel_id;           ///< The ECUAL PWM channel ID controlling this fan's speed.
    uint32_t                  min_speed_duty_percent;   ///< Minimum effective PWM duty cycle percentage (0-100) for PWM fans.
                                                        ///< Below this, fan may not spin or might stall.
    uint32_t                  max_speed_duty_percent;   ///< Maximum effective PWM duty cycle percentage (0-100) for PWM fans.
                                                        ///< Can be less than 100 if fan makes too much noise at full power.
} FAN_Config_t;

/**
 * @brief Initializes all Fan modules based on the configurations defined in fan_config.h.
 * This also initializes the underlying ECUAL PWM and GPIO channels used by the fans.
 * @return APP_OK if all fans are initialized successfully, APP_ERROR otherwise.
 */
uint8_t FAN_Init(void);

/**
 * @brief Sets the speed of a specific fan.
 * For PWM fans: The desired fan speed as a percentage (0-100). 0 means off. Values will be clipped.
 * For ON/OFF fans: 0 means OFF, any value > 0 means ON.
 * @param fan_id The FAN_ID_t of the fan to control.
 * @param speed_percent The desired fan speed as a percentage (0-100).
 * @return APP_OK if successful, APP_ERROR otherwise (e.g., invalid fan_id).
 */
uint8_t FAN_SetSpeed(FAN_ID_t fan_id, uint8_t speed_percent);

/**
 * @brief Turns a specific fan ON.
 * For PWM fans: Sets to its configured minimum speed.
 * For ON/OFF fans: Activates the ON state.
 * @param fan_id The FAN_ID_t of the fan to start.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t FAN_Start(FAN_ID_t fan_id);

/**
 * @brief Turns a specific fan OFF.
 * For PWM fans: Sets speed to 0.
 * For ON/OFF fans: Deactivates the ON state.
 * @param fan_id The FAN_ID_t of the fan to stop.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t FAN_Stop(FAN_ID_t fan_id);

/**
 * @brief Gets the current commanded speed percentage of a specific fan.
 * Note: This returns the *commanded* speed, not necessarily actual measured RPM.
 * For ON/OFF fans, it returns 0 if OFF, 100 if ON.
 * @param fan_id The FAN_ID_t of the fan.
 * @param speed_percent Pointer to a uint8_t variable to store the speed.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t FAN_GetSpeed(FAN_ID_t fan_id, uint8_t *speed_percent);

#endif /* FAN_H */
