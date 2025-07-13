#ifndef UI_CONFIG_H
#define UI_CONFIG_H

#include "stdint.h"
#include "gigagl.h"
#include "list_animation.h"

typedef enum
{
    CONFIG_SHOW_ADC = 0,
    CONFIG_SCALE,
    CONFIG_DFU,
} config_states_enum;

typedef list_animation_t config_state_t;

void ui_draw_config(framebuffer_t fb, config_state_t *state);
void ui_draw_scale_selector(framebuffer_t fb, list_animation_t *state);

#endif // UI_CONFIG_H