# 📖 Understanding the Enhanced KeypadMgr Software Component

The `KeypadMgr` (Keypad Manager) is a crucial software component designed to **handle user input from a physical keypad** in an embedded system. It acts as an intermediary layer between the raw hardware signals from the keypad and other software modules that need to react to user actions.

The primary goals of the enhanced `KeypadMgr` are:
* **Reading physical button states** from a keypad matrix.
* **Debouncing** these noisy physical signals to ensure stable and reliable button presses.
* **Detecting configurable button events** such as presses, releases, and sustained holds **per button**.
* **Queuing events** in a ring buffer for reliable event consumption.
* **Providing flexible per-button event configuration** without touching core implementation.

---

## 🏗️ Key Components and Files

The enhanced `KeypadMgr` SWC is organized into the following files:

* ### `keypad_mgr.h`
    This is the **public interface** for the `KeypadMgr` module. It defines:
    * `Keypad_Button_ID_t`: An enumeration listing all the **logical buttons** on your keypad (e.g., `KEYPAD_BTN_0`, `KEYPAD_BTN_ENTER`, `KEYPAD_BTN_UP`).
    * `Keypad_Event_Type_t`: An enumeration describing the **types of events** a button can generate (`KEYPAD_EVT_PRESS`, `KEYPAD_EVT_HOLD`, `KEYPAD_EVT_RELEASE`).
    * `Keypad_Event_t`: A structure to hold details of a specific keypad event (which button and what type of event).
    * `Keypad_EventHandler_t`: Callback function prototype for immediate event notification.
    * Public function prototypes like `KeypadMgr_Init()`, `KeypadMgr_MainFunction()`, `KeypadMgr_GetEvent()`, and `KeypadMgr_RegisterEventHandler()`.

* ### `keypad_mgr_cfg.h`
    This header file holds **static configuration parameters** for the keypad. This includes:
    * `KEYPAD_NUM_ROWS`, `KEYPAD_NUM_COLUMNS`: The dimensions of your keypad matrix (e.g., `4x4`).
    * `KEYPAD_ROW_GPIOS`, `KEYPAD_COL_GPIOS`: The **specific GPIO pins** connected to each row and column of your physical keypad.
    * `KEYPAD_SCAN_PERIOD_MS`, `KEYPAD_DEBOUNCE_TICKS`, `KEYPAD_HOLD_TICKS`: Timing parameters for debounce and hold detection.
    * `KEYPAD_EVENT_QUEUE_DEPTH`: Maximum number of events that can be buffered.
    * **Per-button event configuration**: Defines which events (PRESS/HOLD/RELEASE) each button should generate using enable flags.
    * `KEYPAD_EVT_ENABLE_*` flags: Bit masks for configuring event types per button.

* ### `keypad_mgr_cfg.c`
    This source file provides the **definitions for the static configuration data** declared in `keypad_mgr_cfg.h`. Most notably, it contains:
    * `g_keypad_row_gpios[]`, `g_keypad_col_gpios[]`: GPIO pin arrays for hardware abstraction.
    * `g_keypad_rowcol_map[][]`: 2D array mapping physical row/column positions to logical button IDs.
    * `g_keypad_event_config[]`: Array defining which events each button generates (configured via macros).

* ### `keypad_mgr.c`
    This is the **core implementation file** for the enhanced `KeypadMgr` module. It contains:
    * Internal data structures (`KeyBtnState_t`) to track the raw, debounced, and event-generating state of each button.
    * Event queue management with ring buffer (`EvQueue_t`) for reliable event storage and retrieval.
    * The `KeypadMgr_MainFunction()` which performs the periodic scanning, debouncing, and configurable event generation.
    * `map_rowcol_to_button()`: Function that translates a physical row/column intersection into a `Keypad_Button_ID_t`.
    * `is_event_enabled()`: Function that checks if a specific event type is enabled for a given button.
    * `process_button()`: The heart of the debouncing and configurable event detection logic.
    * Hardware abstraction using `HAL_GPIO_SetLevel()` and `HAL_GPIO_GetLevel()` functions.

---

## 🚀 How It Works: The Enhanced Operational Flow

The enhanced `KeypadMgr` operates on a periodic basis, typically called from an RTOS task or a main loop.

1.  ### **Initialization (`KeypadMgr_Init`)**
    * When the system starts, `KeypadMgr_Init()` is called once.
    * It initializes internal state variables for all buttons (setting them to a released, un-debounced state).
    * It initializes the event queue (ring buffer) to empty state.
    * It logs the configuration parameters for debugging.

