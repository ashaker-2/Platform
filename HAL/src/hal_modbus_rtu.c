/**
 * @file hal_modbus_rtu.c
 * @brief Implementation for the Hardware Abstraction Layer (HAL) Modbus RTU component.
 *
 * This file implements the hardware-independent Modbus RTU interface, mapping logical
 * Modbus operations to the underlying MCAL UART driver. It handles frame assembly,
 * CRC calculation, transmission, reception, and error detection.
 */

#include "hal_modbus_rtu.h"     // Public header for HAL_ModbusRTU
#include "hal_modbus_rtu_cfg.h" // Configuration header for HAL_ModbusRTU
#include "mcal_uart.h"          // MCAL layer for direct UART hardware access
#include "app_common.h"         // Common application definitions
#include "system_monitor.h"     // For reporting faults
#include "logger.h"             // For logging
#include <string.h>             // For memcpy, memset

// --- Private Data Structures ---
/**
 * @brief Runtime state for the HAL Modbus RTU module.
 */
typedef struct {
    HAL_ModbusRTU_State_t           current_state;
    HAL_ModbusRTU_Config_t          config;
    bool                            is_initialized;
    uint8_t                         tx_buffer[HAL_MODBUS_RTU_TX_BUFFER_SIZE];
    uint8_t                         rx_buffer[HAL_MODBUS_RTU_RX_BUFFER_SIZE];
    uint16_t                        rx_buffer_idx;
    uint32_t                        last_rx_timestamp; // For inter-character timeout (T3.5)

    // Callbacks
    HAL_ModbusRTU_FrameReceivedCallback_t       frame_received_cb;
    HAL_ModbusRTU_TransmissionCompleteCallback_t transmission_complete_cb;
    HAL_ModbusRTU_ErrorCallback_t               error_cb;
} HAL_ModbusRTU_ModuleState_t;

// --- Private Variables ---
static HAL_ModbusRTU_ModuleState_t s_modbus_rtu_state;

// --- Private Function Prototypes ---
static void HAL_ModbusRTU_UpdateState(HAL_ModbusRTU_State_t new_state);
static uint16_t HAL_ModbusRTU_CalculateCRC16_Internal(const uint8_t *data_p, uint16_t length);

// MCAL UART callback handlers (to be registered with MCAL_UART)
static void MCAL_UART_RxCpltHandler(const uint8_t *data_p, uint16_t length);
static void MCAL_UART_TxCpltHandler(void);
static void MCAL_UART_ErrorHandler(uint32_t error_code);

// --- Public Function Implementations ---

