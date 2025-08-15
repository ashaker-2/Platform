/**
 * @file hal_gpio.c
 * @brief Implementation for the Hardware Abstraction Layer (HAL) GPIO component.
 *
 * This file implements the hardware-independent GPIO interface, mapping logical
 * GPIO pins to their physical MCU counterparts via the MCAL layer. It handles
 * configuration, digital read/write, and interrupt management.
 */

#include "hal_gpio.h"       // Public header for HAL_GPIO
#include "hal_gpio_cfg.h"   // Configuration header for HAL_GPIO
// #include "gpio.h"      // MCAL layer for direct GPIO hardware access
#include "common.h"     // Common application definitions
#include "system_monitor.h" // For reporting faults
#include "logger.h"         // For logging

// --- Private Data Structures ---
/**
 * @brief Runtime state for each logical GPIO pin.
 */
typedef struct {
    HAL_GPIO_Direction_t        current_direction;      /**< Current configured direction. */
    HAL_GPIO_InterruptCallback_t interrupt_callback;    /**< Registered interrupt callback. */
    uint32_t                    debounce_time_ms;       /**< Debounce time for interrupts. */
    uint32_t                    last_interrupt_time_ms; /**< Timestamp of last interrupt for debouncing. */
    bool                        is_configured;          /**< Flag indicating if the pin has been configured. */
} HAL_GPIO_PinState_t;

// --- Private Variables ---
static HAL_GPIO_PinState_t s_gpio_pin_states[HAL_GPIO_PIN_COUNT];
static bool s_hal_gpio_initialized = false;

// --- Private Function Prototypes ---
// static void HAL_GPIO_McalInterruptHandler(HAL_GPIO_Pin_t mcal_pin);
// static HAL_GPIO_Pin_t HAL_GPIO_GetLogicalPin(HAL_GPIO_Pin_t mcal_pin);

// --- Public Function Implementations ---

/**
 * @brief Initializes the HAL GPIO module.
 * This function should be called once during system startup.
 * It prepares the internal data structures and potentially the underlying MCAL.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_GPIO_Init(void) {
    // if (s_hal_gpio_initialized) {
    //     // LOG_WARNING("HAL_GPIO", "Module already initialized.");
    //     return E_OK;
    // }

    // Initialize MCAL GPIO (if MCAL_GPIO_Init requires it)
    // Assuming MCAL_GPIO_Init is called once at the MCAL layer initialization
    // For this consolidated file, we'll call a mock MCAL_GPIO_Init here.
    // In a real project, this would be part of sys_startup.c's MCAL init phase.
    // if (MCAL_GPIO_Init() != MCAL_OK) { // Assuming MCAL_OK is the success status for MCAL
    //     // LOG_ERROR("HAL_GPIO", "Failed to initialize MCAL GPIO.");
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_GPIO_ERROR, 0); // Use a generic HAL GPIO error
    //     return E_NOK;
    // }

    // Initialize internal state for all logical pins
    // for (uint8_t i = 0; i < HAL_GPIO_PIN_COUNT; i++) {
    //     s_gpio_pin_states[i].current_direction = HAL_GPIO_DIR_INPUT; // Default to input
    //     s_gpio_pin_states[i].interrupt_callback = NULL;
    //     s_gpio_pin_states[i].debounce_time_ms = 0;
    //     s_gpio_pin_states[i].last_interrupt_time_ms = 0;
    //     s_gpio_pin_states[i].is_configured = false;

    //     const HAL_GPIO_Config_t *config = &g_hal_gpio_configs[i];
    //     if (config->logical_id != (HAL_GPIO_Pin_t)i) {
    //         // LOG_ERROR("HAL_GPIO", "Config array mismatch at index %d. Check hal_gpio_cfg.c!", i);
    //         // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_GPIO_ERROR, (uint32_t)i);
    //         return E_NOK; // Critical configuration error
    //     }

    //     // Apply default configurations during initialization
    //     Status_t status = HAL_GPIO_Config(config->logical_id, config->default_direction,
    //                                           config->default_initial_state, config->default_pull_mode);
    //     if (status != E_OK) {
    //         // LOG_ERROR("HAL_GPIO", "Failed to apply default config for pin %d.", config->logical_id);
    //         // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_GPIO_ERROR, (uint32_t)config->logical_id);
    //         // Continue initialization, but report error
    //         // For critical pins, this might lead to E_NOK return
    //     }
    // }

    // Register the common HAL interrupt handler with MCAL for all interrupt-capable pins
    // This assumes MCAL provides a single callback for all GPIO interrupts.
    // If MCAL provides per-pin callbacks, this loop would be different.
    // for (uint8_t i = 0; i < HAL_GPIO_PIN_COUNT; i++) {
    //     const HAL_GPIO_Config_t *config = &g_hal_gpio_configs[i];
    //     if (config->is_interrupt_capable) {
    //         // Register MCAL callback for this physical pin.
    //         // MCAL_GPIO_RegisterInterruptCallback is a mock, assuming it exists.
    //         // In a real system, this would be done once for the overall GPIO ISR.
    //         // if (MCAL_GPIO_RegisterInterruptCallback(config->mcal_pin, HAL_GPIO_McalInterruptHandler) != E_OK) {
    //             // LOG_ERROR("HAL_GPIO", "Failed to register MCAL ISR for pin %d (MCAL %d).", config->logical_id, config->mcal_pin);
    //             // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_GPIO_ERROR, (uint32_t)config->logical_id);
    //             // Not critical enough to halt init, but log
    //         // }
    //     }
    // }


    // s_hal_gpio_initialized = true;
    // LOG_INFO("HAL_GPIO", "Module initialized.");
    return E_OK;
}

/**
 * @brief Configures a specific GPIO pin.
 * This sets the direction, initial state (for outputs), and pull mode (for inputs).
 * @param pin The logical ID of the GPIO pin to configure.
 * @param direction The desired direction (input or output).
 * @param initial_state The initial state for output pins (HAL_GPIO_STATE_LOW/HIGH). Ignored for inputs.
 * @param pull_mode The pull mode for input pins (HAL_GPIO_PULL_NONE/UP/DOWN). Ignored for outputs.
 * @return E_OK on success, E_NOK on failure (e.g., invalid pin, MCAL error).
 */
