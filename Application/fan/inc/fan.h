// app/inc/fan.h

#ifndef FAN_H // Renamed include guard
#define FAN_H

#include <stdint.h>
#include <stdbool.h>
#include "app_common.h"     // For APP_OK/APP_ERROR
#include "ecual_pwm.h"      // For ECUAL_PWM_Channel_ID_t (if fan uses PWM)
#include "ecual_gpio.h"     // For ECUAL_GPIO_ID_t (if fan has enable pin)

/**
 * @brief User-friendly IDs for configured Fan instances.
 * Add an entry here for each physical fan you configure.
 */
typedef enum FAN_ID_t { // Renamed enum
    FAN_MOTOR_1 = 0,    ///< First Fan Motor
    // FAN_MOTOR_2,       // Uncomment if you have more fans
    FAN_TOTAL_FANS      ///< Keep this last to get the count // Renamed enum member
} FAN_ID_t;

/**
 * @brief Structure to hold the configuration for a single Fan.
 */
typedef struct FAN_Config_t { // Renamed struct
    ECUAL_PWM_Channel_ID_t    pwm_channel_id;       ///< The ECUAL PWM channel ID controlling this fan.
    bool                      has_enable_gpio;      ///< True if there's a separate GPIO for fan enable.
    ECUAL_GPIO_ID_t           enable_gpio_id;       ///< The ECUAL GPIO ID for the enable pin (if has_enable_gpio is true).
    uint32_t                  min_speed_duty_percent; ///< Minimum effective speed percentage (0-100)
                                                    ///< Below this, fan may not spin or might stall.
    uint32_t                  max_speed_duty_percent; ///< Maximum effective speed percentage (0-100)
                                                    ///< Can be less than 100 if fan makes too much noise at full power.
} FAN_Config_t;

/**
 * @brief Initializes all Fan modules based on the configurations defined in fan_config.h.
 * This also initializes the underlying ECUAL PWM and GPIO channels used by the fans.
 * @return APP_OK if all fans are initialized successfully, APP_ERROR otherwise.
 */
uint8_t FAN_Init(void); // Renamed function

/**
 * @brief Sets the speed of a specific fan.
 * @param fan_id The FAN_ID_t of the fan to control.
 * @param speed_percent The desired fan speed as a percentage (0-100).
 * 0 means off. Values will be clipped between min/max_speed_duty_percent.
 * @return APP_OK if successful, APP_ERROR otherwise (e.g., invalid fan_id).
 */
uint8_t FAN_SetSpeed(FAN_ID_t fan_id, uint8_t speed_percent); // Renamed function

/**
 * @brief Turns a specific fan ON (sets to its configured minimum speed).
 * This is an alias for SetSpeed(min_speed_duty_percent).
 * @param fan_id The FAN_ID_t of the fan to start.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t FAN_Start(FAN_ID_t fan_id); // Renamed function

/**
 * @brief Turns a specific fan OFF (sets speed to 0 and disables enable GPIO if applicable).
 * @param fan_id The FAN_ID_t of the fan to stop.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t FAN_Stop(FAN_ID_t fan_id); // Renamed function

/**
 * @brief Gets the current configured speed percentage of a specific fan.
 * Note: This returns the *commanded* speed, not necessarily actual measured RPM.
 * @param fan_id The FAN_ID_t of the fan.
 * @param speed_percent Pointer to a uint8_t variable to store the speed.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t FAN_GetSpeed(FAN_ID_t fan_id, uint8_t *speed_percent); // Renamed function

#endif /* FAN_H */