/**
 * @brief Initializes the HAL Modbus RTU module.
 * This function should be called once during system startup.
 * It configures the underlying UART and prepares the Modbus RTU state machine.
 * @param config_p Pointer to the Modbus RTU configuration parameters.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_ModbusRTU_Init(const HAL_ModbusRTU_Config_t *config_p) {
    if (s_modbus_rtu_state.is_initialized) {
        LOG_WARNING("HAL_ModbusRTU", "Module already initialized.");
        return HAL_OK;
    }
    if (config_p == NULL) {
        LOG_ERROR("HAL_ModbusRTU", "Init failed: config_p is NULL.");
        return HAL_INVALID_PARAM;
    }

    // Initialize internal state
    memset(&s_modbus_rtu_state, 0, sizeof(HAL_ModbusRTU_ModuleState_t));
    memcpy(&s_modbus_rtu_state.config, config_p, sizeof(HAL_ModbusRTU_Config_t));
    HAL_ModbusRTU_UpdateState(HAL_MODBUS_RTU_STATE_IDLE);

    // Prepare MCAL UART configuration
    MCAL_UART_Config_t mcal_uart_config = {
        .baud_rate = config_p->baud_rate,
        .data_bits = config_p->data_bits,
        .stop_bits = config_p->stop_bits,
        .parity = config_p->parity,
        .flow_control = MCAL_UART_FLOW_NONE // Modbus RTU typically doesn't use hardware flow control
    };

    MCAL_UART_Callbacks_t mcal_uart_cbs = {
        .rx_cplt_cb = MCAL_UART_RxCpltHandler,
        .tx_cplt_cb = MCAL_UART_TxCpltHandler,
        .error_cb = MCAL_UART_ErrorHandler
    };

    // Initialize MCAL UART
    // Assuming a specific UART instance is used for Modbus RTU (e.g., MCAL_UART_INSTANCE_MODBUS)
    if (MCAL_UART_Init(MCAL_UART_INSTANCE_MODBUS, &mcal_uart_config, &mcal_uart_cbs) != APP_OK) {
        LOG_ERROR("HAL_ModbusRTU", "Failed to initialize MCAL UART for Modbus RTU.");
        SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_MODBUS_RTU_ERROR, 0);
        return HAL_ERROR;
    }

    // Start UART reception in interrupt mode
    if (MCAL_UART_Receive_IT(MCAL_UART_INSTANCE_MODBUS, s_modbus_rtu_state.rx_buffer, 1) != APP_OK) {
        LOG_ERROR("HAL_ModbusRTU", "Failed to start initial UART reception.");
        SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_MODBUS_RTU_ERROR, 1);
        return HAL_ERROR;
    }

    s_modbus_rtu_state.is_initialized = true;
    LOG_INFO("HAL_ModbusRTU", "Module initialized. Role: %s, Baud: %lu.",
             (config_p->role == HAL_MODBUS_RTU_ROLE_MASTER) ? "Master" : "Slave", config_p->baud_rate);
    return HAL_OK;
}

/**
 * @brief Registers callback functions for Modbus RTU events.
 * @param frame_received_cb Callback for received frames. Can be NULL.
 * @param transmission_complete_cb Callback for transmission completion. Can be NULL.
 * @param error_cb Callback for error events. Can be NULL.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_ModbusRTU_RegisterCallbacks(HAL_ModbusRTU_FrameReceivedCallback_t frame_received_cb,
                                             HAL_ModbusRTU_TransmissionCompleteCallback_t transmission_complete_cb,
                                             HAL_ModbusRTU_ErrorCallback_t error_cb) {
    if (!s_modbus_rtu_state.is_initialized) {
        LOG_ERROR("HAL_ModbusRTU", "Cannot register callbacks: Module not initialized.");
        return HAL_NOT_INITIALIZED;
    }

    s_modbus_rtu_state.frame_received_cb = frame_received_cb;
    s_modbus_rtu_state.transmission_complete_cb = transmission_complete_cb;
    s_modbus_rtu_state.error_cb = error_cb;

    LOG_DEBUG("HAL_ModbusRTU", "Callbacks registered.");
    return HAL_OK;
}

/**
 * @brief Sends a Modbus RTU frame.
 * This function calculates the CRC and transmits the complete frame over UART.
 * @param frame_p Pointer to the data to send (excluding CRC).
 * @param length The length of the data to send (excluding CRC).
 * @return HAL_OK on success, HAL_ERROR on failure (e.g., busy, invalid parameters).
 */
HAL_Status_t HAL_ModbusRTU_SendFrame(const uint8_t *frame_p, uint16_t length) {
    if (!s_modbus_rtu_state.is_initialized) {
        LOG_ERROR("HAL_ModbusRTU", "SendFrame failed: Module not initialized.");
        return HAL_NOT_INITIALIZED;
    }
    if (s_modbus_rtu_state.current_state == HAL_MODBUS_RTU_STATE_TRANSMITTING) {
        LOG_WARNING("HAL_ModbusRTU", "SendFrame failed: Module is busy transmitting.");
        return HAL_BUSY;
    }
    if (frame_p == NULL || length == 0 || (length + 2) > HAL_MODBUS_RTU_TX_BUFFER_SIZE) { // +2 for CRC
        LOG_ERROR("HAL_ModbusRTU", "SendFrame failed: Invalid data_p, zero length, or buffer too small.");
        return HAL_INVALID_PARAM;
    }

    // Copy data to TX buffer
    memcpy(s_modbus_rtu_state.tx_buffer, frame_p, length);

    // Calculate and append CRC
    uint16_t crc = HAL_ModbusRTU_CalculateCRC16_Internal(frame_p, length);
    s_modbus_rtu_state.tx_buffer[length] = (uint8_t)(crc & 0xFF);         // Low byte
    s_modbus_rtu_state.tx_buffer[length + 1] = (uint8_t)((crc >> 8) & 0xFF); // High byte

    HAL_ModbusRTU_UpdateState(HAL_MODBUS_RTU_STATE_TRANSMITTING);

    // Transmit the frame via MCAL UART
    if (MCAL_UART_Transmit_IT(MCAL_UART_INSTANCE_MODBUS, s_modbus_rtu_state.tx_buffer, length + 2) != APP_OK) {
        LOG_ERROR("HAL_ModbusRTU", "MCAL UART transmit failed.");
        SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_MODBUS_RTU_ERROR, 2);
        HAL_ModbusRTU_UpdateState(HAL_MODBUS_RTU_STATE_ERROR);
        if (s_modbus_rtu_state.error_cb != NULL) {
            s_modbus_rtu_state.error_cb(MODBUS_ERROR_TRANSMIT_FAILED); // Define MODBUS_ERROR_TRANSMIT_FAILED
        }
        return HAL_ERROR;
    }

    LOG_VERBOSE("HAL_ModbusRTU", "Transmitting %u bytes.", length + 2);
    return HAL_OK;
}

