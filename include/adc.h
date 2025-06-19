#ifndef ADC_H
#define ADC_H
#include <stdint.h>

#define ADC_CHANNELS 11

#define ADC_AXIS_X 10
#define ADC_AXIS_Y 9

#define ADC_KNOB 8

extern uint16_t adc_buff[ADC_CHANNELS];

extern volatile uint8_t adc_complete;

void adc_init_dma();

void adc_print_buffer();

#endif // ADC_H