#include <avr/io.h>
#include <avr/interrupt.h>

#include "module/imports.h"
#include "lcd/lcd.h"
#include "encoder.h"

MODULE_IMPORT_FUNCTIONS(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS)


#define COUNT_PER_PULSE 4 // Number of counts per pulse (depends on encoder type)
#define PULSE_PER_REV 12 // Number of pulses per revolution (depends on encoder type)
#define COUNT_PER_REV (COUNT_PER_PULSE * PULSE_PER_REV) // Total counts per revolution


void encoder_test() {
  cli();
  // encoder_pos += encoder_dt;
  encoder_dt = 0;
  sei();

}

void encoder_test_lcd() {

}

int main() {

  // disable watchdog
  MCUSR = ~_BV(WDRF); // clear watchdog reset flag
  WDTCSR = _BV(WDCE);
  WDTCSR = 0x00;

  // Set clock prescaler to 1
  CLKPR = 0x80; // Enable clock change
  CLKPR = 0x00; // Set clock prescaler to 1

  DDRB = 0xFF; // Set PORTB as output
  sei();
  void (*encoder_test_fptr)(void) = encoder_test;
  lcd_t lcd;
  lcd.fns = &lcd_fns;
  // if (lcd_fns.init != 0) {
    MODULE_CALL_THIS(display, init, &lcd.display);
    // encoder_test_fptr = encoder_test_lcd;
  // }
  while(1) {
    encoder_test_fptr();
  }
}