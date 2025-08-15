/**
 * @file modbus.c
 * @brief Service Layer for Modbus RTU Master - Implementation.
 *
 * This module provides the concrete implementation for the Modbus RTU Master
 * service interface defined in `modbus.h`. It handles Modbus protocol
 * framing, CRC calculation, and communication with Modbus slave devices
 * over UART using the ESP-IDF UART driver.
 */

#include "modbus.h"
#include "modbus_cfg.h" // Include configuration header

#include "driver/uart.h"     // ESP-IDF UART driver
// #include "esp_log.h"         // ESP-IDF logging
// #include "freertos/FreeRTOS.h" // For FreeRTOS types (e.g., tick conversion)
// #include "freertos/task.h"   // For vTaskDelay

// Define a tag for ESP-IDF logging
static const char *TAG = "MODBUS";

// Modbus RTU defines
#define MODBUS_MAX_ADU_LENGTH       256 // Max Application Data Unit (ADU) length
#define MODBUS_RTU_HEADER_LENGTH    1   // Slave Address
#define MODBUS_RTU_FOOTER_LENGTH    2   // CRC
#define MODBUS_RTU_MIN_ADU_LENGTH   4   // Slave Address + Function Code + Data (min 1 byte) + CRC

// Modbus Function Codes
#define FC_READ_COILS               0x01
#define FC_READ_DISCRETE_INPUTS     0x02 // Not implemented in this version, but good to note
#define FC_READ_HOLDING_REGISTERS   0x03
#define FC_READ_INPUT_REGISTERS     0x04
#define FC_WRITE_SINGLE_COIL        0x05
#define FC_WRITE_SINGLE_REGISTER    0x06
#define FC_WRITE_MULTIPLE_COILS     0x0F
#define FC_WRITE_MULTIPLE_REGISTERS 0x10

// Modbus Exception Codes (Function Code ORed with 0x80)
#define EXCEPTION_ILLEGAL_FUNCTION          0x01
#define EXCEPTION_ILLEGAL_DATA_ADDRESS      0x02
#define EXCEPTION_ILLEGAL_DATA_VALUE        0x03
#define EXCEPTION_SLAVE_DEVICE_FAILURE      0x04
#define EXCEPTION_ACKNOWLEDGE               0x05
#define EXCEPTION_SLAVE_DEVICE_BUSY         0x06
#define EXCEPTION_GATEWAY_PATH_UNAVAILABLE  0x0A
#define EXCEPTION_GATEWAY_TARGET_NO_RESPONSE 0x0B

/**
 * @brief Structure to hold runtime data for each Modbus UART port.
 */
typedef struct
{
    bool initialized;                       /**< Flag indicating if the port is initialized. */
    uart_port_t esp_uart_port;              /**< ESP-IDF UART port number. */
    SemaphoreHandle_t mutex;                /**< Mutex to protect Modbus communication on this port. */
} MODBUS_PortData_t;

// Array to hold runtime data for each Modbus UART port
static MODBUS_PortData_t g_modbus_port_data[MODBUS_UART_PORT_MAX];

/**
 * @brief Internal helper to map MODBUS_UartPort_t to ESP-IDF uart_port_t.
 * @param port The Modbus UART port.
 * @param esp_port Pointer to store the ESP-IDF UART port number.
 * @return true if mapping is successful, false otherwise.
 */
static bool modbus_map_port_to_esp_port(MODBUS_UartPort_t port, uart_port_t *esp_port)
{
    switch (port)
    {
    case MODBUS_UART_PORT_0:
        *esp_port = UART_NUM_0;
        break;
    case MODBUS_UART_PORT_1:
        *esp_port = UART_NUM_1;
        break;
    case MODBUS_UART_PORT_2:
        *esp_port = UART_NUM_2;
        break;
    default:
        ESP_LOGE(TAG, "Invalid Modbus UART port: %d", port);
        return false;
    }
    return true;
}

