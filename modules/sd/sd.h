#pragma once

#include "module.h"
#include "blockdev.h"

#define _(...) 
#define SD_FUNCTION_EXPORTS(modname, o) \
  o(modname, preinit, void) \
  o(modname, initialise, uint8_t) \
  o(modname, detected,   uint8_t) \
  o(modname, rdblock,    uint8_t, void*, uint32_t, uint8_t*) \
  o(modname, wrblock,    uint8_t, void*, uint32_t, const uint8_t*)

#define SD_API_VER 1
#define SD_MODULE_ID 0x0103

DECLARE_MODULE(sd, SD_MODULE_ID, SD_FUNCTION_EXPORTS);