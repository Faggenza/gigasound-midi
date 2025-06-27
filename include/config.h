#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f401xe.h"
#include "led.h"

typedef struct
{
    uint16_t version;
    uint8_t color[N_LED][3];
} config_t;

extern bool config_modified;
extern config_t config;

#endif // CONFIG_H