/**
 * @brief Internal helper to convert ESP-IDF error codes to MODBUS_Status_t.
 * @param esp_err The ESP-IDF error code.
 * @return The corresponding MODBUS_Status_t.
 */
static MODBUS_Status_t modbus_esp_err_to_status(esp_err_t esp_err)
{
    switch (esp_err)
    {
    case ESP_OK:
        return MODBUS_STATUS_OK;
    case ESP_ERR_INVALID_ARG:
        return MODBUS_STATUS_INVALID_PARAM;
    case ESP_FAIL: // Generic failure, often due to timeout or other comms issue
        return MODBUS_STATUS_ERROR;
    case ESP_ERR_TIMEOUT:
        return MODBUS_STATUS_TIMEOUT;
    case ESP_ERR_INVALID_STATE: // UART not initialized or in wrong state
        return MODBUS_STATUS_NOT_INITIALIZED;
    default:
        return MODBUS_STATUS_ERROR;
    }
}

/**
 * @brief Calculates the Modbus RTU CRC16 checksum.
 * @param buf Pointer to the data buffer.
 * @param len Length of the data buffer.
 * @return The calculated CRC16 value.
 */
static uint16_t modbus_calculate_crc(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t pos = 0; pos < len; pos++)
    {
        crc ^= (uint16_t)buf[pos];          // XOR byte into least significant byte of crc
        for (int i = 8; i != 0; i--)        // Loop over each bit
        {
            if ((crc & 0x0001) != 0)        // If the LSB is set
            {
                crc >>= 1;                  // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else                            // Else LSB is not set
            {
                crc >>= 1;                  // Just shift right
            }
        }
    }
    return crc;
}

/**
 * @brief Handles Modbus exception responses.
 * @param function_code The function code from the request.
 * @param exception_code The exception code received.
 * @return The corresponding MODBUS_Status_t.
 */
static MODBUS_Status_t modbus_handle_exception(uint8_t function_code, uint8_t exception_code)
{
    ESP_LOGE(TAG, "Modbus Exception (FC: 0x%02X, ExCode: 0x%02X)", function_code, exception_code);
    switch (exception_code)
    {
    case EXCEPTION_ILLEGAL_FUNCTION:
        return MODBUS_STATUS_ILLEGAL_FUNCTION;
    case EXCEPTION_ILLEGAL_DATA_ADDRESS:
        return MODBUS_STATUS_ILLEGAL_DATA_ADDRESS;
    case EXCEPTION_ILLEGAL_DATA_VALUE:
        return MODBUS_STATUS_ILLEGAL_DATA_VALUE;
    case EXCEPTION_SLAVE_DEVICE_FAILURE:
        return MODBUS_STATUS_SLAVE_DEVICE_FAILURE;
    case EXCEPTION_ACKNOWLEDGE:
        return MODBUS_STATUS_ACKNOWLEDGE;
    case EXCEPTION_SLAVE_DEVICE_BUSY:
        return MODBUS_STATUS_SLAVE_BUSY;
    case EXCEPTION_GATEWAY_PATH_UNAVAILABLE:
        return MODBUS_STATUS_GATEWAY_PATH_UNAVAILABLE;
    case EXCEPTION_GATEWAY_TARGET_NO_RESPONSE:
        return MODBUS_STATUS_GATEWAY_TARGET_NO_RESPONSE;
    default:
        return MODBUS_STATUS_ERROR; // Unknown exception
    }
}

/**
 * @brief Internal function to send a Modbus request and receive a response.
 * @param port The Modbus UART port.
 * @param request_buffer Pointer to the request PDU (without CRC).
 * @param request_len Length of the request PDU.
 * @param response_buffer Pointer to a buffer to store the response ADU (including CRC).
 * @param max_response_len Maximum size of the response buffer.
 * @param actual_response_len Pointer to store the actual length of the received response.
 * @return MODBUS_Status_t if successful, an error code otherwise.
 */
