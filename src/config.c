#include "config.h"

bool config_modified = false;

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

    config_modified = false; // Reset modified flag after saving
}