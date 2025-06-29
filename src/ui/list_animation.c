#include "ui/list_animation.h"
#include "stdint.h"
#include "input.h"
#include "stdbool.h"

uint8_t animate(list_animation_t *state, int start, int end)
{
    if (state->old_selection == state->selected)
    {
        return end;
    }

    uint8_t ret = start + (((end - start) * state->animation_frame) / LIST_ANIMATION_STEPS);
    state->animation_frame++;
    if (state->animation_frame == LIST_ANIMATION_STEPS)
    {
        state->old_selection = state->selected;
        state->animation_frame = 0;
    }
    return ret;
}

// Check if we need to animate the list up/down
// If that's the case, return true.
// The callee is responsible to draw the ui for the next N frames
bool animate_list(list_animation_t *state, uint8_t limit)
{
    if (was_key_pressed(UP))
    {
        state->old_selection = state->selected;
        state->selected = (state->selected + limit - 1) % limit;
        state->animation_frame = 0;
        return true;
    }
    else if (was_key_pressed(DOWN))
    {
        state->old_selection = state->selected;
        state->selected = (state->selected + 1) % limit;
        state->animation_frame = 0;
        return true;
    }
    return false;
}