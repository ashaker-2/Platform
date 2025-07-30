// app/inc/light_control_config.h

#ifndef LIGHT_CONTROL_CONFIG_H
#define LIGHT_CONTROL_CONFIG_H

#include <stdint.h>
#include "light_control.h" // Includes LIGHT_Config_t definition

// Declare the external constant array defined in light_control_config.c
extern const LIGHT_Config_t light_configurations[];

// Declare the external constant for the number of configurations
extern const uint32_t LIGHT_NUM_CONFIGURATIONS;

#endif /* LIGHT_CONTROL_CONFIG_H */
