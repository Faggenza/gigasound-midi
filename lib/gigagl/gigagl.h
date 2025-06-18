#ifndef GIGASOUND_H
#define GIGASOUND_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define WIDTH 128
#define HEIGHT 64

#define GGL_BLACK false
#define GGL_WHITE true

typedef uint8_t framebuffer_t[HEIGHT / 8][WIDTH];

typedef struct
{
    const uint8_t height;
    const uint8_t stride; // Number of bytes per row in the packed data
    const uint8_t *data;  // Packed data, 1 bit per pixel
} ggl_sprite_t;

typedef struct
{
    const uint8_t stride;
    const uint8_t height;
    const uint8_t *data;
} ggl_icon_t;

void ggl_set_pixel(framebuffer_t fb, uint8_t x, uint8_t y, bool color);
bool ggl_get_pixel(framebuffer_t fb, uint8_t x, uint8_t y);
void ggl_draw_lineH(framebuffer_t fb, uint8_t x, uint8_t y, uint8_t length, bool color, uint8_t thinkness);
void ggl_draw_lineV(framebuffer_t fb, uint8_t x, uint8_t y, uint8_t length, bool color, uint8_t thinkness);
void ggl_draw_rect_round(framebuffer_t fb, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color, uint8_t delete);
void ggl_draw_sprite(framebuffer_t fb, uint8_t x, uint8_t y, ggl_sprite_t sprite);
void ggl_draw_icon(framebuffer_t fb, uint8_t x, uint8_t y, ggl_icon_t icon, bool invert);
void ggl_clear_fb(framebuffer_t fb);
void ggl_draw_rect(framebuffer_t fb, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color);
void ggl_draw_rect_fill(framebuffer_t fb, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color);

#endif // GIGASOUND_H