#ifndef LED_H
#define LED_H
#include <stdint.h>
#include <strings.h>

typedef union
{
    struct
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
    uint8_t rgb[3];
} color_t;

#define LED_BUTTON_BASE 11
#define LED_KNOB_BASE 0
#define LED_MODE 10
#define LED_STOP 9
#define LED_PLAY 8

#define RED   \
    (color_t) \
    {{255, 0, 0}}
#define ORANGE \
    (color_t)  \
    {{255, 140, 0}}
#define YELLOW \
    (color_t)  \
    {{255, 255, 0}}
#define L_GREEN \
    (color_t)   \
    {{0, 255, 0}}
#define TEAL  \
    (color_t) \
    {{0, 220, 180}}
#define CYAN  \
    (color_t) \
    {{0, 180, 255}}
#define SKY_BLUE \
    (color_t)    \
    {{80, 180, 255}}
#define BLUE  \
    (color_t) \
    {{0, 0, 255}}
#define PURPLE \
    (color_t)  \
    {{180, 70, 255}}
#define MAGENTA \
    (color_t)   \
    {{255, 0, 180}}
#define PINK  \
    (color_t) \
    {{255, 105, 180}}
#define OFF   \
    (color_t) \
    {{0, 0, 0}}

#define N_LED 19

// (50us / (1/(84/32)) ) / 8 bits
#define RESET_SYMBOLS_N 25

#define LED_BUFF_N (N_LED * 9) + RESET_SYMBOLS_N

extern uint8_t led_buff[LED_BUFF_N];

void set_led(size_t index, color_t color, float brightness);
void clear_leds(void);

#endif // LED_H
