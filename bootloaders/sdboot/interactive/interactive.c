#include "sdboot/interactive.h"
#include "lcd/lcd.h"
#include "gfx/gfx.h"
#include "gui/gui.h"
#include "font/font5x7.h"
#include "buttons/buttons.h"
#include "sd/sd.h"
#include "fat/fat.h"
#include "module/module.h"
#include "module/imports.h"
#include "module/pic.h"
#include "boot/boot.h"

#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include <string.h>
#include <alloca.h>

static const char menu_opt_0[] PROGMEM = "Boot from SD file";
static const char menu_opt_1[] PROGMEM = "Reboot application";

typedef struct{
  module_fns_t *module_fns;
  gfx_t *gfx;
}ctx_t;


void flash_program_filedescriptor(FileSystem_t *fs, file_descriptor_t fd, uint_farptr_t addr, module_fns_t *module_fns) {

  boot_fns_t boot_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME(boot, BOOT_MODULE_ID, BOOT_FUNCTION_EXPORTS, &boot_fns, module_fns);

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



static
__attribute__((noreturn))
void app_reboot(ctx_t *ctx) {
  
  module_fns_t *module_fns = ctx->module_fns;

  boot_fns_t boot_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME(boot, BOOT_MODULE_ID, BOOT_FUNCTION_EXPORTS, &boot_fns, module_fns);
  
  MODULE_CALL_FNS(boot, reboot, &boot_fns, REBOOT_MODE_APP);
  __builtin_unreachable();

}

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

static
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


void __attribute__((noreturn)) sd_boot(ctx_t *ctx) {
  char buf[64]; 
  display_region_t r;
  gfx_t *gfx = ctx->gfx;
  module_fns_t *module_fns = ctx->module_fns;
  MODULE_CALL_THIS(gfx, nostroke, gfx);
  MODULE_CALL_THIS(gfx, fill, gfx, BLACK);
  {
    static const display_region_t _r PROGMEM = {0, 0, 319, 239};
    memcpy_PF(&r, pgm_get_far_address(_r) + GET_MODULE_DATA_PTR_OFFSET(), sizeof(display_region_t));
  }
  MODULE_CALL_THIS(gfx, rectangle, gfx, r);
  MODULE_CALL_THIS(gfx, fill, gfx, WHITE);
  {
    static const char msg[] PROGMEM = "Booting from SD card";
    strcpy_PF(buf, pgm_get_far_address(msg) + GET_MODULE_DATA_PTR_OFFSET());
  }
  {
    static const display_region_t _r PROGMEM = {0, 20, 319, 239};
    memcpy_PF(&r, pgm_get_far_address(_r) + GET_MODULE_DATA_PTR_OFFSET(), sizeof(display_region_t));
  }
  MODULE_CALL_THIS(gfx, text, gfx, r, buf);

  uint_farptr_t msgp;

  sd_fns_t sd_fns;
  buttons_fns_t buttons_fns;
  gui_fns_t gui_fns;
  MODULE_IMPORT_FUNCTIONS_RUNTIME(sd, SD_MODULE_ID, SD_FUNCTION_EXPORTS, &sd_fns, module_fns);
  MODULE_IMPORT_FUNCTIONS_RUNTIME(buttons, BUTTONS_MODULE_ID, BUTTONS_FUNCTION_EXPORTS, &buttons_fns, module_fns);
  MODULE_IMPORT_FUNCTIONS_RUNTIME(gui, GUI_MODULE_ID, GUI_FUNCTION_EXPORTS, &gui_fns, module_fns);


  MODULE_CALL_FNS(sd, preinit, &sd_fns);


  GUI_t gui = {
    .fns = &gui_fns,
    .gfx = gfx,
    .buttons_fns = &buttons_fns,
  };

  MODULE_CALL_THIS(gui, init, &gui);
  
  {
    static const char msg[] PROGMEM = "Insert card";
    strcpy_PF(buf, pgm_get_far_address(msg) + GET_MODULE_DATA_PTR_OFFSET());
  }

  while(!sd_fns.sd_detected()) {
    MODULE_CALL_THIS(gui, msgbox, &gui, buf, MSGBOX_OK);
    PINB = 0x80;
    // wait for card to be inserted
    _delay_ms(100);
  }
  MODULE_CALL_THIS(gfx, fill, gfx, WHITE);
  MODULE_CALL_THIS(gfx, textSize, gfx, 24);
  
  strcpy_PF(buf, MOD_PFSTR("SD card detected"));
  memcpy_PF(&r, MOD_PFTYPE(display_region_t, {0, 40, 319, 239}), sizeof(display_region_t));
  MODULE_CALL_THIS(gfx, text, gfx, r, buf);

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
      msgp = MOD_PFSTR("Error initializing card");
      goto end;
    }
    if (partitions_init(&root_bd, &partition_bd) != 0) {
      msgp = MOD_PFSTR("Error initializing partitions");
      goto end;
    }
  }
  {
    fat_fns_t fat_fns;
    MODULE_IMPORT_FUNCTIONS_RUNTIME(fat, FAT_MODULE_ID, FAT_FUNCTION_EXPORTS, &fat_fns, module_fns);

    FAT_FileSystem_t fs;
    fs.fns = &fat_fns;
    MODULE_CALL_THIS(fat, init, &fs, &partition_bd);
    fstatus_t ret;
    ret = MODULE_CALL_THIS(fs, mount, &fs.fs, false, false);
    if (ret != 0) {
      msgp = MOD_PFSTR("Error mounting filesystem");
      goto end;
    }
    strcpy_PF(buf, MOD_PFSTR("/lafortuna/apps/"));
    // strcpy_PF(buf, MOD_PFSTR("/lafortuna/apps/"));

    char hex(uint8_t c) {
      if (c < 10) return '0' + c;
      return 'A' + c - 10;
    }

    file_descriptor_t fd = MODULE_CALL_THIS(gui, choose_file, &gui, &fs.fs, buf);
    if (fd < 0) {
      strcpy_PF(buf, MOD_PFSTR("No file selected "));
      uint8_t i = 0;
      while(buf[i])i++;
      buf[i++] = hex((((uint8_t)fd)>>4)&0x0f);
      buf[i++] = hex((((uint8_t)fd)>>0)&0x0F);
      buf[i++] = 0;
      MODULE_CALL_THIS(gui, msgbox, &gui, buf, MSGBOX_OK);
      error(1);
    }
    strcpy_PF(buf, MOD_PFSTR("Boot from file"));
    MODULE_CALL_THIS(gui, msgbox, &gui, buf, MSGBOX_OK);
    PINB = 0x80;

    flash_program_filedescriptor(&fs.fs, fd, 0x0000UL, module_fns);
    blink(4);
    
    boot_fns_t boot_fns;
    MODULE_IMPORT_FUNCTIONS_RUNTIME(boot, BOOT_MODULE_ID, BOOT_FUNCTION_EXPORTS, &boot_fns, module_fns);

    MODULE_CALL_FNS(boot, reboot, &boot_fns, REBOOT_MODE_APP);
    __builtin_unreachable();
  }

