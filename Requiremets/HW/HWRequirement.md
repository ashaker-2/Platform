# Hardware Requirements Specification (HWRS) — Environmental Monitoring & Control System

---

## 1. Electrical and Power Requirements

* **Measurement Accuracy - Temperature**: The temperature measurement hardware shall have an accurate value of $0.1^{\circ}C$.

* **Measurement Deviation - Temperature**: The temperature measurement hardware shall have a deviation value of $\pm1\%$.

* **Measurement Accuracy - Humidity**: The humidity measurement hardware shall have an accurate value of 0.1%.

* **Measurement Deviation - Humidity**: The humidity measurement hardware shall have a deviation value of $\pm5\%$.

* **Input Power Voltage**: The system hardware shall operate with an input power voltage of 220 VAC $\pm$10VAC.

* **Frequency**: The system hardware shall operate at a frequency of 50-60 Hz.

* **Electric Power Consumption**: The system hardware shall consume 9 VA of electric power.

* **Relay Fireproofing Rate**: The relays shall have a fireproofing rate of 5A/250VAC.

* **Relay Alarm Fireproofing Rate**: The relay alarm shall have a fireproofing rate of 0.5A 250VAC / 1A 30VDC.

* **Output Dry Contact Rating**: All dry contact outputs shall have a maximum rating of 3A/250VAC.

* **Earthing**: The protective conductor (P.E.) shall be connected to the G (Ground) connector.

* **Earth Electrode Resistance**: If the earth electrode resistance is greater than 5 ohms, a new earth electrode must be used.

* **Circuit Breaker**: The system input power line shall be protected by a 10A Circuit Breaker.

* **Surge Protection**: The system input power line shall include surge protection.

* **Ground Wire**: The ground wire shall be 6 square mm.

* **Ground Resistance**: The ground resistance shall be less than 5 Ohm.

---

## 2. Environmental Requirements

* **Operating Temperature**: The system hardware shall operate within a temperature range of -20 to $60^{\circ}C$.

* **Measurement Range - Temperature**: The system hardware shall be capable of measuring temperature within a range of -10.0 to $70.0^{\circ}C$.

* **Measurement Range - Humidity**: The system hardware shall be capable of measuring humidity within a range of 0.0-99.9%RH.

* **Installation Location**: The system hardware shall be installed in a dry place.

* **Installation Proximity to Electrical Cabinet**: The system hardware shall be installed approximately 1 meter away from the electrical cabinet.

---

## 3. Maintainability and Reliability

* **Product Warranty**: The product hardware warranty period is one year from the date of invoice.

* **Defect-Free Guarantee**: The product hardware shall be free of defects in materials or workmanship and conform to technical specifications.

* **Backup and Alarm Systems**: The user shall provide adequate backup and alarm systems for critical operations in case of controller hardware failure.

---

## 4. Hardware Interfaces

* **Power Input Connectors**: The system hardware shall provide connectors for L (220 VAC) and N (0 VAC) for power input.

* **Ground Connector**: The system hardware shall provide a G (Ground) connector for a protective conductor.

* **Temperature Sensor Connection**: The system hardware shall support a temperature sensor connected to T-IN and 0V inputs using a two-wire cable.

* **Humidity Sensor Connection**: The system hardware shall support a humidity sensor connected to 12V (Brown), 0V (Green), and HUM (White) inputs using a three-wire cable.

* **Analog Output (A-OUT)**: The system hardware shall have an A-OUT for analog output to a variable speed fan (inverter).

* **Specific Relay Outputs**: The system hardware shall provide physical connections for FAN1, FAN2, FAN3, FAN4, PUMP, HT, and NO/COM/NC (Alarm) relay outputs.

* **RS-485 Cable Details**: The RS-485 connection hardware shall utilize D+, D-, and G (shield) wires.


## 5. Peripheral/Pin Assignment

