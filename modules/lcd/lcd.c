#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "lcd.h"
#include "pic.h"

#define LCD_CMD_NO_OPERATION								0x00
#define LCD_CMD_SOFTWARE_RESET								0x01
#define LCD_CMD_READ_DISPLAY_IDENTIFICATION_INFORMATION		0x04
#define LCD_CMD_READ_DISPLAY_STATUS							0x09
#define LCD_CMD_READ_DISPLAY_POWER_MODE						0x0A
#define LCD_CMD_READ_DISPLAY_MADCTL							0x0B
#define LCD_CMD_READ_DISPLAY_PIXEL_FORMAT					0x0C
#define LCD_CMD_READ_DISPLAY_IMAGE_FORMAT					0x0D
#define LCD_CMD_READ_DISPLAY_SIGNAL_MODE					0x0E
#define LCD_CMD_READ_DISPLAY_SELF_DIAGNOSTIC_RESULT			0x0F
#define LCD_CMD_ENTER_SLEEP_MODE							0x10
#define LCD_CMD_SLEEP_OUT									0x11	
#define LCD_CMD_PARTIAL_MODE_ON								0x12
#define LCD_CMD_NORMAL_DISPLAY_MODE_ON						0x13
#define LCD_CMD_DISPLAY_INVERSION_OFF						0x20
#define LCD_CMD_DISPLAY_INVERSION_ON						0x21
#define LCD_CMD_GAMMA_SET									0x26
#define LCD_CMD_DISPLAY_OFF									0x28
#define LCD_CMD_DISPLAY_ON									0x29
#define LCD_CMD_COLUMN_ADDRESS_SET							0x2A
#define LCD_CMD_PAGE_ADDRESS_SET							0x2B
#define LCD_CMD_MEMORY_WRITE								0x2C
#define LCD_CMD_COLOR_SET									0x2D
#define LCD_CMD_MEMORY_READ									0x2E
#define LCD_CMD_PARTIAL_AREA								0x30
#define LCD_CMD_VERTICAL_SCROLLING_DEFINITION				0x33
#define LCD_CMD_TEARING_EFFECT_LINE_OFF						0x34
#define LCD_CMD_TEARING_EFFECT_LINE_ON						0x35
#define LCD_CMD_MEMORY_ACCESS_CONTROL						0x36	
#define LCD_CMD_VERTICAL_SCROLLING_START_ADDRESS			0x37
#define LCD_CMD_IDLE_MODE_OFF								0x38
#define LCD_CMD_IDLE_MODE_ON								0x39
#define LCD_CMD_PIXEL_FORMAT_SET							0x3A
#define LCD_CMD_WRITE_MEMORY_CONTINUE						0x3C
#define LCD_CMD_READ_MEMORY_CONTINUE						0x3E
#define LCD_CMD_SET_TEAR_SCANLINE							0x44
#define LCD_CMD_GET_SCANLINE								0x45
#define LCD_CMD_WRITE_DISPLAY_BRIGHTNESS					0x51
#define LCD_CMD_READ_DISPLAY_BRIGHTNESS						0x52
#define LCD_CMD_WRITE_CTRL_DISPLAY							0x53
#define LCD_CMD_READ_CTRL_DISPLAY							0x54
#define LCD_CMD_WRITE_CONTENT_ADAPTIVE_BRIGHTNESS_CONTROL	0x55
#define LCD_CMD_READ_CONTENT_ADAPTIVE_BRIGHTNESS_CONTROL	0x56
#define LCD_CMD_WRITE_CABC_MINIMUM_BRIGHTNESS				0x5E
#define LCD_CMD_READ_CABC_MINIMUM_BRIGHTNESS				0x5F
#define LCD_CMD_READ_ID1									0xDA
#define LCD_CMD_READ_ID2									0xDB
#define LCD_CMD_READ_ID3									0xDC