static MODBUS_Status_t modbus_send_receive(MODBUS_UartPort_t port,
                                                   const uint8_t *request_buffer, uint16_t request_len,
                                                   uint8_t *response_buffer, uint16_t max_response_len,
                                                   uint16_t *actual_response_len)
{
    if (!g_modbus_port_data[port].initialized)
    {
        return MODBUS_STATUS_NOT_INITIALIZED;
    }

    uart_port_t esp_uart_port = g_modbus_port_data[port].esp_uart_port;
    const MODBUS_PortConfig_t *cfg = &g_modbus_port_configs[port];
    esp_err_t ret = ESP_OK;
    MODBUS_Status_t status = MODBUS_STATUS_ERROR;
    uint8_t tx_buffer[MODBUS_MAX_ADU_LENGTH];
    uint16_t tx_len = request_len;

    // Copy PDU to TX buffer
    memcpy(tx_buffer, request_buffer, request_len);

    // Add CRC
    uint16_t crc = modbus_calculate_crc(tx_buffer, tx_len);
    tx_buffer[tx_len++] = (uint8_t)(crc & 0xFF);         // CRC Low Byte
    tx_buffer[tx_len++] = (uint8_t)((crc >> 8) & 0xFF);  // CRC High Byte

    // Take mutex before sending
    if (xSemaphoreTake(g_modbus_port_data[port].mutex, portMAX_DELAY) != pdTRUE)
    {
        ESP_LOGE(TAG, "Failed to take Modbus mutex for port %d", port);
        return MODBUS_STATUS_BUSY;
    }

    // Clear UART RX buffer before sending
    uart_flush_input(esp_uart_port);

    // Set RTS (DE) high for transmission if configured
    if (cfg->rts_gpio_num != -1)
    {
        uart_set_pin(esp_uart_port, cfg->tx_gpio_num, cfg->rx_gpio_num, cfg->rts_gpio_num, UART_PIN_NO_CHANGE);
        uart_set_mode(esp_uart_port, UART_MODE_RS485_HALF_DUPLEX);
    }

    // Send request
    int bytes_written = uart_write_bytes(esp_uart_port, (const char *)tx_buffer, tx_len);
    if (bytes_written != tx_len)
    {
        ESP_LOGE(TAG, "Failed to write all bytes to UART for port %d. Wrote %d of %d.", port, bytes_written, tx_len);
        status = MODBUS_STATUS_ERROR;
        goto release_mutex;
    }

    // Wait for transmission to complete (important for RS485 DE control)
    ret = uart_wait_tx_done(esp_uart_port, pdMS_TO_TICKS(100)); // Short timeout for TX done
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "UART TX done wait timeout for port %d: %s", port, esp_err_to_name(ret));
        status = modbus_esp_err_to_status(ret);
        goto release_mutex;
    }

    // Set RTS (DE) low for reception if configured
    if (cfg->rts_gpio_num != -1)
    {
        // Revert to normal UART mode or ensure DE is low
        uart_set_mode(esp_uart_port, UART_MODE_UART_TX_RX); // Or specifically set RTS low
    }


    // Read response
    TickType_t start_ticks = xTaskGetTickCount();
    TickType_t end_ticks = start_ticks + pdMS_TO_TICKS(cfg->response_timeout_ms);
    *actual_response_len = 0;

    while (xTaskGetTickCount() < end_ticks && *actual_response_len < max_response_len)
    {
        int len = uart_read_bytes(esp_uart_port, response_buffer + *actual_response_len, max_response_len - *actual_response_len, pdMS_TO_TICKS(10));
        if (len > 0)
        {
            *actual_response_len += len;
            // Check for minimum ADU length (slave_addr + func_code + data + CRC)
            // This is a heuristic, a proper Modbus stack would check for 3.5 character times
            if (*actual_response_len >= MODBUS_RTU_MIN_ADU_LENGTH)
            {
                // If enough bytes for CRC, check if complete
                uint16_t expected_crc = modbus_calculate_crc(response_buffer, *actual_response_len - 2);
                uint16_t received_crc = (response_buffer[*actual_response_len - 1] << 8) | response_buffer[*actual_response_len - 2];

                if (expected_crc == received_crc)
                {
                    status = MODBUS_STATUS_OK;
                    goto release_mutex; // Response received and CRC matches
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1)); // Small delay to yield
    }

    if (*actual_response_len == 0)
    {
        ESP_LOGE(TAG, "Modbus response timeout for port %d.", port);
        status = MODBUS_STATUS_TIMEOUT;
    }
    else
    {
        ESP_LOGE(TAG, "Modbus response CRC error or incomplete for port %d. Received %d bytes.", port, *actual_response_len);
        status = MODBUS_STATUS_CRC_ERROR; // Or incomplete
    }

release_mutex:
    xSemaphoreGive(g_modbus_port_data[port].mutex);
    return status;
}


