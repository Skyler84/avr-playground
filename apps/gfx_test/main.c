#include "lcd/lcd.h"
#include "gfx/gfx.h"
#include "module/imports.h"

// #include "softserial.h"

#include <avr/io.h>
#include <util/delay.h>


MODULE_IMPORT_FUNCTIONS(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS);

int main() {
  
  // set clock prescaler to 1
  CLKPR = _BV(CLKPCE); // Enable change
  CLKPR = 0; // Set prescaler to 1

  lcd_t lcd;
  lcd.fns = &lcd_fns;
  gfx_t gfx;
  gfx.fns = &gfx_fns;

  MODULE_CALL_THIS(display, init, &lcd.display);
  // MODULE_CALL_THIS(lcd, set_orientation, &lcd, West);
  MODULE_CALL_THIS(gfx, init, &gfx, &lcd.display);
  MODULE_CALL_THIS(gfx, fill, &gfx, RED);
  MODULE_CALL_THIS(gfx, nostroke, &gfx);

  display_region_t r = {
    .x1 = 10,
    .y1 = 10,
    .x2 = 20,
    .y2 = 30,
  };
  MODULE_CALL_THIS(gfx, rectangle, &gfx, r);
  MODULE_CALL_THIS(gfx, stroke, &gfx, GREEN);
  MODULE_CALL_THIS(gfx, strokeWeight, &gfx, 3);
  MODULE_CALL_THIS(gfx, line, &gfx, (gfx_coord_t){110, 110}, (gfx_coord_t){120, 130});
  MODULE_CALL_THIS(gfx, rectangle, &gfx, (gfx_region_t){50, 50, 100, 100});
  MODULE_CALL_THIS(gfx, triangle, &gfx, (gfx_coord_t){150, 150}, (gfx_coord_t){200, 150}, (gfx_coord_t){150, 180});
  DDRB = 0x80;
  while(1) {
    PINB = 0x80;
    _delay_ms(1000);
  }
}