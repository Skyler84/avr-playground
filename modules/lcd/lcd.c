#include "lcd/lcd.h"
#include "ili934x.h"
#include "module/pic.h"

#include <stdint.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/io.h>

// #define NOINLINE __attribute__((noinline))
#define NOINLINE

#ifdef __AVR_AT90USB1286__

#define CMD_REG (*(volatile uint8_t*)0x4000)
#define DATA_REG (*(volatile uint8_t*)0x4100)

static void write_cmd(uint8_t cmd) {
  CMD_REG=cmd;
}

static void write_data(uint8_t data) {
  DATA_REG=data;
}

// static uint8_t read_data() {
//   return DATA_REG;
// }

#else
static void NOINLINE write_cmd(uint8_t cmd) {
  RS_lo();
  WRITE(cmd);
  WR_lo();
  WR_hi();
}

static void NOINLINE write_data(uint8_t data) {
  RS_hi();
  WRITE(data);
  WR_lo();
  WR_hi();
  
}

// static uint8_t NOINLINE read_data() {
//   DATA_DDR = 0;
//   RD_lo();
//   uint8_t ret = DATA_PIN;
//   RD_hi();
//   DATA_DDR = 0xff;
//   return ret;
// }

#endif

inline static void write_data16(uint16_t data) {
  write_data((data) >> 8);
  write_data((data) & 0xFF);
}

// static void NOINLINE write_cmd_data(uint8_t cmd, uint8_t ndata, char *data) {
//   uint8_t i;
//   char *d = data;
//   write_cmd(cmd);
//   for(i=0; i<ndata; i++)
//     write_data(*d++);
// }

#define pgm_read_byte_elpm(addr)   \
(__extension__({                \
    uint16_t __addr16 = (uint16_t)(addr); \
    uint8_t __result;           \
    __asm__ __volatile__        \
    (                           \
        "elpm" "\n\t"           \
        "mov %0, r0" "\n\t"     \
        : "=r" (__result)       \
        : "z" (__addr16)        \
        : "r0"                  \
    );                          \
    __result;                   \
}))

#define pgm_read_word_elpm(addr)         \
(__extension__({                            \
    uint16_t __addr16 = (uint16_t)(addr);   \
    uint16_t __result;                      \
    __asm__ __volatile__                    \
    (                                       \
        "elpm"           "\n\t"              \
        "mov %A0, r0"   "\n\t"              \
        "adiw r30, 1"   "\n\t"              \
        "elpm"           "\n\t"              \
        "mov %B0, r0"   "\n\t"              \
        : "=r" (__result), "=z" (__addr16)  \
        : "1" (__addr16)                    \
        : "r0"                              \
    );                                      \
    __result;                               \
}))

static void NOINLINE write_cmd_data_seq_P(const uint8_t *seq) {
  uint8_t cmd;
  while((cmd = pgm_read_byte_elpm(seq++)) != 0) {
    write_cmd(cmd);
    uint8_t ndata = pgm_read_byte_elpm(seq++);
    for (uint8_t i=0; i<ndata; i++) {
      uint8_t data = pgm_read_byte_elpm(seq++);
      write_data(data);
    }
  }
}

static void NOINLINE lcd_set_window(lcd_xcoord_t xs, lcd_xcoord_t xe, lcd_ycoord_t ys, lcd_ycoord_t ye) {
  write_cmd(COLUMN_ADDRESS_SET);
  write_data16(xs);
  write_data16(xe);
  
  write_cmd(PAGE_ADDRESS_SET);
  write_data16(ys);
  write_data16(ye);
  
}

MODULE_FN_PROTOS(lcd, LCD_FUNCTION_EXPORTS)

