# **Detailed Design Document: TemperatureControl Component**

## **1. Introduction**

### **1.1. Purpose**

This document details the design of the TemperatureControl component. Its primary purpose is to provide a high-level interface for acquiring temperature data from various types of temperature sensors (e.g., NTC thermistors, DS18B20 digital sensors, RTD, thermocouples). It abstracts the complexities of the underlying analog (ADC) or digital (I2C, SPI, 1-Wire, or custom GPIO protocol) sensor drivers, allowing systemMgr to simply request the current temperature value.

### **1.2. Scope**

The scope of this document covers the temperature module's architecture, functional behavior, interfaces, dependencies, and resource considerations. It details how temperature receives read requests from systemMgr (via RTE services) and interacts with the underlying MCAL_ADC, MCAL_I2C, MCAL_SPI, MCAL_GPIO, or HAL_1WIRE drivers for sensor communication.

### **1.3. References**

* Software Architecture Document (SAD) - Smart Device Firmware (Final Version)  
* Detailed Design Document: RTE  
* Detailed Design Document: MCAL_ADC (if analog temperature sensor)  
* Detailed Design Document: MCAL_I2C (if I2C temperature sensor)  
* Detailed Design Document: MCAL_SPI (if SPI temperature sensor)  
* Detailed Design Document: MCAL_GPIO (if custom digital protocol or NTC with external ADC)  
* Detailed Design Document: HAL_1WIRE (if 1-Wire sensors like DS18B20 are used)  
* Temperature Sensor Datasheet (e.g., NTC, DS18B20, LM35, PT100, K-type thermocouple)

## **2. Functional Description**

The TemperatureControl component provides the following core functionalities:

1. **Initialization**: Initialize the sensor's communication interface (e.g., ADC channel, I2C bus, 1-Wire bus) and perform any sensor-specific power-up or configuration sequences.  
2. **Read Temperature**: Acquire the current temperature value from the sensor. This may involve reading raw data, applying calibration, and converting to a standard unit (e.g., Celsius).  
3. **Error Reporting**: Detect and report any failures during sensor communication or data acquisition (e.g., sensor not responding, CRC error, out-of-range reading) to the SystemMonitor via RTE_Service_SystemMonitor_ReportFault().

## **3. Non-Functional Requirements**

### **3.1. Performance**

* **Accuracy**: Temperature measurements shall be accurate within specified tolerances (refer to HwReqirement.md).  
* **Update Rate**: Temperature data should be updated frequently enough to support the control loop (e.g., RTE_SensorReadTask periodicity).  
* **Responsiveness**: Sensor reads should complete within a reasonable timeframe.

### **3.2. Memory**

* **Minimal Footprint**: The temperature module shall have a minimal memory footprint.

### **3.3. Reliability**

* **Robustness**: The module shall be robust against sensor communication errors or invalid readings.  
* **Data Validity**: Implement checks for out-of-range or obviously erroneous sensor data.  
* **Retry Mechanism**: Implement a retry mechanism for transient sensor read failures.

## **4. Architectural Context**

As per the SAD (Section 3.1.2, Application Layer), temperature resides in the Application Layer. It receives read requests from systemMgr (via RTE_Service_TEMP_SENSOR_Read()). It then interacts with the appropriate MCAL or HAL drivers (MCAL_ADC, MCAL_I2C, MCAL_SPI, MCAL_GPIO, or HAL_1WIRE) to communicate with the physical temperature sensor.

## **5. Design Details**

### **5.1. Module Structure**

The TemperatureControl component will consist of the following files:

* TemperatureControl/inc/temperature.h: Public header file containing function prototypes and temperature-specific definitions.  
* TemperatureControl/src/temperature.c: Source file containing the implementation of temperature sensor reading logic.  
* TemperatureControl/cfg/temperature_cfg.h: Configuration header for sensor type, communication interface details (e.g., ADC channel, I2C address, 1-Wire bus ID), and calibration parameters.

