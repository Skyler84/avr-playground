#include "buttons/buttons.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "module/pic.h"

MODULE_FN_PROTOS(buttons, BUTTONS_FUNCTION_EXPORTS);

typedef struct btn
{
    volatile uint8_t *ddr;
    volatile uint8_t *port;
    volatile uint8_t *pin;
    uint8_t bitmask;
}btn_t;

static const btn_t btns[] PROGMEM = {
    {&DDRE, &PORTE, &PINE, 0x80}, // BTN_C
    {&DDRC, &PORTC, &PINC, 0x04}, // BTN_N
    {&DDRC, &PORTC, &PINC, 0x08}, // BTN_E
    {&DDRC, &PORTC, &PINC, 0x10}, // BTN_W
    {&DDRC, &PORTC, &PINC, 0x20}, // BTN_S
};

static void button_get(btn_id_t btn_id, btn_t *btn)
{
  uint_farptr_t module_offset = GET_MODULE_DATA_PTR_OFFSET();
    btn->ddr = (volatile uint8_t *)pgm_read_word_far(((uintptr_t)&btns[btn_id].ddr) + module_offset);
    btn->port = (volatile uint8_t *)pgm_read_word_far(((uintptr_t)&btns[btn_id].port) + module_offset);
    btn->pin = (volatile uint8_t *)pgm_read_word_far(((uintptr_t)&btns[btn_id].pin) + module_offset);
    btn->bitmask = pgm_read_byte_far(((uintptr_t)&btns[btn_id].bitmask) + module_offset);
}

void buttons_init() {
  btn_t btn;
  for (uint8_t i = 0; i < 5; i++) {
    button_get(i, &btn);
    // Set button pin as input
    *btn.ddr &= ~btn.bitmask; 
    // Enable pull-up resistor
    *btn.port |= btn.bitmask;
  }
}

int8_t buttons_is_pressed(btn_id_t btn_id)
{
  btn_t btn;
  button_get(btn_id, &btn);

  return !(*(btn.pin) & btn.bitmask);
}

int8_t buttons_is_released(btn_id_t btn_id)
{
    return !buttons_is_pressed(btn_id);
}

int8_t buttons_clicked(btn_id_t btn_id)
{
    // check if button is clicked
    if (buttons_is_released(btn_id))
    {
        return 0;
    }
    buttons_wait_for_release(btn_id);
    return 1;
}

void buttons_wait_for_press(btn_id_t btn_id)
{
    int timeout = 0;
    while (timeout++ < 100)
    {
        // wait for button press
        if (buttons_is_released(btn_id))
            timeout = 0;
        _delay_us(100);
    }
}

void buttons_wait_for_release(btn_id_t btn_id)
{
    int timeout = 0;
    while (timeout++ < 100)
    {
        // wait for button release
        if (buttons_is_pressed(btn_id))
            timeout = 0;
        _delay_us(100);
    }
}

void buttons_wait_for_click(btn_id_t btn_id)
{
    buttons_wait_for_press(btn_id);
    buttons_wait_for_release(btn_id);
}


REGISTER_MODULE(buttons, BUTTONS_MODULE_ID, BUTTONS_FUNCTION_EXPORTS, BUTTONS_API_VER);