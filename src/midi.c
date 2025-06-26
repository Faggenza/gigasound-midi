#include "tusb.h"

/*
 The MIDI interface always creates input and output port/jack descriptors
 regardless of these being used or not. Therefore incoming traffic should be read
 (possibly just discarded) to avoid the sender blocking in IO
*/
void midi_discard_packet(void)
{
    while (tud_midi_available())
    {
        uint8_t packet[4];
        tud_midi_packet_read(packet);
    }
}

#define MASTER_CHANNEL 0
#define CABLE_NUM 0

static void send_cc(uint8_t channel, uint8_t cc_num, uint8_t value)
{
    uint8_t packet[3] = {0xB0 | channel, cc_num, value};
    tud_midi_stream_write(CABLE_NUM, packet, 3);
}

// MPE Control Change Registered Parameter Number (RPN)
#define CC_101_MSB 0x65
#define CC_100_LSB 0x64
#define MPE_CONFIGURATION_RPN 0x06

void midi_mpe_init(void)
{
    midi_discard_packet();
    send_cc(MASTER_CHANNEL, CC_101_MSB, 0x00);
    send_cc(MASTER_CHANNEL, CC_100_LSB, MPE_CONFIGURATION_RPN);
    // We are using 8 channels
    send_cc(MASTER_CHANNEL, MPE_CONFIGURATION_RPN, 0x08);
}

// Set global pitch bend sensitivity
// see equation on page 24
void midi_set_pitch_bend_sensitivity(uint8_t sensitivity)
{
    send_cc(MASTER_CHANNEL, CC_101_MSB, 0x00);
    send_cc(MASTER_CHANNEL, CC_100_LSB, 0x00);
    send_cc(MASTER_CHANNEL, MPE_CONFIGURATION_RPN, sensitivity & 0x7f);
}

void midi_set_pitch_bend(uint16_t pitch)
{
    uint8_t lsb = pitch & 0x7f;
    uint8_t msb = (pitch >> 7) & 0x7f;
    uint8_t packet[3] = {0xE0 | MASTER_CHANNEL, lsb, msb};
    tud_midi_stream_write(CABLE_NUM, packet, 3);
}

void midi_set_channel_pressure(uint8_t channel, uint8_t pressure)
{
    uint8_t packet[3] = {0xD0 | channel, pressure & 0x7f};
    tud_midi_stream_write(CABLE_NUM, packet, 2);
}

void midi_send_note_on(uint8_t channel, uint8_t note, uint8_t velocity)
{
    midi_set_channel_pressure(channel, 0);
    uint8_t packet[3] = {0x90 | channel, note, velocity};
    tud_midi_stream_write(CABLE_NUM, packet, 3);
}

void midi_send_note_off(uint8_t channel, uint8_t note)
{
    uint8_t packet[3] = {0x80 | channel, note, 0};
    tud_midi_stream_write(CABLE_NUM, packet, 3);
}

void midi_send_cmd(uint8_t cmd)
{
    uint8_t packet[1] = {cmd};
    tud_midi_stream_write(CABLE_NUM, packet, 1);
}

#define MIDI_MODULATION_MSB 0x01
#define MIDI_MODULATION_LSB 0x21

void midi_send_modulation(uint16_t value)
{
    uint8_t lsb = value & 0x7f;
    uint8_t msb = (value >> 7) & 0x7f;
    send_cc(MASTER_CHANNEL, MIDI_MODULATION_MSB, msb);
    send_cc(MASTER_CHANNEL, MIDI_MODULATION_LSB, lsb);
}

/* Example task */

bool notes_on = false;
uint8_t pressure = 0;
uint16_t pitch = 0;
uint8_t sensitivity = 0;
const uint8_t note_sequence[] = {74, 78, 81, 86, 90, 93, 98, 102};

void midi_task(void)
{
    midi_discard_packet();
    if (tud_midi_mounted() && !notes_on)
    {
        notes_on = true;
        for (int i = 1; i <= 8; i++)
        {
            midi_send_note_on(i, note_sequence[i - 1], 127);
        }
    }

    for (int i = 1; i <= 8; i++)
    {
        midi_set_channel_pressure(i, pressure++);
    }
    // send_pitch_bend(pitch++);
    midi_set_pitch_bend_sensitivity(sensitivity++);
    midi_send_modulation(pitch++);
}
