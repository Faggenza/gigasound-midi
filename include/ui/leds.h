#ifndef UI_LEDS_H
#define UI_LEDS_H

#include "gigagl.h"

typedef struct
{
    uint8_t colors[3];
} led_state_t;

void ui_draw_leds(framebuffer_t fb, led_state_t *state);

#endif // UI_LEDS_H