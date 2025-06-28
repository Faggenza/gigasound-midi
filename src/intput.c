#include <stdint.h>
#include "adc.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "input.h"

// BIT MASKS for the keys
static uint8_t pressed = 0;

static uint32_t timers[8] = {0};

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint8_t key;
    switch (GPIO_Pin)
    {
    case PLAY_Pin:
        key = PLAY;
        break;
    case STOP_Pin:
        key = STOP;
        break;
    case MODE_Pin:
        key = MODE;
        break;
    default:
        return;
    }
    if (timers[key] != 0 && HAL_GetTick() - timers[key] < DEBOUNCE_BUTTON_TIME_MS)
    {
        // Debounce: ignore if the key was pressed less than 50ms ago
        return;
    }
    timers[key] = HAL_GetTick();
    pressed |= (1 << key);
}

uint8_t is_key_down(in_key_t key)
{
    switch (key)
    {
    case PLAY:
        return HAL_GPIO_ReadPin(GPIOC, PLAY_Pin) == GPIO_PIN_RESET;
    case STOP:
        return HAL_GPIO_ReadPin(GPIOC, STOP_Pin) == GPIO_PIN_RESET;
    case MODE:
        return HAL_GPIO_ReadPin(GPIOC, MODE_Pin) == GPIO_PIN_RESET;
        // Todo: implement getting the calibration data
    case RIGHT:
        return adc_buff[ADC_AXIS_X] > 2800;
    case LEFT:
        return adc_buff[ADC_AXIS_X] < 1200;
    case UP:
        return adc_buff[ADC_AXIS_Y] < 1200;
    case DOWN:
        return adc_buff[ADC_AXIS_Y] > 2800;
    default:
        return 0; // Invalid key
    }
}

void update_axis_states()
{
    // Debounce logic for axis keys
    for (int i = RIGHT; i <= DOWN; i++)
    {
        if (is_key_down(i))
        {
            if (HAL_GetTick() - timers[i] >= DEBOUNCE_JOYCON_TIME_MS)
            {
                timers[i] = HAL_GetTick();
                pressed |= (1 << i);
            }
        }
    }
}

void clear_pressed()
{
    pressed = 0;
}

uint8_t was_key_pressed(in_key_t key)
{
    uint8_t res = pressed & (1 << key);
    pressed &= ~(1 << key);
    return res;
}

uint8_t knob_step()
{
    return 8 - ((adc_buff[ADC_KNOB] + 300) / 512);
}