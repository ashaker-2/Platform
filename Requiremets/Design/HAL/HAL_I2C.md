# **Detailed Design Document: HAL_I2C Component**

## **1\. Introduction**

### **1.1. Purpose**

This document details the design of the HAL\_I2C component, which provides a hardware abstraction layer for Inter-Integrated Circuit (I2C) communication peripherals. Its primary purpose is to offer a standardized, microcontroller-independent interface for managing I2C bus operations, abstracting the low-level register access and specific MCU details from higher layers.

### **1.2. Scope**

The scope of this document covers the HAL\_I2C module's architecture, functional behavior, interfaces, dependencies, and resource considerations. It details how the HAL layer interacts with the underlying Microcontroller Abstraction Layer (MCAL) for I2C operations.

### **1.3. References**

* Software Architecture Document (SAD) \- Smart Device Firmware (Final Version)  
* MCAL I2C Driver Specification (Conceptual, as it's the lower layer)  
* MCU Datasheet / Reference Manual (for specific I2C capabilities)  
* I2C Bus Specification

## **2\. Functional Description**

The HAL\_I2C component provides the following core functionalities:

1. I2C Initialization: Configure specific I2C master units, including clock speed (baud rate), GPIO pins for SDA/SCL, and optional pull-up resistors.  
2. Master Transmit: Send a block of bytes to a specific I2C slave device address.  
3. Master Receive: Receive a block of bytes from a specific I2C slave device address.  
4. Master Transmit-Receive (Combined): Perform a combined write-then-read operation (e.g., sending a register address then reading data from that address).  
5. Error Reporting: Report any failures during I2C operations (e.g., initialization failure, NACK, arbitration loss, bus timeout) to the SystemMonitor via RTE\_Service\_SystemMonitor\_ReportFault().

## **3\. Non-Functional Requirements**

### **3.1. Performance**

* Speed: Support standard and fast-mode I2C clock speeds (e.g., 100 kHz, 400 kHz).  
* Reliability: Ensure robust data transfer over the I2C bus, minimizing data corruption or loss.  
* Blocking/Non-Blocking (Optional): Support both blocking (synchronous) and potentially non-blocking (asynchronous, with callbacks) operations for flexibility. This design focuses on blocking for simplicity.

### **3.2. Memory**

* Minimal Footprint: The HAL\_I2C code and data shall have a minimal memory footprint.  
* Buffer Management: Efficiently manage transmit and receive buffers for I2C transactions.

### **3.3. Reliability**

* Robustness: The module shall handle bus errors (e.g., NACK, arbitration loss, bus hang) gracefully.  
* Fault Isolation: Failures in I2C communication should be isolated and reported without crashing the system.  
* Timeout Handling: Implement timeouts for I2C transactions to prevent indefinite waiting.

## **4\. Architectural Context**

As per the SAD (Section 3.1.2, HAL Layer), HAL\_I2C resides in the Hardware Abstraction Layer. It acts as an intermediary between higher layers (e.g., Application/display, Application/temperature for digital I2C sensors) and the MCAL\_I2C driver. HAL\_I2C translates generic I2C requests into MCAL-specific calls.

## **5\. Design Details**

### **5.1. Module Structure**

The HAL\_I2C component will consist of the following files:

* HAL/inc/hal\_i2c.h: Public header file containing function prototypes, data types, and error codes.  
* HAL/src/hal\_i2c.c: Source file containing the implementation of the HAL\_I2C functions.  
* HAL/cfg/hal\_i2c\_cfg.h: Configuration header for static I2C bus definitions and initial settings.

### **5.2. Public Interface (API)**
```c
// In HAL/inc/hal\_i2c.h

// Enum for I2C port IDs (logical identifiers for each hardware I2C instance)  
typedef enum {  
    HAL\_I2C\_PORT\_0,  
    HAL\_I2C\_PORT\_1,  
    // Add more I2C ports as needed based on hardware availability  
    HAL\_I2C\_PORT\_COUNT  
} HAL\_I2C\_PortId\_t;

// Structure for initial I2C bus configuration  
typedef struct {  
    HAL\_I2C\_PortId\_t port\_id;  
    uint32\_t clk\_speed\_hz;  // I2C clock speed in Hz (e.g., 100000 for 100kHz)  
    uint8\_t sda\_gpio\_pin;   // GPIO pin for SDA  
    uint8\_t scl\_gpio\_pin;   // GPIO pin for SCL  
    bool sda\_pullup\_enable; // Enable internal pull-up for SDA  
    bool scl\_pullup\_enable; // Enable internal pull-up for SCL  
    uint32\_t timeout\_ms;    // Default transaction timeout in milliseconds  
} HAL\_I2C\_Config\_t;

/\*\*  
 \* @brief Initializes all configured I2C peripherals based on the predefined array.  
 \* This function should be called once during system initialization.  
 \* @return APP\_OK on success, APP\_ERROR if any I2C port fails to initialize.  
 \*/  
APP\_Status\_t HAL\_I2C\_Init(void);

/\*\*  
 \* @brief Performs an I2C master write operation.  
 \* @param port\_id The ID of the I2C port to use.  
 \* @param slave\_address The 7-bit I2C slave address.  
 \* @param data\_out Pointer to the data buffer to transmit.  
 \* @param data\_out\_len Length of the data to transmit.  
 \* @return APP\_OK on success, APP\_ERROR on failure.  
 \*/  
APP\_Status\_t HAL\_I2C\_MasterWrite(HAL\_I2C\_PortId\_t port\_id, uint8\_t slave\_address,  
                                 const uint8\_t \*data\_out, uint16\_t data\_out\_len);

/\*\*  
 \* @brief Performs an I2C master read operation.  
 \* @param port\_id The ID of the I2C port to use.  
 \* @param slave\_address The 7-bit I2C slave address.  
 \* @param data\_in Pointer to the buffer to store received data.  
 \* @param data\_in\_len Length of the data to receive.  
 \* @return APP\_OK on success, APP\_ERROR on failure.  
 \*/  
APP\_Status\_t HAL\_I2C\_MasterRead(HAL\_I2C\_PortId\_t port\_id, uint8\_t slave\_address,  
                                uint8\_t \*data\_in, uint16\_t data\_in\_len);

/\*\*  
 \* @brief Performs a combined I2C master write-then-read operation.  
 \* This is typically used to write a register address and then read data from it.  
 \* @param port\_id The ID of the I2C port to use.  
 \* @param slave\_address The 7-bit I2C slave address.  
 \* @param data\_out Pointer to the data buffer to transmit (e.g., register address).  
 \* @param data\_out\_len Length of the data to transmit.  
 \* @param data\_in Pointer to the buffer to store received data.  
 \* @param data\_in\_len Length of the data to receive.  
 \* @return APP\_OK on success, APP\_ERROR on failure.  
 \*/  
APP\_Status\_t HAL\_I2C\_MasterWriteRead(HAL\_I2C\_PortId\_t port\_id, uint8\_t slave\_address,  
                                     const uint8\_t \*data\_out, uint16\_t data\_out\_len,  
                                     uint8\_t \*data\_in, uint16\_t data\_in\_len);
```
### **5.3. Internal Design**

The HAL\_I2C module will act as a wrapper around the MCAL\_I2C functions. It will perform input validation, parameter translation, and handle error reporting before delegating the actual hardware access to the MCAL layer.

1. **Initialization (HAL\_I2C\_Init)**:  
   * This function will **loop through the hal\_i2c\_initial\_config array** defined in HAL/cfg/hal\_i2c\_cfg.h.  
   * For each entry in the array:  
     * Validate the port\_id against HAL\_I2C\_PORT\_COUNT.  
     * Call MCAL\_I2C\_Init(mcal\_port\_id, mcal\_clk\_speed, mcal\_sda\_pin, mcal\_scl\_pin, mcal\_sda\_pullup, mcal\_scl\_pullup).  
     * If MCAL\_I2C\_Init returns an error for *any* port, report HAL\_I2C\_INIT\_FAILURE to SystemMonitor. The function should continue to attempt to initialize remaining ports but will ultimately return APP\_ERROR if any initialization fails.  
   * If all I2C ports are initialized successfully, return APP\_OK.  
2. **Master Write (HAL\_I2C\_MasterWrite)**:  
   * Validate port\_id, slave\_address, data\_out, and data\_out\_len.  
   * Call MCAL\_I2C\_MasterTransmit(mcal\_port\_id, mcal\_slave\_address, data\_out, data\_out\_len, config-\>timeout\_ms).  
   * If MCAL\_I2C\_MasterTransmit returns an error, translate the MCAL error code into a HAL\_I2C specific fault (e.g., HAL\_I2C\_NACK\_ERROR, HAL\_I2C\_BUS\_TIMEOUT, HAL\_I2C\_ARBITRATION\_LOSS) and report to SystemMonitor.  
3. **Master Read (HAL\_I2C\_MasterRead)**:  
   * Validate port\_id, slave\_address, data\_in, and data\_in\_len.  
   * Call MCAL\_I2C\_MasterReceive(mcal\_port\_id, mcal\_slave\_address, data\_in, data\_in\_len, config-\>timeout\_ms).  
   * If MCAL\_I2C\_MasterReceive returns an error, translate and report the fault to SystemMonitor.  
4. **Master Write-Read (HAL\_I2C\_MasterWriteRead)**:  
   * Validate all input parameters.  
   * This function will typically perform two sequential MCAL calls:  
     * MCAL\_I2C\_MasterTransmit(mcal\_port\_id, mcal\_slave\_address, data\_out, data\_out\_len, config-\>timeout\_ms, true /\* no\_stop\_bit \*/). The true for no\_stop\_bit is crucial to keep the bus active for the subsequent read.  
     * MCAL\_I2C\_MasterReceive(mcal\_port\_id, mcal\_slave\_address, data\_in, data\_in\_len, config-\>timeout\_ms).  
   * If either MCAL call returns an error, translate and report the fault to SystemMonitor. Ensure proper error handling if the first transmit fails before the receive.

**Sequence Diagram (Example: HAL\_I2C\_MasterWrite):**
```mermaid
sequenceDiagram  
    participant App as Application Layer (e.g., Display Module)  
    participant RTE as Runtime Environment  
    participant HAL\_I2C as HAL/I2C  
    participant MCAL\_I2C as MCAL/I2C  
    participant SystemMonitor as Application/SystemMonitor

    App-\>\>RTE: RTE\_Service\_I2C\_MasterWrite(I2C\_PORT\_DISPLAY, SLAVE\_LCD\_ADDR, cmd\_data, len)  
    RTE-\>\>HAL\_I2C: HAL\_I2C\_MasterWrite(HAL\_I2C\_PORT\_0, SLAVE\_LCD\_ADDR, cmd\_data, len)  
    HAL\_I2C-\>\>HAL\_I2C: Validate parameters  
    HAL\_I2C-\>\>MCAL\_I2C: MCAL\_I2C\_MasterTransmit(MCAL\_I2C\_PORT\_0, SLAVE\_LCD\_ADDR, cmd\_data, len, timeout)  
    alt MCAL\_I2C\_MasterTransmit returns MCAL\_ERROR  
        MCAL\_I2C--xHAL\_I2C: Return MCAL\_ERROR (e.g., NACK)  
        HAL\_I2C-\>\>SystemMonitor: RTE\_Service\_SystemMonitor\_ReportFault(HAL\_I2C\_NACK\_ERROR, SEVERITY\_LOW, ...)  
        HAL\_I2C--xRTE: Return APP\_ERROR  
        RTE--xApp: Return APP\_ERROR  
    else MCAL\_I2C\_MasterTransmit returns MCAL\_OK  
        MCAL\_I2C--\>\>HAL\_I2C: Return MCAL\_OK  
        HAL\_I2C--\>\>RTE: Return APP\_OK  
        RTE--\>\>App: Return APP\_OK  
    end
```
### **5.4. Dependencies**

* **Mcal/i2c/inc/mcal\_i2c.h**: For calling low-level I2C driver functions.  
* **Application/logger/inc/logger.h**: For internal logging.  
* **Rte/inc/Rte.h**: For calling RTE\_Service\_SystemMonitor\_ReportFault().  
* **Application/common/inc/app\_common.h**: For APP\_Status\_t and APP\_OK/APP\_ERROR.  
* **HAL/cfg/hal\_i2c\_cfg.h**: For the hal\_i2c\_initial\_config array and HAL\_I2C\_Config\_t structure.

### **5.5. Error Handling**

* **Input Validation**: All public API functions will validate input parameters (e.g., valid port\_id, non-NULL pointers, valid lengths, valid slave address).  
* **MCAL Error Propagation**: Errors returned by MCAL\_I2C functions will be caught by HAL\_I2C.  
* **Fault Reporting**: Upon detection of an error (invalid input, MCAL failure, I2C bus error), HAL\_I2C will report a specific fault ID (e.g., HAL\_I2C\_INIT\_FAILURE, HAL\_I2C\_NACK\_ERROR, HAL\_I2C\_ARBITRATION\_LOSS, HAL\_I2C\_BUS\_TIMEOUT, HAL\_I2C\_TRANSACTION\_FAILURE) to SystemMonitor via the RTE service.  
* **Return Status**: All public API functions will return APP\_ERROR on failure. HAL\_I2C\_Init will return APP\_ERROR if *any* port fails to initialize.

### **5.6. Configuration**

The HAL/cfg/hal\_i2c\_cfg.h file will contain:

* Macros or enums for logical I2C port IDs (e.g., I2C\_PORT\_DISPLAY\_LCD, I2C\_PORT\_TEMP\_HUM\_SENSOR).  
* The hal\_i2c\_initial\_config array, which defines the initial configuration for all I2C peripherals used by the system.  
* The size of the hal\_i2c\_initial\_config array.
  
```c
// Example: HAL/cfg/hal\_i2c\_cfg.h  
\#define I2C\_PORT\_DISPLAY\_LCD            HAL\_I2C\_PORT\_0  
\#define I2C\_PORT\_TEMP\_HUM\_SENSOR        HAL\_I2C\_PORT\_1

// Initial configuration array  
extern const HAL\_I2C\_Config\_t hal\_i2c\_initial\_config\[\];  
extern const uint32\_t hal\_i2c\_initial\_config\_size;
```

### **5.7. Resource Usage**

* **Flash**: Moderate, depending on the number of API functions and the size of the hal\_i2c\_initial\_config array.  
* **RAM**: Minimal for internal state and transaction buffers.  
* **CPU**: Low for individual transactions. Can increase with high data rates, frequent transactions, or complex error recovery.

## **6\. Test Considerations**

### **6.1. Unit Testing**

* **Mock MCAL\_I2C**: Unit tests for HAL\_I2C will mock the MCAL\_I2C functions to isolate HAL\_I2C's logic.  
* **Test Cases**:  
  * HAL\_I2C\_Init: Test with a valid hal\_i2c\_initial\_config array. Verify MCAL\_I2C\_Init calls for each entry. Test scenarios where MCAL calls fail (verify APP\_ERROR return and SystemMonitor fault reporting).  
  * HAL\_I2C\_MasterWrite: Test valid/invalid parameters. Mock MCAL\_I2C\_MasterTransmit to simulate success, NACK, timeout, arbitration loss. Verify correct return status and fault reporting.  
  * HAL\_I2C\_MasterRead: Similar to write, test valid/invalid parameters and mock MCAL\_I2C\_MasterReceive for various outcomes.  
  * HAL\_I2C\_MasterWriteRead: Test combined operations, ensuring correct sequential calls to MCAL\_I2C\_MasterTransmit (with no stop bit) and MCAL\_I2C\_MasterReceive. Test error handling if the write part fails before the read.  
  * Error reporting: Verify that RTE\_Service\_SystemMonitor\_ReportFault() is called with the correct fault ID on various error conditions.

### **6.2. Integration Testing**

* **HAL-MCAL Integration**: Verify that HAL\_I2C correctly interfaces with the actual MCAL\_I2C driver.  
* **Loopback Test (Conceptual)**: While true I2C loopback is complex, a simple test might involve connecting two I2C ports on the same MCU (if available) or using an external I2C slave emulator.  
* **External Device Communication**: Connect to actual I2C slave devices (e.g., LCD, temperature sensor, EEPROM) and verify successful communication (write/read data).  
* **Error Injection**: Introduce I2C bus errors (e.g., disconnecting SDA/SCL during a transaction, causing NACKs by addressing a non-existent slave) to verify that HAL\_I2C detects and reports these faults correctly to SystemMonitor.  
* **Throughput Test**: Test communication at different clock speeds with varying data lengths to assess performance and stability.

### **6.3. System Testing**

* **End-to-End Functionality**: Verify that all application features relying on I2C (e.g., display updates, digital sensor readings) work correctly within the integrated system.  
* **Long-Term Stability**: Run the system for extended periods with active I2C communication to detect any issues related to bus hangs, memory leaks, or unhandled errors.  
* **Power Modes**: Ensure I2C peripherals behave correctly during power mode transitions (e.g., low-power states, re-initialization on wake-up).
