# 📋 UI Specification

## 1. UI Menu Hierarchy (2×16 LCD)

``` markdown
Main Screens (rotates with LEFT/RIGHT, default timeout to Screen1):
1. System Average
   L1: "Avg T:24.5C H:55%"
   L2: "Mode:AUTO 12:34"

2. Per-Sensor Details
   NA

3. Actuator States
   L1: "Fan:ON Heat:OFF"
   L2: "Pump:OFF Vent:ON"

4. Mode + Time
   L1: "Mode:AUTO"
   L2: "Time:12:34"

5. Config Menu (ENTER HOLD from any screen)
   L1: "1:Temp 2:Hum 3:PerSens"
   L2: "4:Fan 5:Ven 6:Pump"
       "7:Heater 8:Light"
```

------------------------------------------------------------------------

## 2. 🌀 UI State Machine (Text Diagram)

``` markdown
[Main Screens Loop]
 ┌──────────────┐
 | Screen 1 Avg | ←→ LEFT/RIGHT → 
 └──────────────┘
        ↓
 ┌──────────────┐
 | Screen 2 Per | 
 └──────────────┘
        ↓
 ┌──────────────┐
 | Screen 3 Act | 
 └──────────────┘
        ↓
 ┌──────────────┐
 | Screen 4 M+T | 
 └──────────────┘

ENTER HOLD (anywhere) → Config Menu

=====================================================
Config Menu Options
1 → Global Temp Thresholds
   - "Set Tmin:___"
   - "Set Tmax:___"

2 → Global Hum Thresholds
   - "Set Hmin:___"
   - "Set Hmax:___"

3 → Per-Sensor Config
   - "Not Implemented"

4–7 → Actuator Config (Fan, Ven, Pump, Heater)
   Step 1: "Xn Mode? 1:Auto 2:Man"
   Step 2 (if Manual): 
      L1: "Xn ON:___s"
      L2: "OFF:___s"
      - ENTER hold → toggle units (s/m/h)
      - CURSOR keys → switch ON/OFF fields
      - HOLD LEFT/RIGHT → switch actuators

8 → Light Schedule
   L1: "ON:HH:MM"
   L2: "OFF:HH:MM"
   - Validations: 0 ≤ HH ≤ 23, 0 ≤ MM ≤ 59, ON < OFF
```

------------------------------------------------------------------------

## 3. 📑 State Transition Table (Matrix)

  ----------------------------------------------------------------------------
  **Current    **Key        **Next        **Action**
  State**      Press**      State**       
  ------------ ------------ ------------- ------------------------------------
  Main Screen  LEFT/RIGHT   Prev/Next     Rotate between main screens
  (1--4)                    Screen        

  Main Screen  ENTER HOLD   Config Menu   Enter configuration
  (any)                                   

  Config Menu  BACK (short) Main Screen 1 Exit without saving

  Config Menu  1            Temp Config   Prompt Tmin
                            (Tmin input)  

  Temp Config  NUM          Temp Config   Append digit
  (Tmin)                    (Tmin input)  

  Temp Config  BACK short   Temp Config   Erase digit
  (Tmin)                    (Tmin input)  

  Temp Config  BACK hold    Config Menu   Cancel
  (Tmin)                                  

  Temp Config  ENTER short  Temp Config   Confirm Tmin
  (Tmin)                    (Tmax input)  

  Temp Config  ENTER short  Config Menu   Save Tmin/Tmax
  (Tmax)                                  

  Config Menu  2            Hum Config    Prompt Hmin
                            (Hmin input)  

  Hum Config   ENTER short  Config Menu   Save Hmin/Hmax
  (Hmax)                                  

  Config Menu  3            Per-Sensor    Show "Not Implemented"
                            Config        

  Config Menu  4--7         Actuator Mode Choose actuator
                            Select        

  Actuator     1            Config Menu   Set AUTO → return
  Mode Select                             

  Actuator     2            Actuator      Go to ON/OFF input screen
  Mode Select               Manual ON/OFF 

  Actuator     NUM          Same          Append digit
  Manual                                  
  ON/OFF                                  

  Actuator     BACK short   Same          Erase digit
  Manual                                  
  ON/OFF                                  

  Actuator     BACK hold    Config Menu   Cancel
  Manual                                  
  ON/OFF                                  

  Actuator     ENTER short  Config Menu   Save ON/OFF
  Manual                                  
  ON/OFF                                  

  Actuator     ENTER hold   Same          Toggle unit (s/m/h)
  Manual                                  
  ON/OFF                                  

  Actuator     LEFT/RIGHT   Same          Move cursor between ON/OFF fields
  Manual                                  
  ON/OFF                                  

  Actuator     HOLD         Next/Prev     Switch actuator
  Manual       LEFT/RIGHT   Actuator      
  ON/OFF                                  

  Config Menu  8            Light         Prompt ON:HH:MM
                            Schedule      

  Light        NUM          Same          Append digit
  Schedule (ON                            
  time)                                   

  Light        BACK short   Same          Erase digit
  Schedule (ON                            
  time)                                   

  Light        BACK hold    Config Menu   Cancel
  Schedule (ON                            
  time)                                   

  Light        ENTER short  OFF time      Move to OFF time
  Schedule (ON              input         
  time)                                   

  Light        ENTER short  Config Menu   Save ON/OFF times (validate: ON \<
  Schedule                                OFF)
  (OFF time)                              

  Any Config   Timeout      Main Screen 1 Cancel unsaved & return to dashboard
  State                                   
  ----------------------------------------------------------------------------
