// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Each function is listed with its return type, its name, the parenthesized names of its parameters, which the
// interpose functions pass on to the replacements, and then its parameters.

#ifdef DMibMemoryInterpose

#	ifndef DMibMemoryInterpose0
#		define DMibMemoryInterpose0(d_Return, d_Function, ...) DMibMemoryInterpose(d_Return, d_Function, (), __VA_ARGS__)
#	endif
#	ifndef DMibMemoryInterpose1
#		define DMibMemoryInterpose1(d_Return, d_Function, d_Args, ...) DMibMemoryInterpose(d_Return, d_Function, d_Args, __VA_ARGS__)
#	endif
#	ifndef DMibMemoryInterposeCpp1
#		define DMibMemoryInterposeCpp1 DMibMemoryInterpose1
#	endif
#	ifndef DMibMemoryInterposeCpp2
#		define DMibMemoryInterposeCpp2 DMibMemoryInterpose2
#	endif
#	ifndef DMibMemoryInterposeCpp3
#		define DMibMemoryInterposeCpp3 DMibMemoryInterpose3
#	endif
#	ifndef DMibMemoryInterpose2
#		define DMibMemoryInterpose2(d_Return, d_Function, d_Args, ...) DMibMemoryInterpose(d_Return, d_Function, d_Args, __VA_ARGS__)
#	endif
#	ifndef DMibMemoryInterpose3
#		define DMibMemoryInterpose3(d_Return, d_Function, d_Args, ...) DMibMemoryInterpose(d_Return, d_Function, d_Args, __VA_ARGS__)
#	endif
#	ifndef DMibMemoryInterpose4
#		define DMibMemoryInterpose4(d_Return, d_Function, d_Args, ...) DMibMemoryInterpose(d_Return, d_Function, d_Args, __VA_ARGS__)
#	endif
#	ifndef DMibMemoryInterpose5
#		define DMibMemoryInterpose5(d_Return, d_Function, d_Args, ...) DMibMemoryInterpose(d_Return, d_Function, d_Args, __VA_ARGS__)
#	endif
#	ifndef DMibMemoryInterpose6
#		define DMibMemoryInterpose6(d_Return, d_Function, d_Args, ...) DMibMemoryInterpose(d_Return, d_Function, d_Args, __VA_ARGS__)
#	endif
#	ifndef DMibMemoryInterpose7
#		define DMibMemoryInterpose7(d_Return, d_Function, d_Args, ...) DMibMemoryInterpose(d_Return, d_Function, d_Args, __VA_ARGS__)
#	endif
#	ifndef DMibMemoryInterpose8
#		define DMibMemoryInterpose8(d_Return, d_Function, d_Args, ...) DMibMemoryInterpose(d_Return, d_Function, d_Args, __VA_ARGS__)
#	endif
#else

#	ifndef DMibMemoryInterpose1
#		define DMibMemoryInterpose1 DMibMemoryInterpose0
#	endif
#	ifndef DMibMemoryInterposeCpp1
#		define DMibMemoryInterposeCpp1 DMibMemoryInterpose0
#	endif
#	ifndef DMibMemoryInterposeCpp2
#		define DMibMemoryInterposeCpp2 DMibMemoryInterpose0
#	endif
#	ifndef DMibMemoryInterposeCpp3
#		define DMibMemoryInterposeCpp3 DMibMemoryInterpose0
#	endif
#	ifndef DMibMemoryInterpose2
#		define DMibMemoryInterpose2 DMibMemoryInterpose0
#	endif
#	ifndef DMibMemoryInterpose3
#		define DMibMemoryInterpose3 DMibMemoryInterpose0
#	endif
#	ifndef DMibMemoryInterpose4
#		define DMibMemoryInterpose4 DMibMemoryInterpose0
#	endif
#	ifndef DMibMemoryInterpose5
#		define DMibMemoryInterpose5 DMibMemoryInterpose0
#	endif
#	ifndef DMibMemoryInterpose6
#		define DMibMemoryInterpose6 DMibMemoryInterpose0
#	endif
#	ifndef DMibMemoryInterpose7
#		define DMibMemoryInterpose7 DMibMemoryInterpose0
#	endif
#	ifndef DMibMemoryInterpose8
#		define DMibMemoryInterpose8 DMibMemoryInterpose0
#	endif

