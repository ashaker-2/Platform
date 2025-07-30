// ecual/src/ecual_gpio.c

#include "ecual_gpio.h"
#include "ecual_gpio_config.h" // Now includes the configuration array

// ESP32 SoC header files for direct register access
#include "soc/gpio_reg.h"   // GPIO control registers
#include "soc/io_mux_reg.h" // IO_MUX registers for pin functions and pull-ups/downs

// Define general return values for ECUAL functions
#define ECUAL_OK    (0)
#define ECUAL_ERROR (1)

/**
 * @brief Internal helper to validate GPIO pin number.
 * @param gpio_num The GPIO pin number.
 * @return 1 if valid, 0 if invalid.
 */
static uint8_t ECUAL_GPIO_IsValidPin(uint8_t gpio_num) {
    if (gpio_num > ECUAL_GPIO_MAX_PIN_NUM || (gpio_num >= 34 && gpio_num <= 39 && gpio_num != 34 && gpio_num != 35 && gpio_num != 36 && gpio_num != 37 && gpio_num != 38 && gpio_num != 39)) {
        // Corrected check for input-only pins (GPIO34-39 are input only)
        // If a pin within 34-39 range is requested as OUTPUT, it's an error.
        // For simplicity, we just return 0 if it's beyond max or falls into the input-only range
        // that's not actually an input-only pin.
        // A more robust check might be:
        // if (gpio_num > ECUAL_GPIO_MAX_PIN_NUM || (gpio_num >= 34 && gpio_num <= 39 && direction == ECUAL_GPIO_DIR_OUTPUT))
        // but for ECUAL_GPIO_IsValidPin, just checking the number is fine.
        return 0;
    }
    return 1;
}

uint8_t ECUAL_GPIO_Init(void) {
    uint32_t i;
    uint8_t ret = ECUAL_OK;

    // Loop through all defined GPIO pin configurations and apply them
    for (i = 0; i < ECUAL_NUM_GPIO_CONFIGURATIONS; i++) {
        const ECUAL_GPIO_Config_t *cfg = &gpio_pin_configurations[i];

        // 1. Set Direction
        if (ECUAL_GPIO_SetDirection(cfg->gpio_num, cfg->direction) != ECUAL_OK) {
            ret = ECUAL_ERROR;
            // Optionally, add error logging here
            continue; // Move to next pin if direction setting failed
        }

        // 2. Set Pull Resistors (for both input and output, though usually for input)
        if (ECUAL_GPIO_SetPull(cfg->gpio_num, cfg->pull) != ECUAL_OK) {
            ret = ECUAL_ERROR;
            // Optionally, add error logging here
            continue; // Move to next pin if pull setting failed
        }

        // 3. Set Initial State for Output Pins
        if (cfg->direction == ECUAL_GPIO_DIR_OUTPUT) {
            if (ECUAL_GPIO_SetState(cfg->gpio_num, cfg->initial_state) != ECUAL_OK) {
                ret = ECUAL_ERROR;
                // Optionally, add error logging here
                continue; // Move to next pin if state setting failed
            }
        }
    }

    return ret; // Returns ECUAL_OK only if all configurations were applied successfully
}


// ECUAL_GPIO_SetDirection, ECUAL_GPIO_SetState, ECUAL_GPIO_GetState, ECUAL_GPIO_SetPull
// remain the same as in the previous response, but ensure ECUAL_GPIO_IsValidPin logic
// is updated for output/input-only pins if you want strict validation.

// --- Rest of ecual_gpio.c functions (as previously provided) ---

uint8_t ECUAL_GPIO_SetDirection(uint8_t gpio_num, ECUAL_GPIO_Direction_t direction) {
    if (!ECUAL_GPIO_IsValidPin(gpio_num)) {
        return ECUAL_ERROR;
    }
    
    // Check for input-only pins if trying to set as output (GPIO34-39)
    if (direction == ECUAL_GPIO_DIR_OUTPUT && gpio_num >= 34 && gpio_num <= 39) {
        return ECUAL_ERROR; // Cannot set input-only pins as output
    }

    if (direction == ECUAL_GPIO_DIR_OUTPUT) {
        GPIO.enable_w1tc = (1ULL << gpio_num); // Disable output first
        GPIO.enable_w1ts = (1ULL << gpio_num); // Then enable output
    } else { // ECUAL_GPIO_DIR_INPUT
        GPIO.enable_w1tc = (1ULL << gpio_num); // Disable output
    }

    return ECUAL_OK;
}

uint8_t ECUAL_GPIO_SetState(uint8_t gpio_num, ECUAL_GPIO_State_t state) {
    if (!ECUAL_GPIO_IsValidPin(gpio_num)) {
        return ECUAL_ERROR;
    }

    // Check if the pin is actually configured as output before trying to set state
    // (Optional, but good for robustness; requires reading GPIO.enable_reg)
    // For now, assuming it's correctly configured as output.

    if (state == ECUAL_GPIO_STATE_HIGH) {
        GPIO.out_w1ts = (1ULL << gpio_num);
    } else {
        GPIO.out_w1tc = (1ULL << gpio_num);
    }

    return ECUAL_OK;
}

ECUAL_GPIO_State_t ECUAL_GPIO_GetState(uint8_t gpio_num) {
    if (!ECUAL_GPIO_IsValidPin(gpio_num)) {
        return ECUAL_GPIO_STATE_LOW; // Return a safe default or error
    }

    if (GPIO.in >> gpio_num & 0x1) {
        return ECUAL_GPIO_STATE_HIGH;
    } else {
        return ECUAL_GPIO_STATE_LOW;
    }
}

uint8_t ECUAL_GPIO_SetPull(uint8_t gpio_num, ECUAL_GPIO_Pull_t pull) {
    if (!ECUAL_GPIO_IsValidPin(gpio_num)) {
        return ECUAL_ERROR;
    }

    volatile uint32_t *mux_reg = &IO_MUX.gpio[gpio_num];
    uint32_t reg_val = *mux_reg;

    // Clear existing pull bits
    reg_val &= ~(BIT(FUN_PU_S) | BIT(FUN_PD_S));

    if (pull == ECUAL_GPIO_PULL_UP) {
        reg_val |= BIT(FUN_PU_S);
    } else if (pull == ECUAL_GPIO_PULL_DOWN) {
        reg_val |= BIT(FUN_PD_S);
    }
    
    // For pins 34-39, FUN_DRV_S (drive strength) might not be relevant/present.
    // Ensure we don't accidentally write to reserved bits or misconfigure.
    // For standard GPIOs, the function bits are typically default to GPIO_FUNC,
    // so just modifying FUN_PU/FUN_PD is generally safe.
    *mux_reg = reg_val;

    return ECUAL_OK;
}
