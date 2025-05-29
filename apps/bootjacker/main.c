#include "spm.h"
#include "instructions.h"
#include "gui.h"

#include "module/imports.h"
#include "lcd/lcd.h"
#include "gfx/gfx.h"
#include "font/font5x7.h"
#include "boot/boot.h"
#include "fat/fat.h"
#include "blockdev/blockdev.h"
#include "sd/sd.h"

#include <avr/io.h>
#include <avr/power.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdio.h>
#include <string.h>


MODULE_IMPORT_FUNCTIONS(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(font5x7, FONT5X7_MODULE_ID, FONT5X7_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(fat, FAT_MODULE_ID, FAT_FUNCTION_EXPORTS);
MODULE_IMPORT_FUNCTIONS(sd, SD_MODULE_ID, SD_FUNCTION_EXPORTS);
// MODULE_IMPORT_FUNCTIONS(boot, BOOT_MODULE_ID, BOOT_FUNCTION_EXPORTS);

lcd_t lcd = {
  .fns = &lcd_fns,
};

gfx_t gfx = {
  .display = &lcd.display,
  .fns = &gfx_fns,
};

font5x7_t font = {
  .fns = &font5x7_fns,
};

FAT_FileSystem_t fs;

struct spm_sequence seq;


uint8_t line = 1;
const uint16_t line_height = 20;
char buf[64];

int println(const char *format, ...) __attribute__((format(printf, 1, 2), noinline));
int println(const char *format, ...) {
  va_list args;
  va_start(args, format);
  int len = vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);
  MODULE_CALL_THIS(gfx, fill, &gfx, WHITE);
  MODULE_CALL_THIS(gfx, textSize, &gfx, 24);
  MODULE_CALL_THIS(gfx, text, &gfx, ((display_region_t){0, line_height * line++, 319 - 2 * 2, 239 - 2 * 2}), buf);
  
  return len;
}

void clear_display() {
  MODULE_CALL_THIS(gfx, fill, &gfx, BLACK);
  MODULE_CALL_THIS(gfx, nostroke, &gfx);
  MODULE_CALL_THIS(gfx, rectangle, &gfx, (display_region_t){0, 0, 320, 240});
  line = 1;
}


typedef enum fuse_byte{
  fuse_low_byte = 0x0000,
  fuse_lock_byte = 0x0001,
  fuse_extended_byte = 0x0002,
  fuse_high_byte = 0x0003,
} fuse_byte_t;

struct spm_sequence find_spm_sequence() {
  uint16_t instruction_buf[4] = {0};
  uint32_t address = 0x1E000UL;
  struct spm_sequence seq = {0};
  while(address < 0x20000) {

    memmove(&instruction_buf[0], &instruction_buf[1], 
            sizeof(instruction_buf) - sizeof(instruction_buf[0]));
    instruction_buf[3] = pgm_read_word_far(address);

    if (!is_spm_instruction(instruction_buf[3])) {
      address += 2;
      continue;
    }

    if (is_out_instruction(instruction_buf[2]) &&
        out_instruction_addr(instruction_buf[2]) == _SFR_IO_ADDR(SPMCSR)) {
      // Found a sequence that starts with an OUT instruction to SPMCSR
      seq.seq_type = SPM_SEQUENCE_TYPE_OUT_SPMCSR;
      seq.seq = (void(*)(void))((uintptr_t)(address/2-1));
      return seq;
    }
    if (is_sts_instruction(instruction_buf[1]) &&
        instruction_buf[2] == _SFR_MEM_ADDR(SPMCSR)) {
      // Found a sequence that starts with an STS instruction to SPMCSR
      seq.seq_type = SPM_SEQUENCE_TYPE_STS_SPMCSR;
      seq.seq = (void(*)(void))((uintptr_t)(address/2-2));
      return seq;
    }
    address += 2;    
  }
  return seq;
}

uint8_t get_fuse_bits(fuse_byte_t fuse_byte) {
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

boot_size_t get_boot_size() {
  uint8_t hfuse = get_fuse_bits(fuse_high_byte);
  switch(hfuse & 0x06) {
    case 0x00: return boot_size_4096_words; // 8K bootloader
    case 0x02: return boot_size_2048_words; // 4K bootloader
    case 0x04: return boot_size_1024_words; // 2K bootloader
    case 0x06: return boot_size_512_words;  // 1K bootloader
  }
  __builtin_unreachable();
}

uint16_t boot_size_to_bytes(boot_size_t size) {
  switch(size) {
    case boot_size_512_words: return 1024; // 1K bootloader
    case boot_size_1024_words: return 2048; // 2K bootloader
    case boot_size_2048_words: return 4096; // 4K bootloader
    case boot_size_4096_words: return 8192; // 8K bootloader
    default: return 0;
  }
  __builtin_unreachable();
}

#define DECLARE_FUNCSIZE(funcname)         \
  extern unsigned int funcname##_funcsize; \
  asm(#funcname "_funcsize: .long . - " #funcname "\n\t")

#define FUNCSIZE(funcname) \
  funcname##_funcsize

// this stub can be copied anywhere in the bootloader section
// 
void __attribute__((naked)) bootloader_stub(uint8_t cmd) {
  asm volatile(
    "out %[spmcsr], %[in]\n\t"
    "spm\n\t"
    "nop\n\t"
    "ret\n\t"
    : 
    : [spmcsr] "I" (_SFR_IO_ADDR(SPMCSR))
    , [in] "r" (cmd)
  );
}

DECLARE_FUNCSIZE(bootloader_stub);

void install_bootloader_stub(uint32_t page_addr) {
  println("Installing SPM stub...");
  {
    uint8_t page_buf[256];
    for (uint16_t i = 0; i < 256; i++) {
      page_buf[i] = pgm_read_byte_far(((uint_farptr_t)(uintptr_t)&bootloader_stub)*2+i); // fill the page with 0xff
    }
    //erase a single block at the end of the bootloader section
    page_erase(seq, page_addr);
    page_program(seq, page_addr, page_buf);
    bool verified = true;
    verified = page_verify(page_addr, page_buf);
    for (uint16_t i = 0; i < 256; i++) {
      if (pgm_read_byte_far(page_addr + i) != page_buf[i]) {
        println("byte %d not 0x%02x", i, page_buf[i]);
        verified = false;
      }
    }
    if (!verified) {
      println("Error installing SPM stub");
      while(1);
    } else {
      println("SPM stub installed");
      seq.seq = (void(*)(void))((uintptr_t)(page_addr/2));
      seq.seq_type = SPM_SEQUENCE_TYPE_OUT_SPMCSR; // set sequence type to OUT SPMCSR
    }
  }
}


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

volatile int x = 0;
extern void setup();
void setup() {

  clock_prescale_set(clock_div_1);

  MODULE_CALL_THIS(display, init, &lcd.display);
  MODULE_CALL_THIS(gfx, init, &gfx, &lcd.display);
  MODULE_CALL_THIS(lcd, set_orientation, &lcd, West);
  
  MODULE_CALL_THIS(gfx, fill, &gfx, BLACK);
  MODULE_CALL_THIS(gfx, nostroke, &gfx);
  MODULE_CALL_THIS(gfx, rectangle, &gfx, (display_region_t){0, 0, 320, 240});
  MODULE_CALL_THIS(gfx, fill, &gfx, WHITE);
  MODULE_CALL_THIS(gfx, textFont, &gfx, &font.base);
  MODULE_CALL_THIS(gfx, textSize, &gfx, 24);
}

extern void check_lock_bits();
void check_lock_bits() {
  uint8_t lock_bits = get_fuse_bits(fuse_lock_byte);
  println("Lock bits: 0x%02X", lock_bits);

  if ((lock_bits & 0x3f) != 0x3f) {
    println("Unable to continue");
    while(1);
  } else {
    println("Lock bits ok");
  }
}

extern void check_fuses();
void check_fuses(){
  
}

int main()
{
  setup();

  MCUCR = _BV(IVCE); // enable interrupt vector change
  MCUCR = 0; // set interrupt vector to application section

  check_lock_bits();

  {
    uint8_t fuse_bits[3];
    fuse_bits[0] = get_fuse_bits(fuse_low_byte);
    fuse_bits[1] = get_fuse_bits(fuse_high_byte);
    fuse_bits[2] = get_fuse_bits(fuse_extended_byte);
    println("Fuses: l:%02X h:%02X e:%02X", fuse_bits[0], fuse_bits[1], fuse_bits[2]);
  }

  {
    seq = find_spm_sequence();
    if (seq.seq == NULL) {
      println("SPM sequence not found");
      while(1);
    } else {
      println("SPM found at %p", seq.seq);
    }
  }
  uint8_t page_buf[256];
  for (int i = 0; i < 256; i++){
    page_buf[i] = 0xff;
  }
  uint32_t page_addr = 0x1f800UL;
  
  blockdev_sector_fns_t sd_bd_fns = {
    .read_sector  = (bd_read_sector_fn_t )sd_fns.sd_rdblock,
    .write_sector = (bd_write_sector_fn_t)sd_fns.sd_wrblock,
  };
  
  GUI_t gui = {
    .gfx = &gfx,
  };
  
  MODULE_CALL_FNS(sd, preinit, &sd_fns);
  gui_init(&gui);
  
  while(!MODULE_CALL_FNS(sd, detected, &sd_fns)) {
    gui_msgbox(&gui,"Insert card", MSGBOX_OK);
    PINB = 0x80;
    // wait for card to be inserted
    _delay_ms(100);
  }
  // while(!x);
  
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

  // snp
  {
    uint8_t errno;
    if ((errno = MODULE_CALL_FNS(sd, initialise, &sd_fns)) != 0) {
      static const char msg[] PROGMEM = "Error initializing card";
      gui_msgboxP(&gui, pgm_get_far_address(msg), MSGBOX_OK);
      while(1);
    }
    if (partitions_init(&root_bd, &partition_bd) != 0) {
      static const char msg[] PROGMEM = "Error initializing partitions";
      gui_msgboxP(&gui, pgm_get_far_address(msg), MSGBOX_OK);
      while(1);
    }
  }
  
  println("SD card mounted");
  
  {
    fs.fns = &fat_fns;
    volatile uint32_t x = (uintptr_t)fat_fns.fat_init;
    MODULE_CALL_THIS(fat, init, &fs, &partition_bd);
    fstatus_t ret;

    ret = MODULE_CALL_THIS(fs, mount, &fs.fs, false, false);
    if (ret != 0) {
      const char msg[] = "Error mounting filesystem";
      gui_msgbox(&gui,msg, MSGBOX_OK);
      while(1);
    }
    
    // file_descriptor_t fd = gui_choose_file(&gui, &fs.fs, "/lafortuna/bootloaders/");
    file_descriptor_t fd = gui_choose_file(&gui, &fs.fs, "/lafortuna/bootloaders/sdboot/");
    gui_msgbox(&gui, "File selected", MSGBOX_OK);
    (void)fd;
    // clear display
    clear_display();

    println("Checking file");
    uint32_t size = 0;
    union{
      uint16_t word;
      uint8_t bytes[2];
    } size_u;
    uint16_t instructions[3] = {0};
    bool found_suitable = false;
    while(MODULE_CALL_THIS(fs, read, &fs.fs, fd, (char*)&size_u, sizeof(size_u)) == sizeof(size_u)) {
      size += sizeof(size_u);
      memmove(instructions, &instructions[1], sizeof(instructions) - sizeof(instructions[0]));
      instructions[2] = size_u.word;
      if (!is_spm_instruction(instructions[2])) {
        continue;
      }
      if (is_out_instruction(instructions[1]) &&
          out_instruction_addr(instructions[1]) == _SFR_IO_ADDR(SPMCSR)) {
        // Found a sequence that starts with an OUT instruction to SPMCSR
        found_suitable = true;
      } else if (is_sts_instruction(instructions[0]) &&
                 instructions[1] == _SFR_MEM_ADDR(SPMCSR)) {
        // Found a sequence that starts with an STS instruction to SPMCSR
        found_suitable = true;
      }
    }
    println("File size: %lu bytes", size);
    if (size > boot_size_to_bytes(get_boot_size())) {
      println("bootloader too large!");
      while(1);
    }
    if (!found_suitable) {
      println("No suitable SPM found");
      while(1);
    } else {
      println("Suitable SPM found");
    }

    gui_msgbox(&gui, "Reflash bootloader?", MSGBOX_OK);
    clear_display();
    install_bootloader_stub(0x1ff00UL);

    println("Reflashing bootloader...");
    MODULE_CALL_THIS(fs, seek, &fs.fs, fd, 0, SEEK_SET);
    switch(get_boot_size()) {
      case boot_size_512_words:
        println("Bootloader size: 1KB");
        break;
      case boot_size_1024_words:
        println("Bootloader size: 2KB");
        break;
      case boot_size_2048_words:
        println("Bootloader size: 4KB");
        break;
      case boot_size_4096_words:
        println("Bootloader size: 8KB");
        break;
    }
    page_addr = 0x20000UL - boot_size_to_bytes(get_boot_size()); // start of the bootloader section
    do{
      ret = MODULE_CALL_THIS(fs, read, &fs.fs, fd, (char*)page_buf, sizeof(page_buf));
      if (ret < 0) {
        println("Error reading file: %d", ret);
        break;
      }
      if (ret == 0) {
        break; // end of file
      }
      if (ret < sizeof(page_buf)) {
        memset(&page_buf[ret], 0xff, sizeof(page_buf) - ret); // fill the rest with 0xff
      }
      seq = find_spm_sequence(); // find the SPM sequence again
      if (page_addr == 0x1ff00UL && (((uint_farptr_t)(uintptr_t)seq.seq)*2 & 0x1ff00UL) == 0x1ff00UL) {
        println("oh no, aborting");
        while(1);
      }
      page_erase(seq, page_addr); // erase page
      seq = find_spm_sequence(); // find the SPM sequence again
      page_program(seq, page_addr, page_buf); // program page
      if (!page_verify(page_addr, page_buf)) {
        println("Error verifying page at %#05lx", page_addr);
        while(1);
      }
      page_addr += sizeof(page_buf);
    } while(ret == sizeof(page_buf));
    MODULE_CALL_THIS(fs, close, &fs.fs, fd);
  }

  clear_display();
  println("Bootloader reflashed");


  while(1);

  __builtin_unreachable();
}