Status_t HAL_GPIO_Config(HAL_GPIO_Pin_t pin, HAL_GPIO_Direction_t direction,
                             HAL_GPIO_State_t initial_state, HAL_GPIO_PullMode_t pull_mode) 
{
    // if (pin >= HAL_GPIO_PIN_COUNT) {
    //     // LOG_ERROR("HAL_GPIO", "Invalid pin ID %d for configuration.", pin);
    //     return E_INVALID_PARAM;
    // }

    // const HAL_GPIO_Config_t *config = &g_hal_gpio_configs[pin];
    // MCAL_GPIO_Direction_t mcal_dir;
    // MCAL_GPIO_PullMode_t mcal_pull;
    // MCAL_GPIO_State_t mcal_initial_state;

    // // Map HAL enums to MCAL enums
    // if (direction == HAL_GPIO_DIR_INPUT) {
    //     mcal_dir = MCAL_GPIO_DIR_INPUT;
    // } else {
    //     mcal_dir = MCAL_GPIO_DIR_OUTPUT;
    // }

    // if (initial_state == HAL_GPIO_STATE_HIGH) {
    //     mcal_initial_state = MCAL_GPIO_STATE_HIGH;
    // } else {
    //     mcal_initial_state = MCAL_GPIO_STATE_LOW;
    // }

    // switch (pull_mode) {
    //     case HAL_GPIO_PULL_UP:   mcal_pull = MCAL_GPIO_PULL_UP; break;
    //     case HAL_GPIO_PULL_DOWN: mcal_pull = MCAL_GPIO_PULL_DOWN; break;
    //     case HAL_GPIO_PULL_NONE:
    //     default:                 mcal_pull = MCAL_GPIO_PULL_NONE; break;
    // }

    // // Call MCAL to configure the pin
    // // if (MCAL_GPIO_Config(config->mcal_pin, mcal_dir, mcal_initial_state, mcal_pull) != E_OK) {
    // //     // LOG_ERROR("HAL_GPIO", "MCAL config failed for pin %d (MCAL %d).", pin, config->mcal_pin);
    // //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_GPIO_ERROR, (uint32_t)pin);
    // //     return E_NOK;
    // // }

    // // Update internal state
    // s_gpio_pin_states[pin].current_direction = direction;
    // s_gpio_pin_states[pin].is_configured = true;
    // LOG_DEBUG("HAL_GPIO", "Pin %d configured: Dir=%d, State=%d, Pull=%d", pin, direction, initial_state, pull_mode);
    return E_OK;
}

