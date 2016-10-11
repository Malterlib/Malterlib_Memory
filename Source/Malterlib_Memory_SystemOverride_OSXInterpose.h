// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <mach/message.h>
#include <sys/event.h>

extern "C" void * _Znam (size_t) __attribute__((weak_import));  // operator new
extern "C" void * _Znwm (size_t)__attribute__((weak_import));  // operator new
extern "C" void * _ZnwmRKSt9nothrow_t (size_t) __attribute__((weak_import)); // nothrow variants
extern "C" void * _ZnamRKSt9nothrow_t (size_t) __attribute__((weak_import)); // operator new nothrow
	
extern "C" void _ZdaPv (void *) __attribute__((weak_import)); // operator delete
extern "C" void _ZdlPv (void *) __attribute__((weak_import)); // operator delete 
extern "C" void _ZdaPvRKSt9nothrow_t (void *) __attribute__((weak_import)); // operator delete nothrow
extern "C" void _ZdlPvRKSt9nothrow_t (void *) __attribute__((weak_import)); // operator delete nothrow

extern "C" void vfree(void *) __attribute__((weak_import));

extern "C" mach_msg_return_t mach_msg_trap(
				mach_msg_header_t *msg,
				mach_msg_option_t option,
				mach_msg_size_t send_size,
				mach_msg_size_t rcv_size,
				mach_port_name_t rcv_name,
				mach_msg_timeout_t timeout,
				mach_port_name_t notify);

extern "C" kern_return_t semaphore_timedwait_trap(
				mach_port_name_t wait_name,
				unsigned int sec,
				clock_res_t nsec);

extern "C" kern_return_t semaphore_wait_trap(
				mach_port_name_t wait_name);

extern "C" kern_return_t semaphore_wait_signal_trap(
				mach_port_name_t wait_name,
				mach_port_name_t signal_name);

extern "C" kern_return_t semaphore_timedwait_signal_trap(
				mach_port_name_t wait_name,
				mach_port_name_t signal_name,
				unsigned int sec,
				clock_res_t nsec);

extern "C" int __workq_kernreturn(int options, user_addr_t item, int affinity, int prio);
extern "C" uint32_t __psynch_cvwait(user_addr_t cv, uint64_t cvlsgen, uint32_t cvugen, user_addr_t mutex, uint64_t mugen, uint32_t flags, int64_t sec, uint32_t nsec);

#define DMibMemoryInterpose(d_Return, d_Function, d_Args, ...) d_Return (*d_Function)(__VA_ARGS__);

struct COriginalFunctions
{
#include "Malterlib_Memory_SystemOverride_OSXInterposeFunctions.h"
};
