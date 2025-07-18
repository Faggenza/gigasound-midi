// Original version from: https://github.com/uNetworking/SSD1306

#ifndef SSD1306_MINIMAL_H
#define SSD1306_MINIMAL_H

#define SSD1306_MINIMAL_SLAVE_ADDR 0x3c << 1
#include <stdint.h>
#include <stm32f4xx_hal.h>
#include "gigagl.h"

extern I2C_HandleTypeDef hi2c1;

static uint8_t dma_buffer[(128 * 8) + 1] = {0};
framebuffer_t *fb = (framebuffer_t *)(dma_buffer + 1); // Point to the framebuffer part of the buffer

volatile bool fb_updating = false;

void SSD1306_MINIMAL_transferFramebuffer()
{
  dma_buffer[0] = 0x40; // Command byte for data transfer
  if (fb_updating)
  {
    return; // Prevent concurrent updates
  }
  HAL_I2C_Master_Transmit_DMA(&hi2c1, SSD1306_MINIMAL_SLAVE_ADDR, dma_buffer, sizeof(dma_buffer));
  fb_updating = true;
}

// DMA transfer callback
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    fb_updating = false;
  }
}

void SSD1306_MINIMAL_init()
{
  unsigned char initialization[] = {
      /* Enable charge pump regulator (RESET = ) */
      0x80,
      0x8d,
      0x80,
      0x14,
      /* Display On (RESET = ) */
      0x80,
      0xaf,
      /* Set Memory Addressing Mode to Horizontal Addressing Mode (RESET = Page Addressing Mode) */
      0x80,
      0x20,
      0x80,
      0x0,
      /* Reset Column Address (for horizontal addressing) */
      0x80,
      0x21,
      0x80,
      0,
      0x80,
      127,
      /* Reset Page Address (for horizontal addressing) */
      0x80,
      0x22,
      0x80,
      0,
      0x80,
      7,
      /*Set Left/Right rotation*/
      0x80,
      0xa0 | 0x01,
      /*Set Scan Direction*/
      0x80,
      0xc0 | 0x08,
  };

  HAL_I2C_Master_Transmit(&hi2c1, SSD1306_MINIMAL_SLAVE_ADDR, initialization, sizeof(initialization), UINT32_MAX);
}

#endif