| Peripheral          | Typical Pins (GPIO)                                                                                                                                                  | Total Pins* |
|---------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------|
| **UART0** (default console/flash) | TX0 = GPIO1, RX0 = GPIO3                                                                                                                                         | 2           |
| **UART1**           | TX1 = GPIO10**, RX1 = GPIO9**, RTS1 = GPIO11**, CTS1 = GPIO6** *(remap possible; commonly remapped to GPIO16, GPIO17)*                                                  | 4 (flexible)|
| **UART2**           | TX2 = GPIO17, RX2 = GPIO16, RTS2 = GPIO8**, CTS2 = GPIO7** *(remap possible)*                                                                                           | 4 (flexible)|
| **SPI0/1** (flash)  | SCK = GPIO6**, MISO = GPIO7**, MOSI = GPIO8**, CS0 = GPIO11** *(used internally for flash — not for general use)*                                                       | 4 (reserved)|
| **HSPI** (SPI2)     | SCK = GPIO14, MISO = GPIO12, MOSI = GPIO13, CS0 = GPIO15 *(can be remapped)*                                                                                            | 4           |
| **VSPI** (SPI3)     | SCK = GPIO18, MISO = GPIO19, MOSI = GPIO23, CS0 = GPIO5 *(can be remapped)*                                                                                             | 4           |
| **I²C**             | SDA = GPIO21, SCL = GPIO22 *(both remappable)*                                                                                                                         | 2           |
| **I²S0**            | Multiple pins — default BCK = GPIO26, WS = GPIO25, DATA_OUT = GPIO22, DATA_IN = GPIO23 *(fully remappable)*                                                              | ≥4          |
| **PWM (LEDC)**      | Any output-capable GPIO *(not 34–39)*                                                                                                                                  | up to 16 ch |
| **ADC1**            | CH0 = GPIO36, CH1 = GPIO37, CH2 = GPIO38, CH3 = GPIO39, CH4 = GPIO32, CH5 = GPIO33, CH6 = GPIO34, CH7 = GPIO35                                                           | 8           |
| **ADC2**            | CH0 = GPIO4, CH1 = GPIO0, CH2 = GPIO2, CH3 = GPIO15, CH4 = GPIO13, CH5 = GPIO12, CH6 = GPIO14, CH7 = GPIO27, CH8 = GPIO25, CH9 = GPIO26 *(Wi-Fi and buetooth blocks ADC2)*             | 10          |
| **DAC**             | DAC1 = GPIO25, DAC2 = GPIO26                                                                                                                                           | 2           |
| **Touch**           | T0 = GPIO4, T1 = GPIO0, T2 = GPIO2, T3 = GPIO15, T4 = GPIO13, T5 = GPIO12, T6 = GPIO14, T7 = GPIO27, T8 = GPIO33, T9 = GPIO32                                              | 10          |
| **Hall Sensor**     | Internal via ADC1_CH0/ADC1_CH3 (GPIO36/GPIO39)                                                                                                                         | 2 (shared)  |
| **CAN (TWAI)**      | TX = GPIO21*, RX = GPIO22* *(fully remappable)*                                                                                                                         | 2           |
| **Ethernet (RMII)** | Multiple dedicated pins: GPIO0, GPIO23, GPIO19, GPIO22, GPIO25, GPIO26, GPIO27, GPIO14, GPIO12, GPIO15 *(requires EMAC interface)*                                      | ~9          |
| **Internal Wi-Fi**  | Internal RF — no external GPIO required                                                                                                                                | 0           |
| **Internal BLE**    | Internal RF — no external GPIO required                                                                                                                                | 0           |

\* **Total Pins** column counts distinct GPIOs in default mapping — many peripherals can be remapped using the ESP32 GPIO matrix.  
\** Pins 6–11 are usually connected to the onboard flash chip on ESP32-WROOM modules — avoid using them externally.



## 6. HW Wiring

