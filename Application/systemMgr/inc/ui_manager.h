/**
 * @file ui_manager.h
 * @brief Public API for the User Interface Manager (UI Manager)
 *
 * Provides initialization and periodic main loop functions.
 * Uses state machine internally to handle screens and user interaction.
 */

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize UI Manager.
 *
 * - Clears LCD and resets state machine.
 * - Loads system configuration snapshot for editing.
 * - Displays initial dashboard screen.
 */
void UI_MGR_Init(void);

/**
 * @brief Main periodic function for UI Manager.
 *
 * Must be called every `UI_MGR_MAIN_PERIOD_MS` (e.g. in system tick).
 * - Reads keypad events
 * - Updates screen state
 * - Handles menu timeouts
 */
void UI_MGR_MainFunction(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_MANAGER_H */
