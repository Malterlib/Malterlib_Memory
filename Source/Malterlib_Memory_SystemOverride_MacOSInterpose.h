// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

typedef unsigned long long malloc_type_id_t;

#define _MALLOC_UNDERSCORE_MALLOC_TYPE_H_

#if !defined(_MALLOC_TYPED)
#define _MALLOC_TYPED(override, type_param_pos)
#endif

extern "C"
{
	struct _malloc_zone_t;
	typedef struct _malloc_zone_t malloc_zone_t;
}

#include <malloc/malloc.h>

#include <mach/message.h>
#include <mach/semaphore.h>
#include <sys/event.h>
#if DMibConfig_Thread_DebugThreadLocals
#include <pthread.h>
#endif

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

using malloc_options_np_t = uint64_t;
#if __MAC_OS_X_VERSION_MAX_ALLOWED < 260000
	using malloc_zone_malloc_options_t = uint64_t;

	extern "C" void * malloc_zone_malloc_with_options(malloc_zone_t *zone, size_t align, size_t size, malloc_zone_malloc_options_t opts) __attribute__((weak_import));
	extern "C" void * malloc_type_zone_malloc_with_options(malloc_zone_t *zone, size_t alignment, size_t size, malloc_type_id_t type_id, malloc_zone_malloc_options_t opts) __attribute__((weak_import));
#endif

extern "C" boolean_t malloc_zone_claimed_address(malloc_zone_t *_0, void *_1) __attribute__((weak_import));
extern "C" void *malloc_zone_malloc_with_options_np(malloc_zone_t *_0, size_t _1, size_t _2, malloc_options_np_t _3) __attribute__((weak_import));
extern "C" void *malloc_type_zone_malloc_with_options_np(malloc_zone_t *zone, size_t align, size_t size, malloc_options_np_t options, malloc_type_id_t type_id) __attribute__((weak_import));
extern "C" void *malloc_type_zone_malloc_with_options_internal(malloc_zone_t *zone, size_t align, size_t size, malloc_options_np_t options, malloc_type_id_t type_id) __attribute__((weak_import));

extern "C" int __workq_kernreturn(int options, user_addr_t item, int affinity, int prio);
extern "C" uint32_t __psynch_cvwait(user_addr_t cv, uint64_t cvlsgen, uint32_t cvugen, user_addr_t mutex, uint64_t mugen, uint32_t flags, int64_t sec, uint32_t nsec);
extern "C" void _malloc_fork_prepare() __attribute__((weak_import));
extern "C" void _malloc_fork_child() __attribute__((weak_import));
extern "C" void _malloc_fork_parent() __attribute__((weak_import));
extern "C" void __exit(int _0) __attribute__((weak_import));
extern "C" void _exit(int _0) __attribute__((weak_import));

extern "C" void *malloc_type_malloc(size_t size, malloc_type_id_t type_id) __attribute__((weak_import));
extern "C" void *malloc_type_calloc(size_t count, size_t size, malloc_type_id_t type_id) __attribute__((weak_import));
extern "C" void malloc_type_free(void * ptr, malloc_type_id_t type_id) __attribute__((weak_import));
extern "C" void *malloc_type_realloc(void * ptr, size_t size, malloc_type_id_t type_id) __attribute__((weak_import));
extern "C" void *malloc_type_valloc(size_t size, malloc_type_id_t type_id) __attribute__((weak_import));
extern "C" void *malloc_type_aligned_alloc(size_t alignment, size_t size, malloc_type_id_t type_id) __attribute__((weak_import));
extern "C" int malloc_type_posix_memalign(void * *memptr, size_t alignment, size_t size, malloc_type_id_t type_id) __attribute__((weak_import));

extern "C" void *malloc_type_zone_malloc(malloc_zone_t *zone, size_t size, malloc_type_id_t type_id) __attribute__((weak_import));
extern "C" void *malloc_type_zone_calloc(malloc_zone_t *zone, size_t count, size_t size, malloc_type_id_t type_id) __attribute__((weak_import));
extern "C" void malloc_type_zone_free(malloc_zone_t *zone, void * ptr, malloc_type_id_t type_id) __attribute__((weak_import));
extern "C" void *malloc_type_zone_realloc(malloc_zone_t *zone, void * ptr, size_t size, malloc_type_id_t type_id) __attribute__((weak_import));
extern "C" void *malloc_type_zone_valloc(malloc_zone_t *zone, size_t size, malloc_type_id_t type_id) __attribute__((weak_import));
extern "C" void *malloc_type_zone_memalign(malloc_zone_t *zone, size_t alignment, size_t size, malloc_type_id_t type_id) __attribute__((weak_import));

#define DMibMemoryInterpose(d_Return, d_Function, d_Args, ...) d_Return (*d_Function)(__VA_ARGS__);

struct COriginalFunctions
{
#include "Malterlib_Memory_SystemOverride_MacOSInterposeFunctions.h"
};
