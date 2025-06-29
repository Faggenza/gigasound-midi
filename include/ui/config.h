#ifndef UI_CONFIG_H
#define UI_CONFIG_H

#include "stdint.h"
#include "gigagl.h"
#include "list_animation.h"

typedef list_animation_t config_state_t;

void ui_draw_config(framebuffer_t fb, config_state_t *state);

#endif // UI_CONFIG_H