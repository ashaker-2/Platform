# System Requirements Specification (SyRS) — Environmental Monitoring & Control System

---

## 1. Introduction
This document defines the **system-level requirements** for a smart environmental monitoring and control platform. It supports sensing, actuation, and both wired (Modbus) and wireless (Bluetooth) interfaces. Designed for modularity and robustness, it accommodates a flexible software architecture and evolving hardware platforms.

---

## 2. Functional Requirements

### 2.1 Sensor Inputs
- **SyRS-02-01-01:** The system shall support three distinct types of temperature sensors:
  - Sensor A (e.g., NTC or DS18B20)
  - Sensor B (e.g., RTD or analog)
  - Sensor C (e.g., thermocouple or digital)
- **SyRS-02-01-02:** The system shall support a humidity sensor with analog or digital signal input.
- **SyRS-02-01-03:** The system shall include an onboard temperature sensor to monitor the controller’s internal temperature.
- **SyRS-02-01-04:** The system shall receive temperature and humidity data from external Modbus devices.

### 2.2 Control Outputs
- **SyRS-02-02-01:** Four relay outputs shall be available for cooling control.
- **SyRS-02-02-02:** Four relay outputs shall be available for lighting control.
- **SyRS-02-02-03:** Four relay outputs shall be provided for ventilation control.
- **SyRS-02-02-04:** One relay output shall control the heating system.
- **SyRS-02-02-05:** One relay shall control a humidity pump (mandatory).
- **SyRS-02-02-06:** One PWM output shall control the internal system fan.
- **SyRS-02-02-07:** One relay output shall be used for alarm signaling.
- **SyRS-02-02-08:** Each relay shall support AC-1 loads up to 4A.

### 2.3 Display and Indicators
- **SyRS-02-03-01:** The system shall have five LED indicators:
  - LED1 (System status – blinking when operational)
  - LED2 (Cooling active)
  - LED3 (Lighting active)
  - LED4 (Pump active)
  - LED5 (Comm active)

### 2.4 Modes and Interfaces
- **SyRS-02-04-01:** Configuration shall be possible via buttons, Bluetooth, and Modbus.
- **SyRS-02-04-02:** The system shall support configuration and monitoring over Bluetooth (BLE).
- **SyRS-02-04-03:** The system shall support configuration and monitoring over Modbus RTU (RS485).

### 2.5 Alarms and Errors
- **SyRS-02-05-01:** Alarms shall activate if sensor values exceed predefined thresholds.
- **SyRS-02-05-02:** System faults shall disable outputs and be indicated by LED1.

---

## 3. Non-Functional Requirements

### 3.1 Electrical and Environmental
- **SyRS-03-01-01:** Input supply options: 12V DC, 24V DC, or 230V AC.
- **SyRS-03-01-02:** Max power consumption: 4 VA.
- **SyRS-03-01-03:** Operating temperature: −10 °C to +55 °C.
- **SyRS-03-01-04:** Relative humidity: 0–85% RH, non-condensing.
- **SyRS-03-01-05:** Enclosure shall meet IP54 standards.

### 3.2 Mechanical
- **SyRS-03-02-01:** Panel and wall mounting supported.
- **SyRS-03-02-02:** Dimensions: 100 × 50 × 90 mm.
- **SyRS-03-02-03:** Weight: approx. 1 kg.

### 3.3 Reliability
- **SyRS-03-03-01:** Configuration parameters stored in non-volatile memory.
- **SyRS-03-03-02:** Autonomous fault recovery without reboot.
- **SyRS-03-03-03:** Minimum uptime 99.5% per year.

---

## 4. Communication Interfaces

### 4.1 Bluetooth (BLE)
- **SyRS-04-01-01:** BLE 4.0 or higher shall be supported.
- **SyRS-04-01-02:** BLE shall allow viewing sensors, controlling actuators, modifying config.
- **SyRS-04-01-03:** Bluetooth shall support OTA firmware update.

### 4.2 Modbus RTU (RS485)
- **SyRS-04-02-01:** Modbus RTU support over RS485.
- **SyRS-04-02-02:** The system shall act as both Master and Slave.
- **SyRS-04-02-03:** Supported function codes: 0x03, 0x04, 0x06, 0x10.
- **SyRS-04-02-04:** Modbus shall support OTA firmware update.

---

## 5. Constraints

- **SyRS-05-01-01:** Only one interface (BLE or Modbus) active at a time.
- **SyRS-05-01-02:** Pump control is mandatory and cannot be bypassed.
- **SyRS-05-01-03:** Fan is controlled via PWM based on internal temp.
- **SyRS-05-01-04:** Bluetooth updates must be secured:
  - Require PIN or pairing auth
  - Log failed attempts
  - Session timeout after 3 min
- **SyRS-05-01-05:** Critical settings must be stored in NVM.
- **SyRS-05-01-06:** Bluetooth and Modbus cannot be active simultaneously.
- **SyRS-05-01-07:** Fault conditions must force safe state.

---

## 6. Memory Architecture

- **SyRS-06-01-01:** Flash shall be split as follows:
  - **Bootloader Region:** Secure, read-only
  - **Bank A:** Factory Application (diagnostics + production)
  - **Bank B:** Application_A
  - **Bank C:** Application_B
- **SyRS-06-01-02:** Bootloader shall:
  - Validate CRC/hash of apps
  - Switch between A/B/C
  - Enter Factory App on fault
  - Lock JTAG after production

---

## 7. External Interfaces

| Interface         | Description                                 |
|------------------|---------------------------------------------|
| Temp Inputs       | 3 sensor types (analog/digital/NTC)         |
| Humidity Input    | Analog or digital input                     |
| Board Temp Sensor | For internal monitoring                     |
| Cooling Relays    | 4 outputs                                   |
| Lighting Relays   | 4 outputs                                   |
| Ventilation Relays| 4 outputs                                   |
| Heating Relay     | 1 output                                    |
| Pump Relay        | 1 output                                    |
| PWM Fan Output    | Controlled via onboard temp sensor          |
| Alarm Relay       | 1 output                                    |
| LEDs              | 5 indicators                                |
| Buttons           | Local configuration                         |
| Bluetooth         | Wireless config & update                    |
| RS485             | Modbus control & update                     |

---

## 8. Glossary
- **PWM:** Pulse Width Modulation
- **Modbus RTU:** Serial protocol for SCADA/PLC
- **OTA:** Over-the-Air update
- **JTAG Lock:** Hardware-level programming lock
- **AC-1 Load:** Non-inductive load class
- **NVM:** Non-Volatile Memory
- **Factory App:** Recovery image
- **Bootloader:** Minimal secure boot manager

