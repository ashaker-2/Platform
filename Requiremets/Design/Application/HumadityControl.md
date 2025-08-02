# **Detailed Design Document: HumidityControl Component**

## **1. Introduction**

### **1.1. Purpose**

This document details the design of the HumidityControl component. Its primary purpose is to provide a high-level interface for acquiring humidity data from a connected humidity sensor. It abstracts the complexities of the underlying analog (ADC) or digital (I2C, SPI, or custom GPIO protocol) sensor drivers, allowing systemMgr to simply request the current humidity value.

### **1.2. Scope**

The scope of this document covers the humidity module's architecture, functional behavior, interfaces, dependencies, and resource considerations. It details how humidity receives read requests from systemMgr (via RTE services) and interacts with the underlying MCAL_ADC, MCAL_I2C, MCAL_SPI, or MCAL_GPIO drivers for sensor communication.

### **1.3. References**

* Software Architecture Document (SAD) - Smart Device Firmware (Final Version)  
* Detailed Design Document: RTE  
* Detailed Design Document: MCAL_ADC (if analog humidity sensor)  
* Detailed Design Document: MCAL_I2C (if I2C humidity sensor)  
* Detailed Design Document: MCAL_SPI (if SPI humidity sensor)  
* Detailed Design Document: MCAL_GPIO (if custom digital protocol or DHT-like sensor)  
* Humidity Sensor Datasheet (e.g., DHT11/22, SHT30, HIH-4030)

## **2. Functional Description**

The HumidityControl component provides the following core functionalities:

1. **Initialization**: Initialize the sensor's communication interface (e.g., ADC channel, I2C bus) and perform any sensor-specific power-up or configuration sequences.  
2. **Read Humidity**: Acquire the current humidity value from the sensor. This may involve reading raw data, applying calibration, and converting to a standard unit (e.g., percentage relative humidity).  
3. **Error Reporting**: Detect and report any failures during sensor communication or data acquisition (e.g., sensor not responding, CRC error, out-of-range reading) to the SystemMonitor via RTE_Service_SystemMonitor_ReportFault().

## **3. Non-Functional Requirements**

### **3.1. Performance**

* **Accuracy**: Humidity measurements shall be accurate within specified tolerances (refer to HwReqirement.md).  
* **Update Rate**: Humidity data should be updated frequently enough to support the control loop (e.g., RTE_SensorReadTask periodicity).  
* **Responsiveness**: Sensor reads should complete within a reasonable timeframe.

### **3.2. Memory**

* **Minimal Footprint**: The humidity module shall have a minimal memory footprint.

### **3.3. Reliability**

* **Robustness**: The module shall be robust against sensor communication errors or invalid readings.  
* **Data Validity**: Implement checks for out-of-range or obviously erroneous sensor data.  
* **Retry Mechanism**: Implement a retry mechanism for transient sensor read failures.

## **4. Architectural Context**

As per the SAD (Section 3.1.2, Application Layer), humidity resides in the Application Layer. It receives read requests from systemMgr (via RTE_Service_HUMIDITY_SENSOR_Read()). It then interacts with the appropriate MCAL drivers (MCAL_ADC, MCAL_I2C, MCAL_SPI, or MCAL_GPIO) to communicate with the physical humidity sensor.

## **5. Design Details**

### **5.1. Module Structure**

The HumidityControl component will consist of the following files:

* HumidityControl/inc/humidity.h: Public header file containing function prototypes and humidity-specific definitions.  
* HumidityControl/src/humidity.c: Source file containing the implementation of humidity sensor reading logic.  
* HumidityControl/cfg/humidity_cfg.h: Configuration header for sensor type, communication interface details (e.g., ADC channel, I2C address, GPIO pins), and calibration parameters.

### **5.2. Public Interface (API)**

// In HumidityControl/inc/humidity.h
```c
#include "Application/common/inc/app_common.h" // For APP_Status_t

// --- Public Functions ---

/**  
 * @brief Initializes the Humidity module and the humidity sensor hardware.  
 * @return APP_OK on success, APP_ERROR on failure.  
 */  
APP_Status_t HUMIDITY_Init(void);

/**  
 * @brief Reads the current humidity value from the sensor.  
 * @param humidity_percent Pointer to store the humidity value in percentage (0.0 - 100.0).  
 * @return APP_OK on successful read, APP_ERROR on failure.  
 */  
APP_Status_t HUMIDITY_Read(float *humidity_percent);
```

### **5.3. Internal Design**

The HumidityControl module will encapsulate the sensor-specific communication protocol and data conversion. It will implement retry logic for robust reads.

1. **Initialization (HUMIDITY_Init)**:  
   * **Communication Interface Init**:  
     * If HUMIDITY_SENSOR_TYPE_ANALOG_ADC is configured: Call MCAL_ADC_Init() for the HUMIDITY_ADC_CHANNEL.  
     * If HUMIDITY_SENSOR_TYPE_I2C is configured: Call MCAL_I2C_Init() for the HUMIDITY_I2C_PORT and HUMIDITY_I2C_ADDRESS.  
     * If HUMIDITY_SENSOR_TYPE_DHT is configured: Call MCAL_GPIO_Init() for the DHT_DATA_PIN and configure it as input/output as needed for the one-wire protocol.  
   * Perform any sensor-specific power-up or configuration (e.g., sending commands to an I2C sensor).  
   * If any underlying MCAL initialization fails, report FAULT_ID_HUMIDITY_INIT_FAILED to SystemMonitor via RTE_Service_SystemMonitor_ReportFault().  
   * Return APP_OK.  
