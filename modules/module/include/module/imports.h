#pragma once

#define IMPORT_FN(modname, name, ...)\
  modname##_fns.name = (modname##_##name##_fn_t)module_fn_lookup(modname##_##name##_fn_id, module);\
  if (modname##_fns.name == NULL) {while(1);}

#define MODULE_IMPORT_FUNCTIONS(modname, id, exports)\
  DEFINE_IMPORTED_FNS(modname, exports);\
  static void __attribute__((constructor)) modname##_module_imports_init() {\
    module_t *module = module_find_by_id(id);\
    if (module == NULL) {while(1);}\
    static const uint16_t __memx fn_ids[(exports(modname, COUNT) +0)] = {exports(modname, MODULE_ENUM_FN_ID)};\
    for (uint8_t i = 0; i < (exports(modname, COUNT)); i++) {\
      modname##_fns.fptr_arr[i] = module_fn_lookup(fn_ids[i], module);\
      if (modname##_fns.fptr_arr[i] == NULL) {while(0);}\
    }\
  }

// #define MODULE_TRAMPOLINE_FN(modname, name, returns, ...) \
//     returns modname##_##name(__VA_ARGS__) {\
//         return modname##_fns.name(__VA_ARGS__);\
//     }

// #define MODULE_TRAMPOLINE_EXPORT(modname, name, returns, ...) \
//     extern returns modname##_##name(__VA_ARGS__);

// #define MODULE_DECLARE_TRAMPOLINES(modname, exports) \
//     exports(modname, MODULE_TRAMPOLINE_EXPORT)

// #define MODULE_DEFINE_TRAMPOLINES(modname, exports) \
//     exports(modname, MODULE_TRAMPOLINE_FN)

#define MODULE_CALL(modname, name, funcs, ...) \
    ((modname##_fns_t*)funcs)->name(__VA_ARGS__)