#pragma once

#include <stdint.h>

#ifndef MODULE_AS_STATIC_LIB

#define ASM_POP_RETURN_ADDR(id) \
"    pop %B" #id "\n" \
"    pop %A" #id "\n" \

#define ASM_PUSH_PC_LABEL_ADDR(label) \
"      rcall " #label "  \n" /*push PC onto stack*/ \
#label ": \n"                                       \

#define MODULE_ADJUST_DATA_PTR(ptr) \
    (void*)((uintptr_t)(ptr) + GET_MODULE_DATA_PTR_OFFSET())
#define MODULE_ADJUST_FN_PTR(ptr) \
    (void*)((uintptr_t)(ptr) + GET_MODULE_FN_PTR_OFFSET())


inline static uintptr_t GET_MODULE_FN_PTR_OFFSET() 
{
    #if 0
    uintptr_t rel;
    asm volatile( 
    ASM_PUSH_PC_LABEL_ADDR(test_label)  
    ASM_POP_RETURN_ADDR(0)       
    // "      subi %A0, pm_lo8(test_label)\n"
    // "      sbci %B0, pm_hi8(test_label)\n"
    : "=r" (rel)
    ::);
    extern void test_label();
    return rel - (uintptr_t)&test_label;
    #else
    uintptr_t rel;
    asm volatile( 
    ASM_PUSH_PC_LABEL_ADDR(L_%=)  
    ASM_POP_RETURN_ADDR(0)       
    "      subi %A0, pm_lo8(L_%=)\n"
    "      sbci %B0, pm_hi8(L_%=)\n"
    : "=r" (rel)
    ::);
    return rel;
    #endif
}

inline static uintptr_t GET_MODULE_DATA_PTR_OFFSET() 
{
    return GET_MODULE_FN_PTR_OFFSET() * 2;
}

#else

inline static uintptr_t GET_MODULE_FN_PTR_OFFSET() 
{
    return 0;
}
inline static uintptr_t GET_MODULE_DATA_PTR_OFFSET() 
{
    return 0;
}

#endif

