# **Detailed Design Document: HAL_EEPROM Component**

## **1. Introduction**

### **1.1. Purpose**

This document details the design of the HAL_EEPROM component, which provides a hardware abstraction layer for accessing external or emulated EEPROM (Electrically Erasable Programmable Read-Only Memory). Its primary purpose is to offer a standardized, microcontroller-independent interface for byte-level or block-level read/write operations to non-volatile memory, suitable for storing small, frequently updated configuration parameters or calibration data.

### **1.2. Scope**

The scope of this document covers the HAL_EEPROM module's architecture, functional behavior, interfaces, dependencies, and resource considerations. It details how the HAL layer interacts with the underlying Microcontroller Abstraction Layer (MCAL) for EEPROM operations (which might involve I2C, SPI, or internal Flash emulation).

### **1.3. References**

* Software Architecture Document (SAD) - Smart Device Firmware (Final Version)  
* MCAL EEPROM Driver Specification (Conceptual, as it's the lower layer, or MCAL I2C/SPI if external EEPROM)  
* MCU Datasheet / Reference Manual (for internal EEPROM emulation or peripheral details for external EEPROM)

## **2. Functional Description**

The HAL_EEPROM component provides the following core functionalities:

1. **EEPROM Initialization**: Initialize the EEPROM interface and underlying communication peripheral (if external).  
2. **Byte Write**: Write a single byte to a specific EEPROM address.  
3. **Block Write**: Write a block of bytes to a specific EEPROM address.  
4. **Byte Read**: Read a single byte from a specific EEPROM address.  
5. **Block Read**: Read a block of bytes from a specific EEPROM address.  
6. **Error Reporting**: Report any failures during EEPROM operations (e.g., initialization failure, write failure, read error, invalid address/length, communication error) to the SystemMonitor via RTE_Service_SystemMonitor_ReportFault().

## **3. Non-Functional Requirements**

### **3.1. Performance**

* **Write/Read Speed**: EEPROM operations shall be performed within acceptable timeframes, suitable for configuration updates.  
* **Endurance**: The EEPROM interface shall support the specified number of erase/write cycles (typically much higher than Flash for byte-level writes).

### **3.2. Memory**

* **Minimal Footprint**: The HAL_EEPROM code and data shall have a minimal memory footprint.  
* **Buffer Management**: Efficiently manage internal buffers for write/read operations (if necessary for page writes).

### **3.3. Reliability**

* **Data Integrity**: Ensure data written to EEPROM is reliably stored and retrieved without corruption.  
* **Robustness**: The module shall handle invalid memory access attempts or hardware failures gracefully.  
* **Wear Leveling (if emulated)**: If EEPROM is emulated on Flash, the underlying MCAL/driver should ideally implement wear-leveling to extend Flash lifetime.

## **4. Architectural Context**

As per the SAD (Section 3.1.2, HAL Layer), HAL_EEPROM resides in the Hardware Abstraction Layer. It acts as an intermediary between higher layers like Application/storage (for persistent configuration data) and the MCAL_EEPROM driver (or MCAL_I2C/MCAL_SPI if an external EEPROM chip is used). HAL_EEPROM translates generic EEPROM access requests into MCAL-specific calls.

## **5. Design Details**

### **5.1. Module Structure**

The HAL_EEPROM component will consist of the following files:

* HAL/inc/hal_eeprom.h: Public header file containing function prototypes, data types, and error codes.  
* HAL/src/hal_eeprom.c: Source file containing the implementation of the HAL_EEPROM functions.  
* HAL/cfg/hal_eeprom_cfg.h: Configuration header for EEPROM size, page size (if applicable), and communication interface details (e.g., I2C address).

### **5.2. Public Interface (API)**

```c
// In HAL/inc/hal_eeprom.h

// Enum for EEPROM status/error codes  
typedef enum {  
    HAL_EEPROM_OK = 0,  
    HAL_EEPROM_ERROR_INIT_FAILED,  
    HAL_EEPROM_ERROR_WRITE_FAILED,  
    HAL_EEPROM_ERROR_READ_FAILED,  
    HAL_EEPROM_ERROR_INVALID_ADDRESS,  
    HAL_EEPROM_ERROR_INVALID_LENGTH,  
    HAL_EEPROM_ERROR_COMM_FAILED, // For external EEPROM (I2C/SPI)  
    // Add more specific errors as needed  
} HAL_EEPROM_Status_t;

/**  
 * @brief Initializes the HAL_EEPROM module and the underlying EEPROM interface.  
 * This function should be called once during system initialization.  
 * @return HAL_EEPROM_OK on success, an error code on failure.  
 */  
HAL_EEPROM_Status_t HAL_EEPROM_Init(void);

/**  
 * @brief Writes a single byte to a specific EEPROM address.  
 * @param address The target EEPROM address.  
 * @param data The byte to write.  
 * @return HAL_EEPROM_OK on success, an error code on failure.  
 */  
HAL_EEPROM_Status_t HAL_EEPROM_WriteByte(uint32_t address, uint8_t data);

/**  
 * @brief Writes a block of data to EEPROM memory.  
 * @param address The start address to write to.  
 * @param data_out Pointer to the data buffer to write.  
 * @param data_len Length of the data to write.  
 * @return HAL_EEPROM_OK on success, an error code on failure.  
 */  
HAL_EEPROM_Status_t HAL_EEPROM_WriteBlock(uint32_t address, const uint8_t *data_out, uint32_t data_len);

/**  
 * @brief Reads a single byte from a specific EEPROM address.  
 * @param address The target EEPROM address.  
 * @param data Pointer to store the read byte.  
 * @return HAL_EEPROM_OK on success, an error code on failure.  
 */  
HAL_EEPROM_Status_t HAL_EEPROM_ReadByte(uint32_t address, uint8_t *data);

/**  
 * @brief Reads a block of data from EEPROM memory.  
 * @param address The start address to read from.  
 * @param data_in Pointer to the buffer to store read data.  
 * @param data_len Length of the data to read.  
 * @return HAL_EEPROM_OK on success, an error code on failure.  
 */  
HAL_EEPROM_Status_t HAL_EEPROM_ReadBlock(uint32_t address, uint8_t *data_in, uint32_t data_len);

/**  
 * @brief Gets the total size of the EEPROM memory.  
 * @return The total size of EEPROM memory in bytes.  
 */  
uint32_t HAL_EEPROM_GetTotalSize(void);
```

### **5.3. Internal Design**

The HAL_EEPROM module will act as a wrapper around the MCAL_EEPROM functions (or HAL_I2C/HAL_SPI if using an external chip). It will perform input validation (address bounds checking), handle page writes if necessary, and manage error reporting.

1. **Initialization (HAL_EEPROM_Init)**:  
   * If using MCAL_EEPROM: Call MCAL_EEPROM_Init().  
   * If using external EEPROM (e.g., via I2C): Call HAL_I2C_Init() for the relevant port and potentially perform a dummy read/write to verify EEPROM presence.  
   * If any underlying initialization fails, report HAL_EEPROM_ERROR_INIT_FAILED to SystemMonitor.  
   * Return HAL_EEPROM_OK on success.  
2. **Write Byte (HAL_EEPROM_WriteByte)**:  
   * Validate address for bounds.  
   * If using MCAL_EEPROM: Call MCAL_EEPROM_WriteByte(mcal_address, data).  
   * If using external EEPROM (e.g., I2C): Formulate an I2C write command (e.g., slave address + EEPROM address + data byte), then call HAL_I2C_MasterWrite().  
   * If the underlying write fails, report HAL_EEPROM_ERROR_WRITE_FAILED or HAL_EEPROM_ERROR_COMM_FAILED to SystemMonitor.  
3. **Write Block (HAL_EEPROM_WriteBlock)**:  
   * Validate address, data_out, and data_len. Perform bounds checking.  
   * If using MCAL_EEPROM: Call MCAL_EEPROM_WriteBlock(mcal_address, data_out, data_len).  
   * If using external EEPROM (e.g., I2C): This might involve breaking the block into page-sized writes if the EEPROM has page write limitations. Each page write would involve forming an I2C command (slave address + EEPROM address + page data), then calling HAL_I2C_MasterWrite().  
   * If any underlying write fails, report HAL_EEPROM_ERROR_WRITE_FAILED or HAL_EEPROM_ERROR_COMM_FAILED to SystemMonitor.  
4. **Read Byte (HAL_EEPROM_ReadByte)**:  
   * Validate address for bounds.  
   * If using MCAL_EEPROM: Call MCAL_EEPROM_ReadByte(mcal_address, data).  
   * If using external EEPROM (e.g., I2C): Formulate an I2C write command (slave address + EEPROM address) followed by an I2C read command (slave address + read data), then call HAL_I2C_MasterWriteRead().  
   * If the underlying read fails, report HAL_EEPROM_ERROR_READ_FAILED or HAL_EEPROM_ERROR_COMM_FAILED to SystemMonitor.  
5. **Read Block (HAL_EEPROM_ReadBlock)**:  
   * Validate address, data_in, and data_len. Perform bounds checking.  
   * If using MCAL_EEPROM: Call MCAL_EEPROM_ReadBlock(mcal_address, data_in, data_len).  
   * If using external EEPROM (e.g., I2C): Formulate an I2C write command (slave address + EEPROM start address) followed by a read command for the block length, then call HAL_I2C_MasterWriteRead().  
   * If the underlying read fails, report HAL_EEPROM_ERROR_READ_FAILED or HAL_EEPROM_ERROR_COMM_FAILED to SystemMonitor.  
6. **Get Total Size (HAL_EEPROM_GetTotalSize)**:  
   * This function simply returns the value defined in hal_eeprom_cfg.h.

**Sequence Diagram (Example: HAL_EEPROM_WriteBlock to External I2C EEPROM):**

```mermaid
sequenceDiagram  
    participant AppLayer as Application Layer (e.g., Storage Module)  
    participant RTE as Runtime Environment  
    participant HAL_EEPROM as HAL/EEPROM  
    participant HAL_I2C as HAL/I2C  
    participant MCAL_I2C as MCAL/I2C  
    participant SystemMonitor as Application/SystemMonitor

    AppLayer->>RTE: RTE_Service_STORAGE_SaveSettings(settings_data, len)  
    RTE->>HAL_EEPROM: HAL_EEPROM_WriteBlock(SETTINGS_EEPROM_ADDR, settings_data, len)  
    HAL_EEPROM->>HAL_EEPROM: Validate address, data, length, bounds  
    loop for each page/chunk  
        HAL_EEPROM->>HAL_I2C: HAL_I2C_MasterWrite(EEPROM_I2C_PORT, EEPROM_SLAVE_ADDR, [EEPROM_ADDR_BYTE_H, EEPROM_ADDR_BYTE_L, PAGE_DATA], PAGE_LEN)  
        HAL_I2C->>MCAL_I2C: MCAL_I2C_MasterTransmit(...)  
        alt MCAL_I2C_MasterTransmit returns MCAL_ERROR  
            MCAL_I2C--xHAL_I2C: Return MCAL_ERROR  
            HAL_I2C--xHAL_EEPROM: Return E_NOK  
            HAL_EEPROM->>SystemMonitor: RTE_Service_SystemMonitor_ReportFault(HAL_EEPROM_ERROR_COMM_FAILED,  ...)  
            HAL_EEPROM--xRTE: Return HAL_EEPROM_ERROR_WRITE_FAILED  
            RTE--xAppLayer: Return E_NOK  
            break loop  
        else MCAL_I2C_MasterTransmit returns MCAL_OK  
            MCAL_I2C-->>HAL_I2C: Return MCAL_OK  
            HAL_I2C-->>HAL_EEPROM: Return E_OK  
            HAL_EEPROM->>HAL_EEPROM: Delay for write cycle (if needed)  
        end  
    end  
    HAL_EEPROM-->>RTE: Return HAL_EEPROM_OK  
    RTE-->>AppLayer: Return E_OK
```

### **5.4. Dependencies**

* **Mcal/eeprom/inc/mcal_eeprom.h**: If using an MCU's dedicated internal EEPROM driver.  
* **HAL/inc/hal_i2c.h** or **HAL/inc/hal_spi.h**: If using an external EEPROM chip requiring I2C or SPI communication.  
* **Application/logger/inc/logger.h**: For internal logging.  
* **Rte/inc/Rte.h**: For calling RTE_Service_SystemMonitor_ReportFault().  
* **Application/common/inc/common.h**: For APP_Status_t.  
* **HAL/cfg/hal_eeprom_cfg.h**: For EEPROM configuration.

### **5.5. Error Handling**

* **Input Validation**: All public API functions will validate input parameters (e.g., valid addresses, non-NULL pointers, lengths within bounds).  
* **MCAL/HAL Communication Error Propagation**: Errors returned by MCAL_EEPROM or underlying HAL_I2C/HAL_SPI functions will be caught by HAL_EEPROM.  
* **Fault Reporting**: Upon detection of an error (invalid input, underlying driver failure, communication error), HAL_EEPROM will report a specific fault ID (e.g., HAL_EEPROM_ERROR_INIT_FAILED, HAL_EEPROM_ERROR_WRITE_FAILED, HAL_EEPROM_ERROR_READ_FAILED, HAL_EEPROM_ERROR_INVALID_ADDRESS, HAL_EEPROM_ERROR_COMM_FAILED) to SystemMonitor via the RTE service.  
* **Return Status**: All public API functions will return HAL_EEPROM_Status_t indicating success or specific error.

### **5.6. Configuration**

The HAL/cfg/hal_eeprom_cfg.h file will contain:

```c
* Total EEPROM size.  
* EEPROM page size (if applicable for block writes).  
* For external EEPROM: I2C/SPI port ID, slave address/CS pin.

// Example: HAL/cfg/hal_eeprom_cfg.h  
#define HAL_EEPROM_TOTAL_SIZE_BYTES  1024 // Example: 1KB EEPROM  
#define HAL_EEPROM_PAGE_SIZE_BYTES   16   // Example: 16-byte page write

// For external I2C EEPROM  
#define HAL_EEPROM_I2C_PORT_ID       HAL_I2C_PORT_0  
#define HAL_EEPROM_I2C_SLAVE_ADDRESS 0x50 // Example 7-bit I2C address
```

### **5.7. Resource Usage**

* **Flash**: Minimal for the driver code itself.  
* **RAM**: Minimal for internal state. May require small buffers for page writes if the underlying MCAL/communication protocol requires it.  
* **CPU**: Low for read operations. Higher for write operations due to their blocking nature and internal EEPROM write cycle times.

## **6. Test Considerations**

### **6.1. Unit Testing**

* **Mock MCAL_EEPROM / HAL_I2C / HAL_SPI**: Unit tests for HAL_EEPROM will mock the underlying MCAL or HAL communication functions to isolate HAL_EEPROM's logic.  
* **Test Cases**:  
  * HAL_EEPROM_Init: Verify underlying driver initialization calls and error propagation.  
  * HAL_EEPROM_WriteByte/ReadByte: Test valid/invalid addresses. Verify correct underlying calls and data integrity.  
  * HAL_EEPROM_WriteBlock/ReadBlock: Test valid/invalid addresses, data, lengths (including boundary conditions, partial block writes, writes across page boundaries). Verify correct underlying calls and data integrity.  
  * HAL_EEPROM_GetTotalSize: Verify correct return value from configuration.  
  * Error reporting: Verify that RTE_Service_SystemMonitor_ReportFault() is called with the correct fault ID on various error conditions (e.g., HAL_EEPROM_ERROR_INVALID_ADDRESS, HAL_EEPROM_ERROR_WRITE_FAILED, HAL_EEPROM_ERROR_COMM_FAILED).

### **6.2. Integration Testing**

* **HAL-MCAL/HAL_COMM Integration**: Verify that HAL_EEPROM correctly interfaces with the actual MCAL_EEPROM driver or HAL_I2C/HAL_SPI and the physical EEPROM chip.  
* **Read/Write Cycles**: Perform sequences of write and read operations to verify data integrity.  
* **Boundary Conditions**: Test writing/reading at the very beginning, end, and across page boundaries of the EEPROM.  
* **Power Cycling**: Test data persistence across power cycles after write operations.  
* **Endurance Testing (Long-term)**: If possible, perform automated write cycles to stress test EEPROM endurance, though this is often done at a lower driver level or during hardware validation.  
* **Fault Injection**: Simulate underlying communication errors (e.g., by disconnecting I2C/SPI lines, causing NACKs) to verify HAL_EEPROM's error handling and reporting.

### **6.3. System Testing**

* **Configuration Persistence**: Verify that application configurations (e.g., calibration values, last known state) saved via Application/storage (which uses HAL_EEPROM) persist across reboots.  
* **System Robustness**: Observe system behavior during and after EEPROM operations to ensure no unexpected crashes or performance degradation.
