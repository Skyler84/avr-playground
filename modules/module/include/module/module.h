#pragma once

#include <stdint.h>
#include <stdlib.h>

#define CAT(a, b) a ## b
#define XCAT(a, b) CAT(a, b)

#define AVR_MODULE_MAGIC 0x4D4F447F // 'MOD\x7F' in ASCII

typedef uint16_t module_id_t;
typedef uint16_t module_fn_id_t;
typedef void (*fn_ptr_t)();

#define pgm_ptr_to_fn_ptr(p) ((((uintptr_t)p)/2))

typedef struct {
  module_fn_id_t id;
  fn_ptr_t fn;
}module_fn_t;

typedef const __flash struct{
  uint32_t magic;
  // uint16_t size;
  uint16_t id;
  // const __flash char* name;
  module_fn_t fn_table[];
} module_t;

// ------------------------------------

#define MODULE_ENUM_FN_ID(modname, name, ...)\
modname##_##name##_fn_id,

// Declare the module's function_id_t enum
#define MODULE_DECLARE_FN_IDS(modname, exports)\
typedef enum modname##_function_id_t{\
  modname##_function_none = 0,\
  exports(modname, MODULE_ENUM_FN_ID)\
  modname##_function_id_count,\
  modname##_function_api_ver = -1\
} modname##_function_id_t;\

// -----------------------------------

#define MODULE_ENUM_FN_TYPE(modname, name, returns, ...)\
  typedef returns (*modname##_##name##_fn_t)(__VA_ARGS__);

// Declare function pointer typedefs
#define MODULE_DECLARE_FN_TYPES(modname, exports)\
  exports(modname, MODULE_ENUM_FN_TYPE)

#define MODULE_DEFINE_FN(modname, name, returns, ...)\
  returns modname##_##name(__VA_ARGS__)

// --------------------------------

#define DEFINE_IMPORTED_FN(modname, name, ...)\
  modname##_##name##_fn_t name;

#define DEFINE_IMPORTED_FNS(modname, exports)\
  modname##_fns_t modname##_fns

#define COUNT(...) +1

#define MODULE_DECLARE_FNS(modname, exports)\
typedef struct modname##_fns_t{\
  union{\
    struct{\
      exports(modname, DEFINE_IMPORTED_FN)\
    };\
    fn_ptr_t fptr_arr[(exports(modname, COUNT))];\
  };\
} modname##_fns_t\

#ifndef MODULE_AS_STATIC_LIB
// Declare all enums/structs/typedefs associated with this module
#define DECLARE_MODULE(modname, id, exports) \
  MODULE_DECLARE_FN_IDS(modname, exports)\
  MODULE_DECLARE_FN_TYPES(modname, exports)\
  MODULE_DECLARE_FNS(modname, exports)

#else

#define MODULE_DECLARE_EXTERN_FN(modname, name, returns, ...)\
  extern returns modname##_##name(__VA_ARGS__);

#define MODULE_DECLARE_EXTERN(modname, exports) \
  exports(modname, MODULE_DECLARE_EXTERN_FN)

#define DECLARE_MODULE(modname, id, exports) \
  MODULE_DECLARE_EXTERN(modname, exports) \
  MODULE_DECLARE_FN_IDS(modname, exports)\
  MODULE_DECLARE_FN_TYPES(modname, exports)\
  MODULE_DECLARE_FNS(modname, exports)
  // #error aoeu

#endif

#include "module/imports.h"

// --------------------------------

#define REGISTER_MODULE_FN(modname, name, ...) \
  { modname##_##name##_fn_id, (fn_ptr_t)&modname##_##name },

#define REGISTER_MODULE(modname, id, exports, api_ver)\
  static const module_fn_t module_fn_table[] __attribute__((used, section(".module.table"))) = {\
    exports(modname, REGISTER_MODULE_FN)\
    {0xffff, (void(*)())api_ver},\
    {0x0000, NULL}\
  }; \
  static const module_id_t module_id __attribute__((used, section(".module.id"))) = id\


#define MODULE_FN_EXPORTS(modname, o) \
  o(module, fn_lookup, fn_ptr_t, module_fn_id_t, module_t*)\
  o(module, next, module_t*, module_t*)\
  o(module, find_by_id, module_t*, module_id_t)\


#define DECLARE_FN_PROTO(modname, name, returns, ...) \
  returns modname##_##name(__VA_ARGS__);

#define MODULE_FN_PROTOS(modname, exports) \
  exports(modname, DECLARE_FN_PROTO)


#define MODULE_FUNCTION_EXPORTS(modname, o) \
  o(modname, fn_lookup, fn_ptr_t, module_fn_id_t, module_t*)\
  o(modname, next, module_t*, module_t*)\
  o(modname, find_by_id, module_t*, module_id_t)

#define MOD_CALL(modname, this, fname) \
  (((modname##_fns_t*)this)->fname)

#define MODULE_API_VER 0x0001
#define MODULE_MODULE_ID 0x0000

DECLARE_MODULE(module, MODULE_MODULE_ID, MODULE_FUNCTION_EXPORTS);