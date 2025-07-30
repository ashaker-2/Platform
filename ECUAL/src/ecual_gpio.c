// ecual/src/ecual_gpio.c

#include "ecual_gpio.h"
#include "ecual_gpio_config.h"

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
    if (gpio_num > ECUAL_GPIO_MAX_PIN_NUM) { // ESP32 has GPIO0-39
        // Add more specific checks if certain pins are reserved/input-only (e.g., GPIO34-39 are input-only)
        return 0;
    }
    return 1;
}

uint8_t ECUAL_GPIO_Init(void) {
    // For ESP32, the GPIO peripheral is generally always enabled after boot.
    // No specific clock enable or global initialization required for basic GPIO.
    // More complex setups (e.g., interrupts, low-power modes) might need additional init.
    return ECUAL_OK;
}

uint8_t ECUAL_GPIO_SetDirection(uint8_t gpio_num, ECUAL_GPIO_Direction_t direction) {
    if (!ECUAL_GPIO_IsValidPin(gpio_num)) {
        return ECUAL_ERROR;
    }

    if (direction == ECUAL_GPIO_DIR_OUTPUT) {
        // Set pin as output
        // Clear bit in GPIO_ENABLE_W1TC_REG (clear output disable)
        // Set bit in GPIO_ENABLE_W1TS_REG (set output enable)
        GPIO.enable_w1tc = (1 << gpio_num); // Disable output for this pin first
        GPIO.enable_w1ts = (1 << gpio_num); // Then enable it
        
        // Configure IO_MUX for output (connect GPIO matrix to pad)
        // This is crucial for ESP32 as it determines the pin's function
        // GPIO_FUNC_OUT_SEL_CFG_REG(gpio_num) configures the output mux for the specific GPIO.
        // The default value GPIO_FUNC_OUT_SEL_CFG_REG(gpio_num).sel = 0x100 means GPIO matrix direct output.
        // It's often already configured this way for simple output, but explicit setting is safer.
        // For general GPIO output, we want to select the GPIO matrix output path (usually value 0x100 for GPIO)
        // Check esp32 technical reference manual for exact values if complex muxing is needed.
        // For simple GPIO, writing the GPIO_OUT_REG/ENABLE_REG usually suffices if the default mux is GPIO.
        // However, for certain pins or more robust control, setting the IO_MUX for output is needed.
        // Example for output configuration, often handled by a higher layer or assumed by GPIO_ENABLE_W1TS:
        // WRITE_PERI_REG(IO_MUX_GPIO%d_REG, (READ_PERI_REG(IO_MUX_GPIO%d_REG) & ~FUN_DRV_M) | FUN_DRV_V(0x2) | FUN_PU | FUN_PD); // Example for drive strength, pull-ups if needed
        // For output, we typically clear the function bits to default GPIO.

    } else { // ECUAL_GPIO_DIR_INPUT
        // Set pin as input
        // Clear bit in GPIO_ENABLE_W1TC_REG (disable output for this pin)
        GPIO.enable_w1tc = (1 << gpio_num);

        // Configure IO_MUX for input (connect input signal to GPIO matrix)
        // No specific IO_MUX register writes often needed just for input,
        // but pull-up/down are configured through IO_MUX.
        // Input enable is generally automatic when output is disabled.
    }

    return ECUAL_OK;
}

uint8_t ECUAL_GPIO_SetState(uint8_t gpio_num, ECUAL_GPIO_State_t state) {
    if (!ECUAL_GPIO_IsValidPin(gpio_num)) {
        return ECUAL_ERROR;
    }

    if (state == ECUAL_GPIO_STATE_HIGH) {
        // Set the bit in GPIO_OUT_W1TS_REG (Write 1 to Set)
        GPIO.out_w1ts = (1 << gpio_num);
    } else {
        // Set the bit in GPIO_OUT_W1TC_REG (Write 1 to Clear)
        GPIO.out_w1tc = (1 << gpio_num);
    }

    return ECUAL_OK;
}

ECUAL_GPIO_State_t ECUAL_GPIO_GetState(uint8_t gpio_num) {
    if (!ECUAL_GPIO_IsValidPin(gpio_num)) {
        // Return a default/error state, or handle error
        return ECUAL_GPIO_STATE_LOW; // Or some error indicator
    }

    // Read the bit from GPIO_IN_REG
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

    // IO_MUX registers control pull-ups/downs.
    // Each GPIO pin (0-39) has a corresponding IO_MUX_GPIOX_REG.
    // We need to manipulate the FUN_PU (pull-up enable) and FUN_PD (pull-down enable) bits.

    volatile uint32_t *mux_reg = &IO_MUX.gpio[gpio_num];
    uint32_t reg_val = *mux_reg;

    // Clear existing pull bits
    reg_val &= ~(BIT(FUN_PU_S) | BIT(FUN_PD_S)); // Clear pull-up and pull-down bits

    if (pull == ECUAL_GPIO_PULL_UP) {
        reg_val |= BIT(FUN_PU_S); // Set pull-up bit
    } else if (pull == ECUAL_GPIO_PULL_DOWN) {
        reg_val |= BIT(FUN_PD_S); // Set pull-down bit
    }
    // If ECUAL_GPIO_PULL_NONE, both bits remain cleared.

    *mux_reg = reg_val;

    return ECUAL_OK;
}
