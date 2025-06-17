// Original version from: https://github.com/uNetworking/SSD1306

#ifndef SSD1306_MINIMAL_H
#define SSD1306_MINIMAL_H

#define SSD1306_MINIMAL_SLAVE_ADDR 0x3c << 1
#include <stdint.h>
#include <stm32f4xx_hal.h>

extern I2C_HandleTypeDef hi2c1;

/* Transfers the entire framebuffer in 64 I2C data messages */
void SSD1306_MINIMAL_transferFramebuffer(uint8_t *fb)
{
  HAL_I2C_Mem_Write_DMA(&hi2c1, SSD1306_MINIMAL_SLAVE_ADDR, 0x40, 1, fb, 128 * 64);
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
