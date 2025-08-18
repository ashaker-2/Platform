/* ============================================================================
 * SOURCE FILE: HardwareAbstractionLayer/src/HAL_Timer.c
 * ============================================================================*/
/**
 * @file HAL_Timer.c
 * @brief Implements the public API functions for hardware timer operations,
 * including the module's initialization function.
 * These functions wrap the ESP-IDF timer driver calls with a common status return.
 */

#include "HAL_Timer.h"      // Header for HAL_Timer functions
#include "HAL_Timer_Cfg.h"  // To access Timer configuration array
#include "esp_log.h"        // ESP-IDF logging library
#include "driver/timer.h"   // ESP-IDF timer driver
#include "esp_err.h"        // For ESP_OK, ESP_FAIL etc.

static const char *TAG = "HAL_Timer";

/**
 * @brief Initializes and configures all hardware timers according to configurations
 * defined in the internal `s_timer_configurations` array from `HAL_Timer_Cfg.c`.
 *
 * @return E_OK on success, or an error code.
 */
Status_t HAL_Timer_Init(void) {
    esp_err_t ret;

    ESP_LOGI(TAG, "Applying Timer configurations from HAL_Timer_Cfg.c...");

    for (size_t i = 0; i < s_num_timer_configurations; i++) {
        const timer_cfg_item_t *cfg_item = &s_timer_configurations[i];

        ret = timer_init(cfg_item->group_id, cfg_item->timer_id, &cfg_item->config);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Timer %d-%d init failed: %s", cfg_item->group_id, cfg_item->timer_id, esp_err_to_name(ret));
            return E_ERROR;
        }
        timer_set_counter_value(cfg_item->group_id, cfg_item->timer_id, 0);
        timer_set_alarm_value(cfg_item->group_id, cfg_item->timer_id, cfg_item->initial_alarm_value);
        // Do not enable interrupt or start timer here; that's for HAL_Timer.c public functions
        ESP_LOGD(TAG, "Timer %d-%d configured for %llu us period.",
                 cfg_item->group_id, cfg_item->timer_id, cfg_item->initial_alarm_value);
    }

    ESP_LOGI(TAG, "All timers initialized successfully.");
    return E_OK;
}

/**
 * @brief Starts a previously initialized timer.
 * @param group_id The timer group ID.
 * @param timer_id The identifier of the timer to start.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_Timer_Start(timer_group_t group_id, timer_idx_t timer_id) {
    esp_err_t ret = timer_start(group_id, timer_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Timer %d-%d: %s", group_id, timer_id, esp_err_to_name(ret));
        return E_ERROR;
    }
    ESP_LOGD(TAG, "Timer %d-%d started.", group_id, timer_id);
    return E_OK;
}

/**
 * @brief Stops a running timer.
 * @param group_id The timer group ID.
 * @param timer_id The identifier of the timer to stop.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_Timer_Stop(timer_group_t group_id, timer_idx_t timer_id) {
    esp_err_t ret = timer_pause(group_id, timer_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop Timer %d-%d: %s", group_id, timer_id, esp_err_to_name(ret));
        return E_ERROR;
    }
    ESP_LOGD(TAG, "Timer %d-%d stopped.", group_id, timer_id);
    return E_OK;
}

/**
 * @brief Attaches a callback function to a timer interrupt.
 * @param group_id The timer group ID.
 * @param timer_id The identifier of the timer.
 * @param callback The function to call when the timer interrupt occurs.
 * @param arg An optional argument to pass to the callback function.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_Timer_AttachInterrupt(timer_group_t group_id, timer_idx_t timer_id, void (*callback)(void*), void* arg) {
    esp_err_t ret = timer_isr_callback_add(group_id, timer_id, callback, arg, 0); // Last param: ISR flags
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to attach ISR to Timer %d-%d: %s", group_id, timer_id, esp_err_to_name(ret));
        return E_ERROR;
    }
    ESP_LOGD(TAG, "ISR attached to Timer %d-%d.");
    return E_OK;
}

/**
 * @brief Enables the alarm for a specific timer.
 * The alarm value should have been set during configuration or dynamically before this call.
 * @param group_id The timer group ID.
 * @param timer_id The identifier of the timer.
 * @param enable True to enable the alarm, false to disable.
 * @return E_OK on success, or an error code.
 */
Status_t HAL_Timer_EnableAlarm(timer_group_t group_id, timer_idx_t timer_id, bool enable) {
    esp_err_t ret;
    if (enable) {
        ret = timer_set_alarm(group_id, timer_id, TIMER_ALARM_EN);
    } else {
        ret = timer_set_alarm(group_id, timer_id, TIMER_ALARM_DIS);
    }

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to %s alarm for Timer %d-%d: %s", enable ? "enable" : "disable", group_id, timer_id, esp_err_to_name(ret));
        return E_ERROR;
    }
    ESP_LOGD(TAG, "Alarm %s for Timer %d-%d.", enable ? "enabled" : "disabled", group_id, timer_id);
    return E_OK;
}