/* Extended Commands */
#define LCD_CMD_RGB_INTERFACE_SIGNAL_CONTROL				0xB0
#define LCD_CMD_FRAME_CONTROL_IN_NORMAL_MODE				0xB1
#define LCD_CMD_FRAME_CONTROL_IN_IDLE_MODE					0xB2
#define LCD_CMD_FRAME_CONTROL_IN_PARTIAL_MODE				0xB3
#define LCD_CMD_DISPLAY_INVERSION_CONTROL					0xB4
#define LCD_CMD_BLANKING_PORCH_CONTROL						0xB5
#define LCD_CMD_DISPLAY_FUNCTION_CONTROL					0xB6
#define LCD_CMD_ENTRY_MODE_SET								0xB7
#define LCD_CMD_BACKLIGHT_CONTROL_1							0xB8
#define LCD_CMD_BACKLIGHT_CONTROL_2							0xB9
#define LCD_CMD_BACKLIGHT_CONTROL_3							0xBA
#define LCD_CMD_BACKLIGHT_CONTROL_4							0xBB
#define LCD_CMD_BACKLIGHT_CONTROL_5							0xBC
#define LCD_CMD_BACKLIGHT_CONTROL_7							0xBE
#define LCD_CMD_BACKLIGHT_CONTROL_8							0xBF
#define LCD_CMD_POWER_CONTROL_1								0xC0
#define LCD_CMD_POWER_CONTROL_2								0xC1
#define LCD_CMD_POWER_CONTROL3_(FOR_NORMAL_MODE)			0xC2
#define LCD_CMD_POWER_CONTROL4_(FOR_IDLE_MODE)				0xC3
#define LCD_CMD_POWER_CONTROL5_(FOR_PARTIAL_MODE)			0xC4
#define LCD_CMD_VCOM_CONTROL_1								0xC5
#define LCD_CMD_VCOM_CONTROL_2								0xC7
#define LCD_CMD_NV_MEMORY_WRITE								0xD0
#define LCD_CMD_NV_MEMORY_PROTECTION_KEY					0xD1
#define LCD_CMD_NV_MEMORY_STATUS_READ						0xD2
#define LCD_CMD_READ_ID4									0xD3
#define LCD_CMD_POSITIVE_GAMMA_CORRECTION					0xE0
#define LCD_CMD_NEGATIVE_GAMMA_CORRECTION					0xE1
#define LCD_CMD_DIGITAL_GAMMA_CONTROL						0xE2
#define LCD_CMD_DIGITAL_GAMMA_CONTROL2						0xE3
#define LCD_CMD_INTERFACE_CONTROL							0xF6

/* Undocumented commands */
#define LCD_CMD_INTERNAL_IC_SETTING							0xCB
#define LCD_CMD_GAMMA_DISABLE								0xF2


#define u16_HIbyte(x) (x>>8)
#define u16_LObyte(x) (x&0xff)

// #define NOINLINE __attribute__((noinline))
#ifndef NOINLINE
#define NOINLINE
#endif

EXPORTED_FUNCTIONS(DECLARE_FN);

#define LCD_WIDTH 240
#define LCD_HEIGHT 320

#if defined(__AVR_ATmega644P__)

#define CTRL A
#define DATA C
#define XCAT(a, b) a##b
#define CAT(a, b) XCAT(a, b)

#define CTRL_PORT CAT(PORT, CTRL)
#define DATA_PORT CAT(PORT, DATA)
#define CTRL_DDR CAT(DDR, CTRL)
#define DATA_DDR CAT(DDR, DATA)

#define CS_PIN _BV(0)
#define BLC_PIN _BV(1)
#define RESET_PIN _BV(2)
#define WR_PIN _BV(3)
#define RS_PIN _BV(4)
#define RD_PIN _BV(5)
#define VSYNC_PIN _BV(6)

#define CS_LO() do{CTRL_PORT&=~CS_PIN;}while(0)
#define CS_HI() do{CTRL_PORT|=CS_PIN;}while(0)
#define BLC_LO() do{CTRL_PORT&=~BLC_PIN;}while(0)
#define BLC_HI() do{CTRL_PORT|=BLC_PIN;}while(0)
#define RESET_LO() do{CTRL_PORT&=~RESET_PIN;}while(0)
#define RESET_HI() do{CTRL_PORT|=RESET_PIN;}while(0)
#define RS_LO() do{CTRL_PORT&=~RS_PIN;}while(0)
#define RS_HI() do{CTRL_PORT|=RS_PIN;}while(0)
#define WR_LO() do{CTRL_PORT&=~WR_PIN;}while(0)
#define WR_HI() do{CTRL_PORT|=WR_PIN;}while(0)
#define RD_LO() do{CTRL_PORT&=~RD_PIN;}while(0)
#define RD_HI() do{CTRL_PORT|=RD_PIN;}while(0)
#define VSYNC_LO() do{CTRL_PORT&=~VSYNC_PIN;}while(0)
#define VSYNC_HI() do{CTRL_PORT|=VSYNC_PIN;}while(0)

#define LCD_RS_CMD RS_LO
#define LCD_RS_DATA RS_HI
#define LCD_WR(x) do{DATA_PORT=(x);WR_LO();WR_HI();}while(0)


static void NOINLINE lcd_cmd(uint8_t cmd)
{
    LCD_RS_CMD();
    LCD_WR(cmd);
}
static void NOINLINE lcd_data(uint8_t data)
{
    LCD_RS_DATA();
    LCD_WR(data);
}
static void NOINLINE lcd_data16(uint16_t data)
{
    LCD_RS_DATA();
    LCD_WR(u16_HIbyte(data));
    LCD_WR(u16_LObyte(data));
}