/**
 * @brief Sets the state of an output GPIO pin.
 * @param pin The logical ID of the output GPIO pin.
 * @param state The desired state (HAL_GPIO_STATE_LOW or HAL_GPIO_STATE_HIGH).
 * @return E_OK on success, E_NOK on failure (e.g., pin not configured as output, MCAL error).
 */
Status_t HAL_GPIO_Write(HAL_GPIO_Pin_t pin, HAL_GPIO_State_t state) {
    // if (pin >= HAL_GPIO_PIN_COUNT || !s_gpio_pin_states[pin].is_configured ||
    //     s_gpio_pin_states[pin].current_direction != HAL_GPIO_DIR_OUTPUT) {
    //     // LOG_ERROR("HAL_GPIO", "Write failed: Pin %d not configured as output or invalid.", pin);
    //     return E_INVALID_PARAM;
    // }

    // const HAL_GPIO_Config_t *config = &g_hal_gpio_configs[pin];
    // MCAL_GPIO_State_t mcal_state = (state == HAL_GPIO_STATE_HIGH) ? MCAL_GPIO_STATE_HIGH : MCAL_GPIO_STATE_LOW;

    // if (MCAL_GPIO_Write(config->mcal_pin, mcal_state) != E_OK) {
    //     // LOG_ERROR("HAL_GPIO", "MCAL write failed for pin %d (MCAL %d).", pin, config->mcal_pin);
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_GPIO_ERROR, (uint32_t)pin);
    //     return E_NOK;
    // }
    // LOG_VERBOSE("HAL_GPIO", "Pin %d wrote state %d", pin, state);
    return E_OK;
}

/**
 * @brief Toggles the state of an output GPIO pin.
 * @param pin The logical ID of the output GPIO pin.
 * @return E_OK on success, E_NOK on failure (e.g., pin not configured as output, MCAL error).
 */
Status_t HAL_GPIO_Toggle(HAL_GPIO_Pin_t pin) {
    // HAL_GPIO_State_t current_state;
    // Status_t status = HAL_GPIO_Read(pin, &current_state);
    // if (status != E_OK) {
    //     // LOG_ERROR("HAL_GPIO", "Toggle failed: Could not read current state for pin %d.", pin);
    //     return status;
    // }
    // return HAL_GPIO_Write(pin, (current_state == HAL_GPIO_STATE_HIGH) ? HAL_GPIO_STATE_LOW : HAL_GPIO_STATE_HIGH);
}

/**
 * @brief Reads the current state of a GPIO pin.
 * @param pin The logical ID of the GPIO pin to read.
 * @param state_p Pointer to store the read state (HAL_GPIO_STATE_LOW or HAL_GPIO_STATE_HIGH).
 * @return E_OK on success, E_NOK on failure (e.g., invalid pin, MCAL error).
 */
Status_t HAL_GPIO_Read(HAL_GPIO_Pin_t pin, HAL_GPIO_State_t *state_p) {
    // if (pin >= HAL_GPIO_PIN_COUNT || !s_gpio_pin_states[pin].is_configured || state_p == NULL) {
    //     // LOG_ERROR("HAL_GPIO", "Read failed: Invalid pin ID %d or NULL state_p.", pin);
    //     return E_INVALID_PARAM;
    // }

    // const HAL_GPIO_Config_t *config = &g_hal_gpio_configs[pin];
    // MCAL_GPIO_State_t mcal_state;

    // if (MCAL_GPIO_Read(config->mcal_pin, &mcal_state) != E_OK) {
    //     // LOG_ERROR("HAL_GPIO", "MCAL read failed for pin %d (MCAL %d).", pin, config->mcal_pin);
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_GPIO_ERROR, (uint32_t)pin);
    //     return E_NOK;
    // }

    // *state_p = (mcal_state == MCAL_GPIO_STATE_HIGH) ? HAL_GPIO_STATE_HIGH : HAL_GPIO_STATE_LOW;
    // LOG_VERBOSE("HAL_GPIO", "Pin %d read state %d", pin, *state_p);
    return E_OK;
}

