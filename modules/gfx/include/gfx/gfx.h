#pragma once

#include "module/module.h"
#include "display/display.h"
#include "fonts/fonts.h"

#include <stdbool.h>

typedef display_coord_t gfx_coord_t;
typedef display_region_t gfx_region_t;
typedef display_colour_t gfx_colour_t;

typedef struct gfx gfx_t;

#define GFX_FUNCTION_EXPORTS(modname, o) \
    o(modname, init, void, gfx_t *gfx, display_t *display) \
    o(modname, colour, gfx_colour_t, uint8_t, uint8_t, uint8_t) \
    o(modname, colourA, gfx_colour_t, uint8_t, uint8_t, uint8_t, uint8_t) \
    o(modname, fill, void, gfx_t *gfx, gfx_colour_t colour) \
    o(modname, nofill, void, gfx_t *gfx) \
    o(modname, stroke, void, gfx_t *gfx, gfx_colour_t colour) \
    o(modname, strokeWeight, void, gfx_t *gfx, uint8_t weight) \
    o(modname, nostroke, void, gfx_t *gfx)\
    o(modname, point, void, gfx_t *gfx, gfx_coord_t coord) \
    o(modname, line, void, gfx_t *gfx, gfx_coord_t start, gfx_coord_t end) \
    o(modname, rectangle, void, gfx_t *gfx, gfx_region_t r) \
    o(modname, circle, void, gfx_t *gfx, gfx_coord_t center, uint8_t radius) \
    o(modname, ellipse, void, gfx_t *gfx, gfx_coord_t focusA, gfx_coord_t focusB, uint8_t radius) \
    o(modname, triangle, void, gfx_t *gfx, gfx_coord_t a, gfx_coord_t b, gfx_coord_t c, gfx_colour_t colour) \
    o(modname, arc, void, gfx_t *gfx, gfx_coord_t center, uint8_t radiusX, uint8_t radiusY, uint16_t startAngle, uint16_t endAngle) \
    o(modname, text, void, gfx_t *gfx, gfx_region_t box, const char *text) \
    o(modname, textP, void, gfx_t *gfx, gfx_region_t coord, const char *text) \
    o(modname, textSize, void, gfx_t *gfx) \
    o(modname, textFont, void, gfx_t *gfx, font_t *font)

#define GFX_MODULE_ID 0x0108

DECLARE_MODULE(gfx, GFX_MODULE_ID, GFX_FUNCTION_EXPORTS);

struct gfx {
    gfx_fns_t *fns;
    display_t *display;
    bool fill;
    bool stroke;
    uint8_t strokeWeight;
    gfx_colour_t strokeColour;
    gfx_colour_t fillColour;
    font_t *font;

};