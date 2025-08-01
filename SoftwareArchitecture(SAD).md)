# Software Architecture Document (SAD) 

## 1. Introduction

### 1.1. Purpose

This Software Architecture Document (SAD) describes the high-level design and structure of the firmware for a the Environmental Monitoring & Control System.

The primary purpose of this document is to provide a clear, comprehensive, and consistent overview of the system's architecture, detailing its components, their interactions, and the key design decisions. It serves as a foundational reference for development, testing, maintenance, and future enhancements.

### 1.2. Scope

The scope of this document encompasses the entire software stack of the Environmental Monitoring & Control System firmware, from the hardware abstraction layer to the application logic and external communication services. It covers:

* The overall system structure and its decomposition into logical layers and modules.

* The runtime behavior, including OS tasks, their responsibilities, and inter-task communication mechanisms.

* The organization of the codebase within the file system and the build process.

* The mapping of software components to the underlying hardware.

* Key architectural design principles, such as modularity, real-time performance, and scalability.

This document does **not** cover specific low-level driver implementations (e.g., how an I2C transaction is performed at the bit level) or detailed algorithms within individual modules, which would be found in their respective design documents.

### 1.3. Definitions, Acronyms, and Abbreviations

* **SAD**: Software Architecture Document

* **RTE**: Runtime Environment

* **HAL**: ECU Abstraction Layer (or Embedded Control Unit Abstraction Layer)

* **MCU**: Microcontroller Unit

* **OS**: Operating System (FreeRTOS in this case)

* **BLE**: Bluetooth Low Energy

* **Wi-Fi**: Wireless Fidelity

* **Modbus**: A serial communication protocol

* **HWM**: High Water Mark (for stack usage)

* **CPU Load**: Percentage of CPU time spent on active tasks vs. idle task.

* **APP_OK / APP_ERROR**: Standard return codes for success/failure.

### 1.4. References

* IEEE 1471-2000 (Recommended Practice for Architectural Description of Software-Intensive Systems) - Conceptual basis for architectural views.

* FreeRTOS Documentation (for RTOS concepts and APIs).

* ESP-IDF Programming Guide (for ESP32-specific details).

## 2. Architectural Goals and Constraints

### 2.1. Goals

* **Modularity and Decoupling**: Achieve high component modularity and low coupling between modules. This is primarily enforced by the **Runtime Environment (RTE)** pattern, ensuring that modules interact exclusively through well-defined service interfaces.

* **Maintainability**: The clear layering, modular design, and consistent communication mechanisms aim to make the codebase easy to understand, debug, and modify over its lifecycle.

* **Real-Time Performance**: Utilize FreeRTOS to manage concurrent tasks with defined priorities and periodicities, ensuring that critical operations meet their deadlines and the system remains responsive.

* **Scalability**: The architecture should easily accommodate future expansion, such as adding new sensors, actuators, communication protocols, or application features, with minimal impact on existing code.

* **Testability**: Decoupled modules and well-defined interfaces facilitate unit testing and integration testing.

* **Resource Efficiency**: Design for efficient use of microcontroller resources (CPU, RAM, Flash) inherent to embedded systems.

### 2.2. Constraints

* **FreeRTOS Operating System**: All concurrency and task management must adhere to FreeRTOS principles and APIs.

* **Microcontroller Platform**: The design assumes an ESP32-like microcontroller, leveraging its peripherals and development framework (e.g., ESP-IDF).

* **Communication Protocols**: Mandatory support for Modbus and Bluetooth (BLE), with optional Wi-Fi.

* **Limited Resources**: Embedded nature implies constraints on memory (RAM, Flash) and processing power.

* **C Programming Language**: The entire firmware is implemented in C.

## 3. Architectural Representation: The 4+1 View Model

The system architecture is described using the "4+1" view model, providing different perspectives for various stakeholders.

### 3.1. Logical View

The logical view describes the system's functional decomposition, key abstractions, and how components are structured into layers.

#### 3.1.1. Layered Architecture

The system employs a strict layered architecture to achieve separation of concerns and reduce interdependencies.

**(Conceptual Diagram: Layered Architecture)**

