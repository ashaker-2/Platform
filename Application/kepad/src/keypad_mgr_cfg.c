/**
 * @file keypad_mgr_cfg.c
 * @brief Enhanced board-specific keypad mapping and event configuration.
 *
 * This file exposes arrays of row/column GPIOs, button mapping table,
 * and per-button event configuration. Keeping it here makes layout
 * and behavior changes possible without touching core keypad logic.
 *
 * NOTE: KeypadMgr remains generic and only emits configured events per button.
 * The mapping and event config below are EXAMPLE layouts; change to reflect your needs.
 */

#include "keypad_mgr_cfg.h"

/* Expose arrays used by keypad_mgr.c (initialized from macros) */
const uint8_t g_keypad_row_gpios[KEYPAD_NUM_ROWS] = KEYPAD_ROW_GPIOS;
const uint8_t g_keypad_col_gpios[KEYPAD_NUM_COLUMNS] = KEYPAD_COL_GPIOS;

/* Row/col -> button map (example layout):
 *
 * R0: C0->0  C1->1  C2->2   C3->3
 * R1: C0->4  C1->5  C2->6   C3->7
 * R2: C0->8  C1->9  C2->ERASE C3->ENTER
 * R3: C0->UP C1->DOWN C2->LEFT C3->RIGHT
 */
const Keypad_Button_ID_t g_keypad_rowcol_map[KEYPAD_NUM_ROWS][KEYPAD_NUM_COLUMNS] = {
    { KEYPAD_BTN_0, KEYPAD_BTN_1, KEYPAD_BTN_2, KEYPAD_BTN_3 },
    { KEYPAD_BTN_4, KEYPAD_BTN_5, KEYPAD_BTN_6, KEYPAD_BTN_7 },
    { KEYPAD_BTN_8, KEYPAD_BTN_9, KEYPAD_BTN_ERASE, KEYPAD_BTN_ENTER },
    { KEYPAD_BTN_UP, KEYPAD_BTN_DOWN, KEYPAD_BTN_LEFT, KEYPAD_BTN_RIGHT }
};

/* Per-button event configuration - defines which events each button generates */
const uint8_t g_keypad_event_config[KEYPAD_BTN_MAX] = KEYPAD_EVENT_CONFIG;