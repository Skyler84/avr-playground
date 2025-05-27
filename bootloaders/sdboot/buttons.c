#include "buttons.h"
#include <avr/io.h>
#include <util/delay.h>

struct btn_t
{
    volatile uint8_t *port;
    uint8_t bit;
};

static const struct btn_t btns[] = {
    {&PINE, 0x80}, // BTN_C
    {&PINC, 0x04}, // BTN_N
    {&PINC, 0x08}, // BTN_E
    {&PINC, 0x10}, // BTN_W
    {&PINC, 0x20}, // BTN_S
};

int8_t is_button_pressed(enum _btn_id_t btn_id)
{
    return !(*(btns[btn_id].port) & btns[btn_id].bit);
}

int8_t is_button_released(enum _btn_id_t btn_id)
{
    return !is_button_pressed(btn_id);
}

int8_t button_clicked(enum _btn_id_t btn_id)
{
    // check if button is clicked
    if (is_button_released(btn_id))
    {
        return 0;
    }
    wait_button_release(btn_id);
    return 1;
}

void wait_button_press(enum _btn_id_t btn_id)
{
    int timeout = 0;
    while (timeout++ < 100)
    {
        // wait for button press
        if (is_button_released(btn_id))
            timeout = 0;
        _delay_us(100);
    }
}

void wait_button_release(enum _btn_id_t btn_id)
{
    int timeout = 0;
    while (timeout++ < 100)
    {
        // wait for button release
        if (is_button_pressed(btn_id))
            timeout = 0;
        _delay_us(100);
    }
}

void wait_button_click(enum _btn_id_t btn_id)
{
    // wait for button press
    wait_button_press(btn_id);
    // wait for button release
    wait_button_release(btn_id);
}