#endif

#if defined(DMibMemoryInterpose_Hooks)
#	define DMibMemoryInterpose_SomeHooks
#endif

#ifndef DMibMemoryInterpose_SomeHooks
DMibMemoryInterpose0(malloc_zone_t *, malloc_default_zone)
DMibMemoryInterpose2(malloc_zone_t *, malloc_create_zone, (_StartSize, _Flags), vm_size_t _StartSize, unsigned _Flags)
DMibMemoryInterpose1(void, malloc_destroy_zone, (_pZone), malloc_zone_t *_pZone)
DMibMemoryInterpose2(void *, malloc_zone_malloc, (_pZone, _Size), malloc_zone_t *_pZone, size_t _Size)
DMibMemoryInterpose3(void *, malloc_zone_calloc, (_pZone, _nItems, _Size), malloc_zone_t *_pZone, size_t _nItems, size_t _Size)
DMibMemoryInterpose2(void *, malloc_zone_valloc, (_pZone, _Size), malloc_zone_t *_pZone, size_t _Size)
DMibMemoryInterpose2(void, malloc_zone_free, (_pZone, _pMemory), malloc_zone_t *_pZone, void *_pMemory)
DMibMemoryInterpose3(void *, malloc_zone_realloc, (_pZone, _pMemory, _Size), malloc_zone_t *_pZone, void *_pMemory, size_t _Size)
DMibMemoryInterpose1(malloc_zone_t *, malloc_zone_from_ptr, (_pMemory), void const *_pMemory)
DMibMemoryInterpose1(size_t, malloc_size, (_pMemory), void const *_pMemory)
DMibMemoryInterpose1(size_t, malloc_good_size, (_Size), size_t _Size)
DMibMemoryInterpose3(void *, malloc_zone_memalign, (_pZone, _Alignment, _Size), malloc_zone_t *_pZone, size_t _Alignment, size_t _Size)
DMibMemoryInterpose4(unsigned, malloc_zone_batch_malloc, (_pZone, _Size, o_pResults, _nRequested), malloc_zone_t *_pZone, size_t _Size, void **o_pResults, unsigned _nRequested)
DMibMemoryInterpose3(void, malloc_zone_batch_free, (_pZone, _pToBeFreed, _nToBeFreed), malloc_zone_t *_pZone, void **_pToBeFreed, unsigned _nToBeFreed)
DMibMemoryInterpose0(malloc_zone_t *, malloc_default_purgeable_zone)
DMibMemoryInterpose1(void, malloc_make_purgeable, (_pMemory), void *_pMemory)
DMibMemoryInterpose1(int, malloc_make_nonpurgeable, (_pMemory), void *_pMemory)
DMibMemoryInterpose1(void, malloc_zone_register, (_pZone), malloc_zone_t *_pZone)
DMibMemoryInterpose1(void, malloc_zone_unregister, (_pZone), malloc_zone_t *_pZone)
DMibMemoryInterpose2(void, malloc_set_zone_name, (_pZone, _pName), malloc_zone_t *_pZone, char const *_pName)
#	ifndef DMibMemoryInterpose_Hooks
DMibMemoryInterpose1(char const *, malloc_get_zone_name, (_pZone), malloc_zone_t *_pZone)
#	endif
DMibMemoryInterpose2(size_t, malloc_zone_pressure_relief, (_pZone, _Goal), malloc_zone_t *_pZone, size_t _Goal)
DMibMemoryInterpose4(kern_return_t, malloc_get_all_zones, (_Task, _fReader, o_pAddresses, o_pCount), task_t _Task, memory_reader_t _fReader, vm_address_t **o_pAddresses, unsigned *o_pCount)
DMibMemoryInterpose1(void, malloc_zone_print_ptr_info, (_pMemory), void *_pMemory)
#	ifndef DMibMemoryInterpose_Hooks
DMibMemoryInterpose1(boolean_t, malloc_zone_check, (_pZone), malloc_zone_t *_pZone)
#	endif
DMibMemoryInterpose2(void, malloc_zone_print, (_pZone, _bVerbose), malloc_zone_t *_pZone, boolean_t _bVerbose)
#	ifndef DMibMemoryInterpose_Hooks
DMibMemoryInterpose2(void, malloc_zone_statistics, (_pZone, o_pStatistics), malloc_zone_t *_pZone, malloc_statistics_t *o_pStatistics)
#	endif
DMibMemoryInterpose2(void, malloc_zone_log, (_pZone, _pAddress), malloc_zone_t *_pZone, void *_pAddress)
#	ifndef DMibMemoryInterpose_Hooks
DMibMemoryInterpose1(boolean_t, malloc_zone_enable_discharge_checking, (_pZone), malloc_zone_t *_pZone)
DMibMemoryInterpose1(void, malloc_zone_disable_discharge_checking, (_pZone), malloc_zone_t *_pZone)
#	endif
DMibMemoryInterpose2(void, malloc_zone_discharge, (_pZone, _pMemory), malloc_zone_t *_pZone, void *_pMemory)
DMibMemoryInterpose2(void, malloc_zone_enumerate_discharged_pointers, (_pZone, _fReportDischarged), malloc_zone_t *_pZone, void (^_fReportDischarged)(void *_pMemory, void *_pInfo))

