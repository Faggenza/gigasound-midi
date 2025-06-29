#ifndef UI_MENU_H
#define UI_MENU_H

#include "gigagl.h"
#include "ui/list_animation.h"

typedef list_animation_t menu_state_t;

void ui_draw_menu(framebuffer_t fb, menu_state_t *state);

#endif // UI_MENU_H