MODBUS_Status_t MODBUS_Init(MODBUS_UartPort_t port)
{
    if (port >= MODBUS_UART_PORT_MAX)
    {
        ESP_LOGE(TAG, "Invalid Modbus UART port: %d", port);
        return MODBUS_STATUS_INVALID_PARAM;
    }

    if (g_modbus_port_data[port].initialized)
    {
        ESP_LOGW(TAG, "Modbus UART port %d already initialized.", port);
        return MODBUS_STATUS_ALREADY_INITIALIZED;
    }

    uart_port_t esp_uart_port;
    if (!modbus_map_port_to_esp_port(port, &esp_uart_port))
    {
        return MODBUS_STATUS_INVALID_PARAM;
    }

    const MODBUS_PortConfig_t *cfg = &g_modbus_port_configs[port];
    esp_err_t ret;

    uart_config_t uart_config = {
        .baud_rate = cfg->baud_rate,
        .data_bits = (cfg->data_bits == MODBUS_DATA_BITS_8) ? UART_DATA_8_BITS : UART_DATA_7_BITS,
        .parity = (cfg->parity == MODBUS_PARITY_NONE) ? UART_PARITY_DISABLE :
                  ((cfg->parity == MODBUS_PARITY_EVEN) ? UART_PARITY_EVEN : UART_PARITY_ODD),
        .stop_bits = (cfg->stop_bits == MODBUS_STOP_BITS_1) ? UART_STOP_BITS_1 : UART_STOP_BITS_2,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_APB,
    };

    // Configure UART parameters
    ret = uart_param_config(esp_uart_port, &uart_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure UART parameters for port %d: %s", port, esp_err_to_name(ret));
        return modbus_esp_err_to_status(ret);
    }

    // Set UART pins (TX, RX, RTS, CTS)
    ret = uart_set_pin(esp_uart_port, cfg->tx_gpio_num, cfg->rx_gpio_num, cfg->rts_gpio_num, UART_PIN_NO_CHANGE);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set UART pins for port %d: %s", port, esp_err_to_name(ret));
        return modbus_esp_err_to_status(ret);
    }

    // Install UART driver, and get the queue.
    ret = uart_driver_install(esp_uart_port, cfg->rx_buffer_size, cfg->tx_buffer_size, 0, NULL, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install UART driver for port %d: %s", port, esp_err_to_name(ret));
        return modbus_esp_err_to_status(ret);
    }

    // Create mutex for this port
    g_modbus_port_data[port].mutex = xSemaphoreCreateMutex();
    if (g_modbus_port_data[port].mutex == NULL)
    {
        ESP_LOGE(TAG, "Failed to create mutex for Modbus port %d", port);
        uart_driver_delete(esp_uart_port);
        return MODBUS_STATUS_ERROR;
    }

    g_modbus_port_data[port].initialized = true;
    g_modbus_port_data[port].esp_uart_port = esp_uart_port;

    ESP_LOGI(TAG, "Modbus service initialized on UART%d (TX:%d, RX:%d, RTS:%d, Baud:%lu, Data:%d, Stop:%d, Parity:%d)",
             port, cfg->tx_gpio_num, cfg->rx_gpio_num, cfg->rts_gpio_num, cfg->baud_rate, cfg->data_bits, cfg->stop_bits, cfg->parity);
    return MODBUS_STATUS_OK;
}

