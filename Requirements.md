## Climate Controller System Requirements

### 1. Functional Requirements

#### 1.1. Monitoring and Display
* [cite_start]The System shall display the current indoor temperature. [cite: 240]
* [cite_start]The System shall display the current indoor humidity if a humidity sensor is connected. [cite: 241]
* [cite_start]If no humidity sensor is present, the system shall display fan speed in place of humidity. [cite: 250]
* [cite_start]The display shall alternate between temperature and humidity readings. [cite: 240]
* [cite_start]The System shall display the current fan speed. [cite: 251]
* [cite_start]The System shall display alarm statuses if errors occur. [cite: 252]
* [cite_start]The System shall indicate an "INL" alarm when the current temperature is greater than or equal to the specified temperature. [cite: 253]
* [cite_start]The System shall indicate an alarm when the current temperature is less than or equal to the specified temperature. [cite: 255]
* [cite_start]The System shall indicate an alarm when the current humidity is greater than or equal to the specified humidity. [cite: 256]
* [cite_start]Upon power-on, the System shall display its version number. [cite: 243, 244]
* [cite_start]The System shall record and display the maximum temperature recorded. [cite: 262]
* [cite_start]The System shall record and display the minimum temperature recorded. [cite: 263]
* [cite_start]The System shall continuously record data. [cite: 266]

#### 1.2. Control Outputs
* [cite_start]The System shall have 4 Fan Relay Outputs. [cite: 12]
* [cite_start]The System shall have 1 Cool Relay Output. [cite: 13]
* [cite_start]The System shall have 1 Spray (OPT) Relay Output. [cite: 14]
* [cite_start]The System shall have 1 Heat Relay Output. [cite: 15]
* [cite_start]The System shall have 1 Alarm Relay Output. [cite: 16]
* [cite_start]All relay outputs shall be dry contact. [cite: 171]
* [cite_start]The System shall be capable of connecting to a variable speed fan via an A-OUT (0-10V) analog output. [cite: 176, 195]
* [cite_start]The System shall control Fan group 1 via the FAN1 output. [cite: 175]
* [cite_start]The System shall control Fan group 2 via the FAN2 output. [cite: 175]
* [cite_start]The System shall control Fan group 3 via the FAN3 output. [cite: 175]
* [cite_start]The System shall control Fan group 4 via the FAN4 output. [cite: 175]
* [cite_start]The System shall control a cooling pump via the PUMP output. [cite: 175]
* [cite_start]The System shall control a heater via the HT output. [cite: 175]
* [cite_start]The System shall activate an external relay via the NO/COM/NC alarm output. [cite: 172, 175]

#### 1.3. User Interaction and Configuration
* [cite_start]The user shall be able to cycle through display modes (indoor temperature, indoor humidity/fan speed, current fan speed, alarm status) by pressing a designated button. [cite: 246, 248, 249, 250, 251, 252]
* [cite_start]The user shall be able to clear recorded temperature data by pressing a designated button for 5 seconds. [cite: 264, 265]
* [cite_start]The user shall be able to change individual set points for functions such as Ventilation. [cite: 268, 270]
* [cite_start]The system shall display the set point alternately with the function name (e.g., "Fan 1"). [cite: 271]
* [cite_start]The user shall be able to increase a set point using an "increase" button. [cite: 272, 273]
* [cite_start]The user shall be able to decrease a set point using a "reduce" button. [cite: 273, 274]
* [cite_start]The user shall be able to navigate to the next setting (e.g., Fan2) by pressing the function button again. [cite: 275, 276]
* [cite_start]The user shall be able to enter a setting mode for any featured function by pressing its respective button. [cite: 277]
* [cite_start]The user shall be able to exit the current setting mode by pressing any other function button. [cite: 278]

#### 1.4. Data Handling
* [cite_start]The system shall clear recorded data upon power-off. [cite: 266]

### 2. Non-Functional Requirements

#### 2.1. Performance Requirements
* [cite_start]**Measurement Accuracy - Temperature**: The temperature measurement shall have an accurate value of $0.1^{\circ}C$. [cite: 64]
* [cite_start]**Measurement Deviation - Temperature**: The temperature measurement shall have a deviation value of $\pm1\%$. [cite: 64]
* [cite_start]**Measurement Accuracy - Humidity**: The humidity measurement shall have an accurate value of 0.1%. [cite: 64]
* [cite_start]**Measurement Deviation - Humidity**: The humidity measurement shall have a deviation value of $\pm5\%$. [cite: 64]