#elif defined(__AVR_AT90USB1286__)

#define LCD_CMD *((volatile uint8_t*)0x4000)
#define LCD_DATA *((volatile uint8_t*)0x4100)

#define RESET C, 7
#define BLC B,4
#define VSYNC C,1

#define CAT(a, b) a##b
#define XCAT(a, b) CAT(a, b)
#define FIRST(a, ...) a
#define SECOND(a, ...) FIRST(__VA_ARGS__)
#define PIN(p) _BV(SECOND(p))
#define PORT(p) XCAT(PORT, FIRST(p))
#define PORTPIN(p) XCAT(PIN, FIRST(p))
#define DDR(p) XCAT(DDR, FIRST(p))

#define CS_HI()
#define CS_LO()
#define RESET_HI() do{PORT(RESET) |= PIN(RESET);}while(0)
#define RESET_LO() do{PORT(RESET) &= ~PIN(RESET);}while(0)
#define BLC_HI() do{PORT(BLC) |= PIN(BLC);}while(0)
#define BLC_LO() do{PORT(BLC) &= ~PIN(BLC);}while(0)
#define VSYNC_HI() do{PORT(VSYNC) |= PIN(VSYNC);}while(0)
#define VSYNC_LO() do{PORT(VSYNC) &= ~PIN(VSYNC);}while(0)


static void NOINLINE lcd_cmd(uint8_t cmd)
{
    LCD_CMD = cmd;
}
static void NOINLINE lcd_data(uint8_t data)
{
    LCD_DATA = data;
}
static void NOINLINE lcd_data16(uint16_t data)
{
    LCD_DATA = u16_HIbyte(data);
    LCD_DATA = u16_LObyte(data);
}
#else
#error "Unknown mmcu"
#endif

static void NOINLINE lcd_cmd_dataP(uint8_t cmd, uint8_t ndata, const uint8_t *data)
{
    lcd_cmd(cmd);
    while(ndata--)
        lcd_data(pgm_read_byte(data++));
}

/**
 * iterate over a run-length-encoded of sorts
 * array of data rather than deal with the overhead
 * of lots and lots of sequential function calls!
 */
static void NOINLINE lcd_cmd_data_seqP(const uint8_t *seq)
{
    uint8_t cmd;
    while(cmd = pgm_read_byte(seq++)){
        lcd_cmd(cmd);
        uint8_t ndata = pgm_read_byte(seq++);
        while(ndata--)
            lcd_data(pgm_read_byte(seq++));
    }
}

static void NOINLINE lcd_set_window(lcd_xcoord_t xs, lcd_xcoord_t xe, lcd_ycoord_t ys, lcd_ycoord_t ye) {
    lcd_cmd(LCD_CMD_COLUMN_ADDRESS_SET);
    lcd_data16(xs);
    lcd_data16(xe);    
    lcd_cmd(LCD_CMD_PAGE_ADDRESS_SET);
    lcd_data16(ys);
    lcd_data16(ye);
}

const volatile __flash int x = 1;

struct S{
    char m;
    const char *s;
};
const volatile __flash struct S s = {
    2, "a"
};

