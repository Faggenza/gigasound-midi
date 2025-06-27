#include "gigagl.h"
#include "assets.h"
#include "ui/leds.h"
#include <math.h>
#include "ui/leds.h"

#define PROGRESS_BAR_WIDTH 101
void draw_progress_bar(framebuffer_t fb, uint8_t x, uint8_t y, uint8_t progress)
{
    ggl_draw_icon(fb, x, y, bar_icon, false);
    uint8_t filled_length = ((uint16_t)progress * PROGRESS_BAR_WIDTH) / 255;
    ggl_draw_rect_fill(fb, x + 4, y + 1, filled_length, bar_icon.height - 2, true);
}

void ui_draw_leds(framebuffer_t fb, led_state_t *state)
{
    ggl_clear_fb(fb);
    for (uint8_t i = 0; i < 3; i++)
    {
        draw_progress_bar(fb, 16, 20 + (i * 15), state->colors[i]);
    }
}
