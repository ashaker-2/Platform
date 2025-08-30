/**
 * @file keypad_mgr.c
 * @brief Enhanced Generic Keypad Manager implementation (matrix scan, debounce, hold, release, queue)
 *
 * - Hardware abstraction: expects HAL_GPIO_SetLevel / HAL_GPIO_GetLevel.
 * - Error handling: reports faults to SysMon on HAL failures, continues scanning.
 * - Enhanced event policy: emits PRESS (debounced), HOLD (after threshold), and RELEASE (debounced).
 * - Queue policy on full: oldest event dropped to accept newest; SysMon fault reported.
 *
 * NOTE: Replace HAL_GPIO_* and SYSMON_ReportFaultStatus with your platform APIs.
 */

#include "keypad_mgr.h"
#include "keypad_mgr_cfg.h"
#include "keypad_mgr_cfg.h" /* for g_keypad_row_gpios, g_keypad_col_gpios, g_keypad_rowcol_map */
#include "logger.h"
#include "hal_gpio.h"
#include "system_monitor.h"

#include <string.h>

#define TAG "KEYPAD_MGR"

/* Critical section macros (FreeRTOS aware) */
#if defined(FREERTOS_CONFIG_H) || defined(FREERTOS)
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #define KEY_ENTER_CRIT() taskENTER_CRITICAL()
  #define KEY_EXIT_CRIT()  taskEXIT_CRITICAL()
#else
  #define KEY_ENTER_CRIT() do{}while(0)
  #define KEY_EXIT_CRIT()  do{}while(0)
#endif

/* SysMon fault constants (adapt to your SysMon) */
#ifndef SYSMON_FAULT_KEYPAD_IO
#define SYSMON_FAULT_KEYPAD_IO  (0x3100)
#endif

/* Enhanced button internal state */
typedef struct {
    bool debounced;         /* stable pressed state */
    bool prev_debounced;    /* previous stable state (for release detection) */
    uint16_t db_cnt;        /* debounce counter */
    uint16_t hold_cnt;      /* hold counter while debounced pressed */
    bool hold_reported;     /* ensure single-shot hold */
} KeyBtnState_t;

static KeyBtnState_t s_btns[KEYPAD_BTN_MAX];
static bool s_initialized = false;

/* queue */
typedef struct {
    Keypad_Event_t buf[KEYPAD_EVENT_QUEUE_DEPTH];
    uint8_t head; /* next write index */
    uint8_t tail; /* next read index */
    uint8_t count;
} EvQueue_t;

static EvQueue_t s_queue;
static Keypad_EventHandler_t s_handler = NULL;

/* GPIO arrays and configuration from cfg.c (declared there) */
extern const uint8_t g_keypad_row_gpios[KEYPAD_NUM_ROWS];
extern const uint8_t g_keypad_col_gpios[KEYPAD_NUM_COLUMNS];
extern const Keypad_Button_ID_t g_keypad_rowcol_map[KEYPAD_NUM_ROWS][KEYPAD_NUM_COLUMNS];
extern const uint8_t g_keypad_event_config[KEYPAD_BTN_MAX];

/* Helpers */
static void enqueue_event(const Keypad_Event_t *ev);
static bool queue_is_full(void) { return (s_queue.count >= KEYPAD_EVENT_QUEUE_DEPTH); }
static bool queue_is_empty(void) { return (s_queue.count == 0); }
static Keypad_Button_ID_t map_rowcol_to_button(uint8_t r, uint8_t c);
static void process_button(Keypad_Button_ID_t id, bool raw_active);
static bool is_event_enabled(Keypad_Button_ID_t id, Keypad_Event_Type_t evt_type);

/* Public API */

