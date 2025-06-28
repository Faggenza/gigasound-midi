#ifndef INIT_H
#define INIT_H

#include "stm32f4xx_hal.h"

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_ADC1_Init(void);
void MX_SPI3_Init(void);
void MX_DMA_Init(void);
void MX_USB_OTG_FS_PCD_Init(void);
void MX_I2C1_Init(void);
extern ADC_HandleTypeDef hadc1;
extern SPI_HandleTypeDef hspi3;
extern DMA_HandleTypeDef hdma_spi3_tx;
extern UART_HandleTypeDef huart1;
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_i2c1_tx;

#endif // INIT_H