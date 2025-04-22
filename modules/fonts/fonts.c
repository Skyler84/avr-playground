#include "fonts.h"
#include "font5x7.h"
#include "pic.h"

MODULE_FN_PROTOS(fonts, FONTS_FUNCTION_EXPORTS)


font_t* fonts_get_default() {
  uintptr_t mod_offset = GET_MODULE_DATA_PTR_OFFSET();
  return (font_t*)(((uintptr_t)&font5x7)+mod_offset);
}

uint8_t fonts_get_char_width(const font_t* font, uint8_t *bitmap_char) {
  return 5;
  if (font->char_width > 0) {
    return font->char_width;
  }
  return bitmap_char[-1];
}

uint8_t fonts_get_char_height(const font_t* font, uint8_t *bitmap_char) {
  return 7;
  if (font->char_height > 0) {
    return font->char_height;
  }
  if (font->char_width > 0) {
    return bitmap_char[-2];
  }
  return bitmap_char[-1];
}


const uint8_t *fonts_get_char_bitmap(const font_t* font, int16_t c) {
  if (c < ' ' || c > '~') {
    return NULL;
  }
  return font->data + (c - ' ')*5;
  const __flash char_mapping_t *char_map = (const __flash char_mapping_t*)font->data;
  for (uint16_t i = 0; i < font->char_map_size; i++) {
    if (char_map[i].char_code == c) {
      return font->data + font->char_map_size*sizeof(char_mapping_t) + char_map[i].index;
    }
  }
  return NULL;
}

REGISTER_MODULE(fonts, FONTS_MODULE_ID, FONTS_FUNCTION_EXPORTS, FONTS_API_VER);