```
+--------------------------------------------------------------+
|                   Application Layer                          |
| +-----------+  +--------------+  +--------+  +-----------+   |
| |  SYS_MGR  |  |  SYS_STARTUP |  | Heater |  | Humadity  |   |
+--------------------------------------------------------------|
| +---------+  +------------+  +-----------+  +--------------+ |
| | FAN_CTL |  | Diagnostic |  | Light_CTL |  |SystemMonitor | |
+--------------------------------------------------------------|
| +-----------+  +-----------+  +-----------+  +-----------+   |
| |  common   |  |  Display  |  | TEMP_SENS |  | Logger    |   |
+--------------------------------------------------------------|
| +--------+  +--------+  +------------+  +-----------+        |
| |  power |  |  pump  |  | ventilator |  | Light_Indication | |
+--------------------------------------------------------------|
+--------------------------------------------------------------+
|                  Runtime Environment (RTE)                   |
+--------------------------------------------------------------+
|                        Service Layer                         |
| +-----------+          +-----------+          +-----------+  |
| |     OS    |          |  ComM     |          | MODBUS_MW |  |
+--------------------------------------------------------------+
|                   Hardware Abstraction Layer (HAL)           |
| +-----------+  +-----------+  +-----------+  +-----------+   |
| | HAL_GPIO  |  | HAL_PWM   |  | HAL_ADC   |  | HAL_UART  |   |
+--------------------------------------------------------------+
| +-----------+  +-----------+  +-----------+  +-----------+   |
| | HAL_TIMER |  | HAL_RTC   |  | ECUAL_I2C |  | ECUAL_UART|   |
+--------------------------------------------------------------+
|                       MCAL Hardware                          |
| +------+     +-----+     +-----+      +-----+    +-----+     |
| | GPIO |     | PWM |     | ADC |      | I2C |    | spi |     |
+--------------------------------------------------------------+
| +------+     +-----+     +-----+      +------+               |
| | UART |     | RTC |     | BLE |      | WIFI |               |
+--------------------------------------------------------------+
```



#### 3.1.2. Layer Descriptions

* **Application Layer**:

  * **Purpose**: Implements the core business logic and high-level functionalities of the Environmental Monitoring & Control System.

  * **Modules**:

    * `sys_mgr`: The central control logic. Manages operational parameters, applies control algorithms (e.g., fan staging, heater control, pump control), and determines overall system state and alarms. It is the "brain" of the application.

    * `fan`, `heater`, `pump`, `ventilator`, `light_control`: Modules encapsulating the control logic for specific actuators. They expose functions like `SetSpeed()`, `On()`, `Off()`.

    * `temp_sensor`, `humidity_sensor`: Modules for reading data from specific sensor types. They expose functions like `ReadTemperature()`, `ReadHumidity()`.

    * `character_display`: Manages interaction with character-based LCDs.

    * `light_indication`: Controls indicator LEDs.

    * `logger`: Provides a standardized logging mechanism for all layers.

    * `system_monitor`: Calculates and provides system health metrics (CPU load, stack usage).

  * **Interaction**: Modules in this layer **do not** directly call functions in other application modules or middleware. All inter-module communication is exclusively routed through **RTE Service Functions**. They can, however, directly call functions within their own module or common utilities like `logger`.

* **Runtime Environment (RTE) Layer**:

  * **Purpose**: Acts as the central communication hub and task manager for the entire application. It decouples application modules from each other and from the middleware.

  * **Components**:

    * **RTE Service Functions**: A set of public functions (`RTE_Service_ModuleName_FunctionName()`) that serve as the *only* allowed interface for modules to interact. When a module needs to call another module's function, it calls the corresponding RTE Service, which then internally calls the target module's actual function. This enforces strict decoupling.

    * **FreeRTOS Tasks**: The RTE is responsible for creating and managing all FreeRTOS tasks in the system, including initialization tasks and permanent application tasks.

  * **Interaction**: The RTE directly calls functions in the Application Layer and Service Layer. Application modules call RTE Services.

* **Service Layer**:

  * **Purpose**: Provides high-level protocol implementations for external communication, abstracting the complexities of network and serial protocols.

  * **Modules**:

    * `modbus_middleware`: Implements the Modbus RTU/TCP protocol logic (e.g., register mapping, request/response handling). It interacts with `ECUAL_UART` or network drivers.

    * `bluetooth_middleware`: Manages Bluetooth (BLE/Classic) advertising, connections, and GATT services/characteristics. It interacts with native Bluetooth drivers.

    * `wifi_middleware`: Handles Wi-Fi connectivity (STA/AP mode) and higher-level network protocols (e.g., HTTP, MQTT). It interacts with native Wi-Fi drivers.

  * **Interaction**: Service modules are primarily called by the `COMMUNICATION_STACK_MainTask` (which is part of RTE). They interact with the underlying HAL/Driver layer. When they need to provide data or commands to the application, they do so via RTE Service calls.

* **HW Abstraction Layer (HAL)**:

  * **Purpose**: Provides a hardware-agnostic interface to the microcontroller's peripherals. It abstracts away the low-level register access and specific MCU details.

  * **Modules**: `ecual_gpio`, `ecual_pwm`, `ecual_adc`, `ecual_i2c`, `ecual_uart`, `ecual_common` (for uptime, etc.).

  * **Interaction**: HAL modules are called directly by application modules (e.g., `fan.c` calls `ECUAL_PWM_SetDutyCycle()`) or middleware modules (e.g., `modbus_middleware.c` calls `ECUAL_UART_Read()`).

### 3.2. Process View

The process view describes the system's runtime behavior, focusing on the FreeRTOS tasks, their responsibilities, priorities, and communication mechanisms.

#### 3.2.1. Task Breakdown

All tasks are created and managed by the RTE. Priorities are assigned to ensure real-time responsiveness for critical functions.

