#pragma once
#include <stdint.h>


enum btn_id_t {
    BTN_C = 0,
    BTN_N,
    BTN_E, 
    BTN_S,
    BTN_W,
};

extern void buttons_init();
extern int8_t is_button_pressed(enum btn_id_t btn_id);
extern int8_t is_button_released(enum btn_id_t btn_id);
extern int8_t button_clicked(enum btn_id_t btn_id);
extern void wait_button_press(enum btn_id_t btn_id);
extern void wait_button_release(enum btn_id_t btn_id);
extern void wait_button_click(enum btn_id_t btn_id);
