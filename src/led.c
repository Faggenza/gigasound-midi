#include "led.h"
#include "assert.h"

uint8_t led_buff[LED_BUFF_N] = {0};

static inline uint8_t bit_to_ws(uint8_t i, uint8_t color)
{
    if ((color >> i) & 0x01)
    {
        return 0b110;
    }
    else
    {
        return 0b100;
    }
}

void set_led(size_t index, color_t color, float brightness)
{
    assert(index < N_LED);                            // Index out of bounds
    assert(brightness >= 0.0f && brightness <= 1.0f); // Brightness out of bounds

    color.r = (uint8_t)(color.r * brightness);
    color.g = (uint8_t)(color.g * brightness);
    color.b = (uint8_t)(color.b * brightness);

    uint8_t r = color.r;
    uint8_t g = color.g;
    uint8_t b = color.b;
    size_t offset = index * 9;
    led_buff[offset + 0] = bit_to_ws(7, g) << 5 | bit_to_ws(6, g) << 2 | (bit_to_ws(5, g) >> 1);
    led_buff[offset + 1] = bit_to_ws(5, g) << 7 | bit_to_ws(4, g) << 4 | bit_to_ws(3, g) << 1 | (bit_to_ws(2, g) >> 2);
    led_buff[offset + 2] = bit_to_ws(2, g) << 6 | bit_to_ws(1, g) << 3 | bit_to_ws(0, g);

    led_buff[offset + 3] = bit_to_ws(7, r) << 5 | bit_to_ws(6, r) << 2 | (bit_to_ws(5, r) >> 1);
    led_buff[offset + 4] = bit_to_ws(5, r) << 7 | bit_to_ws(4, r) << 4 | bit_to_ws(3, r) << 1 | (bit_to_ws(2, r) >> 2);
    led_buff[offset + 5] = bit_to_ws(2, r) << 6 | bit_to_ws(1, r) << 3 | bit_to_ws(0, r);

    led_buff[offset + 6] = bit_to_ws(7, b) << 5 | bit_to_ws(6, b) << 2 | (bit_to_ws(5, b) >> 1);
    led_buff[offset + 7] = bit_to_ws(5, b) << 7 | bit_to_ws(4, b) << 4 | bit_to_ws(3, b) << 1 | (bit_to_ws(2, b) >> 2);
    led_buff[offset + 8] = bit_to_ws(2, b) << 6 | bit_to_ws(1, b) << 3 | bit_to_ws(0, b);
}