* **`RTE_HwInitTask`**

  * **Responsibility**: Initializes all modules in the HAL layer.

  * **Periodicity**: Runs once at system startup.

  * **Priority**: `configMAX_PRIORITIES - 1` (Very High)

  * **Lifecycle**: Self-deletes after completing initialization.

* **`RTE_AppInitTask`**

  * **Responsibility**: Initializes all modules in the Application Layer (including `sys_mgr`, `system_monitor`, and `COM_MGR_Init`). Configures initial system parameters (e.g., operational ranges). Creates and starts all permanent application tasks.

  * **Periodicity**: Runs once after `RTE_HwInitTask` completes.

  * **Priority**: `configMAX_PRIORITIES - 2` (High)

  * **Lifecycle**: Self-deletes after completing initialization and starting permanent tasks.

* **`RTE_App_20ms_SensorReadTask`**

  * **Responsibility**: 
    * Periodically reads all environmental sensors (temperature, humidity, heatsink) via RTE services. It then uses RTE services to update the `sys_mgr` with the latest readings.

  * **Periodicity**: 20 ms

  * **Priority**: 6 (High)

* **`RTE_App_100ms_ActuatorControlTask`**

  * **Responsibility**: 
    * Periodically retrieves the current system state (sensor readings, operational parameters) from `sys_mgr` via RTE services. It then applies the control logic (e.g., fan staging, heater activation, pump control, ventilator scheduling) and commands the actuators via RTE services.

  * **Periodicity**: 100 ms

  * **Priority**: 5 (High)

* **`RTE_Service_100ms_ComMgrTask`**

  * **Responsibility**: 
    * Acts as the central orchestrator for all communication protocols. It periodically calls the `_Process()` functions of `modbus`, `bluetooth`, and `wifi`. It also handles data exchange: pulling system data from `sys_mgr` (via RTE) to expose externally, and pushing external commands/data (from middleware) to `sys_mgr` (via RTE).

  * **Periodicity**: 100 ms

  * **Priority**: 4 (Medium)

* **`RTE_Service_1000ms_DisplayIndicationTask`**

  * **Responsibility**: 
    * Periodically retrieves system status, sensor readings, and alarm states from `sys_mgr` via RTE services. It then updates the character displays and controls alarm indication LEDs (e.g., `LIGHT_INDICATION_ERROR`) via RTE services.
    * Handles general, lower-priority system-wide operations. This includes toggling system status LEDs, managing display backlights, and logging simulated time.
  
  * **Periodicity**: 1000 ms (1 second)

  * **Priority**: 3 (Medium)

* **`RTE_Service_1000ms_LogHealthMTask`**

  * **Responsibility**: 
    * Periodically calculates and logs system health metrics, including CPU load and stack High Water Marks (HWM) for all active tasks.

  * **Periodicity**: 1000 ms (1 seconds)

  * **Priority**: 1 (Lowest)

#### 3.2.2. Inter-Task Communication

* **Shared State with Mutexes**: The `sys_mgr` module maintains a central `sys_mgr_state_t` structure that holds all critical system data (current sensor readings, operational parameters, actuator states, alarm flags). Access to this shared structure is protected by a FreeRTOS Mutex (`sys_mgr_state_mutex`). Any task (via an RTE service) that needs to read from or write to this state must first acquire the mutex.

* **RTE Service Calls**: All inter-task and inter-module communication occurs through synchronous function calls to RTE Services. This simplifies the data flow and centralizes dependency management.

* **No Direct Queues/Semaphores (for Application Logic)**: Application-level modules do not directly use FreeRTOS queues or semaphores to communicate with each other. This complexity is hidden and managed internally by the RTE (if needed for specific asynchronous operations, though currently, the model is primarily synchronous via mutex-protected shared state).

### 3.3. Development View

The development view describes the system's organization within the file system and how the project is built.

#### 3.3.1. Directory Structure

