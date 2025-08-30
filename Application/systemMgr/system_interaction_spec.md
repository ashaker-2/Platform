# System Interaction Spec (Keypad → SysMgrUX → SysMgr)

## 1. Keypad Manager (KeypadMgr)
- **Responsibilities**  
  - Scan keypad matrix (rows/cols).  
  - Debounce, detect `PRESS / RELEASE / HOLD`.  
  - Map **row/col → button_id → SysMgrUX event**.  
  - Push events into a **queue (depth = 8)**.  
  - Forward events to **SysMgrUX** (not SysMgr directly).  

- **Behavior**  
  - **PRESS** → generates event.  
  - **HOLD** → generates single event after threshold (no auto-repeat).  
  - **RELEASE** → does **not** notify SysMgrUX.  
  - If HAL GPIO error → raise fault to SysMon, but continue scanning.  

---

## 2. User Experience / Menu Manager (SysMgrUX)
- **Responsibilities**  
  - Consume **all events** from KeypadMgr (not just last).  
  - Handle menu navigation, timeouts, and user input flows.  
  - Decide **when to notify SysMgr** with confirmed data.  
  - Apply default values on timeout if needed.  

- **Event Handling Rules**  
  - Logs only on **state changes** (not every scan).  
  - Mode selection / thresholds are **confirmed only on Enter**.  
  - Timeout → discard unconfirmed entries (for thresholds) or apply defaults (for schedules).  

---

## 3. System Manager (SysMgr)
- **Responsibilities**  
  - Maintain **system state and configuration**.  
  - React to **confirmed events** from SysMgrUX only.  
  - Ignore low-level key events.  

---

## 4. Scenarios

### Scenario 1: Thresholds (Temp / Humidity)
1. User presses **Enter** → `SysMgrUX: OpenMenu`.  
2. Navigate → **Set Threshold**.  
3. User enters new threshold.  
4. Press **Enter** → `SysMgrUX` validates → **notify SysMgr** (`SysMgr_Event_SetTempThreshold` / `SysMgr_Event_SetHumThreshold`).  
5. If no Enter → timeout → return to main screen (SysMgr not notified).  

---

### Scenario 2: Mode = Automatic
1. User presses **Enter** → `SysMgrUX: OpenMenu`.  
2. Navigate → select **Automatic**.  
3. User presses **Enter** → confirm.  
4. **SysMgrUX → SysMgr**: `SysMgr_Event_ModeChange_Auto`.  
5. SysMgr switches immediately.  

---

### Scenario 3: Mode = Manual / Hybrid
1. User presses **Enter** → `SysMgrUX: OpenMenu`.  
2. Navigate → select **Manual** or **Hybrid**.  
3. User presses **Enter** → confirm.  
4. **SysMgrUX enters scheduling flow**:  
   - Collect actuator **On/Off times**.  
   - On each **Enter confirm** → `SysMgr_Event_SetActuatorSchedule`.  
5. If user confirms all → SysMgrUX sends final `SysMgr_Event_ModeChange_Manual` / `Hybrid`.  
6. If timeout before entry → SysMgrUX falls back to defaults → notifies SysMgr with mode change + defaults.  

---

## 5. Design Rules

- **AUTO mode** → SysMgr notified **only after Enter confirm**.  
- **MANUAL / HYBRID** → SysMgr notified after Enter + schedules OR timeout → defaults.  
- **Thresholds** → SysMgr notified only after Enter confirm.  
- **Unmapped keys** → ignored silently (may be used in future).  
- **Event Queue depth** = 8.  
- **Global debounce/hold timing** applies to all buttons.  

---

👉 This way, **KeypadMgr stays low-level**, **SysMgrUX owns UX flows**, and **SysMgr only sees confirmed, meaningful events**.



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
