/**
 * @file hal_timer.c
 * @brief Hardware Abstraction Layer for Timer - Implementation.
 *
 * This module provides the concrete implementation for the Timer HAL interface
 * defined in `hal_timer.h`. It utilizes the ESP-IDF Timer driver to
 * configure and manage hardware timers on the ESP32.
 */

#include "hal_timer.h"
#include "hal_timer_cfg.h" // Include configuration header

#include "driver/timer.h"      // ESP-IDF Timer driver
#include "esp_log.h"           // ESP-IDF logging
#include "freertos/FreeRTOS.h" // For FreeRTOS types
#include "freertos/semphr.h"   // For semaphores if needed for synchronization (not directly used here for ISR)

// Define a tag for ESP-IDF logging
static const char *TAG = "HAL_TIMER";

/**
 * @brief Structure to hold runtime data for each Timer channel.
 */
typedef struct
{
    bool initialized;              /**< Flag indicating if the channel is initialized. */
    HAL_TIMER_Callback_t callback; /**< Callback function for timer expiry. */
    void *user_data;               /**< User data to pass to the callback. */
    timer_group_t timer_group;     /**< ESP-IDF Timer Group. */
    timer_idx_t timer_idx;         /**< ESP-IDF Timer Index within the group. */
} HAL_TIMER_ChannelData_t;

// Array to hold runtime data for each Timer channel
static HAL_TIMER_ChannelData_t g_timer_channel_data[HAL_TIMER_CHANNEL_MAX];

/**
 * @brief Internal helper to map HAL_TIMER_Channel_t to ESP-IDF timer group and index.
 * @param channel The HAL timer channel.
 * @param group Pointer to store the ESP-IDF timer group.
 * @param idx Pointer to store the ESP-IDF timer index.
 * @return true if mapping is successful, false otherwise.
 */
static bool hal_timer_map_channel_to_esp_timer(HAL_TIMER_Channel_t channel, timer_group_t *group, timer_idx_t *idx)
{
    switch (channel)
    {
    case HAL_TIMER_CHANNEL_0:
        *group = TIMER_GROUP_0;
        *idx = TIMER_0;
        break;
    case HAL_TIMER_CHANNEL_1:
        *group = TIMER_GROUP_0;
        *idx = TIMER_1;
        break;
    // Add more mappings if HAL_TIMER_CHANNEL_MAX is increased
    /*
    case HAL_TIMER_CHANNEL_2:
        *group = TIMER_GROUP_1;
        *idx = TIMER_0;
        break;
    */
    default:
        ESP_LOGE(TAG, "Invalid HAL Timer channel: %d", channel);
        return false;
    }
    return true;
}

/**
 * @brief Internal ISR callback for ESP-IDF timers.
 *
 * This function is registered with the ESP-IDF timer driver and is called
 * when a timer reaches its alarm value. It then invokes the user-defined
 * HAL callback.
 *
 * @param arg Pointer to the HAL_TIMER_Channel_t (passed as user data).
 * @return True if a higher priority task was woken, false otherwise.
 */
static bool IRAM_ATTR timer_isr_callback(void *arg)
{
    HAL_TIMER_Channel_t channel = (HAL_TIMER_Channel_t)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (channel < HAL_TIMER_CHANNEL_MAX && g_timer_channel_data[channel].initialized)
    {
        timer_group_t group = g_timer_channel_data[channel].timer_group;
        timer_idx_t idx = g_timer_channel_data[channel].timer_idx;

        // Clear the interrupt status
        if (group == TIMER_GROUP_0)
        {
            TIMERG0.int_clr_timers.val = BIT(idx);
        }
        else
        { // TIMER_GROUP_1
            TIMERG1.int_clr_timers.val = BIT(idx);
        }

        // Re-enable the alarm if in auto-reload mode
        if (g_hal_timer_channel_configs[channel].mode == HAL_TIMER_MODE_AUTO_RELOAD)
        {
            timer_set_alarm_value(group, idx, g_hal_timer_channel_configs[channel].default_period_us); // Re-set alarm value
            timer_start(group, idx);                                                                   // Restart timer
        }

        // Invoke the user-defined callback
        if (g_timer_channel_data[channel].callback != NULL)
        {
            g_timer_channel_data[channel].callback(channel, g_timer_channel_data[channel].user_data);
        }
    }
    else
    {
        ESP_LOGE(TAG, "Timer ISR received for uninitialized or invalid channel: %d", channel);
    }

    return xHigherPriorityTaskWoken == pdTRUE;
}

