
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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_SPI3_Init();

  // Turn all LEDs off
  for (size_t i = 0; i < N_LED; i++)
  {
    set_led(i, (color_t){0, 0, 0}, 0.0f);
  }
  HAL_SPI_Transmit(&hspi3, led_buff, LED_BUFF_N, 1000);
  HAL_SPI_Transmit(&hspi3, led_buff, LED_BUFF_N, 1000);

  HAL_SPI_Transmit_DMA(&hspi3, led_buff, LED_BUFF_N);
  uint32_t i = 0;
  while (1)
  {
    set_led(i % N_LED, RED, 0.1f);
    set_led((i + 1) % N_LED, GREEN, 0.1f);
    set_led((i + 2) % N_LED, BLUE, 0.1f);
    set_led((i + 3) % N_LED, YELLOW, 0.1f);
    set_led((i + 3) % N_LED, CYAN, 0.1f);
    set_led((i + 4) % N_LED, PURPLE, 0.1f);
    set_led((i + 5) % N_LED, MAGENTA, 0.1f);
    set_led((i + 6) % N_LED, TEAL, 0.1f);
    HAL_Delay(100);
    i++;
    // HAL_ADC_Start(&hadc1);
    // if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK)
    // {
    //   puts("Timeout adc\n");
    //   continue;
    // }
    // HAL_ADC_Stop(&hadc1);
    // uint32_t adc_val = HAL_ADC_GetValue(&hadc1);
    // printf("%04lu\n", adc_val); // 0 .. 4095
  }
}