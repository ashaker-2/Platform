/**
 * @file keypad_mgr.c
 * @brief Implementation file for the Keypad Manager (KeypadMgr) module.
 */

#include "keypad_mgr.h"
#include "keypad_mgr_cfg.h" // For KEYPAD_NUM_ROWS, KEYPAD_NUM_COLUMNS, etc., and GPIOs
#include "logger.h"         // Assuming a logger utility for debug/info/error messages
#include "hal_gpio.h"
#include <string.h> // For memset

// --- Private Defines ---
#define TAG "KEYPAD_MGR"

// --- Private Data Structures ---
/**
 * @brief Internal state for each physical button.
 */
typedef struct
{
    bool current_physical_state;         /**< Raw, un-debounced state (true if pressed) */
    bool debounced_state;                /**< Debounced state (true if pressed) */
    uint16_t debounce_counter;           /**< Counter for debounce logic */
    uint16_t hold_counter;               /**< Counter for hold event detection */
    Keypad_Event_Type_t last_event_type; /**< Last event generated for this button */
} Keypad_Button_State_t;

// --- Private Global Variables ---
static Keypad_Button_State_t g_button_states[KEYPAD_BUTTON_MAX];
static bool g_keypad_mgr_initialized = false;
static Keypad_Event_t g_last_keypad_event = {.button_id = KEYPAD_BUTTON_MAX, .event_type = KEYPAD_EVENT_NONE};

// Array of row GPIOs for iteration (defined in keypad_mgr_cfg.h)
static const uint8_t g_keypad_row_gpios[KEYPAD_NUM_ROWS] = KEYPAD_ROW_GPIOS;
// Array of column GPIOs for iteration (defined in keypad_mgr_cfg.h)
static const uint8_t g_keypad_col_gpios[KEYPAD_NUM_COLUMNS] = KEYPAD_COL_GPIOS;

// --- Private Function Prototypes ---
static Keypad_Button_ID_t prv_get_button_id(uint8_t row_idx, uint8_t col_idx);
static void prv_process_button_state(Keypad_Button_ID_t button_id, bool raw_state);
/**
 * @brief Sets the state of a specific keypad row GPIO.
 * @param row_gpio The GPIO pin number for the row.
 * @param state True for active (e.g., LOW to sink current), false for inactive (e.g., HIGH/Hi-Z).
 * @return Status_t E_OK on success.
 */
static Status_t Keypad_SetRow(uint8_t row_gpio, bool state);

/**
 * @brief Reads the state of all keypad column GPIOs.
 * @param column_states_out Pointer to an array to store column states (true if active).
 * @return Status_t E_OK on success.
 */
static Status_t Keypad_ReadColumns(bool column_states_out[KEYPAD_NUM_COLUMNS]);

// --- Public Function Implementations ---

Status_t KeypadMgr_Init(void)
{
    if (g_keypad_mgr_initialized)
    {
        LOGW(TAG, "Keypad Manager already initialized.");
        return E_ALREADY_INITIALIZED;
    }

    LOGI(TAG, "Initializing Keypad Manager...");

    // Initialize all button states
    memset(g_button_states, 0, sizeof(g_button_states));
    for (int i = 0; i < KEYPAD_BUTTON_MAX; i++)
    {
        g_button_states[i].last_event_type = KEYPAD_EVENT_NONE;
    }

    g_keypad_mgr_initialized = true;
    LOGI(TAG, "Keypad Manager initialized successfully. %d Rows, %d Columns.", KEYPAD_NUM_ROWS, KEYPAD_NUM_COLUMNS);
    return E_OK;
}