MODBUS_Status_t MODBUS_DeInit(MODBUS_UartPort_t port)
{
    if (port >= MODBUS_UART_PORT_MAX)
    {
        ESP_LOGE(TAG, "Invalid Modbus UART port: %d", port);
        return MODBUS_STATUS_INVALID_PARAM;
    }
    if (!g_modbus_port_data[port].initialized)
    {
        ESP_LOGW(TAG, "Modbus UART port %d not initialized.", port);
        return MODBUS_STATUS_OK; // Already de-initialized or never initialized
    }

    uart_port_t esp_uart_port = g_modbus_port_data[port].esp_uart_port;
    esp_err_t ret = uart_driver_delete(esp_uart_port);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to delete UART driver for port %d: %s", port, esp_err_to_name(ret));
        return modbus_esp_err_to_status(ret);
    }

    if (g_modbus_port_data[port].mutex != NULL)
    {
        vSemaphoreDelete(g_modbus_port_data[port].mutex);
        g_modbus_port_data[port].mutex = NULL;
    }

    g_modbus_port_data[port].initialized = false;
    ESP_LOGI(TAG, "Modbus service on UART%d de-initialized successfully.", port);
    return MODBUS_STATUS_OK;
}

/**
 * @brief Generic Modbus master request handler.
 * @param port The Modbus UART port.
 * @param slave_addr The Modbus slave address.
 * @param function_code The Modbus function code.
 * @param start_addr The starting address (register/coil).
 * @param quantity The quantity of registers/coils.
 * @param write_data Optional pointer to data to write (for write functions).
 * @param write_data_len Length of write_data.
 * @param read_data_buffer Pointer to buffer for read data (for read functions).
 * @param read_data_buffer_len Expected length of read data.
 * @return MODBUS_Status_t.
 */
