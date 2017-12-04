// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

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

#if defined(DMibMemoryInterpose_Hooks) && !defined(DMibMemoryOverrideDll)
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

DMibMemoryInterpose1(void *, malloc, size_t _0)
DMibMemoryInterpose1(void *, valloc, size_t _0)
DMibMemoryInterpose2(void *, calloc, size_t _0, size_t _1)
DMibMemoryInterpose2(void *, realloc, void *_0, size_t _1)
DMibMemoryInterpose2(void *, reallocf, void *_0, size_t _1)
DMibMemoryInterpose1(void, free, void *_0)
DMibMemoryInterpose1(void, vfree, void *_0)
DMibMemoryInterpose3(int, posix_memalign, void **_0, size_t _1, size_t _2)
	
DMibMemoryInterpose1(int, malloc_jumpstart, int _0)

#ifndef DMibMemoryInterpose_Hooks
DMibMemoryInterposeCpp1(void *, _Znwm, size_t _0)
DMibMemoryInterposeCpp1(void *, _Znam, size_t _0)
DMibMemoryInterposeCpp1(void , _ZdlPv, void * _0)
DMibMemoryInterposeCpp1(void , _ZdaPv, void * _0)
DMibMemoryInterposeCpp1(void *, _ZnwmRKSt9nothrow_t, size_t _0)
DMibMemoryInterposeCpp1(void *, _ZnamRKSt9nothrow_t, size_t _0)
DMibMemoryInterposeCpp1(void , _ZdaPvRKSt9nothrow_t, void *_0)
DMibMemoryInterposeCpp1(void , _ZdlPvRKSt9nothrow_t, void *_0)
#endif

#endif

DMibMemoryInterpose3(kern_return_t, semaphore_timedwait_trap, mach_port_name_t _0, unsigned int _1, clock_res_t _2)
DMibMemoryInterpose1(kern_return_t, semaphore_wait_trap, mach_port_name_t _0)
DMibMemoryInterpose2(kern_return_t, semaphore_wait_signal_trap, mach_port_name_t _0, mach_port_name_t _1)
DMibMemoryInterpose4(kern_return_t, semaphore_timedwait_signal_trap, mach_port_name_t _0, mach_port_name_t _1, unsigned int _2, clock_res_t _3)
DMibMemoryInterpose4(int, __workq_kernreturn, int _0, user_addr_t _1, int _2, int _3)
DMibMemoryInterpose8(uint32_t, __psynch_cvwait, user_addr_t _0, uint64_t _1, uint32_t _2, user_addr_t _3, uint64_t _4, uint32_t _5, int64_t _6, uint32_t _7)
DMibMemoryInterpose6(int, kevent, int _0, const struct kevent *_1, int _2, struct kevent *_3, int _4, const struct timespec *_5)
DMibMemoryInterpose7(int, kevent64, int _0, const struct kevent64_s *_1, int _2, struct kevent64_s *_3, int _4, unsigned int _5, const struct timespec *_6)
DMibMemoryInterpose7(mach_msg_return_t ,mach_msg_trap, mach_msg_header_t *_0, mach_msg_option_t _1, mach_msg_size_t _2, mach_msg_size_t _3, mach_port_name_t _4, mach_msg_timeout_t _5, mach_port_name_t _6)
DMibMemoryInterpose0(void,_malloc_fork_prepare)


#undef DMibMemoryInterpose0
#undef DMibMemoryInterpose1
#undef DMibMemoryInterposeCpp1
#undef DMibMemoryInterpose2
#undef DMibMemoryInterpose3
#undef DMibMemoryInterpose4
#undef DMibMemoryInterpose5
#undef DMibMemoryInterpose6
#undef DMibMemoryInterpose7
#undef DMibMemoryInterpose8
#undef DMibMemoryInterpose
#undef DMibMemoryInterpose_SomeHooks
