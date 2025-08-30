/**
 * @file keypad_mgr_cfg.h
 * @brief KeypadMgr configuration (GPIOs, timing, queue depth)
 *
 * Edit pin numbers and defines to match your hardware/board.
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
#define KEYPAD_EVENT_QUEUE_DEPTH 8

#endif /* KEYPAD_MGR_CFG_H */
