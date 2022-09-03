// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <mach/message.h>
#include <mach/semaphore.h>
#include <sys/event.h>

extern "C" void *_Znam(size_t) __attribute__((weak_import));
extern "C" void *_ZnamRKSt9nothrow_t(size_t) __attribute__((weak_import));
extern "C" void *_ZnamSt11align_val_t(size_t, size_t) __attribute__((weak_import));
extern "C" void *_ZnamSt11align_val_tRKSt9nothrow_t(size_t, size_t) __attribute__((weak_import));
extern "C" void *_Znwm(size_t) __attribute__((weak_import));
extern "C" void *_ZnwmRKSt9nothrow_t(size_t) __attribute__((weak_import));
extern "C" void *_ZnwmSt11align_val_t(size_t, size_t) __attribute__((weak_import));
extern "C" void *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t, size_t) __attribute__((weak_import));
extern "C" void _ZdaPv(void *) __attribute__((weak_import));
extern "C" void _ZdaPvRKSt9nothrow_t(void *) __attribute__((weak_import));
extern "C" void _ZdaPvSt11align_val_t(void *, size_t) __attribute__((weak_import));
extern "C" void _ZdaPvSt11align_val_tRKSt9nothrow_t(void *, size_t) __attribute__((weak_import));
extern "C" void _ZdaPvm(void *, size_t) __attribute__((weak_import));
extern "C" void _ZdaPvmSt11align_val_t(void *, size_t, size_t) __attribute__((weak_import));
extern "C" void _ZdlPv(void *) __attribute__((weak_import));
extern "C" void _ZdlPvRKSt9nothrow_t(void *) __attribute__((weak_import));
extern "C" void _ZdlPvSt11align_val_t(void *, size_t) __attribute__((weak_import));
extern "C" void _ZdlPvSt11align_val_tRKSt9nothrow_t(void *, size_t) __attribute__((weak_import));
extern "C" void _ZdlPvm(void *, size_t) __attribute__((weak_import));
extern "C" void _ZdlPvmSt11align_val_t(void *, size_t, size_t) __attribute__((weak_import));

extern "C" void vfree(void *) __attribute__((weak_import));

extern "C" int __workq_kernreturn(int options, user_addr_t item, int affinity, int prio);
extern "C" uint32_t __psynch_cvwait(user_addr_t cv, uint64_t cvlsgen, uint32_t cvugen, user_addr_t mutex, uint64_t mugen, uint32_t flags, int64_t sec, uint32_t nsec);
extern "C" void _malloc_fork_prepare() __attribute__((weak_import));
extern "C" void _malloc_fork_child() __attribute__((weak_import));
extern "C" void _malloc_fork_parent() __attribute__((weak_import));
extern "C" void __exit(int _0) __attribute__((weak_import));
extern "C" void _exit(int _0) __attribute__((weak_import));

#define DMibMemoryInterpose(d_Return, d_Function, d_Args, ...) d_Return (*d_Function)(__VA_ARGS__);

struct COriginalFunctions
{
#include "Malterlib_Memory_SystemOverride_MacOSInterposeFunctions.h"
};