DMibMemoryInterpose2(boolean_t, malloc_zone_claimed_address, (_pZone, _pMemory), malloc_zone_t *_pZone, void *_pMemory)
DMibMemoryInterpose4(void *, malloc_zone_malloc_with_options_np, (_pZone, _Alignment, _Size, _Options), malloc_zone_t *_pZone, size_t _Alignment, size_t _Size, uint64_t _Options)
DMibMemoryInterpose4
	(
		void *
		, malloc_zone_malloc_with_options
		, (_pZone, _Alignment, _Size, _Options)
		, malloc_zone_t *_pZone
		, size_t _Alignment
		, size_t _Size
		, malloc_zone_malloc_options_t _Options
	)

DMibMemoryInterpose5
	(
		void *
		, malloc_type_zone_malloc_with_options_np
		, (_pZone, _Alignment, _Size, _Options, _TypeID)
		, malloc_zone_t *_pZone
		, size_t _Alignment
		, size_t _Size
		, malloc_options_np_t _Options
		, malloc_type_id_t _TypeID
	)
DMibMemoryInterpose5
	(
		void *
		, malloc_type_zone_malloc_with_options_internal
		, (_pZone, _Alignment, _Size, _Options, _TypeID)
		, malloc_zone_t *_pZone
		, size_t _Alignment
		, size_t _Size
		, malloc_options_np_t _Options
		, malloc_type_id_t _TypeID
	)
DMibMemoryInterpose5
	(
		void *
		, malloc_type_zone_malloc_with_options
		, (_pZone, _Alignment, _Size, _TypeID, _Options)
		, malloc_zone_t *_pZone
		, size_t _Alignment
		, size_t _Size
		, malloc_type_id_t _TypeID
		, malloc_zone_malloc_options_t _Options
	)