2. **Read Humidity (HUMIDITY_Read)**:  
   * Validate humidity_percent pointer.  
   * Implement a retry loop (HUMIDITY_READ_RETRIES times).  
   * Inside the loop:  
     * **Read Raw Data based on HUMIDITY_SENSOR_TYPE**:  
       * If HUMIDITY_SENSOR_TYPE_ANALOG_ADC: Call MCAL_ADC_Read(HUMIDITY_ADC_CHANNEL).  
       * If HUMIDITY_SENSOR_TYPE_I2C: Call MCAL_I2C_MasterWriteRead() to send read command and receive data.  
       * If HUMIDITY_SENSOR_TYPE_DHT: Implement the DHT one-wire protocol using MCAL_GPIO for bit-banging and SERVICE_OS_DelayUs() for precise timing.  
     * **Data Validation and CRC**: Check for valid sensor response, and if applicable, verify CRC (e.g., for DHT, SHT30).  
     * If data is valid:  
       * **Calibration and Conversion**: Apply calibration factors (from humidity_cfg.h) to convert raw sensor data into percentage relative humidity.  
       * Store the result in *humidity_percent.  
       * Return APP_OK.  
     * If data is invalid or CRC fails, increment retry counter and continue loop.  
   * If all retries fail, report FAULT_ID_HUMIDITY_SENSOR_READ_FAILED to SystemMonitor.  
   * Return APP_ERROR.

**Sequence Diagram (Example: systemMgr Reads Humidity - I2C Sensor):**
```mermaid
sequenceDiagram  
    participant SystemMgr as Application/systemMgr  
    participant RTE as Runtime Environment  
    participant Humidity as HumidityControl  
    participant MCAL_I2C as MCAL/I2C  
    participant SystemMonitor as Application/SystemMonitor

    SystemMgr->>RTE: RTE_Service_HUMIDITY_SENSOR_Read(&humidity_value)  
    RTE->>Humidity: HUMIDITY_Read(&humidity_value)  
    loop HUMIDITY_READ_RETRIES  
        Humidity->>MCAL_I2C: MCAL_I2C_MasterWriteRead(HUMIDITY_I2C_PORT, HUMIDITY_I2C_ADDRESS, read_cmd, read_cmd_len, raw_data_buf, raw_data_len)  
        alt MCAL_I2C returns APP_ERROR or raw_data is invalid  
            MCAL_I2C--xHumidity: Return APP_ERROR  
            Humidity->>Humidity: Increment retry counter, delay  
        else MCAL_I2C returns APP_OK and raw_data is valid  
            MCAL_I2C-->>Humidity: Return APP_OK (raw_data_buf)  
            Humidity->>Humidity: Apply calibration, convert to percent  
            Humidity->>Humidity: Store result in humidity_value  
            Humidity-->>RTE: Return APP_OK  
            RTE-->>SystemMgr: Return APP_OK  
            break loop  
        end  
    end  
    alt All retries failed  
        Humidity->>SystemMonitor: RTE_Service_SystemMonitor_ReportFault(FAULT_ID_HUMIDITY_SENSOR_READ_FAILED, SEVERITY_HIGH, 0)  
        Humidity--xRTE: Return APP_ERROR  
        RTE--xSystemMgr: Return APP_ERROR  
    end
``````

### **5.4. Dependencies**

* **Application/common/inc/app_common.h**: For APP_Status_t.  
* **Application/logger/inc/logger.h**: For logging sensor errors.  
* **Application/SystemMonitor/inc/system_monitor.h**: For SystemMonitor_FaultId_t (e.g., FAULT_ID_HUMIDITY_INIT_FAILED).  
* **Rte/inc/Rte.h**: For calling RTE_Service_SystemMonitor_ReportFault().  
* **Mcal/adc/inc/mcal_adc.h**: If using an analog humidity sensor.  
* **Mcal/i2c/inc/mcal_i2c.h**: If using an I2C humidity sensor.  
* **Mcal/spi/inc/mcal_spi.h**: If using an SPI humidity sensor.  
* **Mcal/gpio/inc/mcal_gpio.h**: If using a custom digital protocol (e.g., DHT).  
* **Service/os/inc/service_os.h**: For delays (SERVICE_OS_DelayUs) needed for bit-banging protocols.

### **5.5. Error Handling**

* **Initialization Failure**: If underlying MCAL initialization fails, FAULT_ID_HUMIDITY_INIT_FAILED is reported.  
* **Communication/Read Failure**: If sensor communication fails or returns invalid data after retries, FAULT_ID_HUMIDITY_SENSOR_READ_FAILED is reported.  
* **Data Out of Range**: If the converted humidity value is outside a reasonable range (e.g., <0 or >100), FAULT_ID_HUMIDITY_SENSOR_OUT_OF_RANGE could be reported (or the value clamped).  
* **Retry Logic**: The internal retry mechanism helps mitigate transient communication issues.

### **5.6. Configuration**

The HumidityControl/cfg/humidity_cfg.h file will contain:

* **Sensor Type**: Define HUMIDITY_SENSOR_TYPE_ANALOG_ADC, HUMIDITY_SENSOR_TYPE_I2C, HUMIDITY_SENSOR_TYPE_DHT, etc.  
* **Communication Interface Details**:  
  * For ADC: HUMIDITY_ADC_CHANNEL, HUMIDITY_ADC_CALIBRATION_SLOPE, HUMIDITY_ADC_CALIBRATION_OFFSET.  
  * For I2C: HUMIDITY_I2C_PORT, HUMIDITY_I2C_ADDRESS, sensor-specific commands.  
  * For DHT: DHT_DATA_PIN, DHT_TYPE (11 or 22).  
* **Retry Count**: HUMIDITY_READ_RETRIES.  
* **Read Delay**: HUMIDITY_READ_DELAY_MS (time between reads if sensor requires it, or between retries).

// Example: HumidityControl/cfg/humidity_cfg.h

```c
// Choose humidity sensor type  
#define HUMIDITY_SENSOR_TYPE_ANALOG_ADC 0  
#define HUMIDITY_SENSOR_TYPE_I2C        1  
#define HUMIDITY_SENSOR_TYPE_DHT        0