```text

├── Application/
│   ├── SystemMonitor/                # Contains system_monitor.h/.c
│   ├── SystemStartup/                # Contains startup.c (app_main entry point)
│   ├── common/                       # Contains app_common.h
│   ├── diagnostic/                   # (Placeholder for diagnostic features)
│   ├── display/                      # Contains character_display.h/.c
│   ├── fan/                          # Contains fan.h/.c
│   ├── heater/                       # Contains heater.h/.c
│   ├── humadity/                     # Contains humidity_sensor.h/.c 
│   ├── lightControl/                 # Contains light_control.h/.c
│   ├── lightIndication/              # Contains light_indication.h/.c
│   ├── logger/                       # Contains logger.h/.c
│   ├── power/                        # (Placeholder for power management)
│   ├── pump/                         # Contains pump.h/.c
│   ├── systemMgr/                    # Contains sys_mgr.h/.c
│   ├── temperature/                  # Contains temp_sensor.h/.c
│   ├── ventilator/                   # Contains ventilator.h/.c
│   ├── cmake/                        # CMake configuration for Application modules
│   └── CMakeLists.txt                # Main CMakeLists for the Application layer
├── HAL/
│   ├── cfg/                          # Configuration files for HAL modules
│   ├── src/                          # Source files for HAL modules
│   ├── inc/                          # Include files for HAL modules
│   ├── cmake/                        # CMake configuration for HAL
│   └── CMakeLists.txt                # Main CMakeLists for the HAL layer
├── Mcal/
│   ├── adc/                          # MCAL ADC driver (inc/src)
│   ├── uart/                         # MCAL UART driver (inc/src)
│   ├── gpio/                         # MCAL GPIO driver (inc/src)
│   ├── i2c/                          # MCAL I2C driver (inc/src)
│   ├── pwm/                          # MCAL PWM driver (inc/src)
│   ├── spi/                          # MCAL SPI driver (inc/src)
│   ├── timers/                       # MCAL Timers driver (inc/src)
│   ├── Bluetooth/                    # Bluetooth driver (inc/src)
│   ├── Wifi/                         # WiFi driver (inc/src)
│   ├── cmake/                        # CMake configuration for MCAL
│   └── CMakeLists.txt                # Main CMakeLists for the MCAL layer
├── Service/
│   ├── modbus/                       # Modbus Middleware (inc/src)
│   ├── os/                           # OS (FreeRTOS) related configurations/wrappers
│   ├── ComM/                         # Communication Manager (inc/src)
│   ├── cmake/                        # CMake configuration for Service layer
│   └── CMakeLists.txt                # Main CMakeLists for the Service layer
├── Rte/
│   ├── cfg/                          # Configuration files for RTE
│   ├── inc/                          # Include files for RTE (Rte.h)
│   ├── src/                          # Source files for RTE (Rte.c)
│   ├── cmake/                        # CMake configuration for RTE
│   └── CMakeLists.txt                # Main CMakeLists for the RTE layer
├── cmake/
│   ├── esp32_flash.ld                # Linker script
│   ├── esp32_toolchain.cmake         # Toolchain definition for ESP32
│   ├── gcc_toolchain.cmake           # Generic GCC toolchain definition
│   └── project_global_settings.cmake # Global project settings
└── build/                            # Build output directory
```

#### 3.3.2. Build System

## Build System Overview

The project uses **CMake** as its build system, suitable for modular embedded development (e.g., ESP-IDF based).

- **Root `CMakeLists.txt`**: Defines the main project and integrates modular components under directories like `Application`, `HAL`, `Mcal`, `Service`, and `Rte`.

- **`Application/CMakeLists.txt`**: Builds the application logic as a static library, pulling in submodules like `SystemMonitor`, `fan`, `temperature`, etc. Includes middleware and interface layers.

- **`HAL/`, `Mcal/`, `Service/`, and `Rte/` `CMakeLists.txt`**: Each defines static libraries for hardware abstraction, microcontroller drivers, communication stacks (e.g., Modbus, Bluetooth, Wi-Fi), and runtime integration.

#### 3.3.3. Coding Standards and Conventions

* **Language**: All source code is written in **C**, following modular and layered architecture principles.

* **Naming Conventions**:

  * **Modules/Components**: Upper snake case — `MODULE_NAME` (e.g., `FAN_CTL`, `SYS_MGR`).

  * **Public Functions**: `MODULE_NAME_FunctionName()` — used for APIs exposed to other layers/modules  
    _Example_: `TEMP_SENS_ReadValue()`

  * **RTE Interfaces**: Prefixed with `RTE_` to reflect integration role  
    _Example_: `RTE_SetVentilationSpeed()`

  * **Internal/Static Functions**: Lower snake case — `module_name_function_name()`  
    _Example_: `temp_sens_filter_raw_data()`

  * **Constants & Macros**: Upper snake case with module prefix  
    _Example_: `FAN_CTL_MAX_SPEED`, `POWER_FAILURE_TIMEOUT_MS`

  * **Types & Enums**: PascalCase with `_t` suffix  
    _Example_: `FanSpeed_t`, `SystemState_t`

* **Error Handling**:
  * Return codes like `E_OK`, `E_NOK`, or detailed status codes (e.g., `TEMP_SENSOR_ERROR_TIMEOUT`)
  * Critical failures may trigger task termination, system reset, or fault logging.

* **Mutex Protection**:
  * All access to shared data structures (e.g., `sys_mgr_state_t`) is synchronized using **FreeRTOS mutexes** or **semaphores**.
  * Locking ensures thread safety in concurrent environments such as tasks and ISRs.

* **Logging**:
  * Unified logging macros:  
    `LOGI` (Info), `LOGW` (Warning), `LOGE` (Error), `LOGD` (Debug), `LOGV` (Verbose)
  * Logs should be non-blocking and module-aware, especially in real-time contexts.

### 3.4. Physical View

The physical view describes the mapping of software components to the hardware platform.

* **Microcontroller (MCU)**: The central processing unit (e.g., ESP32) that executes all OS tasks and hosts the entire software stack. It provides the core CPU, memory (RAM, Flash), and internal peripherals.

