#include "calibrate.h"
#include "uart.h"

joycon_calibration calibrate_joycon(uint16_t *adc_buff, uint32_t check)
{
    joycon_calibration cal = {
        .x_min = 4095,
        .x_max = 0,
        .y_min = 4095,
        .y_max = 0};

    uint32_t start = HAL_GetTick();
    uint32_t current = start;
    printf("nuovo ciclo %d", 0);
    while ((current - start) <= check)
    {
        if (adc_complete)
        {
            adc_complete = 0;
            if (adc_buff[10] < cal.x_min)
                cal.x_min = adc_buff[10];
            if (adc_buff[10] > cal.x_max)
                cal.x_max = adc_buff[10];
            if (adc_buff[9] < cal.y_min)
                cal.y_min = adc_buff[9];
            if (adc_buff[9] > cal.y_max)
                cal.y_max = adc_buff[9];
        }
        current = HAL_GetTick();
    }
    return cal;
}