static void lcd_init() {
    #if defined (__AVR_ATMega644P__)
    DATA_DDR = 0xff;
    CTRL_DDR = 0x7f;
    #endif

    // lcd_cmd(x);
    DDRB = s.m;
    
	uint16_t x, y;
	RESET_LO();
	_delay_ms(100);
	RESET_HI();
	_delay_ms(100);
    #if defined (__AVR_ATMega644P__)
	RS_HI();
	WR_HI();
	RD_HI(); 
	CS_LO();
    #endif
	BLC_LO();
	VSYNC_HI();
	lcd_cmd(LCD_CMD_DISPLAY_OFF);
	lcd_cmd(LCD_CMD_SLEEP_OUT);
	_delay_ms(60);
    const static uint8_t init_seq[] PROGMEM = {
        LCD_CMD_INTERNAL_IC_SETTING,	      1, 0x01,
        LCD_CMD_POWER_CONTROL_1,		      2, 0x26,0x08,
        LCD_CMD_POWER_CONTROL_2,		      1, 0x10,
        LCD_CMD_VCOM_CONTROL_1,				  2, 0x35,0x3E,
        LCD_CMD_MEMORY_ACCESS_CONTROL,		  1, 0x48,
        LCD_CMD_RGB_INTERFACE_SIGNAL_CONTROL, 1, 0x4A,  // Set the DE/Hsync/Vsync/Dotclk polarity
        LCD_CMD_FRAME_CONTROL_IN_NORMAL_MODE, 2, 0x00,0x1B, // 70Hz
        LCD_CMD_DISPLAY_FUNCTION_CONTROL,	  4, 0x0A,0x82,0x27,0x00,
        LCD_CMD_VCOM_CONTROL_2,			      1, 0xB5,
        LCD_CMD_INTERFACE_CONTROL,			  3, 0x01,0x00,0x00, // System interface
        LCD_CMD_GAMMA_DISABLE,				  1, 0x00, 
        LCD_CMD_GAMMA_SET,					  1, 0x01, // Select Gamma curve 1
        LCD_CMD_PIXEL_FORMAT_SET,			  1, 0x55, // 0x66 - 18bit /pixel,  0x55 - 16bit/pixel
        LCD_CMD_POSITIVE_GAMMA_CORRECTION,	 15, 0x1F,0x1A,0x18,0x0A,0x0F,0x06,0x45,0x87,0x32,0x0A,0x07,0x02,0x07,0x05,0x00,
        LCD_CMD_NEGATIVE_GAMMA_CORRECTION,	 15, 0x00,0x25,0x27,0x05,0x10,0x09,0x3A,0x78,0x4D,0x05,0x18,0x0D,0x38,0x3A,0x1F,
        LCD_CMD_COLUMN_ADDRESS_SET,			  4, 0x00,0x00,0x00,0xEF,
        LCD_CMD_PAGE_ADDRESS_SET,			  4, 0x00,0x00,0x01,0x3F,
        LCD_CMD_TEARING_EFFECT_LINE_OFF,      0,
        LCD_CMD_DISPLAY_INVERSION_CONTROL,	  1, 0x00,
        LCD_CMD_ENTRY_MODE_SET,				  1, 0x07,
        /* Clear display */
        LCD_CMD_MEMORY_WRITE,				  0,
        0
    };
    lcd_cmd_data_seqP(&init_seq[0] + GET_MODULE_DATA_PTR_OFFSET());
    // lcd_cmd_data_seqP(MODULE_ADJUST_DATA_PTR(&init_seq));
	for(x=0; x<240; x++)
		for(y=0; y<320; y++)
			lcd_data16(0x0000);
	lcd_cmd(LCD_CMD_DISPLAY_ON);
	_delay_ms(50);
	BLC_HI();
}

static void lcd_clear(lcd_color_t col) {
    lcd_fill_rect(0, LCD_WIDTH-1, 0, LCD_HEIGHT-1, col);
}

static void lcd_set_pixel(lcd_xcoord_t x, lcd_ycoord_t y, lcd_color_t col) {
    lcd_set_window(x, x, y, y);
    lcd_cmd(LCD_CMD_MEMORY_WRITE);
    lcd_data16(col);
}

static void lcd_draw_hline(lcd_xcoord_t xstart, lcd_xcoord_t xend, lcd_ycoord_t y, lcd_color_t col) {
    lcd_set_window(xstart, xend, y, y);
    lcd_cmd(LCD_CMD_MEMORY_WRITE);
    for (; xstart <= xend; xstart++)
        lcd_data16(col);
}

static void lcd_draw_vline(lcd_xcoord_t x, lcd_xcoord_t ystart, lcd_ycoord_t yend, lcd_color_t col) {
    lcd_set_window(x, x, ystart, yend);
    lcd_cmd(LCD_CMD_MEMORY_WRITE);
    for (; ystart <= yend; ystart++)
        lcd_data16(col);
}

static void lcd_fill_rect(lcd_xcoord_t xstart, lcd_xcoord_t xend, lcd_ycoord_t ystart, lcd_ycoord_t yend, lcd_color_t col) {
    lcd_set_window(xstart, xend, ystart, yend);
    lcd_xcoord_t x;
    lcd_ycoord_t y;
    for (x = xstart; x <= xend; x++) {
        for (y = ystart; y <= yend; y++) {
            lcd_data16(col);
        }
    }
}
static void lcd_fill_rect_mapped(lcd_xcoord_t xstart, lcd_xcoord_t xend, lcd_ycoord_t ystart, lcd_ycoord_t yend, lcd_color_t *col) {
    lcd_set_window(xstart, xend, ystart, yend);
    lcd_xcoord_t x;
    lcd_ycoord_t y;
    for (x = xstart; x <= xend; x++) {
        for (y = ystart; y <= yend; y++) {
            lcd_data16(*col++);
        }
    }
}


#define MODULE_NAME "liblcd"


REGISTER_MODULE(MODULE_NAME, EXPORTED_FUNCTIONS, LCD_API_VER);
