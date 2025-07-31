// app/middleware/inc/modbus_middleware.h
#ifndef MODBUS_MIDDLEWARE_H
#define MODBUS_MIDDLEWARE_H

#include <stdint.h>


// Define common Modbus register addresses for our system
// These are examples and should be finalized based on your actual Modbus map.
#define MODBUS_SLAVE_ID             1       // Default slave ID for our device
#define MODBUS_REG_ROOM_TEMP_X100   0x0001  // Holding Register for Room Temperature (multiplied by 100)
#define MODBUS_REG_HUMIDITY_X100    0x0002  // Holding Register for Room Humidity (multiplied by 100)
#define MODBUS_REG_HEATSINK_TEMP_X100 0x0003// Holding Register for Heatsink Temperature (multiplied by 100)
#define MODBUS_REG_FAN_STAGE        0x0004  // Holding Register for Fan Stage (0-100)
#define MODBUS_REG_HEATER_STATE     0x0005  // Coil/Holding Register for Heater State (0=OFF, 1=ON)
#define MODBUS_REG_PUMP_STATE       0x0006  // Coil/Holding Register for Pump State (0=OFF, 1=ON)
#define MODBUS_REG_VENTILATOR_STATE 0x0007  // Coil/Holding Register for Ventilator State (0=OFF, 1=ON)

// Example writable registers (from Modbus Master perspective)
#define MODBUS_REG_SET_MIN_OP_TEMP_X100   0x0100 // Set Min Operational Temp (written by master)
#define MODBUS_REG_SET_MAX_OP_TEMP_X100   0x0101 // Set Max Operational Temp (written by master)
#define MODBUS_REG_SET_MIN_OP_HUM_X100    0x0102 // Set Min Operational Humidity (written by master)
#define MODBUS_REG_SET_MAX_OP_HUM_X100    0x0103 // Set Max Operational Humidity (written by master)


/**
 * @brief Initialize the Modbus middleware.
 * This function sets up the underlying Modbus driver (e.g., UART for RTU, TCP stack for TCP).
 * In a real system, this would configure serial ports or network interfaces.
 * @return APP_OK if successful, APP_ERROR otherwise.
 */
uint8_t MODBUS_MW_Init(void);

/**
 * @brief Reads a value from a Modbus holding register (simulated).
 * This function typically serves requests from a Modbus master. In our system,
 * it can also be used by the COMM_MainTask to check if a master has written to a register.
 * @param slave_address The Modbus slave ID (our device).
 * @param register_address The 16-bit address of the holding register.
 * @param value Pointer to a uint16_t where the read value will be stored.
 * @return APP_OK if the read operation is successful, APP_ERROR otherwise.
 */
uint8_t MODBUS_MW_ReadHoldingRegister(uint8_t slave_address, uint16_t register_address, uint16_t *value);

/**
 * @brief Writes a value to a Modbus holding register (simulated).
 * This function is used by the COMM_MainTask to update internal register values
 * that a Modbus master can then read.
 * @param slave_address The Modbus slave ID (our device).
 * @param register_address The 16-bit address of the holding register.
 * @param value The 16-bit value to write.
 * @return APP_OK if the write operation is successful, APP_ERROR otherwise.
 */
uint8_t MODBUS_MW_WriteHoldingRegister(uint8_t slave_address, uint16_t register_address, uint16_t value);

/**
 * @brief Performs periodic processing for the Modbus stack.
 * This function would typically handle incoming Modbus requests, process them,
 * and manage responses. It is intended to be called frequently by COMM_MainTask.
 * @return APP_OK if processing is successful, APP_ERROR if an unrecoverable error occurs.
 */
uint8_t MODBUS_MW_Process(void);

#endif // MODBUS_MIDDLEWARE_H