DMibMemoryInterpose3(void *, malloc_type_zone_malloc, (_pZone, _Size, _TypeID), malloc_zone_t *_pZone, size_t _Size, malloc_type_id_t _TypeID)
DMibMemoryInterpose4(void *, malloc_type_zone_calloc, (_pZone, _nItems, _Size, _TypeID), malloc_zone_t *_pZone, size_t _nItems, size_t _Size, malloc_type_id_t _TypeID)
DMibMemoryInterpose3(void, malloc_type_zone_free, (_pZone, _pMemory, _TypeID), malloc_zone_t *_pZone, void *_pMemory, malloc_type_id_t _TypeID)
DMibMemoryInterpose4(void *, malloc_type_zone_realloc, (_pZone, _pMemory, _Size, _TypeID), malloc_zone_t *_pZone, void *_pMemory, size_t _Size, malloc_type_id_t _TypeID)
DMibMemoryInterpose3(void *, malloc_type_zone_valloc, (_pZone, _Size, _TypeID), malloc_zone_t *_pZone, size_t _Size, malloc_type_id_t _TypeID)
DMibMemoryInterpose4(void *, malloc_type_zone_memalign, (_pZone, _Alignment, _Size, _TypeID), malloc_zone_t *_pZone, size_t _Alignment, size_t _Size, malloc_type_id_t _TypeID)

DMibMemoryInterpose1(void *, malloc, (_Size), size_t _Size)
DMibMemoryInterpose1(void *, valloc, (_Size), size_t _Size)
DMibMemoryInterpose2(void *, calloc, (_nItems, _Size), size_t _nItems, size_t _Size)
DMibMemoryInterpose2(void *, realloc, (_pMemory, _Size), void *_pMemory, size_t _Size)
DMibMemoryInterpose2(void *, reallocf, (_pMemory, _Size), void *_pMemory, size_t _Size)
DMibMemoryInterpose1(void, free, (_pMemory), void *_pMemory)
DMibMemoryInterpose1(void, vfree, (_pMemory), void *_pMemory)
DMibMemoryInterpose3(int, posix_memalign, (o_pMemory, _Alignment, _Size), void **o_pMemory, size_t _Alignment, size_t _Size)
DMibMemoryInterpose2(void *, aligned_alloc, (_Alignment, _Size), size_t _Alignment, size_t _Size)

DMibMemoryInterpose2(void *, malloc_type_malloc, (_Size, _TypeID), size_t _Size, malloc_type_id_t _TypeID)
DMibMemoryInterpose3(void *, malloc_type_calloc, (_nItems, _Size, _TypeID), size_t _nItems, size_t _Size, malloc_type_id_t _TypeID)
DMibMemoryInterpose2(void, malloc_type_free, (_pMemory, _TypeID), void *_pMemory, malloc_type_id_t _TypeID)
DMibMemoryInterpose3(void *, malloc_type_realloc, (_pMemory, _Size, _TypeID), void *_pMemory, size_t _Size, malloc_type_id_t _TypeID)
DMibMemoryInterpose2(void *, malloc_type_valloc, (_Size, _TypeID), size_t _Size, malloc_type_id_t _TypeID)
DMibMemoryInterpose3(void *, malloc_type_aligned_alloc, (_Alignment, _Size, _TypeID), size_t _Alignment, size_t _Size, malloc_type_id_t _TypeID)
DMibMemoryInterpose4(int, malloc_type_posix_memalign, (o_pMemory, _Alignment, _Size, _TypeID), void **o_pMemory, size_t _Alignment, size_t _Size, malloc_type_id_t _TypeID)

DMibMemoryInterpose1(int, malloc_jumpstart, (_Value), int _Value)

