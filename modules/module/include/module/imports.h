#pragma once

#include "module/module.h"

#define IMPORT_FN(modname, name, ...)\
  modname##_fns.name = (modname##_##name##_fn_t)module_fn_lookup(modname##_##name##_fn_id, module);\
  if (modname##_fns.name == NULL) {while(1);}

#define DEFINE_IMPORTED_FN_STATIC(modname, name, ...)\
  .modname##_##name = modname##_##name,

#define DEFINE_IMPORTED_FNS_STATIC(modname, exports)\
  modname##_fns_t modname##_fns = {\
    exports(modname, DEFINE_IMPORTED_FN_STATIC)\
  };

#define MODULE_IMPORT_STATIC(modname, id, exports) \
  DEFINE_IMPORTED_FNS_STATIC(modname, exports);

#define MODULE_IMPORT_MODULE(modname, id, exports) \
  DEFINE_IMPORTED_FNS(modname, exports);\
  static void __attribute__((constructor)) modname##_module_imports_init() \
  MODULE_IMPORT_FUNCTIONS_RUNTIME_MODULE(modname, id, exports, &modname##_fns)

#define MODULE_IMPORT_FUNCTIONS(modname, id, exports)\
  XCAT(MODULE_IMPORT_, XCAT(modname,_MODTYPE))(modname, id, exports)

#define MODULE_IMPORT_FUNCTION_STATIC(modname, name, returns, ...) \


#define MODULE_IMPORT_FUNCTIONS_RUNTIME_STATIC(modname, id, exports, fns) \
{\
  *(fns) = (modname##_fns_t){\
    exports(modname, DEFINE_IMPORTED_FN_STATIC)\
  };\
}
#define MODULE_IMPORT_FUNCTIONS_RUNTIME_MODULE(modname, id, exports, fns) \
{\
  uint16_t modname##_fn_ids[] = {\
    exports(modname, MODULE_ENUM_FN_ID)\
    0\
  };\
  module_init_fns((fns)->fptr_arr, id, modname##_fn_ids);\
}

#define MODULE_IMPORT_FUNCTIONS_RUNTIME(modname, id, exports, fns)\
  XCAT(MODULE_IMPORT_FUNCTIONS_RUNTIME_, XCAT(modname,_MODTYPE))(modname, id, exports, fns)

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

#define MODULE_CALL_INTERFACE MODULE_CALL_MODULE

#define MODULE_CALL_MODULE(modname, name, this, ...) \
    ((modname##_fns_t*)(this)->fns)->modname##_##name((this)__VA_OPT__(,) __VA_ARGS__)

    
#define MODULE_CALL_FNS_STATIC(modname, name, fns, ...) \
modname##_##name(__VA_ARGS__)

#define MODULE_CALL_FNS_INTERFACE MODULE_CALL_FNS_MODULE

#define MODULE_CALL_FNS_MODULE(modname, name, fns, ...) \
((modname##_fns_t*)(fns))->modname##_##name(__VA_ARGS__)

#define MODULE_CALL_THIS(modname, name, this, ...) \
  XCAT(MODULE_CALL_, XCAT(modname,_MODTYPE))(modname, name, this,__VA_ARGS__)

#define MODULE_CALL_FNS(modname, name, fns, ...) \
  XCAT(MODULE_CALL_FNS_, XCAT(modname,_MODTYPE))(modname, name, fns,__VA_ARGS__)
