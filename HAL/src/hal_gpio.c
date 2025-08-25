/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/src/HAL_GPIO.c
 * ============================================================================*/
/**
 * @file HAL_GPIO.c
 * @brief Implements the public API functions for direct ESP32 GPIO operations,
 * including the module's initialization function.
 * These functions wrap the ESP-IDF GPIO driver calls with a common status return.
 */

#include "HAL_GPIO.h"       // Header for HAL_GPIO functions
#include "HAL_GPIO_Cfg.h"   // To access GPIO configuration array
#include "esp_log.h"        // ESP-IDF logging library
#include "driver/gpio.h"    // ESP-IDF GPIO driver
#include "esp_err.h"        // For ESP_OK, ESP_FAIL, etc.

static const char *TAG = "HAL_GPIO";

/**
 * @brief Initializes all ESP32 direct GPIO pins according to the configurations
 * defined in the internal `s_gpio_configurations` array from `HAL_GPIO_Cfg.c`.
 * Iterates through the array and applies each `gpio_config_item_t` structure.
 *
 * @return E_OK if all configurations are successful, otherwise an error code.
 */
Status_t HAL_GPIO_Init(void) {
    esp_err_t ret;

    ESP_LOGI(TAG, "Applying GPIO configurations from HAL_GPIO_Cfg.c...");

    // Iterate through the array of configurations and apply each one
    for (size_t i = 0; i < s_num_gpio_configurations; i++) {
        const gpio_config_item_t *cfg_item = &s_gpio_configurations[i];
        gpio_config_t io_conf = {
            .pin_bit_mask = cfg_item->pin_bit_mask,
            .mode = cfg_item->mode,
            .pull_up_en = cfg_item->pull_up_en,
            .pull_down_en = cfg_item->pull_down_en,
            .intr_type = cfg_item->intr_type,
        };
        ret = gpio_config(&io_conf);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to apply GPIO config[%zu]: %s", i, esp_err_to_name(ret));
            return E_ERROR;
        }
    }

    ESP_LOGI(TAG, "All direct ESP32 GPIOs configured successfully.");
    return E_OK;
}

/**
 * @brief Sets the output level of a configured GPIO pin.
 * @param gpio_num The GPIO number to control.
 * @param level The desired output level (0 for low, 1 for high).
 * @return E_OK on success, or an error code.
 */
Status_t HAL_GPIO_SetLevel(gpio_num_t gpio_num, uint8_t level) {
    esp_err_t ret = gpio_set_level(gpio_num, level);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set GPIO %d level to %d: %s", gpio_num, level, esp_err_to_name(ret));
        return E_ERROR;
    }
    return E_OK;
}

/**
 * @brief Reads the input level of a configured GPIO pin.
 * @param gpio_num The GPIO number to read.
 * @param level_out Pointer to store the read level (0 for low, 1 for high).
 * @return E_OK on success, or an error code.
 */
Status_t HAL_GPIO_GetLevel(gpio_num_t gpio_num, uint8_t *level_out) {
    if (level_out == NULL) {
        ESP_LOGE(TAG, "HAL_GPIO_GetLevel: level_out pointer is NULL for GPIO %d.", gpio_num);
        return E_INVALID_PARAM;
    }
    *level_out = gpio_get_level(gpio_num);
    return E_OK;
}

/**
 * @brief Configures the direction of a specific GPIO pin at runtime.
 * @param gpio_num The GPIO number to configure.
 * @param mode The desired GPIO mode (e.g., GPIO_MODE_INPUT, GPIO_MODE_OUTPUT).
 * @return E_OK on success, or an error code.
 */
Status_t HAL_GPIO_SetDirection(gpio_num_t gpio_num, gpio_mode_t mode) {
    esp_err_t ret = gpio_set_direction(gpio_num, mode);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set GPIO %d direction to %d: %s", gpio_num, mode, esp_err_to_name(ret));
        return E_ERROR;
    }
    return E_OK;
}

/**
 * @brief Enables or disables the internal pull-up resistor for a GPIO pin.
 * @param gpio_num The GPIO number.
 * @param enable True to enable pull-up, false to disable.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_GPIO_SetPullUp(gpio_num_t gpio_num, bool enable) {
    if (enable) {
        esp_err_t ret = gpio_pullup_en(gpio_num);
        return (ret == ESP_OK) ? E_OK : E_ERROR;
    } else {
        esp_err_t ret = gpio_pullup_dis(gpio_num);
        return (ret == ESP_OK) ? E_OK : E_ERROR;
    }
}

/**
 * @brief Enables or disables the internal pull-down resistor for a GPIO pin.
 * @param gpio_num The GPIO number.
 * @param enable True to enable pull-down, false to disable.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_GPIO_SetPullDown(gpio_num_t gpio_num, bool enable) {
    if (enable) {
        esp_err_t ret = gpio_pulldown_en(gpio_num);
        return (ret == ESP_OK) ? E_OK : E_ERROR;
    } else {
        esp_err_t ret = gpio_pulldown_dis(gpio_num);
        return (ret == ESP_OK) ? E_OK : E_ERROR;
    }
}
