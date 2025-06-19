
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
// #include "ui/home.h"

void Error_Handler(void)
{
  __disable_irq();
  asm volatile("bkpt 0");
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
  bool mode = false;

  // int i = 0;
  ui_draw_menu(*fb, &menu_state);
  SSD1306_MINIMAL_transferFramebuffer();
  while (1)
  {
    tud_task();
    // midi_task();

    if (adc_complete)
    {
      update_axis_states();
      adc_complete = 0;
      uint8_t n_leds = knob_step();
      if (n_leds == 7)
      {
        jump_to_bootloader();
      }
      for (uint8_t i = 0; i < 8; i++)
      {
        if (i <= n_leds)
        {
          set_led(i, RED, 0.1f);
        }
        else
        {
          set_led(i, BLACK, 0.0f);
        }
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
    if (was_key_pressed(MODE))
    {
      mode = !mode;
      set_led(10, mode ? BLUE : BLACK, 0.1f);
    }
    // HAL_Delay(100);
  }
  // SSD1306_MINIMAL_transferFramebuffer(fb);
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