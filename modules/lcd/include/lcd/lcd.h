#pragma once

#include "display/display.h"
#include "module/module.h"
#define _(...)

typedef display_xcoord_t lcd_xcoord_t;
typedef display_ycoord_t lcd_ycoord_t;
typedef display_colour_t lcd_colour_t;

#define LCDWIDTH	240
#define LCDHEIGHT	320

/* Colour definitions RGB565 */
#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F      
#define GREEN       0x07E0      
#define CYAN        0x07FF      
#define RED         0xF800      
#define MAGENTA     0xF81F      
#define YELLOW      0xFFE0     


typedef enum {North, West, South, East} orientation;

typedef display_coord_t lcd_coord_t;

typedef struct lcd lcd_t;


#define LCD_FUNCTION_EXPORTS(modname, o)\
    DISPLAY_FUNCTION_INTERFACE(modname, o) \
    o(modname,set_orientation, void, lcd_t*, orientation o)\
    o(modname,get_width, lcd_xcoord_t, lcd_t*) \
    o(modname,get_height, lcd_ycoord_t, lcd_t*) \
    o(modname,get_size, lcd_coord_t, lcd_t*) \


#define LCD_API_VER 1
#define LCD_MODULE_ID 0x0100

DECLARE_MODULE(lcd, LCD_MODULE_ID, LCD_FUNCTION_EXPORTS);


struct lcd{
    union{
        display_t display;
        lcd_fns_t *fns;
    };
};