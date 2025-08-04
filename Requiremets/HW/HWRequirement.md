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
