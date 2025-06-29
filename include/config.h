#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "led.h"
#include "calibrate.h"

// Change this every every breaking change
#define CONFIG_VERSION 1

typedef struct
{
    uint16_t version;
    color_t color[N_LED];
    joycon_calibration joycon_calibration;
} config_t;

extern bool config_modified;
extern config_t config;

void config_init();
void config_save_to_flash();

#endif // CONFIG_H