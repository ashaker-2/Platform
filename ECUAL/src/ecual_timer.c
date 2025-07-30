// ecual/src/ecual_timer.c

#include "ecual_timer.h"        // Includes driver/timer.h
#include "ecual_timer_config.h" // For configurations and ID map
#include "ecual_common.h"       // For ECUAL_OK and ECUAL_ERROR
#include "esp_log.h"            // For logging if needed

static const char *TAG = "ECUAL_TIMER";

// Static array to store registered callbacks and their arguments
static struct {
    ECUAL_TIMER_Callback_t callback;
    void                   *arg;
} timer_callbacks[ECUAL_TIMER_TOTAL_TIMERS];

// Internal ISR handler, dispatches to user callback
static void IRAM_ATTR timer_isr_handler(void *arg) {
    int timer_id_val = (int)arg; // The ECUAL_TIMER_ID_t is passed as arg

    // Retrieve the actual timer group and index from the map
    ECUAL_TIMER_Group_t group = timer_id_map[timer_id_val].group;
    ECUAL_TIMER_Index_t index = timer_id_map[timer_id_val].index;

    // Acknowledge the interrupt
    // Check if the alarm interrupt flag is set
    if (timer_group_get_intr_status_in_isr(group) & BIT(index)) {
        timer_group_clr_intr_status_in_isr(group, index); // Clear alarm interrupt bit

        // Call the user-registered callback if it exists
        if (timer_callbacks[timer_id_val].callback != NULL) {
            timer_callbacks[timer_id_val].callback(timer_callbacks[timer_id_val].arg);
        }

        // If auto-reload is enabled and the timer counts up, the counter is reset to 0
        // or the start value on alarm. For down-counting, it needs to be reset manually.
        // The driver handles auto-reload, so no manual reset is needed if enabled.
    }
}


uint8_t ECUAL_TIMER_Init(void) {
    esp_err_t mcal_ret;
    uint8_t ecual_ret = ECUAL_OK;
    uint32_t i;

    // Initialize callback array to NULL
    for (i = 0; i < ECUAL_TIMER_TOTAL_TIMERS; i++) {
        timer_callbacks[i].callback = NULL;
        timer_callbacks[i].arg = NULL;
    }

    for (i = 0; i < ECUAL_NUM_TIMER_CONFIGURATIONS; i++) {
        const ECUAL_TIMER_Config_t *cfg = &timer_configurations[i];

        // 1. Configure timer parameters
        timer_config_t timer_conf = {
            .divider = cfg->divider,
            .counter_dir = (timer_count_dir_t)cfg->count_direction, // Direct cast
            .counter_en = TIMER_PAUSE, // Start paused
            .alarm_en = (cfg->enable_alarm ? TIMER_ALARM_EN : TIMER_ALARM_DIS),
            .auto_reload = (timer_autoreload_t)cfg->auto_reload, // Direct cast
        };

        mcal_ret = timer_init((timer_group_t)cfg->group, (timer_idx_t)cfg->index, &timer_conf);
        if (mcal_ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to init Timer %d:%d (err 0x%x)", cfg->group, cfg->index, mcal_ret);
            ecual_ret = ECUAL_ERROR;
            continue;
        }

        // 2. Set initial counter value
        mcal_ret = timer_set_counter_value((timer_group_t)cfg->group, (timer_idx_t)cfg->index, cfg->initial_count);
        if (mcal_ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set counter value for Timer %d:%d (err 0x%x)", cfg->group, cfg->index, mcal_ret);
            ecual_ret = ECUAL_ERROR;
            continue;
        }

        // 3. Set alarm value if enabled
        if (cfg->enable_alarm) {
            mcal_ret = timer_set_alarm_value((timer_group_t)cfg->group, (timer_idx_t)cfg->index, cfg->alarm_value);
            if (mcal_ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to set alarm value for Timer %d:%d (err 0x%x)", cfg->group, cfg->index, mcal_ret);
                ecual_ret = ECUAL_ERROR;
                continue;
            }
        }

        // 4. Enable interrupt and register ISR if enabled
        if (cfg->enable_interrupt) {
            mcal_ret = timer_enable_intr((timer_group_t)cfg->group, (timer_idx_t)cfg->index);
            if (mcal_ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to enable interrupt for Timer %d:%d (err 0x%x)", cfg->group, cfg->index, mcal_ret);
                ecual_ret = ECUAL_ERROR;
                continue;
            }

            // Register the internal ISR, passing the ECUAL_TIMER_ID_t as arg
            mcal_ret = timer_isr_register(
                (timer_group_t)cfg->group,
                (timer_idx_t)cfg->index,
                timer_isr_handler,
                (void *)i, // Pass the index 'i' (which corresponds to ECUAL_TIMER_ID_t)
                ESP_INTR_FLAG_IRAM, // Execute ISR from IRAM for performance
                NULL
            );
            if (mcal_ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to register ISR for Timer %d:%d (err 0x%x)", cfg->group, cfg->index, mcal_ret);
                ecual_ret = ECUAL_ERROR;
                continue;
            }

            // Store the user-defined callback from configuration
            timer_callbacks[i].callback = cfg->callback;
            timer_callbacks[i].arg = cfg->callback_arg;
        }
    }

    return ecual_ret;
}

