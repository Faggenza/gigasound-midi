#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "init.h"
#include "uart.h"

#define ADC_CHANNELS 11

#define ADC_AXIS_X 10
#define ADC_AXIS_Y 9

#define ADC_KNOB 8

uint16_t adc_buff[ADC_CHANNELS] = {0};

volatile uint8_t adc_complete = 0;

void adc_init_dma()
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buff, 11);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    adc_complete = 1;
}

void adc_print_buffer()
{
    if (adc_complete)
    {
        adc_complete = 0;
        printf("ADC: ");
        for (size_t j = 0; j < 11; j++)
        {
            printf("CH%d: %04u, ", j, adc_buff[j]);
        }
        puts("\n");
    }
}