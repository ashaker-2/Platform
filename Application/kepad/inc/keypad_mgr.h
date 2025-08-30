/**
 * @file keypad_mgr.h
 * @brief Enhanced Keypad Manager (generic) public API
 *
 * - 4x4 keypad (16 keys): reports Keypad_Button_ID_t and event type (PRESS/HOLD/RELEASE)
 * - Debounce + HOLD detection (configurable via keypad_mgr_cfg.h)
 * - Three events per button: PRESS (on press), HOLD (after threshold), RELEASE (on release)
 * - Events buffered in a ring queue (depth = KEYPAD_EVENT_QUEUE_DEPTH)
 * - Consumer may poll via KeypadMgr_GetEvent() or register callback via KeypadMgr_RegisterEventHandler()
 *
 * KeypadMgr is intentionally hardware/layout-aware but functionality-agnostic:
 * it does NOT map buttons to UX or SysMgr actions. Mapping must be done by SysMgrUX.
 */

#ifndef KEYPAD_MGR_H
#define KEYPAD_MGR_H

#include <stdint.h>
#include <stdbool.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 16 logical buttons (one entry per possible key position) */
typedef enum {
    KEYPAD_BTN_0 = 0,
    KEYPAD_BTN_1,
    KEYPAD_BTN_2,
    KEYPAD_BTN_3,
    KEYPAD_BTN_4,
    KEYPAD_BTN_5,
    KEYPAD_BTN_6,
    KEYPAD_BTN_7,
    KEYPAD_BTN_8,
    KEYPAD_BTN_9,
    KEYPAD_BTN_BACK,
    KEYPAD_BTN_ERASE,
    KEYPAD_BTN_ENTER,
    KEYPAD_BTN_UP,
    KEYPAD_BTN_DOWN,
    KEYPAD_BTN_LEFT,
    KEYPAD_BTN_RIGHT,
    KEYPAD_BTN_MAX
} Keypad_Button_ID_t;

/* Enhanced event types - now supports PRESS, HOLD, and RELEASE */
typedef enum {
    KEYPAD_EVT_NONE = 0,
    KEYPAD_EVT_PRESS,   /* Debounced press (button just pressed) */
    KEYPAD_EVT_HOLD,    /* Hold event (after threshold while pressed) */
    KEYPAD_EVT_RELEASE  /* Debounced release (button just released) */
} Keypad_Event_Type_t;

/* Event enable mask flags - for configuring which events to generate per button */
/* These are defined in keypad_mgr_cfg.h - this is just for reference */
/*
typedef enum {
    KEYPAD_EVT_ENABLE_NONE    = 0x00,
    KEYPAD_EVT_ENABLE_PRESS   = 0x01,
    KEYPAD_EVT_ENABLE_HOLD    = 0x02,
    KEYPAD_EVT_ENABLE_RELEASE = 0x04,
    KEYPAD_EVT_ENABLE_ALL     = 0x07
} Keypad_Event_Enable_t;
*/

/* Event structure */
typedef struct {
    Keypad_Button_ID_t button;
    Keypad_Event_Type_t type;
} Keypad_Event_t;

/* Callback prototype for immediate dispatch.
 * This callback is invoked from KeypadMgr_MainFunction() context (scanning task).
 * Keep callbacks short (queue or set flags) to avoid blocking scanning.
 */
typedef void (*Keypad_EventHandler_t)(const Keypad_Event_t *ev);

/* Public API */

/**
 * @brief Initialize the keypad manager.
 * Idempotent; must be called before MainFunction/GetEvent/RegisterHandler.
 */
Status_t KeypadMgr_Init(void);

/**
 * @brief Periodic scanning function (non-blocking).
 * Should be called at KEYPAD_SCAN_PERIOD_MS interval (see keypad_mgr_cfg.h).
 */
void KeypadMgr_MainFunction(void);

/**
 * @brief Non-blocking read of next buffered event.
 * If there's no event available returns E_DATA_STALE.
 */
Status_t KeypadMgr_GetEvent(Keypad_Event_t *out_event);

/**
 * @brief Register/unregister an immediate event handler callback.
 * - handler == NULL => unregister.
 * The callback is called from scanning context; keep short.
 */
void KeypadMgr_RegisterEventHandler(Keypad_EventHandler_t handler);

/**
 * @brief Get number of events currently queued (0..KEYPAD_EVENT_QUEUE_DEPTH).
 */
uint8_t KeypadMgr_GetQueuedCount(void);

#ifdef __cplusplus
}
#endif

#endif /* KEYPAD_MGR_H */