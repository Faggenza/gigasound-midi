#include "stm32f4xx_hal.h"

#define BOOTLOADER_BASE 0x1FFF0000 // STM32F4 system memory start address

void jump_to_bootloader(void)
{
    void (*bootloader)(void);
    uint32_t bootloader_stack = *((uint32_t *)BOOTLOADER_BASE);
    uint32_t bootloader_entry = *((uint32_t *)(BOOTLOADER_BASE + 4));

    // De-initialize peripherals
    HAL_RCC_DeInit();
    HAL_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // Remap system memory to 0x00000000
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();

    // Set MSP to bootloader's stack
    __set_MSP(bootloader_stack);

    // Jump to bootloader
    bootloader = (void (*)(void))bootloader_entry;
    bootloader();
}