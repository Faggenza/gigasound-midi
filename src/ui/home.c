#include "gigagl.h"
#include "assets/button_big.h"
#include "assets/button_small.h"
#include "ui/home.h"

uint8_t dot_sprite[3][4] = {{0, 1, 1, 0}, {1, 1, 1, 1}, {0, 1, 1, 0}};
#define BUTTON_SIZE 18
#define BUTTON_SPACING 6

void DrawDot(framebuffer_t fb, uint8_t x, uint8_t y)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            ggl_set_pixel(fb, x + j, y + i, dot_sprite[i][j]);
        }
    }
}

void DrawCentralLine(framebuffer_t fb)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        ggl_draw_lineH(fb, 2, (HEIGHT / 2) - i, 3 - i, 1, 1);
        ggl_draw_lineH(fb, 2, (HEIGHT / 2) + i, 3 - i, 1, 1);

        ggl_draw_lineH(fb, WIDTH - 5 + i, (HEIGHT / 2) - i, 3 - i, 1, 1);
        ggl_draw_lineH(fb, WIDTH - 5 + i, (HEIGHT / 2) + i, 3 - i, 1, 1);
    }
    for (uint8_t i = 1; i < 4; i++)
    {
        DrawDot(fb, 14 + i * (BUTTON_SIZE + 6), (HEIGHT / 2) - 1);
    }
}

void DrawButtonGrid(framebuffer_t fb, bool small)
{
    for (uint8_t rowstart = 10; rowstart < 38; rowstart += 27)
    {
        for (uint8_t col = 0; col < 4; col++)
        {
            if (small)
            {
                ggl_draw_sprite(fb, 2 + col * button_big_sprite.stride, rowstart, button_small_sprite);
            }
            else
            {
                ggl_draw_sprite(fb, 2 + col * button_big_sprite.stride, rowstart, button_big_sprite);
            }
        }
    }
}
