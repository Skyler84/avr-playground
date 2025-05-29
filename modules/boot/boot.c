#include "boot/boot.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/boot.h>
#include <avr/wdt.h>

typedef enum fuse_byte{
  fuse_low_byte = 0x0000,
  fuse_lock_byte = 0x0001,
  fuse_extended_byte = 0x0002,
  fuse_high_byte = 0x0003,
} fuse_byte_t;

static uint8_t get_fuse_bits(fuse_byte_t fuse_byte) {
  uint8_t out;
  asm volatile(
    "out %[spmcsr], %[spm_bits]\n\t"
    "lpm\n\t"
    "mov %[out], r0\n\t"
    : [out] "=r" (out)
    : [spm_bits] "r" (_BV(BLBSET) | _BV(SPMEN))
    , [spmcsr] "I" (_SFR_IO_ADDR(SPMCSR))
    , "z" (fuse_byte)
    : "r0"
  );
  return out;
}

typedef enum boot_size{
  boot_size_512_words,
  boot_size_1024_words,
  boot_size_2048_words,
  boot_size_4096_words,
} boot_size_t;

static boot_size_t get_boot_size() {
  uint8_t hfuse = get_fuse_bits(fuse_high_byte);
  switch(hfuse & 0x06) {
    case 0x00: return boot_size_4096_words; // 8K bootloader
    case 0x02: return boot_size_2048_words; // 4K bootloader
    case 0x04: return boot_size_1024_words; // 2K bootloader
    case 0x06: return boot_size_512_words;  // 1K bootloader
  }
  __builtin_unreachable();
}

static uint16_t boot_size_to_bytes(boot_size_t size) {
  switch(size) {
    case boot_size_512_words: return 1024; // 1K bootloader
    case boot_size_1024_words: return 2048; // 2K bootloader
    case boot_size_2048_words: return 4096; // 4K bootloader
    case boot_size_4096_words: return 8192; // 8K bootloader
    default: return 0;
  }
  __builtin_unreachable();
}

void boot_reboot(reboot_mode_t reboot_mode) {
  // Reboot the application.
  if (reboot_mode == REBOOT_MODE_APP) {
    #if 0
    asm volatile (
      "ijmp\n\t"
      :: "z" (0) // Jump to the application start address (0x0000).
    );
    #else
    wdt_enable(WDTO_250MS);
    // clear reset flags
    MCUSR = 0;
    while(1);
    #endif
  } else {
    // Reboot the bootloader.
    uint_farptr_t boot_start = 0x20000UL - boot_size_to_bytes(get_boot_size());
    asm volatile (
      "ijmp\n\t"
      ::"z" ((uintptr_t)(boot_start / 2))
    ); // Jump to the bootloader start address.
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
  boot_rww_enable ();
  // Re-enable interrupts (if they were ever enabled).
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