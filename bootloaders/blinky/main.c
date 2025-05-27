#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include "module/imports.h"
#include "boot/boot.h"

MODULE_IMPORT_FUNCTIONS(boot, BOOT_MODULE_ID, BOOT_FUNCTION_EXPORTS);

int main() 
{
  (void)boot_fns;
  if (MCUSR & _BV(EXTRF)) {
    MCUSR = ~_BV(EXTRF); // clear external reset flag
    wdt_disable(); // disable watchdog
    asm("jmp 0x0000"); // jump to application start
  }
  DDRB = 0x80;
  while(1) {
    PINB = 0x80;
    _delay_ms(100);
  }
  return 0;
}