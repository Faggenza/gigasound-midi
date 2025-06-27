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

int ease_animation_led(int steps, int current_step, int start, int end)
{
    return start + (((end - start) * current_step) / steps);
}

void ui_draw_leds(framebuffer_t fb, led_state_t *state)
{
    // Calculate start and end positions for animation
    int start_y = 20 + (state->rgb_selected_old * 15);
    int end_y = 20 + (state->rgb_selected * 15);
    int selection_y = end_y;

    const uint8_t anim_steps = 5;

    ggl_clear_fb(fb);
    if (state->rgb_selected_old != state->rgb_selected)
    {
        if (state->animation_frame < anim_steps)
        {
            selection_y = ease_animation_led(anim_steps, state->animation_frame, start_y, end_y);
            state->animation_frame++;
        }
        else
        {
            state->rgb_selected_old = state->rgb_selected;
            state->animation_frame = 0;
            selection_y = end_y;
        }
    }

    // Draw the selection rectangle around the selected letter
    ggl_draw_rect_round_fill(fb, 5, selection_y - 1, 12, 12, true, false);

    // Draw the progress bars and text
    for (uint8_t i = 0; i < 3; i++)
    {
        draw_progress_bar(fb, 16, 20 + (i * 15), state->colors[i]);
        ggl_draw_text(fb, 9, 20 + (i * 15), i == 0 ? "R" : i == 1 ? "G"
                                                                  : "B",
                      font_data, true);
    }
}
