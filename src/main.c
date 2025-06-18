
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
#include "gigagl.h"
#include "ui/menu.h"

void Error_Handler(void)
{
  __disable_irq();
  asm volatile("bkpt 0");
}
volatile uint8_t adc_complete = 0;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  adc_complete = 1;
}

uint8_t play_pressed = 0;
uint8_t stop_pressed = 0;
uint8_t mode_pressed = 0;

uint8_t play_is_on, stop_is_on, mode_is_on = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == PLAY_Pin)
  {
    play_pressed = 1;
  }
  else if (GPIO_Pin == STOP_Pin)
  {
    stop_pressed = 1;
  }
  else if (GPIO_Pin == MODE_Pin)
  {
    mode_pressed = 1;
  }
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

  uint16_t adc_buff[11] = {0};

  // Turn all LEDs off
  for (size_t i = 0; i < N_LED; i++)
  {
    set_led(i, (color_t){0, 0, 0}, 0.0f);
  }

  HAL_SPI_Transmit(&hspi3, led_buff, LED_BUFF_N, 1000);
  HAL_SPI_Transmit(&hspi3, led_buff, LED_BUFF_N, 1000);

  HAL_SPI_Transmit_DMA(&hspi3, led_buff, LED_BUFF_N);
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buff, 11);

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
      .animation_frame = 0};

  int i = 0;
  while (1)
  {
    SSD1306_MINIMAL_transferFramebuffer();
    i++;
    // ggl_set_pixel(*fb, i % 128, 0, GGL_WHITE);
    dma_buffer[1 + (i % 1024)] ^= 0xFF; // Set the first pixel to white
    tud_task();
    // HAL_Delay(100);
    // midi_task();

    // set_led(i % N_LED, RED, 0.1f);
    // set_led((i + 1) % N_LED, GREEN, 0.1f);
    // set_led((i + 2) % N_LED, BLUE, 0.1f);
    // set_led((i + 3) % N_LED, YELLOW, 0.1f);
    // set_led((i + 3) % N_LED, CYAN, 0.1f);
    // set_led((i + 4) % N_LED, PURPLE, 0.1f);
    // set_led((i + 5) % N_LED, MAGENTA, 0.1f);
    // set_led((i + 6) % N_LED, TEAL, 0.1f);
    if (adc_complete)
    {
      adc_complete = 0;
      printf("ADC: ");
      // for (size_t j = 0; j < 11; j++)
      // {
      //   printf("CH%d: %04lu, ", j, adc_buff[j]);
      // }
      // puts("\n");
      uint8_t n_leds = 8 - ((adc_buff[8] + 300) / 512);
      if (n_leds == 7)
      {
        jump_to_bootloader();
      }
      // printf("n_leds: %d, adc_8: ", n_leds, adc_buff[8]);
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
    }
    if (play_pressed)
    {
      play_pressed = 0;
      if (play_is_on)
      {
        play_is_on = 0;
        set_led(8, BLACK, 0.1f);
      }
      else
      {
        play_is_on = 1;
        set_led(8, GREEN, 0.1f);
      }
    }
    if (stop_pressed)
    {
      stop_pressed = 0;
      if (stop_is_on)
      {
        stop_is_on = 0;
        set_led(9, BLACK, 0.1f);
      }
      else
      {
        stop_is_on = 1;
        set_led(9, RED, 0.1f);
      }
    }
    if (mode_pressed)
    {
      mode_pressed = 0;
      if (mode_is_on)
      {
        mode_is_on = 0;
        set_led(10, BLACK, 0.1f);
      }
      else
      {
        mode_is_on = 1;
        set_led(10, BLUE, 0.1f);
      }
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