* **Internal Peripherals**:

  * **GPIOs**: Used for digital I/O (e.g., controlling LEDs, reading simple switches).

  * **PWM**: For controlling motor speeds (fans) or dimming lights.

  * **ADC**: For reading analog sensor values (e.g., some temperature/humidity sensors, or analog inputs from other devices).

  * **I2C**: For digital sensor communication (e.g., high-precision temperature/humidity sensors, character displays).

  * **UART**: For serial communication, specifically for Modbus RTU.

  * **Timers**: Used by OS for task scheduling and by `Service_Layer` for uptime tracking and delays.

* **External Interfaces**:

  * **Modbus RTU**: Communicates over UART to external Modbus masters/slaves.

  * **Bluetooth (BLE)**: Utilizes the MCU's integrated Bluetooth radio for wireless communication with mobile apps or other BLE devices.

  * **Wi-Fi**: Utilizes the MCU's integrated Wi-Fi radio for network connectivity (e.g., connecting to a local router, cloud services).

* **Sensors**: Physical devices that measure environmental parameters (e.g., DHT11/22 for temp/hum, NTC thermistors).

* **Actuators**: Physical devices that perform actions (e.g., DC fans, heating elements, water pumps, exhaust fans, lights).

* **Displays/Indicators**: Character LCDs, LEDs.

## 4. Key Architectural Design Decisions

### 4.1. The Runtime Environment (RTE) Pattern

#### RTE Design Decision

**Decision**: Implement a lightweight Runtime Environment (RTE) layer to decouple modules and manage inter-layer communication in a structured and scalable way.

- **Service Layer Communication** is routed through `RTE_Service_` **macros** (e.g., `RTE_Service_Modbus_Send`).
- **MCAL Access** is abstracted via `RTE_HAL_` **macros** (e.g., `RTE_HAL_ReadADC`), allowing hardware interaction without tight coupling.
- **Application Layer Communication** uses `RTE_App_` **macros** (e.g., `RTE_App_SetFanSpeed`), enabling clean data/control flow between high-level components.

**Rationale**:

- **Strong Decoupling**: Modules do not directly call each other, reducing compile-time and runtime dependencies. This makes individual modules more reusable and less prone to ripple effects from changes in other modules.

- **Improved Testability**: Each module can be unit-tested in isolation by mocking RTE macros, simplifying the creation of test harnesses.

- **Centralized Control**: The RTE acts as a single point for managing and potentially modifying communication flows, enforcing access control, or adding logging/profiling without altering individual modules.

- **Scalability**: Adding new features or modules primarily involves defining new RTE macros and implementing them in `Rte.c`, minimizing changes to existing application logic.

- **Use of Macros**: Using macros instead of functions allows for zero-overhead abstraction, inlining calls at compile-time, which is critical in embedded systems where performance and code size are constrained.

### 4.2. Communication with Orchestration

* **Decision**: Introduce a dedicated Service Layer for communication protocols (`modbus`, `bluetooth`, `wifi`) and centralize their management under a single `COMMUNICATION_MGR_Task` (within RTE).

* **Rationale**:

  * **Protocol Abstraction**: The application layer is entirely unaware of the specific communication protocols. It simply requests data or sends commands via generic `RTE_Service_ComM_...` calls.

  * **Modularity**: Each communication protocol is encapsulated in its own middleware module, making it easy to swap, update, or add new protocols without affecting other parts of the system.

  * **Resource Management**: A single communication task can better manage shared resources (e.g., UART for Modbus, network interfaces for Wi-Fi/Bluetooth) and prioritize communication needs.

  * **Simplified Application Logic**: Application modules do not need to deal with the complexities of Modbus registers, Bluetooth GATT profiles, or Wi-Fi connection states.

### 4.3. Task-Based Initialization

* **Decision**: Perform HAL and Application module initialization within dedicated, self-deleting OS tasks (`RTE_HwInitTask`, `RTE_AppInitTask`).

* **Rationale**:

  * **Sequential and Controlled Startup**: Ensures that initialization steps occur in a defined order (hardware before application, etc.).

  * **Leverage OS Features**: Allows initialization routines to use FreeRTOS features like `vTaskDelay()` for power-up sequences, mutexes for shared resource setup, or even temporary queues if needed during complex initialization.

  * **Resource Optimization**: Initialization tasks are temporary; they delete themselves after completing their job, freeing up their stack and TCB memory for permanent application tasks.

  * **Clear Separation**: Separates the one-time startup logic from the continuous runtime logic.

### 4.4. Shared Data with Mutexes

* **Decision**: Critical shared data is encapsulated within its owning module and accessed by other modules only through RTE services, with internal mutex protection.

* **Rationale**:

  * **Thread Safety**: Prevents race conditions and data corruption when multiple OS tasks concurrently access or modify the same data.

  * **Data Integrity**: Ensures that data is always in a consistent state.

  * **Encapsulation**: The mutex management is internal to the module, hiding the synchronization complexity from other modules.

## 5. Component Descriptions

### 5.1. `SystemStartup`

* **Role**: The absolute entry point of the application.