### **5.2. Public Interface (API)**

// In TemperatureControl/inc/temperature.h
```c
#include "Application/common/inc/app_common.h" // For APP_Status_t

// --- Public Functions ---

/**  
 * @brief Initializes the Temperature module and the temperature sensor hardware.  
 * @return APP_OK on success, APP_ERROR on failure.  
 */  
APP_Status_t TEMPERATURE_Init(void);

/**  
 * @brief Reads the current temperature value from the sensor.  
 * @param temperature_c Pointer to store the temperature value in Celsius.  
 * @return APP_OK on successful read, APP_ERROR on failure.  
 */  
APP_Status_t TEMPERATURE_Read(float *temperature_c);
```

### **5.3. Internal Design**

The TemperatureControl module will encapsulate the sensor-specific communication protocol and data conversion. It will implement retry logic for robust reads.

1. **Initialization (TEMPERATURE_Init)**:  
   * **Communication Interface Init**:  
     * If TEMP_SENSOR_TYPE_ANALOG_ADC is configured: Call MCAL_ADC_Init() for the TEMP_ADC_CHANNEL.  
     * If TEMP_SENSOR_TYPE_I2C is configured: Call MCAL_I2C_Init() for the TEMP_I2C_PORT and TEMP_I2C_ADDRESS.  
     * If TEMP_SENSOR_TYPE_1WIRE_DS18B20 is configured: Call HAL_1WIRE_Init() for the ONE_WIRE_BUS_ID.  
   * Perform any sensor-specific power-up or configuration (e.g., sending commands to an I2C sensor, starting DS18B20 conversion).  
   * If any underlying MCAL/HAL initialization fails, report FAULT_ID_TEMP_SENSOR_INIT_FAILED to SystemMonitor via RTE_Service_SystemMonitor_ReportFault().  
   * Return APP_OK.  
2. **Read Temperature (TEMPERATURE_Read)**:  
   * Validate temperature_c pointer.  
   * Implement a retry loop (TEMP_READ_RETRIES times).  
   * Inside the loop:  
     * **Read Raw Data based on TEMP_SENSOR_TYPE**:  
       * If TEMP_SENSOR_TYPE_ANALOG_ADC: Call MCAL_ADC_Read(TEMP_ADC_CHANNEL).  
       * If TEMP_SENSOR_TYPE_I2C: Call MCAL_I2C_MasterWriteRead() to send read command and receive data.  
       * If TEMP_SENSOR_TYPE_1WIRE_DS18B20: Call HAL_1WIRE_ReadDevice(ONE_WIRE_BUS_ID, DS18B20_ROM_ADDRESS, raw_data_buf).  
     * **Data Validation and CRC**: Check for valid sensor response, and if applicable, verify CRC (e.g., for DS18B20).  
     * If data is valid:  
       * **Calibration and Conversion**: Apply calibration factors (from temperature_cfg.h) to convert raw sensor data into Celsius.  
       * Store the result in *temperature_c.  
       * Return APP_OK.  
     * If data is invalid or CRC fails, increment retry counter and continue loop.  
   * If all retries fail, report FAULT_ID_TEMP_SENSOR_READ_FAILED to SystemMonitor.  
   * Return APP_ERROR.

