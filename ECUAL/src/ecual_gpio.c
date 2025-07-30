// ecual/src/ecual_gpio.c

#include "ecual_gpio.h"        // Includes driver/gpio.h
#include "ecual_gpio_config.h" // For pin configurations array (extern declaration)

// No local definitions for ECUAL_OK/ERROR needed here anymore

uint8_t ECUAL_GPIO_Init(void) {
    uint32_t i;
    uint8_t ecual_ret = ECUAL_OK;
    esp_err_t mcal_ret;

    for (i = 0; i < ECUAL_NUM_GPIO_CONFIGURATIONS; i++) {
        const ECUAL_GPIO_Config_t *cfg = &gpio_pin_configurations[i];

        // 1. Set Direction - Direct cast possible due to enum value alignment
        mcal_ret = gpio_set_direction(cfg->gpio_num, (gpio_mode_t)cfg->direction);
        if (mcal_ret != ESP_OK) { // Still check against ESP_OK as that's what mcal_ret returns
            ecual_ret = ECUAL_ERROR;
            // Optionally, add error logging here
            continue;
        }

        // 2. Set Pull Resistors - Direct cast possible due to enum value alignment
        mcal_ret = gpio_set_pull_mode(cfg->gpio_num, (gpio_pull_mode_t)cfg->pull);
        if (mcal_ret != ESP_OK) {
            ecual_ret = ECUAL_ERROR;
            continue;
        }

        // 3. Set Initial State for Output Pins - Direct cast possible due to enum value alignment
        if (cfg->direction == ECUAL_GPIO_DIR_OUTPUT) {
            mcal_ret = gpio_set_level(cfg->gpio_num, (int)cfg->initial_state);
            if (mcal_ret != ESP_OK) {
                ecual_ret = ECUAL_ERROR;
                continue;
            }
        }
    }

    return ecual_ret;
}

uint8_t ECUAL_GPIO_SetDirection(uint8_t gpio_num, ECUAL_GPIO_Direction_t direction) {
    esp_err_t mcal_ret = gpio_set_direction(gpio_num, (gpio_mode_t)direction);
    // Use ECUAL_OK and ECUAL_ERROR directly
    return (mcal_ret == ESP_OK) ? ECUAL_OK : ECUAL_ERROR;
}

uint8_t ECUAL_GPIO_SetState(uint8_t gpio_num, ECUAL_GPIO_State_t state) {
    esp_err_t mcal_ret = gpio_set_level(gpio_num, (int)state);
    return (mcal_ret == ESP_OK) ? ECUAL_OK : ECUAL_ERROR;
}

ECUAL_GPIO_State_t ECUAL_GPIO_GetState(uint8_t gpio_num) {
    return (ECUAL_GPIO_State_t)gpio_get_level(gpio_num);
}

uint8_t ECUAL_GPIO_SetPull(uint8_t gpio_num, ECUAL_GPIO_Pull_t pull) {
    esp_err_t mcal_ret = gpio_set_pull_mode(gpio_num, (gpio_pull_mode_t)pull);
    return (mcal_ret == ESP_OK) ? ECUAL_OK : ECUAL_ERROR;
}
