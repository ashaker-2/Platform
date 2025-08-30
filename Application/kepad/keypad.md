# 📖 Understanding the KeypadMgr Software Component

The `KeypadMgr` (Keypad Manager) is a crucial software component designed to **handle user input from a physical keypad** in an embedded system. It acts as an intermediary layer between the raw hardware signals from the keypad and other software modules (like the System Manager) that need to react to user actions.

The primary goals of `KeypadMgr` are:
* **Reading physical button states** from a keypad matrix.
* **Debouncing** these noisy physical signals to ensure stable and reliable button presses.
* **Detecting various button events** such as single presses, releases, and sustained holds.
* **Translating these events** into higher-level system events that other components can understand and act upon.

---

## 🏗️ Key Components and Files

The `KeypadMgr` SWC is organized into the following files:

* ### `keypad_mgr.h`
    This is the **public interface** for the `KeypadMgr` module. It defines:
    * `Keypad_Button_ID_t`: An enumeration listing all the **logical buttons** on your keypad (e.g., `KEYPAD_BUTTON_0`, `KEYPAD_BUTTON_MODE_AUTO`, `KEYPAD_BUTTON_UP`).
    * `Keypad_Event_Type_t`: An enumeration describing the **types of events** a button can generate (`PRESS`, `RELEASE`, `HOLD`).
    * `Keypad_Event_t`: A structure to hold details of a specific keypad event (which button and what type of event).
    * Public function prototypes like `KeypadMgr_Init()` (for initialization) and `KeypadMgr_MainFunction()` (the periodic processing function).

* ### `keypad_mgr_cfg.h`
    This header file holds **static configuration parameters** for the keypad. This includes:
    * `KEYPAD_NUM_ROWS`, `KEYPAD_NUM_COLUMNS`: The dimensions of your keypad matrix (e.g., `4x4`).
    * `KEYPAD_ROW_GPIOS`, `KEYPAD_COL_GPIOS`: The **specific GPIO pins** connected to each row and column of your physical keypad.
    * `KEYPAD_SCAN_PERIOD_MS`, `KEYPAD_DEBOUNCE_TICKS`, `KEYPAD_HOLD_TICKS`: Timing parameters for debounce and hold detection.
    * `Keypad_SysMgr_Event_Mapping_t`: A structure and an external array declaration (`g_keypad_sys_mgr_event_map`) that define how a logical button press maps to a specific `SysMgr_Event_t` (System Manager event).

* ### `keypad_mgr_cfg.c`
    This source file provides the **definitions for the static configuration data** declared in `keypad_mgr_cfg.h`. Most notably, it contains the `g_keypad_sys_mgr_event_map` array, which establishes the crucial link between your physical buttons and the actions they trigger in the `SysMgr`.

* ### `keypad_mgr.c`
    This is the **core implementation file** for the `KeypadMgr` module. It contains:
    * Internal data structures (`Keypad_Button_State_t`) to track the raw, debounced, and event-generating state of each button.
    * The `KeypadMgr_MainFunction()` which performs the periodic scanning, debouncing, and event generation.
    * `prv_get_button_id()`: A critical private function that translates a physical row/column intersection into a `Keypad_Button_ID_t`. This is where your keypad's physical wiring layout is logically mapped.
    * `prv_process_button_state()`: The heart of the debouncing and event detection logic for individual buttons.
    * **Mock HAL (Hardware Abstraction Layer) functions**: These are placeholder functions (e.g., `HAL_Keypad_SetRow`, `HAL_Keypad_ReadColumns`) that you'll replace with your actual hardware-specific GPIO control code.

---

## 🚀 How It Works: The Operational Flow

The `KeypadMgr` operates on a periodic basis, typically called from an RTOS task or a main loop.

1.  ### **Initialization (`KeypadMgr_Init`)**
    * When the system starts, `KeypadMgr_Init()` is called once.
    * It initializes internal state variables for all buttons (setting them to a released, un-debounced state).
    * It calls `HAL_Keypad_Init()` to initialize the underlying GPIOs used for the keypad.

