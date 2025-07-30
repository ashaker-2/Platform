// app/inc/fan_config.h

#ifndef FAN_CONFIG_H // Renamed include guard
#define FAN_CONFIG_H

#include <stdint.h>
#include "fan.h" // Includes fan.h for FAN_Config_t definition // Updated include

// Declare the external constant array defined in fan_config.c
extern const FAN_Config_t fan_configurations[]; // Renamed struct

// Declare the external constant for the number of configurations
extern const uint32_t FAN_NUM_CONFIGURATIONS; // Renamed constant

#endif /* FAN_CONFIG_H */
