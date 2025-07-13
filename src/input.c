#include <stdint.h>
#include "adc.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "input.h"
#include "config.h"

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
    case JOYC_Pin:
        key = JOYC;
        break;
    default:
        asm("bkpt");
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
        return HAL_GPIO_ReadPin(GPIOB, PLAY_Pin) == GPIO_PIN_RESET;
    case STOP:
        return HAL_GPIO_ReadPin(GPIOB, STOP_Pin) == GPIO_PIN_RESET;
    case MODE:
        return HAL_GPIO_ReadPin(GPIOB, MODE_Pin) == GPIO_PIN_RESET;
    case JOYC:
        return HAL_GPIO_ReadPin(GPIOB, JOYC_Pin) == GPIO_PIN_RESET;
    case RIGHT:
        return adc_buff[ADC_AXIS_X] > (config.joycon_calibration.x_max - 300);
    case LEFT:
        return adc_buff[ADC_AXIS_X] < (config.joycon_calibration.x_min + 100);
    case UP:
        return adc_buff[ADC_AXIS_Y] < (config.joycon_calibration.y_min + 50);
    case DOWN:
        return adc_buff[ADC_AXIS_Y] > (config.joycon_calibration.y_max - 50);
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
    // Only for buttons:
    if (key < RIGHT)
    {
        // If it was pressed less than 50ms ago
        // or it's not pressed anymore, ignore
        if (!is_key_down(key) && (HAL_GetTick() - timers[key] < 50))
        {
            res = 0;
        }
    }

    pressed &= ~(1 << key);
    return res;
}

static uint8_t last_step = 0;

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define PADDING 100

uint8_t knob_step()
{
    uint16_t knob_value = 4096 - adc_buff[ADC_KNOB];
    uint8_t current_step = MIN(knob_value / 512, 7);

    // Hysteresis cycle
    if (current_step > last_step && (knob_value - (last_step * 512)) > PADDING)
    {
        last_step = current_step;
    }
    else if (current_step < last_step && ((last_step * 512) - knob_value) > PADDING)
    {
        last_step = current_step;
    }

    return last_step;
}