#include "gigagl.h"
#include "assets/menu_side.h"
#include "assets/menu_selected.h"
#include "assets/icon_about.h"
#include "assets/icon_graph.h"
#include "assets/icon_lamp.h"
#include "assets/icon_sensitivity.h"
#include "ui/menu.h"
#include <math.h>

static const float ease_lut[] = {
    0.000000f, 0.000024f, 0.000189f, 0.000648f, 0.001512f, 0.002880f, 0.004848f, 0.007512f,
    0.010968f, 0.015312f, 0.020640f, 0.027048f, 0.034632f, 0.043488f, 0.053712f, 0.065400f,
    0.078648f, 0.093552f, 0.110208f, 0.128712f, 0.149160f, 0.171648f, 0.196272f, 0.223128f,
    0.252312f, 0.283920f, 0.318048f, 0.354792f, 0.394248f, 0.436512f, 0.481680f, 0.529848f,
    0.581112f, 0.635568f, 0.693312f, 0.754440f, 0.819048f, 0.887232f, 0.959088f, 1.000000f};

int ease_animation(uint8_t steps, uint8_t current_step, uint8_t start, uint8_t end)
{
    // Ease in-out cubic
    float ease_t = ease_lut[current_step * (sizeof(ease_lut) / sizeof(ease_lut[0]) - 1) / steps];
    return (int)((float)start + ((float)end - (float)start) * (float)ease_t + 0.5f);
    // return (int)((float)start + ((float)end - (float)start) * ((float)current_step / (float)steps));
}

void ui_draw_menu(framebuffer_t fb, menu_state_t *state)
{
    ggl_clear_fb(fb);
    ggl_draw_icon(fb, 0, 0, menu_side_icon, 0);

    int start_y = 2 + state->old_selection * (menu_side_icon.height + 1);
    int end_y = 2 + state->selected * (menu_selected_icon.height + 1);

    // Animate Y position with easing if selection changed
    const uint8_t anim_steps = 5;
    int y = end_y;

    if (state->old_selection != state->selected)
    {
        if (state->animation_frame < anim_steps)
        {
            y = ease_animation(anim_steps, state->animation_frame, start_y, end_y);
            if (y < 0)
                y = 0; // Ensure y is not negative
            if (y >= end_y)
                y = end_y; // Ensure y does not exceed end_y

            state->animation_frame++;
        }
        else
        {
            state->old_selection = state->selected;
            state->animation_frame = 0;
            y = end_y; // Ensure final position is correct
        }
    }

    ggl_draw_icon(fb, menu_side_icon.stride * 8, y, menu_selected_icon, false);

    ggl_icon_t icons[] = {
        icon_lamp_icon,
        icon_graph_icon,
        icon_sensitivity_icon,
        icon_about_icon};

    for (uint8_t i = 0; i < sizeof(icons) / sizeof(icons[0]); i++)
    {
        int icon_x = 100;
        int icon_y = 1 + i * (menu_selected_icon.height + 1);
        ggl_draw_icon(fb, icon_x, icon_y, icons[i], true);
    }
}
