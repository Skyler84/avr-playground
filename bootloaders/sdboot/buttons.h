#pragma once
#include <stdint.h>


enum _btn_id_t {
    BTN_C = 0,
    BTN_N,
    BTN_E, 
    BTN_S,
    BTN_W,
};

extern void buttons_init();
extern int8_t is_button_pressed(enum _btn_id_t btn_id);
extern int8_t is_button_released(enum _btn_id_t btn_id);
extern int8_t button_clicked(enum _btn_id_t btn_id);
extern void wait_button_press(enum _btn_id_t btn_id);
extern void wait_button_release(enum _btn_id_t btn_id);
extern void wait_button_click(enum _btn_id_t btn_id);
