
#include "stdio.h"
#include "led.h"
#include "midi.h"
#include "adc.h"
#include "input.h"
#include "gigagl.h"
#include "ui/menu.h"
#include "assets.h"
#include "calibrate.h"
#include <stdlib.h>
#include "ui/leds.h"
#include "config.h"
#include "scale.h"

#ifdef EMULATOR
#include "emulator.h"
#else
#include "main.h"
#include "uart.h"
#include "init.h"
#include "bsp/board_api.h"
#include "bootloader.h"
#include "ssd1306.h"
#include "tusb.h"
#endif

void Error_Handler(void)
{
  __disable_irq();
#ifdef EMULATOR
  breakpoint();
#else
  asm("bkpt 0");
#endif
}

typedef enum
{
  MIDI_PLAYBACK,
  MENU_SCREEN,
  LED_SCREEN,
  CURVE_SCREEN,
  SENSITIVITY_SCREEN,
  ABOUT_SCREEN,
  COLOR_SCREEN,
} state_t;

state_t state = MIDI_PLAYBACK;

// Good artists copy, great artists steal -Pablo Picasso
uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
  if ((in_max - in_min) == 0)
    return 0;
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// This function must be called every time you are entering a busy loop
// It updates the input and keeps the usb connection alive
void loop_task()
{
  tud_task();
  if (adc_complete)
  {
    // adc_print_buffer();
    adc_complete = 0;
    update_axis_states();
  }
}

typedef enum
{
  NO_ANIMATION,
  BACK,
  FORWARD,
} animation_direction_t;

animation_direction_t dir = NO_ANIMATION;

framebuffer_t backbuffer = {0};

// Animate the transition from backbuffer to frontbuffer, swipe right to left
// https://easings.net/#easeOutCubic
// Array.from({length: 25}, (v,i) => Math.floor(easeOutCubic((i/25)) * 128))
uint8_t keyframes[] = {0, 23, 44, 61, 76,
                       89, 99, 107, 114, 118,
                       122, 124, 125, 126, 126, 128};

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

void animate_switch()
{
  switch (dir)
  {
  case NO_ANIMATION:
    memcpy(fb, &backbuffer, sizeof(framebuffer_t));
    while (fb_updating)
      loop_task();
    SSD1306_MINIMAL_transferFramebuffer();
    break;
  case BACK:
    uint8_t prev_delta = 0;
    for (uint8_t i = 0; i < sizeof(keyframes) - 2; i++)
    {
      while (fb_updating)
        loop_task();

      uint8_t delta = -((int)keyframes[sizeof(keyframes) - 3 - i] - 128);

      // Shift fwd the current fb by delta pixels
      for (uint8_t x = WIDTH - 1; x >= delta; x--)
      {
        // Copy the column
        for (uint8_t y = 0; y < (HEIGHT >> 3); y++)
        {
          (*fb)[y][x] = (*fb)[y][x - (delta - prev_delta)];
        }
      }

      prev_delta = delta;
      for (uint8_t x = 0; x < delta && x < WIDTH; x++)
      {
        // Copy the column
        for (uint8_t y = 0; y < (HEIGHT >> 3); y++)
        {
          (*fb)[y][x] = backbuffer[y][x];
        }
      }
      SSD1306_MINIMAL_transferFramebuffer();
    }
    break;
  case FORWARD:
    for (uint8_t i = 0; i < sizeof(keyframes); i++)
    {
      while (fb_updating)
        loop_task();

      uint8_t delta = keyframes[i];
      for (uint8_t x = 0; x < delta && x < WIDTH; x++)
      {
        // Copy the column
        for (uint8_t y = 0; y < HEIGHT; y++)
        {
          ggl_set_pixel(*fb, WIDTH - 1 - x, y, ggl_get_pixel(backbuffer, delta - 1 - x, y));
        }
      }
      SSD1306_MINIMAL_transferFramebuffer();
    }
    break;
  }
  ggl_clear_fb(backbuffer);
}

