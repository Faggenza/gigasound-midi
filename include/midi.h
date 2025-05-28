#include <stdint.h>

#define MIDI_STOP 0xFC
#define MIDI_START 0xFA
#define MIDI_CONTINUE 0xFB
void midi_send_cmd(uint8_t cmd);

void midi_discard_packet(void);
void midi_mpe_init(void);
void midi_set_pitch_bend_sensitivity(uint8_t sensitivity);
void midi_set_pitch_bend(uint16_t pitch);
void midi_set_channel_pressure(uint8_t channel, uint8_t pressure);
void midi_send_note_on(uint8_t channel, uint8_t note, uint8_t velocity);
void midi_send_note_off(uint8_t channel, uint8_t note);
void midi_send_modulation(uint16_t value);
void midi_task(void);