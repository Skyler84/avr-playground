#include "module.h"

#include <avr/pgmspace.h>

extern module_fn_t module_fn_table_default[] __attribute__((weak));

fn_ptr_t module_fn_lookup(uint16_t _id, module_t* module) {
  const module_fn_t *fn;
  if (module == NULL) {
    fn = module_fn_table_default;
  } else {
    fn = module->fn_table;
  }
  if (fn == NULL) {
    return NULL;
  }
  uint16_t id;
  while (id = pgm_read_word(&fn->id)) {
    if (id == _id) {
      return (fn_ptr_t)(pgm_read_word(&fn->fn) + pgm_ptr_to_fn_ptr(module));
    }
    fn++;
  }
  return NULL;
}

module_t *module_next(module_t* module) {
  uintptr_t addr = (uintptr_t)module;
  do{
    addr += 2;
    module_t *mod = (module_t*)addr;
    if (mod->magic == AVR_MODULE_MAGIC) {
      return mod;
    }
  } while(addr && addr < 0x20000);
  return NULL;
}

module_t *module_find_by_id(module_id_t id) {
  module_t *module = NULL;
  while (module = module_next(module)) {
    if (module->magic == AVR_MODULE_MAGIC && module->id == id) {
      return module;
    }
  }
  return NULL;
}

REGISTER_MODULE(module, MODULE_MODULE_ID, MODULE_FUNCTION_EXPORTS, MODULE_API_VER);