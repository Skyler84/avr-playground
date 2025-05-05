#include "module/module.h"

#include <avr/pgmspace.h>
#include <stddef.h>

extern module_fn_t module_fn_table_default[] __attribute__((weak));

fn_ptr_t module_fn_lookup(uint16_t _id, moduleptr_t module) {
  const module_fn_t *fn;
  if (module == 0) {
    fn = module_fn_table_default;
  } else {
    fn = (module_fn_t*)pgm_read_word_far(module + offsetof(module_t, fn_table));
  }
  if (fn == NULL) {
    return NULL;
  }
  uint16_t id;
  while ((id = pgm_read_word(&fn->id)) != 0x0000U) {
    if (id == _id) {
      return (fn_ptr_t)(pgm_read_word(&fn->fn) + pgm_ptr_to_fn_ptr(module));
    }
    fn++;
  }
  return NULL;
}

moduleptr_t module_next(moduleptr_t module) {
  moduleptr_t addr = (moduleptr_t)module & ~1;
  do{
    addr += 2;
    moduleptr_t mod = (moduleptr_t)addr;
    uint32_t magic = pgm_read_dword_far(addr);
    if (magic == AVR_MODULE_MAGIC) {
      return mod;
    }
  } while(addr && addr < 0x20000);
  return (moduleptr_t)0;
}

moduleptr_t module_find_by_id(module_id_t id) {
  moduleptr_t module = (moduleptr_t)0x0000;
  while ((module = module_next(module)) != 0) {
    uint16_t mod_id = pgm_read_word_far(module + offsetof(module_t, id));
    if (mod_id == id) {
      return module;
    }
  }
  return (moduleptr_t)0;
}

REGISTER_MODULE(module, MODULE_MODULE_ID, MODULE_FUNCTION_EXPORTS, MODULE_API_VER);