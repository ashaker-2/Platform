// app/inc/character_display_config.h

#ifndef CHARACTER_DISPLAY_CONFIG_H
#define CHARACTER_DISPLAY_CONFIG_H

#include <stdint.h>
#include "character_display.h" // Includes CHARACTER_DISPLAY_Config_t definition

// Declare the external constant array defined in character_display_config.c
extern const CHARACTER_DISPLAY_Config_t character_display_configurations[];

// Declare the external constant for the number of configurations
extern const uint32_t CHARACTER_DISPLAY_NUM_CONFIGURATIONS;

#endif /* CHARACTER_DISPLAY_CONFIG_H */
