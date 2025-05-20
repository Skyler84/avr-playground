#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/boot.h>

#include "main.h"
#include "encoder.h"
#include "buttons.h"
#include "gui.h"

#include "module/imports.h"
#include "font/font5x7.h"
#include "lcd/lcd.h"
#include "sd/sd.h"
#include "fat/fat.h"
#include "blockdev/blockdev.h"



void __attribute__((noreturn)) app_reboot() {
  // reboot using watchdog
#if 0
  WDTCSR = _BV(WDCE);
  WDTCSR = _BV(WDE); // 16ms
  while(1);
#else
  // reboot using reset
  asm volatile ("jmp 0x0000");
#endif
  __builtin_unreachable();
}

void __attribute__((noreturn)) bl_reboot() {
  // reboot bootloader?
  ((void(*)())0x0000)();
  __builtin_unreachable();
}

void __attribute__((noreturn)) error() {
  // gui_msgboxP(&gui,PSTR("Please eject card"), MSGBOX_OK);
  // while(sd_fns.detected()) wdt_reset();
  // reboot bootloader?
  bl_reboot();
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

int8_t partitions_init() {
//   sd_bd_fns.read_sector  = (bd_read_sector_fn_t )sd_fns.rdblock;
//   sd_bd_fns.write_sector = (bd_write_sector_fn_t)sd_fns.wrblock;
//   // initialize partitions
//   root_bd.fns = &sd_bd_fns;
//   root_bd.fn_ctx = NULL;
//   root_bd.sector_start = 0;
//   root_bd.sector_count = -1;

//   partition_bd.fns = &sd_bd_fns;
//   partition_bd.fn_ctx = NULL;
//   partition_bd.sector_start = 0;
//   partition_bd.sector_count = 0;

//   uint8_t buf[512];
//   blockdev_read_sector(&root_bd, 0, buf);
//   uint16_t magic = *((uint16_t*)&buf[510]);
//   // gui_msgboxP(&gui,PSTR("Reading MBR"), MSGBOX_OK);
//   if (magic != 0xAA55) {
//     // gui_msgboxP(&gui,PSTR("Error reading MBR"), MSGBOX_OK);
//     goto err;
//   }
//   uint32_t start_sector = *((uint32_t*)(buf + 0x1BE + 8));
//   uint32_t sector_count = *((uint32_t*)(buf + 0x1BE + 12));
//   if (blockdev_partition(&root_bd, &partition_bd, start_sector, sector_count) != 0) {
//     // error partitioning blockdev
//     // gui_msgboxP(&gui,PSTR("Error partitioning"), MSGBOX_OK);
//     goto err;
//   }
//   blockdev_read_sector(&partition_bd, 0, buf);
//   magic = *((uint16_t*)&buf[510]);
//   if (magic != 0xAA55) {
//     // gui_msgboxP(&gui,PSTR("Error reading MBR"), MSGBOX_OK);
//     goto err;
//   }
//   return 0;
// err:
//   return -1;
  return 0;
}

void flash_page_erase_program(uint32_t page, const uint8_t *buf) {

  uint16_t i;
  uint8_t sreg;

  // Disable interrupts.

  sreg = SREG;
  cli();

  eeprom_busy_wait ();

  boot_page_erase (page);
  boot_spm_busy_wait ();      // Wait until the memory is erased.

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

char hex[] = "0123456789ABCDEF";
lcd_t lcd;
void lcd_debug_u16(lcd_xcoord_t /* x */, lcd_ycoord_t /* y */, uint16_t /* val */) {
  // .display_char(x, y, 1, fonts_fns.get_default(), &fonts_fns, hex[(val>>12)&0x0f], WHITE);
  // .display_char(x+6, y, 1, fonts_fns.get_default(), &fonts_fns, hex[(val>>8)&0x0f], WHITE);
  // .display_char(x+12, y, 1, fonts_fns.get_default(), &fonts_fns, hex[(val>>4)&0x0f], WHITE);
  // .display_char(x+18, y, 1, fonts_fns.get_default(), &fonts_fns, hex[(val>>0)&0x0f], WHITE);
}

void __attribute__((noreturn)) sd_boot(gfx_t *gfx) {
  MODULE_CALL_THIS(gfx, nostroke, gfx);
  MODULE_CALL_THIS(gfx, fill, gfx, RED);
  MODULE_CALL_THIS(gfx, rectangle, gfx, ((display_region_t){0, 0, 320, 240}));
  MODULE_CALL_THIS(gfx, fill, gfx, BLACK);
  MODULE_CALL_THIS(gfx, rectangle, gfx, ((display_region_t){0, 0, 320, 240}));
  MODULE_CALL_THIS(gfx, fill, gfx, WHITE);
  MODULE_CALL_THIS(gfx, text, gfx, ((display_region_t){0, 20, 319, 239}), "Booting from SD card");


  sd_fns_t sd_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME(sd, SD_MODULE_ID, SD_FUNCTION_EXPORTS, &sd_fns);

  MODULE_CALL_FNS(sd, preinit, &sd_fns);
  GUI_t gui = {
    .gfx = gfx,
  };

  gui_init(&gui);

  while(!sd_fns.sd_detected()) {
    gui_msgbox(&gui,"Insert card", MSGBOX_OK);
    PINB = 0x80;
    // wait for card to be inserted
    _delay_ms(100);
  }
  MODULE_CALL_THIS(gfx, fill, gfx, WHITE);
  MODULE_CALL_THIS(gfx, textSize, gfx, 24);
  MODULE_CALL_THIS(gfx, text, gfx, ((display_region_t){0, 40, 319, 239}), "SD card detected");

  {
    uint8_t errno;
    if ((errno = MODULE_CALL_FNS(sd, initialise, &sd_fns)) != 0) {
      static const char msg[] = "Error initializing card";
      // static const char msg[] PROGMEM = "Error initializing card";
      char buf[32];
      snprintf(buf, sizeof(buf), "%s %d", msg, errno);
      gui_msgbox(&gui, buf, MSGBOX_OK);
      goto end;
    }
    if (partitions_init() != 0) {
      static const char msg[] PROGMEM = "Error initializing partitions";
      gui_msgboxP(&gui, pgm_get_far_address(msg), MSGBOX_OK);
      goto end;
    }
  }
  {
    fat_fns_t fat_fns;
    MODULE_IMPORT_FUNCTIONS_RUNTIME(fat, FAT_MODULE_ID, FAT_FUNCTION_EXPORTS, &fat_fns);

    blockdev_sector_fns_t sd_bd_fns;
    sd_bd_fns.read_sector  = (bd_read_sector_fn_t )sd_fns.sd_rdblock;
    sd_bd_fns.write_sector = (bd_write_sector_fn_t)sd_fns.sd_wrblock;
    
    // BlockDev root_bd = {
    //   .fns = &sd_bd_fns,
    //   .fn_ctx = NULL,
    //   .sector_start = 0,
    //   .sector_count = -1,
    // };

    BlockDev partition_bd = {
      .fns = &sd_bd_fns,
      .fn_ctx = NULL,
      .sector_start = 0,
      .sector_count = 0,
    };


    FAT_FileSystem_t fs;
    fs.fns = &fat_fns;
    MODULE_CALL_THIS(fat, init, &fs, &partition_bd);
    fstatus_t ret;
    // char hex[] = "0123456789ABCDEF";
    ret = MODULE_CALL_THIS(fat, fat_mount, &fs, false, false);
    if (ret != 0) {
      const char msg[] = "Error mounting filesystem";
      gui_msgbox(&gui,msg, MSGBOX_OK);
      goto end;
    }

    gui_choose_file(&gui, &fs.fs, "/");
    gui_msgbox(&gui,"Booting from file", MSGBOX_OK);

  }

  


end:
  error();
while(1);
}

extern void blink_bits(uint32_t value, uint8_t nbits);

static void __attribute__((noreturn)) run_interactive() {
  DDRB |= 0x80;
  DDRE &= ~0x80;
  PORTE |= 0x80;
  DDRC &= ~0x3C;
  PORTC |= 0x3C;
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

  const char *menu[] = {
    ("Boot from SD file"),
    ("Reboot application")
  };

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
  encoder_init();
  DDRE &= ~0x80;
  PORTE |= 0x80;
  int8_t dt = 0;
  while (func == NULL) {
    wdt_reset();
    if (button_clicked(0)) {
      func = menu_func[selection];
      break;
    }
    dt = encoder_dt(0);
    if (dt < 2 && dt > -2) {
      continue;
    }
    int8_t sign = dt > 0 ? 1 : -1;
    encoder_dt(sign*2);
    selection += sign;
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

int main() {
  // check if extreset
  if ((MCUSR & _BV(EXTRF)) == 0) {
    app_reboot();
  }

  // disable watchdog
  MCUSR = ~_BV(WDRF); // clear watchdog reset flag
  WDTCSR = _BV(WDCE);
  WDTCSR = 0x00;

  // enable boot ivsel
  MCUCR |= _BV(IVCE);
  MCUCR = _BV(IVSEL); // set bootloader vector

  // // Set clock prescaler to 1
  CLKPR = 0x80; // Enable clock change
  CLKPR = 0x00; // Set clock prescaler to 1
  RAMPZ = 1;

  DDRB = 0x80;
  PORTB |= 0x80;
  // while(1) {
  //   PINB = 0x80;
  //   _delay_ms(1000);
  // }

  run_interactive();
  __builtin_unreachable();
}