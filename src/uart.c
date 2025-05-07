#include "stdarg.h"
#include "string.h"
#include "main.h"
#include "stdio.h"

UART_HandleTypeDef huart2;

int printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    HAL_UART_Transmit(&huart2, (unsigned char *)buffer, strlen(buffer), 100);
    va_end(args);
    return strlen(buffer);
}

int puts(const char *str)
{
    HAL_UART_Transmit(&huart2, (unsigned char *)str, strlen(str), 100);
    return strlen(str);
}

void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}