* **Responsibilities**:

  * Performs minimal essential hardware initialization (e.g., UART for early logging) by calling a base MCAL function.

  * Initializes the `logger` module.

  * Creates the very first FreeRTOS task, `RTE_HwInitTask`, by calling `RTE_Init()`.

  * Starts the FreeRTOS scheduler.

* **Dependencies**: 
  * `Rte.h`, `logger.h`, `hal_uart.h` , FreeRTOS headers.

### 5.2. `RTE`

* **Role**: The core of the Runtime Environment. It manages all tasks and provides the sole interface for inter-module communication.

* **Responsibilities (`Rte.c`)**:

  * **`RTE_Init()`**: Creates `RTE_HwInitTask` and `RTE_AppInitTask()`.

  * **`RTE_HwInitTask()`**: Initializes **MCAL modules** , **HAL modules** and deletes itself.

  * **`RTE_AppInitTask()`**:  
    * Initializes **Service Layer** and **Application Layer**.
    * Configures initial system parameters.
    * Calls `RTE_StartAllPermanentTasks()`. 
    * Deletes itself.

  * **`RTE_StartAllPermanentTasks()`**: Contains all `xTaskCreate()` calls for the permanent application tasks :
    *  `RTE_SensorReadTask`
    *  `RTE_ActuatorControlTask` 
    *  `RTE_DisplayAlarmTask`
    *  `RTE_MainLoopTask`
    *  `SYS_MON_Task`
    *  `COMMUNICATION_MGR_Task`

  * **`RTE_SensorReadTask()`**: 
    * Periodically calls `RTE_Service_ProcessSensorReadings()`.

  * **`RTE_ActuatorControlTask()`**: 
    * Periodically calls `RTE_Service_ControlActuators()`.

  * **`RTE_DisplayAlarmTask()`**: 
    * Periodically calls `RTE_Service_UpdateDisplayAndAlarm()`.

  * **`RTE_MainLoopTask()`**: 
    * Handles general system heartbeat and low-priority tasks.

  * **`COMMUNICATION_STACK_MainTask()`**: 
    * The main communication task. Periodically calls `COMMUNICATION_STACK_ProcessModbus()`, `COMMUNICATION_STACK_ProcessBluetooth()`, `COMMUNICATION_STACK_ProcessWiFi()` (which are functions in `Service/communication_stack_interface.c`).

  * **`RTE_Service_...()` functions**: 
    * Implement the actual routing of calls to the target modules (e.g., `RTE_Service_SetOperationalTemperature()` calls `Application/systemMgr/sys_mgr.c::SYS_MGR_SetOperationalTemperature()`).

* **Dependencies**: 
  * Directly includes headers from `Mcal/*/inc`, `HAL/inc`, `Service/*/inc`, `Application/*/inc`, and FreeRTOS headers.

### 5.3. `SystemMgr`

* **Role**: Central application logic and state management.

* **Responsibilities**:

  * Manages operational temperature/humidity ranges and ventilator/light schedules.

  * Stores current sensor readings and actuator states.

  * Implements the fan staging, heater control, pump control, and ventilator control logic.

  * Manages fire alarm conditions and state.

  * Provides getter/setter functions for its internal state.

* **Dependencies**: 
  * `sys_mgr.h`, `logger.h`, `app_common.h`, `Rte.h`, `freertos/semphr.h` (for mutex).

### 5.4. `SystemMonitor`

* **Role**: Monitors system health.

* **Responsibilities**:

  * Calculates CPU load based on idle task runtime.

  * Calculates total minimum free stack (HWM) across all tasks.

  * Provides getter functions for these metrics.

* **Dependencies**: 
  * `logger.h`, `app_common.h`, FreeRTOS headers.

### 5.5. `ComM(Communication MGR)`

* **Role**: 
  * Provides the functional interface between the `COM_MGR_Task` and the actual communication modules. It encapsulates the data exchange logic.

* **Responsibilities**:

  * **`COM_MGR_ProcessModbus()`**: Calls `MODBUS_Process()` and handles data flow between `systemMgr` (via RTE) and Modbus registers.

  * **`COM_MGR_ProcessBluetooth()`**: Calls `BLUETOOTH_Process()` and handles data flow between `systemMgr` (via RTE) and Bluetooth characteristics.

  * **`COM_MGR_ProcessWiFi()`**: Calls `WIFI_Process()` and handles data flow between `systemMgr` (via RTE) and Wi-Fi network services.

  * **`COM_MGR_Internal_...()` functions**: Direct wrappers around middleware functions, called by `RTE_Service_COMM_...` functions.

* **Dependencies**: 
  * `comm_interface.h`, `logger.h`, `Rte.h` , `modbus.h`, `bluetooth.h`, `wifi.h`.

### 5.6. `Modbus`

* **Role**: Implements the Modbus protocol specific logic.

* **Responsibilities**:

  * Initializes Modbus driver (e.g., using `Mcal/uart`).

  * Manages simulated Modbus holding registers.

  * Provides functions to read from and write to these registers.

  * **`MODBUS_Process()`**: Handles incoming Modbus requests and dispatches them.

