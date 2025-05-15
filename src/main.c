
#include "main.h"
#include "stdio.h"
#include "uart.h"
#include "init.h"
#include "led.h"
#include "tusb.h"

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

int main(void)
{

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_SPI3_Init();
  MX_USB_OTG_FS_PCD_Init();

  // uint16_t adc_buff[11] = {0};

  // // Turn all LEDs off
  // for (size_t i = 0; i < N_LED; i++)
  // {
  //   set_led(i, (color_t){0, 0, 0}, 0.0f);
  // }
  // HAL_SPI_Transmit(&hspi3, led_buff, LED_BUFF_N, 1000);
  // HAL_SPI_Transmit(&hspi3, led_buff, LED_BUFF_N, 1000);

  // HAL_SPI_Transmit_DMA(&hspi3, led_buff, LED_BUFF_N);
  // HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buff, 11);
  // uint32_t i = 0;
  while (1)
  {
    // set_led(i % N_LED, RED, 0.1f);
    // set_led((i + 1) % N_LED, GREEN, 0.1f);
    // set_led((i + 2) % N_LED, BLUE, 0.1f);
    // set_led((i + 3) % N_LED, YELLOW, 0.1f);
    // set_led((i + 3) % N_LED, CYAN, 0.1f);
    // set_led((i + 4) % N_LED, PURPLE, 0.1f);
    // set_led((i + 5) % N_LED, MAGENTA, 0.1f);
    // set_led((i + 6) % N_LED, TEAL, 0.1f);
    // i++;
    // if (adc_complete)
    // {
    //   adc_complete = 0;
    //   printf("ADC: ");
    //   for (size_t j = 0; j < 11; j++)
    //   {
    //     printf("CH%d: %04lu, ", j, adc_buff[j]);
    //   }
    //   puts("\n");
    // }
    // HAL_Delay(100);
  }
}