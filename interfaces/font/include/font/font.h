#pragma once
#include "module/module.h"
#include "display/display.h"
#include <stdbool.h>

#define _(...)

typedef struct font font_t;

#define FONT_FUNCTION_INTERFACE(modname, o)\
  o(modname, char_get_bounds,  display_region_t , const font_t* font, wchar_t c, uint16_t ppiscale_pt) \
  o(modname, get_char_pixel,   bool             , const font_t* font, wchar_t c, display_coord_t, uint16_t ppiscale_pt) \
  o(modname, get_char_advance, display_coord_t  , const font_t* font, wchar_t c, uint16_t ppiscale_pt) \

MODULE_DECLARE_FN_IDS(font, FONT_FUNCTION_INTERFACE)
MODULE_DECLARE_FN_TYPES(font, FONT_FUNCTION_INTERFACE)
MODULE_DECLARE_FNS(font, FONT_FUNCTION_INTERFACE);

struct font{
  font_fns_t *fns;
};