* **Dependencies**: 
  * `modbus.h`, `logger.h`, `hal_uart.h`.

### 5.7. `Bluetooth`

* **Role**: Implements Bluetooth (BLE) protocol specific logic.

* **Responsibilities**:

  * Initializes Bluetooth stack (controller, host, GAP, GATT services).

  * Provides functions to send data (notifications/indications) via GATT characteristics.

  * **`BLUETOOTH_Process()`**: Manages advertising, connections, and incoming GATT writes.

* **Dependencies**: 
  * `bluetooth.h`, `logger.h`

### 5.8. `WIFI`

* **Role**: Implements Wi-Fi connectivity and network-level data transfer.

* **Responsibilities**:

  * Initializes Wi-Fi driver.

  * Manages connection to access points.

  * Provides functions to send data over the network (e.g., HTTP, MQTT).

  * **`WIFI_Process()`**: Handles network events and manages data queues.

* **Dependencies**: 
  * `wifi.h`, `logger.h`.

### 5.9. Microcontroller Abstraction Layer (MCAL) Modules

* **Role**: Provide direct, low-level hardware abstraction for specific MCU peripherals.

* **Responsibilities**: Encapsulate register-level access and provide a clean C API for each peripheral (e.g., `GPIO_SetPinDirection()`, `PWM_SetDutyCycle()`).

* **Dependencies**: Only MCU-specific headers (e.g., `driver/mcu_registers.h` or `<driver/gpio.h>`).

### 5.10. Hardware Abstraction Layer (HAL)

* **Role**:  It would typically provide higher-level drivers or composite drivers that utilize MCAL functions. For example, a `HAL_DisplayDriver` might use `Mcal/gpio` and `Mcal/i2c` to control an LCD.

* **Responsibilities**: Implement higher-level peripheral control logic.

* **Dependencies**: `Mcal/*/inc` headers.

### 5.11. Other Application Modules 
* **Like**: `fan`, `heater`, `temperature`, `display`, `lightControl`, `lightIndication`
  
* **Role**: 
  * Encapsulate specific hardware control or sensor reading logic.

* **Responsibilities**: 
  * Provide basic `_Init()` functions and functional APIs (e.g., `FAN_SetSpeed()`, `TEMP_SENSOR_ReadTemperature()`).

* **Dependencies**: 
  * Only their own `_config.h` and relevant `HAL/inc` or `Mcal/*/inc` headers (if they directly use MCAL/HAL functions, which is typically via RTE now). Their public functions are called by RTE Services.

## 6. Scenarios

### 6.1. Scenario 1: Sensor Reading and Actuator Control

**Description**: This scenario illustrates the core control loop where sensor data is read and actuators are adjusted based on operational parameters.

 1. **Sensor Read (Process View)**: `RTE_SensorReadTask` wakes up (every 20 ms).

 2. **RTE Service Call (Logical View)**: `RTE_SensorReadTask` calls `RTE_Service_ProcessSensorReadings()`.

 3. **Sensor Data Acquisition (Logical View)**: `RTE_Service_ProcessSensorReadings()` internally calls `SYS_MGR_ProcessSensorReadings()`. `SYS_MGR_ProcessSensorReadings()` then calls `TEMP_SENSOR_ReadTemperature()` and `HUMIDITY_SENSOR_ReadHumidity()` (which internally use HAL functions like `HAL_ADC_Read()`).

 4. **State Update (Logical View)**: `SYS_MGR_ProcessSensorReadings()` acquires `sys_mgr_state_mutex`, updates `sys_mgr_state.current_room_temp_c` and `sys_mgr_state.current_room_humidity_p`, and releases the mutex.

 5. **Actuator Control (Process View)**: `RTE_ActuatorControlTask` wakes up (every 100 ms).

 6. **RTE Service Call (Logical View)**: `RTE_ActuatorControlTask` calls `RTE_Service_ControlActuators()`.

 7. **Control Logic (Logical View)**: `RTE_Service_ControlActuators()` internally calls `SYS_MGR_ControlActuators()`. `SYS_MGR_ControlActuators()` acquires `sys_mgr_state_mutex` to read current sensor values and operational parameters.

 8. **Actuator Command (Logical View)**: Based on the control logic (e.g., if `current_room_temp_c` > `operational_temp_max`), `SYS_MGR_ControlActuators()` calls `RTE_Service_FAN_SetSpeed()` or `RTE_Service_HEATER_SetState()`.

 9. **Hardware Control (Logical View)**: The RTE Service (e.g., `RTE_Service_FAN_SetSpeed()`) internally calls `FAN_SetSpeed()`, which then calls `ECUAL_PWM_SetDutyCycle()` to control the physical fan.

10. **State Update (Logical View)**: `SYS_MGR_ControlActuators()` updates `sys_mgr_state.current_fan_stage`, `sys_mgr_state.heater_is_working`, etc., and releases the mutex.

### 6.2. Scenario 2: Modbus Command to Change Operational Temperature

