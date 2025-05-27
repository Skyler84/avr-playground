#pragma once

#include "module/module.h"

#include <stdbool.h>

enum EventType {
  EVENT_TYPE_NONE = 0,
  EVENT_TYPE_BUTTON,
  EVENT_TYPE_SCROLL,
};

#define BUTTON_EVENT_PRESS 1
#define BUTTON_EVENT_RELEASE 0


typedef struct InputEvent {
  uint8_t type; // EventType
  uint8_t id; // Button number (0-7 for buttons, 0 for touch, 1 for joystick)
  int16_t value; // Depends on EventType.

} InputEvent_t;

typedef struct inputs inputs_t;

enum BtnID {
  BTN_ID_NONE = 0,
  BTN_ID_UP,
  BTN_ID_DOWN,
  BTN_ID_LEFT,
  BTN_ID_RIGHT,
  BTN_ID_CENTER,
  BTN_ID_A,
  BTN_ID_B,
  BTN_ID_C,
  BTN_ID_D,
};

typedef struct inputs inputs_t;

#define INPUTS_FUNCTIONS_EXPORT(modname, o) \
  o(modname, init          , void         , inputs_t* ) \
  o(modname, update        , void         , inputs_t* ) \
  o(modname, check_overflow, bool         , inputs_t* ) \
  o(modname, check_empty   , bool         , inputs_t* ) \
  o(modname, get_event     , InputEvent_t , inputs_t* )

#define INPUTS_MODULE_ID 0x01f0

DECLARE_MODULE(inputs, INPUTS_MODULE_ID, INPUTS_FUNCTIONS_EXPORT);

struct inputs
{
  inputs_fns_t *fns;
  InputEvent_t event_buffer[16]; // circular Buffer for events
  uint8_t event_count; // Number of events in the buffer
  uint8_t event_index; // Current index for reading events
  bool overflow;
};