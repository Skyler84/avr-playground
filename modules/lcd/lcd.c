#include "lcd.h"
#include "ili934x.h"
#include "pic.h"

#include <stdint.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

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

static uint8_t read_data() {
  return DATA_REG;
}

#else
#pragma optimize(3)
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

#endif

inline static void write_data16(uint16_t data) {
  write_data((data) >> 8);
  write_data((data) & 0xFF);
}

static void NOINLINE write_cmd_data(uint8_t cmd, uint8_t ndata, char *data) {
  uint8_t i;
  char *d = data;
  write_cmd(cmd);
  for(i=0; i<ndata; i++)
    write_data(*d++);
}

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
  while(cmd = pgm_read_byte_elpm(seq++)) {
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

#pragma optimise(s)

void lcd_init() {

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
  const static uint8_t PROGMEM init_seq[] = {
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
    /* Clear display */
	for(x=0; x<240; x++)
		for(y=0; y<320; y++)
			write_data16(0x0000);
	write_cmd(DISPLAY_ON);
	_delay_ms(50);
	BLC_hi();
}

#pragma optimize(3)

void lcd_set_orientation(orientation o) {
  
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

void lcd_clear(lcd_colour_t col) {
  lcd_xcoord_t w;
  lcd_ycoord_t h;
  write_cmd(MEMORY_ACCESS_CONTROL);
  uint8_t data = read_data();
  switch(data) {
    case 0x48:
    case 0x88:
      w = LCDWIDTH;
      h = LCDHEIGHT;
      break;
    case 0xE8:
    case 0x28:
      w = LCDHEIGHT;
      h = LCDWIDTH;
      break;
  }
  lcd_set_window(0, w-1, 0, h-1);
  write_cmd(MEMORY_WRITE);
  for (lcd_xcoord_t x=0; x<LCDWIDTH; x++) {
    for (lcd_ycoord_t y=0; y<LCDHEIGHT; y++) {
      write_data16(col);
    }
  }
}

void lcd_set_pixel(lcd_xcoord_t x, lcd_ycoord_t y, lcd_colour_t col) {
  lcd_set_window(x, x, y, y);
  write_cmd(MEMORY_WRITE);
  write_data16(col);
}

void lcd_fill_rectangle(lcd_xcoord_t xs, lcd_xcoord_t xe, lcd_ycoord_t ys, lcd_ycoord_t ye, lcd_colour_t col) {
  lcd_set_window(xs, xe, ys, ye);
  write_cmd(MEMORY_WRITE);
  for (lcd_xcoord_t x=xs; x<=xe; x++) {
    for (lcd_ycoord_t y=ys; y<=ye; y++) {
      write_data16(col);
    }
  }
}

void lcd_fill_rectangle_indexed(lcd_xcoord_t xs, lcd_xcoord_t xe, lcd_ycoord_t ys, lcd_ycoord_t ye, const lcd_colour_t* col) {
  lcd_set_window(xs, xe, ys, ye);
  write_cmd(MEMORY_WRITE);
  for (lcd_xcoord_t x=xs; x<=xe; x++) {
    for (lcd_ycoord_t y=ys; y<=ye; y++) {
      write_data16(*col++);
    }
  }
}

void lcd_fill_rectangle_indexedP(lcd_xcoord_t xs, lcd_xcoord_t xe, lcd_ycoord_t ys, lcd_ycoord_t ye, const lcd_colour_t* col) {
  lcd_set_window(xs, xe, ys, ye);
  write_cmd(MEMORY_WRITE);
  for (lcd_xcoord_t x=xs; x<=xe; x++) {
    for (lcd_ycoord_t y=ys; y<=ye; y++) {
      write_data16(pgm_read_word_elpm(col++));
    }
  }
}

void lcd_display_char(lcd_xcoord_t x, lcd_ycoord_t y, uint8_t scale, font_t *font, fonts_fns_t *font_fns, char c, lcd_colour_t col)
{
  uint8_t char_width = font_fns->get_char_width(font, (uint8_t*)&c);
  uint8_t char_height = font_fns->get_char_height(font, (uint8_t*)&c);
  const uint8_t *bitmap_char = font_fns->get_char_bitmap(font, c);
  if (bitmap_char == NULL) {
    bitmap_char = font_fns->get_char_bitmap(font, '?');
  }
  for (uint8_t i=0; i<char_width; i++) {
    for (uint8_t j=0; j<char_height; j++) {
      if (pgm_read_byte_elpm(&bitmap_char[i]) & (1 << j)) {
        lcd_fill_rectangle(x+i*scale, x+(i+1)*scale-1, y+j*scale, y+(j+1)*scale-1, col);
      }
    }
  }
}

void lcd_display_stringP(lcd_xcoord_t _x, lcd_xcoord_t wrap, lcd_ycoord_t y, uint8_t scale, font_t *font, fonts_fns_t *font_fns, const char* str, lcd_colour_t col) {
  char c;
  lcd_xcoord_t x = _x;
  uint8_t char_width = font_fns->get_char_width(font, (uint8_t*)&c);
  uint8_t char_height = font_fns->get_char_height(font, (uint8_t*)&c);
  while (c = pgm_read_byte_elpm(str++)) {
    lcd_display_char(x, y, scale, font, font_fns, c, col);
    uint8_t char_width = font_fns->get_char_width(font, (uint8_t*)&c);
    x += (char_width+1)*scale;
    if (x > wrap - font->char_width*scale-10) {
      uint8_t char_height = font_fns->get_char_height(font, (uint8_t*)&c);
      x = _x;
      y += (char_height+1)*scale;
    }
  }
}

void lcd_display_string(lcd_xcoord_t _x, lcd_xcoord_t wrap, lcd_ycoord_t y, uint8_t scale, font_t *font, fonts_fns_t *font_fns, const char* str, lcd_colour_t col) {
  char c;
  lcd_xcoord_t x = _x;
  while (c = *str++) {
    lcd_display_char(x, y, scale, font, font_fns, c, col);
    uint8_t char_width = font_fns->get_char_width(font, (uint8_t*)&c);
    x += (char_width+1)*scale;
    if (x > wrap - font->char_width*scale-10) {
      uint8_t char_height = font_fns->get_char_height(font, (uint8_t*)&c);
      x = _x;
      y += (char_height+1)*scale;
    }
  }
}

lcd_xcoord_t lcd_get_width() {
  return lcd_get_size().x;
}

lcd_ycoord_t lcd_get_height() {
  return lcd_get_size().y;
}

lcd_coord_t lcd_get_size(orientation o) {
  lcd_coord_t size;
  write_cmd(MEMORY_ACCESS_CONTROL);
  uint8_t data = read_data();
  switch(o) {
    case North:
    case South:
      size.x = LCDWIDTH;
      size.y = LCDHEIGHT;
      break;
    case East:
    case West:
      size.x = LCDHEIGHT;
      size.y = LCDWIDTH;
      break;
  }
  return size;
}


REGISTER_MODULE(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS, LCD_API_VER);
