#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_adc.h"
#include "stm32f4xx_ll_usart.h"
#include "stdio.h"
#include "stdarg.h"

#define LED_PIN LL_GPIO_PIN_5
#define LED_GPIO_PORT GPIOA
#define LED_GPIO_CLK_ENABLE() LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA)
#define SYS_CLOCK 84000000L

ADC_HandleTypeDef hadc1;
static void MX_ADC1_Init(void);

UART_HandleTypeDef huart1;
static void MX_USART2_UART_Init(void);

void UART_Printf(const char *fmt, ...)
{
    char buff[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
    HAL_UART_Transmit(&huart1, (uint8_t *)buff, strlen(buff),
                      HAL_MAX_DELAY);
    va_end(args);
}

void Configure_GPIO(void)
{
    LED_GPIO_CLK_ENABLE();
    LL_GPIO_SetPinMode(LED_GPIO_PORT, LED_PIN, LL_GPIO_MODE_OUTPUT);
}

void SystemClock_Config(void)
{
    /* Enable HSE oscillator */
    LL_RCC_HSE_EnableBypass();
    LL_RCC_HSE_Enable();
    while (LL_RCC_HSE_IsReady() != 1)
    {
    };

    /* Set FLASH latency */
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);

    /* Main PLL configuration and activation */
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_8, 400, LL_RCC_PLLP_DIV_4);
    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1)
    {
    };

    /* Sysclk activation on the main PLL */
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
    {
    };

    /* Set APB1 & APB2 prescaler */
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

    /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
    SystemCoreClock = SYS_CLOCK;

    LL_Init1msTick(SystemCoreClock);
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

    ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
     */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
     */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
}
void _Error_Handler(char *file, int line)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

static void MX_USART2_UART_Init(void)
{

    huart1.Instance = USART2;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_ODD;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{

    GPIO_InitTypeDef GPIO_InitStruct;
    if (huart->Instance == USART2)
    {
        /* USER CODE BEGIN USART2_MspInit 0 */

        /* USER CODE END USART2_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        /**USART2 GPIO Configuration
        PA9     ------> USART2_TX
        PA10     ------> USART2_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USER CODE BEGIN USART2_MspInit 1 */

        /* USER CODE END USART2_MspInit 1 */
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{

    if (huart->Instance == USART2)
    {
        /* USER CODE BEGIN USART2_MspDeInit 0 */

        /* USER CODE END USART2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /**USART2 GPIO Configuration
        PA9     ------> USART2_TX
        PA10     ------> USART2_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

        /* USER CODE BEGIN USART2_MspDeInit 1 */

        /* USER CODE END USART2_MspDeInit 1 */
    }
}

int main(void)
{
    SystemClock_Config();
    MX_ADC1_Init();
    Configure_GPIO();
    HAL_ADC_Start(&hadc1);
    MX_USART2_UART_Init();
    while (1)
    {
        uint8_t buff = 0x41;
        HAL_UART_Transmit(&huart1, &buff, 1, HAL_MAX_DELAY);
        // UART_Printf("A");
        LL_mDelay(500);
    }
    while (1)
    {
        LL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
        uint32_t adc_val = HAL_ADC_GetValue(&hadc1);
        UART_Printf("ADC: %04lu\n", adc_val); // 0 .. 4095
        LL_mDelay(1000);
    }
}

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}