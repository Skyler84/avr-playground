#pragma once

#define IMPORT_FN(modname, name, ...)\
  modname##_fns.name = (modname##_##name##_fn_t)module_fn_lookup(modname##_##name##_fn_id, module);\
  if (modname##_fns.name == NULL) {while(1);}

#define MODULE_IMPORT_FUNCTIONS(modname, id, exports)\
  void module_runtime_init_error_handler(module_id_t, moduleptr_t*, uint16_t, fn_ptr_t *) __attribute__((weak));\
  DEFINE_IMPORTED_FNS(modname, exports);\
  static void __attribute__((constructor)) modname##_module_imports_init() {\
    moduleptr_t module = module_find_by_id(id);\
    if (module == (moduleptr_t)0) {module_runtime_init_error_handler(id, &module, 0, NULL);}\
    static const uint16_t __memx fn_ids[(exports(modname, COUNT) +0)] = {exports(modname, MODULE_ENUM_FN_ID)};\
    for (uint8_t i = 0; i < (exports(modname, COUNT)); i++) {\
      fn_ptr_t fptr = module_fn_lookup(fn_ids[i], module);\
      if (fptr == NULL) {module_runtime_init_error_handler(id, &module, fn_ids[i], &fptr);}\
      modname##_fns.fptr_arr[i] = fptr; \
    }\
  }
/*
 #define MODULE_TRAMPOLINE_FN(modname, name, returns, ...) \
     returns modname##_##name(__VA_ARGS__) {\
         return modname##_fns.name(__VA_ARGS__);\
     }

 #define MODULE_TRAMPOLINE_EXPORT(modname, name, returns, ...) \
     extern returns modname##_##name(__VA_ARGS__);

 #define MODULE_DECLARE_TRAMPOLINES(modname, exports) \
     exports(modname, MODULE_TRAMPOLINE_EXPORT)

 #define MODULE_DEFINE_TRAMPOLINES(modname, exports) \
     exports(modname, MODULE_TRAMPOLINE_FN)
*/

#define MODULE_CALL_STATIC(modname, name, this, ...) \
    modname##_##name((this)__VA_OPT__(,) __VA_ARGS__)

#define MODULE_CALL_INTERFACE MODULE_CALL_STATIC

#define MODULE_CALL_MODULE(modname, name, this, ...) \
    ((modname##_fns_t*)(this)->fns)->name((this)__VA_OPT__(,) __VA_ARGS__)

#define MODULE_CALL(modname, name, this, ...) \
  XCAT(MODULE_CALL_, XCAT(modname,_MODTYPE))(modname, name, this,__VA_ARGS__)
