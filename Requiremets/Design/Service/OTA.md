# **Detailed Design Document: OTA Component**

## **1. Introduction**

### **1.1. Purpose**

This document details the design of the OTA (Over-the-Air Update) component, which resides in the Service Layer. Its primary purpose is to manage the firmware update process, enabling the device to receive, verify, and apply new firmware images wirelessly (e.g., via Wi-Fi or Bluetooth). It abstracts the complexities of firmware image handling, Flash writing, and bootloader interaction from the Application/diagnostic module.

### **1.2. Scope**

The scope of this document covers the OTA module's architecture, functional behavior, interfaces, dependencies, and resource considerations. It details how OTA interacts with ComM for data reception, HAL_FLASH for memory writing, and the bootloader for image selection.

### **1.3. References**

* Software Architecture Document (SAD) - Smart Device Firmware (Final Version)  
* Detailed Design Document: HAL_FLASH  
* Detailed Design Document: ComM  
* MCU Bootloader Documentation (Conceptual, as it's typically vendor-specific)  
* Firmware Image Format Specification (Conceptual, e.g., signed, encrypted, versioned)

## **2. Functional Description**

The OTA component provides the following core functionalities:

1. **OTA Initialization**: Initialize the OTA service, including setting up internal state and preparing for update requests.  
2. **Update Initiation**: Receive a command to start an OTA update, typically from Application/diagnostic. This includes specifying the source (e.g., URL for HTTP download, or a pre-negotiated BLE characteristic).  
3. **Firmware Download**: Manage the download of the new firmware image, typically in chunks, via the ComM module.  
4. **Image Verification**: Verify the integrity (e.g., CRC, checksum) and authenticity (e.g., digital signature) of the downloaded firmware image.  
5. **Flash Writing**: Write the downloaded firmware chunks to the designated OTA update partition in Flash memory using HAL_FLASH.  
6. **Update Finalization**: After successful download and verification, mark the new firmware image as valid for booting.  
7. **Rollback Mechanism**: Support rolling back to the previous valid firmware image in case of update failure or issues with the new firmware.  
8. **Status Reporting**: Report the current OTA status (e.g., downloading, verifying, success, failure) to Application/diagnostic and SystemMonitor.  
9. **Error Reporting**: Report any failures during OTA operations (e.g., download failure, verification failure, Flash write error, insufficient power) to the SystemMonitor via RTE_SystemMonitor_ReportFault().

## **3. Non-Functional Requirements**

### **3.1. Performance**

* **Update Speed**: Complete firmware updates within a reasonable timeframe, considering image size and network bandwidth.  
* **Minimal Downtime**: Minimize the time the device is non-operational during the update process.

### **3.2. Memory**

* **Efficient Buffer Usage**: Optimize RAM usage for download buffers, especially for large firmware images.  
* **Flash Partitioning**: Assume a robust Flash partitioning scheme (e.g., two application banks + OTA data partition) to support safe updates.

### **3.3. Reliability**

* **Atomic Updates**: Ensure that updates are atomic; either the new firmware is fully installed and valid, or the old firmware remains fully functional.  
* **Fault Tolerance**: Handle network interruptions, power loss during download/write, and corrupted images gracefully.  
* **Security**: Ensure firmware images are authenticated and encrypted to prevent unauthorized or malicious updates (SRS-07-01-04).

## **4. Architectural Context**

As per the SAD (Section 3.1.2, Service Layer), OTA resides in the Service Layer. It is initiated by Application/diagnostic (via RTE services), which provides the high-level command. OTA then utilizes ComM for network communication and HAL_FLASH for writing to Flash memory. It interacts with the MCU's bootloader for image selection.

## **5. Design Details**

### **5.1. Module Structure**

The OTA component will consist of the following files:

* Service/ota/inc/service_ota.h: Public header file containing function prototypes, data types, and error codes.  
* Service/ota/src/service_ota.c: Source file containing the implementation of the OTA functions.  
* Service/ota/cfg/service_ota_cfg.h: Configuration header for OTA partition addresses, buffer sizes, and verification settings.

### **5.2. Public Interface (API)**

// In Service/ota/inc/service_ota.h
```c
// Enum for OTA status/error codes  
typedef enum {  
    OTA_OK = 0,  
    OTA_ERROR_INIT_FAILED,  
    OTA_ERROR_ALREADY_IN_PROGRESS,  
    OTA_ERROR_DOWNLOAD_FAILED,  
    OTA_ERROR_VERIFICATION_FAILED,  
    OTA_ERROR_FLASH_WRITE_FAILED,  
    OTA_ERROR_BOOT_SET_FAILED,  
    OTA_ERROR_INSUFFICIENT_POWER, // New error type  
    OTA_ERROR_INVALID_PARAM,  
    // Add more specific errors as needed  
} OTA_Status_t;

// Enum for OTA update state  
typedef enum {  
    OTA_STATE_IDLE,  
    OTA_STATE_INITIATING,  
    OTA_STATE_DOWNLOADING,  
    OTA_STATE_VERIFYING,  
    OTA_STATE_WRITING_FLASH,  
    OTA_STATE_FINALIZING,  
    OTA_STATE_SUCCESS,  
    OTA_STATE_FAILED,  
    OTA_STATE_ROLLBACK_INITIATED,  
} OTA_State_t;

// Function pointer for OTA status updates  
typedef void (*OTA_StatusCallback_t)(OTA_State_t state, OTA_Status_t error_code, uint32_t progress_percent);

/**  
 * @brief Initializes the OTA module.  
 * This function should be called once during system initialization.  
 * @param status_cb Callback function to report OTA status changes.  
 * @return OTA_OK on success, an error code on failure.  
 */  
OTA_Status_t OTA_Init(OTA_StatusCallback_t status_cb);

/**  
 * @brief Initiates an Over-the-Air firmware update.  
 * This function checks for sufficient power before starting the update.  
 * @param download_url URL for HTTP download, or other protocol-specific info.  
 * @return OTA_OK if update initiated successfully, an error code otherwise.  
 */  
OTA_Status_t OTA_StartUpdate(const char *download_url);

/**  
 * @brief Requests a rollback to the previously valid firmware image.  
 * This function should only be called if the current firmware is deemed invalid.  
 * @return OTA_OK on success, an error code on failure.  
 */  
OTA_Status_t OTA_RequestRollback(void);

/**  
 * @brief Gets the current OTA update state.  
 * @return The current OTA update state.  
 */  
OTA_State_t OTA_GetCurrentState(void);
```
### **5.3. Internal Design**

The OTA module will implement a state machine to manage the update process. It will interact with ComM for network operations and HAL_FLASH for memory access.

1. **Initialization (OTA_Init)**:  
   * Validate and store the status_cb callback.  
   * Initialize internal state variables (e.g., current_ota_state = OTA_STATE_IDLE).  
   * Perform any necessary setup related to the bootloader interface (e.g., query current boot partition).  
   * Return OTA_OK.  
2. **Start Update (OTA_StartUpdate)**:  
   * Validate download_url and check current_ota_state (must be IDLE).  
   * **Power Check**: Call RTE_SYS_MGR_CheckPowerReadinessForOTA() (a new RTE service to systemMgr). If systemMgr reports insufficient power, report OTA_ERROR_INSUFFICIENT_POWER to SystemMonitor and return error.  
   * Set current_ota_state = OTA_STATE_INITIATING.  
   * Notify status_cb.  
   * Spawn a dedicated internal OTA task or initiate an asynchronous download process (e.g., via ComM's HTTP client). This task/process will handle the actual download, verification, and flashing.  
3. **Internal OTA Task/Process (Conceptual)**:  
   * **State: OTA_STATE_DOWNLOADING**:  
     * Use ComM (e.g., COMM_Send(COMM_PROTOCOL_WIFI, HTTP_GET_REQUEST, ...) for HTTP download, or COMM_Receive for chunked BLE transfer).  
     * Receive firmware chunks into a temporary RAM buffer.  
     * Update progress and notify status_cb.  
     * Calculate CRC/checksum of received chunks.  
     * If download fails, transition to OTA_STATE_FAILED, report OTA_ERROR_DOWNLOAD_FAILED.  
   * **State: OTA_STATE_VERIFYING**:  
     * After download, verify the complete image (e.g., compare full image CRC, check digital signature).  
     * If verification fails, transition to OTA_STATE_FAILED, report OTA_ERROR_VERIFICATION_FAILED.  
   * **State: OTA_STATE_WRITING_FLASH**:  
     * Identify the target OTA partition in Flash (e.g., the inactive application bank).  
     * Call HAL_FLASH_EraseSector() for all sectors in the target partition. If fails, report OTA_ERROR_FLASH_WRITE_FAILED.  
     * Call HAL_FLASH_Write() to write firmware chunks from RAM buffer to Flash. If fails, report OTA_ERROR_FLASH_WRITE_FAILED.  
     * Update progress and notify status_cb.  
   * **State: OTA_STATE_FINALIZING**:  
     * Call bootloader-specific API to mark the newly written partition as bootable (e.g., esp_ota_set_boot_partition()).  
     * If fails, report OTA_ERROR_BOOT_SET_FAILED.  
     * Transition to OTA_STATE_SUCCESS or OTA_STATE_FAILED.  
     * Notify status_cb.  
     * Request system reboot via RTE_DIAGNOSTIC_RequestReboot().  
4. **Request Rollback (OTA_RequestRollback)**:  
   * Validate current state (e.g., not during an active update).  
   * Call bootloader-specific API to mark the previous valid partition as bootable (e.g., esp_ota_mark_app_valid_cancel_rollback()).  
   * Set current_ota_state = OTA_STATE_ROLLBACK_INITIATED.  
   * Notify status_cb.  
   * Request system reboot via RTE_DIAGNOSTIC_RequestReboot().  
5. **Get Current State (OTA_GetCurrentState)**:  
   * Simply returns current_ota_state.

**Sequence Diagram (Example: OTA Update Process):**

sequenceDiagram  
    participant Diagnostic as Application/Diagnostic  
    participant RTE as Runtime Environment  
    participant SystemMgr as Application/systemMgr  
    participant OTA as Service/OTA  
    participant ComM as Service/ComM  
    participant HAL_FLASH as HAL/FLASH  
    participant SystemMonitor as Application/SystemMonitor  
    participant Bootloader as MCU Bootloader

    Diagnostic->>RTE: RTE_OTA_StartUpdate(url)  
    RTE->>OTA: OTA_StartUpdate(url)  
    OTA->>RTE: RTE_SYS_MGR_CheckPowerReadinessForOTA()  
    RTE->>SystemMgr: SYS_MGR_CheckPowerReadinessForOTA()  
    SystemMgr-->>RTE: Return APP_OK (sufficient power)  
    RTE-->>OTA: Return APP_OK  
    OTA->>OTA: Set state = INITIATING  
    OTA->>OTA: Call status_cb(INITIATING, OK, 0)  
    OTA->>OTA: Start internal OTA task/process

    Note over OTA: Internal OTA Task/Process  
    loop Download Chunks  
        OTA->>ComM: COMM_Send(WIFI, HTTP_GET_CHUNK_REQ, ...)  
        ComM-->>OTA: Return OK  
        OTA->>ComM: COMM_Receive(WIFI, buffer, ...)  
        ComM-->>OTA: Return OK (chunk data)  
        OTA->>OTA: Update progress, calculate CRC  
        OTA->>OTA: Call status_cb(DOWNLOADING, OK, progress)  
    end  
    OTA->>OTA: Set state = VERIFYING  
    OTA->>OTA: Call status_cb(VERIFYING, OK, 100)  
    OTA->>OTA: Verify full image (CRC, signature)  
    alt Verification Fails  
        OTA->>OTA: Set state = FAILED  
        OTA->>SystemMonitor: RTE_SystemMonitor_ReportFault(OTA_VERIFICATION_FAILED, CRITICAL, ...)  
        OTA->>OTA: Call status_cb(FAILED, VERIFICATION_FAILED, 100)  
        OTA--xRTE: (Task exits or reports failure)  
    else Verification Success  
        OTA->>OTA: Set state = WRITING_FLASH  
        OTA->>OTA: Call status_cb(WRITING_FLASH, OK, 0)  
        loop Write Flash Sectors  
            OTA->>HAL_FLASH: HAL_FLASH_EraseSector(addr)  
            HAL_FLASH-->>OTA: Return OK  
            OTA->>HAL_FLASH: HAL_FLASH_Write(addr, data, len)  
            HAL_FLASH-->>OTA: Return OK  
            OTA->>OTA: Update progress, call status_cb  
        end  
        OTA->>OTA: Set state = FINALIZING  
        OTA->>OTA: Call status_cb(FINALIZING, OK, 100)  
        OTA->>Bootloader: esp_ota_set_boot_partition(new_partition)  
        Bootloader-->>OTA: Return OK  
        OTA->>OTA: Set state = SUCCESS  
        OTA->>OTA: Call status_cb(SUCCESS, OK, 100)  
        OTA->>RTE: RTE_DIAGNOSTIC_RequestReboot()  
    end

### **5.4. Dependencies**

* **Service/ComM/inc/service_comm.h**: For network communication (downloading firmware).  
* **HAL/inc/hal_flash.h**: For writing to Flash memory.  
* **Application/logger/inc/logger.h**: For internal logging.  
* **Rte/inc/Rte.h**: For calling RTE_SystemMonitor_ReportFault(), RTE_SYS_MGR_CheckPowerReadinessForOTA(), and RTE_DIAGNOSTIC_RequestReboot().  
* **Application/common/inc/app_common.h**: For APP_Status_t.  
* **Service/ota/cfg/service_ota_cfg.h**: For configuration.  
* **MCU-specific Bootloader/OTA APIs**: (e.g., esp_ota_ops.h for ESP-IDF) - these are implicitly called by OTA and are considered part of the underlying platform.

### **5.5. Error Handling**

* **Input Validation**: All public API functions will validate input parameters.  
* **Dependency Error Propagation**: Errors returned by ComM or HAL_FLASH will be caught by OTA.  
* **Power Check**: OTA explicitly checks for sufficient power via systemMgr before initiating an update.  
* **Fault Reporting**: Upon detection of an error (e.g., download failure, verification failure, Flash write error, bootloader error, insufficient power), OTA will report a specific fault ID (e.g., OTA_ERROR_DOWNLOAD_FAILED, OTA_ERROR_VERIFICATION_FAILED, OTA_ERROR_FLASH_WRITE_FAILED, OTA_ERROR_BOOT_SET_FAILED, OTA_ERROR_INSUFFICIENT_POWER) to SystemMonitor via the RTE service.  
* **Status Callbacks**: The status_cb is crucial for higher layers to monitor the update process and react to failures.  
* **Return Status**: All public API functions will return OTA_Status_t indicating success or specific error.

### **5.6. Configuration**

The Service/ota/cfg/service_ota_cfg.h file will contain:

* OTA update partition address and size (must align with Flash memory map).  
* Download buffer size.  
* Firmware image verification settings (e.g., enable/disable CRC, signature check).  
* Default rollback behavior (e.g., auto-rollback on first boot failure).

// Example: Service/ota/cfg/service_ota_cfg.h  
#define OTA_UPDATE_PARTITION_ADDR       HAL_FLASH_APP_BANK_B_ADDR // Example: Use second app bank  
#define OTA_DOWNLOAD_BUFFER_SIZE_BYTES  4096 // 4KB buffer for chunks  
#define OTA_ENABLE_SIGNATURE_VERIFICATION 1 // Enable cryptographic signature check  
#define OTA_ENABLE_CRC_CHECK              1 // Enable CRC check

### **5.7. Resource Usage**

* **Flash**: Minimal for the driver code itself. Requires a dedicated partition for the new firmware image (typically a size equal to the application bank).  
* **RAM**: Moderate, primarily for the download buffer (OTA_DOWNLOAD_BUFFER_SIZE_BYTES).  
* **CPU**: Moderate to high during download (network processing) and Flash writing (blocking operations).

## **6. Test Considerations**

### **6.1. Unit Testing**

* **Mock Dependencies**: Unit tests for OTA will mock ComM, HAL_FLASH, and the bootloader APIs to isolate OTA's logic.  
* **Test Cases**:  
  * OTA_Init: Verify callback registration and initial state.  
  * OTA_StartUpdate:  
    * Test valid/invalid URLs.  
    * Mock RTE_SYS_MGR_CheckPowerReadinessForOTA() to return sufficient/insufficient power and verify behavior.  
    * Verify state transitions (IDLE -> INITIATING).  
    * Mock ComM to simulate download success/failure (timeout, corrupted data).  
    * Mock HAL_FLASH to simulate erase/write success/failure.  
    * Mock bootloader APIs for setting boot partition.  
    * Verify status_cb is called with correct states and progress.  
    * Verify SystemMonitor fault reporting for all failure scenarios.  
  * OTA_RequestRollback: Test valid/invalid states. Verify bootloader rollback API call and SystemMonitor reporting.  
  * OTA_GetCurrentState: Verify it returns the correct internal state.

### **6.2. Integration Testing**

* **OTA-ComM-Flash Integration**: Verify that OTA correctly interfaces with ComM for network download and HAL_FLASH for memory writing.  
* **Full OTA Cycle**: Perform a complete OTA update using a test server providing a valid firmware image. Verify the device successfully updates and boots into the new firmware.  
* **Failure Scenarios**:  
  * **Network Interruption**: Disconnect Wi-Fi/BLE during download. Verify OTA detects failure and reports.  
  * **Corrupted Image**: Provide a firmware image with incorrect CRC/signature. Verify verification failure.  
  * **Flash Write Error**: Simulate a Flash write error (if possible in test setup).  
  * **Power Loss**: Simulate power loss during different phases of the update (download, write). Verify system recovers to a valid state (either old firmware or new if write completed).  
* **Rollback Test**: Intentionally install a "bad" firmware that causes a boot failure, then verify the system automatically rolls back to the previous good firmware.

### **6.3. System Testing**

* **End-to-End OTA**: Perform OTA updates in a real-world environment, including various network conditions.  
* **Long-Term Stability**: Perform multiple consecutive OTA updates to test the robustness of the system and Flash endurance.  
* **Security Validation**: Verify that only signed/encrypted firmware images are accepted, and unauthorized updates are rejected.  
* **Resource Impact**: Monitor CPU, RAM, and Flash usage during OTA to ensure it doesn't negatively impact other critical system functions.