HAL_TIMER_Status_t HAL_TIMER_Init(HAL_TIMER_Channel_t channel)
{
    if (channel >= HAL_TIMER_CHANNEL_MAX)
    {
        ESP_LOGE(TAG, "Invalid Timer channel: %d", channel);
        return HAL_TIMER_STATUS_INVALID_PARAM;
    }

    if (g_timer_channel_data[channel].initialized)
    {
        ESP_LOGW(TAG, "Timer channel %d already initialized.", channel);
        return HAL_TIMER_STATUS_OK;
    }

    const HAL_TIMER_ChannelConfig_t *cfg = &g_hal_timer_channel_configs[channel];
    timer_group_t group;
    timer_idx_t idx;

    if (!hal_timer_map_channel_to_esp_timer(channel, &group, &idx))
    {
        return HAL_TIMER_STATUS_INVALID_PARAM;
    }

    timer_config_t timer_config = {
        .alarm_en = TIMER_ALARM_EN,
        .counter_en = TIMER_PAUSE, // Start paused
        .intr_type = TIMER_INTR_LEVEL,
        .counter_dir = (cfg->direction == HAL_TIMER_DIR_UP) ? TIMER_COUNT_UP : TIMER_COUNT_DOWN,
        .auto_reload = (cfg->mode == HAL_TIMER_MODE_AUTO_RELOAD) ? TIMER_AUTORELOAD_EN : TIMER_AUTORELOAD_DIS,
        .divider = cfg->prescaler,
    };

    esp_err_t ret;

    // Initialize timer
    ret = timer_init(group, idx, &timer_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize Timer %d:%d: %s", group, idx, esp_err_to_name(ret));
        return HAL_TIMER_STATUS_ERROR;
    }

    // Set initial counter value (usually 0 for up-counting)
    ret = timer_set_counter_value(group, idx, 0x00000000ULL);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set counter value for Timer %d:%d: %s", group, idx, esp_err_to_name(ret));
        timer_deinit(group, idx);
        return HAL_TIMER_STATUS_ERROR;
    }

    // Set alarm value (period)
    ret = timer_set_alarm_value(group, idx, cfg->default_period_us);
    if (ret != ESP_OK)
    {
        LOGE(TAG, "Failed to set alarm value for Timer %d:%d: %s", group, idx, esp_err_to_name(ret));
        timer_deinit(group, idx);
        return HAL_TIMER_STATUS_ERROR;
    }

    // Enable timer interrupt
    ret = timer_enable_intr(group, idx);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to enable interrupt for Timer %d:%d: %s", group, idx, esp_err_to_name(ret));
        timer_deinit(group, idx);
        return HAL_TIMER_STATUS_ERROR;
    }

    // Register ISR
    ret = timer_isr_register(group, idx, timer_isr_callback, (void *)channel, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1, NULL); // Use ESP_INTR_FLAG_IRAM for ISR in IRAM
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register ISR for Timer %d:%d: %s", group, idx, esp_err_to_name(ret));
        timer_disable_intr(group, idx);
        timer_deinit(group, idx);
        return HAL_TIMER_STATUS_ERROR;
    }

    g_timer_channel_data[channel].initialized = true;
    g_timer_channel_data[channel].callback = cfg->default_callback;
    g_timer_channel_data[channel].user_data = cfg->default_user_data;
    g_timer_channel_data[channel].timer_group = group;
    g_timer_channel_data[channel].timer_idx = idx;

    LOGI(TAG, "Timer channel %d (TG%d_T%d) initialized. Period: %llu us, Prescaler: %lu",
         channel, group, idx, cfg->default_period_us, cfg->prescaler);
    return HAL_TIMER_STATUS_OK;
}

HAL_TIMER_Status_t HAL_TIMER_DeInit(HAL_TIMER_Channel_t channel)
{
    if (channel >= HAL_TIMER_CHANNEL_MAX)
    {
        ESP_LOGE(TAG, "Invalid Timer channel: %d", channel);
        return HAL_TIMER_STATUS_INVALID_PARAM;
    }

    if (!g_timer_channel_data[channel].initialized)
    {
        ESP_LOGW(TAG, "Timer channel %d not initialized.", channel);
        return HAL_TIMER_STATUS_OK;
    }

    timer_group_t group = g_timer_channel_data[channel].timer_group;
    timer_idx_t idx = g_timer_channel_data[channel].timer_idx;
    esp_err_t ret;

    ret = timer_disable_intr(group, idx);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to disable interrupt for Timer %d:%d: %s", group, idx, esp_err_to_name(ret));
        // Continue with deinit even if disabling interrupt fails
    }

    ret = timer_deinit(group, idx);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to deinitialize Timer %d:%d: %s", group, idx, esp_err_to_name(ret));
        return HAL_TIMER_STATUS_ERROR;
    }

    g_timer_channel_data[channel].initialized = false;
    g_timer_channel_data[channel].callback = NULL;
    g_timer_channel_data[channel].user_data = NULL;
    LOGI(TAG, "Timer channel %d de-initialized successfully.", channel);
    return HAL_TIMER_STATUS_OK;
}