**Sequence Diagram (Example: systemMgr Reads Temperature - DS18B20 Sensor):**
```mermaid
sequenceDiagram  
    participant SystemMgr as Application/systemMgr  
    participant RTE as Runtime Environment  
    participant Temperature as TemperatureControl  
    participant HAL_1WIRE as HAL/1WIRE  
    participant SystemMonitor as Application/SystemMonitor

    SystemMgr->>RTE: RTE_Service_TEMP_SENSOR_Read(&temp_value)  
    RTE->>Temperature: TEMPERATURE_Read(&temp_value)  
    loop TEMP_READ_RETRIES  
        Temperature->>HAL_1WIRE: HAL_1WIRE_ReadDevice(ONE_WIRE_BUS_ID, DS18B20_ROM_ADDRESS, raw_data_buf)  
        alt HAL_1WIRE returns APP_ERROR or raw_data is invalid  
            HAL_1WIRE--xTemperature: Return APP_ERROR  
            Temperature->>Temperature: Increment retry counter, delay  
        else HAL_1WIRE returns APP_OK and raw_data is valid  
            HAL_1WIRE-->>Temperature: Return APP_OK (raw_data_buf)  
            Temperature->>Temperature: Apply calibration, convert to Celsius  
            Temperature->>Temperature: Store result in temp_value  
            Temperature-->>RTE: Return APP_OK  
            RTE-->>SystemMgr: Return APP_OK  
            break loop  
        end  
    end  
    alt All retries failed  
        Temperature->>SystemMonitor: RTE_Service_SystemMonitor_ReportFault(FAULT_ID_TEMP_SENSOR_READ_FAILED, SEVERITY_HIGH, 0)  
        Temperature--xRTE: Return APP_ERROR  
        RTE--xSystemMgr: Return APP_ERROR  
    end
```

### **5.4. Dependencies**

* **Application/common/inc/app_common.h**: For APP_Status_t.  
* **Application/logger/inc/logger.h**: For logging sensor errors.  
* **Application/SystemMonitor/inc/system_monitor.h**: For SystemMonitor_FaultId_t (e.g., FAULT_ID_TEMP_SENSOR_INIT_FAILED).  
* **Rte/inc/Rte.h**: For calling RTE_Service_SystemMonitor_ReportFault().  
* **Mcal/adc/inc/mcal_adc.h**: If using an analog temperature sensor.  
* **Mcal/i2c/inc/mcal_i2c.h**: If using an I2C temperature sensor.  
* **Mcal/spi/inc/mcal_spi.h**: If using an SPI temperature sensor.  
* **Mcal/gpio/inc/mcal_gpio.h**: If using custom digital protocol.  
* **HAL/1wire/inc/hal_1wire.h**: If using 1-Wire sensors like DS18B20.  
* **Service/os/inc/service_os.h**: For delays (SERVICE_OS_DelayMs, SERVICE_OS_DelayUs) needed for some protocols.

### **5.5. Error Handling**

* **Initialization Failure**: If underlying MCAL/HAL initialization fails, FAULT_ID_TEMP_SENSOR_INIT_FAILED is reported.  
* **Communication/Read Failure**: If sensor communication fails or returns invalid data after retries, FAULT_ID_TEMP_SENSOR_READ_FAILED is reported.  
* **Data Out of Range**: If the converted temperature value is outside a reasonable range, FAULT_ID_TEMP_SENSOR_OUT_OF_RANGE could be reported (or the value clamped).  
* **Retry Logic**: The internal retry mechanism helps mitigate transient communication issues.

### **5.6. Configuration**

The TemperatureControl/cfg/temperature_cfg.h file will contain:

* **Sensor Type**: Define TEMP_SENSOR_TYPE_ANALOG_ADC, TEMP_SENSOR_TYPE_I2C, TEMP_SENSOR_TYPE_1WIRE_DS18B20, etc.  
* **Communication Interface Details**:  
  * For ADC: TEMP_ADC_CHANNEL, TEMP_ADC_CALIBRATION_SLOPE, TEMP_ADC_CALIBRATION_OFFSET.  
  * For I2C: TEMP_I2C_PORT, TEMP_I2C_ADDRESS, sensor-specific commands.  
  * For 1-Wire: ONE_WIRE_BUS_ID, DS18B20_ROM_ADDRESS (if a single sensor, or discovery settings for multiple).  
* **Retry Count**: TEMP_READ_RETRIES.  
* **Read Delay**: TEMP_READ_DELAY_MS (time needed for sensor conversion, if applicable).

