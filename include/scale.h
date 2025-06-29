#ifndef SCALE_H
#define SCALE_H

#include "stdint.h"

typedef enum
{
    MAJOR = 0,
    MAJOR_HARMONIC,
    BLUES,
    MINOR_NATURAL,
    MINOR_HARMONIC,
    MINOR_MELODIC_ASCENDING,
    END_SCALE_LIST,
} scale_t;

extern char *scale_to_string[END_SCALE_LIST];

typedef enum
{
    DO = 0,
    DOd,
    RE,
    REd,
    MI,
    FA,
    FAd,
    SOL,
    SOLd,
    LA,
    LAd,
    SI,
} tone_t;

extern char *tone_to_string[12];

uint8_t button_to_midi(uint8_t octave, scale_t scale, tone_t tone, uint8_t button);

#endif // SCALE_H