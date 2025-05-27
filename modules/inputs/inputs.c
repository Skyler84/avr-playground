#include "inputs/inputs.h"

void inputs_init(inputs_t *inputs)
{
    inputs->event_count = 0;
    inputs->event_index = 0;

    //setup hardware specific interrupts etc.
}

void inputs_update(inputs_t *inputs)
{
    // This function should be implemented to read the actual input events
    // and populate the event_buffer.
    // For now, we will just simulate an empty update.
    inputs->event_count = 0;
    inputs->event_index = 0;
}

bool inputs_check_overflow(inputs_t *inputs)
{
    bool ret = inputs->overflow;
    inputs->overflow = false; // Reset overflow flag after checking
    return ret;
}

bool inputs_check_empty(inputs_t *inputs)
{
    return inputs->event_count == 0;
}

InputEvent_t inputs_get_event(inputs_t *inputs)
{
    if (inputs->event_count == 0) {
        // No more events to return
        InputEvent_t empty_event = {0};
        return empty_event;
    }
    
    // Get the next event from the buffer
    InputEvent_t event = inputs->event_buffer[inputs->event_index];
    inputs->event_index++;
    inputs->event_count--;

    if (inputs->event_index >= 16) {
        // Reset index if we reach the end of the buffer
        inputs->event_index = 0;
    }
    return event;
}

static void inputs_put_event(inputs_t *inputs, InputEvent_t event)
{
    if (inputs->event_count < 16) {
        // Add the event to the buffer
        inputs->event_buffer[(inputs->event_index + inputs->event_count) % 16] = event;
        inputs->event_count++;
    } else {
        // Buffer is full, set overflow flag
        inputs->overflow = true;
    }
}