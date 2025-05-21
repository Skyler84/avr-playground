
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

#define GET_MODULE_FN_PTR_OFFSET() ({\
    uintptr_t rel; \
    asm volatile(  \
    ASM_PUSH_PC_LABEL_ADDR(L_%=)   \
    ASM_POP_RETURN_ADDR(0)        \
    "      subi %A0, pm_lo8(L_%=)\n" \
    "      sbci %B0, pm_hi8(L_%=)\n" \
    : "=d" (rel) \
    ::); \
    rel; \
})


inline static uint32_t GET_MODULE_DATA_PTR_OFFSET() 
{
    return ((uint32_t)GET_MODULE_FN_PTR_OFFSET()) * 2;
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

#define indirect_call(name) (&name + GET_MODULE_FN_PTR_OFFSET())


extern int __ashlsi3 (int a, int b);
extern uint64_t __ashldi3 (uint64_t a, int b);
extern long long __ashlti3 (long long a, int b);

// These functions return the result of shifting a left by b bits. 

extern int __ashrsi3 (int a, int b);
extern long __ashrdi3 (long a, int b);
extern long long __ashrti3 (long long a, int b);

// These functions return the result of arithmetically shifting a right by b bits. 

extern int __divsi3 (int a, int b);
extern long __divdi3 (long a, long b);
extern long long __divti3 (long long a, long long b);

// These functions return the quotient of the signed division of a and b. 

extern int __lshrsi3 (int a, int b);
extern uint64_t __lshrdi3 (uint64_t a, int b);
extern long long __lshrti3 (long long a, int b);

// These functions return the result of logically shifting a right by b bits. 

extern int __modsi3 (int a, int b);
extern long __moddi3 (long a, long b);
extern long long __modti3 (long long a, long long b);

// These functions return the remainder of the signed division of a and b. 

extern int __mulsi3 (int a, int b);
extern long __muldi3 (long a, long b);
extern long long __multi3 (long long a, long long b);
extern uint32_t __umulhisi3 (uint16_t a, uint16_t b);

// These functions return the product of a and b. 

extern long __negdi2 (long a);
extern long long __negti2 (long long a);

// These functions return the negation of a. 

extern unsigned int __udivsi3 (unsigned int a, unsigned int b);
extern unsigned long __udivdi3 (unsigned long a, unsigned long b);
// extern unsigned long long __udivmodsi4 (unsigned long long a, unsigned long long b);

// These functions return the quotient of the unsigned division of a and b. 

extern unsigned long __udivmoddi3 (unsigned long a, unsigned long b, unsigned long *c);
// extern unsigned long __udivmoddi3 (unsigned long a, unsigned long b, unsigned long *c);
// extern unsigned long long __udivti3 (unsigned long long a, unsigned long long b, unsigned long long *c);
extern unsigned long __udivmodsi4(unsigned long num, unsigned long den, int modwanted);

// These functions calculate both the quotient and remainder of the unsigned division of a and b. The return value is the quotient, and the remainder is placed in variable pointed to by c. 

extern unsigned int __umodsi3 (unsigned int a, unsigned int b);
extern unsigned long __umoddi3 (unsigned long a, unsigned long b);
extern unsigned long long __umodti3 (unsigned long long a, unsigned long long b);

// These functions return the remainder of the unsigned division of a a