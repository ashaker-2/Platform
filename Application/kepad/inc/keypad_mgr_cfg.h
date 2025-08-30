/**
 * @file keypad_mgr_cfg.h
 * @brief Enhanced KeypadMgr configuration (GPIOs, timing, queue depth, per-button event config)
 *
 * Edit pin numbers, timing, and per-button event configurations to match your requirements.
 */

#ifndef KEYPAD_MGR_CFG_H
#define KEYPAD_MGR_CFG_H

#include "keypad_mgr.h"

/* Matrix dimensions */
#define KEYPAD_NUM_ROWS     4
#define KEYPAD_NUM_COLUMNS  4

/* Example Row GPIO numbers (adjust to board) */
#define KEYPAD_ROW_GPIO0  4
#define KEYPAD_ROW_GPIO1  12
#define KEYPAD_ROW_GPIO2  13
#define KEYPAD_ROW_GPIO3  0
#define KEYPAD_ROW_GPIOS   { KEYPAD_ROW_GPIO0, KEYPAD_ROW_GPIO1, KEYPAD_ROW_GPIO2, KEYPAD_ROW_GPIO3 }

/* Example Column GPIO numbers (adjust to board) */
#define KEYPAD_COL_GPIO0  36
#define KEYPAD_COL_GPIO1  37
#define KEYPAD_COL_GPIO2  38
#define KEYPAD_COL_GPIO3  39
#define KEYPAD_COL_GPIOS   { KEYPAD_COL_GPIO0, KEYPAD_COL_GPIO1, KEYPAD_COL_GPIO2, KEYPAD_COL_GPIO3 }

/* Timing (global) */
#define KEYPAD_SCAN_PERIOD_MS  50   /* call interval for KeypadMgr_MainFunction */
#define KEYPAD_DEBOUNCE_TICKS  2    /* debounce: ticks * SCAN_PERIOD (2*50ms = 100ms) */
#define KEYPAD_HOLD_TICKS      20   /* hold threshold (20*50ms = 1000ms) */

/* Event queue depth (max buffered events) */
#define KEYPAD_EVENT_QUEUE_DEPTH 16

/* Event enable flags - used to configure which events each button generates */
#define KEYPAD_EVT_ENABLE_NONE    0x00  /* Button disabled - no events */
#define KEYPAD_EVT_ENABLE_PRESS   0x01  /* Enable PRESS event */
#define KEYPAD_EVT_ENABLE_HOLD    0x02  /* Enable HOLD event */
#define KEYPAD_EVT_ENABLE_RELEASE 0x04  /* Enable RELEASE event */
#define KEYPAD_EVT_ENABLE_ALL     0x07  /* Enable all events (PRESS + HOLD + RELEASE) */

/* Per-button event configuration - define which events each button should generate */
#define KEYPAD_BTN_0_EVENTS      (KEYPAD_EVT_ENABLE_PRESS)                              /* Numbers: only press */
#define KEYPAD_BTN_1_EVENTS      (KEYPAD_EVT_ENABLE_PRESS)
#define KEYPAD_BTN_2_EVENTS      (KEYPAD_EVT_ENABLE_PRESS)
#define KEYPAD_BTN_3_EVENTS      (KEYPAD_EVT_ENABLE_PRESS)
#define KEYPAD_BTN_4_EVENTS      (KEYPAD_EVT_ENABLE_PRESS)
#define KEYPAD_BTN_5_EVENTS      (KEYPAD_EVT_ENABLE_PRESS)
#define KEYPAD_BTN_6_EVENTS      (KEYPAD_EVT_ENABLE_PRESS)
#define KEYPAD_BTN_7_EVENTS      (KEYPAD_EVT_ENABLE_PRESS)
#define KEYPAD_BTN_8_EVENTS      (KEYPAD_EVT_ENABLE_PRESS)
#define KEYPAD_BTN_9_EVENTS      (KEYPAD_EVT_ENABLE_PRESS)

#define KEYPAD_BTN_BACK_EVENTS   (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD)     /* Back: press + hold */
#define KEYPAD_BTN_ERASE_EVENTS  (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD | KEYPAD_EVT_ENABLE_RELEASE) /* Erase: all events */
#define KEYPAD_BTN_ENTER_EVENTS  (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_RELEASE)  /* Enter: press + release */

#define KEYPAD_BTN_UP_EVENTS     (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD | KEYPAD_EVT_ENABLE_RELEASE) /* Navigation: all events */
#define KEYPAD_BTN_DOWN_EVENTS   (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD | KEYPAD_EVT_ENABLE_RELEASE)
#define KEYPAD_BTN_LEFT_EVENTS   (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD | KEYPAD_EVT_ENABLE_RELEASE)
#define KEYPAD_BTN_RIGHT_EVENTS  (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD | KEYPAD_EVT_ENABLE_RELEASE)

/* Macro to create the event configuration array */
#define KEYPAD_EVENT_CONFIG { \
    KEYPAD_BTN_0_EVENTS,      \
    KEYPAD_BTN_1_EVENTS,      \
    KEYPAD_BTN_2_EVENTS,      \
    KEYPAD_BTN_3_EVENTS,      \
    KEYPAD_BTN_4_EVENTS,      \
    KEYPAD_BTN_5_EVENTS,      \
    KEYPAD_BTN_6_EVENTS,      \
    KEYPAD_BTN_7_EVENTS,      \
    KEYPAD_BTN_8_EVENTS,      \
    KEYPAD_BTN_9_EVENTS,      \
    KEYPAD_BTN_BACK_EVENTS,   \
    KEYPAD_BTN_ERASE_EVENTS,  \
    KEYPAD_BTN_ENTER_EVENTS,  \
    KEYPAD_BTN_UP_EVENTS,     \
    KEYPAD_BTN_DOWN_EVENTS,   \
    KEYPAD_BTN_LEFT_EVENTS,   \
    KEYPAD_BTN_RIGHT_EVENTS   \
}

#endif /* KEYPAD_MGR_CFG_H */