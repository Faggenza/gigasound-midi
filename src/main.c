
#include "main.h"
#include "stdio.h"
#include "uart.h"
#include "init.h"
#include "led.h"
#include "bsp/board_api.h"
#include "tusb.h"

void midi_task(void);

void Error_Handler(void)
{
  __disable_irq();
  asm volatile("bkpt 0");
}
volatile uint8_t adc_complete = 0;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  adc_complete = 1;
}

int main(void)
{

  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_SPI3_Init();
  MX_USB_OTG_FS_PCD_Init();

  // uint16_t adc_buff[11] = {0};

  // // Turn all LEDs off
  // for (size_t i = 0; i < N_LED; i++)
  // {
  //   set_led(i, (color_t){0, 0, 0}, 0.0f);
  // }
  // HAL_SPI_Transmit(&hspi3, led_buff, LED_BUFF_N, 1000);
  // HAL_SPI_Transmit(&hspi3, led_buff, LED_BUFF_N, 1000);

  // HAL_SPI_Transmit_DMA(&hspi3, led_buff, LED_BUFF_N);
  // HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buff, 11);
  // uint32_t i = 0;

  // init device stack on configured roothub port
  tusb_rhport_init_t dev_init = {
      .role = TUSB_ROLE_DEVICE,
      .speed = TUSB_SPEED_AUTO};
  tusb_init(BOARD_TUD_RHPORT, &dev_init);

  if (board_init_after_tusb)
  {
    board_init_after_tusb();
  }
  while (1)
  {
    tud_task();
    midi_task();
    // midi_task();
    //  set_led(i % N_LED, RED, 0.1f);
    //  set_led((i + 1) % N_LED, GREEN, 0.1f);
    //  set_led((i + 2) % N_LED, BLUE, 0.1f);
    //  set_led((i + 3) % N_LED, YELLOW, 0.1f);
    //  set_led((i + 3) % N_LED, CYAN, 0.1f);
    //  set_led((i + 4) % N_LED, PURPLE, 0.1f);
    //  set_led((i + 5) % N_LED, MAGENTA, 0.1f);
    //  set_led((i + 6) % N_LED, TEAL, 0.1f);
    //  i++;
    //  if (adc_complete)
    //  {
    //    adc_complete = 0;
    //    printf("ADC: ");
    //    for (size_t j = 0; j < 11; j++)
    //    {
    //      printf("CH%d: %04lu, ", j, adc_buff[j]);
    //    }
    //    puts("\n");
    //  }
    //  HAL_Delay(100);
  }
}

// Invoked when device is mounted
void tud_mount_cb(void)
{
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void)remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}

//--------------------------------------------------------------------+
// MIDI Task
//--------------------------------------------------------------------+

// Variable that holds the current position in the sequence.
uint32_t note_pos = 0;

// Store example melody as an array of note values
const uint8_t note_sequence[] = {
    74, 78, 81, 86, 90, 93, 98, 102, 57, 61, 66, 69, 73, 78, 81, 85, 88, 92, 97, 100, 97, 92, 88, 85, 81, 78,
    74, 69, 66, 62, 57, 62, 66, 69, 74, 78, 81, 86, 90, 93, 97, 102, 97, 93, 90, 85, 81, 78, 73, 68, 64, 61,
    56, 61, 64, 68, 74, 78, 81, 86, 90, 93, 98, 102};

void midi_task(void)
{
  static uint32_t start_ms = 0;

  uint8_t const cable_num = 0; // MIDI jack associated with USB endpoint
  uint8_t const channel = 0;   // 0 for channel 1

  // The MIDI interface always creates input and output port/jack descriptors
  // regardless of these being used or not. Therefore incoming traffic should be read
  // (possibly just discarded) to avoid the sender blocking in IO
  while (tud_midi_available())
  {
    uint8_t packet[4];
    tud_midi_packet_read(packet);
  }

  // Previous positions in the note sequence.
  int previous = (int)(note_pos - 1);

  // If we currently are at position 0, set the
  // previous position to the last note in the sequence.
  if (previous < 0)
  {
    previous = sizeof(note_sequence) - 1;
  }

  // Send Note On for current position at full velocity (127) on channel 1.
  uint8_t note_on[3] = {0x90 | channel, note_sequence[note_pos], 127};
  tud_midi_stream_write(cable_num, note_on, 3);

  // Send Note Off for previous note.
  uint8_t note_off[3] = {0x80 | channel, note_sequence[previous], 0};
  tud_midi_stream_write(cable_num, note_off, 3);

  // Increment position
  note_pos++;

  // If we are at the end of the sequence, start over.
  if (note_pos >= sizeof(note_sequence))
  {
    note_pos = 0;
  }
}
