#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "adc.h"

#define DEBOUNCE_BUTTON_TIME_MS 150
#define DEBOUNCE_JOYCON_TIME_MS 250
typedef enum
{
    PLAY = 0,
    STOP,
    MODE,
    // Axis keys
    RIGHT,
    LEFT,
    UP,
    DOWN,
} in_key_t;

uint8_t is_key_down(in_key_t key);

void update_axis_states();

uint8_t was_key_pressed(in_key_t key);

uint8_t knob_step();

void clear_pressed();

#endif // INPUT_H