#	ifndef DMibMemoryInterpose_Hooks
DMibMemoryInterposeCpp1(void *, _Znam, (_Size), size_t _Size)
DMibMemoryInterposeCpp1(void *, _ZnamRKSt9nothrow_t, (_Size), size_t _Size)
DMibMemoryInterposeCpp2(void *, _ZnamSt11align_val_t, (_Size, _Alignment), size_t _Size, size_t _Alignment)
DMibMemoryInterposeCpp2(void *, _ZnamSt11align_val_tRKSt9nothrow_t, (_Size, _Alignment), size_t _Size, size_t _Alignment)
DMibMemoryInterposeCpp1(void *, _Znwm, (_Size), size_t _Size)
DMibMemoryInterposeCpp1(void *, _ZnwmRKSt9nothrow_t, (_Size), size_t _Size)
DMibMemoryInterposeCpp2(void *, _ZnwmSt11align_val_t, (_Size, _Alignment), size_t _Size, size_t _Alignment)
DMibMemoryInterposeCpp2(void *, _ZnwmSt11align_val_tRKSt9nothrow_t, (_Size, _Alignment), size_t _Size, size_t _Alignment)
DMibMemoryInterposeCpp1(void, _ZdaPv, (_pMemory), void *_pMemory)
DMibMemoryInterposeCpp1(void, _ZdaPvRKSt9nothrow_t, (_pMemory), void *_pMemory)
DMibMemoryInterposeCpp2(void, _ZdaPvSt11align_val_t, (_pMemory, _Alignment), void *_pMemory, size_t _Alignment)
DMibMemoryInterposeCpp2(void, _ZdaPvSt11align_val_tRKSt9nothrow_t, (_pMemory, _Alignment), void *_pMemory, size_t _Alignment)
DMibMemoryInterposeCpp2(void, _ZdaPvm, (_pMemory, _Size), void *_pMemory, size_t _Size)
DMibMemoryInterposeCpp3(void, _ZdaPvmSt11align_val_t, (_pMemory, _Size, _Alignment), void *_pMemory, size_t _Size, size_t _Alignment)
DMibMemoryInterposeCpp1(void, _ZdlPv, (_pMemory), void *_pMemory)
DMibMemoryInterposeCpp1(void, _ZdlPvRKSt9nothrow_t, (_pMemory), void *_pMemory)
DMibMemoryInterposeCpp2(void, _ZdlPvSt11align_val_t, (_pMemory, _Alignment), void *_pMemory, size_t _Alignment)
DMibMemoryInterposeCpp2(void, _ZdlPvSt11align_val_tRKSt9nothrow_t, (_pMemory, _Alignment), void *_pMemory, size_t _Alignment)
DMibMemoryInterposeCpp2(void, _ZdlPvm, (_pMemory, _Size), void *_pMemory, size_t _Size)
DMibMemoryInterposeCpp3(void, _ZdlPvmSt11align_val_t, (_pMemory, _Size, _Alignment), void *_pMemory, size_t _Size, size_t _Alignment)

#	endif

#endif

DMibMemoryInterpose2(kern_return_t, semaphore_timedwait, (_Semaphore, _WaitTime), semaphore_t _Semaphore, mach_timespec_t _WaitTime)
DMibMemoryInterpose1(kern_return_t, semaphore_wait, (_Semaphore), semaphore_t _Semaphore)
DMibMemoryInterpose2(kern_return_t, semaphore_wait_signal, (_WaitSemaphore, _SignalSemaphore), semaphore_t _WaitSemaphore, semaphore_t _SignalSemaphore)
DMibMemoryInterpose3
	(
		kern_return_t
		, semaphore_timedwait_signal
		, (_WaitSemaphore, _SignalSemaphore, _WaitTime)
		, semaphore_t _WaitSemaphore
		, semaphore_t _SignalSemaphore
		, mach_timespec_t _WaitTime
	)

DMibMemoryInterpose4(int, __workq_kernreturn, (_Options, _Item, _Affinity, _Priority), int _Options, user_addr_t _Item, int _Affinity, int _Priority)
DMibMemoryInterpose8
	(
		uint32_t
		, __psynch_cvwait
		, (_Condition, _ConditionLSGeneration, _ConditionUGeneration, _Mutex, _MutexGeneration, _Flags, _Seconds, _Nanoseconds)
		, user_addr_t _Condition
		, uint64_t _ConditionLSGeneration
		, uint32_t _ConditionUGeneration
		, user_addr_t _Mutex
		, uint64_t _MutexGeneration
		, uint32_t _Flags
		, int64_t _Seconds
		, uint32_t _Nanoseconds
	)
