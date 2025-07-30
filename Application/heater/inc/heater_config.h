// app/inc/heater_config.h

#ifndef HEATER_CONFIG_H
#define HEATER_CONFIG_H

#include <stdint.h>
#include "heater.h" // Includes HEATER_Config_t definition

// Declare the external constant array defined in heater_config.c
extern const HEATER_Config_t heater_configurations[];

// Declare the external constant for the number of configurations
extern const uint32_t HEATER_NUM_CONFIGURATIONS;

#endif /* HEATER_CONFIG_H */
