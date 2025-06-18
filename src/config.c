#include <stdint.h>
#include <stdbool.h>
#include "stm32f401xe.h"

typedef struct
{
    uint16_t version
} config_t;

config_t config = {
    .version = 0x0001 // Initial version
};

void config_get_from_flash(config_t *cfg)
{
    //Read the configuration from flash memory
    
}