2.  ### **Periodic Scan (`KeypadMgr_MainFunction`)**
    This function is called repeatedly at a defined `KEYPAD_SCAN_PERIOD_MS` (e.g., every 50ms).
    * **Row Scanning:** It iterates through each row of the keypad matrix.
        * For the current row, it sets its corresponding GPIO pin to an **active state** (pulling it low).
        * It then **reads the state of all column GPIO pins**. If a button in the active row is pressed, its corresponding column pin will also be active (pulled low).
        * After reading, it **deactivates the row GPIO**.
    * **Button ID Mapping:** For each row-column intersection, it uses `map_rowcol_to_button()` to determine the logical `Keypad_Button_ID_t` using the configured mapping table.
    * **Configurable State Processing:** For each detected button:
        * It captures the **raw physical state** of the button (pressed or released).
        * It applies **debouncing logic**. If the raw state is stable over `KEYPAD_DEBOUNCE_TICKS` scan periods, the button's `debounced` state is updated.
        * **Configurable Event Generation:**
            * If a button transitions from released to debounced-pressed **and PRESS events are enabled**, a `KEYPAD_EVT_PRESS` is generated and queued.
            * If a button transitions from pressed to debounced-released **and RELEASE events are enabled**, a `KEYPAD_EVT_RELEASE` is generated and queued.
            * If a button remains in the debounced-pressed state for `KEYPAD_HOLD_TICKS` scan periods **and HOLD events are enabled**, a `KEYPAD_EVT_HOLD` is generated (only once per continuous hold).
        * **Event Queuing:** All generated events are stored in a ring buffer queue for later consumption.

3.  ### **Event Consumption**
    The enhanced system provides flexible event consumption patterns:
    * **Polling Mode (`KeypadMgr_GetEvent`):** Consumer modules can poll for events from the queue.
    * **Callback Mode (`KeypadMgr_RegisterEventHandler`):** Register a callback for immediate event notification.
    * **Queue Management:** The system tracks queue depth and handles overflow by dropping oldest events.

---

## 🛠️ Enhanced Integration with Your System

To integrate the enhanced `KeypadMgr` into your embedded project:

1.  **Configure Per-Button Events:** In `keypad_mgr_cfg.h`, define which events each button should generate:
    ```c
    /* Example configurations */
    #define KEYPAD_BTN_0_EVENTS      (KEYPAD_EVT_ENABLE_PRESS)                    /* Numbers: only press */
    #define KEYPAD_BTN_ERASE_EVENTS  (KEYPAD_EVT_ENABLE_ALL)                     /* Erase: all events */
    #define KEYPAD_BTN_ENTER_EVENTS  (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_RELEASE) /* Enter: press + release */
    #define KEYPAD_BTN_BACK_EVENTS   (KEYPAD_EVT_ENABLE_NONE)                    /* Unused button */
    ```

2.  **Verify Button Mapping:** Review the `g_keypad_rowcol_map[][]` array in `keypad_mgr_cfg.c` to ensure it accurately reflects your physical keypad layout.

3.  **Implement Hardware Layer:** Replace `HAL_GPIO_SetLevel()` and `HAL_GPIO_GetLevel()` with your platform's GPIO functions.

4.  **Consume Events:** Choose your preferred event consumption pattern:
    ```c
    /* Polling mode */
    void main_task(void) {
        KeypadMgr_MainFunction();
        
        Keypad_Event_t event;
        while (KeypadMgr_GetEvent(&event) == E_OK) {
            handle_keypad_event(&event);
        }
    }
    
    /* Callback mode */
    void keypad_callback(const Keypad_Event_t *ev) {
        handle_keypad_event(ev);  /* Keep short! */
    }
    
    void init(void) {
        KeypadMgr_RegisterEventHandler(keypad_callback);
    }
    ```

5.  **Call Periodic Function:** Ensure `KeypadMgr_MainFunction()` is called at the correct interval in your RTOS task or main loop.

---

## 🎯 Enhanced Keypad 4x4 Button Behavior Configuration

The enhanced system allows **per-button configuration** of events. Here are practical examples:

### Configuration Example 1: Basic Numeric Entry

| Button      | Configured Events | Short Press (PRESS)          | Long Press (HOLD)       | Release (RELEASE)     |
|-------------|-------------------|------------------------------|-------------------------|-----------------------|
| **0–9**     | PRESS only        | Enter digit                  | —                       | —                     |
| **Erase**   | PRESS + HOLD      | Delete last digit            | Clear all / Cancel      | —                     |
| **Enter**   | PRESS only        | Validate / Save input        | —                       | —                     |
| **Left**    | NONE              | —                            | —                       | —                     |
| **Right**   | NONE              | —                            | —                       | —                     |
| **Up**      | NONE              | —                            | —                       | —                     |
| **Down**    | NONE              | —                            | —                       | —                     |

