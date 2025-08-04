/**
 * @file hal_modbus_rtu.h
 * @brief Public header for the Hardware Abstraction Layer (HAL) Modbus RTU component.
 *
 * This component provides a hardware-independent interface for Modbus RTU
 * communication over a serial interface (e.g., UART). It abstracts the
 * underlying MCAL UART driver and provides a clean API for higher-level modules
 * to send and receive Modbus RTU frames, handling CRC calculation and frame parsing.
 * It supports both Master and Slave roles.
 */

#ifndef HAL_MODBUS_RTU_H
#define HAL_MODBUS_RTU_H

#include "app_common.h" // For APP_Status_t and HAL_Status_t
#include <stdint.h>   // For uint8_t, uint16_t
#include <stdbool.h>  // For bool

// --- Type Definitions ---

/**
 * @brief Enumeration for Modbus RTU roles.
 */
typedef enum {
    HAL_MODBUS_RTU_ROLE_MASTER = 0, /**< Device acts as a Modbus Master. */
    HAL_MODBUS_RTU_ROLE_SLAVE,      /**< Device acts as a Modbus Slave. */
    HAL_MODBUS_RTU_ROLE_COUNT
} HAL_ModbusRTU_Role_t;

/**
 * @brief Enumeration for Modbus RTU communication states.
 */
typedef enum {
    HAL_MODBUS_RTU_STATE_IDLE = 0,      /**< Modbus RTU is idle. */
    HAL_MODBUS_RTU_STATE_TRANSMITTING,  /**< Modbus RTU is currently transmitting a frame. */
    HAL_MODBUS_RTU_STATE_RECEIVING,     /**< Modbus RTU is currently receiving a frame. */
    HAL_MODBUS_RTU_STATE_ERROR          /**< An error state has occurred. */
} HAL_ModbusRTU_State_t;

/**
 * @brief Structure for Modbus RTU configuration parameters.
 */
typedef struct {
    uint32_t                baud_rate;      /**< Baud rate for the serial communication (e.g., 9600, 19200). */
    uint8_t                 data_bits;      /**< Number of data bits (e.g., 8). */
    uint8_t                 stop_bits;      /**< Number of stop bits (e.g., 1, 2). */
    char                    parity;         /**< Parity setting ('N' for None, 'E' for Even, 'O' for Odd). */
    HAL_ModbusRTU_Role_t    role;           /**< Role of the device (Master or Slave). */
    uint8_t                 slave_address;  /**< Slave address if in Slave role (1-247). Ignored for Master. */
    uint16_t                response_timeout_ms; /**< Master response timeout in milliseconds. */
} HAL_ModbusRTU_Config_t;

// --- Callback Function Pointers ---

/**
 * @brief Callback function type for a complete Modbus RTU frame reception.
 * This callback is invoked when a valid Modbus RTU frame (including CRC) is received.
 * @param frame_p Pointer to the received Modbus RTU frame data (excluding CRC).
 * @param length The length of the received frame data (excluding CRC).
 */
typedef void (*HAL_ModbusRTU_FrameReceivedCallback_t)(const uint8_t *frame_p, uint16_t length);

/**
 * @brief Callback function type for Modbus RTU transmission completion.
 * This callback is invoked when a Modbus RTU frame has been successfully transmitted.
 */
typedef void (*HAL_ModbusRTU_TransmissionCompleteCallback_t)(void);

/**
 * @brief Callback function type for Modbus RTU error events.
 * @param error_code An error code indicating the type of error (e.g., CRC error, timeout).
 */
typedef void (*HAL_ModbusRTU_ErrorCallback_t)(uint16_t error_code);

// --- Public Functions ---

/**
 * @brief Initializes the HAL Modbus RTU module.
 * This function should be called once during system startup.
 * It configures the underlying UART and prepares the Modbus RTU state machine.
 * @param config_p Pointer to the Modbus RTU configuration parameters.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_ModbusRTU_Init(const HAL_ModbusRTU_Config_t *config_p);

/**
 * @brief Registers callback functions for Modbus RTU events.
 * @param frame_received_cb Callback for received frames. Can be NULL.
 * @param transmission_complete_cb Callback for transmission completion. Can be NULL.
 * @param error_cb Callback for error events. Can be NULL.
 * @return HAL_OK on success, HAL_ERROR on failure.
 */
HAL_Status_t HAL_ModbusRTU_RegisterCallbacks(HAL_ModbusRTU_FrameReceivedCallback_t frame_received_cb,
                                             HAL_ModbusRTU_TransmissionCompleteCallback_t transmission_complete_cb,
                                             HAL_ModbusRTU_ErrorCallback_t error_cb);

/**
 * @brief Sends a Modbus RTU frame.
 * This function calculates the CRC and transmits the complete frame over UART.
 * @param frame_p Pointer to the data to send (excluding CRC).
 * @param length The length of the data to send (excluding CRC).
 * @return HAL_OK on success, HAL_ERROR on failure (e.g., busy, invalid parameters).
 */
HAL_Status_t HAL_ModbusRTU_SendFrame(const uint8_t *frame_p, uint16_t length);

/**
 * @brief Gets the current Modbus RTU communication state.
 * @return The current HAL_ModbusRTU_State_t.
 */
HAL_ModbusRTU_State_t HAL_ModbusRTU_GetState(void);

/**
 * @brief Calculates the Modbus RTU CRC16 for a given data buffer.
 * This utility function can be used by higher layers if they need to pre-calculate CRC.
 * @param data_p Pointer to the data buffer.
 * @param length The length of the data buffer.
 * @return The calculated 16-bit CRC value.
 */
uint16_t HAL_ModbusRTU_CalculateCRC16(const uint8_t *data_p, uint16_t length);

#endif // HAL_MODBUS_RTU_H
