
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "led.h"
#include "midi.h"
#include "adc.h"
#include "input.h"
#include "gigagl.h"
#include "assets.h"
#include "calibrate.h"
#include "config.h"
#include "scale.h"
#include "ui/menu.h"
#include "ui/leds.h"
#include "ui/config.h"
#include "ui/home.h"

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
  CONFIG_SCREEN,
  SENSITIVITY_SCREEN,
  ABOUT_SCREEN,
  COLOR_SCREEN,
  SCALE_SCREEN,
  ADC_SCREEN,
} state_t;

state_t state = MIDI_PLAYBACK;

// Good artists copy, great artists steal -Pablo Picasso
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

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

  HAL_Delay(50); // Wait for inputs to stabilize
  if (is_key_down(MODE))
  {
    jump_to_bootloader();
  }

  config_init();

  SSD1306_MINIMAL_init();

  adc_init_dma();

  // Turn all LEDs off and start DMA
  clear_leds();
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
      .timers = {0},
  };

  menu_state_t menu_state = {
      .old_selection = 0,
      .selected = 0,
      .animation_frame = 0,
  };

  led_state_t selected_led = {
      .color = OFF,
      .led_selected = 0,
      .list = {0},
  };

  config_state_t config_state = {
      .selected = 0,
      .animation_frame = 0,
      .old_selection = 0,
  };

  list_animation_t scale_select_state = {
      .selected = 0,
      .animation_frame = 0,
      .old_selection = 0,
  };

  // Calibrate the joystick at very first bootup
  if (config.joycon_calibration.calibrated == false)
  {
    state = SENSITIVITY_SCREEN;
  }

  while (1)
  {
    // Every time we change state, we want to clear the pressed buttons
    // To avoid handling the stame inputs multiple time
    clear_pressed();
    clear_leds();
    switch (state)
    {
    case MIDI_PLAYBACK:
      // So if we have deselected the scale, we want to select the first available one

      for (uint8_t i = 0; i < END_SCALE_LIST; i++)
      {
        if (config.scales_enabled[i])
        {
          playback_state.scale = i;
          break;
        }
      }

      ui_draw_home(backbuffer, &playback_state);
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
            midi_send_note_off(i, button_to_midi(playback_state.last_knob, playback_state.scale, playback_state.tone, i));
            playback_state.key_pressed[i] = false;
            playback_state.timers[i] = 0;
            set_led(i, OFF, 0.0f);
          }
          playback_state.last_knob = playback_state.current_knob;
          // Update the knob LEDs
          for (uint8_t i = 0; i < 8; i++)
          {
            set_led(i, OFF, 0.0f);
          }
          set_led(LED_KNOB_BASE + playback_state.current_knob, config.color[LED_KNOB_BASE], 1.0f);
        }

        // Calculate velocity based on two thresholds:
        uint16_t threshold_p1 = 3800;
        uint16_t threshold_p2 = 3600;

        for (uint8_t i = 0; i < 8; i++)
        {
          // Quirk for the second button that is broken
          if (i == 1)
          {
            threshold_p1 = 1000;
            threshold_p2 = 850;
          }
          else
          {
            threshold_p1 = 3800;
            threshold_p2 = 3600;
          }

          if (adc_buff[i] < threshold_p2 && playback_state.key_pressed[i] == false)
          {
            playback_state.key_pressed[i] = true;
            uint8_t note = button_to_midi(playback_state.current_knob, playback_state.scale, playback_state.tone, i);

            uint8_t velocity = 127;
            uint32_t t2 = HAL_GetTick();
            uint32_t t1 = playback_state.timers[i];
            // If t1 == 0 it means the user pressed so fast, the firmware didn't have time
            // to register t1, so we leave max velocity
            uint16_t dt = t2 - t1;
            if (t1 != 0 && dt != 0 && i != 1)
            {
              uint16_t distance = (threshold_p1 - adc_buff[i]) << 4;
              uint16_t v = distance / dt;

              velocity = MIN(map(v, 0, 200, 0, 127), 127);
            }

            midi_send_note_on(i, note, velocity);
            set_led(LED_BUTTON_BASE + i, config.color[LED_BUTTON_BASE + i], 0.1f);

            // Check if it's a black key (sharp/flat) using standard MIDI convention
            uint8_t note_in_octaves = note - playback_state.current_knob * 12;
            ggl_draw_rect_fill(*fb, strokes[note_in_octaves].x, strokes[note_in_octaves].y, strokes[note_in_octaves].inverted ? 3 : 4, 8, strokes[note_in_octaves].inverted);
          }
          else if (adc_buff[i] < threshold_p1 && playback_state.timers[i] == 0 && playback_state.key_pressed[i] == false)
          {
            playback_state.timers[i] = HAL_GetTick();
          }
          else if (adc_buff[i] >= threshold_p1 && playback_state.key_pressed[i] == true)
          {
            playback_state.key_pressed[i] = false;
            playback_state.timers[i] = 0;

            uint8_t note = button_to_midi(playback_state.current_knob, playback_state.scale, playback_state.tone, i);

            midi_send_note_off(i, note);
            set_led(LED_BUTTON_BASE + i, OFF, 0.0f);

            uint8_t note_in_octaves = note - playback_state.current_knob * 12;
            ggl_draw_rect_fill(*fb, strokes[note_in_octaves].x, strokes[note_in_octaves].y, strokes[note_in_octaves].inverted ? 3 : 4, 8, !strokes[note_in_octaves].inverted);
          }

          if (playback_state.key_pressed[i])
          {
            midi_set_channel_pressure(i, MIN((threshold_p1 - adc_buff[i]) >> 4, 127));
            set_led(LED_BUTTON_BASE + i, config.color[LED_BUTTON_BASE + i], ((4096 - adc_buff[i]) >> 4) / 500.0f);
          }
        }
        uint16_t p = map(adc_buff[ADC_AXIS_Y], config.joycon_calibration.y_max + 40, config.joycon_calibration.y_min - 40, 0, 16384);
        midi_set_pitch_bend(p);

        if (adc_buff[ADC_AXIS_X] < (config.joycon_calibration.x_max - config.joycon_calibration.x_min - 200))
        {
          uint16_t m = map(adc_buff[ADC_AXIS_X], (config.joycon_calibration.x_max - config.joycon_calibration.x_min) - 200, 0, 0, 16384);
          midi_send_modulation(m);
        }

        if (was_key_pressed(MODE))
        {
          // Clear all the previous notes
          for (uint8_t i = 0; i < 8; i++)
          {
            midi_send_note_off(i, button_to_midi(playback_state.last_knob, playback_state.scale, playback_state.tone, i));
            playback_state.key_pressed[i] = false;
            playback_state.timers[i] = 0;
          }

          for (uint8_t i = 1; i < END_SCALE_LIST; i++)
          {
            uint8_t selected = (playback_state.scale + i) % END_SCALE_LIST;
            if (config.scales_enabled[selected])
            {
              playback_state.scale = selected;
              break;
            }
          }

          while (fb_updating)
            loop_task();

          ui_draw_home(*fb, &playback_state);

          SSD1306_MINIMAL_transferFramebuffer();
          // Clear leds after drawing to screen to minimize artifacts
          for (uint8_t i = 0; i < 8; i++)
          {
            set_led(LED_BUTTON_BASE + i, OFF, 0.0f);
          }
        }

        if (was_key_pressed(RIGHT))
        {
          // Clear all the previous notes
          for (uint8_t i = 0; i < 8; i++)
          {
            midi_send_note_off(i, button_to_midi(playback_state.last_knob, playback_state.scale, playback_state.tone, i));
            playback_state.key_pressed[i] = false;
            playback_state.timers[i] = 0;
          }

          playback_state.tone = (playback_state.tone + 1) % 12;

          while (fb_updating)
            loop_task();

          ui_draw_home(*fb, &playback_state);

          SSD1306_MINIMAL_transferFramebuffer();

          // Clear leds after drawing to screen to minimize artifacts
          for (uint8_t i = 0; i < 8; i++)
          {
            set_led(LED_BUTTON_BASE + i, OFF, 0.0f);
          }
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
        if (was_key_pressed(JOYC))
        {
          state = MENU_SCREEN;
          dir = FORWARD;
          break;
        }
        if (!fb_updating)
        {
          SSD1306_MINIMAL_transferFramebuffer();
        }
      }

      break;
    case MENU_SCREEN:
      ui_draw_menu(backbuffer, &menu_state);
      animate_switch();
      while (1)
      {
        loop_task();
        if (was_key_pressed(MODE) || was_key_pressed(LEFT) || was_key_pressed(JOYC))
        {
          config_save_to_flash();
          state = MIDI_PLAYBACK;
          dir = BACK;
          break;
        }
        if (animate_list(&menu_state, 4))
        {
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
          state_t menu_states[] = {LED_SCREEN, CONFIG_SCREEN, SENSITIVITY_SCREEN, ABOUT_SCREEN};
          state = menu_states[menu_state.selected];
          dir = FORWARD;
          break;
        }
      }
      break;
    case LED_SCREEN:
      ggl_draw_text(backbuffer, 25, 16, "Click any axis", font_data, 0);
      ggl_draw_text(backbuffer, 25, 30, "to select LED", font_data, 0);
      animate_switch();
      uint8_t current_knob = knob_step();
      uint8_t last_knob = current_knob;
      float intensity = 0.1f;
      while (1)
      {
        loop_task();
        if (was_key_pressed(LEFT))
        {
          state = MENU_SCREEN;
          dir = BACK;
          break;
        }

        // Pulse all LEDs
        for (uint8_t i = 0; i < 8; i++)
        {
          float wave = 0.5f + 0.5f * sinf(HAL_GetTick() / 300.0f + i);
          set_led(LED_BUTTON_BASE + i, config.color[LED_BUTTON_BASE + i], wave * intensity);
        }
        for (uint8_t i = 0; i < 8; i++)
        {
          float wave = 0.5f + 0.5f * sinf(HAL_GetTick() / 300.0f + i + 8);
          set_led(LED_KNOB_BASE + i, config.color[LED_KNOB_BASE], wave * intensity);
        }
        set_led(LED_PLAY, config.color[LED_PLAY], 0.2f + 0.8f * sinf(HAL_GetTick() / 500.0f));
        set_led(LED_STOP, config.color[LED_STOP], 0.2f + 0.8f * sinf(HAL_GetTick() / 500.0f + 1));
        set_led(LED_MODE, config.color[LED_MODE], 0.2f + 0.8f * sinf(HAL_GetTick() / 500.0f + 2));

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

          if (adc_buff[i] < threshold)
          {
            selected_led.led_selected = LED_BUTTON_BASE + i;
            selected_led.color = config.color[selected_led.led_selected];
            selected_led.list = (list_animation_t){0};
            state = COLOR_SCREEN;
            dir = FORWARD;
            goto led_screen_exit;
          }
        }

        if ((current_knob = knob_step()) != last_knob)
        {
          last_knob = current_knob;
          selected_led.led_selected = LED_KNOB_BASE;
          selected_led.color = config.color[selected_led.led_selected];
          selected_led.list = (list_animation_t){0};
          state = COLOR_SCREEN;
          dir = FORWARD;
          break;
        }

        in_key_t keys_to_check[] = {PLAY, STOP, MODE};
        uint8_t keys_leds[] = {LED_PLAY, LED_STOP, LED_MODE};
        for (size_t i = 0; i < sizeof(keys_to_check) / sizeof(keys_to_check[0]); i++)
        {
          if (was_key_pressed(keys_to_check[i]))
          {
            selected_led.led_selected = keys_leds[i];
            selected_led.color = config.color[selected_led.led_selected];
            selected_led.list = (list_animation_t){};
            state = COLOR_SCREEN;
            dir = FORWARD;
            goto led_screen_exit;
          }
        }
      }
    led_screen_exit:
      break;
    case COLOR_SCREEN:
      ui_draw_leds(backbuffer, &selected_led);
      animate_switch();
      while (1)
      {
        loop_task();

        animate_list(&selected_led.list, 3);
        if (is_key_down(RIGHT))
        {
          config_modified = true;
          selected_led.color.rgb[selected_led.list.selected] += 1;
          while (fb_updating)
            loop_task();
          ui_draw_leds(*fb, &selected_led);
          SSD1306_MINIMAL_transferFramebuffer();
        }
        else if (is_key_down(LEFT))
        {
          config_modified = true;
          selected_led.color.rgb[selected_led.list.selected] -= 1;
          while (fb_updating)
            loop_task();
          ui_draw_leds(*fb, &selected_led);
          SSD1306_MINIMAL_transferFramebuffer();
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
    case CONFIG_SCREEN:
      ui_draw_config(backbuffer, &config_state);
      animate_switch();
      while (1)
      {
        loop_task();
        if (was_key_pressed(LEFT) || was_key_pressed(STOP))
        {
          state = MENU_SCREEN;
          dir = BACK;
          break;
        }

        if (animate_list(&config_state, 3))
        {
          for (size_t i = 0; i < 6; i++)
          {
            while (fb_updating)
              loop_task();
            ui_draw_config(*fb, &config_state);
            SSD1306_MINIMAL_transferFramebuffer();
          }
        }

        if (was_key_pressed(RIGHT))
        {
          switch (config_state.selected)
          {
          case CONFIG_SHOW_ADC:
            state = ADC_SCREEN;
            dir = FORWARD;
            goto exit_config_screen;
          case CONFIG_SCALE:
            state = SCALE_SCREEN;
            dir = FORWARD;
            goto exit_config_screen;
          case CONFIG_DFU:
            while (fb_updating)
              loop_task();
            ggl_clear_fb(*fb);
            ggl_draw_text(*fb, 40, 18, "DFU Mode", font_data, 0);
            SSD1306_MINIMAL_transferFramebuffer();
            while (fb_updating)
              loop_task();
            jump_to_bootloader();
            break;
          }
        }
      }
    exit_config_screen:
      break;
    case ADC_SCREEN:
      ggl_draw_text(backbuffer, 26, 8, "ADC Debugging", font_data, 0);
      animate_switch();
      while (1)
      {
        loop_task();
        if (was_key_pressed(STOP))
        {
          state = CONFIG_SCREEN;
          dir = BACK;
          break;
        }
        if (!fb_updating)
        {
          ggl_clear_fb(*fb);
          ggl_draw_text(*fb, 26, 8, "ADC Debugging", font_data, 0);
          for (uint8_t i = 0; i < 11; i++)
          {
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "%4d", adc_buff[i]);
            uint8_t x = 6 + (i / 3) * 30;  // Each column starts 40 pixels apart
            uint8_t y = 20 + (i % 3) * 10; // Each row is 10 pixels apart within a column
            ggl_draw_text(*fb, x, y, buffer, font_data, 0);
          }
          SSD1306_MINIMAL_transferFramebuffer();
        }
      }
      break;
    case SCALE_SCREEN:
      ui_draw_scale_selector(backbuffer, &scale_select_state);
      animate_switch();
      while (1)
      {
        loop_task();
        if (was_key_pressed(LEFT) || was_key_pressed(STOP))
        {
          bool consistant = false;
          for (uint8_t i = 0; i < END_SCALE_LIST; i++)
          {
            if (config.scales_enabled[i])
            {
              consistant = true;
              break;
            }
          }
          // Make sure that at least the basic major scale is enabled
          // if the user happens to deselect all of them
          if (!consistant)
          {
            config.scales_enabled[MAJOR] = true;
          }
          state = CONFIG_SCREEN;
          dir = BACK;
          break;
        }

        if (animate_list(&scale_select_state, END_SCALE_LIST))
        {
          for (size_t i = 0; i < 6; i++)
          {
            while (fb_updating)
              loop_task();
            ui_draw_scale_selector(*fb, &scale_select_state);
            SSD1306_MINIMAL_transferFramebuffer();
          }
        }

        if (was_key_pressed(RIGHT))
        {
          config.scales_enabled[scale_select_state.selected] = !config.scales_enabled[scale_select_state.selected];
          config_modified = true;
          while (fb_updating)
            loop_task();
          ui_draw_scale_selector(*fb, &scale_select_state);
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
      ggl_draw_text(backbuffer, 8, 10, "GigaSound", font_data, 0);
      ggl_draw_text(backbuffer, 8, 30, "Version 1234", font_data, 0);
      animate_switch();
      while (1)
      {
        loop_task();
        if (was_key_pressed(STOP) || was_key_pressed(LEFT))
        {
          state = MENU_SCREEN;
          dir = BACK;
          break;
        }
      }
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