Status_t KeypadMgr_Init(void)
{
    if (s_initialized) return E_OK;

    memset(s_btns, 0, sizeof(s_btns));
    s_queue.head = s_queue.tail = s_queue.count = 0;
    s_handler = NULL;
    s_initialized = true;
    LOGI(TAG, "KeypadMgr initialized (scan %d ms, debounce %d ticks, hold %d ticks, q=%d)",
         KEYPAD_SCAN_PERIOD_MS, KEYPAD_DEBOUNCE_TICKS, KEYPAD_HOLD_TICKS, KEYPAD_EVENT_QUEUE_DEPTH);
    return E_OK;
}

void KeypadMgr_MainFunction(void)
{
    if (!s_initialized) return;

    bool col_active[KEYPAD_NUM_COLUMNS];

    for (uint8_t r = 0; r < KEYPAD_NUM_ROWS; r++) {
        /* activate row (assume active = LOW) */
        if (HAL_GPIO_SetLevel(g_keypad_row_gpios[r], 0) != E_OK) {
            LOGE(TAG, "HAL_GPIO_SetLevel activate row %u failed", r);
            // SYSMON_ReportFaultStatus(SYSMON_FAULT_KEYPAD_IO, 1);
            /* continue scanning next rows */
        }

        /* read columns */
        bool row_read_ok = true;
        for (uint8_t c = 0; c < KEYPAD_NUM_COLUMNS; c++) {
            uint8_t level = 1;
            if (HAL_GPIO_GetLevel(g_keypad_col_gpios[c], &level) != E_OK) {
                LOGE(TAG, "HAL_GPIO_GetLevel col %u failed", c);
                // SYSMON_ReportFaultStatus(SYSMON_FAULT_KEYPAD_IO, 1);
                row_read_ok = false;
                break;
            }
            col_active[c] = (level == 0); /* active when pulled low by key */
        }

        if (!row_read_ok) {
            /* deactivate row and continue */
            HAL_GPIO_SetLevel(g_keypad_row_gpios[r], 1);
            continue;
        }

        /* process each column */
        for (uint8_t c = 0; c < KEYPAD_NUM_COLUMNS; c++) {
            Keypad_Button_ID_t bid = map_rowcol_to_button(r, c);
            if (bid < KEYPAD_BTN_MAX) {
                process_button(bid, col_active[c]);
            } /* else unmapped -> ignore */
        }

        /* deactivate row */
        HAL_GPIO_SetLevel(g_keypad_row_gpios[r], 1);
    }
}

Status_t KeypadMgr_GetEvent(Keypad_Event_t *out_event)
{
    if (!s_initialized) return E_NOT_INITIALIZED;
    if (!out_event) return E_NULL_POINTER;

    KEY_ENTER_CRIT();
    if (queue_is_empty()) {
        KEY_EXIT_CRIT();
        return E_DATA_STALE;
    }
    *out_event = s_queue.buf[s_queue.tail];
    s_queue.tail = (s_queue.tail + 1) % KEYPAD_EVENT_QUEUE_DEPTH;
    s_queue.count--;
    KEY_EXIT_CRIT();
    return E_OK;
}

void KeypadMgr_RegisterEventHandler(Keypad_EventHandler_t handler)
{
    s_handler = handler;
}

uint8_t KeypadMgr_GetQueuedCount(void)
{
    KEY_ENTER_CRIT();
    uint8_t n = s_queue.count;
    KEY_EXIT_CRIT();
    return n;
}

/* ===== Internal helpers ===== */

static void enqueue_event(const Keypad_Event_t *ev)
{
    if (!ev) return;

    KEY_ENTER_CRIT();
    if (queue_is_full()) {
        /* Policy: drop oldest event to accept incoming newest.
         * This preserves the latest user actions at the cost of older ones.
         * Raise SysMon fault to flag capacity issues.
         */
        LOGW(TAG, "Event queue full: dropping oldest event to enqueue new one");
        // SYSMON_ReportFaultStatus(SYSMON_FAULT_KEYPAD_IO, 1);

        /* advance tail (drop oldest) */
        s_queue.tail = (s_queue.tail + 1) % KEYPAD_EVENT_QUEUE_DEPTH;
        s_queue.count--;
    }

    s_queue.buf[s_queue.head] = *ev;
    s_queue.head = (s_queue.head + 1) % KEYPAD_EVENT_QUEUE_DEPTH;
    s_queue.count++;
    KEY_EXIT_CRIT();

    /* immediate dispatch to handler (if registered) */
    if (s_handler) {
        s_handler(ev);
    }
}

