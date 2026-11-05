// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

typedef unsigned long long malloc_type_id_t; // NOLINT(malterlib-type-naming): the name in the macOS SDK

#define _MALLOC_UNDERSCORE_MALLOC_TYPE_H_ // NOLINT(malterlib-macro-naming)

#if !defined(_MALLOC_TYPED)
#	define _MALLOC_TYPED(d_Override, d_TypeParamPos) // NOLINT(malterlib-macro-naming)
#endif

extern "C"
{
	struct _malloc_zone_t;
	typedef struct _malloc_zone_t malloc_zone_t; // NOLINT(malterlib-type-naming): the name in the macOS SDK
}

#include <malloc/malloc.h>

#include <mach/message.h>
#include <mach/semaphore.h>
#include <sys/event.h>
#include <poll.h>
#include <time.h>
#if DMibConfig_Thread_DebugThreadLocals
#	include <pthread.h>
#endif

extern "C" void *_Znam(size_t _Size) __attribute__((weak_import));
extern "C" void *_ZnamRKSt9nothrow_t(size_t _Size) __attribute__((weak_import));
extern "C" void *_ZnamSt11align_val_t(size_t _Size, size_t _Alignment) __attribute__((weak_import));
extern "C" void *_ZnamSt11align_val_tRKSt9nothrow_t(size_t _Size, size_t _Alignment) __attribute__((weak_import));
extern "C" void *_Znwm(size_t _Size) __attribute__((weak_import));
extern "C" void *_ZnwmRKSt9nothrow_t(size_t _Size) __attribute__((weak_import));
extern "C" void *_ZnwmSt11align_val_t(size_t _Size, size_t _Alignment) __attribute__((weak_import));
extern "C" void *_ZnwmSt11align_val_tRKSt9nothrow_t(size_t _Size, size_t _Alignment) __attribute__((weak_import));
extern "C" void _ZdaPv(void *_pMemory) __attribute__((weak_import));
extern "C" void _ZdaPvRKSt9nothrow_t(void *_pMemory) __attribute__((weak_import));
extern "C" void _ZdaPvSt11align_val_t(void *_pMemory, size_t _Alignment) __attribute__((weak_import));
extern "C" void _ZdaPvSt11align_val_tRKSt9nothrow_t(void *_pMemory, size_t _Alignment) __attribute__((weak_import));
extern "C" void _ZdaPvm(void *_pMemory, size_t _Size) __attribute__((weak_import));
extern "C" void _ZdaPvmSt11align_val_t(void *_pMemory, size_t _Size, size_t _Alignment) __attribute__((weak_import));
extern "C" void _ZdlPv(void *_pMemory) __attribute__((weak_import));
extern "C" void _ZdlPvRKSt9nothrow_t(void *_pMemory) __attribute__((weak_import));
extern "C" void _ZdlPvSt11align_val_t(void *_pMemory, size_t _Alignment) __attribute__((weak_import));
extern "C" void _ZdlPvSt11align_val_tRKSt9nothrow_t(void *_pMemory, size_t _Alignment) __attribute__((weak_import));
extern "C" void _ZdlPvm(void *_pMemory, size_t _Size) __attribute__((weak_import));
extern "C" void _ZdlPvmSt11align_val_t(void *_pMemory, size_t _Size, size_t _Alignment) __attribute__((weak_import));

extern "C" void vfree(void *_pMemory) __attribute__((weak_import));

using malloc_options_np_t = uint64_t; // NOLINT(malterlib-type-naming): the name in the macOS SDK
#if __MAC_OS_X_VERSION_MAX_ALLOWED < 260000
using malloc_zone_malloc_options_t = uint64_t;

extern "C" void *malloc_zone_malloc_with_options(malloc_zone_t *_pZone, size_t _Alignment, size_t _Size, malloc_zone_malloc_options_t _Options) __attribute__((weak_import));
extern "C" void *malloc_type_zone_malloc_with_options
	(
		malloc_zone_t *_pZone
		, size_t _Alignment
		, size_t _Size
		, malloc_type_id_t _TypeID
		, malloc_zone_malloc_options_t _Options
	)
	__attribute__((weak_import))
;
#endif

extern "C" boolean_t malloc_zone_claimed_address(malloc_zone_t *_pZone, void *_pMemory) __attribute__((weak_import));
extern "C" void *malloc_zone_malloc_with_options_np(malloc_zone_t *_pZone, size_t _Alignment, size_t _Size, malloc_options_np_t _Options) __attribute__((weak_import));
extern "C" void *malloc_type_zone_malloc_with_options_np
	(
		malloc_zone_t *_pZone
		, size_t _Alignment
		, size_t _Size
		, malloc_options_np_t _Options
		, malloc_type_id_t _TypeID
	)
	__attribute__((weak_import))
