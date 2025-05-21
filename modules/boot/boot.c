#include "boot/boot.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/boot.h>

void boot_reboot(uint8_t reboot_mode) {
  // Reboot the application.
  if (reboot_mode == 0) {
    asm volatile ("jmp 0x0000");
  } else {
    // Reboot the bootloader.
    asm volatile ("jmp 0x3C00");
  }
  __builtin_unreachable();
}

uint32_t boot_get_page_size() {
  // Return the page size of the flash memory.
  return SPM_PAGESIZE;
}

void boot_flash_erase(uint32_t page) {
  // Erase the flash page.
  uint8_t sreg = SREG;
  cli(); // Disable interrupts.
  eeprom_busy_wait();
  boot_page_erase(page);
  boot_spm_busy_wait(); // Wait until the memory is erased.
  SREG = sreg; // Re-enable interrupts.
}

void boot_flash_program(uint32_t page, const uint8_t *buf) {
  // Program the flash page.
  uint16_t i;
  uint8_t sreg;

  // Disable interrupts.

  sreg = SREG;
  cli();

  eeprom_busy_wait ();

  for (i=0; i<SPM_PAGESIZE; i+=2)
  {
      // Set up little-endian word.

      uint16_t w = *buf++;
      w += (*buf++) << 8;

      boot_page_fill (page + i, w);
  }

  boot_page_write (page);     // Store buffer in flash page.
  boot_spm_busy_wait();       // Wait until the memory is written.

  // Reenable RWW-section again. We need this if we want to jump back
  // to the application after bootloading.

  boot_rww_enable ();

  // Re-enable interrupts (if they were ever enabled).

  SREG = sreg;
}

REGISTER_MODULE(boot, BOOT_MODULE_ID, BOOT_FUNCTION_EXPORTS, BOOT_API_VER);