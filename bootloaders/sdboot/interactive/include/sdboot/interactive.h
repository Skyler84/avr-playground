#pragma once

#include "module/module.h"

#define SDBOOT_INTERACTIVE_FUNCTION_EXPORTS(modname, o) \
  o(modname, run, void, module_fns_t*) \

  // o(modname, sd_boot, void, ctx_t*) \

#define SDBOOT_INTERACTIVE_MODULE_ID 0x000f

DECLARE_MODULE(sdboot_interactive, SDBOOT_INTERACTIVE_MODULE_ID, SDBOOT_INTERACTIVE_FUNCTION_EXPORTS);