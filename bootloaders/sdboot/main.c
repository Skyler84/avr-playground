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

#include "main.h"

#include "module/imports.h"
#include "font/font5x7.h"
#include "lcd/lcd.h"
#include "sd/sd.h"
#include "fat/fat.h"
#include "blockdev/blockdev.h"
#include "boot/boot.h"
#include "buttons/buttons.h"
#include "buttons.h"
#include "gui/gui.h"



void __attribute__((noreturn)) app_reboot() {
  // reboot using watchdog
#if 1
  wdt_enable(WDTO_250MS);
  // clear reset flags
  MCUSR = 0;
  memset((void*)0x0100, 0, RAMEND - 0x0100);
  while(1);
#else
  // reboot using reset
  asm volatile ("jmp 0x0000");
#endif
  __builtin_unreachable();
}

void __attribute__((noreturn)) bl_reboot() {
  // reboot bootloader?
  asm volatile("jmp 0x1e000"::);
  __builtin_unreachable();
}

void __attribute__((noreturn)) error() {
  bl_reboot();
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
  app_reboot();
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

typedef struct{
  sd_fns_t *sd_fns;
  buttons_fns_t *buttons_fns;
  gui_fns_t *gui_fns;
  gfx_fns_t *gfx_fns;
  font5x7_fns_t *font_fns;
  boot_fns_t *boot_fns;
  fat_fns_t *fat_fns;
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

void __attribute__((noreturn)) sd_boot(gfx_t *gfx) {
  MODULE_CALL_THIS(gfx, nostroke, gfx);
  MODULE_CALL_THIS(gfx, fill, gfx, BLACK);
  MODULE_CALL_THIS(gfx, rectangle, gfx, ((display_region_t){0, 0, 320, 240}));
  MODULE_CALL_THIS(gfx, fill, gfx, WHITE);
  MODULE_CALL_THIS(gfx, text, gfx, ((display_region_t){0, 20, 319, 239}), "Booting from SD card");

  uint_farptr_t msgp;

  sd_fns_t sd_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME(sd, SD_MODULE_ID, SD_FUNCTION_EXPORTS, &sd_fns);

  buttons_fns_t buttons_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME(buttons, BUTTONS_MODULE_ID, BUTTONS_FUNCTION_EXPORTS, &buttons_fns);

  MODULE_CALL_FNS(sd, preinit, &sd_fns);

  gui_fns_t gui_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME(gui, GUI_MODULE_ID, GUI_FUNCTION_EXPORTS, &gui_fns);

  GUI_t gui = {
    .fns = &gui_fns,
    .gfx = gfx,
    .buttons_fns = &buttons_fns,
  };

  MODULE_CALL_THIS(gui, init, &gui);

  while(!sd_fns.sd_detected()) {
    MODULE_CALL_THIS(gui, msgbox, &gui,"Insert card", MSGBOX_OK);
    PINB = 0x80;
    // wait for card to be inserted
    _delay_ms(100);
  }
  MODULE_CALL_THIS(gfx, fill, gfx, WHITE);
  MODULE_CALL_THIS(gfx, textSize, gfx, 24);
  MODULE_CALL_THIS(gfx, text, gfx, ((display_region_t){0, 40, 319, 239}), "SD card detected");

  blockdev_sector_fns_t sd_bd_fns;
  sd_bd_fns.read_sector  = (bd_read_sector_fn_t )sd_fns.sd_rdblock;
  sd_bd_fns.write_sector = (bd_write_sector_fn_t)sd_fns.sd_wrblock;
  
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

  {
    uint8_t errno;
    if ((errno = MODULE_CALL_FNS(sd, initialise, &sd_fns)) != 0) {
      static const char msg[] = "Error initializing card";
      msgp = pgm_get_far_address(msg);
      goto end;
    }
    if (partitions_init(&root_bd, &partition_bd) != 0) {
      static const char msg[] PROGMEM = "Error initializing partitions";
      msgp = pgm_get_far_address(msg);
      goto end;
    }
  }
  {
    fat_fns_t fat_fns;
    MODULE_IMPORT_FUNCTIONS_RUNTIME(fat, FAT_MODULE_ID, FAT_FUNCTION_EXPORTS, &fat_fns);

    FAT_FileSystem_t fs;
    fs.fns = &fat_fns;
    MODULE_CALL_THIS(fat, init, &fs, &partition_bd);
    fstatus_t ret;
    ret = MODULE_CALL_THIS(fs, mount, &fs.fs, false, false);
    if (ret != 0) {
      static const char msg[] PROGMEM = "Error mounting filesystem";
      msgp = pgm_get_far_address(msg);
      goto end;
    }

    file_descriptor_t fd = MODULE_CALL_THIS(gui, choose_file, &gui, &fs.fs, "/");
    // file_descriptor_t fd = gui_choose_file(&gui, &fs.fs, "/lafortuna/apps/");
    MODULE_CALL_THIS(gui, msgbox, &gui,"Boot from file", MSGBOX_OK);
    PINB = 0x80;

    flash_program_filedescriptor(&fs.fs, fd, 0x0000UL);

    for (int i = 0;i < 4; i++){
      _delay_ms(200);
      PORTB |= 0x80;
      _delay_ms(200);
      PORTB &= ~0x80;
    }

    app_reboot();
  }

  


end:
  MODULE_CALL_THIS(gui, msgboxP, &gui, msgp, MSGBOX_OK);
  error();
while(1);
}

extern void blink_bits(uint32_t value, uint8_t nbits);


static void __attribute__((noreturn)) run_interactive() {
  lcd_fns_t lcd_fns;
  gfx_fns_t gfx_fns;
  font5x7_fns_t font_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS, &lcd_fns);
  MODULE_IMPORT_FUNCTIONS_RUNTIME(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS, &gfx_fns);
  MODULE_IMPORT_FUNCTIONS_RUNTIME(font5x7, FONT5X7_MODULE_ID, FONT5X7_FUNCTION_EXPORTS, &font_fns);


  lcd_t lcd = {
    .fns = &lcd_fns,
  };
  gfx_t gfx = {
    .display = &lcd.display,
    .fns = &gfx_fns,
  };
  font5x7_t font = {
    .fns = &font_fns,
  };

  buttons_fns_t buttons_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME(buttons, BUTTONS_MODULE_ID, BUTTONS_FUNCTION_EXPORTS, &buttons_fns);
  MODULE_CALL_FNS(buttons, init, &buttons_fns);

  const char *menu[] = {
    ("Boot from SD file"),
    ("Reboot application")
  };

  // const char *menu[] = {
  //   ("1"),
  //   ("2")
  // };

  typedef __attribute__((noreturn)) void (*menu_func_t)(gfx_t *gfx);
  menu_func_t menu_func[] = {
    sd_boot,
    app_reboot
  };

  MODULE_CALL_THIS(gfx, init, &gfx, &lcd.display);
  MODULE_CALL_THIS(lcd, set_orientation, &lcd, West);
  MODULE_CALL_THIS(gfx, fill, &gfx, WHITE);
  MODULE_CALL_THIS(gfx, nostroke, &gfx);
  MODULE_CALL_THIS(font5x7, init, &font);
  MODULE_CALL_THIS(gfx, textFont, &gfx, &font.base);
  MODULE_CALL_THIS(gfx, textSize, &gfx, 24);
  MODULE_CALL_THIS(gfx, char, &gfx, (display_coord_t){0, 40}, '>');

  for (int i = 0; i < 2; i++) {
    MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){20, 40+i*20, 319, 60*i*20}), menu[i]);
  }
  PINB = 0x80;
  int8_t selection = 0;
  __attribute__((noreturn)) menu_func_t func = NULL;
  while (func == NULL) {
    wdt_reset();
    if (MODULE_CALL_FNS(buttons, clicked, &buttons_fns, BTN_ID_CENTER)) {
      func = menu_func[selection];
      break;
    }
    if (MODULE_CALL_FNS(buttons, clicked, &buttons_fns, BTN_ID_NORTH)) {
      selection--;
    } else if (MODULE_CALL_FNS(buttons, clicked, &buttons_fns, BTN_ID_SOUTH)) {
      selection++;
    } else {
      continue;
    }
    if (selection < 0) {
      selection = 0;
    } else if (selection > 1) {
      selection = 1;
    }
    MODULE_CALL_THIS(gfx, fill, &gfx, BLACK);
    MODULE_CALL_THIS(gfx, nostroke, &gfx);
    MODULE_CALL_THIS(gfx, rectangle, &gfx, (display_region_t){0, 20, 19, 40+2*20});
    MODULE_CALL_THIS(gfx, fill, &gfx, WHITE);
    MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 40+selection*20, 19, 60+selection*20}), ">");
    PINB = 0x80;
  }

  func(&gfx);
}

void bootstrap() {
  // check if we have all required modules.
  // if not, reinstall them all!

  // we already should have FAT/SD/BOOT modules included.
  module_id_t modules[] = {
    LCD_MODULE_ID,
    GFX_MODULE_ID,
    FONT5X7_MODULE_ID,
    BUTTONS_MODULE_ID,
  };
  // for (uint8_t i = 0; i < sizeof(modules) / sizeof(module_id_t); i++) {
  //   if (MODULE_IS_MISSING(modules[i])) {
  //     MODULE_INSTALL(modules[i]);
  //   }
  // }
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

  run_interactive();
  __builtin_unreachable();
}