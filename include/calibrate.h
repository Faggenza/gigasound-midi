#ifndef CALIBRATE_H
#define CALIBRATE_H

#include "main.h"

#define X_AXIS 10
#define Y_AXIS 9
extern volatile uint8_t adc_complete;

typedef struct
{
    uint16_t x_min;
    uint16_t x_max;
    uint16_t y_min;
    uint16_t y_max;
} joycon_calibration;

joycon_calibration calibrate_joycon(uint16_t *adc_buff, uint32_t check);
#endif // CALIBRATE_H