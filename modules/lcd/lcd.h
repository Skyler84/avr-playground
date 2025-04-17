#pragma once

#include <stdint.h>

#include "module.h"

#define EXPORTED_FUNCTIONS(o)\
o(lcd_init, void)\
o(lcd_clear, void, lcd_color_t)\
o(lcd_set_pixel, void, lcd_xcoord_t,lcd_ycoord_t,lcd_color_t)\
o(lcd_fill_rect, void, lcd_xcoord_t,lcd_xcoord_t,lcd_ycoord_t,lcd_ycoord_t,lcd_color_t)\
o(lcd_fill_rect_mapped, void, lcd_xcoord_t,lcd_xcoord_t,lcd_ycoord_t,lcd_ycoord_t,lcd_color_t*)

typedef uint16_t lcd_color_t;
typedef uint16_t lcd_xcoord_t;
typedef uint16_t lcd_ycoord_t;

EXPORTED_FUNCTIONS(DECLARE_FN_TYPE);
#define LCD_API_VER 0x0001

enum FunctionID {
    LCD_FN_ID_NONE = 0,
    EXPORTED_FUNCTIONS(ENUM_FN_ID)
};
