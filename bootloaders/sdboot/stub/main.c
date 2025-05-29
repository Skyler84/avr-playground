#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <alloca.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/power.h>

#include "module/imports.h"
#include "font/font5x7.h"
#include "lcd/lcd.h"
#include "sd/sd.h"
#include "fat/fat.h"
#include "blockdev/blockdev.h"
#include "boot/boot.h"
#include "buttons/buttons.h"
#include "gui/gui.h"

#include "sdboot/interactive.h"




void blink(uint8_t n) {
  PORTB &= ~0x80;
  _delay_ms(1000);
  n*=2;
  for (uint8_t i = 0; i < n; i++) {
    PINB = 0x80;
    _delay_ms(200);
  }
  _delay_ms(1000);
}

void __attribute__((noreturn)) error(uint8_t code) {
  while(1) {
    blink(code);
  }
  __builtin_unreachable();
}

void __attribute__((noreturn)) boot_from_file(const char *filename, uint32_t load_addr) {
  (void)filename;
  (void)load_addr;
  // MOD_CALL(sd, &sd_fns, preinit);
  // MOD_CALL(sd, &sd_fns, initialise);
  // sd_fns.preinit();
  // sd_fns.initialise();

  // open file
  // program file to flash
  // ((void(*)())0x0000)();
  boot_reboot(REBOOT_MODE_APP);
  __builtin_unreachable();
}

int8_t partitions_init(BlockDev *root_bd, BlockDev *partition_bd) {
  uint8_t buf[512];
  blockdev_read_sector(root_bd, 0, buf);
  uint16_t magic = *((uint16_t*)&buf[510]);
  if (magic != 0xAA55) {
    return -1;
  }
  uint32_t start_sector = *((uint32_t*)(buf + 0x1BE + 8));
  uint32_t sector_count = *((uint32_t*)(buf + 0x1BE + 12));
  if (blockdev_partition(root_bd, partition_bd, start_sector, sector_count) != 0) {
    // error partitioning blockdev
    return -2;
  }
  blockdev_read_sector(partition_bd, 0, buf);
  magic = *((uint16_t*)&buf[510]);
  if (magic != 0xAA55) {
    return -3;
  }
  return 0;
}

typedef union{
  struct{
    sd_fns_t *sd_fns;
    buttons_fns_t *buttons_fns;
    gui_fns_t *gui_fns;
    gfx_fns_t *gfx_fns;
    font5x7_fns_t *font_fns;
    boot_fns_t *boot_fns;
    fat_fns_t *fat_fns;
  };

}sdboot_modules_t;

void populate_modules(sdboot_modules_t *modules) {
  if (modules->sd_fns) MODULE_IMPORT_FUNCTIONS_RUNTIME(sd, SD_MODULE_ID, SD_FUNCTION_EXPORTS, modules->sd_fns);
  if (modules->buttons_fns) MODULE_IMPORT_FUNCTIONS_RUNTIME(buttons, BUTTONS_MODULE_ID, BUTTONS_FUNCTION_EXPORTS, modules->buttons_fns);
  if (modules->gui_fns) MODULE_IMPORT_FUNCTIONS_RUNTIME(gui, GUI_MODULE_ID, GUI_FUNCTION_EXPORTS, modules->gui_fns);
  if (modules->gfx_fns) MODULE_IMPORT_FUNCTIONS_RUNTIME(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS, modules->gfx_fns);
  if (modules->font_fns) MODULE_IMPORT_FUNCTIONS_RUNTIME(font5x7, FONT5X7_MODULE_ID, FONT5X7_FUNCTION_EXPORTS, modules->font_fns);
  if (modules->boot_fns) MODULE_IMPORT_FUNCTIONS_RUNTIME(boot, BOOT_MODULE_ID, BOOT_FUNCTION_EXPORTS, modules->boot_fns);
  if (modules->fat_fns) MODULE_IMPORT_FUNCTIONS_RUNTIME(fat, FAT_MODULE_ID, FAT_FUNCTION_EXPORTS, modules->fat_fns);

}


