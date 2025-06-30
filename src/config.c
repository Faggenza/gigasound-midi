#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "eeprom.h"
#include "string.h"
#include "config.h"
#include "led.h"

bool config_modified = false;

static config_t default_config = {
    .version = CONFIG_VERSION, // Initial version
    .joycon_calibration = {0},
    .scales_enabled = {true},
    .limit_updates = false,
    .color = {
        RED,
        RED,
        RED,
        RED,
        RED,
        RED,
        RED,
        RED,
        L_GREEN,
        RED,
        BLUE,
        PURPLE,
        PURPLE,
        PURPLE,
        PURPLE,
        PURPLE,
        PURPLE,
        PURPLE,
        PURPLE,
    },
};

config_t config = {0};

uint16_t VirtAddVarTab[sizeof(config_t) / 2] = {0};

void config_init()
{

    for (uint16_t addr = 0; addr < (sizeof(config_t) / 2); addr++)
    {
        VirtAddVarTab[addr] = addr;
    }

    HAL_FLASH_Unlock();
    EE_Init();
    // Read the configuration from flash memory

    for (uint16_t addr = 0; addr < (sizeof(config_t) / 2); addr++)
    {
        EE_ReadVariable(addr, (uint16_t *)(&(config)) + addr);
    }

    if (config.version != CONFIG_VERSION)
    {
        memcpy(&config, &default_config, sizeof(config_t));
        config_save_to_flash();
    }

    HAL_FLASH_Lock();
}

void config_save_to_flash()
{
    // Save the configuration to flash memory
    // This function should handle writing the config to a specific flash address
    // and ensure that the flash is erased before writing.
    if (!config_modified)
        return;

    HAL_FLASH_Unlock();

    for (uint16_t addr = 0; addr < (sizeof(config_t) / 2); addr++)
    {
        EE_WriteVariable(addr, *((uint16_t *)(&(config)) + addr));
    }

    HAL_FLASH_Lock();

    config_modified = false; // Reset modified flag after saving
}