void KeypadMgr_MainFunction(void)
{
    if (!g_keypad_mgr_initialized)
    {
        LOGW(TAG, "MainFunction called before initialization.");
        return;
    }

    bool column_states[KEYPAD_NUM_COLUMNS];

    // Iterate through each row
    for (uint8_t row_idx = 0; row_idx < KEYPAD_NUM_ROWS; row_idx++)
    {
        // Set current row GPIO to active (e.g., LOW)
        Keypad_SetRow(g_keypad_row_gpios[row_idx], true);

        // Read column states
        Status_t status = Keypad_ReadColumns(column_states);
        if (status != E_OK)
        {
            LOGE(TAG, "Failed to read columns for row GPIO%d. Status: %d", g_keypad_row_gpios[row_idx], status);
            // Consider fault reporting via SysMon here
            // SYSMGR_REPORT_FAULT_STATUS(SYSMON_FAULT_KEYPAD_READ_ERROR, SYSMON_FAULT_STATUS_ACTIVE);
            Keypad_SetRow(g_keypad_row_gpios[row_idx], false); // Deactivate row before continuing
            continue;
        }

        // Process each column for the current row
        for (uint8_t col_idx = 0; col_idx < KEYPAD_NUM_COLUMNS; col_idx++)
        {
            Keypad_Button_ID_t button_id = prv_get_button_id(row_idx, col_idx);
            if (button_id < KEYPAD_BUTTON_MAX)
            { // Check if it's a valid mapped button
                prv_process_button_state(button_id, column_states[col_idx]);
            }
        }

        // Deactivate current row GPIO (e.g., set HIGH/Hi-Z)
        Keypad_SetRow(g_keypad_row_gpios[row_idx], false);
    }
}

Status_t KeypadMgr_GetLastEvent(Keypad_Event_t *event_out)
{
    if (!g_keypad_mgr_initialized)
        return E_NOT_INITIALIZED;
    if (event_out == NULL)
        return E_NULL_POINTER;

    if (g_last_keypad_event.event_type == KEYPAD_EVENT_NONE)
    {
        return E_DATA_STALE; // No new event
    }

    *event_out = g_last_keypad_event;
    g_last_keypad_event.event_type = KEYPAD_EVENT_NONE; // Clear event after reading
    return E_OK;
}

// --- Private Function Implementations ---

/**
 * @brief Maps a physical row and column index to a logical Keypad_Button_ID_t.
 * This function defines your specific keypad's wiring layout and its logical button mapping.
 *
 * | Keypad physical layout assumption:
 * |          | Col0 (GPIO36)  | Col1 (GPIO37)  | Col2 (GPIO38)  | Col3 (GPIO39)  |
 * | :------- | :------------- | :------------- | :------------- | :------------- |
 * | R0 (GP4) | BUTTON_0       | BUTTON_1       | BUTTON_2       | BUTTON_3       |
 * | R1 (GP12)| MODE_AUTO      | MODE_HYBRID    | MODE_MANUAL    | UP             |
 * | R2 (GP13)| DOWN           | ENTER          | (Unmapped)     | (Unmapped)     |
 * | R3 (GP0) | (Unmapped)     | (Unmapped)     | (Unmapped)     | (Unmapped)     |
 *
 * @param row_idx The row index (0 to KEYPAD_NUM_ROWS-1).
 * @param col_idx The column index (0 to KEYPAD_NUM_COLUMNS-1).
 * @return The corresponding Keypad_Button_ID_t, or KEYPAD_BUTTON_MAX if no mapping.
 */
static Keypad_Button_ID_t prv_get_button_id(uint8_t row_idx, uint8_t col_idx)
{
    // Mapping logic based on the assumed physical layout and Keypad_Button_ID_t enum
    if (row_idx == 0)
    {
        if (col_idx == 0)
            return KEYPAD_BUTTON_0;
        if (col_idx == 1)
            return KEYPAD_BUTTON_1;
        if (col_idx == 2)
            return KEYPAD_BUTTON_2;
        if (col_idx == 3)
            return KEYPAD_BUTTON_3;
    }
    else if (row_idx == 1)
    {
        if (col_idx == 0)
            return KEYPAD_BUTTON_MODE_AUTO;
        if (col_idx == 1)
            return KEYPAD_BUTTON_MODE_HYBRID;
        if (col_idx == 2)
            return KEYPAD_BUTTON_MODE_MANUAL;
        if (col_idx == 3)
            return KEYPAD_BUTTON_UP;
    }
    else if (row_idx == 2)
    {
        if (col_idx == 0)
            return KEYPAD_BUTTON_DOWN;
        if (col_idx == 1)
            return KEYPAD_BUTTON_ENTER;
    }
    // For any unmapped row/col combination
    return KEYPAD_BUTTON_MAX;
}

/**
 * @brief Processes the raw state of a single button, handling debouncing and events.
 * @param button_id The ID of the button to process.
 * @param raw_state The raw physical state of the button (true if pressed).
 */
