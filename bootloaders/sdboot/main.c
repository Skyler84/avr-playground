#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
#include "fonts/fonts.h"
#include "lcd/lcd.h"
#include "sd/sd.h"
#include "fat/fat.h"

MODULE_IMPORT_FUNCTIONS(fat, FAT_MODULE_ID, FAT_FUNCTION_EXPORTS)
MODULE_IMPORT_FUNCTIONS(fonts, FONTS_MODULE_ID, FONTS_FUNCTION_EXPORTS)
MODULE_IMPORT_FUNCTIONS(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS)
MODULE_IMPORT_FUNCTIONS(sd, SD_MODULE_ID, SD_FUNCTION_EXPORTS)

// asm("
//   .org 0x1fffe\n"
//   "  rjmp module_fn_lookup\n"
// );



// static blockdev_sector_fns_t sd_fns = {
//   .read_sector = sd_rdblock,
//   .write_sector = sd_wrblock,
// };

static blockdev_sector_fns_t sd_bd_fns;

static BlockDev root_bd = {
  .fns = &sd_bd_fns,
  .fn_ctx = NULL,
  .sector_start = 0,
  .sector_count = -1,
};

static BlockDev partition_bd = {
  .fns = &sd_bd_fns,
  .fn_ctx = NULL,
  .sector_start = 0,
  .sector_count = 0,
};

FileSystem_fns_t fat_fs_fns;
FAT_FileSystem_t fs;

GUI_t gui;


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
  gui_msgboxP(&gui,PSTR("Please eject card"), MSGBOX_OK);
  while(sd_detected()) wdt_reset();
  // reboot bootloader?
  bl_reboot();
}

void __attribute__((noreturn)) boot_from_file(const char *filename, uint32_t load_addr) {
  (void)filename;
  (void)load_addr;
  // MOD_CALL(sd, &sd_fns, preinit);
  // MOD_CALL(sd, &sd_fns, initialise);
  sd_fns.preinit();
  sd_fns.initialise();

  // open file
  // program file to flash
  // ((void(*)())0x0000)();
  app_reboot();
}

