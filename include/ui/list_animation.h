#ifndef LIST_ANIMATION_H
#define LIST_ANIMATION_H

#include "stdint.h"
#include "stdbool.h"

#define LIST_ANIMATION_STEPS 5

typedef struct
{
    uint8_t old_selection; // Previous selected menu item index
    uint8_t selected;      // Index of the currently selected menu item
    uint8_t animation_frame;
} list_animation_t;

uint8_t animate(list_animation_t *state, int start, int end);
bool animate_list(list_animation_t *state, uint8_t limit);

#endif // LIST_ANIMATION_H