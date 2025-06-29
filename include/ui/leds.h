#ifndef UI_LEDS_H
#define UI_LEDS_H

#include "gigagl.h"
#include "led.h"
#include "list_animation.h"

typedef struct
{
    color_t color;
    uint8_t led_selected;
    list_animation_t list;
} led_state_t;

void ui_draw_leds(framebuffer_t fb, led_state_t *state);

#endif // UI_LEDS_H