2.  ### **Periodic Scan (`KeypadMgr_MainFunction`)**
    This function is called repeatedly at a defined `KEYPAD_SCAN_PERIOD_MS` (e.g., every 50ms).
    * **Row Scanning:** It iterates through each row of the keypad matrix.
        * For the current row, it sets its corresponding GPIO pin to an **active state** (e.g., pulling it low).
        * It then **reads the state of all column GPIO pins**. If a button in the active row is pressed, its corresponding column pin will also be active (e.g., pulled low).
        * After reading, it **deactivates the row GPIO**.
    * **Button ID Mapping (`prv_get_button_id`):** For each row-column intersection, it uses the `prv_get_button_id()` function to determine the logical `Keypad_Button_ID_t`. This function contains the hardcoded map of your physical keypad layout to logical buttons.
    * **State Processing (`prv_process_button_state`):** For each detected button:
        * It captures the **raw physical state** of the button (pressed or released).
        * It applies **debouncing logic**. If the raw state is stable over `KEYPAD_DEBOUNCE_TICKS` scan periods, the button's `debounced_state` is updated.
        * **Event Generation:**
            * If a button transitions from released to debounced-pressed, a `KEYPAD_EVENT_PRESS` is generated.
            * If a button transitions from pressed to debounced-released, a `KEYPAD_EVENT_RELEASE` is generated.
            * If a button remains in the debounced-pressed state for `KEYPAD_HOLD_TICKS` scan periods, a `KEYPAD_EVENT_HOLD` is generated (only once per continuous hold).
        * **Event Forwarding:** For specific buttons (like mode selection buttons, as defined in `g_keypad_sys_mgr_event_map`), the `KeypadMgr` **directly calls `SysMgr_HandleUserEvent()`** to inform the System Manager of the user's intended action. This is a common pattern for critical, direct-action events.

3.  ### **Event Retrieval (Optional: `KeypadMgr_GetLastEvent`)**
    While critical events are often forwarded directly, other modules can query `KeypadMgr_GetLastEvent()` to retrieve the most recent generic keypad event if needed. This event is cleared after being read.

---

## 🛠️ Integration with Your System

To integrate the `KeypadMgr` into your embedded project:

1.  **Replace Mock HAL:** Implement the `HAL_Keypad_Init()`, `HAL_Keypad_SetRow()`, and `HAL_Keypad_ReadColumns()` functions with your board's specific GPIO control code. This is where you'll configure GPIOs as inputs/outputs, set their states, and read their values.
2.  **Verify Button Mapping:** Carefully review the `prv_get_button_id()` function and the `Keypad_Button_ID_t` enum to ensure they accurately reflect the physical layout and desired logical functions of your keypad.
3.  **Define `SysMgr_Event_Mapping_t`:** In `keypad_mgr_cfg.c`, adjust the `g_keypad_sys_mgr_event_map` array to map your `Keypad_Button_ID_t`s to the appropriate `SysMgr_Event_t`s.
4.  **Call `KeypadMgr_MainFunction`:** Ensure this function is called periodically in your RTOS task or main loop with the correct timing (`KEYPAD_SCAN_PERIOD_MS`).

By following these steps, your `KeypadMgr` will reliably capture and translate user input, forming a robust foundation for your system's user interface.



# Keypad 4x4 Button Behavior

## Button Mapping (16 Keys)

| Button      | Short Press (Click)                 | Long Press / Hold                  |
|-------------|-------------------------------------|------------------------------------|
| **0–9**     | Enter digit                         | —                                  |
| **Erase**   | Delete last digit                   | Back / Cancel                      |
| **Enter**   | Validate / Save input               | Open Menu                          |
| **Left**    | Move cursor left                    | Navigate previous menu/screen      |
| **Right**   | Move cursor right                   | Navigate next menu/screen          |
| **Up**      | Move cursor up (next field)         | Fast navigation in config screens  |
| **Down**    | Move cursor down (next field)       | Fast navigation in config screens  |