#### 2.2. Electrical and Power Requirements
* [cite_start]**Input Power Voltage**: The System shall operate with an input power voltage of 220 VAC $\pm$10VAC. [cite: 64]
* [cite_start]**Frequency**: The System shall operate at a frequency of 50-60 Hz. [cite: 64]
* [cite_start]**Electric Power Consumption**: The System shall consume 9 VA of electric power. [cite: 64]
* [cite_start]**Relay Fireproofing Rate**: The relays shall have a fireproofing rate of 5A/250VAC. [cite: 64]
* [cite_start]**Relay Alarm Fireproofing Rate**: The relay alarm shall have a fireproofing rate of 0.5A 250VAC / 1A 30VDC. [cite: 64]
* [cite_start]**Output Dry Contact Rating**: All dry contact outputs shall have a maximum rating of 3A/250VAC. [cite: 171]
* [cite_start]**Earthing**: The protective conductor (P.E.) shall be connected to the G (Ground) connector. [cite: 70]
* [cite_start]**Earth Electrode Resistance**: If the earth electrode resistance is greater than 5 ohms, a new earth electrode must be used. [cite: 71]
* [cite_start]**Circuit Breaker**: The system input power line shall be protected by a 10A Circuit Breaker. [cite: 105]
* [cite_start]**Surge Protection**: The system input power line shall include surge protection. [cite: 106]
* [cite_start]**Ground Wire**: The ground wire shall be 6 square mm. [cite: 107]
* [cite_start]**Ground Resistance**: The ground resistance shall be less than 5 Ohm. [cite: 110]

#### 2.3. Environmental Requirements
* [cite_start]**Operating Temperature**: The System shall operate within a temperature range of -20 to $60^{\circ}C$. [cite: 64]
* [cite_start]**Measurement Range - Temperature**: The System shall measure temperature within a range of -10.0 to $70.0^{\circ}C$. [cite: 64]
* [cite_start]**Measurement Range - Humidity**: The System shall measure humidity within a range of 0.0-99.9%RH. [cite: 64]
* [cite_start]**Installation Location**: The System shall be installed in a dry place. [cite: 68]
* [cite_start]**Installation Proximity to Electrical Cabinet**: The System shall be installed approximately 1 meter away from the electrical cabinet. [cite: 68]

#### 2.4. Usability Requirements
* [cite_start]The display representation of temperature and humidity shall be alternately displayed. [cite: 240]

#### 2.5. Maintainability and Reliability
* [cite_start]The product warranty period is one year from the date of invoice. [cite: 35]
* [cite_start]The product shall be free of defects in materials or workmanship. [cite: 36]
* [cite_start]The product shall conform to technical specifications. [cite: 36]
* [cite_start]The user shall provide adequate backup and alarm systems for critical operations in case of controller failure. [cite: 54, 55]

### 3. Interface Requirements

#### 3.1. Hardware Interfaces
* [cite_start]The System shall provide connectors for L (220 VAC) and N (0 VAC) for power input. [cite: 69, 113, 114]
* [cite_start]The System shall provide a G (Ground) connector for a protective conductor. [cite: 70]
* [cite_start]The System shall support a temperature sensor connected to T-IN and 0V inputs using a two-wire cable. [cite: 119, 120, 141]
* [cite_start]The System shall support a humidity sensor connected to 12V (Brown), 0V (Green), and HUM (White) inputs using a three-wire cable. [cite: 122, 123, 138, 139]
* [cite_start]The System shall have multiple relay outputs, including FAN1, FAN2, FAN3, FAN4, PUMP, HT, and NO/COM/NC (Alarm). [cite: 175, 186, 189, 190, 192, 182, 194, 180, 181]
* [cite_start]The System shall have an A-OUT for analog output. [cite: 176, 184]

#### 3.2. Communication Interfaces
* [cite_start]The System shall support network communication using an RS-485 cable. [cite: 160]
* [cite_start]The RS-485 connection shall utilize D+, D-, and G (shield) wires. [cite: 160, 161]
* [cite_start]The System can be connected to a maximum of 30 nodes in a communication network. [cite: 156]
* [cite_start]The network shall utilize TCLP (Temp Communication Line Protection) for protection. [cite: 166]
* [cite_start]The system shall be able to convert data to a computer via a MASTER LINK. [cite: 165, 166]
* [cite_start]The system shall have a program interface (TEMP.VIEW) that allows parameter editing and status viewing from connected houses. [cite: 167]
