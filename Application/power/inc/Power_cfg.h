#ifndef POWER_CFG_H
#define POWER_CFG_H

#include "hal_adc.h"  // For HAL_ADC_Unit_t, HAL_ADC_Channel_t
#include "hal_gpio.h" // For HAL_GPIO_Pin_t
#include "system_monitor.h" // For fault IDs

/**
 * @file power_cfg.h
 * @brief Configuration header for the PowerMgr component.
 *
 * This file defines all the necessary configuration macros for the
 * PowerMgr module.
 */

// --- Fault ID Definitions (from SystemMonitor) ---
#define FAULT_ID_POWER_INIT_FAILURE     (SystemMonitor_FaultId_t)210
#define FAULT_ID_POWER_ADC_READ_FAILURE (SystemMonitor_FaultId_t)211
#define FAULT_ID_POWER_OVER_POWER       (SystemMonitor_FaultId_t)212

// --- Power Monitoring ADC Channel Mappings ---
#define POWER_VOLTAGE_ADC_UNIT      HAL_ADC_UNIT_0
#define POWER_VOLTAGE_ADC_CHANNEL   HAL_ADC_CHANNEL_2 // Example channel
#define POWER_CURRENT_ADC_UNIT      HAL_ADC_UNIT_0
#define POWER_CURRENT_ADC_CHANNEL   HAL_ADC_CHANNEL_3 // Example channel

// --- ADC Raw Value to Physical Unit Scaling Factors ---
// These values depend on your voltage divider, current sensor, and ADC reference.
// Example: 12-bit ADC (0-4095), 3.3V ref, voltage divider 10:1 (so 1V input = 0.1V at ADC)
// 3300mV / 4096 counts = 0.805 mV/count. If 10:1 divider, then 8.05 mV/count for actual voltage.
#define POWER_VOLTAGE_SCALE_FACTOR  8.05f // mV per ADC count
#define POWER_CURRENT_SCALE_FACTOR  0.5f  // mA per ADC count (e.g., for a current sensor outputting 1V/Amp)

// --- Power Thresholds for Fault Reporting ---
#define POWER_OVERCURRENT_THRESHOLD_MA  4500 // 4.5 Amps (e.g., for SyRS-02-02-08: 4A AC-1 load)
#define POWER_UNDERVOLTAGE_THRESHOLD_MV 10000 // 10 Volts (e.g., for 12V supply)
#define POWER_OVERPOWER_THRESHOLD_MW    50000 // 50 Watts (e.g., 4 VA max consumption)

// --- Power Mode Specific GPIOs ---
// Example: A GPIO pin to enable/disable a main power rail
#define POWER_MAIN_RAIL_ENABLE_GPIO_PIN HAL_GPIO_PIN_26

// Example: A button pin configured as wake-up source from sleep
#define POWER_WAKEUP_BUTTON_GPIO_PIN    HAL_GPIO_PIN_27

// --- Periodic Task Settings ---
#define POWER_MONITOR_PERIOD_MS         1000 // POWER_MainFunction called every 1 second

#endif // POWER_CFG_H