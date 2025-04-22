#pragma once

#include "display.h"
#include "module.h"
#include "fonts.h"
#define _(...)

typedef display_xcoord_t lcd_xcoord_t;
typedef display_ycoord_t lcd_ycoord_t;
typedef display_colour_t lcd_colour_t;


#define LCDWIDTH	240
#define LCDHEIGHT	320

typedef enum {North, West, South, East} orientation;

typedef struct lcd_coord{
    lcd_xcoord_t x;
    lcd_ycoord_t y;
}lcd_coord_t;

#define LCD_FUNCTION_EXPORTS(modname, o)\
o(modname,init, void)\
o(modname,set_orientation, void, orientation o)\
o(modname,clear, void, lcd_colour_t)\
o(modname,set_pixel, void, lcd_xcoord_t, lcd_ycoord_t, lcd_colour_t)\
o(modname,fill_rectangle, void, lcd_xcoord_t, lcd_xcoord_t, lcd_ycoord_t, lcd_ycoord_t, lcd_colour_t)\
o(modname,fill_rectangle_indexed, void, lcd_xcoord_t, lcd_xcoord_t, lcd_ycoord_t, lcd_ycoord_t, const lcd_colour_t*)\
o(modname,fill_rectangle_indexedP, void, lcd_xcoord_t, lcd_xcoord_t, lcd_ycoord_t, lcd_ycoord_t, const lcd_colour_t*)\
o(modname,display_char, void, lcd_xcoord_t, lcd_ycoord_t, uint8_t scale, font_t *font, fonts_fns_t *font_fns, char c, lcd_colour_t col)\
o(modname,display_string, void, lcd_xcoord_t, lcd_xcoord_t, lcd_ycoord_t, uint8_t scale, font_t *font, fonts_fns_t *font_fns, const char *s, lcd_colour_t col)\
o(modname,display_stringP, void, lcd_xcoord_t, lcd_xcoord_t, lcd_ycoord_t, uint8_t scale, font_t *font, fonts_fns_t *font_fns, const char *s, lcd_colour_t col)\
o(modname,get_width, lcd_xcoord_t) \
o(modname,get_height, lcd_ycoord_t) \
o(modname,get_size, lcd_coord_t) \


#define LCD_API_VER 1
#define LCD_MODULE_ID 0x0100

DECLARE_MODULE(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS);
