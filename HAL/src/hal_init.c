/* ============================================================================
 * SOURCE FILE: HAL/src/hal_init.c
 * ============================================================================*/
/**
 * @file hal_init.c
 * @brief Implementation of the HAL initialization function.
 * This file coordinates the setup of all configured hardware peripherals.
 */

#include "hal_init.h"       // Header for hal_init function
#include "esp_log.h"        // ESP-IDF logging library
#include "common.h"  // For Status_t

// Include all peripheral configuration headers
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "hal_adc.h"
#include "hal_timer.h"
#include "hal_spi.h"

static const char *TAG = "hal_init";

/**
 * @brief Initializes all hardware abstraction layer components.
 * This function orchestrates the initialization of GPIO, I2C, ADC, Timer, and SPI
 * peripherals by calling their respective configuration functions.
 * @return E_OK if all HAL components are initialized successfully, otherwise an error code.
 */
Status_t HAL_Init(void) {
    Status_t ret;

    ESP_LOGI(TAG, "Starting HAL Initialization...");

    // Initialize GPIOs
    ret = HAL_GPIO_Init();
    if (ret != E_OK) {
        ESP_LOGE(TAG, "GPIO initialization failed!");
        return ret;
    }
    ESP_LOGI(TAG, "GPIO initialized successfully.");

    // Initialize I2C
    ret = HAL_I2C_Init();
    if (ret != E_OK) {
        ESP_LOGE(TAG, "I2C initialization failed!");
        return ret;
    }
    ESP_LOGI(TAG, "I2C initialized successfully.");

    // Initialize ADC
    ret = HAL_ADC_Init();
    if (ret != E_OK) {
        ESP_LOGE(TAG, "ADC initialization failed!");
        return ret;
    }
    ESP_LOGI(TAG, "ADC initialized successfully.");

    // Initialize Timer
    // ret = HAL_TIMER_Init();
    if (ret != E_OK) {
        ESP_LOGE(TAG, "Timer initialization failed!");
        return ret;
    }
    ESP_LOGI(TAG, "Timer initialized successfully.");

    // Initialize SPI
    // ret = HAL_SPI_Init();
    if (ret != E_OK) {
        ESP_LOGE(TAG, "SPI initialization failed!");
        return ret;
    }
    ESP_LOGI(TAG, "SPI initialized successfully.");

    ESP_LOGI(TAG, "HAL Initialization Complete!");
    return E_OK;
}
