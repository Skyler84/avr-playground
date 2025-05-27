#include "module/imports.h"

#include "lcd/lcd.h"
#include "gfx/gfx.h"
#include "font/font5x7.h"

#include <avr/io.h>
#include <avr/power.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/boot.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

MODULE_IMPORT_FUNCTIONS(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(font5x7, FONT5X7_MODULE_ID, FONT5X7_FUNCTION_EXPORTS);

lcd_t lcd = {
    .fns = &lcd_fns,
};

gfx_t gfx = {
    .display = &lcd.display,
    .fns = &gfx_fns,
};

font5x7_t font = {
    .fns = &font5x7_fns,
};

int main()
{
  clock_prescale_set(clock_div_1);

  MODULE_CALL_THIS(gfx, init, &gfx, &lcd.display);
  MODULE_CALL_THIS(lcd, set_orientation, &lcd, West);
  MODULE_CALL_THIS(gfx, fill, &gfx, BLACK);
  MODULE_CALL_THIS(gfx, nostroke, &gfx);
  MODULE_CALL_THIS(gfx, rectangle, &gfx, (display_region_t){0, 0, 320, 240});
  MODULE_CALL_THIS(gfx, fill, &gfx, WHITE);
  MODULE_CALL_THIS(gfx, textFont, &gfx, &font.base);
  MODULE_CALL_THIS(gfx, textSize, &gfx, 24);
  uint8_t line_no = 1;
  MODULE_CALL_THIS(gfx, text, &gfx,
                   ((display_region_t){0, 20 * line_no++, 319 - 2 * 2, 239 - 2 * 2}),
                   "RWW Test Bootloader");


  // uint32_t page = 0x00000; // Page to erase, adjust as needed.
  uint32_t page = 0x1f800; // Page to erase, adjust as needed.

  uint8_t sreg = SREG;
  cli(); // Disable interrupts.
  eeprom_busy_wait();
  boot_page_erase(page);
  // boot_spm_busy_wait(); // Wait until the memory is erased.
  SREG = sreg; // Re-enable interrupts.
                 
  if (SPMCSR & _BV(RWWSB)) {
    boot_spm_busy_wait();
    boot_rww_enable(); // Re-enable RWW section.
    MODULE_CALL_THIS(gfx, text, &gfx,
                     ((display_region_t){0, 20 * line_no++, 319 - 2 * 2, 239 - 2 * 2}),
                     "RWW section is busy");
  } else {
    MODULE_CALL_THIS(gfx, text, &gfx,
                     ((display_region_t){0, 20 * line_no++, 319 - 2 * 2, 239 - 2 * 2}),
                     "RWW section is not busy");
  }

  while (1)
    ;
  __builtin_unreachable();
}