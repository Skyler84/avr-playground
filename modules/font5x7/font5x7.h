#pragma once

#include <stdint.h>

typedef struct{
  uint8_t width; // width of the font in pixels
  uint8_t height; // height of the font in pixels
  const uint8_t data[]; // pointer to the font data
} font5x7_internal_t;

extern const font5x7_internal_t font5x7;