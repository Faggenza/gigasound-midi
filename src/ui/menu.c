#include "gigagl.h"
#include "assets.h"
#include "ui/menu.h"
#include <math.h>
#include "ui/list_animation.h"

void ui_draw_menu(framebuffer_t fb, menu_state_t *state)
{
    ggl_clear_fb(fb);
    ggl_draw_icon(fb, 0, 0, menu_side_icon, 0);

    int start_y = 2 + state->old_selection * (menu_selected_icon.height + 1);
    int end_y = 2 + state->selected * (menu_selected_icon.height + 1);

    ggl_draw_icon(fb, menu_side_icon.width, animate(state, start_y, end_y), menu_selected_icon, false);

    struct
    {
        char *text;
        ggl_icon_t icon;
    } entries[] = {
        {"LEDs", icon_lamp_icon},
        {"Config", icon_gear_icon},
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