#if HUMIDITY_SENSOR_TYPE_ANALOG_ADC  
#define HUMIDITY_ADC_CHANNEL            MCAL_ADC_CHANNEL_2  
#define HUMIDITY_ADC_CALIBRATION_SLOPE  0.04f // Example: 0.04%RH per ADC LSB  
#define HUMIDITY_ADC_CALIBRATION_OFFSET 0.0f  
#elif HUMIDITY_SENSOR_TYPE_I2C  
#define HUMIDITY_I2C_PORT               MCAL_I2C_PORT_0  
#define HUMIDITY_I2C_ADDRESS            0x40 // Example for SHT30  
// Define sensor-specific commands/registers here  
#elif HUMIDITY_SENSOR_TYPE_DHT  
#define DHT_DATA_PIN                    21  
#define DHT_TYPE                        DHT_TYPE_22 // or DHT_TYPE_11  
#endif

#define HUMIDITY_READ_RETRIES           3 // Number of times to retry a read on failure  
#define HUMIDITY_READ_DELAY_MS          2000 // Delay between DHT reads, if applicable, or retries
```

### **5.7. Resource Usage**

* **Flash**: Low to moderate, depending on the complexity of the sensor protocol (e.g., bit-banging for DHT requires more code).  
* **RAM**: Very low, for internal state and temporary read buffers.  
* **CPU**: Low to moderate, depending on sensor type. Analog reads are fast. Digital protocols like DHT require precise timing and more CPU cycles.

## **6. Test Considerations**

### **6.1. Unit Testing**

* **Mock Dependencies**: Unit tests for humidity will mock MCAL_ADC_Read(), MCAL_I2C_MasterWriteRead(), MCAL_GPIO_Init(), MCAL_GPIO_SetState(), MCAL_GPIO_GetState(), SERVICE_OS_DelayUs(), and RTE_Service_SystemMonitor_ReportFault().  
* **Test Cases**:  
  * HUMIDITY_Init: Verify correct MCAL initialization calls. Test initialization failure and fault reporting.  
  * HUMIDITY_Read:  
    * Test successful reads: Mock raw data to simulate various humidity values and verify correct conversion to percentage.  
    * Test read failures: Mock MCAL calls to return errors or invalid data. Verify retry logic and FAULT_ID_HUMIDITY_SENSOR_READ_FAILED after all retries.  
    * Test CRC errors (for digital sensors like DHT).  
    * Test out-of-range raw data and verify FAULT_ID_HUMIDITY_SENSOR_OUT_OF_RANGE (if implemented).

### **6.2. Integration Testing**

* **Humidity-MCAL Integration**: Verify that humidity correctly interfaces with the actual MCAL drivers and the physical humidity sensor.  
* **Accuracy Verification**: Compare humidity's reported values with a calibrated external humidity meter.  
* **SystemMgr Integration**: Verify that systemMgr receives accurate humidity data from humidity via RTE.  
* **Fault Injection**: Disconnect the humidity sensor, or introduce noise on its communication lines, and verify that humidity reports FAULT_ID_HUMIDITY_SENSOR_READ_FAILED to SystemMonitor.

### **6.3. System Testing**

* **End-to-End Control Loop**: Verify that the system's overall control loop (sensor read -> systemMgr logic -> actuator control) functions correctly based on humidity readings.  
* **Environmental Chambers**: Test the system in controlled humidity environments (e.g., humidity chamber) to verify performance across the sensor's operating range.  
* **Long-Term Reliability**: Run the system for extended periods to ensure continuous and reliable humidity monitoring.
