#ifndef UI_HOME_H
#define UI_HOME_H

#include <stdbool.h>
#include <stdint.h>
#include "assets.h"
#include "scale.h"

typedef struct
{
    uint8_t current_knob;
    uint8_t last_knob;
    bool stopped;
    bool playing;
    bool key_pressed[8];
    // Stores the time when the button reach pressure < p1
    uint32_t timers[8];
    scale_t scale;
    tone_t tone;
} midi_playback_state_t;

typedef struct
{
    uint8_t x;
    uint8_t y;
    bool inverted;
} stroke_position_t;

extern stroke_position_t strokes[24];

void ui_draw_home(framebuffer_t fb, midi_playback_state_t *playback_state);

#endif // UI_HOME_H