#include "scale.h"
#include "stdint.h"

char *scale_to_string[END_SCALE_LIST] = {"Major", "Major Harmonic", "Blues", "Minor Natural", "Minor Harmonic", "Minor Melodic Ascending"};

char *tone_to_string[12] = {"Do", "Do#", "Re", "Re#", "Mi", "Fa", "Fa#", "Sol", "Sol#", "La", "La#", "Si"};

static uint8_t intervals[END_SCALE_LIST][8] = {
    {DO, RE, MI, FA, SOL, LA, SI, DO},     // MAJOR
    {DO, RE, MI, FA, SOL, SOLd, SI, DO},   // MAJOR_HARMONIC
    {DO, REd, FA, FAd, FAd, SOL, LA, DO},  // BLUES
    {DO, RE, REd, FA, SOL, SOLd, LAd, DO}, // MINOR_NATURAL
    {DO, RE, REd, FA, SOL, SOLd, SI, DO},  // MINOR_HARMONIC
    {DO, RE, REd, FA, SOL, LA, SI, DO},    // MINOR_MELODIC_ASCENDING
};

// octave shifts everything by 12 semitones
// button from 0 to 7, "Do to Do" in if tone is Do
uint8_t button_to_midi(uint8_t octave, scale_t scale, tone_t tone, uint8_t button)
{
    return tone + (octave * 12) + intervals[tone][button];
}