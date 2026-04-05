// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifdef DMibMemoryInterpose

#ifndef DMibMemoryInterpose0
#	define DMibMemoryInterpose0(d_Return, d_Function, ...) DMibMemoryInterpose(d_Return, d_Function, (), __VA_ARGS__)
#endif
#ifndef DMibMemoryInterpose1
#	define DMibMemoryInterpose1(d_Return, d_Function, ...) DMibMemoryInterpose(d_Return, d_Function, (_0), __VA_ARGS__)
#endif
#ifndef DMibMemoryInterposeCpp1
#	define DMibMemoryInterposeCpp1 DMibMemoryInterpose1
#endif
#ifndef DMibMemoryInterposeCpp2
#	define DMibMemoryInterposeCpp2 DMibMemoryInterpose2
#endif
#ifndef DMibMemoryInterposeCpp3
#	define DMibMemoryInterposeCpp3 DMibMemoryInterpose3
#endif
#ifndef DMibMemoryInterpose2
#	define DMibMemoryInterpose2(d_Return, d_Function, ...) DMibMemoryInterpose(d_Return, d_Function, (_0, _1), __VA_ARGS__)
#endif
#ifndef DMibMemoryInterpose3
#	define DMibMemoryInterpose3(d_Return, d_Function, ...) DMibMemoryInterpose(d_Return, d_Function, (_0, _1, _2), __VA_ARGS__)
#endif
#ifndef DMibMemoryInterpose4
#	define DMibMemoryInterpose4(d_Return, d_Function, ...) DMibMemoryInterpose(d_Return, d_Function, (_0, _1, _2, _3), __VA_ARGS__)
#endif
#ifndef DMibMemoryInterpose5
#	define DMibMemoryInterpose5(d_Return, d_Function, ...) DMibMemoryInterpose(d_Return, d_Function, (_0, _1, _2, _3, _4), __VA_ARGS__)
#endif
#ifndef DMibMemoryInterpose6
#	define DMibMemoryInterpose6(d_Return, d_Function, ...) DMibMemoryInterpose(d_Return, d_Function, (_0, _1, _2, _3, _4, _5), __VA_ARGS__)
#endif
#ifndef DMibMemoryInterpose7
#	define DMibMemoryInterpose7(d_Return, d_Function, ...) DMibMemoryInterpose(d_Return, d_Function, (_0, _1, _2, _3, _4, _5, _6), __VA_ARGS__)
#endif
#ifndef DMibMemoryInterpose8
#	define DMibMemoryInterpose8(d_Return, d_Function, ...) DMibMemoryInterpose(d_Return, d_Function, (_0, _1, _2, _3, _4, _5, _6, _7), __VA_ARGS__)
#endif
#else

#ifndef DMibMemoryInterpose1
#define DMibMemoryInterpose1 DMibMemoryInterpose0
#endif
#ifndef DMibMemoryInterposeCpp1
#define DMibMemoryInterposeCpp1 DMibMemoryInterpose0
#endif
#ifndef DMibMemoryInterposeCpp2
#define DMibMemoryInterposeCpp2 DMibMemoryInterpose0
#endif
#ifndef DMibMemoryInterposeCpp3
#define DMibMemoryInterposeCpp3 DMibMemoryInterpose0
#endif
#ifndef DMibMemoryInterpose2
#define DMibMemoryInterpose2 DMibMemoryInterpose0
#endif
#ifndef DMibMemoryInterpose3
#define DMibMemoryInterpose3 DMibMemoryInterpose0
#endif
#ifndef DMibMemoryInterpose4
#define DMibMemoryInterpose4 DMibMemoryInterpose0
#endif
#ifndef DMibMemoryInterpose5
#define DMibMemoryInterpose5 DMibMemoryInterpose0
#endif
#ifndef DMibMemoryInterpose6
#define DMibMemoryInterpose6 DMibMemoryInterpose0
#endif
#ifndef DMibMemoryInterpose7
#define DMibMemoryInterpose7 DMibMemoryInterpose0
#endif
#ifndef DMibMemoryInterpose8
#define DMibMemoryInterpose8 DMibMemoryInterpose0
#endif

#endif

#if defined(DMibMemoryInterpose_Hooks)
#define DMibMemoryInterpose_SomeHooks
#endif

