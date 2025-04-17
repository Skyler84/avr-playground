#pragma once

#include <stdint.h>

#define DECLARE_FN_TYPE(name, returns, ...) \
typedef returns (*name##_fn_t)(__VA_ARGS__);
#define DECLARE_FN(name, returns, ...) \
static returns name(__VA_ARGS__);
#define ENUM_FN_ID(name, ...) \
name##_fn_id,
#define ENUM_FN_ENTRY(name, ...) \
{name##_fn_id, name},


typedef struct{
    uint16_t id;
    void(*fn)();
} module_fn;

#define REGISTER_MODULE(name, fns, ver)\
const module_fn module_fns[] __attribute__((section(".module_begin"), used)) = {\
    fns(ENUM_FN_ENTRY)\
    {0xffff, (void*)ver},\
    {0,0}\
}

typedef void (*fn_ptr)();

extern fn_ptr module_resolve_fn(uint16_t id, const module_fn *fns);