| HW Component         | Peripheral    | Used    | Notes / Usage                                     |
| ---------------------| ------------- | ------- | ------------------------------------------------- |
| **Console/Flash**    | UART0         | Yes     | Debug / programming console                       |
| **IOExpender**       | I²C1          | Yes     | Handle Relayes                                    |
| **Display**          | GPIO          | Yes     | Display System Status on Character Screen         |
| **Keypad**           | GPIO          | Yes     | Get User Inputs                                   |
| **Temp0 NTC**        | ADC1          | Yes     | NTC temperature sensing                           |
| **Temp1 NTC**        | ADC1          | Yes     | NTC temperature sensing                           |
| **Temp_Hum0**        | GPIO          | Yes     | Digital temperature/humidity Sensor               |
| **Temp_Hum1**        | GPIO          | Yes     | Digital temperature/humidity Sensor               |
| **Temp_Hum2**        | GPIO          | Yes     | Digital temperature/humidity Sensor               |
| **Temp_Hum3**        | GPIO          | Yes     | Digital temperature/humidity Sensor               |
| **Alarm**            | GPIO          | Yes     | for communication                                 |
| **FAN 1**            | IOExpender0   | Yes     | Control device Connected to Realy                 |
| **FAN 2**            | IOExpender1   | Yes     | Control device Connected to Realy                 |
| **FAN 3**            | IOExpender2   | Yes     | Control device Connected to Realy                 |
| **FAN 4**            | IOExpender3   | Yes     | Control device Connected to Realy                 |
| **VEN 1**            | IOExpender4   | Yes     | Control device Connected to Realy                 |
| **VEN 2**            | IOExpender5   | Yes     | Control device Connected to Realy                 |
| **VEN 3**            | IOExpender6   | Yes     | Control device Connected to Realy                 |
| **VEN 4**            | IOExpender7   | Yes     | Control device Connected to Realy                 |
| **Light**            | IOExpender8   | Yes     | Control device Connected to Realy                 |
| **Heater 1**         | IOExpender9   | Yes     | Control device Connected to Realy                 |
| **PUMP 1**           | IOExpender10  | Yes     | Control device Connected to Realy                 |
| **FAN LED**          | IOExpender11  | Yes     | Control device Connected to Realy                 |
| **VEN LED**          | IOExpender12  | Yes     | Control device Connected to Realy                 |
| **Light LED**        | IOExpender13  | Yes     | Control device Connected to Realy                 |
| **Pump LED**         | IOExpender14  | Yes     | Control device Connected to Realy                 |
| **Heater LED**       | IOExpender15  | Yes     | Control device Connected to Realy                 |
| **Modbus or KNX**    | UART1         | No      | for communication                                 |
| **WIFI**             | ADC2(Blocked) | No      | for communication                                 |
| **BLE**              | ADC2(Blocked) | Yes     | for communication                                 |
| **External Flash**   | SPI_A(VSPI)   | No      | Data Storage                                      |
| **EEPROM**           | I²C0          | No      | Data Storage                                      |



## Console & Flash Wiring
| HW Component   | Pin Assignment            | Pin Direction         | Implemented | Notes /Usage                                      |
|----------------|---------------------------|---------------------- | ----------- |-------------------------------------------------- |
| Console/Flash  | TX0 = GPIO1, RX0 = GPIO3  | TX: Output, RX: Input |     YES     | Standard debug and programming interface (UART0). |

---

## IO Expander
| HW Component   | Pin Assignment             | Pin Direction       | Implemented | Notes / Usage                                            |
|----------------|--------------------------- |---------------------| ----------- |--------------------------------------------------------- |
| IO Expander    | SDA = GPIO21, SCL = GPIO22 | Bidirectional (I/O) |     YES     | I²C1 bus for communication with the CH423S GPIO Expander.|

---

## Display Wiring (4-bit Mode)
| HW Component | Pin Assignment | Pin Direction | Implemented | Notes / Usage                                   |
|--------------|----------------|---------------| ----------- |-------------------------------------------------|
| Display      | RS = GPIO25, E = GPIO26, DB4 = GPIO27, DB5 = GPIO32, DB6 = GPIO33, DB7 = GPIO14  | Output |     YES     | Displays system status on the character screen. **Note: GPIO14 is a strapping pin; its state during boot affects boot mode. Ensure proper external pull-up/down if it needs to be high/low at boot.** |

---


## Keypad Wiring (4x4 Matrix)
| HW Component | Pin Assignment                 | Pin Direction | Implemented | Notes / Usage              |
|--------------|--------------------------------|---------------| ----------- |----------------------------|
| Keypad Rows  | R1 = GPIO4, R2 = GPIO12, R3 = GPIO13, R4 = GPIO0 | Output  |     YES     | Sends signals to scan rows. **CAUTION: GPIO0 is a strapping pin (boot button); holding it low during boot enters flash mode. GPIO4, 12, 13 are also strapping pins. Careful hardware design (external pull-ups/downs) is crucial to ensure correct boot behavior and prevent unintended actions.** |
| Keypad Columns | Col1 = GPIO36, Col2 = GPIO37, Col3 = GPIO38, Col4 = GPIO39 | Input |     YES     | Reads signals to detect key presses. **Ideal as these are input-only GPIOs (ADC1 channels 0-3 respectively) and do not conflict with Wi-Fi/BLE.** |

---

## NTC Temperature Sensors
| HW Component  | Pin Assignment       | Pin Direction | Implemented | Notes / Usage           |
|---------------|----------------------|---------------| ----------- |-------------------------|
| Temp0 NTC     | GPIO34 (ADC1_CH6)    | Input         |     YES     | NTC temperature sensing.|
| Temp1 NTC     | GPIO35 (ADC1_CH7)    | Input         |     YES     | NTC temperature sensing.|
---