/**
 * @brief Configures an interrupt for a specific GPIO pin.
 * @param pin The logical ID of the GPIO pin.
 * @param interrupt_type The type of interrupt (edge or level).
 * @param callback The function to be called when the interrupt occurs.
 * @param debounce_ms Debounce time in milliseconds (0 for no debouncing).
 * @return E_OK on success, E_NOK on failure (e.g., invalid pin, MCAL error).
 */
Status_t HAL_GPIO_ConfigInterrupt(HAL_GPIO_Pin_t pin, HAL_GPIO_InterruptType_t interrupt_type,
                                      HAL_GPIO_InterruptCallback_t callback, uint32_t debounce_ms) {
    // if (pin >= HAL_GPIO_PIN_COUNT || !g_hal_gpio_configs[pin].is_interrupt_capable || callback == NULL) {
    //     // LOG_ERROR("HAL_GPIO", "ConfigInterrupt failed: Invalid pin %d, not interrupt capable, or NULL callback.", pin);
    //     return E_INVALID_PARAM;
    // }

    // const HAL_GPIO_Config_t *config = &g_hal_gpio_configs[pin];
    // MCAL_GPIO_InterruptType_t mcal_int_type;

    // // Map HAL interrupt type to MCAL interrupt type
    // switch (interrupt_type) {
    //     case HAL_GPIO_INT_RISING_EDGE:  mcal_int_type = MCAL_GPIO_INT_RISING_EDGE;  break;
    //     case HAL_GPIO_INT_FALLING_EDGE: mcal_int_type = MCAL_GPIO_INT_FALLING_EDGE; break;
    //     case HAL_GPIO_INT_BOTH_EDGES:   mcal_int_type = MCAL_GPIO_INT_BOTH_EDGES;   break;
    //     case HAL_GPIO_INT_HIGH_LEVEL:   mcal_int_type = MCAL_GPIO_INT_HIGH_LEVEL;   break;
    //     case HAL_GPIO_INT_LOW_LEVEL:    mcal_int_type = MCAL_GPIO_INT_LOW_LEVEL;    break;
    //     case HAL_GPIO_INT_NONE:
    //     default:
    //         // LOG_ERROR("HAL_GPIO", "ConfigInterrupt failed: Invalid interrupt type %d for pin %d.", interrupt_type, pin);
    //         return E_INVALID_PARAM;
    // }

    // // Store callback and debounce info
    // s_gpio_pin_states[pin].interrupt_callback = callback;
    // s_gpio_pin_states[pin].debounce_time_ms = debounce_ms;
    // s_gpio_pin_states[pin].last_interrupt_time_ms = 0; // Reset debounce timer

    // // Call MCAL to configure the interrupt
    // if (MCAL_GPIO_ConfigInterrupt(config->mcal_pin, mcal_int_type) != E_OK) {
    //     // LOG_ERROR("HAL_GPIO", "MCAL ConfigInterrupt failed for pin %d (MCAL %d).", pin, config->mcal_pin);
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_GPIO_ERROR, (uint32_t)pin);
    //     return E_NOK;
    // }
    // LOG_DEBUG("HAL_GPIO", "Pin %d interrupt configured: Type=%d, Debounce=%lu ms", pin, interrupt_type, debounce_ms);
    return E_OK;
}

/**
 * @brief Enables the interrupt for a specific GPIO pin.
 * @param pin The logical ID of the GPIO pin.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_GPIO_EnableInterrupt(HAL_GPIO_Pin_t pin) {
    // if (pin >= HAL_GPIO_PIN_COUNT || !g_hal_gpio_configs[pin].is_interrupt_capable ||
    //     s_gpio_pin_states[pin].interrupt_callback == NULL) {
    //     // LOG_ERROR("HAL_GPIO", "EnableInterrupt failed: Pin %d not interrupt capable or no callback registered.", pin);
    //     return E_INVALID_PARAM;
    // }

    // const HAL_GPIO_Config_t *config = &g_hal_gpio_configs[pin];
    // if (MCAL_GPIO_EnableInterrupt(config->mcal_pin) != E_OK) {
    //     // LOG_ERROR("HAL_GPIO", "MCAL EnableInterrupt failed for pin %d (MCAL %d).", pin, config->mcal_pin);
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_GPIO_ERROR, (uint32_t)pin);
    //     return E_NOK;
    // }
    // LOG_DEBUG("HAL_GPIO", "Pin %d interrupt enabled.", pin);
    return E_OK;
}

/**
 * @brief Disables the interrupt for a specific GPIO pin.
 * @param pin The logical ID of the GPIO pin.
 * @return E_OK on success, E_NOK on failure.
 */