#ifndef DMibMemoryInterpose_SomeHooks
DMibMemoryInterpose0(malloc_zone_t *, malloc_default_zone)
DMibMemoryInterpose2(malloc_zone_t *, malloc_create_zone, vm_size_t _0, unsigned _1)
DMibMemoryInterpose1(void, malloc_destroy_zone, malloc_zone_t *_0)
DMibMemoryInterpose2(void *, malloc_zone_malloc, malloc_zone_t *_0, size_t _1)
DMibMemoryInterpose3(void *, malloc_zone_calloc, malloc_zone_t *_0, size_t _1, size_t _2)
DMibMemoryInterpose2(void *, malloc_zone_valloc, malloc_zone_t *_0, size_t _1)
DMibMemoryInterpose2(void, malloc_zone_free, malloc_zone_t *_0, void * _1)
DMibMemoryInterpose3(void *, malloc_zone_realloc, malloc_zone_t *_0, void * _1, size_t _2)
DMibMemoryInterpose1(malloc_zone_t *, malloc_zone_from_ptr, const void *_0)
DMibMemoryInterpose1(size_t, malloc_size, const void *_0)
DMibMemoryInterpose1(size_t, malloc_good_size, size_t _0)
DMibMemoryInterpose3(void *, malloc_zone_memalign, malloc_zone_t *_0, size_t _1, size_t _2)
DMibMemoryInterpose4(unsigned, malloc_zone_batch_malloc, malloc_zone_t *_0, size_t _1, void **_2, unsigned _3)
DMibMemoryInterpose3(void, malloc_zone_batch_free, malloc_zone_t *_0, void **_1, unsigned _2)
DMibMemoryInterpose0(malloc_zone_t *, malloc_default_purgeable_zone)
DMibMemoryInterpose1(void, malloc_make_purgeable, void *_0)
DMibMemoryInterpose1(int, malloc_make_nonpurgeable, void *_0)
DMibMemoryInterpose1(void, malloc_zone_register, malloc_zone_t *_0)
DMibMemoryInterpose1(void, malloc_zone_unregister, malloc_zone_t *_0)
DMibMemoryInterpose2(void, malloc_set_zone_name, malloc_zone_t *_0, const char *_1)
#ifndef DMibMemoryInterpose_Hooks
DMibMemoryInterpose1(const char *, malloc_get_zone_name, malloc_zone_t *_0)
#endif
DMibMemoryInterpose2(size_t, malloc_zone_pressure_relief, malloc_zone_t *_0, size_t _1)
DMibMemoryInterpose4(kern_return_t, malloc_get_all_zones, task_t _0, memory_reader_t _1, vm_address_t **_2, unsigned *_3)
DMibMemoryInterpose1(void, malloc_zone_print_ptr_info, void *_0)
#ifndef DMibMemoryInterpose_Hooks
DMibMemoryInterpose1(boolean_t, malloc_zone_check, malloc_zone_t *_0)
#endif
DMibMemoryInterpose2(void, malloc_zone_print, malloc_zone_t *_0, boolean_t _1)
#ifndef DMibMemoryInterpose_Hooks
DMibMemoryInterpose2(void, malloc_zone_statistics, malloc_zone_t *_0, malloc_statistics_t *_1)
#endif
DMibMemoryInterpose2(void, malloc_zone_log, malloc_zone_t *_0, void *_1)
#ifndef DMibMemoryInterpose_Hooks
DMibMemoryInterpose1(boolean_t, malloc_zone_enable_discharge_checking, malloc_zone_t *_0)
DMibMemoryInterpose1(void, malloc_zone_disable_discharge_checking, malloc_zone_t *_0)
#endif
DMibMemoryInterpose2(void, malloc_zone_discharge, malloc_zone_t *_0, void * _1)
DMibMemoryInterpose2(void, malloc_zone_enumerate_discharged_pointers, malloc_zone_t *_0, void (^_1)(void *memory, void *info))

DMibMemoryInterpose2(boolean_t, malloc_zone_claimed_address, malloc_zone_t *_0, void *_1)
DMibMemoryInterpose4(void *, malloc_zone_malloc_with_options_np, malloc_zone_t *_0, size_t _1, size_t _2, uint64_t _3)
DMibMemoryInterpose4(void *, malloc_zone_malloc_with_options, malloc_zone_t *_0, size_t _1, size_t _2, malloc_zone_malloc_options_t _3)

DMibMemoryInterpose5(void *, malloc_type_zone_malloc_with_options_np, malloc_zone_t *_0, size_t _1, size_t _2, malloc_options_np_t _3, malloc_type_id_t _4)
DMibMemoryInterpose5(void *, malloc_type_zone_malloc_with_options_internal, malloc_zone_t *_0, size_t _1, size_t _2, malloc_options_np_t _3, malloc_type_id_t _4)
DMibMemoryInterpose5(void *, malloc_type_zone_malloc_with_options, malloc_zone_t *_0, size_t _1, size_t _2, malloc_type_id_t _3, malloc_zone_malloc_options_t _4)