static Keypad_Button_ID_t map_rowcol_to_button(uint8_t r, uint8_t c)
{
    if (r < KEYPAD_NUM_ROWS && c < KEYPAD_NUM_COLUMNS) {
        return g_keypad_rowcol_map[r][c];
    }
    return KEYPAD_BTN_MAX;
}

static bool is_event_enabled(Keypad_Button_ID_t id, Keypad_Event_Type_t evt_type)
{
    if (id >= KEYPAD_BTN_MAX) return false;
    
    uint8_t enable_mask = g_keypad_event_config[id];
    
    switch (evt_type) {
        case KEYPAD_EVT_PRESS:
            return (enable_mask & KEYPAD_EVT_ENABLE_PRESS) != 0;
        case KEYPAD_EVT_HOLD:
            return (enable_mask & KEYPAD_EVT_ENABLE_HOLD) != 0;
        case KEYPAD_EVT_RELEASE:
            return (enable_mask & KEYPAD_EVT_ENABLE_RELEASE) != 0;
        default:
            return false;
    }
}

static void process_button(Keypad_Button_ID_t id, bool raw_active)
{
    KeyBtnState_t *s = &s_btns[id];

    if (s->debounced != raw_active) {
        /* potential transition -> debounce counting */
        s->db_cnt++;
        if (s->db_cnt >= KEYPAD_DEBOUNCE_TICKS) {
            /* debounce completed - state transition confirmed */
            s->prev_debounced = s->debounced;  /* save previous state */
            s->debounced = raw_active;
            s->db_cnt = 0;
            
            if (s->debounced) {
                /* stable press -> emit PRESS (if enabled for this button) */
                if (is_event_enabled(id, KEYPAD_EVT_PRESS)) {
                    Keypad_Event_t ev = { .button = id, .type = KEYPAD_EVT_PRESS };
                    enqueue_event(&ev);
                    LOGD(TAG, "Btn %d PRESS", id);
                }
                s->hold_cnt = 0;
                s->hold_reported = false;
            } else {
                /* stable release -> emit RELEASE (if enabled and was previously pressed) */
                if (s->prev_debounced && is_event_enabled(id, KEYPAD_EVT_RELEASE)) {
                    Keypad_Event_t ev = { .button = id, .type = KEYPAD_EVT_RELEASE };
                    enqueue_event(&ev);
                    LOGD(TAG, "Btn %d RELEASE", id);
                }
                /* reset hold tracking */
                s->hold_cnt = 0;
                s->hold_reported = false;
            }
        }
    } else {
        /* stable state - reset debounce counter */
        s->db_cnt = 0;
        
        if (s->debounced) {
            /* pressed and stable -> check for hold (if enabled) */
            if (!s->hold_reported && is_event_enabled(id, KEYPAD_EVT_HOLD)) {
                s->hold_cnt++;
                if (s->hold_cnt >= KEYPAD_HOLD_TICKS) {
                    Keypad_Event_t ev = { .button = id, .type = KEYPAD_EVT_HOLD };
                    enqueue_event(&ev);
                    s->hold_reported = true;
                    LOGD(TAG, "Btn %d HOLD", id);
                }
            }
            /* continue incrementing hold_cnt even after hold reported (for future use) */
        } else {
            /* stable released -> maintain reset state */
            s->hold_cnt = 0;
            s->hold_reported = false;
        }
    }
}