static const ECUAL_TIMER_Internal_Map_t* get_timer_map(ECUAL_TIMER_ID_t timer_id) {
    if (timer_id >= ECUAL_TIMER_TOTAL_TIMERS) {
        ESP_LOGE(TAG, "Invalid Timer ID: %d", timer_id);
        return NULL;
    }
    return &timer_id_map[timer_id];
}


uint8_t ECUAL_TIMER_Start(ECUAL_TIMER_ID_t timer_id) {
    const ECUAL_TIMER_Internal_Map_t *map = get_timer_map(timer_id);
    if (map == NULL) { return ECUAL_ERROR; }

    esp_err_t mcal_ret = timer_start((timer_group_t)map->group, (timer_idx_t)map->index);
    return (mcal_ret == ESP_OK) ? ECUAL_OK : ECUAL_ERROR;
}

uint8_t ECUAL_TIMER_Stop(ECUAL_TIMER_ID_t timer_id) {
    const ECUAL_TIMER_Internal_Map_t *map = get_timer_map(timer_id);
    if (map == NULL) { return ECUAL_ERROR; }

    esp_err_t mcal_ret = timer_pause((timer_group_t)map->group, (timer_idx_t)map->index);
    return (mcal_ret == ESP_OK) ? ECUAL_OK : ECUAL_ERROR;
}

uint8_t ECUAL_TIMER_GetCount(ECUAL_TIMER_ID_t timer_id, uint64_t *count) {
    if (count == NULL) { return ECUAL_ERROR; }
    const ECUAL_TIMER_Internal_Map_t *map = get_timer_map(timer_id);
    if (map == NULL) { return ECUAL_ERROR; }

    esp_err_t mcal_ret = timer_get_counter_value((timer_group_t)map->group, (timer_idx_t)map->index, count);
    return (mcal_ret == ESP_OK) ? ECUAL_OK : ECUAL_ERROR;
}

uint8_t ECUAL_TIMER_SetCount(ECUAL_TIMER_ID_t timer_id, uint64_t count) {
    const ECUAL_TIMER_Internal_Map_t *map = get_timer_map(timer_id);
    if (map == NULL) { return ECUAL_ERROR; }

    esp_err_t mcal_ret = timer_set_counter_value((timer_group_t)map->group, (timer_idx_t)map->index, count);
    return (mcal_ret == ESP_OK) ? ECUAL_OK : ECUAL_ERROR;
}

uint8_t ECUAL_TIMER_RegisterCallback(ECUAL_TIMER_ID_t timer_id, ECUAL_TIMER_Callback_t callback, void *arg) {
    if (timer_id >= ECUAL_TIMER_TOTAL_TIMERS) {
        return ECUAL_ERROR; // Invalid timer ID
    }

    timer_callbacks[timer_id].callback = callback;
    timer_callbacks[timer_id].arg = arg;

    return ECUAL_OK;
}
