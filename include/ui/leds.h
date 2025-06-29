#ifndef UI_LEDS_H
#define UI_LEDS_H

#include "gigagl.h"
#include "led.h"

typedef struct
{
    color_t color;
    uint8_t rgb_selected;
    uint8_t rgb_selected_old;
    uint8_t led_selected;
    uint8_t animation_frame;
} led_state_t;

void ui_draw_leds(framebuffer_t fb, led_state_t *state);

#endif // UI_LEDS_H