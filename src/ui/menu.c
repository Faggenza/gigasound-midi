#include "gigagl.h"
#include "assets/menu_side.h"
#include "assets/menu_selected.h"
#include "assets/icon_about.h"
#include "assets/icon_graph.h"
#include "assets/icon_lamp.h"
#include "assets/icon_sensitivity.h"
#include "ui/menu.h"
#include <math.h>
#include "assets/font.h"

int ease_animation(int steps, int current_step, int start, int end)
{
    return start + (((end - start) * current_step) / steps);
}

void ui_draw_menu(framebuffer_t fb, menu_state_t *state)
{
    ggl_clear_fb(fb);
    ggl_draw_icon(fb, 0, 0, menu_side_icon, 0);

    int start_y = 2 + state->old_selection * (menu_selected_icon.height + 1);
    int end_y = 2 + state->selected * (menu_selected_icon.height + 1);

    // Animate Y position with easing if selection changed
    const uint8_t anim_steps = 5;
    int y = end_y;

    if (state->old_selection != state->selected)
    {
        if (state->animation_frame < anim_steps)
        {
            y = ease_animation(anim_steps, state->animation_frame, start_y, end_y);
            // if (y < 0)
            //     y = 0; // Ensure y is not negative
            // if (y >= end_y)
            //     y = end_y; // Ensure y does not exceed end_y

            state->animation_frame++;
        }
        else
        {
            state->old_selection = state->selected;
            state->animation_frame = 0;
            y = end_y; // Ensure final position is correct
        }
    }

    ggl_draw_icon(fb, menu_side_icon.width, y, menu_selected_icon, false);

    struct
    {
        char *text;
        ggl_icon_t icon;
    } entries[] = {
        {"LEDs", icon_lamp_icon},
        {"Curve", icon_graph_icon},
        {"Sensitivity", icon_sensitivity_icon},
        {"About", icon_about_icon}};

    for (uint8_t i = 0; i < sizeof(entries) / sizeof(entries[0]); i++)
    {
        int icon_x = 100;
        int icon_y = 1 + i * (menu_selected_icon.height + 1);
        ggl_draw_icon(fb, icon_x, icon_y, entries[i].icon, true);
        ggl_draw_text(fb, menu_side_icon.width + 6, icon_y + 3, entries[i].text, font_data, true);
    }
}
