// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

extern "C" void * _Znam (size_t) __attribute__((weak_import));  // operator new
extern "C" void * _Znwm (size_t)__attribute__((weak_import));  // operator new
extern "C" void * _ZnwmRKSt9nothrow_t (size_t) __attribute__((weak_import)); // nothrow variants
extern "C" void * _ZnamRKSt9nothrow_t (size_t) __attribute__((weak_import)); // operator new nothrow
	
extern "C" void _ZdaPv (void *) __attribute__((weak_import)); // operator delete
extern "C" void _ZdlPv (void *) __attribute__((weak_import)); // operator delete 
extern "C" void _ZdaPvRKSt9nothrow_t (void *) __attribute__((weak_import)); // operator delete nothrow
extern "C" void _ZdlPvRKSt9nothrow_t (void *) __attribute__((weak_import)); // operator delete nothrow

extern "C" void vfree(void *) __attribute__((weak_import));

#undef DMibMemoryInterpose0
#undef DMibMemoryInterpose1
#undef DMibMemoryInterpose2
#undef DMibMemoryInterpose3
#undef DMibMemoryInterpose4
#define DMibMemoryInterpose0(d_Return, d_Function, ...) d_Return (*d_Function)(__VA_ARGS__);
#define DMibMemoryInterpose1(d_Return, d_Function, ...) d_Return (*d_Function)(__VA_ARGS__);
#define DMibMemoryInterpose2(d_Return, d_Function, ...) d_Return (*d_Function)(__VA_ARGS__);
#define DMibMemoryInterpose3(d_Return, d_Function, ...) d_Return (*d_Function)(__VA_ARGS__);
#define DMibMemoryInterpose4(d_Return, d_Function, ...) d_Return (*d_Function)(__VA_ARGS__);

struct COriginalFunctions
{
#include "Malterlib_Memory_SystemOverride_OSXInterposeFunctions.h"
};
