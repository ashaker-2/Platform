// app/inc/light_indication.h

#ifndef LIGHT_INDICATION_H
#define LIGHT_INDICATION_H

#include <stdint.h>
#include <stdbool.h>
#include "app_common.h"  // For APP_OK/APP_ERROR
#include "ecual_gpio.h"  // For ECUAL_GPIO_ID_t and ECUAL_GPIO_State_t

/**
 * @brief User-friendly IDs for configured Light Indication (LED) instances.
 */
typedef enum LIGHT_INDICATION_ID_t {
    LIGHT_INDICATION_SYSTEM_STATUS = 0, ///< LED to indicate general system operation (e.g., heartbeat)
    LIGHT_INDICATION_ERROR,             ///< LED to indicate an error state
    LIGHT_INDICATION_WIFI_STATUS,       ///< LED to indicate WiFi connection status
    LIGHT_INDICATION_TOTAL_UNITS        ///< Keep this last to get the count
} LIGHT_INDICATION_ID_t;

/**
 * @brief Structure to hold the configuration for a single Light Indication unit.
 */
typedef struct LIGHT_INDICATION_Config_t {
    ECUAL_GPIO_ID_t         gpio_id;      ///< The ECUAL GPIO ID connected to the LED.
    ECUAL_GPIO_State_t      active_state; ///< The GPIO state (HIGH or LOW) that turns the LED ON.
} LIGHT_INDICATION_Config_t;

/**
 * @brief Initializes all Light Indication units based on their configurations.
 * This sets up the underlying GPIOs to their initial (OFF) states.
 * @return APP_OK if all indicators are initialized successfully, APP_ERROR otherwise.
 */
uint8_t LIGHT_INDICATION_Init(void);

/**
 * @brief Turns a specific Light Indication unit ON.
 * @param unit_id The LIGHT_INDICATION_ID_t of the LED unit to control.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_INDICATION_On(LIGHT_INDICATION_ID_t unit_id);

/**
 * @brief Turns a specific Light Indication unit OFF.
 * @param unit_id The LIGHT_INDICATION_ID_t of the LED unit to control.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_INDICATION_Off(LIGHT_INDICATION_ID_t unit_id);

/**
 * @brief Toggles the state of a specific Light Indication unit.
 * @param unit_id The LIGHT_INDICATION_ID_t of the LED unit to control.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_INDICATION_Toggle(LIGHT_INDICATION_ID_t unit_id);

/**
 * @brief Gets the current ON/OFF state of a specific Light Indication unit.
 * @param unit_id The LIGHT_INDICATION_ID_t of the LED unit.
 * @param is_on Pointer to a boolean variable to store the state (true if ON, false if OFF).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_INDICATION_GetState(LIGHT_INDICATION_ID_t unit_id, bool *is_on);

#endif /* LIGHT_INDICATION_H */// app/inc/light_indication.h

#ifndef LIGHT_INDICATION_H
#define LIGHT_INDICATION_H

#include <stdint.h>
#include <stdbool.h>
#include "app_common.h"  // For APP_OK/APP_ERROR
#include "ecual_gpio.h"  // For ECUAL_GPIO_ID_t and ECUAL_GPIO_State_t

/**
 * @brief User-friendly IDs for configured Light Indication (LED) instances.
 */
typedef enum LIGHT_INDICATION_ID_t {
    LIGHT_INDICATION_SYSTEM_STATUS = 0, ///< LED to indicate general system operation (e.g., heartbeat)
    LIGHT_INDICATION_ERROR,             ///< LED to indicate an error state
    LIGHT_INDICATION_WIFI_STATUS,       ///< LED to indicate WiFi connection status
    LIGHT_INDICATION_TOTAL_UNITS        ///< Keep this last to get the count
} LIGHT_INDICATION_ID_t;

/**
 * @brief Structure to hold the configuration for a single Light Indication unit.
 */
typedef struct LIGHT_INDICATION_Config_t {
    ECUAL_GPIO_ID_t         gpio_id;      ///< The ECUAL GPIO ID connected to the LED.
    ECUAL_GPIO_State_t      active_state; ///< The GPIO state (HIGH or LOW) that turns the LED ON.
} LIGHT_INDICATION_Config_t;

/**
 * @brief Initializes all Light Indication units based on their configurations.
 * This sets up the underlying GPIOs to their initial (OFF) states.
 * @return APP_OK if all indicators are initialized successfully, APP_ERROR otherwise.
 */
uint8_t LIGHT_INDICATION_Init(void);

/**
 * @brief Turns a specific Light Indication unit ON.
 * @param unit_id The LIGHT_INDICATION_ID_t of the LED unit to control.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_INDICATION_On(LIGHT_INDICATION_ID_t unit_id);

/**
 * @brief Turns a specific Light Indication unit OFF.
 * @param unit_id The LIGHT_INDICATION_ID_t of the LED unit to control.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_INDICATION_Off(LIGHT_INDICATION_ID_t unit_id);

/**
 * @brief Toggles the state of a specific Light Indication unit.
 * @param unit_id The LIGHT_INDICATION_ID_t of the LED unit to control.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_INDICATION_Toggle(LIGHT_INDICATION_ID_t unit_id);

/**
 * @brief Gets the current ON/OFF state of a specific Light Indication unit.
 * @param unit_id The LIGHT_INDICATION_ID_t of the LED unit.
 * @param is_on Pointer to a boolean variable to store the state (true if ON, false if OFF).
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t LIGHT_INDICATION_GetState(LIGHT_INDICATION_ID_t unit_id, bool *is_on);

#endif /* LIGHT_INDICATION_H */
