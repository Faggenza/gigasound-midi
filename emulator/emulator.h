#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include "gigagl.h"
#include "config.h"
#include "led.h"
#include "input.h"
#include "adc.h"

#include <raylib.h>

#define SCALE 10
#define WIDTH 128
#define HEIGH 64

uint16_t adc_buff[ADC_CHANNELS] = {4096, 4096, 4096, 4096, 4096, 4096, 4096, 4096, 4096, 4096, 4096};

static uint8_t dma_buffer[(128 * 8) + 1] = {0};
framebuffer_t *fb = (framebuffer_t *)(dma_buffer + 1); // Point to the framebuffer part of the buffer

config_t config = {
    .scales_enabled = {false},
    .joycon_calibration = {
        .calibrated = true,
    },
};

bool config_modified = false;

bool fb_updating = false;
uint8_t led_buff[LED_BUFF_N] = {0};

void set_led(size_t index, color_t color, float brightness)
{
    // printf("Setting LED %d to (%d,%d,%d) at %f%%\n", index, color.r, color.g, color.b, brightness * 100);
}

void clear_leds(void)
{
    printf("Clearing all LEDs\n");
    for (size_t i = 0; i < N_LED; i++)
    {
        set_led(i, OFF, 0.0f);
    }
}

volatile uint8_t adc_complete = false;

bool debug = false;

void DrawFb()
{
    debug = IsKeyPressed(KEY_SPACE) ? !debug : debug;
    uint8_t mouse_x = GetMouseX() / SCALE;
    uint8_t mouse_y = GetMouseY() / SCALE;
    if (debug && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        printf("Mouse: %d, %d\n", mouse_x, mouse_y);
    }

    for (uint8_t y = 0; y < HEIGHT; y++)
    {
        for (uint8_t x = 0; x < WIDTH; x++)
        {
            Color color = ggl_get_pixel(*fb, x, y) ? (Color){255, 255, 255, 0xff} : (Color){0, 0, 0, 0xff};
            if (debug && x == mouse_x && y == mouse_y)
            {
                color = RED;
            }
            DrawRectangle(x * SCALE, y * SCALE, SCALE, SCALE, color);
        }
    }

    if (debug)
    {
        for (uint8_t y = 0; y < HEIGHT / 8; y++)
        {
            for (uint8_t x = 0; x < WIDTH; x++)
            {
                DrawRectangleLinesEx((Rectangle){x * SCALE, y * SCALE * 8, SCALE, SCALE * 8}, 1, RED);
            }
        }
    }
}

void __disable_irq()
{
    printf("[NOT IMPLEMENTED] Disabling irqs..\n");
}

void breakpoint()
{
    raise(SIGINT);
}

void HAL_Init()
{
    printf("Initializing HAL");
    // Raylib init code here
    const int screenWidth = WIDTH * SCALE;
    const int screenHeight = HEIGHT * SCALE;

    InitWindow(screenWidth, screenHeight, "[gigasound] ui virtual");

    SetTargetFPS(40);
}

void SystemClock_Config()
{
    printf("[STUB] Setting up system clocks\n");
}

void MX_GPIO_Init()
{
    printf("[STUB] MX_GPIO_Init called\n");
}

void MX_DMA_Init()
{
    printf("[STUB] MX_DMA_Init called\n");
}

void MX_I2C1_Init()
{
    printf("[STUB] MX_I2C1_Init called\n");
}

void MX_ADC1_Init()
{
    printf("[STUB] MX_ADC1_Init called\n");
}

void MX_USART1_UART_Init()
{
    printf("[STUB] MX_USART1_UART_Init called\n");
}

void MX_SPI3_Init()
{
    printf("[STUB] MX_SPI3_Init called\n");
}

void MX_USB_OTG_FS_PCD_Init()
{
    printf("[STUB] MX_USB_OTG_FS_PCD_Init called\n");
}

void SSD1306_MINIMAL_init()
{
    printf("[STUB] SSD1306_MINIMAL_init called\n");
}

// Dummy SPI handle and buffer for compilation
typedef struct
{
} SPI_HandleTypeDef;

SPI_HandleTypeDef hspi3;

