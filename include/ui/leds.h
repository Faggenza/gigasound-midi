#ifndef UI_LEDS_H
#define UI_LEDS_H

#include "gigagl.h"

typedef struct
{
    uint8_t colors[3];
    uint8_t rgb_selected;
    uint8_t led_selected;
} led_state_t;

void ui_draw_leds(framebuffer_t fb, led_state_t *state);

#endif // UI_LEDS_H