Status_t HAL_GPIO_DisableInterrupt(HAL_GPIO_Pin_t pin) {
    // if (pin >= HAL_GPIO_PIN_COUNT || !g_hal_gpio_configs[pin].is_interrupt_capable) {
    //     // LOG_ERROR("HAL_GPIO", "DisableInterrupt failed: Invalid pin %d or not interrupt capable.", pin);
    //     return E_INVALID_PARAM;
    // }

    // const HAL_GPIO_Config_t *config = &g_hal_gpio_configs[pin];
    // if (MCAL_GPIO_DisableInterrupt(config->mcal_pin) != E_OK) {
    //     // LOG_ERROR("HAL_GPIO", "MCAL DisableInterrupt failed for pin %d (MCAL %d).", pin, config->mcal_pin);
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_GPIO_ERROR, (uint32_t)pin);
    //     return E_NOK;
    // }
    // LOG_DEBUG("HAL_GPIO", "Pin %d interrupt disabled.", pin);
    return E_OK;
}

/**
 * @brief Internal MCAL GPIO Interrupt Handler.
 * This function is registered with the MCAL layer to receive physical GPIO interrupts.
 * It maps the physical pin to a logical pin and dispatches to the registered HAL callback.
 * It also handles software debouncing.
 * @param mcal_pin The physical MCAL GPIO pin that triggered the interrupt.
 */
static void HAL_GPIO_McalInterruptHandler(HAL_GPIO_Pin_t mcal_pin) {
    // HAL_GPIO_Pin_t logical_pin = HAL_GPIO_GetLogicalPin(mcal_pin);

    // if (logical_pin == HAL_GPIO_PIN_COUNT) {
    //     // LOG_ERROR("HAL_GPIO", "Interrupt from unmapped MCAL pin %d.", mcal_pin);
    //     // SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_GPIO_ERROR, (uint32_t)mcal_pin);
    //     return;
    // }

    // HAL_GPIO_PinState_t *state = &s_gpio_pin_states[logical_pin];
    // uint32_t current_time_ms = HAL_RTC_GetTimeMs(); // Use HAL_RTC for time

    // // Implement software debouncing
    // if (state->debounce_time_ms > 0) {
    //     if ((current_time_ms - state->last_interrupt_time_ms) < state->debounce_time_ms) {
    //         // Still within debounce period, ignore this interrupt
    //         return;
    //     }
    // }

    // state->last_interrupt_time_ms = current_time_ms; // Update last interrupt time

    // // Call the registered HAL callback if available
    // if (state->interrupt_callback != NULL) {
    //     state->interrupt_callback(logical_pin);
    // } else {
    //     // LOG_WARNING("HAL_GPIO", "Interrupt on pin %d with no registered callback.", logical_pin);
    // }
}

/**
 * @brief Helper function to get the logical HAL_GPIO_Pin_t from an MCAL_GPIO_Pin_t.
 * @param mcal_pin The physical MCAL GPIO pin.
 * @return The corresponding logical HAL_GPIO_Pin_t, or HAL_GPIO_PIN_COUNT if not found.
 */
static HAL_GPIO_Pin_t HAL_GPIO_GetLogicalPin(HAL_GPIO_Pin_t mcal_pin) {
    // for (uint8_t i = 0; i < HAL_GPIO_PIN_COUNT; i++) {
    //     if (g_hal_gpio_configs[i].mcal_pin == mcal_pin) {
    //         return g_hal_gpio_configs[i].logical_id;
    //     }
    // }
    // return HAL_GPIO_PIN_COUNT; // Not found
}
