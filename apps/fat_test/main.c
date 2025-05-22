#include <avr/io.h>
#include <util/delay.h>

#include <string.h>

#include "module/imports.h"
#include "sd/sd.h"
#include "lcd/lcd.h"
#include "gfx/gfx.h"
#include "font/font5x7.h"
#include "fs/fs.h"
#include "fat/fat.h"

MODULE_IMPORT_FUNCTIONS(sd, SD_MODULE_ID, SD_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(font5x7, FONT5X7_MODULE_ID, FONT5X7_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(fat, FAT_MODULE_ID, FAT_FUNCTION_EXPORTS);

font5x7_t font = {
  .fns = &font5x7_fns,
};

lcd_t lcd = {
  .fns = &lcd_fns,
};

gfx_t gfx = {
  .display = &lcd.display,
  .fns = &gfx_fns,
};

FAT_FileSystem_t fs = {
  .fns = &fat_fns,
};


int8_t partitions_init(BlockDev *root_bd, BlockDev *partition_bd) {


  uint8_t buf[512];
  blockdev_read_sector(root_bd, 0, buf);
  uint16_t magic = *((uint16_t*)&buf[510]);
  // gui_msgboxP(&gui,PSTR("Reading MBR"), MSGBOX_OK);
  if (magic != 0xAA55) {
    // gui_msgboxP(&gui,PSTR("Error reading MBR"), MSGBOX_OK);
    goto err;
  }
  uint32_t start_sector = *((uint32_t*)(buf + 0x1BE + 8));
  uint32_t sector_count = *((uint32_t*)(buf + 0x1BE + 12));
  if (blockdev_partition(root_bd, partition_bd, start_sector, sector_count) != 0) {
    // error partitioning blockdev
    // gui_msgboxP(&gui,PSTR("Error partitioning"), MSGBOX_OK);
    goto err;
  }
  blockdev_read_sector(partition_bd, 0, buf);
  magic = *((uint16_t*)&buf[510]);
  if (magic != 0xAA55) {
    // gui_msgboxP(&gui,PSTR("Error reading MBR"), MSGBOX_OK);
    goto err;
  }
  return 0;
err:
  return -1;
}


int main() {
  CLKPR = 0x80;
  CLKPR = 0x00;
  // disable watchdog
  // check if extreset
  // if ((MCUSR & _BV(WDRF)) || ((MCUSR & _BV(EXTRF)) == 0)) {
  //   MCUSR = ~_BV(WDRF); // clear watchdog reset flag
  //   wdt_disable();
  // }
  // initialize lcd
  MODULE_CALL_THIS(gfx, init, &gfx, &lcd.display);
  MODULE_CALL_THIS(lcd, set_orientation, &lcd, West);
  MODULE_CALL_THIS(gfx, fill, &gfx, WHITE);
  MODULE_CALL_THIS(gfx, nostroke, &gfx);
  MODULE_CALL_THIS(font5x7, init, &font);
  MODULE_CALL_THIS(gfx, textFont, &gfx, &font.base);
  MODULE_CALL_THIS(gfx, textSize, &gfx, 24);
  MODULE_CALL_FNS(sd, preinit, &sd_fns);
  if (MODULE_CALL_FNS(sd, initialise, &sd_fns) != 0) {
    MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 20, 319, 239}), "SD card not detected");
    while(1);
  } else {
    MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 20, 319, 239}), "SD card initialised");
  }

  blockdev_sector_fns_t sd_bd_fns = {
    .read_sector  = (bd_read_sector_fn_t )sd_fns.sd_rdblock,
    .write_sector = (bd_write_sector_fn_t)sd_fns.sd_wrblock,
  };

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

  if (partitions_init(&root_bd, &partition_bd) != 0) {
    MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 40, 319, 239}), "Error initializing partitions");
    while(1);
  } else {
    MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 40, 319, 239}), "Partitions initialized");
  }

  MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 60, 319, 239}), "Mounting filesystem");
  MODULE_CALL_THIS(fat, init, &fs, &partition_bd);
  if (MODULE_CALL_THIS(fs, mount, &fs.fs, false, false) != 0) {
    MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 80, 319, 239}), "Error mounting filesystem");
    while(1);
  } else {
    MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 80, 319, 239}), "Filesystem mounted");
  }
  MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 100, 319, 239}), "Reading root directory");
  const char *path[] = {
    "LAFORT~1",
    "APPS",
    "FORTUN~1"
  };
  file_descriptor_t dirfd = MODULE_CALL_THIS(fs, open, &fs.fs, "/", O_RDONLY | O_DIRECTORY);
  for (uint8_t i = 0; i < 3; i++) {
    const char *dirname = path[i];
    file_descriptor_t fd = MODULE_CALL_THIS(fs, openat, &fs.fs, dirfd, dirname, O_RDONLY | O_DIRECTORY);

    if (fd < 0) {
      MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 120, 319, 239}), "Error opening directory");
      while(1);
    } else {
      MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 120, 319, 239}), "directory opened");
    }
    MODULE_CALL_THIS(fs, close, &fs.fs, dirfd);
    dirfd = fd;
  }
  MODULE_CALL_THIS(gfx, fill, &gfx, BLACK);
  MODULE_CALL_THIS(gfx, nostroke, &gfx);
  MODULE_CALL_THIS(gfx, rectangle, &gfx, (display_region_t){0, 0, 320, 240});
  MODULE_CALL_THIS(gfx, fill, &gfx, WHITE);
  uint8_t buf[16];
  memset(buf, 0xaa, sizeof(buf));
  MODULE_CALL_THIS(gfx, textSize, &gfx, 12);
  for (uint8_t rows = 0; rows < 10; rows++) {
    MODULE_CALL_THIS(fs, read, &fs.fs, dirfd, (char*)buf, 16);
    for (uint8_t col = 0; col < 16; col++) {
      char cbuf[3];
      snprintf(cbuf, sizeof(cbuf), "%02X", buf[col]);
      MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){col * 20, rows * 10 + 40, 319, 239}), cbuf);
    }
  }

  _delay_ms(2000);

  MODULE_CALL_THIS(gfx, fill, &gfx, BLACK);
  MODULE_CALL_THIS(gfx, nostroke, &gfx);
  MODULE_CALL_THIS(gfx, rectangle, &gfx, (display_region_t){0, 0, 320, 240});
  MODULE_CALL_THIS(gfx, fill, &gfx, WHITE);

  MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 20, 319, 239}), "Reading directory");
  FileInfo_t info;
  fstatus_t ret;
  int i = 0;
  MODULE_CALL_THIS(fs, seek, &fs.fs, dirfd, 0, SEEK_SET);
  while((ret = MODULE_CALL_THIS(fs, getdirents, &fs.fs, dirfd, &info, 1)) == 1) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%s", info.name);
    MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 40 + i * 20, 319, 239}), buf);
    i++;
    if (i > 10) {
      break;
    }
  }


  while(1);

}