```c
/* Configuration in keypad_mgr_cfg.h */
#define KEYPAD_BTN_0_EVENTS      (KEYPAD_EVT_ENABLE_PRESS)
// ... same for 1-9
#define KEYPAD_BTN_ERASE_EVENTS  (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD)
#define KEYPAD_BTN_ENTER_EVENTS  (KEYPAD_EVT_ENABLE_PRESS)
#define KEYPAD_BTN_LEFT_EVENTS   (KEYPAD_EVT_ENABLE_NONE)  /* Unused */
// ... same for RIGHT, UP, DOWN
```

### Configuration Example 2: Advanced Menu Navigation

| Button      | Configured Events | Short Press (PRESS)          | Long Press (HOLD)              | Release (RELEASE)           |
|-------------|-------------------|------------------------------|--------------------------------|-----------------------------|
| **0–9**     | PRESS + HOLD      | Enter digit                  | Quick access function          | —                           |
| **Erase**   | ALL events        | Delete last digit            | Start rapid delete             | Stop rapid delete           |
| **Enter**   | PRESS + RELEASE   | Start action                 | —                              | Confirm action              |
| **Left**    | ALL events        | Move cursor left             | Navigate previous menu/screen  | Stop navigation             |
| **Right**   | ALL events        | Move cursor right            | Navigate next menu/screen      | Stop navigation             |
| **Up**      | ALL events        | Move cursor up               | Fast navigation up             | Stop navigation             |
| **Down**    | ALL events        | Move cursor down             | Fast navigation down           | Stop navigation             |

```c
/* Configuration in keypad_mgr_cfg.h */
#define KEYPAD_BTN_0_EVENTS      (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD)
// ... same for 1-9
#define KEYPAD_BTN_ERASE_EVENTS  (KEYPAD_EVT_ENABLE_ALL)
#define KEYPAD_BTN_ENTER_EVENTS  (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_RELEASE)
#define KEYPAD_BTN_LEFT_EVENTS   (KEYPAD_EVT_ENABLE_ALL)
// ... same for RIGHT, UP, DOWN
```

### Configuration Example 3: Text Input Mode (T9-style)

| Button      | Configured Events | Short Press (PRESS)          | Long Press (HOLD)       | Release (RELEASE)       |
|-------------|-------------------|------------------------------|-------------------------|-------------------------|
| **2 (ABC)** | ALL events        | Cycle A→B→C                  | Insert '2'              | Confirm character       |
| **3 (DEF)** | ALL events        | Cycle D→E→F                  | Insert '3'              | Confirm character       |
| **4 (GHI)** | ALL events        | Cycle G→H→I                  | Insert '4'              | Confirm character       |
| **5 (JKL)** | ALL events        | Cycle J→K→L                  | Insert '5'              | Confirm character       |
| **6 (MNO)** | ALL events        | Cycle M→N→O                  | Insert '6'              | Confirm character       |
| **7(PQRS)** | ALL events        | Cycle P→Q→R→S                | Insert '7'              | Confirm character       |
| **8 (TUV)** | ALL events        | Cycle T→U→V                  | Insert '8'              | Confirm character       |
| **9(WXYZ)** | ALL events        | Cycle W→X→Y→Z                | Insert '9'              | Confirm character       |
| **0**       | PRESS + HOLD      | Insert space                 | Insert '0'              | —                       |
| **1**       | PRESS + HOLD      | Punctuation mode             | Insert '1'              | —                       |
| **Left**    | PRESS + HOLD      | Move cursor left             | Fast cursor left        | —                       |
| **Right**   | PRESS + HOLD      | Move cursor right            | Fast cursor right       | —                       |
| **Up**      | PRESS only        | Toggle uppercase             | —                       | —                       |
| **Down**    | PRESS only        | Switch input mode            | —                       | —                       |
| **Erase**   | PRESS + HOLD      | Delete character             | Delete word/clear       | —                       |
| **Enter**   | PRESS only        | Send/Confirm text            | —                       | —                       |

```c
/* Configuration in keypad_mgr_cfg.h */
#define KEYPAD_BTN_2_EVENTS      (KEYPAD_EVT_ENABLE_ALL)     /* ABC */
#define KEYPAD_BTN_3_EVENTS      (KEYPAD_EVT_ENABLE_ALL)     /* DEF */
// ... same for 4-9
#define KEYPAD_BTN_0_EVENTS      (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD)
#define KEYPAD_BTN_1_EVENTS      (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD)
#define KEYPAD_BTN_LEFT_EVENTS   (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD)
#define KEYPAD_BTN_RIGHT_EVENTS  (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD)
#define KEYPAD_BTN_UP_EVENTS     (KEYPAD_EVT_ENABLE_PRESS)
#define KEYPAD_BTN_DOWN_EVENTS   (KEYPAD_EVT_ENABLE_PRESS)
#define KEYPAD_BTN_ERASE_EVENTS  (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD)
#define KEYPAD_BTN_ENTER_EVENTS  (KEYPAD_EVT_ENABLE_PRESS)
```