// Example: TemperatureControl/cfg/temperature_cfg.h

```c
// Choose temperature sensor type  
#define TEMP_SENSOR_TYPE_ANALOG_ADC     0  
#define TEMP_SENSOR_TYPE_I2C            0  
#define TEMP_SENSOR_TYPE_1WIRE_DS18B20  1

#if TEMP_SENSOR_TYPE_ANALOG_ADC  
#define TEMP_ADC_CHANNEL                MCAL_ADC_CHANNEL_0  
#define TEMP_ADC_CALIBRATION_SLOPE      0.1f // Example: 0.1C per ADC LSB  
#define TEMP_ADC_CALIBRATION_OFFSET     -50.0f // Example offset  
#elif TEMP_SENSOR_TYPE_I2C  
#define TEMP_I2C_PORT                   MCAL_I2C_PORT_0  
#define TEMP_I2C_ADDRESS                0x48 // Example for TMP102  
// Define sensor-specific commands/registers here  
#elif TEMP_SENSOR_TYPE_1WIRE_DS18B20  
#define ONE_WIRE_BUS_ID                 HAL_1WIRE_BUS_0  
// If only one DS18B20, can use a fixed address or discovery  
#define DS18B20_ROM_ADDRESS             {0x28, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x01} // Example ROM address  
#endif

#define TEMP_READ_RETRIES               3 // Number of times to retry a read on failure  
#define TEMP_READ_DELAY_MS              750 // Delay for DS18B20 conversion time (max 750ms)
```

### **5.7. Resource Usage**

* **Flash**: Low to moderate, depending on the complexity of the sensor protocol (e.g., 1-Wire requires more code than simple ADC).  
* **RAM**: Very low, for internal state and temporary read buffers.  
* **CPU**: Low to moderate, depending on sensor type. Analog reads are fast. Digital protocols like 1-Wire require precise timing and more CPU cycles.

## **6. Test Considerations**

### **6.1. Unit Testing**

* **Mock Dependencies**: Unit tests for temperature will mock MCAL_ADC_Read(), MCAL_I2C_MasterWriteRead(), HAL_1WIRE_ReadDevice(), SERVICE_OS_DelayMs(), and RTE_Service_SystemMonitor_ReportFault().  
* **Test Cases**:  
  * TEMPERATURE_Init: Verify correct MCAL/HAL initialization calls. Test initialization failure and fault reporting.  
  * TEMPERATURE_Read:  
    * Test successful reads: Mock raw data to simulate various temperatures and verify correct conversion to Celsius.  
    * Test read failures: Mock MCAL/HAL calls to return errors or invalid data. Verify retry logic and FAULT_ID_TEMP_SENSOR_READ_FAILED after all retries.  
    * Test CRC errors (for digital sensors like DS18B20).  
    * Test out-of-range raw data and verify FAULT_ID_TEMP_SENSOR_OUT_OF_RANGE (if implemented).

### **6.2. Integration Testing**

* **Temperature-MCAL/HAL Integration**: Verify that temperature correctly interfaces with the actual MCAL/HAL drivers and the physical temperature sensor.  
* **Accuracy Verification**: Compare temperature's reported values with a calibrated external thermometer.  
* **SystemMgr Integration**: Verify that systemMgr receives accurate temperature data from temperature via RTE.  
* **Fault Injection**: Disconnect the temperature sensor, or introduce noise on its communication lines, and verify that temperature reports FAULT_ID_TEMP_SENSOR_READ_FAILED to SystemMonitor.

### **6.3. System Testing**

* **End-to-End Control Loop**: Verify that the system's overall control loop (sensor read -> systemMgr logic -> actuator control) functions correctly based on temperature readings.  
* **Environmental Chambers**: Test the system in controlled temperature environments (e.g., climate chamber) to verify performance across the sensor's operating range.  
* **Long-Term Reliability**: Run the system for extended periods to ensure continuous and reliable temperature monitoring.