/**
 * @brief Gets the current Modbus RTU communication state.
 * @return The current HAL_ModbusRTU_State_t.
 */
HAL_ModbusRTU_State_t HAL_ModbusRTU_GetState(void) {
    return s_modbus_rtu_state.current_state;
}

/**
 * @brief Calculates the Modbus RTU CRC16 for a given data buffer.
 * This utility function can be used by higher layers if they need to pre-calculate CRC.
 * @param data_p Pointer to the data buffer.
 * @param length The length of the data buffer.
 * @return The calculated 16-bit CRC value.
 */
uint16_t HAL_ModbusRTU_CalculateCRC16(const uint8_t *data_p, uint16_t length) {
    return HAL_ModbusRTU_CalculateCRC16_Internal(data_p, length);
}

// --- Private Function Implementations ---

/**
 * @brief Updates the internal Modbus RTU state.
 * @param new_state The new Modbus RTU state to set.
 */
static void HAL_ModbusRTU_UpdateState(HAL_ModbusRTU_State_t new_state) {
    if (s_modbus_rtu_state.current_state != new_state) {
        LOG_DEBUG("HAL_ModbusRTU", "State changed from %d to %d.", s_modbus_rtu_state.current_state, new_state);
        s_modbus_rtu_state.current_state = new_state;
    }
}

/**
 * @brief Internal function to calculate the Modbus RTU CRC16.
 * @param data_p Pointer to the data buffer.
 * @param length The length of the data buffer.
 * @return The calculated 16-bit CRC value.
 */