void lcd_init(display_t*) {

#ifdef __AVR_AT90USB1286__
  
  XMCRB = _BV(XMM2) | _BV(XMM1);
  XMCRA = _BV(SRE);
  RESET_DDR |= _BV(RESET_BIT);
  BLC_DDR |= _BV(BLC_BIT);
#else
#endif

	uint16_t x, y;
	RESET_lo();
	_delay_ms(100);
	RESET_hi();
	_delay_ms(100);
#ifndef __AVR_AT90USB1286__
	RS_hi();
	WR_hi();
	RD_hi(); 
	CS_lo();
#endif
	BLC_lo();
	VSYNC_hi();
	write_cmd(DISPLAY_OFF);
	write_cmd(SLEEP_OUT);
	_delay_ms(60);
  static const uint8_t PROGMEM init_seq[] = {
    INTERNAL_IC_SETTING,			 1, 0x01,
	  POWER_CONTROL_1,				 2, 0x26, 0x08,
    POWER_CONTROL_2,				 1, 0x10,
    VCOM_CONTROL_1,				 2, 0x35, 0x3E,
    MEMORY_ACCESS_CONTROL,		 1, 0x48,
    RGB_INTERFACE_SIGNAL_CONTROL, 1, 0x4A, // Set the DE/Hsync/Vsync/Dotclk polarity
    FRAME_CONTROL_IN_NORMAL_MODE, 2, 0x00, 0x1B, // 7
    DISPLAY_FUNCTION_CONTROL,	 4, 0x0A, 0x82, 0x27, 0x00,
    VCOM_CONTROL_2,			     1, 0xB5,
    INTERFACE_CONTROL,			 3, 0x01, 0x00, 0x00, // System interface
    GAMMA_DISABLE,				 1, 0x00, 
    GAMMA_SET,					 1, 0x01, // Select Gamma curve
    PIXEL_FORMAT_SET,			 1, 0x55, // 0x66 - 18bit /pixel,  0x55 - 16bit/pixel
    POSITIVE_GAMMA_CORRECTION,	15, 0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0x87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00,
    NEGATIVE_GAMMA_CORRECTION,	15, 0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F,
    COLUMN_ADDRESS_SET,			 4, 0x00, 0x00, 0x00, 0xEF,
    PAGE_ADDRESS_SET,			 4, 0x00, 0x00, 0x01, 0x3F,
    TEARING_EFFECT_LINE_OFF, 0,
    DISPLAY_INVERSION_CONTROL,	 1, 0x00,
    ENTRY_MODE_SET,				 1, 0x07,
    MEMORY_WRITE, 0,
    0
  };
  write_cmd_data_seq_P(init_seq+GET_MODULE_DATA_PTR_OFFSET());
  // write_cmd_data_seq_P(init_seq);
    /* Clear display */
	for(x=0; x<240; x++)
		for(y=0; y<320; y++)
			write_data16(0x0000);
	write_cmd(DISPLAY_ON);
	_delay_ms(50);
	BLC_hi();
}

void lcd_set_pixel(display_t *, display_coord_t coord, lcd_colour_t col) {
  lcd_set_window(coord.x, coord.x, coord.y, coord.y);
  write_cmd(MEMORY_WRITE);
  write_data16(col);
}

void lcd_region_set(display_t*, display_region_t r) {
  lcd_set_window(r.x1, r.x2, r.y1, r.y2);
}

void lcd_fill(display_t*, display_colour_t col, uint32_t n) {
  write_cmd(MEMORY_WRITE);
  for (; n ; --n) {
    write_data16(col);
  }
}

void lcd_fill_indexed(display_t*, display_colour_t *col, uint32_t n) {
  write_cmd(MEMORY_WRITE);
  for (; n; --n) {
    write_data16(*col++);
  }
}

void lcd_fill_indexedP(display_t*, display_colour_t *col, uint32_t n) {
  write_cmd(MEMORY_WRITE);
  for (; n; --n) {
    write_data16(pgm_read_word_elpm(col++));
  }
}

void lcd_set_orientation(lcd_t *, orientation o) {
  
	write_cmd(MEMORY_ACCESS_CONTROL);
	if (o==North) { 
		// display.width = LCDWIDTH;
		// display.height = LCDHEIGHT;
		write_data(0x48);
	}
	else if (o==West) {
		// display.width = LCDHEIGHT;
		// display.height = LCDWIDTH;
		write_data(0xE8);
	}
	else if (o==South) {
		// display.width = LCDWIDTH;
		// display.height = LCDHEIGHT;
		write_data(0x88);
	}
	else if (o==East) {
		// display.width = LCDHEIGHT;
		// display.height = LCDWIDTH;
		write_data(0x28);
	}
}

lcd_xcoord_t lcd_get_width(lcd_t* this) {
  return lcd_get_size(this).x;
}

lcd_ycoord_t lcd_get_height(lcd_t* this) {
  return lcd_get_size(this).y;
}

lcd_coord_t lcd_get_size(lcd_t*) {
  lcd_coord_t size;
  write_cmd(MEMORY_ACCESS_CONTROL);
  
  size.x = LCDWIDTH;
  size.y = LCDHEIGHT;
  return size;
}



REGISTER_MODULE(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS, LCD_API_VER);