**Description**: An external Modbus master sends a command to update the system's operational temperature range.

1. **Modbus Request Reception (Physical View)**: A Modbus master sends a "Write Multiple Registers" command over UART. The `ecual_uart` driver receives the raw bytes.

2. **Service Processing (Logical View)**: The `COMMUNICATION_STACK_MainTask` wakes up (every 100 ms) and calls `COMMUNICATION_STACK_ProcessModbus()`. This function, in turn, calls `MODBUS_MW_Process()`.

3. **Protocol Interpretation (Logical View)**: `MODBUS_MW_Process()` (within `modbus_middleware.c`) parses the incoming Modbus frame, identifies it as a write command for `MODBUS_REG_SET_MIN_OP_TEMP_X100` and `MODBUS_REG_SET_MAX_OP_TEMP_X100`, and updates its internal simulated Modbus register map.

4. **Data Exchange (Logical View)**: `COMMUNICATION_STACK_ProcessModbus()` then reads these updated registers from `modbus_middleware` (e.g., using `MODBUS_MW_ReadHoldingRegister()`).

5. **RTE Service Call (Logical View)**: `COMMUNICATION_STACK_ProcessModbus()` detects that the new temperature range is different from the current one (by first querying `sys_mgr` via `RTE_Service_GetOperationalTemperature()`). It then calls `RTE_Service_SetOperationalTemperature(new_min_temp, new_max_temp)`.

6. **System State Update (Logical View)**: The RTE Service `RTE_Service_SetOperationalTemperature()` internally calls `SYS_MGR_SetOperationalTemperature()`. `SYS_MGR_SetOperationalTemperature()` acquires `sys_mgr_state_mutex`, updates `sys_mgr_state.operational_temp_min` and `sys_mgr_state.operational_temp_max`, and releases the mutex.

7. **Subsequent Control (Process View)**: In the next cycle, `RTE_ActuatorControlTask` will read the *new* operational temperature range from `sys_mgr` and adjust actuators accordingly.

### 6.3. Scenario 3: Bluetooth Data Broadcast (System Status)

**Description**: The system periodically broadcasts its current sensor readings and actuator statuses via Bluetooth Low Energy (BLE).

1. **Communication Task Wakeup (Process View)**: `COMMUNICATION_STACK_MainTask` wakes up (every 100 ms).

2. **Service Processing (Logical View)**: `COMMUNICATION_STACK_MainTask` calls `COMMUNICATION_STACK_ProcessBluetooth()`. This function, in turn, calls `BLUETOOTH_MW_Process()` to handle any internal BLE stack events (e.g., advertising, connection management).

3. **Data Acquisition (Logical View)**: `COMMUNICATION_STACK_ProcessBluetooth()` needs to send the latest system data. It calls various RTE services to get this data:

   * `RTE_Service_GetCurrentSensorReadings()` (for temperature, humidity)

   * `RTE_Service_GetActuatorStates()` (for fan, heater, pump, ventilator status)

   * `RTE_Service_GetCPULoad()` (for CPU load)

   * `RTE_Service_LIGHT_GetState()` (for light status)

4. **Data Formatting (Logical View)**: `COMMUNICATION_STACK_ProcessBluetooth()` formats the retrieved data into a suitable payload (e.g., a string or a byte array).

5. **Data Transmission (Logical View)**: `COMMUNICATION_STACK_ProcessBluetooth()` then calls `BLUETOOTH_MW_SendData(BT_CHAR_SENSOR_DATA_UUID, formatted_data, data_length)`.

6. **Physical Transmission (Physical View)**: `BLUETOOTH_MW_SendData()` (within `bluetooth_middleware.c`) uses the underlying native Bluetooth APIs to send the data as a GATT notification or indication to connected BLE clients.

## 7. Future Considerations and Improvements

* **Error Handling and Fault Tolerance**: Implement more sophisticated error handling mechanisms, including retry logic, watchdog timers, and fail-safe states for critical components.

* **Configuration Management**: Implement a persistent storage mechanism (e.g., NVS Flash on ESP32) for operational parameters and schedules, allowing them to persist across reboots.

* **Over-the-Air (OTA) Updates**: Integrate OTA update capabilities to allow firmware upgrades remotely via Wi-Fi or Bluetooth.

* **Power Management**: Implement low-power modes and dynamic power scaling for tasks and peripherals to optimize energy consumption.

* **Event-Driven Communication**: For more complex inter-task communication, consider using FreeRTOS queues or event groups directly within RTE services, especially for asynchronous data flows or command passing.

* **Security**: Implement security measures for communication protocols (e.g., TLS for Wi-Fi, BLE pairing/encryption) to protect data integrity and confidentiality.

* **Testing Framework**: Develop a more comprehensive automated testing framework for unit, integration, and system tests.

* **Command Parsing**: For incoming commands from Modbus/Bluetooth/Wi-Fi, implement a robust command parsing and dispatching mechanism within the `COMMUNICATION_STACK_MainTask` to route commands to the correct RTE services.