static MODBUS_Status_t modbus_master_request(MODBUS_UartPort_t port,
                                                     uint8_t slave_addr,
                                                     uint8_t function_code,
                                                     uint16_t start_addr,
                                                     uint16_t quantity,
                                                     const uint8_t *write_data,
                                                     uint16_t write_data_len,
                                                     uint8_t *read_data_buffer,
                                                     uint16_t read_data_buffer_len)
{
    if (port >= MODBUS_UART_PORT_MAX || !g_modbus_port_data[port].initialized)
    {
        ESP_LOGE(TAG, "Modbus port %d not initialized or invalid.", port);
        return MODBUS_STATUS_NOT_INITIALIZED;
    }
    if (slave_addr == 0 || slave_addr > 247)
    {
        ESP_LOGE(TAG, "Invalid slave address: %d", slave_addr);
        return MODBUS_STATUS_INVALID_PARAM;
    }

    uint8_t request[MODBUS_MAX_ADU_LENGTH - MODBUS_RTU_FOOTER_LENGTH]; // PDU max size
    uint16_t request_len = 0;
    uint8_t response[MODBUS_MAX_ADU_LENGTH];
    uint16_t actual_response_len = 0;
    MODBUS_Status_t status = MODBUS_STATUS_ERROR;
    const MODBUS_PortConfig_t *cfg = &g_modbus_port_configs[port];

    // Build request PDU
    request[request_len++] = slave_addr;
    request[request_len++] = function_code;
    request[request_len++] = (uint8_t)((start_addr >> 8) & 0xFF); // Address High
    request[request_len++] = (uint8_t)(start_addr & 0xFF);       // Address Low

    if (function_code == FC_WRITE_SINGLE_COIL || function_code == FC_WRITE_SINGLE_REGISTER)
    {
        // For single write, quantity is the value
        request[request_len++] = (uint8_t)((quantity >> 8) & 0xFF); // Value High
        request[request_len++] = (uint8_t)(quantity & 0xFF);       // Value Low
    }
    else
    {
        // For multiple read/write, quantity is number of items
        request[request_len++] = (uint8_t)((quantity >> 8) & 0xFF); // Quantity High
        request[request_len++] = (uint8_t)(quantity & 0xFF);       // Quantity Low
    }


    if (write_data != NULL && write_data_len > 0)
    {
        // Add byte count for multiple writes
        if (function_code == FC_WRITE_MULTIPLE_REGISTERS)
        {
            request[request_len++] = (uint8_t)(quantity * 2); // Byte count (2 bytes per register)
        }
        else if (function_code == FC_WRITE_MULTIPLE_COILS)
        {
            request[request_len++] = (uint8_t)write_data_len; // Byte count for coils
        }
        memcpy(&request[request_len], write_data, write_data_len);
        request_len += write_data_len;
    }

    for (uint8_t retry = 0; retry <= cfg->max_retries; retry++)
    {
        status = modbus_send_receive(port, request, request_len, response, MODBUS_MAX_ADU_LENGTH, &actual_response_len);

        if (status == MODBUS_STATUS_OK)
        {
            // Validate response: slave address, function code, CRC already checked by modbus_send_receive
            if (response[0] != slave_addr)
            {
                ESP_LOGE(TAG, "Received response from wrong slave address (expected 0x%02X, got 0x%02X)", slave_addr, response[0]);
                status = MODBUS_STATUS_UNEXPECTED_RESPONSE;
                continue; // Retry
            }

            if ((response[1] & 0x7F) != function_code) // Check function code, ignoring exception bit
            {
                ESP_LOGE(TAG, "Received response with wrong function code (expected 0x%02X, got 0x%02X)", function_code, response[1]);
                status = MODBUS_STATUS_UNEXPECTED_RESPONSE;
                continue; // Retry
            }

            // Check for Modbus exception response
            if ((response[1] & 0x80) == 0x80)
            {
                if (actual_response_len >= 3) // Slave_addr + FC + Exception_code + CRC
                {
                    status = modbus_handle_exception(function_code, response[2]);
                    // No retry for exceptions, as it's a valid Modbus response
                    break;
                }
                else
                {
                    ESP_LOGE(TAG, "Incomplete exception response received for port %d", port);
                    status = MODBUS_STATUS_UNEXPECTED_RESPONSE;
                    continue; // Retry
                }
            }

            // Handle successful response based on function code
            switch (function_code)
            {
                case FC_READ_COILS:
                case FC_READ_DISCRETE_INPUTS:
                {
                    // Response: Slave_addr + FC + Byte_Count + Data + CRC
                    if (actual_response_len < 3 + MODBUS_RTU_FOOTER_LENGTH) { // Min 1 byte data
                        status = MODBUS_STATUS_UNEXPECTED_RESPONSE;
                        break;
                    }
                    uint8_t byte_count = response[2];
                    if (actual_response_len != (3 + byte_count + MODBUS_RTU_FOOTER_LENGTH)) {
                        status = MODBUS_STATUS_UNEXPECTED_RESPONSE;
                        break;
                    }
                    if (read_data_buffer != NULL) {
                        memcpy(read_data_buffer, &response[3], byte_count);
                    }
                    status = MODBUS_STATUS_OK;
                    break;
                }
                case FC_READ_HOLDING_REGISTERS:
                case FC_READ_INPUT_REGISTERS:
                {
                    // Response: Slave_addr + FC + Byte_Count + Data + CRC
                    if (actual_response_len < 3 + MODBUS_RTU_FOOTER_LENGTH) { // Min 1 byte data
                        status = MODBUS_STATUS_UNEXPECTED_RESPONSE;
                        break;
                    }
                    uint8_t byte_count = response[2];
                    if (actual_response_len != (3 + byte_count + MODBUS_RTU_FOOTER_LENGTH)) {
                        status = MODBUS_STATUS_UNEXPECTED_RESPONSE;
                        break;
                    }
                    if (read_data_buffer != NULL)
                    {
                        // Copy bytes and convert to uint16_t (big-endian)
                        for (uint16_t i = 0; i < quantity; i++)
                        {
                            read_data_buffer[i*2] = response[3 + (i*2)];
                            read_data_buffer[i*2 + 1] = response[3 + (i*2) + 1];
                        }
                    }
                    status = MODBUS_STATUS_OK;
                    break;
                }
                case FC_WRITE_SINGLE_COIL:
                case FC_WRITE_SINGLE_REGISTER:
                {
                    // Response: Slave_addr + FC + Address + Value + CRC (echo of request)
                    if (actual_response_len != 6 + MODBUS_RTU_FOOTER_LENGTH) {
                        status = MODBUS_STATUS_UNEXPECTED_RESPONSE;
                        break;
                    }
                    // Further validation can be done here (e.g., check echoed address/value)
                    status = MODBUS_STATUS_OK;
                    break;
                }
                case FC_WRITE_MULTIPLE_COILS:
                case FC_WRITE_MULTIPLE_REGISTERS:
                {
                    // Response: Slave_addr + FC + Start_Address + Quantity + CRC
                    if (actual_response_len != 6 + MODBUS_RTU_FOOTER_LENGTH) {
                        status = MODBUS_STATUS_UNEXPECTED_RESPONSE;
                        break;
                    }
                    // Further validation can be done here (e.g., check echoed start address/quantity)
                    status = MODBUS_STATUS_OK;
                    break;
                }
                default:
                    ESP_LOGE(TAG, "Unsupported or unexpected function code in response: 0x%02X", function_code);
                    status = MODBUS_STATUS_UNEXPECTED_RESPONSE;
                    break;
            }
            if (status == MODBUS_STATUS_OK) {
                break; // Success, exit retry loop
            }
        }
        else
        {
            ESP_LOGW(TAG, "Modbus communication failed for port %d, slave 0x%02X (retry %d/%d). Status: %d",
                     port, slave_addr, retry + 1, cfg->max_retries, status);
            vTaskDelay(pdMS_TO_TICKS(50)); // Small delay before retry
        }
    }

    return status;
}


