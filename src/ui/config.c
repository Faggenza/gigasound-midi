#include "ui/config.h"
#include "gigagl.h"
#include "assets.h"
#include "ui/list_animation.h"

void ui_draw_config(framebuffer_t fb, config_state_t *state)
{
    ggl_clear_fb(fb);

    ggl_draw_rect_round_fill(fb, 12, 8 + animate(state, state->old_selection * 20, state->selected * 20), 100, 12, GGL_WHITE, 0);
    ggl_draw_text(fb, 15, 8, "Update rate", font_data, 1);
    ggl_draw_text(fb, 15, 28, "Select scales", font_data, 1);
    ggl_draw_text(fb, 15, 48, "Enter DFU", font_data, 1);
}