## Digital Temp/Humidity Sensors
| HW Component   | Pin Assignment | Pin Direction        | Implemented | Notes / Usage                        |
|----------------|----------------|----------------------| ----------- |--------------------------------------|
| Temp_Hum0      | GPIO19         | Bidirectional (I/O)  |     YES     | Digital temperature/humidity sensor  |
| Temp_Hum1      | GPIO23         | Bidirectional (I/O)  |     YES     | Digital temperature/humidity sensor. |
| Temp_Hum2      | GPIO18         | Bidirectional (I/O)  |     YES     | Digital temperature/humidity sensor. |
| Temp_Hum3      | GPIO5          | Bidirectional (I/O)  |     YES     | Digital temperature/humidity sensor. **Note: GPIO5 is a strapping pin; its state during boot affects boot mode (strap low for boot). Use with caution, especially if configured as output.** |

---

## Alarm
| HW Component   | Pin Assignment | Pin Direction        | Implemented | Notes / Usage                        |
|----------------|----------------|----------------------| ----------- |--------------------------------------|
| Alarm          | GPIO2          | Output               |     YES     | For Warning. **Note: GPIO2 is a strapping pin (strap high for boot); ensure its state at boot does not interfere with boot mode. Also an ADC2 channel, so avoid analog use if Wi-Fi/BLE are active.** |

---



## Relay Control (FANs, VENs, Heater, Pump, Lights)
| HW Component | Pin Assignment | Pin Direction | Notes / Usage            |
|--------------|----------------|---------------|--------------------------|
| FAN 1        | GPIO25         | Output        | Controlled by a relay.   |
| FAN 2        | GPIO26         | Output        | Controlled by a relay.   |
| FAN 3        | GPIO27         | Output        | Controlled by a relay.   |
| FAN 4        | GPIO32         | Output        | Controlled by a relay.   |
| VEN 1        | GPIO33         | Output        | Controlled by a relay.   |
| VEN 2        | GPIO4          | Output        | Controlled by a relay.   |
| VEN 3        | GPIO5          | Output        | Controlled by a relay.   |
| VEN 4        | GPIO12         | Output        | Controlled by a relay.   |
| Heater 1     | GPIO1          | Output        | Controlled by a relay.   |
| PUMP 1       | GPIO15         | Output        | Controlled by a relay.   |
| Light0       | GPIO2          | Output        | Controlled by a relay.   |
| Light1       | GPIO14         | Output        | Controlled by a relay.   |

---



## Other Peripherals
| HW Component   | Pin Assignment | Pin Direction | Notes / Usage                                              |
|----------------|----------------|---------------|------------------------------------------------------------|
| Modbus or KNX  | Not Used       | N/A           | UART1 is available (GPIO17/16) but currently not assigned. |
| Wi-Fi(Internal)| N/A            | N/A           | Conflicts with ADC2 if ADC2 channels were used.            |
| BLE (Internal) | N/A            | N/A           | Conflicts with ADC2 if ADC2 channels were used.            |
| External Flash | Not Used       | N/A           | SPI_A(VSPI) is available if other assigned GPIO are moved. |
| EEPROM         | I²C0 (Not Used)| N/A           | I²C0 bus is available (SDA=GPIO21, SCL=GPIO22 based on typical pins, but these are now used by I2C1 for the expander). If needed, a different I²C bus or expander for EEPROM would be required. |
---


## GPIO Expander 1
| HW Component       | Pin Assignment | Pin Direction | Notes / Usage                        |
|--------------------|----------------|---------------|--------------------------------------|
| FAN 1              | CH423S GP0     | Output        | Control device connected to a relay. |
| FAN 2              | CH423S GP1     | Output        | Control device connected to a relay. |
| FAN 3              | CH423S GP2     | Output        | Control device connected to a relay. |
| FAN 4              | CH423S GP3     | Output        | Control device connected to a relay. |
| VEN 1              | CH423S GP4     | Output        | Control device connected to a relay. |
| VEN 2              | CH423S GP5     | Output        | Control device connected to a relay. |
| VEN 3              | CH423S GP6     | Output        | Control device connected to a relay. |
| VEN 4              | CH423S GP7     | Output        | Control device connected to a relay. |
| Light              | CH423S GP8     | Output        | Control device connected to a relay. |
| Heater 1           | CH423S GP9     | Output        | Control device connected to a relay. |
| PUMP 1             | CH423S GP10    | Output        | Control device connected to a relay. |
| FAN LED            | CH423S GP11    | Output        | Status LED for FAN.                  |
| VEN LED            | CH423S GP12    | Output        | Status LED for VENTILATOR.           |
| Light LED          | CH423S GP13    | Output        | Status LED for LIGHT.                |
| Pump LED           | CH423S GP14    | Output        | Status LED for PUMP.                 |
| Heater LED         | CH423S GP15    | Output        | Status LED for HEATER.               |