;
extern "C" void *malloc_type_zone_malloc_with_options_internal
	(
		malloc_zone_t *_pZone
		, size_t _Alignment
		, size_t _Size
		, malloc_options_np_t _Options
		, malloc_type_id_t _TypeID
	)
	__attribute__((weak_import))
;

extern "C" int __workq_kernreturn(int _Options, user_addr_t _Item, int _Affinity, int _Priority);
extern "C" uint32_t __psynch_cvwait
	(
		user_addr_t _Condition
		, uint64_t _ConditionLSGeneration
		, uint32_t _ConditionUGeneration
		, user_addr_t _Mutex
		, uint64_t _MutexGeneration
		, uint32_t _Flags
		, int64_t _Seconds
		, uint32_t _Nanoseconds
	)
;
extern "C" int __ulock_wait(uint32_t _Operation, void *_pAddress, uint64_t _Value, uint32_t _TimeoutMicroseconds);
extern "C" int __ulock_wait2(uint32_t _Operation, void *_pAddress, uint64_t _Value, uint64_t _TimeoutNanoseconds, uint64_t _Value2) __attribute__((weak_import));
extern "C" int os_sync_wait_on_address(void *_pAddress, uint64_t _Value, size_t _Size, uint32_t _Flags) __attribute__((weak_import));
extern "C" int os_sync_wait_on_address_with_timeout
	(
		void *_pAddress
		, uint64_t _Value
		, size_t _Size
		, uint32_t _Flags
		, uint32_t _ClockID
		, uint64_t _TimeoutNanoseconds
	)
	__attribute__((weak_import))
;
extern "C" void _malloc_fork_prepare() __attribute__((weak_import));
extern "C" void _malloc_fork_child() __attribute__((weak_import));
extern "C" void _malloc_fork_parent() __attribute__((weak_import));
extern "C" void __exit(int _ExitCode) __attribute__((weak_import));
extern "C" void _exit(int _ExitCode) __attribute__((weak_import));

extern "C" void *malloc_type_malloc(size_t _Size, malloc_type_id_t _TypeID) __attribute__((weak_import));
extern "C" void *malloc_type_calloc(size_t _nItems, size_t _Size, malloc_type_id_t _TypeID) __attribute__((weak_import));
extern "C" void malloc_type_free(void *_pMemory, malloc_type_id_t _TypeID) __attribute__((weak_import));
extern "C" void *malloc_type_realloc(void *_pMemory, size_t _Size, malloc_type_id_t _TypeID) __attribute__((weak_import));
extern "C" void *malloc_type_valloc(size_t _Size, malloc_type_id_t _TypeID) __attribute__((weak_import));
extern "C" void *malloc_type_aligned_alloc(size_t _Alignment, size_t _Size, malloc_type_id_t _TypeID) __attribute__((weak_import));
extern "C" int malloc_type_posix_memalign(void **o_pMemory, size_t _Alignment, size_t _Size, malloc_type_id_t _TypeID) __attribute__((weak_import));

extern "C" void *malloc_type_zone_malloc(malloc_zone_t *_pZone, size_t _Size, malloc_type_id_t _TypeID) __attribute__((weak_import));
extern "C" void *malloc_type_zone_calloc(malloc_zone_t *_pZone, size_t _nItems, size_t _Size, malloc_type_id_t _TypeID) __attribute__((weak_import));
extern "C" void malloc_type_zone_free(malloc_zone_t *_pZone, void *_pMemory, malloc_type_id_t _TypeID) __attribute__((weak_import));
extern "C" void *malloc_type_zone_realloc(malloc_zone_t *_pZone, void *_pMemory, size_t _Size, malloc_type_id_t _TypeID) __attribute__((weak_import));
extern "C" void *malloc_type_zone_valloc(malloc_zone_t *_pZone, size_t _Size, malloc_type_id_t _TypeID) __attribute__((weak_import));
extern "C" void *malloc_type_zone_memalign(malloc_zone_t *_pZone, size_t _Alignment, size_t _Size, malloc_type_id_t _TypeID) __attribute__((weak_import));

#define DMibMemoryInterpose(d_Return, d_Function, d_Args, ...) d_Return (*d_Function)(__VA_ARGS__);

struct COriginalFunctions
{
#include "Malterlib_Memory_SystemOverride_MacOSInterposeFunctions.h"
};
