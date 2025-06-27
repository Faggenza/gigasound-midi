#include "gigagl.h"

void ggl_set_pixel(framebuffer_t fb, uint8_t x, uint8_t y, bool color)
{
    if (color)
        fb[y >> 3][x] |= (1 << (y & 7));
    else
        fb[y >> 3][x] &= ~(1 << (y & 7));
}

bool ggl_get_pixel(framebuffer_t fb, uint8_t x, uint8_t y)
{
    return (fb[y >> 3][x] & (1 << (y & 7))) != 0;
}

void ggl_draw_lineH(framebuffer_t fb, uint8_t x, uint8_t y, uint8_t length, bool color, uint8_t thinkness)
{
    for (uint8_t i = 0; i < length; i++)
    {
        for (uint8_t j = 0; j < thinkness; j++)
        {
            ggl_set_pixel(fb, x + i, y + j, color);
        }
    }
}
void ggl_draw_lineV(framebuffer_t fb, uint8_t x, uint8_t y, uint8_t length, bool color, uint8_t thinkness)
{
    for (uint8_t i = 0; i < length; i++)
    {
        for (uint8_t j = 0; j < thinkness; j++)
        {
            ggl_set_pixel(fb, x + j, y + i, color);
        }
    }
}

void ggl_draw_rect(framebuffer_t fb, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color)
{
    ggl_draw_lineH(fb, x, y, width, color, 1);
    ggl_draw_lineH(fb, x, y + height - 1, width, color, 1);
    ggl_draw_lineV(fb, x, y + 1, height - 2, color, 1);
    ggl_draw_lineV(fb, x + width - 1, y + 1, height - 2, color, 1);
}

void ggl_draw_rect_fill(framebuffer_t fb, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color)
{
    for (uint8_t i = 0; i < height; i++)
    {
        ggl_draw_lineH(fb, x, y + i, width, color, 1);
    }
}

void ggl_draw_rect_round(framebuffer_t fb, uint8_t x, uint8_t y, uint8_t width, uint8_t height, bool color, uint8_t delete)
{
    uint8_t thinkness = 2;
    ggl_draw_lineH(fb, x, y, width, color, thinkness);
    ggl_draw_lineH(fb, x, y + height - thinkness, width, color, thinkness);

    // Draw the vertical lines
    ggl_draw_lineV(fb, x, y + thinkness, height - thinkness * 2, color, thinkness);
    ggl_draw_lineV(fb, x + width - thinkness, y + thinkness, height - thinkness * 2, color, thinkness);

    // Remove the corners
    uint8_t negative = color ? 0 : 1;
    if (delete)
    {
        negative = color;
    }

    ggl_set_pixel(fb, x, y, negative);
    ggl_set_pixel(fb, x + 1, y, negative);
    ggl_set_pixel(fb, x, y + 1, negative);
    ggl_set_pixel(fb, x + 2, y + 2, color);

    ggl_set_pixel(fb, x + width - 1, y, negative);
    ggl_set_pixel(fb, x + width - 2, y, negative);
    ggl_set_pixel(fb, x + width - 1, y + 1, negative);
    ggl_set_pixel(fb, x + width - 3, y + 2, color);

    ggl_set_pixel(fb, x + width - 1, y + height - 1, negative);
    ggl_set_pixel(fb, x + width - 2, y + height - 1, negative);
    ggl_set_pixel(fb, x + width - 1, y + height - 2, negative);
    ggl_set_pixel(fb, x + width - 3, y + height - 3, color);

    ggl_set_pixel(fb, x, y + height - 1, negative);
    ggl_set_pixel(fb, x + 1, y + height - 1, negative);
    ggl_set_pixel(fb, x, y + height - 2, negative);
    ggl_set_pixel(fb, x + 2, y + height - 3, color);
}

void ggl_draw_sprite(framebuffer_t fb, uint8_t s, uint8_t y, ggl_sprite_t sprite)
{
    for (uint8_t j = 0; j < sprite.height; j++)
    {
        for (uint8_t i = 0; i < sprite.stride; i++)
        {
            fb[y + j][s + i] = sprite.data[j * sprite.stride + i];
        }
    }
}

void ggl_draw_icon(framebuffer_t fb, uint8_t x, uint8_t y, ggl_icon_t icon, bool invert)
{
    for (uint8_t j = 0; j < icon.height; j++)
    {
        for (uint8_t i = 0; i < icon.width; i++)
        {
            uint8_t rem = i % 8;
            uint8_t byte_index = i / 8;

            if (icon.data[j * icon.width / 8 + byte_index] & (0x80 >> rem))
            {
                if (invert)
                    ggl_set_pixel(fb, x + i, y + j, ggl_get_pixel(fb, x + i, y + j) ^ 1);
                else
                    ggl_set_pixel(fb, x + i, y + j, 1);
            }
        }
    }
}

void ggl_draw_text(framebuffer_t fb, uint8_t x, uint8_t y, const char *text, const uint8_t font[52][11], bool invert)
{
    uint8_t char_n = 0;
    while (*text)
    {
        uint8_t char_index = (uint8_t)*text;
        uint8_t i_start = char_index == 'i' ? 1 : 0;
        uint8_t i_end = char_index == 'i' ? 4 : 5;
        if (char_index == ' ')
        {
            char_n += 6; // Space character width
            text++;
            continue;
        }
        if (char_index & 0x20)
        {
            char_index -= 'a';
        }
        else
        {
            char_index -= 'A';
            char_index += 26;
        }
        for (uint8_t j = 0; j < 11; j++)
        {
            for (uint8_t i = i_start; i < i_end; i++)
            {
                if (font[char_index][j] & (0x80 >> i))
                {
                    if (invert)
                        ggl_set_pixel(fb, x + i + char_n, y + j, ggl_get_pixel(fb, x + i + char_n, y + j) ^ 1);
                    else
                        ggl_set_pixel(fb, x + i + char_n, y + j, 1);
                }
            }
        }
        char_n += i_end - i_start + 1; // Move to the next character position
        text++;
    }
}
void ggl_clear_fb(framebuffer_t fb)
{
    memset(fb, 0, HEIGHT * (WIDTH / 8));
}