#pragma once

#include "module/module.h"

typedef enum btn_id_t {
  BTN_ID_CENTER, // Center button
  BTN_ID_NORTH, // North button
  BTN_ID_EAST, // East button
  BTN_ID_SOUTH, // South button
  BTN_ID_WEST, // West button
} btn_id_t;

#define BUTTONS_FUNCTION_EXPORTS(modname, o) \
  o(modname, init, void) \
  o(modname, is_pressed, int8_t, btn_id_t) \
  o(modname, is_released, int8_t, btn_id_t) \
  o(modname, clicked, int8_t, btn_id_t) \
  o(modname, wait_for_press, void, btn_id_t) \
  o(modname, wait_for_release, void, btn_id_t) \
  o(modname, wait_for_click, void, btn_id_t)

#define BUTTONS_MODULE_ID 0x01f1
#define BUTTONS_API_VER 0x0001

DECLARE_MODULE(buttons, BUTTONS_MODULE_ID, BUTTONS_FUNCTION_EXPORTS);