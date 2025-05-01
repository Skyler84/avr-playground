#pragma once

#include "module/module.h"
#include <stdint.h>

/* Colour definitions RGB565 */
#define WHITE       0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F      
#define GREEN       0x07E0      
#define CYAN        0x07FF      
#define RED         0xF800      
#define MAGENTA     0xF81F      
#define YELLOW      0xFFE0     

typedef uint16_t display_xcoord_t;
typedef uint16_t display_ycoord_t;
typedef uint16_t display_colour_t;

typedef struct display_coord{
    display_xcoord_t x;
    display_ycoord_t y;
}display_coord_t;

typedef struct display_region{
    display_xcoord_t x1;
    display_xcoord_t y1;
    display_xcoord_t x2;
    display_ycoord_t y2;
}display_region_t;

typedef struct display_fns display_fns_t;
typedef struct display display_t;

#define DISPLAY_FUNCTION_INTERFACE(modname, o) \
    o(modname, init, void) \
    o(modname, set_pixel, void, display_t *, display_coord_t, display_colour_t) \
    o(modname, region_set, void, display_t *, display_region_t) \
    o(modname, fill, void, display_t *, display_colour_t, size_t) \
    o(modname, fill_indexed, void, display_t *, display_colour_t *, size_t) \
    o(modname, fill_rectangle, void, display_t *, display_region_t, display_colour_t) \

MODULE_DECLARE_FN_IDS(display, DISPLAY_FUNCTION_INTERFACE)
MODULE_DECLARE_FN_TYPES(display, DISPLAY_FUNCTION_INTERFACE)
MODULE_DECLARE_FNS(display, DISPLAY_FUNCTION_INTERFACE);

struct display{
    display_fns_t *fns;
};
