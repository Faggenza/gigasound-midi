
#include "main.h"
#include "stdio.h"
#include "uart.h"
#include "init.h"

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

  uint32_t color = 15 << 16 | 0 << 8 | 0;
  uint8_t sendData[24];
  int indx = 0;

  for (int i = 23; i >= 0; i--)
  {
    if (((color >> i) & 0x01) == 1)
      sendData[indx++] = 0b110; // store 1
    else
      sendData[indx++] = 0b100; // store 0
  }

  HAL_SPI_Transmit(&hspi3, sendData, 24, 1000);
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