
#include "main.h"
#include "stdio.h"
#include "uart.h"
#include "init.h"
#include "led.h"
#include "bsp/board_api.h"
#include "tusb.h"
#include "midi.h"
#include "bootloader.h"
#include "ssd1306.h"
#include "adc.h"
#include "input.h"
#include "gigagl.h"
#include "ui/menu.h"
#include "assets.h"
#include "calibrate.h"
#include <stdlib.h>
#include "ui/leds.h"
#include "config.h"

void Error_Handler(void)
{
  __disable_irq();
  asm volatile("bkpt 0");
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

led_state_t selected_led = {
    .colors = {0, 0, 0},
    .rgb_selected = 0,
    .led_selected = 0,
};

// Good artists copy, great artists steal -Pablo Picasso
uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
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
    set_led(i, (color_t){0, 0, 0}, 0.0f);
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
  menu_state_t menu_state = {
      .old_selection = 0,
      .selected = 0,
      .animation_frame = 0,
  };

  bool stopped = false;
  bool playing = false;
  bool key_pressed[8] = {false};

  uint8_t last_knob = 0;
  state_t state = MIDI_PLAYBACK;

  joycon_calibration c = {
      .x_min = 0,
      .x_max = 4096,
      .y_min = 0,
      .y_max = 4096,
  };

  while (1)
  {
    tud_task();
    if (adc_complete)
    {
      // adc_print_buffer();
      adc_complete = 0;
      update_axis_states();
    }
    switch (state)
    {
    case MIDI_PLAYBACK:
      uint8_t current_knob = knob_step();
      if (last_knob != current_knob)
      {
        for (uint8_t i = 0; i < 8; i++)
        {
          midi_send_note_off(i, last_knob * 12 + 40 + i);
          key_pressed[i] = false;
        }
        last_knob = current_knob;
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
          threshold = 3600;
        }

        if (adc_buff[i] < threshold && key_pressed[i] == false)
        {
          key_pressed[i] = true;
          midi_send_note_on(i, current_knob * 12 + 40 + i, 127);
          set_led(LED_BUTTON_BASE + i, GREEN, 0.1f);
        }
        else if (adc_buff[i] >= threshold && key_pressed[i] == true)
        {
          key_pressed[i] = false;
          midi_send_note_off(i, current_knob * 12 + 40 + i);
          set_led(LED_BUTTON_BASE + i, GREEN, 0.0f);
        }

        if (key_pressed[i])
        {
          midi_set_channel_pressure(i, (threshold - adc_buff[i]) >> 4);
          set_led(LED_BUTTON_BASE + i, GREEN, ((4096 - adc_buff[i]) >> 4) / 500.0f);
        }
      }
      uint16_t p = map(adc_buff[ADC_AXIS_Y], c.y_max + 40, c.y_min - 40, 0, 16384);
      midi_set_pitch_bend(p);

      uint16_t m = map(abs((int)adc_buff[ADC_AXIS_X] - (c.x_max - c.x_min)), 0, (c.x_max - c.x_min), 0, 16384);

      midi_send_modulation(m);

      // printf("pitch: %u\n", p);
      if (current_knob == 7)
      {
        jump_to_bootloader();
      }
      for (uint8_t i = 0; i < 8; i++)
      {
        if (i <= current_knob)
        {
          set_led(i, RED, 0.1f);
        }
        else
        {
          set_led(i, BLACK, 0.0f);
        }
      }
      if (was_key_pressed(PLAY))
      {
        playing = !playing;
        set_led(8, playing ? GREEN : BLACK, 0.1f);
      }
      if (was_key_pressed(STOP))
      {
        stopped = !stopped;
        set_led(9, stopped ? RED : BLACK, 0.1f);
      }

      // if (was_key_pressed(MODE))
      // {
      //   mode = !mode;
      //   set_led(10, mode ? BLUE : BLACK, 0.1f);
      // }

      // if (!fb_updating)
      // {
      //   ggl_clear_fb(*fb);
      //   ggl_draw_text(*fb, 50, 28, "HOME", font_data, 0);
      //   SSD1306_MINIMAL_transferFramebuffer();
      // }

      if (was_key_pressed(MODE))
      {
        state = MENU_SCREEN;
        while (fb_updating)
          ;
        ui_draw_menu(*fb, &menu_state);
        SSD1306_MINIMAL_transferFramebuffer();
        clear_pressed();
      }

      break;
    case MENU_SCREEN:
      if (was_key_pressed(MODE) || was_key_pressed(LEFT))
      {
        config_save_to_flash();
        state = MIDI_PLAYBACK;
      }
      if (was_key_pressed(UP))
      {
        menu_state.old_selection = menu_state.selected;
        menu_state.selected = (menu_state.selected + 3) % 4;
        menu_state.animation_frame = 0;
        for (size_t i = 0; i < 6; i++)
        {
          while (fb_updating)
            ;
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
            ;
          ui_draw_menu(*fb, &menu_state);
          SSD1306_MINIMAL_transferFramebuffer();
        }
      }
      else if (was_key_pressed(RIGHT))
      {
        switch (menu_state.selected)
        {
        case 0:
          last_knob = knob_step();
          state = LED_SCREEN;
          break;
        case 2:
          state = SENSITIVITY_SCREEN;
        default:
          break;
        }
      }
      break;
    case LED_SCREEN:
      if (was_key_pressed(LEFT) || was_key_pressed(STOP))
      {
        state = MENU_SCREEN;
        while (fb_updating)
          ;
        ui_draw_menu(*fb, &menu_state);
        SSD1306_MINIMAL_transferFramebuffer();
      }

      if (!fb_updating)
      {
        ggl_clear_fb(*fb);
        ggl_draw_text(*fb, 30, 28, "Click any axis", font_data, 0);
        ggl_draw_text(*fb, 30, 40, "to select LED", font_data, 0);
        SSD1306_MINIMAL_transferFramebuffer();

        if ((current_knob = knob_step()) != last_knob)
        {
          last_knob = current_knob;
          selected_led.led_selected = 0;
          memcpy(selected_led.colors, config.color[0], sizeof(config.color[0]));
          selected_led.rgb_selected = 0;
          state = COLOR_SCREEN;
          printf("Selected LED: %u\n", current_knob);
        }
      }
      break;
    case COLOR_SCREEN:
      if (was_key_pressed(STOP))
      {
        last_knob = knob_step();
        state = LED_SCREEN;
        while (fb_updating)
          ;
      }

      while (fb_updating)
        ;
      ui_draw_leds(*fb, &selected_led);
      SSD1306_MINIMAL_transferFramebuffer();
      if (was_key_pressed(UP))
      {
        selected_led.rgb_selected = (selected_led.rgb_selected + 2) % 3;
      }
      else if (was_key_pressed(DOWN))
      {
        selected_led.rgb_selected = (selected_led.rgb_selected + 1) % 3;
      }
      else if (is_key_down(RIGHT))
      {
        config_modified = true;
        selected_led.colors[selected_led.rgb_selected] += 1;
        for (size_t i = 0; i < 8; i++)
        {
          set_led(LED_KNOB_BASE + i, (color_t){selected_led.colors[0], selected_led.colors[1], selected_led.colors[2]}, 1.0f);
        }
        if (selected_led.colors[selected_led.rgb_selected] > 255)
          selected_led.colors[selected_led.rgb_selected] = 255;
      }
      else if (is_key_down(LEFT))
      {
        config_modified = true;
        selected_led.colors[selected_led.rgb_selected] -= 1;
        set_led(selected_led.led_selected, (color_t){selected_led.colors[0], selected_led.colors[1], selected_led.colors[2]}, 1.0f);
        if (selected_led.colors[selected_led.rgb_selected] < 0)
          selected_led.colors[selected_led.rgb_selected] = 0;

        break;
      case SENSITIVITY_SCREEN:
        while (fb_updating)
          ;
        ggl_clear_fb(*fb);
        ggl_draw_text(*fb, 30, 28, "Calibrating", font_data, 0);
        SSD1306_MINIMAL_transferFramebuffer();

        c = calibrate_joycon(adc_buff);
        ggl_clear_fb(*fb);
        state = MENU_SCREEN;
        while (fb_updating)
          ;
        ui_draw_menu(*fb, &menu_state);
        SSD1306_MINIMAL_transferFramebuffer();
        break;
      case ABOUT_SCREEN:
        break;
      default:
        break;
      }
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