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
    scale_t scale;
    tone_t tone;
} midi_playback_state_t;

typedef struct
{
    uint8_t x;
    uint8_t y;
    bool inverted;
} stroke_position_t;

stroke_position_t strokes[24] = {
    {.x = 4, .y = 48, false},   // Do scale 0
    {.x = 8, .y = 32, true},    // Dod scale 0
    {.x = 13, .y = 48, false},  // Re scale 0
    {.x = 8, .y = 32, true},    // Red scale 0
    {.x = 22, .y = 48, false},  // Mi scale 0
    {.x = 31, .y = 48, false},  // Fa scale 0
    {.x = 35, .y = 32, true},   // Fad scale 0
    {.x = 40, .y = 48, false},  // Sol scale 0
    {.x = 45, .y = 32, true},   // Sold scale 0
    {.x = 49, .y = 48, false},  // La scale 0
    {.x = 55, .y = 32, true},   // Lad scale 0
    {.x = 58, .y = 48, false},  // Si scale 0
    {.x = 67, .y = 48, false},  // Do scale 1
    {.x = 71, .y = 32, true},   // Dod scale 1
    {.x = 76, .y = 48, false},  // Re scale 1
    {.x = 82, .y = 32, true},   // Red scale 1
    {.x = 85, .y = 48, false},  // Mi scale 1
    {.x = 94, .y = 48, false},  // Fa scale 1
    {.x = 98, .y = 32, true},   // Fad scale 1
    {.x = 103, .y = 48, false}, // Sol scale 1
    {.x = 108, .y = 32, true},  // Sold scale 1
    {.x = 112, .y = 48, false}, // La scale 1
    {.x = 118, .y = 32, true},  // Lad scale 1
    {.x = 121, .y = 48, false}, // Si scale 1
};

void ui_draw_home(framebuffer_t fb, midi_playback_state_t *playback_state)
{
    ggl_clear_fb(fb);
    ggl_draw_icon(fb, 0, 0, home_keys_icon, 0);
    ggl_draw_text(fb, 4, 4, tone_to_string[playback_state->tone], font_data, 0);
    ggl_draw_text(fb, 32, 4, scale_to_string[playback_state->scale], font_data, 0);
}