MODBUS_Status_t MODBUS_ReadHoldingRegisters(MODBUS_UartPort_t port,
                                                    uint8_t slave_addr,
                                                    uint16_t start_addr,
                                                    uint16_t num_regs,
                                                    uint16_t *data_buffer)
{
    if (num_regs == 0 || num_regs > 125 || data_buffer == NULL)
    {
        ESP_LOGE(TAG, "Invalid parameters for ReadHoldingRegisters: num_regs=%u, data_buffer=%p", num_regs, data_buffer);
        return MODBUS_STATUS_INVALID_PARAM;
    }
    uint16_t expected_read_bytes = num_regs * 2;
    return modbus_master_request(port, slave_addr, FC_READ_HOLDING_REGISTERS,
                                     start_addr, num_regs,
                                     NULL, 0,
                                     (uint8_t *)data_buffer, expected_read_bytes);
}

MODBUS_Status_t MODBUS_ReadInputRegisters(MODBUS_UartPort_t port,
                                                  uint8_t slave_addr,
                                                  uint16_t start_addr,
                                                  uint16_t num_regs,
                                                  uint16_t *data_buffer)
{
    if (num_regs == 0 || num_regs > 125 || data_buffer == NULL)
    {
        ESP_LOGE(TAG, "Invalid parameters for ReadInputRegisters: num_regs=%u, data_buffer=%p", num_regs, data_buffer);
        return MODBUS_STATUS_INVALID_PARAM;
    }
    uint16_t expected_read_bytes = num_regs * 2;
    return modbus_master_request(port, slave_addr, FC_READ_INPUT_REGISTERS,
                                     start_addr, num_regs,
                                     NULL, 0,
                                     (uint8_t *)data_buffer, expected_read_bytes);
}