# Component & Reference List

---

## 1. ESP32-S3-FH4R2 SMD WiFi + BLE5.0 RF SoC (4MB Flash, 2MB PSRAM, QFN56)
- **Buyer Website:** [UGE-One Link](https://uge-one.com/product/esp32-s3-fh4r2-smd-wifi-ble5-0-rf-system-on-a-chip-soc-with-4mb-flash-2mb-psram-qfn56/)
- **Price:** 205.2 EGP
- **Official Product Page:** [Espressif Product Selector](https://products.espressif.com/#/product-selector?language=en&names=ESP32-S3FH4R2)
- **Datasheet:** [ESP32 Datasheet (PDF)](https://www.espressif.com/sites/default/files/documentation/esp32_datasheet_en.pdf)

---

## 2. I2C to GPIO IO Expander Controller (CH423S, SOP28)
- **Buyer Website:** [UGE-One Link](https://uge-one.com/product/i2c-to-gpio-io-expander-controller-ch423s-sop28/)
- **Price:** 60 EGP
- **Datasheet:** [WCH IC Search](https://search.wch-ic.com/)

---

## 3. DHT22 Temperature & Humidity Sensor Module
- **Buyer Website:** [UGE-One Link](https://uge-one.com/product/dht22-temp-humidity-sensor-module/)
- **Price:** 171 EGP
- **Datasheet:** [DHT22 Datasheet (PDF)](https://cdn.sparkfun.com/assets/f/7/d/9/c/DHT22.pdf)
- **Example Application:** [DHT22 Long-Distance Measurement (StackExchange)](https://raspberrypi.stackexchange.com/questions/108330/dht22-long-distance-temperature-measurements-24-meters-wired?utm_source=chatgpt.com)

---

## 4. DHT11 Digital Temperature & Humidity Sensor
- **Buyer Website:** [UGE-One Link](https://uge-one.com/product/dht11-digital-output-temperature-and-humidity-sensor/)
- **Price:** 61 EGP
- **Datasheet:** [DHT11 Product Page](https://uge-one.com/product/dht11-digital-output-temperature-and-humidity-sensor/)

---

## 5. Relay SPDT 5-Pin 12V (T78L DC12V C)
- **Buyer Website:** [UGE-One Link](https://uge-one.com/product/relay-spdt-5pin-12v-t78l-dc12v-c/)
- **Price:** 25 EGP

---

## 6. NTC 10K Waterproof Sensor (50cm Cable)
- **Buyer Website:** [UGE-One Link](https://uge-one.com/product/ntc-10k-waterproof-sensor-with-50cm-cable/)
- **Price:** 65 EGP

---

## PCB Design Examples
- [PCB for ESP32 16 Channels Relay Module IoT Development Board](https://uge-one.com/product/pcb-for-esp32-16-channels-relay-module-iot-development-board/)
- [16 Channels Relay Module for ESP32 IoT Development Board](https://uge-one.com/product/16-channels-relay-module-for-esp32-iot-development-board/)
- [ESP32 8-Channel Wi-Fi + Bluetooth Relay Module (IoT & Smart Home)](https://uge-one.com/product/esp32-8-channel-wi-fi-bluetooth-relay-module-iot-and-smart-home/)
- [16-Channel Relay Module 5V/12V Control Board with Optocoupler Protection + LM2596 Power Supply](https://uge-one.com/product/16-channel-relay-module-5v-12v-control-board-with-optocoupler-protection-belt-in-lm2596-power-supply/)
- [PCB for ESP32 8 Relay 5V Module](https://uge-one.com/product/pcb-for-esp32-8-relay-5v-module/)
- [ESP32 8-Channel Wi-Fi + Bluetooth Relay Module (IoT & Smart Home) - Alt Link](https://uge-one.com/product/esp32-8-channel-wi-fi-bluetooth-relay-module-iot-and-smart-home/#)

---

## Reference Documents
- [TP54 User Manual (PDF)](https://samwha.com.br/wp-content/uploads/2025/02/TP54-User-Manual.pdf)
- [Chickatron 20-2 Controller](https://tempclimatecontroller.com/en/chickatron-20-2/)