end:
  strcpy_PF(buf, msgp);
  MODULE_CALL_THIS(gui, msgbox, &gui, buf, MSGBOX_OK);
  error(3);
}


void __attribute__((noreturn)) sdboot_interactive_run(module_fns_t* module_fns) {
  lcd_fns_t lcd_fns;
  gfx_fns_t gfx_fns;
  font5x7_fns_t font_fns;
  buttons_fns_t buttons_fns;

  MODULE_IMPORT_FUNCTIONS_RUNTIME(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS, &lcd_fns, module_fns);
  MODULE_IMPORT_FUNCTIONS_RUNTIME(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS, &gfx_fns, module_fns);
  MODULE_IMPORT_FUNCTIONS_RUNTIME(font5x7, FONT5X7_MODULE_ID, FONT5X7_FUNCTION_EXPORTS, &font_fns, module_fns);
  MODULE_IMPORT_FUNCTIONS_RUNTIME(buttons, BUTTONS_MODULE_ID, BUTTONS_FUNCTION_EXPORTS, &buttons_fns, module_fns);




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

  ctx_t ctx = {
    .module_fns = module_fns,
    .gfx = &gfx,
  };
  MODULE_CALL_FNS(buttons, init, &buttons_fns);


  typedef __attribute__((noreturn)) void (*menu_func_t)(ctx_t *gfx);
  typedef struct{
    uint_farptr_t s;
    menu_func_t func;
  }menu_entry_t;

  uint_farptr_t module_offset = GET_MODULE_DATA_PTR_OFFSET();

  menu_entry_t menu[] = {
    {
      pgm_get_far_address(menu_opt_0) + module_offset,
      indirect_call(sd_boot),
    },
    {
      pgm_get_far_address(menu_opt_1) + module_offset,
      indirect_call(app_reboot),
    },
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
    char buf[64];
    strcpy_PF(buf, menu[i].s);
    MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){20, 40+i*20, 319, 60*i*20}), buf);
  }
  char sel_str[3] = {'>', '\0'};

  PINB = 0x80;
  int8_t selection = 0;
  __attribute__((noreturn)) menu_func_t func = NULL;
  while (func == NULL) {
    wdt_reset();
    if (MODULE_CALL_FNS(buttons, clicked, &buttons_fns, BTN_ID_CENTER)) {
      if (selection == 0) indirect_call(sd_boot)(&ctx);
      else if (selection == 1) indirect_call(app_reboot)(&ctx);
      func = menu[selection].func;
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
    static const display_region_t rP PROGMEM = {0, 20, 19, 40+2*20};
    display_region_t r;
    memcpy_PF(&r, pgm_get_far_address(rP) + GET_MODULE_DATA_PTR_OFFSET(), sizeof(display_region_t));
    MODULE_CALL_THIS(gfx, fill, &gfx, BLACK);
    MODULE_CALL_THIS(gfx, nostroke, &gfx);
    MODULE_CALL_THIS(gfx, rectangle, &gfx, r);
    MODULE_CALL_THIS(gfx, fill, &gfx, WHITE);
    MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, 40+selection*20, 19, 60+selection*20}), sel_str);
    PINB = 0x80;
  }
  while(1);

  func(&ctx);
}

REGISTER_MODULE(sdboot_interactive, SDBOOT_INTERACTIVE_MODULE_ID, SDBOOT_INTERACTIVE_FUNCTION_EXPORTS, 1);