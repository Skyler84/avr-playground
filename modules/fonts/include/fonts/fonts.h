#pragma once
#include "module/module.h"

#define _(...)

#define FONTS_FUNCTION_EXPORTS(modname, o)\
o(modname, get_default, font_t*)\
_(modname, get_font_by_id, const __flash uint8_t*, uint16_t id)\
_(modname, get_font_by_name, const __flash uint8_t*, const char* name)\
_(modname, get_font_name, const char*, uint16_t id)\
o(modname, get_char_width,  uint8_t  ,const font_t* font, uint8_t *bitmap_char) \
o(modname, get_char_height, uint8_t  ,const font_t* font, uint8_t *bitmap_char) \
o(modname, get_char_bitmap, const uint8_t *,const font_t* font, wchar_t c) \
  
typedef struct char_mapping{
  int16_t char_code;
  uint16_t index;
} char_mapping_t;

typedef const __flash struct font{
  uint8_t char_width, char_height;
  uint16_t char_map_size; // number of charmap characters, useful for binary search
  uint8_t data[];
} font_t;

#define FONTS_API_VER 1
#define FONTS_MODULE_ID 0x0101

DECLARE_MODULE(fonts, FONT_MODULE_ID, FONTS_FUNCTION_EXPORTS);