DMibMemoryInterpose4(int, __ulock_wait, (_Operation, _pAddress, _Value, _TimeoutMicroseconds), uint32_t _Operation, void *_pAddress, uint64_t _Value, uint32_t _TimeoutMicroseconds)
DMibMemoryInterpose5
	(
		int
		, __ulock_wait2
		, (_Operation, _pAddress, _Value, _TimeoutNanoseconds, _Value2)
		, uint32_t _Operation
		, void *_pAddress
		, uint64_t _Value
		, uint64_t _TimeoutNanoseconds
		, uint64_t _Value2
	)
DMibMemoryInterpose4(int, os_sync_wait_on_address, (_pAddress, _Value, _Size, _Flags), void *_pAddress, uint64_t _Value, size_t _Size, uint32_t _Flags)
DMibMemoryInterpose6
	(
		int
		, os_sync_wait_on_address_with_timeout
		, (_pAddress, _Value, _Size, _Flags, _ClockID, _TimeoutNanoseconds)
		, void *_pAddress
		, uint64_t _Value
		, size_t _Size
		, uint32_t _Flags
		, uint32_t _ClockID
		, uint64_t _TimeoutNanoseconds
	)
DMibMemoryInterpose6
	(
		int
		, kevent
		, (_KernelQueue, _pChanges, _nChanges, o_pEvents, _nEvents, _pTimeout)
		, int _KernelQueue
		, struct kevent const *_pChanges
		, int _nChanges
		, struct kevent *o_pEvents
		, int _nEvents
		, struct timespec const *_pTimeout
	)
DMibMemoryInterpose7
	(
		int
		, kevent64
		, (_KernelQueue, _pChanges, _nChanges, o_pEvents, _nEvents, _Flags, _pTimeout)
		, int _KernelQueue
		, struct kevent64_s const *_pChanges
		, int _nChanges
		, struct kevent64_s *o_pEvents
		, int _nEvents
		, unsigned int _Flags
		, struct timespec const *_pTimeout
	)
DMibMemoryInterpose3(int, poll, (_pFileDescriptors, _nFileDescriptors, _Timeout), struct pollfd *_pFileDescriptors, nfds_t _nFileDescriptors, int _Timeout)
DMibMemoryInterpose2(int, nanosleep, (_pRequested, o_pRemaining), struct timespec const *_pRequested, struct timespec *o_pRemaining)
DMibMemoryInterpose0(void, _malloc_fork_prepare)
DMibMemoryInterpose0(void, _malloc_fork_child)
DMibMemoryInterpose0(void, _malloc_fork_parent)
#if DMibConfig_Thread_DebugThreadLocals
DMibMemoryInterpose2(int, pthread_key_create, (o_pKey, _fDestructor), pthread_key_t *o_pKey, void (*_fDestructor)(void *_pValue))
DMibMemoryInterpose1(int, pthread_key_delete, (_Key), pthread_key_t _Key)
#endif

DMibMemoryInterpose1(void, __exit, (_ExitCode), int _ExitCode)
DMibMemoryInterpose1(void, _exit, (_ExitCode), int _ExitCode)

// This needs to be last, as it will override the function used to allocate memory vm_region_64
DMibMemoryInterpose7
	(
		mach_msg_return_t
		, mach_msg
		, (_pMessage, _Option, _SendSize, _ReceiveSize, _ReceiveName, _Timeout, _Notify)
		, mach_msg_header_t *_pMessage
		, mach_msg_option_t _Option
		, mach_msg_size_t _SendSize
		, mach_msg_size_t _ReceiveSize
		, mach_port_name_t _ReceiveName
		, mach_msg_timeout_t _Timeout
		, mach_port_name_t _Notify
	)

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