void HAL_SPI_Transmit(SPI_HandleTypeDef *h, void *buffer, int n, int timeout)
{
    printf("[NOT IMPLEMENTED] HAL_SPI_Transmit\n");
}

void HAL_SPI_Transmit_DMA(SPI_HandleTypeDef *h, void *buffer, int n)
{
    printf("[NOT IMPLEMENTED] HAL_SPI_Transmit_DMA\n");
}
typedef struct
{
    int role;
    int speed;
} tusb_rhport_init_t;

#define TUSB_ROLE_DEVICE 0
#define TUSB_SPEED_AUTO 0
#define BOARD_TUD_RHPORT 0

void tusb_init(int port, tusb_rhport_init_t *dev)
{
    printf("[STUB] tusb_init\n");
}
void board_init_after_tusb()
{
    printf("[STUB] board_init_after_tusb\n");
}

#define BOARD_TUD_PORT NULL

void tud_task()
{
    for (int i = 0; i < 8; i++)
    {
        int key = KEY_ONE + i; // Map keys 1 to 8
        adc_buff[i] = IsKeyDown(key) ? 800 : 4096;
    }

    EndDrawing();
    if (WindowShouldClose())
    {
        CloseWindow();
        exit(0);
    }
    BeginDrawing();
    DrawFb();
    // printf("[NOT IMPLEMENTED] tud_task\n");
}

void jump_to_bootloader()
{
    printf("Going into DFU\n");
    exit(0);
}

void SSD1306_MINIMAL_transferFramebuffer()
{
    if (fb_updating)
        return;

    fb_updating = true;

    // printf("Framebuffer transfer\n");
    EndDrawing();
    BeginDrawing();
    DrawFb();
    fb_updating = false;
}

void adc_init_dma() { printf("[NOT IMPLEMENTED] adc_init_dma\n"); }
void clear_pressed()
{
    // raise(SIGINT);
    printf("[STUB] clear_pressed\n");
}
void config_init() { printf("[NOT IMPLEMENTED] config_init\n"); }
void config_save_to_flash() { printf("[NOT IMPLEMENTED] config_save_to_flash\n"); }

void midi_discard_packet(void) { printf("[STUB] midi_discard_packet called\n"); }
void midi_mpe_init(void) { printf("[STUB] midi_mpe_init called\n"); }
void midi_set_pitch_bend_sensitivity(uint8_t sensitivity)
{
    printf("[STUB] midi_set_pitch_bend_sensitivity called with %u\n", sensitivity);
}
void midi_set_pitch_bend(uint16_t pitch)
{
    // printf("[STUB] midi_set_pitch_bend called with %u\n", pitch);
}
void midi_set_channel_pressure(uint8_t channel, uint8_t pressure) { printf("[STUB] midi_set_channel_pressure called with channel %u, pressure %u\n", channel, pressure); }
void midi_send_note_on(uint8_t channel, uint8_t note, uint8_t velocity) { printf("[STUB] midi_send_note_on called with channel %u, note %u, velocity %u\n", channel, note, velocity); }
void midi_send_note_off(uint8_t channel, uint8_t note) { printf("[STUB] midi_send_note_off called with channel %u, note %u\n", channel, note); }
void midi_send_modulation(uint16_t value) { printf("[STUB] midi_send_modulation called with value %u\n", value); }
void midi_task(void) { printf("[STUB] midi_task called\n"); }

int key_to_ray(in_key_t key)
{
    switch (key)
    {
    case PLAY:
        return KEY_SPACE;
    case STOP:
        return KEY_BACKSPACE;
    case MODE:
        return KEY_M;
    case RIGHT:
        return KEY_L;
    case LEFT:
        return KEY_H;
    case UP:
        return KEY_K;
    case DOWN:
        return KEY_J;
    }
}

uint8_t is_key_down(in_key_t key)
{
    // printf("[checking if key is down]\n");
    return IsKeyDown(key_to_ray(key));
}

void update_axis_states() {}

uint8_t was_key_pressed(in_key_t key)
{
    bool r = IsKeyPressed(key_to_ray(key));
    if (r)
        printf("reporting key %d as pressed\n", key);
    return r;
}

uint8_t knob_step()
{
    return 0;
}