MODBUS_Status_t MODBUS_WriteSingleRegister(MODBUS_UartPort_t port,
                                                   uint8_t slave_addr,
                                                   uint16_t reg_addr,
                                                   uint16_t value)
{
    // For single register write, quantity parameter is the value itself
    return modbus_master_request(port, slave_addr, FC_WRITE_SINGLE_REGISTER,
                                     reg_addr, value,
                                     NULL, 0,
                                     NULL, 0);
}

MODBUS_Status_t MODBUS_WriteMultipleRegisters(MODBUS_UartPort_t port,
                                                      uint8_t slave_addr,
                                                      uint16_t start_addr,
                                                      uint16_t num_regs,
                                                      const uint16_t *data_buffer)
{
    if (num_regs == 0 || num_regs > 123 || data_buffer == NULL)
    {
        ESP_LOGE(TAG, "Invalid parameters for WriteMultipleRegisters: num_regs=%u, data_buffer=%p", num_regs, data_buffer);
        return MODBUS_STATUS_INVALID_PARAM;
    }
    // Modbus RTU expects data in bytes, big-endian for registers
    uint8_t byte_data[num_regs * 2];
    for (uint16_t i = 0; i < num_regs; i++)
    {
        byte_data[i * 2] = (uint8_t)((data_buffer[i] >> 8) & 0xFF);   // High byte
        byte_data[i * 2 + 1] = (uint8_t)(data_buffer[i] & 0xFF);      // Low byte
    }

    return modbus_master_request(port, slave_addr, FC_WRITE_MULTIPLE_REGISTERS,
                                     start_addr, num_regs,
                                     byte_data, num_regs * 2,
                                     NULL, 0);
}

MODBUS_Status_t MODBUS_ReadCoils(MODBUS_UartPort_t port,
                                         uint8_t slave_addr,
                                         uint16_t start_addr,
                                         uint16_t num_coils,
                                         uint8_t *data_buffer)
{
    if (num_coils == 0 || num_coils > 2000 || data_buffer == NULL)
    {
        ESP_LOGE(TAG, "Invalid parameters for ReadCoils: num_coils=%u, data_buffer=%p", num_coils, data_buffer);
        return MODBUS_STATUS_INVALID_PARAM;
    }
    uint16_t expected_read_bytes = (num_coils + 7) / 8; // Ceiling division
    return modbus_master_request(port, slave_addr, FC_READ_COILS,
                                     start_addr, num_coils,
                                     NULL, 0,
                                     data_buffer, expected_read_bytes);
}

MODBUS_Status_t MODBUS_WriteSingleCoil(MODBUS_UartPort_t port,
                                               uint8_t slave_addr,
                                               uint16_t coil_addr,
                                               bool state)
{
    uint16_t value = state ? 0xFF00 : 0x0000; // Modbus standard for ON/OFF
    return modbus_master_request(port, slave_addr, FC_WRITE_SINGLE_COIL,
                                     coil_addr, value,
                                     NULL, 0,
                                     NULL, 0);
}

MODBUS_Status_t MODBUS_WriteMultipleCoils(MODBUS_UartPort_t port,
                                                  uint8_t slave_addr,
                                                  uint16_t start_addr,
                                                  uint16_t num_coils,
                                                  const uint8_t *data_buffer)
{
    if (num_coils == 0 || num_coils > 1968 || data_buffer == NULL)
    {
        ESP_LOGE(TAG, "Invalid parameters for WriteMultipleCoils: num_coils=%u, data_buffer=%p", num_coils, data_buffer);
        return MODBUS_STATUS_INVALID_PARAM;
    }
    uint16_t byte_count = (num_coils + 7) / 8; // Ceiling division
    return modbus_master_request(port, slave_addr, FC_WRITE_MULTIPLE_COILS,
                                     start_addr, num_coils,
                                     data_buffer, byte_count,
                                     NULL, 0);
}