HAL_TIMER_Status_t HAL_TIMER_Start(HAL_TIMER_Channel_t channel)
{
    if (channel >= HAL_TIMER_CHANNEL_MAX || !g_timer_channel_data[channel].initialized)
    {
        ESP_LOGE(TAG, "Timer channel %d not initialized or invalid.", channel);
        return HAL_TIMER_STATUS_NOT_INITIALIZED;
    }

    timer_group_t group = g_timer_channel_data[channel].timer_group;
    timer_idx_t idx = g_timer_channel_data[channel].timer_idx;

    esp_err_t ret = timer_start(group, idx);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start Timer %d:%d: %s", group, idx, esp_err_to_name(ret));
        return HAL_TIMER_STATUS_ERROR;
    }
    LOGI(TAG, "Timer channel %d started.", channel);
    return HAL_TIMER_STATUS_OK;
}

HAL_TIMER_Status_t HAL_TIMER_Stop(HAL_TIMER_Channel_t channel)
{
    if (channel >= HAL_TIMER_CHANNEL_MAX || !g_timer_channel_data[channel].initialized)
    {
        ESP_LOGE(TAG, "Timer channel %d not initialized or invalid.", channel);
        return HAL_TIMER_STATUS_NOT_INITIALIZED;
    }

    timer_group_t group = g_timer_channel_data[channel].timer_group;
    timer_idx_t idx = g_timer_channel_data[channel].timer_idx;

    esp_err_t ret = timer_pause(group, idx);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to stop Timer %d:%d: %s", group, idx, esp_err_to_name(ret));
        return HAL_TIMER_STATUS_ERROR;
    }
    LOGI(TAG, "Timer channel %d stopped.", channel);
    return HAL_TIMER_STATUS_OK;
}

HAL_TIMER_Status_t HAL_TIMER_SetPeriod(HAL_TIMER_Channel_t channel, uint64_t period_us)
{
    if (channel >= HAL_TIMER_CHANNEL_MAX || !g_timer_channel_data[channel].initialized)
    {
        ESP_LOGE(TAG, "Timer channel %d not initialized or invalid.", channel);
        return HAL_TIMER_STATUS_NOT_INITIALIZED;
    }
    if (period_us == 0)
    {
        ESP_LOGE(TAG, "Period cannot be 0 for Timer channel %d.", channel);
        return HAL_TIMER_STATUS_INVALID_PARAM;
    }

    timer_group_t group = g_timer_channel_data[channel].timer_group;
    timer_idx_t idx = g_timer_channel_data[channel].timer_idx;

    // Convert microseconds to timer ticks based on prescaler
    // The APB_CLK is typically 80MHz. Prescaler of 80 means 1 tick = 1 us.
    // So, period_us is directly the number of ticks needed.
    uint64_t ticks = period_us;

    esp_err_t ret = timer_set_alarm_value(group, idx, ticks);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set period for Timer %d:%d: %s", group, idx, esp_err_to_name(ret));
        return HAL_TIMER_STATUS_ERROR;
    }
    LOGI(TAG, "Timer channel %d period set to %llu us.", channel, period_us);
    return HAL_TIMER_STATUS_OK;
}

HAL_TIMER_Status_t HAL_TIMER_GetCounterValue(HAL_TIMER_Channel_t channel, uint64_t *value)
{
    if (channel >= HAL_TIMER_CHANNEL_MAX || !g_timer_channel_data[channel].initialized)
    {
        ESP_LOGE(TAG, "Timer channel %d not initialized or invalid.", channel);
        return HAL_TIMER_STATUS_NOT_INITIALIZED;
    }
    if (value == NULL)
    {
        ESP_LOGE(TAG, "Value pointer is NULL for Timer channel %d.", channel);
        return HAL_TIMER_STATUS_INVALID_PARAM;
    }

    timer_group_t group = g_timer_channel_data[channel].timer_group;
    timer_idx_t idx = g_timer_channel_data[channel].timer_idx;

    esp_err_t ret = timer_get_counter_value(group, idx, value);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get counter value for Timer %d:%d: %s", group, idx, esp_err_to_name(ret));
        return HAL_TIMER_STATUS_ERROR;
    }
    return HAL_TIMER_STATUS_OK;
}

HAL_TIMER_Status_t HAL_TIMER_RegisterCallback(HAL_TIMER_Channel_t channel, HAL_TIMER_Callback_t callback, void *user_data)
{
    if (channel >= HAL_TIMER_CHANNEL_MAX || !g_timer_channel_data[channel].initialized)
    {
        ESP_LOGE(TAG, "Timer channel %d not initialized or invalid.", channel);
        return HAL_TIMER_STATUS_NOT_INITIALIZED;
    }

    g_timer_channel_data[channel].callback = callback;
    g_timer_channel_data[channel].user_data = user_data;
    LOGI(TAG, "Timer channel %d callback registered.", channel);
    return HAL_TIMER_STATUS_OK;
}
