/* ============================================================================
 * SOURCE FILE: HAL/inc/hal_cfg.h
 * ============================================================================*/
/**
 * @file hal_cfg.h
 * @brief Defines physical pin assignments and fundamental hardware settings
 * for the Hardware Abstraction Layer (HAL). This file is internal to the HAL
 * and provides the HAL with its understanding of the board's hardware layout.
 * It is not intended for direct use or interpretation by the application layer.
 */
#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include "driver/gpio.h"       // For GPIO_NUM_x definitions
#include "driver/adc.h"        // For ADC_UNIT_x, ADC_CHANNEL_x
#include "driver/uart.h"       // For UART_NUM_x
#include "driver/i2c.h"        // For I2C_NUM_x
#include "driver/spi_master.h" // For SPI_HOST_x

// --- ESP32 Direct GPIO Pin Assignments ---

// Console/Flash UART0 (Default ESP-IDF, usually implicitly configured at startup)
#define HW_UART0_TX_GPIO GPIO_NUM_1
#define HW_UART0_RX_GPIO GPIO_NUM_3

// I2C Bus for IO Expander (I2C1 from your table, mapped to ESP32's hardware I2C_NUM_0)
#define HW_I2C_EXPANDER_PORT I2C_NUM_0 // ESP32's hardware I2C port 0
#define HW_I2C_EXPANDER_SDA_GPIO GPIO_NUM_21
#define HW_I2C_EXPANDER_SCL_GPIO GPIO_NUM_22

// Display Wiring (4-bit Mode LCD)
#define HW_DISPLAY_RS_GPIO GPIO_NUM_25
#define HW_DISPLAY_E_GPIO GPIO_NUM_26
#define HW_DISPLAY_DB4_GPIO GPIO_NUM_27
#define HW_DISPLAY_DB5_GPIO GPIO_NUM_32
#define HW_DISPLAY_DB6_GPIO GPIO_NUM_33
#define HW_DISPLAY_DB7_GPIO GPIO_NUM_14 // Note: Bootstrapping pin.

// Keypad Wiring (4x4 Matrix)
// Rows: Outputs, Columns: Inputs. Note: Strapping pins used for rows.
#define HW_KEYPAD_ROW1_GPIO GPIO_NUM_4  // Bootstrapping pin (pull-up at boot)
#define HW_KEYPAD_ROW2_GPIO GPIO_NUM_12 // Bootstrapping pin (pull-down at boot)
#define HW_KEYPAD_ROW3_GPIO GPIO_NUM_13 // Bootstrapping pin (pull-up at boot)
#define HW_KEYPAD_ROW4_GPIO GPIO_NUM_0  // **CRITICAL: Bootstrapping pin (boot button).**
                                        // **Requires careful external pull-up/down.**
// Keypad Columns: Using input-only GPIOs from ADC1, ideal for passive inputs.
#define HW_KEYPAD_COL1_GPIO GPIO_NUM_36 // Input-only pin (ADC1_CH0)
#define HW_KEYPAD_COL2_GPIO GPIO_NUM_37 // Input-only pin (ADC1_CH1)
#define HW_KEYPAD_COL3_GPIO GPIO_NUM_38 // Input-only pin (ADC1_CH2)
#define HW_KEYPAD_COL4_GPIO GPIO_NUM_39 // Input-only pin (ADC1_CH3)

// NTC Temperature Sensors (ADC1)
#define HW_ADC1_NTC_TEMP1_CHANNEL ADC1_CHANNEL_6 // Corresponds to GPIO34
#define HW_ADC1_NTC_TEMP2_CHANNEL ADC1_CHANNEL_7 // Corresponds to GPIO35
#define HW_ADC1_UNIT ADC_UNIT_1                  // Specifies ADC Unit 1

// Digital Temperature/Humidity Sensors (Bidirectional I/O)
#define HW_TEMP_HUM_0_GPIO GPIO_NUM_19
#define HW_TEMP_HUM_1_GPIO GPIO_NUM_23
#define HW_TEMP_HUM_2_GPIO GPIO_NUM_18
#define HW_TEMP_HUM_3_GPIO GPIO_NUM_5 // Note: Bootstrapping pin.

// Alarm Output
#define HW_ALARM_GPIO GPIO_NUM_2 // Note: Bootstrapping pin, also ADC2.

// --- CH423S GPIO Expander Configuration ---
#define HW_CH423S_I2C_ADDR (0x40) // Default I2C address for CH423S. Verify datasheet.

// CH423S GPIO Pin Mappings (0-15) - used for relay control and LEDs
#define HW_CH423S_GP_FAN_1 (0)
#define HW_CH423S_GP_FAN_2 (1)
#define HW_CH423S_GP_FAN_3 (2)
#define HW_CH423S_GP_FAN_4 (3)

#define HW_CH423S_GP_VEN_1 (4)
#define HW_CH423S_GP_VEN_2 (5)
#define HW_CH423S_GP_VEN_3 (6)
#define HW_CH423S_GP_VEN_4 (7)

#define HW_CH423S_GP_LIGHT (8)
#define HW_CH423S_GP_HEATER_1 (9)
#define HW_CH423S_GP_PUMP_1 (10)

#define HW_CH423S_GP_FAN_LED (11)
#define HW_CH423S_GP_VEN_LED (12)
#define HW_CH423S_GP_LIGHT_LED (13)
#define HW_CH423S_GP_PUMP_LED (14)
#define HW_CH423S_GP_HEATER_LED (15)

// --- Unused Peripherals (for completeness) ---
// These GPIOs are listed in your table but marked as "No" for usage in this system.
#define HW_UART1_TX_GPIO_UNUSED GPIO_NUM_17 // Modbus/KNX TX
#define HW_UART1_RX_GPIO_UNUSED GPIO_NUM_16 // Modbus/KNX RX

#define HW_SPI_HOST_UNUSED SPI2_HOST         // Or SPI3_HOST
#define HW_SPI_MISO_UNUSED GPIO_NUM_XX       // Placeholder
#define HW_SPI_MOSI_UNUSED GPIO_NUM_XX       // Placeholder
#define HW_SPI_CLK_UNUSED GPIO_NUM_XX        // Placeholder
#define HW_SPI_CS_EXTERNAL_FLASH GPIO_NUM_XX // Placeholder for External Flash CS

#define HW_I2C0_SDA_EEPROM_UNUSED GPIO_NUM_XX // Placeholder for EEPROM I2C0 SDA
#define HW_I2C0_SCL_EEPROM_UNUSED GPIO_NUM_XX // Placeholder for EEPROM I2C0 SCL

#endif /* HAL_CONFIG_H */
