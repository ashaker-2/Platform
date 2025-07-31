## TP54 Climate Controller System Requirements

### 1. Functional Requirements

#### 1.1. Monitoring and Display
* The System shall display the current indoor temperature.  
* The System shall display the current indoor humidity if a humidity sensor is connected. 
* If no humidity sensor is present, the system shall display fan speed in place of humidity. 
* The display shall alternate between temperature and humidity readings.  
* The System shall display the current fan speed.  
* The System shall display alarm statuses if errors occur.  
* The System shall indicate an "INL" alarm when the current temperature is greater than or equal to the specified temperature.  
* The System shall indicate an alarm when the current temperature is less than or equal to the specified temperature.  
* The System shall indicate an alarm when the current humidity is greater than or equal to the specified humidity.  
* Upon power-on, the System shall display its version number.
* The System shall record and display the maximum temperature recorded.
* The System shall record and display the minimum temperature recorded.
* The System shall continuously record data.

#### 1.2. Control Outputs
* The System shall have 4 Fan Relay Outputs. 
* The System shall have 1 Cool Relay Output. 
* The System shall have 1 Spray (OPT) Relay Output. 
* The System shall have 1 Heat Relay Output.  
* The System shall have 1 Alarm Relay Output.  
* All relay outputs shall be dry contact.  
* The System shall be capable of connecting to a variable speed fan via an A-OUT (0-10V) analog output.  
* The System shall control Fan group 1 via the FAN1 output.  
* The System shall control Fan group 2 via the FAN2 output.  
* The System shall control Fan group 3 via the FAN3 output.  
* The System shall control Fan group 4 via the FAN4 output.  
* The System shall control a cooling pump via the PUMP output.
* The System shall control a heater via the HT output.  
* The System shall activate an external relay via the NO/COM/NC alarm output. 

#### 1.3. User Interaction and Configuration
* The user shall be able to cycle through display modes (indoor temperature, indoor humidity/fan speed, current fan speed, alarm status) by pressing a designated button.   
* The user shall be able to clear recorded temperature data by pressing a designated button for 5 seconds. 
* The user shall be able to change individual set points for functions such as Ventilation.  
* The system shall display the set point alternately with the function name (e.g., "Fan 1"). 
* The user shall be able to increase a set point using an "increase" button.  
* The user shall be able to decrease a set point using a "reduce" button.  
* The user shall be able to navigate to the next setting (e.g., Fan2) by pressing the function button again.  
* The user shall be able to enter a setting mode for any featured function by pressing its respective button.  
* The user shall be able to exit the current setting mode by pressing any other function button.

#### 1.4. Data Handling
* The system shall clear recorded data upon power-off. 

### 2. Non-Functional Requirements

#### 2.1. Performance Requirements
* **Measurement Accuracy - Temperature**: The temperature measurement shall have an accurate value of $0.1^{\circ}C$.  
* **Measurement Deviation - Temperature**: The temperature measurement shall have a deviation value of $\pm1\%$.  
* **Measurement Accuracy - Humidity**: The humidity measurement shall have an accurate value of 0.1%.  
* **Measurement Deviation - Humidity**: The humidity measurement shall have a deviation value of $\pm5\%$.  

#### 2.2. Electrical and Power Requirements
* **Input Power Voltage**: The System shall operate with an input power voltage of 220 VAC $\pm$10VAC.  
* **Frequency**: The System shall operate at a frequency of 50-60 Hz.  
* **Electric Power Consumption**: The System shall consume 9 VA of electric power.  
* **Relay Fireproofing Rate**: The relays shall have a fireproofing rate of 5A/250VAC.  
* **Relay Alarm Fireproofing Rate**: The relay alarm shall have a fireproofing rate of 0.5A 250VAC / 1A 30VDC.  
* **Output Dry Contact Rating**: All dry contact outputs shall have a maximum rating of 3A/250VAC.  
* **Earthing**: The protective conductor (P.E.) shall be connected to the G (Ground) connector.  
* **Earth Electrode Resistance**: If the earth electrode resistance is greater than 5 ohms, a new earth electrode must be used.  71]
* **Circuit Breaker**: The system input power line shall be protected by a 10A Circuit Breaker.  
* **Surge Protection**: The system input power line shall include surge protection.  106]
* **Ground Wire**: The ground wire shall be 6 square mm.  
* **Ground Resistance**: The ground resistance shall be less than 5 Ohm. 

#### 2.3. Environmental Requirements
* **Operating Temperature**: The System shall operate within a temperature range of -20 to $60^{\circ}C$.  
* **Measurement Range - Temperature**: The System shall measure temperature within a range of -10.0 to $70.0^{\circ}C$.  
* **Measurement Range - Humidity**: The System shall measure humidity within a range of 0.0-99.9%RH.  
* **Installation Location**: The System shall be installed in a dry place.  
* **Installation Proximity to Electrical Cabinet**: The System shall be installed approximately 1 meter away from the electrical cabinet.  

#### 2.4. Usability Requirements
* The display representation of temperature and humidity shall be alternately displayed.  

#### 2.5. Maintainability and Reliability
* The product warranty period is one year from the date of invoice.  
* The product shall be free of defects in materials or workmanship.  
* The product shall conform to technical specifications.  
* The user shall provide adequate backup and alarm systems for critical operations in case of controller failure.

### 3. Interface Requirements

#### 3.1. Hardware Interfaces
* The System shall provide connectors for L (220 VAC) and N (0 VAC) for power input.  
* The System shall provide a G (Ground) connector for a protective conductor.  
* The System shall support a temperature sensor connected to T-IN and 0V inputs using a two-wire cable. 
* The System shall support a humidity sensor connected to 12V (Brown), 0V (Green), and HUM (White) inputs using a three-wire cable. 
* The System shall have multiple relay outputs, including FAN1, FAN2, FAN3, FAN4, PUMP, HT, and NO/COM/NC (Alarm). 
* The System shall have an A-OUT for analog output.

#### 3.2. Communication Interfaces
* The System shall support network communication using an RS-485 cable.  
* The RS-485 connection shall utilize D+, D-, and G (shield) wires.  
* The System can be connected to a maximum of 30 nodes in a communication network.  
* The network shall utilize TCLP (Temp Communication Line Protection) for protection.  
* The system shall be able to convert data to a computer via a MASTER LINK.  
* The system shall have a program interface (TEMP.VIEW) that allows parameter editing and status viewing from connected houses. 
