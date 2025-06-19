#include <stdint.h>
#include <stdbool.h>
#include "stm32f401xe.h"

bool modified = false;
typedef struct
{
    uint16_t version
} config_t;

config_t config = {
    .version = 0x0001 // Initial version
};

void config_get_from_flash(config_t *cfg)
{
    // Read the configuration from flash memory
}

void config_save_to_flash(const config_t *cfg)
{
    // Save the configuration to flash memory
    // This function should handle writing the config to a specific flash address
    // and ensure that the flash is erased before writing.

    modified = false; // Reset modified flag after saving
}