---

## 🔧 Configuration Guide

### Event Enable Flags

Use these flags to configure which events each button generates:

```c
#define KEYPAD_EVT_ENABLE_NONE    0x00  /* Button disabled - no events */
#define KEYPAD_EVT_ENABLE_PRESS   0x01  /* Enable PRESS event */
#define KEYPAD_EVT_ENABLE_HOLD    0x02  /* Enable HOLD event */
#define KEYPAD_EVT_ENABLE_RELEASE 0x04  /* Enable RELEASE event */
#define KEYPAD_EVT_ENABLE_ALL     0x07  /* Enable all events */
```

### Per-Button Configuration Syntax

```c
/* Examples of different configurations: */
#define KEYPAD_BTN_X_EVENTS  (KEYPAD_EVT_ENABLE_PRESS)                      /* Only press */
#define KEYPAD_BTN_Y_EVENTS  (KEYPAD_EVT_ENABLE_PRESS | KEYPAD_EVT_ENABLE_HOLD)  /* Press + hold */
#define KEYPAD_BTN_Z_EVENTS  (KEYPAD_EVT_ENABLE_ALL)                        /* All events */
#define KEYPAD_BTN_W_EVENTS  (KEYPAD_EVT_ENABLE_NONE)                       /* Disabled/unused */
```

### Event Consumption Patterns

#### Pattern 1: Polling Mode (Recommended)
```c
void keypad_task(void) {
    KeypadMgr_MainFunction();  /* Scan keypad every 50ms */
    
    Keypad_Event_t event;
    while (KeypadMgr_GetEvent(&event) == E_OK) {
        switch (event.button) {
            case KEYPAD_BTN_ENTER:
                if (event.type == KEYPAD_EVT_PRESS) {
                    start_action();
                } else if (event.type == KEYPAD_EVT_RELEASE) {
                    confirm_action();  /* Only if RELEASE enabled for ENTER */
                }
                break;
            // ... handle other buttons
        }
    }
}
```

#### Pattern 2: Callback Mode
```c
void keypad_callback(const Keypad_Event_t *event) {
    /* Keep this short! Called from scanning context */
    set_event_flag(event->button, event->type);
}

void init(void) {
    KeypadMgr_RegisterEventHandler(keypad_callback);
}
```

---

## 🎯 Enhanced Features and Benefits

### 1. **Flexible Per-Button Configuration**
- Each button can have a unique combination of events
- Unused buttons can be completely disabled (zero overhead)
- Easy reconfiguration without touching core code

### 2. **Reliable Event Management**
- Ring buffer queue prevents event loss
- Configurable queue depth (`KEYPAD_EVENT_QUEUE_DEPTH`)
- Overflow handling (drops oldest events, reports fault)

### 3. **Hardware Abstraction**
- Clean separation between hardware and logic
- Easy porting to different GPIO libraries
- Error handling for hardware failures

### 4. **Advanced Event Types**
- **PRESS**: Immediate response to button activation
- **HOLD**: Sustained action or alternative function access
- **RELEASE**: Action confirmation or mode termination

### 5. **Resource Efficiency**
- Only configured events are processed and queued
- No runtime overhead for disabled buttons/events
- Compile-time configuration validation

---

## 📊 Queue Management

The event queue automatically handles overflow and provides monitoring:

```c
/* Check queue status */
uint8_t pending = KeypadMgr_GetQueuedCount();
if (pending > (KEYPAD_EVENT_QUEUE_DEPTH / 2)) {
    LOGW("Event queue getting full: %d events pending", pending);
}

/* Adjust queue depth if needed */
#define KEYPAD_EVENT_QUEUE_DEPTH 32  /* Increase from default 16 */
```

### Queue Overflow Policy
- When queue is full, **oldest events are dropped** to accept newest events
- This preserves the latest user actions at the cost of older ones
- System monitor fault is reported for capacity monitoring

---

## 🔍 Error Handling and Monitoring

The enhanced `KeypadMgr` includes comprehensive error handling:

* **GPIO Failures**: Continues scanning even if individual GPIO operations fail
* **Queue Overflow**: Drops oldest events and reports fault to system monitor
* **Hardware Abstraction**: Clean error propagation from HAL layer
* **Graceful Degradation**: Partial failures don't stop the entire scanning process

By following this enhanced design, your `KeypadMgr` provides a flexible, reliable, and efficient foundation for user input handling in embedded systems, with the ability to easily reconfigure button behaviors without modifying core implementation code.