static void prv_process_button_state(Keypad_Button_ID_t button_id, bool raw_state)
{
    Keypad_Button_State_t *btn_state = &g_button_states[button_id];

    btn_state->current_physical_state = raw_state;

    // Debounce logic
    if (raw_state != btn_state->debounced_state)
    {
        btn_state->debounce_counter++;
        if (btn_state->debounce_counter >= KEYPAD_DEBOUNCE_TICKS)
        {
            btn_state->debounced_state = raw_state;
            btn_state->debounce_counter = 0;
            btn_state->hold_counter = 0; // Reset hold counter on state change

            // State change detected (Press or Release)
            if (btn_state->debounced_state)
            {
                LOGD(TAG, "Button %d: PRESS event", button_id);
                g_last_keypad_event = (Keypad_Event_t){.button_id = button_id, .event_type = KEYPAD_EVENT_PRESS};

                // Check mapping and send to SysMgr if it's a mode select button
                for (uint8_t i = 0; i < g_keypad_sys_mgr_event_map_size; i++)
                {
                    if (g_keypad_sys_mgr_event_map[i].button_id == button_id &&
                        g_keypad_sys_mgr_event_map[i].is_mode_select_button &&
                        g_keypad_sys_mgr_event_map[i].event_id != Keypad_Event_NONE)
                    {
                        // SysMgr_HandleUserEvent(g_keypad_sys_mgr_event_map[i].event_id);
                        LOGI(TAG, "Button %d (Mode Select) triggered SysMgr Event: %d", button_id, g_keypad_sys_mgr_event_map[i].event_id);
                        break;
                    }
                }
            }
            else // Released
            {
                LOGD(TAG, "Button %d: RELEASE event", button_id);
                g_last_keypad_event = (Keypad_Event_t){.button_id = button_id, .event_type = KEYPAD_EVENT_RELEASE};
            }
            btn_state->last_event_type = g_last_keypad_event.event_type;
        }
    }
    else
    {
        // No state change, reset debounce counter
        btn_state->debounce_counter = 0;

        // If button is currently debounced-pressed, check for hold event
        if (btn_state->debounced_state)
        {
            btn_state->hold_counter++;
            if (btn_state->hold_counter >= KEYPAD_HOLD_TICKS &&
                btn_state->last_event_type != KEYPAD_EVENT_HOLD)
            {
                LOGD(TAG, "Button %d: HOLD event", button_id);
                g_last_keypad_event = (Keypad_Event_t){.button_id = button_id, .event_type = KEYPAD_EVENT_HOLD};
                btn_state->last_event_type = KEYPAD_EVENT_HOLD;
                // A hold event might also trigger a SysMgr event, depending on requirements.
                // For mode selection, a 'press' is usually sufficient.
            }
        }
        else
        {
            // Button is debounced-released, reset hold counter
            btn_state->hold_counter = 0;
        }
    }
}

/**
 * @brief Sets the state of a specific keypad row GPIO.
 * @param row_gpio The GPIO pin number for the row.
 * @param state True for active (e.g., LOW to sink current), false for inactive (e.g., HIGH).
 * @return Status_t E_OK on success.
 */
static Status_t Keypad_SetRow(uint8_t row_gpio, bool state)
{
    // --- REPLACE with your MCU's GPIO write function ---
    HAL_GPIO_SetLevel(row_gpio, state ? 0 : 1);
    LOGD("HAL_KEYPAD", "Set Row GPIO%d to %s (Physical Level: %s)", row_gpio, state ? "ACTIVE" : "INACTIVE", state ? "LOW" : "HIGH");
    return E_OK;
}

/**
 * @brief Reads the state of all keypad column GPIOs.
 * @param column_states_out Pointer to an array to store column states (true if active).
 * @return Status_t E_OK on success.
 */
static Status_t Keypad_ReadColumns(bool column_states_out[KEYPAD_NUM_COLUMNS])
{
    if (column_states_out == NULL)
        return E_NULL_POINTER;
    uint8_t u8PinState = 0;
    for (uint8_t i = 0; i < KEYPAD_NUM_COLUMNS; i++)
    {
        uint8_t gpio_pin = g_keypad_col_gpios[i];
        // --- REPLACE with your MCU's GPIO read function ---
        // A column is "active" if it's pulled LOW by a pressed button.
        HAL_GPIO_GetLevel(gpio_pin, &u8PinState);
        column_states_out[i] = (u8PinState == 0);
        LOGD("HAL_KEYPAD", "Read Col GPIO%d: %s", gpio_pin, column_states_out[i] ? "ACTIVE (LOW)" : "INACTIVE (HIGH)");
    }
    return E_OK;
}