void flash_program_filedescriptor(FileSystem_t *fs, file_descriptor_t fd, uint_farptr_t addr) {
  
  boot_fns_t boot_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME(boot, BOOT_MODULE_ID, BOOT_FUNCTION_EXPORTS, &boot_fns);

  uint32_t page_size = MODULE_CALL_FNS(boot, get_page_size, &boot_fns);
  uint8_t *page_buf = alloca(page_size);
  fstatus_t ret;
  while(1) {
    ret = MODULE_CALL_THIS(fs, read, fs, fd, (char*)page_buf, page_size);
    MODULE_CALL_FNS(boot, flash_erase, &boot_fns, addr);
    MODULE_CALL_FNS(boot, flash_program, &boot_fns, addr, page_buf);
    addr += page_size;
    if (ret < (int)page_size) {
      break;
    }
    if (addr >= 0x1e000UL) {
      break;
    }
  }
}


void install_modules() {
  blink(5);
  blink(5);
  fat_fns_t fat_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME_STATIC(fat, FAT_MODULE_ID, FAT_FUNCTION_EXPORTS, &fat_fns);

  FAT_FileSystem_t fs = {
    .fns = &fat_fns,
  };
  
  blockdev_sector_fns_t sd_bd_fns;
  sd_bd_fns.read_sector  = (bd_read_sector_fn_t )sd_rdblock;
  sd_bd_fns.write_sector = (bd_write_sector_fn_t)sd_wrblock;
  
  BlockDev root_bd = {
    .fns = &sd_bd_fns,
    .fn_ctx = NULL,
    .sector_start = 0,
    .sector_count = -1,
  };

  BlockDev partition_bd = {
    .fns = &sd_bd_fns,
    .fn_ctx = NULL,
    .sector_start = 0,
    .sector_count = 0,
  };
  sd_preinit();
  while (!sd_detected()) {
    // wait for card to be inserted
    _delay_ms(100);
    // blink led
    PINB = 0x80;
  }
  PORTB &= ~0x80;
  sd_initialise();
  partitions_init(&root_bd, &partition_bd);
  fat_init(&fs, &partition_bd);
  fat_mount(&fs.fs, false, false);
  uint_farptr_t install_addr = MODULE_START_ADDRESS;
  // file_descriptor_t fd = fat_open(&fs.fs, "/sdboot", O_RDONLY);
  file_descriptor_t fd = fat_open(&fs.fs, "/sdboot.boot.mods.bin", O_RDONLY);
  // file_descriptor_t fd = fat_open(&fs.fs, "/lafortuna/bootloaders/sdboot/sdboot.boot.mods.bin", O_RDONLY);
  if (fd < 0) {
    error(2); 
  }
  flash_program_filedescriptor(&fs.fs, fd, install_addr);
  fat_close(&fs.fs, fd);
  blink(2);
}

void bootstrap() {
  // check if we have all required modules.
  // if not, reinstall them all!

  // we already should have FAT/SD/BOOT/MODULE modules included.
  module_id_t modules[] = {
    LCD_MODULE_ID,
    GFX_MODULE_ID,
    FONT5X7_MODULE_ID,
    BUTTONS_MODULE_ID,
  };
  
  for (uint8_t i = 0; i < sizeof(modules) / sizeof(module_id_t); i++) {
    if (module_find_by_id(modules[i]) == 0UL) {
      // reinstall ALL modules.
      install_modules();
      break;
    }
  }
}

int main() {
  // disable watchdog
  // check if extreset
  if ((MCUSR & _BV(WDRF)) || ((MCUSR & _BV(EXTRF)) == 0)) {
    MCUSR = ~_BV(WDRF); // clear watchdog reset flag
    wdt_disable();
    asm("jmp 0x0000");
    __builtin_unreachable();
  }
  wdt_disable();

  // enable boot ivsel
  MCUCR |= _BV(IVCE);
  MCUCR = _BV(IVSEL); // set bootloader vector

  // // Set clock prescaler to 1
  clock_prescale_set(clock_div_1);
  RAMPZ = 1;

  DDRB = 0x80;
  PORTB |= 0x80;

  bootstrap();

  sdboot_interactive_fns_t sdboot_interactive_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME(sdboot_interactive, SDBOOT_INTERACTIVE_MODULE_ID, SDBOOT_INTERACTIVE_FUNCTION_EXPORTS, &sdboot_interactive_fns);
  module_fns_t module_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME(module, MODULE_MODULE_ID, MODULE_FUNCTION_EXPORTS, &module_fns);

  MODULE_CALL_FNS(sdboot_interactive, run, &sdboot_interactive_fns, &module_fns);
  while(1);
  __builtin_unreachable();
}