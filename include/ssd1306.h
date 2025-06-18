// Original version from: https://github.com/uNetworking/SSD1306

#ifndef SSD1306_MINIMAL_H
#define SSD1306_MINIMAL_H

#define SSD1306_MINIMAL_SLAVE_ADDR 0x3c << 1
#include <stdint.h>
#include <stm32f4xx_hal.h>

extern I2C_HandleTypeDef hi2c1;

static uint8_t dma_buffer[(128 * 8) + 1] = {0};
uint8_t *fb = dma_buffer + 1; // framebuffer starts at index 1, index 0 is reserved for the command byte

/* Transfers the entire framebuffer in 64 I2C data messages */
void SSD1306_MINIMAL_transferFramebuffer(uint8_t *fb)
{
  dma_buffer[0] = 0x40; // Command byte for data transfer
  HAL_I2C_Master_Transmit(&hi2c1, SSD1306_MINIMAL_SLAVE_ADDR, dma_buffer, sizeof(dma_buffer), UINT32_MAX);
  // for (int i = 0; i < 64; i++)
  // {
  //   I2C_WRAPPER_beginTransmission(SSD1306_MINIMAL_SLAVE_ADDR);
  //   I2C_WRAPPER_write(0x40);
  //   for (int i = 0; i < 16; i++)
  //   {
  //     I2C_WRAPPER_write(*p);
  //     p++;
  //   }
  //   I2C_WRAPPER_endTransmission();
  // }
}

// Callback for when DMA transfer completes
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c == &hi2c1)
  {
    // DMA transfer to SSD1306 complete
    // Optional: set a flag or trigger next action
  }
}
/* Horizontal addressing mode maps to linear framebuffer */
// void SSD1306_MINIMAL_setPixel(unsigned int x, unsigned int y)
// {
//   x &= 0x7f;
//   y &= 0x3f;
//   SSD1306_MINIMAL_framebuffer[((y & 0xf8) << 4) + x] |= 1 << (y & 7);
// }

void SSD1306_MINIMAL_init()
{
  unsigned char initialization[] = {
      /* Enable charge pump regulator (RESET = ) */
      0x80, 0x8d,
      0x80, 0x14,
      /* Display On (RESET = ) */
      0x80, 0xaf,
      /* Set Memory Addressing Mode to Horizontal Addressing Mode (RESET = Page Addressing Mode) */
      0x80, 0x20,
      0x80, 0x0,
      /* Reset Column Address (for horizontal addressing) */
      0x80, 0x21,
      0x80, 0,
      0x80, 127,
      /* Reset Page Address (for horizontal addressing) */
      0x80, 0x22,
      0x80, 0,
      0x80, 7};

  HAL_I2C_Master_Transmit(&hi2c1, SSD1306_MINIMAL_SLAVE_ADDR, initialization, sizeof(initialization), 10000);
  // I2C_WRAPPER_beginTransmission(SSD1306_MINIMAL_SLAVE_ADDR);
  // for (int i = 0; i < sizeof(initialization); i++)
  // {
  //   I2C_WRAPPER_write(0x80);
  //   I2C_WRAPPER_write(initialization[i]);
  // }
  // I2C_WRAPPER_endTransmission();
}

#endif
