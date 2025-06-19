#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "adc.h"
#include "stm32f4xx_hal.h"
#include "main.h"

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
} key_t;

uint8_t is_key_down(key_t key);

void update_axis_states();

uint8_t was_key_pressed(key_t key);

uint8_t knob_step();

#endif // INPUT_H