DMibMemoryInterpose3(void *, malloc_type_zone_malloc, malloc_zone_t *_0, size_t _1, malloc_type_id_t _2)
DMibMemoryInterpose4(void *, malloc_type_zone_calloc, malloc_zone_t *_0, size_t _1, size_t _2, malloc_type_id_t _3)
DMibMemoryInterpose3(void, malloc_type_zone_free, malloc_zone_t *_0, void *_1, malloc_type_id_t _2)
DMibMemoryInterpose4(void *, malloc_type_zone_realloc, malloc_zone_t *_0, void *_1, size_t _2, malloc_type_id_t _3)
DMibMemoryInterpose3(void *, malloc_type_zone_valloc, malloc_zone_t *_0, size_t _1, malloc_type_id_t _2)
DMibMemoryInterpose4(void *, malloc_type_zone_memalign, malloc_zone_t *_0, size_t _1, size_t _2, malloc_type_id_t _3)

DMibMemoryInterpose1(void *, malloc, size_t _0)
DMibMemoryInterpose1(void *, valloc, size_t _0)
DMibMemoryInterpose2(void *, calloc, size_t _0, size_t _1)
DMibMemoryInterpose2(void *, realloc, void *_0, size_t _1)
DMibMemoryInterpose2(void *, reallocf, void *_0, size_t _1)
DMibMemoryInterpose1(void, free, void *_0)
DMibMemoryInterpose1(void, vfree, void *_0)
DMibMemoryInterpose3(int, posix_memalign, void **_0, size_t _1, size_t _2)
DMibMemoryInterpose2(void *, aligned_alloc, size_t _0, size_t _1)

DMibMemoryInterpose2(void *, malloc_type_malloc, size_t _0, malloc_type_id_t _1)
DMibMemoryInterpose3(void *, malloc_type_calloc, size_t _0, size_t _1, malloc_type_id_t _2)
DMibMemoryInterpose2(void, malloc_type_free, void *_0, malloc_type_id_t _1)
DMibMemoryInterpose3(void *, malloc_type_realloc, void *_0, size_t _1, malloc_type_id_t _2)
DMibMemoryInterpose2(void *, malloc_type_valloc, size_t _0, malloc_type_id_t _1)
DMibMemoryInterpose3(void *, malloc_type_aligned_alloc, size_t _0, size_t _1, malloc_type_id_t _2)
DMibMemoryInterpose4(int, malloc_type_posix_memalign, void **_0, size_t _1, size_t _2, malloc_type_id_t _3)

DMibMemoryInterpose1(int, malloc_jumpstart, int _0)

#ifndef DMibMemoryInterpose_Hooks
DMibMemoryInterposeCpp1(void *, _Znam, size_t _0)
DMibMemoryInterposeCpp1(void *, _ZnamRKSt9nothrow_t, size_t _0)
DMibMemoryInterposeCpp2(void *, _ZnamSt11align_val_t, size_t _0, size_t _1)
DMibMemoryInterposeCpp2(void *, _ZnamSt11align_val_tRKSt9nothrow_t, size_t _0, size_t _1)
DMibMemoryInterposeCpp1(void *, _Znwm, size_t _0)
DMibMemoryInterposeCpp1(void *, _ZnwmRKSt9nothrow_t, size_t _0)
DMibMemoryInterposeCpp2(void *, _ZnwmSt11align_val_t, size_t _0, size_t _1)
DMibMemoryInterposeCpp2(void *, _ZnwmSt11align_val_tRKSt9nothrow_t, size_t _0, size_t _1)
DMibMemoryInterposeCpp1(void , _ZdaPv, void *_0)
DMibMemoryInterposeCpp1(void , _ZdaPvRKSt9nothrow_t, void *_0)
DMibMemoryInterposeCpp2(void , _ZdaPvSt11align_val_t, void *_0, size_t _1)
DMibMemoryInterposeCpp2(void , _ZdaPvSt11align_val_tRKSt9nothrow_t, void *_0, size_t _1)
DMibMemoryInterposeCpp2(void , _ZdaPvm, void *_0, size_t _1)
DMibMemoryInterposeCpp3(void , _ZdaPvmSt11align_val_t, void *_0, size_t _1, size_t _2)
DMibMemoryInterposeCpp1(void , _ZdlPv, void *_0)
DMibMemoryInterposeCpp1(void , _ZdlPvRKSt9nothrow_t, void *_0)
DMibMemoryInterposeCpp2(void , _ZdlPvSt11align_val_t, void *_0, size_t _1)
DMibMemoryInterposeCpp2(void , _ZdlPvSt11align_val_tRKSt9nothrow_t, void *_0, size_t _1)
DMibMemoryInterposeCpp2(void , _ZdlPvm, void *_0, size_t _1)
DMibMemoryInterposeCpp3(void , _ZdlPvmSt11align_val_t, void *_0, size_t _1, size_t _2)

