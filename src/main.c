
#include "main.h"
#include "stdio.h"
#include "uart.h"
#include "init.h"
#include "led.h"

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
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_SPI3_Init();

  // Turn all LEDs off
  for (size_t i = 0; i < N_LED; i++)
  {
    set_led(i, (color_t){0, 0, 0}, 0.0f);
  }
  HAL_SPI_Transmit(&hspi3, led_buff, LED_BUFF_N, 1000);

  set_led(0, RED, 0.1f);
  set_led(1, GREEN, 0.1f);
  set_led(2, BLUE, 0.1f);
  set_led(2, BLUE, 0.1f);

  HAL_SPI_Transmit(&hspi3, led_buff, LED_BUFF_N, 1000);

  while (1)
  {
    // HAL_ADC_Start(&hadc1);
    // if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK)
    // {
    // puts("Timeout adc\n");
    // continue;
    // }
    // HAL_ADC_Stop(&hadc1);
    // uint32_t adc_val = HAL_ADC_GetValue(&hadc1);
    // printf("%04lu\n", adc_val); // 0 .. 4095
  }
}