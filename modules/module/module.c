#include "module/module.h"
#include "module/pic.h"

#include <avr/pgmspace.h>
#include <stddef.h>

extern module_fn_t module_fn_table_default[] __attribute__((weak));

fn_ptr_t module_fn_lookup(uint16_t _id, moduleptr_t module) {
  uint32_t fn = 0;
  if (module == 0) {
    // fn = (uint32_t)module_fn_table_default;
  } else {
    fn = (uint32_t)(module + (uint32_t)offsetof(module_t, fn_table));
  }
  if (fn == 0) {
    return NULL;
  }
  uint16_t module_type = pgm_read_word_far(module + offsetof(module_t, type));
  uint16_t id;
  while ((id = pgm_read_word_far(fn + (uint32_t)offsetof(module_fn_t, id))) != 0x0000U) {
    if (id == _id) {
      return (fn_ptr_t)(pgm_read_word_far(fn + (uint32_t)offsetof(module_fn_t, fn)) + (module_type == MODULE_TYPE_MODULE?pgm_ptr_to_fn_ptr(module):0));
    }
    fn+= sizeof(module_fn_t); 
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

void module_init_fns(fn_ptr_t fns[], module_id_t id, uint16_t fn_ids[]) {
  moduleptr_t module = module_find_by_id(id);
  if (module == (moduleptr_t)0) {
    indirect_call(module_runtime_init_error_handler)(id, &module, 0, NULL);
  }
  for (uint8_t i = 0; fn_ids[i]; i++) {
    fns[i] = module_fn_lookup(fn_ids[i], module);
    if (fns[i] == NULL) {
      indirect_call(module_runtime_init_error_handler)(id, &module, fn_ids[i], &fns[i]);
    }
  }
}

void module_runtime_init_error_handler(module_id_t, moduleptr_t*, uint16_t, fn_ptr_t *) __attribute__((weak));
void module_runtime_init_error_handler(module_id_t, moduleptr_t*, uint16_t, fn_ptr_t *) 
{
  while(1);
}

REGISTER_MODULE(module, MODULE_MODULE_ID, MODULE_FUNCTION_EXPORTS, MODULE_API_VER);