int8_t partitions_init() {
  sd_bd_fns.read_sector  = (bd_read_sector_fn_t )sd_fns.rdblock;
  sd_bd_fns.write_sector = (bd_write_sector_fn_t)sd_fns.wrblock;
  // initialize partitions
  root_bd.fns = &sd_bd_fns;
  root_bd.fn_ctx = NULL;
  root_bd.sector_start = 0;
  root_bd.sector_count = -1;

  partition_bd.fns = &sd_bd_fns;
  partition_bd.fn_ctx = NULL;
  partition_bd.sector_start = 0;
  partition_bd.sector_count = 0;
  
  uint8_t buf[512];
  blockdev_read_sector(&root_bd, 0, buf);
  uint16_t magic = *((uint16_t*)&buf[510]);
  // gui_msgboxP(&gui,PSTR("Reading MBR"), MSGBOX_OK);
  if (magic != 0xAA55) {
    // gui_msgboxP(&gui,PSTR("Error reading MBR"), MSGBOX_OK);
    goto err;
  }
  uint32_t start_sector = *((uint32_t*)(buf + 0x1BE + 8));
  uint32_t sector_count = *((uint32_t*)(buf + 0x1BE + 12));
  if (blockdev_partition(&root_bd, &partition_bd, start_sector, sector_count) != 0) {
    // error partitioning blockdev
    // gui_msgboxP(&gui,PSTR("Error partitioning"), MSGBOX_OK);
    goto err;
  }
  blockdev_read_sector(&partition_bd, 0, buf);
  magic = *((uint16_t*)&buf[510]);
  if (magic != 0xAA55) {
    // gui_msgboxP(&gui,PSTR("Error reading MBR"), MSGBOX_OK);
    goto err;
  }
  return 0;
err:
  return -1;
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
void lcd_debug_u16(lcd_xcoord_t x, lcd_ycoord_t y, uint16_t val) {
  lcd_fns.display_char(x, y, 1, fonts_fns.get_default(), &fonts_fns, hex[(val>>12)&0x0f], WHITE);
  lcd_fns.display_char(x+6, y, 1, fonts_fns.get_default(), &fonts_fns, hex[(val>>8)&0x0f], WHITE);
  lcd_fns.display_char(x+12, y, 1, fonts_fns.get_default(), &fonts_fns, hex[(val>>4)&0x0f], WHITE);
  lcd_fns.display_char(x+18, y, 1, fonts_fns.get_default(), &fonts_fns, hex[(val>>0)&0x0f], WHITE);
}

void __attribute__((noreturn)) sd_boot() {
  lcd_fns.fill_rectangle(0, 320, 0, 240, BLACK);
  sd_fns.preinit();  

  if (!sd_detected()) {
    gui_msgboxP(&gui,PSTR("Insert card"), MSGBOX_OK);
    while(!sd_detected()) {
      // wait for card to be inserted
      _delay_ms(100);
    }
  }
  {
    if (sd_initialise() != 0) {
      gui_msgboxP(&gui,PSTR("Error initializing card"), MSGBOX_OK);
      goto end;
    }
    if (partitions_init() != 0) {
      gui_msgboxP(&gui,PSTR("Error initializing partitions"), MSGBOX_OK);
      goto end;
    }
  }
  {
    // uint8_t buf[512];
    // fat_fns_init();
    fat_fs_fns = (FileSystem_fns_t){
      .mount = (fs_mount_fn_t)fat_fns.mount,
      .umount = (fs_umount_fn_t)fat_fns.umount,
      .stat = (fs_stat_fn_t)fat_fns.stat,
      .open = (fs_open_fn_t)fat_fns.open,
      .close = (fs_close_fn_t)fat_fns.close,
      .read = (fs_read_fn_t)fat_fns.read,
      .write = (fs_write_fn_t)fat_fns.write,
      .seek = (fs_seek_fn_t)fat_fns.seek,
      .unlink = (fs_unlink_fn_t)fat_fns.unlink,
      .rename = (fs_rename_fn_t)fat_fns.rename,
      // .opendir = (fs_opendir_fn_t)fat_fns.opendir,
      // .closedir = (fs_closedir_fn_t)fat_fns.closedir,
      // .readdir = (fs_readdir_fn_t)fat_fns.readdir,
      .mkdir = (fs_mkdir_fn_t)fat_fns.mkdir,
      .rmdir = (fs_rmdir_fn_t)fat_fns.rmdir,
    };
    // fs.fs.fns = &fat_fs_fns;
    fat_fns.init(&fs, &partition_bd);
    fstatus_t ret;
    // char hex[] = "0123456789ABCDEF";
    ret = fat_fns.mount(&fs, false, false);
    if (ret != 0) {
      gui_msgboxP(&gui,PSTR("Error mounting filesystem"), MSGBOX_OK);
      goto end;
    }
    FileInfo_t info;
    int8_t selection = 0;
    uint8_t num_lines = 6;
    uint8_t line_start = 0;
    char dir[64] = "/FOLDER/";
    file_descriptor_t dirfd = fat_fns.opendir(&fs, dir);
    const uint8_t line_spacing = 30;
    uint8_t selected = 0;
    while(1) {
      lcd_fns.fill_rectangle(0, 320, 0, 20, BLACK);
      // if (line_start > )
      lcd_debug_u16(0, 0, line_start);
      lcd_debug_u16(0, 10, selection);
      lcd_ycoord_t y = 40;
      int8_t line_no = 0;
      uint16_t colors[] = {0x045c, 0x12d1};
      if (selected) {
        // get FileInfo of selected item
        fat_fns.readdir(&fs, dirfd, NULL);
        while (selected-- && (ret = fat_fns.readdir(&fs, dirfd, &info)) == 0) {
          // do nothing
        }
        if (ret != 0) {
          // error reading directory
          gui_msgboxP(&gui,PSTR("Error reading directory"), MSGBOX_OK);
          goto end;
        }
        if (info.type == 1) {
          
          selected = 0;
        } else if(info.type == 2) {
          // directory
          file_descriptor_t newdirfd = fat_fns.opendirat(&fs, dirfd, info.name);
          if (newdirfd < 0) {
            gui_msgboxP(&gui,PSTR("Error opening directory"), MSGBOX_OK);
            goto end;
          }
          fat_fns.closedir(&fs, dirfd);
          dirfd = newdirfd;
          if (dirfd < 0) {
            goto end;
          }
          selected = 0;
          selection = 0;
          line_start = 0;
        } else {
          // unknown type
          goto end;
  
        }
      }
      fat_fns.readdir(&fs, dirfd, NULL);
      for (uint8_t ln = 0; ln < num_lines; ln++) {
        lcd_ycoord_t y = 40 + ln*line_spacing;
        lcd_ycoord_t ys = y - (line_spacing-16)/2;
        lcd_ycoord_t ye = y + (line_spacing+1+16)/2;
        lcd_fns.fill_rectangle(10, 309, ys, ye, colors[ln%2]);
      }
      while ((ret = fat_fns.readdir(&fs, dirfd, &info)) == 0) {
        if (line_no < line_start) {
          line_no++;
          continue;
        }
        if (line_no >= line_start + num_lines) {
          line_no++;
          continue;
        }
        lcd_fns.display_string(40, 0, y, 2, fonts_fns.get_default(), &fonts_fns, info.name, 0xFFFF);
        if (line_no == selection) {
          lcd_fns.display_char(20, y, 2, fonts_fns.get_default(), &fonts_fns, '>', WHITE);
        }
        y += 30;
        line_no++;
      }
      while(1) {
        _delay_ms(10);
        int8_t dt = encoder_dt(0);
        if (button_clicked(BTN_N)) {
          selection--;
          break;
        }
        if (button_clicked(BTN_S)) {
          selection++;
          break;
        }
        if (button_clicked(BTN_C)) {
          // button pressed
          selected = selection+1;
          break;
        }
        if (dt < 2 && dt > -2) {
          continue;
        }
        int8_t sign = dt > 0 ? 1 : -1;
        encoder_dt(dt);
        selection += sign;
        break;
      }
      
      if (selection < 0) {
        selection = 0;
      }
      if (selection >= line_no) {
        selection = line_no-1;
      }
      if (selection < line_start) {
        line_start--;
      }
      if (selection > line_start + num_lines - 1) {
        line_start = selection - num_lines + 1;
      }
      if (selection < line_start) {
        line_start = selection;
      }
      if (line_start > line_no ) {
        line_start = line_no - 1;
      }
    }
  }



end:
  error();
}

void __attribute__((noreturn)) run_interactive() {
  DDRB |= 0x80;
  DDRE &= ~0x80;
  PORTE |= 0x80;
  DDRC &= ~0x3C;
  PORTC |= 0x3C;
  lcd_fns.init();
  lcd_fns.set_orientation(West);
  lcd_fns.clear(0x0000);

  const char *menu[] = {
    PSTR("Boot from SD file"),
    PSTR("Reboot application")
  };

  typedef __attribute__((noreturn)) void (*menu_func_t)(void);
  menu_func_t menu_func[] = {
    sd_boot,
    app_reboot
  };

  for (int i = 0; i < 2; i++) {
    lcd_fns.display_stringP(20, 0, 40 + i*20, 2, fonts_fns.get_default(), &fonts_fns, menu[i], 0xFFFF);
  }
  int8_t selection = 0;
  __attribute__((noreturn)) menu_func_t func = NULL;
  lcd_fns.display_char(0, 40 + selection*20, 2, fonts_fns.get_default(), &fonts_fns, '>', WHITE);
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
    lcd_fns.fill_rectangle(0, 19, 40, 40 + 2*20, BLACK);
    lcd_fns.display_char(0, 40 + selection*20, 2, fonts_fns.get_default(), &fonts_fns, '>', WHITE);
  }

  func();
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

  run_interactive();
  __builtin_unreachable();
}