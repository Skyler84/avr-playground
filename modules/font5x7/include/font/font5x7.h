#pragma once
#include "module/module.h"
#include "font/font.h"

#define _(...)

typedef struct font5x7 font5x7_t;

#define FONT5X7_FUNCTION_EXPORTS(modname, o)\
  FONT_FUNCTION_INTERFACE(modname, o) \
  o(modname, init, void, font5x7_t* font) \
  

#define FONT5X7_API_VER 1
#define FONT5X7_MODULE_ID 0x0111  

#ifndef font5x7_MODTYPE
#error "font5x7_MODTYPE not defined"
#endif

DECLARE_MODULE(font5x7, FONT5X7_MODULE_ID, FONT5X7_FUNCTION_EXPORTS);

struct font5x7{
  union{
    font_t base;
    font5x7_fns_t *fns;
  };
};