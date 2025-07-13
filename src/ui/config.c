#include "ui/config.h"
#include "gigagl.h"
#include "assets.h"
#include "ui/list_animation.h"
#include "config.h"
#include "scale.h"

void ui_draw_config(framebuffer_t fb, config_state_t *state)
{
    ggl_clear_fb(fb);

    ggl_draw_rect_round_fill(fb, 10, 8 + animate(state, state->old_selection * 20, state->selected * 20), 110, 12, GGL_WHITE, 0);
    ggl_draw_text(fb, 13, 8, "Show ADC", font_data, 1);
    ggl_draw_text(fb, 13, 28, "Select scales", font_data, 1);
    ggl_draw_text(fb, 13, 48, "Enter DFU", font_data, 1);
}

void ui_draw_scale_selector(framebuffer_t fb, list_animation_t *state)
{
    ggl_clear_fb(fb);

    uint8_t page = state->selected / 3;
    uint8_t start_index = page * 3;
    uint8_t end_index = start_index + 3;

    // Draw a scroll indicator on the end
    uint8_t total_pages = (END_SCALE_LIST + 2) / 3;
    uint8_t indicator_height = 50 / total_pages; // Height of each scroll indicator segment
    uint8_t indicator_y = 8 + page * indicator_height;

    // Draw the current page indicator
    ggl_draw_rect_fill(fb, 115, indicator_y, 5, indicator_height, GGL_WHITE);

    ggl_draw_rect_round_fill(fb, 19, 8 + animate(state, (state->old_selection % 3) * 20, (state->selected % 3) * 20), 90, 12, GGL_WHITE, 0);

    for (uint8_t i = start_index; i < end_index; i++)
    {
        uint8_t y_offset = 20 * (i - start_index); // Adjust y position for the current page
        if (config.scales_enabled[i])
        {
            ggl_draw_rect_fill(fb, 5, 9 + y_offset, 10, 10, GGL_WHITE);
        }
        else
        {
            ggl_draw_rect(fb, 5, 9 + y_offset, 10, 10, GGL_WHITE);
        }
        ggl_draw_text(fb, 23, 8 + y_offset, scale_to_string[i], font_data, 1);
    }
}