static uint16_t HAL_ModbusRTU_CalculateCRC16_Internal(const uint8_t *data_p, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data_p[i];
        for (uint8_t j = 0; j < 8; j++) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// --- MCAL UART Callback Handlers (Internal to HAL) ---

/**
 * @brief Handler for received data reported by the MCAL UART driver.
 * This function is called for each received byte (or small chunk) and
 * builds up the Modbus frame, checking for inter-character timeout (T3.5)
 * and frame completion.
 * @param data_p Pointer to the received data.
 * @param length Length of the received data (usually 1 byte for single byte reception).
 */
static void MCAL_UART_RxCpltHandler(const uint8_t *data_p, uint16_t length) {
    // In a real system, this handler would need to manage a timer for T3.5
    // and assemble the frame byte by byte. For simplicity, this mock assumes
    // MCAL UART can provide a full frame or handles timeouts internally.
    // A robust implementation would involve:
    // 1. Resetting a T3.5 timer on each byte reception.
    // 2. If T3.5 expires, process the received buffer as a complete frame.
    // 3. If buffer overflows, report error.

    if (!s_modbus_rtu_state.is_initialized) {
        LOG_ERROR("HAL_ModbusRTU", "RxCpltHandler: Module not initialized.");
        return;
    }

    // Append received byte(s) to rx_buffer
    if ((s_modbus_rtu_state.rx_buffer_idx + length) <= HAL_MODBUS_RTU_RX_BUFFER_SIZE) {
        memcpy(&s_modbus_rtu_state.rx_buffer[s_modbus_rtu_state.rx_buffer_idx], data_p, length);
        s_modbus_rtu_state.rx_buffer_idx += length;
        s_modbus_rtu_state.last_rx_timestamp = APP_GetTickCount(); // Assuming a system tick function

        HAL_ModbusRTU_UpdateState(HAL_MODBUS_RTU_STATE_RECEIVING);

        // For a simple mock, let's assume a full frame is received after a certain number of bytes
        // or after some delay (which would be handled by a timer in real application).
        // Here, we'll just check if we received enough bytes for a minimal frame (address + func + CRC)
        // or a full buffer.
        if (s_modbus_rtu_state.rx_buffer_idx >= 4) { // Minimum frame: Address (1) + Function (1) + CRC (2)
            // Check CRC
            uint16_t received_crc = (uint16_t)s_modbus_rtu_state.rx_buffer[s_modbus_rtu_state.rx_buffer_idx - 1] << 8 |
                                    s_modbus_rtu_state.rx_buffer[s_modbus_rtu_state.rx_buffer_idx - 2];
            uint16_t calculated_crc = HAL_ModbusRTU_CalculateCRC16_Internal(s_modbus_rtu_state.rx_buffer, s_modbus_rtu_state.rx_buffer_idx - 2);

            if (received_crc == calculated_crc) {
                LOG_VERBOSE("HAL_ModbusRTU", "Frame received, CRC OK. Length: %u.", s_modbus_rtu_state.rx_buffer_idx);
                if (s_modbus_rtu_state.frame_received_cb != NULL) {
                    // Pass the frame without CRC
                    s_modbus_rtu_state.frame_received_cb(s_modbus_rtu_state.rx_buffer, s_modbus_rtu_state.rx_buffer_idx - 2);
                }
                s_modbus_rtu_state.rx_buffer_idx = 0; // Reset for next frame
                HAL_ModbusRTU_UpdateState(HAL_MODBUS_RTU_STATE_IDLE);
            } else {
                LOG_ERROR("HAL_ModbusRTU", "CRC Mismatch! Received: 0x%04X, Calculated: 0x%04X.", received_crc, calculated_crc);
                SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_MODBUS_RTU_ERROR, 3);
                if (s_modbus_rtu_state.error_cb != NULL) {
                    s_modbus_rtu_state.error_cb(MODBUS_ERROR_CRC_MISMATCH); // Define MODBUS_ERROR_CRC_MISMATCH
                }
                s_modbus_rtu_state.rx_buffer_idx = 0; // Discard frame
                HAL_ModbusRTU_UpdateState(HAL_MODBUS_RTU_STATE_IDLE); // Return to idle
            }
        }
    } else {
        LOG_ERROR("HAL_ModbusRTU", "Rx buffer overflow.");
        SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_MODBUS_RTU_ERROR, 4);
        if (s_modbus_rtu_state.error_cb != NULL) {
            s_modbus_rtu_state.error_cb(MODBUS_ERROR_RX_OVERFLOW); // Define MODBUS_ERROR_RX_OVERFLOW
        }
        s_modbus_rtu_state.rx_buffer_idx = 0; // Discard current buffer
        HAL_ModbusRTU_UpdateState(HAL_MODBUS_RTU_STATE_ERROR); // Enter error state
    }

    // Continue receiving the next byte
    MCAL_UART_Receive_IT(MCAL_UART_INSTANCE_MODBUS, &s_modbus_rtu_state.rx_buffer[s_modbus_rtu_state.rx_buffer_idx], 1);
}

/**
 * @brief Handler for transmission complete event reported by the MCAL UART driver.
 */
static void MCAL_UART_TxCpltHandler(void) {
    HAL_ModbusRTU_UpdateState(HAL_MODBUS_RTU_STATE_IDLE);
    if (s_modbus_rtu_state.transmission_complete_cb != NULL) {
        s_modbus_rtu_state.transmission_complete_cb();
    }
    LOG_VERBOSE("HAL_ModbusRTU", "Transmission complete.");
}

/**
 * @brief Handler for error events reported by the MCAL UART driver.
 * @param error_code The error code from MCAL UART.
 */
static void MCAL_UART_ErrorHandler(uint32_t error_code) {
    LOG_ERROR("HAL_ModbusRTU", "MCAL UART error: 0x%lx.", error_code);
    SYSTEM_MONITOR_ReportFault(FAULT_ID_HAL_MODBUS_RTU_ERROR, error_code);
    HAL_ModbusRTU_UpdateState(HAL_MODBUS_RTU_STATE_ERROR);
    if (s_modbus_rtu_state.error_cb != NULL) {
        s_modbus_rtu_state.error_cb(MODBUS_ERROR_UART_FAULT); // Define MODBUS_ERROR_UART_FAULT
    }
    s_modbus_rtu_state.rx_buffer_idx = 0; // Reset RX buffer on error
}
