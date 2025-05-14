void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_ADC1_Init(void);
void MX_SPI3_Init(void);
void MX_DMA_Init(void);
extern ADC_HandleTypeDef hadc1;
extern SPI_HandleTypeDef hspi3;
extern DMA_HandleTypeDef hdma_spi3_tx;
extern UART_HandleTypeDef huart1;