int main(void)
{

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_SPI3_Init();
  MX_USB_OTG_FS_PCD_Init();

  config_init();

  SSD1306_MINIMAL_init();

  adc_init_dma();
  // Turn all LEDs off
  for (size_t i = 0; i < N_LED; i++)
  {
    set_led(i, OFF, 0.0f);
  }

  HAL_SPI_Transmit(&hspi3, led_buff, LED_BUFF_N, 1000);
  HAL_SPI_Transmit(&hspi3, led_buff, LED_BUFF_N, 1000);

  HAL_SPI_Transmit_DMA(&hspi3, led_buff, LED_BUFF_N);

  // init device stack on configured roothub port
  tusb_rhport_init_t dev_init = {
      .role = TUSB_ROLE_DEVICE,
      .speed = TUSB_SPEED_AUTO};
  tusb_init(BOARD_TUD_RHPORT, &dev_init);

  if (board_init_after_tusb)
  {
    board_init_after_tusb();
  }

  midi_playback_state_t playback_state = {
      .current_knob = 0,
      .last_knob = 0,
      .stopped = false,
      .playing = false,
      .tone = DO,
      .scale = MAJOR,
      .key_pressed = {false},
  };

  menu_state_t menu_state = {
      .old_selection = 0,
      .selected = 0,
      .animation_frame = 0,
  };

  led_state_t selected_led = {
      .color = OFF,
      .rgb_selected = 0,
      .led_selected = 0,
  };

  // TODO: alla primissima inizializzazione si dovrebbe calibrare il joystick
  while (1)
  {
    // Every time we change state, we want to clear the pressed buttons
    // To avoid handling the stame inputs multiple time
    clear_pressed();
    clear_leds();
    printf("Switching to state %d\n", state);
    switch (state)
    {
    case MIDI_PLAYBACK:
      ggl_draw_icon(backbuffer, 0, 0, home_keys_icon, 0);
      ggl_draw_text(backbuffer, 30, 4, tone_to_string[playback_state.tone], font_data, 0);
      ggl_draw_text(backbuffer, 50, 4, scale_to_string[playback_state.scale], font_data, 0);

      animate_switch();
      playback_state.last_knob = 255;

      while (true)
      {
        loop_task();
        playback_state.current_knob = knob_step();
        if (playback_state.last_knob != playback_state.current_knob)
        {
          // Clear all the previous notes
          for (uint8_t i = 0; i < 8; i++)
          {
            midi_send_note_off(i, button_to_midi(playback_state.last_knob, MAJOR, DO, i));
            playback_state.key_pressed[i] = false;
          }
          playback_state.last_knob = playback_state.current_knob;
          // Update the knob LEDs
          for (uint8_t i = 0; i < 8; i++)
          {
            set_led(i, OFF, 0.0f);
          }
          set_led(LED_KNOB_BASE + playback_state.current_knob, config.color[LED_KNOB_BASE], 1.0f);
        }

        uint16_t threshold = 3600;

        for (uint8_t i = 0; i < 8; i++)
        {
          // Quirk for the second button that is broken
          if (i == 1)
          {
            threshold = 1000;
          }
          else
          {
            threshold = 3600;
          }

          if (adc_buff[i] < threshold && playback_state.key_pressed[i] == false)
          {
            playback_state.key_pressed[i] = true;
            midi_send_note_on(i, button_to_midi(playback_state.current_knob, MAJOR, DO, i), 127);
            set_led(LED_BUTTON_BASE + i, config.color[LED_BUTTON_BASE + i], 0.1f);
          }
          else if (adc_buff[i] >= threshold && playback_state.key_pressed[i] == true)
          {
            playback_state.key_pressed[i] = false;
            midi_send_note_off(i, button_to_midi(playback_state.current_knob, MAJOR, DO, i));
            set_led(LED_BUTTON_BASE + i, OFF, 0.0f);
          }

          if (playback_state.key_pressed[i])
          {
            midi_set_channel_pressure(i, (threshold - adc_buff[i]) >> 4);
            set_led(LED_BUTTON_BASE + i, config.color[LED_BUTTON_BASE + i], ((4096 - adc_buff[i]) >> 4) / 500.0f);
          }
        }
        uint16_t p = map(adc_buff[ADC_AXIS_Y], config.joycon_calibration.y_max + 40, config.joycon_calibration.y_min - 40, 0, 16384);
        midi_set_pitch_bend(p);

        uint16_t m = map(abs((int)adc_buff[ADC_AXIS_X] - (config.joycon_calibration.x_max - config.joycon_calibration.x_min)), 0, (config.joycon_calibration.x_max - config.joycon_calibration.x_min), 0, 16384);

        midi_send_modulation(m);

        if (playback_state.current_knob == 7)
        {
          jump_to_bootloader();
        }

        if (was_key_pressed(PLAY))
        {
          playback_state.playing = !playback_state.playing;
          set_led(LED_PLAY, playback_state.playing ? config.color[LED_PLAY] : OFF, 1.0f);
        }
        if (was_key_pressed(STOP))
        {
          playback_state.stopped = !playback_state.stopped;
          set_led(LED_STOP, playback_state.stopped ? config.color[LED_STOP] : OFF, 1.0f);
        }
        if (was_key_pressed(MODE))
        {
          state = MENU_SCREEN;
          dir = FORWARD;
          break;
        }
      }
      break;
    case MENU_SCREEN:
      ui_draw_menu(backbuffer, &menu_state);
      animate_switch();
      while (1)
      {
        loop_task();
        if (was_key_pressed(MODE) || was_key_pressed(LEFT))
        {
          config_save_to_flash();
          state = MIDI_PLAYBACK;
          dir = BACK;
          break;
        }
        if (was_key_pressed(UP))
        {
          menu_state.old_selection = menu_state.selected;
          menu_state.selected = (menu_state.selected + 3) % 4;
          menu_state.animation_frame = 0;
          for (size_t i = 0; i < 6; i++)
          {
            while (fb_updating)
              loop_task();
            ui_draw_menu(*fb, &menu_state);
            SSD1306_MINIMAL_transferFramebuffer();
          }
        }
        else if (was_key_pressed(DOWN))
        {
          menu_state.old_selection = menu_state.selected;
          menu_state.selected = (menu_state.selected + 1) % 4;
          menu_state.animation_frame = 0;
          for (size_t i = 0; i < 6; i++)
          {
            while (fb_updating)
              loop_task();
            ui_draw_menu(*fb, &menu_state);
            SSD1306_MINIMAL_transferFramebuffer();
          }
        }
        else if (was_key_pressed(RIGHT))
        {
          dir = FORWARD;
          switch (menu_state.selected)
          {
          case 0:
            state = LED_SCREEN;
            break;
          case 2:
            state = SENSITIVITY_SCREEN;
          default:
            break;
          }
          break;
        }
      }
      break;
    case LED_SCREEN:
      ggl_draw_text(backbuffer, 30, 28, "Click any axis", font_data, 0);
      ggl_draw_text(backbuffer, 30, 40, "to select LED", font_data, 0);
      animate_switch();
      bool key_pressed[8] = {false};
      uint8_t current_knob = knob_step();
      uint8_t last_knob = current_knob;

      while (1)
      {
        loop_task();
        if (was_key_pressed(LEFT))
        {
          state = MENU_SCREEN;
          dir = BACK;
          break;
        }

        uint16_t threshold = 3600;

        for (uint8_t i = 0; i < 8; i++)
        {
          if (i == 1)
          {
            threshold = 1000;
          }
          else
          {
            threshold = 3000;
          }

          if (adc_buff[i] < threshold && key_pressed[i] == false)
          {
            selected_led.led_selected = LED_BUTTON_BASE + i;
            selected_led.color = config.color[selected_led.led_selected];
            selected_led.rgb_selected = 0;
            state = COLOR_SCREEN;
            dir = FORWARD;
            break;
          }
        }

        if ((current_knob = knob_step()) != last_knob)
        {
          last_knob = current_knob;
          selected_led.led_selected = LED_KNOB_BASE;
          selected_led.color = config.color[selected_led.led_selected];
          selected_led.rgb_selected = 0;
          state = COLOR_SCREEN;
          dir = FORWARD;
          break;
        }

        in_key_t keys_to_check[] = {PLAY, STOP, MODE};
        uint8_t keys_leds[] = {LED_PLAY, LED_STOP, LED_MODE};
        bool flag = false;
        for (size_t i = 0; i < sizeof(keys_to_check) / sizeof(keys_to_check[0]); i++)
        {
          if (was_key_pressed(keys_to_check[i]))
          {
            selected_led.led_selected = keys_leds[i];
            selected_led.color = config.color[selected_led.led_selected];
            selected_led.rgb_selected = 0;
            flag = true;
          }
        }
        if (flag)
        {
          state = COLOR_SCREEN;
          dir = FORWARD;
          break;
        }
      }
      break;
    case COLOR_SCREEN:
      ui_draw_leds(backbuffer, &selected_led);
      animate_switch();

      while (1)
      {
        loop_task();

        if (was_key_pressed(UP))
        {
          selected_led.rgb_selected = (selected_led.rgb_selected + 2) % 3;
        }
        else if (was_key_pressed(DOWN))
        {
          selected_led.rgb_selected = (selected_led.rgb_selected + 1) % 3;
        }
        else if (was_key_pressed(RIGHT))
        {
          config_modified = true;
          selected_led.color.rgb[selected_led.rgb_selected] += 5;
        }
        else if (was_key_pressed(LEFT))
        {
          config_modified = true;
          selected_led.color.rgb[selected_led.rgb_selected] -= 5;
        }
        if (selected_led.led_selected == LED_KNOB_BASE)
        {
          for (size_t i = 0; i < 8; i++)
          {
            set_led(LED_KNOB_BASE + i, selected_led.color, 0.1f);
          }
        }
        else
        {
          set_led(selected_led.led_selected, selected_led.color, 0.1f);
        }
        if (was_key_pressed(STOP))
        {
          config.color[selected_led.led_selected] = selected_led.color;
          state = LED_SCREEN;
          dir = BACK;
          break;
        }

        if (!fb_updating)
        {
          ui_draw_leds(*fb, &selected_led);
          SSD1306_MINIMAL_transferFramebuffer();
        }
      }
      break;
    case SENSITIVITY_SCREEN:
      ggl_draw_text(backbuffer, 30, 18, "Calibrating", font_data, 0);
      ggl_draw_text(backbuffer, 2, 30, "Press STOP when done", font_data, 0);
      animate_switch();

      config.joycon_calibration = calibrate_joycon(adc_buff);
      config_modified = true;
      state = MENU_SCREEN;
      dir = BACK;
      break;
    case ABOUT_SCREEN:
      break;
    default:
      break;
    }
  }
}

// Invoked when device is mounted
void tud_mount_cb(void)
{
  midi_mpe_init();
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void)remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}