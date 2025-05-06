#include "lcd/lcd.h"
#include "module/imports.h"
// #include "softserial.h"

#include <avr/io.h>
#include <util/delay.h>


MODULE_IMPORT_FUNCTIONS(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS);

void blink_bits(uint32_t bits, uint8_t count, uint8_t delay) {
  uint8_t _delay;
  for (uint8_t i = 0; i < count; i++) {
    PORTB = 0x80;

    _delay = delay;
    while(_delay--)_delay_ms(1);

    if (bits & 1<<i) {
      PORTB = 0x80;
    } else {
      PORTB = 0;
    }

    _delay = delay;
    while(_delay--)_delay_ms(1);
    _delay = delay;
    while(_delay--)_delay_ms(1);

    PORTB = 0;

    _delay = delay;
    while(_delay--)_delay_ms(1);  
  }
}

void module_runtime_init_error_handler(module_id_t, moduleptr_t*, uint16_t fn_id, fn_ptr_t *) {
  DDRB = 0x80;
  PORTB = 0;
  for (uint8_t i = 0; i < fn_id; i++) {
    PORTB |= 0x80;
    _delay_ms(400/8);
    PORTB &= ~0x80;
    _delay_ms(400/8);
  }
  _delay_ms(1200/8);
}

int main() {
  
  // set clock prescaler to 1
  CLKPR = _BV(CLKPCE); // Enable change
  CLKPR = 0; // Set prescaler to 1

  lcd_t lcd;
  lcd.fns = &lcd_fns;

  MODULE_CALL_THIS(display, init, &lcd.display);
  display_region_t r = {
    .x1 = 0,
    .y1 = 0,
    .x2 = 240,
    .y2 = 320
  };
  MODULE_CALL_THIS(display, region_set, &lcd.display, r);
  MODULE_CALL_THIS(display, fill, &lcd.display, RED, 240UL*320UL);
  display_coord_t coord = {0,0};
  display_coord_t velocity = {1, 1};
  display_coord_t box_size = {10, 10};
  display_colour_t colour = 0;
  while(1) {
    MODULE_CALL_THIS(display, region_set, &lcd.display, (display_region_t){coord.x, coord.y, coord.x + box_size.x, coord.y + box_size.y});
    MODULE_CALL_THIS(display, fill, &lcd.display, WHITE, box_size.x*box_size.y);
    _delay_ms(3);
    MODULE_CALL_THIS(display, fill, &lcd.display, colour++, box_size.x*box_size.y);
    coord.x += velocity.x;
    coord.y += velocity.y;
    if (coord.x < 0 || coord.x > 240 - box_size.x) {
      velocity.x = -velocity.x;
    }
    if (coord.y < 0 || coord.y > 320 - box_size.y) {
      velocity.y = -velocity.y;
    }
  }
}