#endif

#endif

DMibMemoryInterpose2(kern_return_t, semaphore_timedwait, semaphore_t _0, mach_timespec_t _1)
DMibMemoryInterpose1(kern_return_t, semaphore_wait, semaphore_t _0)
DMibMemoryInterpose2(kern_return_t, semaphore_wait_signal, semaphore_t _0, semaphore_t _1)
DMibMemoryInterpose3(kern_return_t, semaphore_timedwait_signal, semaphore_t _0, semaphore_t _1, mach_timespec_t _2)

DMibMemoryInterpose4(int, __workq_kernreturn, int _0, user_addr_t _1, int _2, int _3)
DMibMemoryInterpose8(uint32_t, __psynch_cvwait, user_addr_t _0, uint64_t _1, uint32_t _2, user_addr_t _3, uint64_t _4, uint32_t _5, int64_t _6, uint32_t _7)
DMibMemoryInterpose6(int, kevent, int _0, const struct kevent *_1, int _2, struct kevent *_3, int _4, const struct timespec *_5)
DMibMemoryInterpose7(int, kevent64, int _0, const struct kevent64_s *_1, int _2, struct kevent64_s *_3, int _4, unsigned int _5, const struct timespec *_6)
DMibMemoryInterpose0(void, _malloc_fork_prepare)
DMibMemoryInterpose0(void, _malloc_fork_child)
DMibMemoryInterpose0(void, _malloc_fork_parent)
#if DMibConfig_Thread_DebugThreadLocals
DMibMemoryInterpose2(int, pthread_key_create, pthread_key_t *_0, void (* _1)(void *))
DMibMemoryInterpose1(int, pthread_key_delete, pthread_key_t _0)
#endif

DMibMemoryInterpose1(void, __exit, int _0)
DMibMemoryInterpose1(void, _exit, int _0)

// This needs to be last, as it will override the function used to allocate memory vm_region_64
DMibMemoryInterpose7(mach_msg_return_t, mach_msg, mach_msg_header_t *_0, mach_msg_option_t _1, mach_msg_size_t _2, mach_msg_size_t _3, mach_port_name_t _4, mach_msg_timeout_t _5, mach_port_name_t _6)

/* Unimplemented:
	__mach_stack_logging_shared_memory_address
	__malloc_late_init
	_malloc_no_asl_log
	_os_cpu_number_override
	mag_set_thread_index
	malloc_check_counter
	malloc_check_each
	malloc_check_start
	malloc_claimed_address
	malloc_create_legacy_default_zone
	malloc_debug
	malloc_engaged_nano
	malloc_engaged_secure_allocator
	malloc_enter_process_memory_limit_warn_mode
	malloc_error
	malloc_freezedry
	malloc_get_thread_options
	malloc_get_wrapped_zone
	malloc_logger
	malloc_memory_event_handler
	malloc_memorypressure_mask_default_4libdispatch
	malloc_num_zones
	malloc_num_zones_allocated
	malloc_printf
	malloc_register_stack_logger
	malloc_sanitizer_get_functions
	malloc_sanitizer_is_enabled
	malloc_sanitizer_set_functions
	malloc_set_thread_options
	malloc_singlethreaded
	malloc_variant_is_debug_4test
	malloc_zero_on_free_disable
	malloc_zones
	mstats
	pgm_extract_report_from_corpse
	reallocarray$DARWIN_EXTSN
	reallocarrayf$DARWIN_EXTSN
	sanitizer_diagnose_fault_from_crash_reporter
	scalable_zone_info
	scalable_zone_statistics
	set_malloc_singlethreaded
	stack_logging_enable_logging
	szone_check_counter
	szone_check_modulo
	szone_check_start
	tiny_print_region_free_list
	turn_off_stack_logging
	turn_on_stack_logging
	xzm_malloc_zone_introspect
	xzm_ptr_lookup_4test
	xzm_type_choose_ptr_bucket_4test
	zeroify_scalable_zone
*/

#undef DMibMemoryInterpose0
#undef DMibMemoryInterpose1
#undef DMibMemoryInterposeCpp1
#undef DMibMemoryInterposeCpp2
#undef DMibMemoryInterposeCpp3
#undef DMibMemoryInterpose2
#undef DMibMemoryInterpose3
#undef DMibMemoryInterpose4
#undef DMibMemoryInterpose5
#undef DMibMemoryInterpose6
#undef DMibMemoryInterpose7
#undef DMibMemoryInterpose8
#undef DMibMemoryInterpose
#undef DMibMemoryInterpose_SomeHooks
