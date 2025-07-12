#include "scale.h"
#include "stdint.h"

char *scale_to_string[END_SCALE_LIST] = {"Major", "Major Harmonic", "Blues", "Minor Natural", "Minor Harmonic", "Minor Melodic"};

char *tone_to_string[12] = {"Do", "DoD", "Re", "ReD", "Mi", "Fa", "FaD", "Sol", "SolD", "La", "LaD", "Si"};

static uint8_t intervals[END_SCALE_LIST][8] = {
    {DO, RE, MI, FA, SOL, LA, SI, DO + 12},     // MAJOR
    {DO, RE, MI, FA, SOL, SOLd, SI, DO + 12},   // MAJOR_HARMONIC
    {DO, REd, MI, FAd, FAd, SOL, LA, DO + 12},  // BLUES
    {DO, RE, REd, FA, SOL, SOLd, LAd, DO + 12}, // MINOR_NATURAL
    {DO, RE, REd, FA, SOL, SOLd, SI, DO + 12},  // MINOR_HARMONIC
    {DO, RE, REd, FA, SOL, LA, SI, DO + 12},    // MINOR_MELODIC_ASCENDING
};

// octave shifts everything by 12 semitones
// button from 0 to 7, "Do to Do" in if tone is Do
uint8_t button_to_midi(uint8_t octave, scale_t scale, tone_t tone, uint8_t button)
{
    return tone + (octave * 12) + intervals[scale][button];
}