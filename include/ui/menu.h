#ifndef UI_MENU_H
#define UI_MENU_H

#include "gigagl.h"

typedef struct
{
    uint8_t old_selection; // Previous selected menu item index
    uint8_t selected;      // Index of the currently selected menu item
    uint8_t animation_frame;
} menu_state_t;

void ui_draw_menu(framebuffer_t fb, menu_state_t *state);

#endif // UI_MENU_H