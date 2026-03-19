// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#if (defined(DMibConfig_OverrideSystemMalloc) || defined(DMibMemoryOverrideDll)) && defined(DMalterlibMemoryOverrideMacOSInitBeforeLibSystemSupport)

#include <Mib/Core/Core>
#include <dlfcn.h>

extern "C" int mach_init(void);
namespace
{
	void (*bootstrap_init)(void) = nullptr;
}


bool fg_MalterlibSystem_InitMacOS1060()
{
	void *pLibSystemInfo = dlsym(RTLD_DEFAULT, "__Libsystem_version");

	if (!pLibSystemInfo)
	{
		DMibTraceSafe("No __Libsystem_version found, malloc override not enabled!\n", 0);
		return false;
	}

	// This path is taken os OXS 10.6

	Dl_info Info;
	if (!dladdr((void *)(umint)pLibSystemInfo, &Info))
	{
		DMibTraceSafe("No dladdr for __Libsystem_version found, malloc override not enabled!\n", 0);
		return false;
	}

	void (* pthread_init)() = nullptr;

	uint8 *pStartAddress = (uint8 *)Info.dli_fbase;
	uint8 *pEndAddress = pStartAddress + 8*1024; // Go through 8 KB at max
	umint nLoop = 0;
	while (pStartAddress < pEndAddress)
	{
		++nLoop;
		Dl_info Info;
		if (dladdr((void *)(umint)pStartAddress, &Info))
		{
			if (Info.dli_sname && NMib::NStr::fg_StrCmp(Info.dli_sname, "pthread_init") == 0)
			{
				(void * &)pthread_init = Info.dli_saddr;
				break;
			}
		}
		// 10.6 function is 614 bytes, so to be save lets walk 256 bytes a time
		pStartAddress += 256;
	}

	if (!pthread_init)
	{
		DMibTraceSafe("No pthread_init found, malloc override not enabled!\n", 0);
		return false;
	}

	mach_init();
	pthread_init();
	return true;
}

bool fg_MalterlibSystem_InitMacOS1070(void *_pPThreadInit)
{
	void (* pthread_init)();
	(void * &)pthread_init = _pPThreadInit;

	// This code path is taken on macOS 10.7 and 10.8
	typedef struct _libkernel_functions {
		mach_port_t (*get_reply_port)(void);
		void (*set_reply_port)(mach_port_t);
		void* (*dlsym)(void*, const char*);
		void *_placeholder_1;
		void *_placeholder_2;
		void (*set_errno)(int);
		int* (*get_errno)(void);

	} _libkernel_functions_t;

	_libkernel_functions_t libkernel_funcs = {
		.get_reply_port = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "_mig_get_reply_port")),
		.set_reply_port = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "_mig_set_reply_port")),
		.get_errno = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "__error")),
		.set_errno = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "cthread_set_errno_self")),
		.dlsym = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "dlsym")),
	};

	if (!libkernel_funcs.get_reply_port)
	{
		DMibTraceSafe("No libkernel_funcs.get_reply_port found, malloc override not enabled!\n", 0);
		return false;
	}
	if (!libkernel_funcs.set_reply_port)
	{
		DMibTraceSafe("No libkernel_funcs.set_reply_port found, malloc override not enabled!\n", 0);
		return false;
	}
	if (!libkernel_funcs.get_errno)
	{
		DMibTraceSafe("No libkernel_funcs.get_errno found, malloc override not enabled!\n", 0);
		return false;
	}
	if (!libkernel_funcs.set_errno)
	{
		DMibTraceSafe("No libkernel_funcs.set_errno found, malloc override not enabled!\n", 0);
		return false;
	}
	if (!libkernel_funcs.dlsym)
	{
		DMibTraceSafe("No libkernel_funcs.dlsym found, malloc override not enabled!\n", 0);
		return false;
	}

	void (*_libkernel_init)(_libkernel_functions_t fns);

	(void * &)_libkernel_init = dlsym(RTLD_DEFAULT, "_libkernel_init");

	if (!_libkernel_init)
	{
		DMibTraceSafe("No _libkernel_init found, malloc override not enabled!\n", 0);
		return false;
	}

	(void * &)bootstrap_init = dlsym(RTLD_DEFAULT, "bootstrap_init");

	if (!bootstrap_init)
	{
		DMibTraceSafe("No bootstrap_init found, malloc override not enabled!\n", 0);
		return false;
	}

	_libkernel_init(libkernel_funcs);
	bootstrap_init();
	mach_init();
	pthread_init();
	return true;
}

bool fg_MalterlibSystem_InitMacOS10100(void *_pPThreadInit, char const* envp[], char const* apple[], const struct ProgramVars * vars)
{
	// This path is taken on macOS 10.10

	struct _libpthread_functions {
		/* Structure version 1. Subsequent versions must only add pointers! */
		unsigned long version;
		void (*exit)(int);
		void* (*malloc)(size_t);
		void  (*free)(void*);
	};

	void (* __pthread_init)(const struct _libpthread_functions *libpthread_funcs, const char *envp[], const char *apple[], const struct ProgramVars *vars);
	(void * &)__pthread_init = _pPThreadInit;

	typedef const struct _libkernel_functions {
		/* Structure version 1. Subsequent versions must only add pointers! */
		unsigned long version;
		void* (*dlsym)(void*, const char*);
		void* (*malloc)(size_t);
		void  (*free)(void*);
		void* (*realloc)(void*, size_t);
		void  (*_pthread_exit_if_canceled)(int);
	} *_libkernel_functions_t;

	typedef const struct _libc_functions {
		/* Structure version 1. Subsequent versions must only add pointers! */
		unsigned long version;
		void* (*libSystem_atfork_prepare)();
		void* (*libSystem_atfork_parent)();
		void* (*libSystem_atfork_child)();
		void* (*_dirhelper)();
		void* (*mach_init_old)();
	} *_libc_functions_t;

	void (* _libkernel_init)(_libkernel_functions_t fns, const char *envp[], const char *apple[], const struct ProgramVars *vars);

	(void * &)_libkernel_init = dlsym(RTLD_DEFAULT, "__libkernel_init");

	if (!_libkernel_init)
		(void * &)_libkernel_init = dlsym(RTLD_DEFAULT, "_libkernel_init");

	if (!_libkernel_init)
	{
		DMibTraceSafe("No _libkernel_init found, malloc override not enabled!\n", 0);
		return false;
	}

	void (* _libc_initializer)(_libc_functions_t fns, const char *envp[], const char *apple[], const struct ProgramVars *vars);

	(void * &)_libc_initializer = dlsym(RTLD_DEFAULT, "_libc_initializer");

	if (!_libc_initializer)
		(void * &)_libc_initializer = dlsym(RTLD_DEFAULT, "libc_initializer");

	if (!_libc_initializer)
	{
		DMibTraceSafe("No _libkernel_init found, malloc override not enabled!\n", 0);
		return false;
	}



	static const struct _libkernel_functions libkernel_funcs = {
		.version = 3,
		.dlsym = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "dlsym")),
		.malloc = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "malloc")),
		.free = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "free")),
		.realloc = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "realloc")),
		._pthread_exit_if_canceled = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "_pthread_exit_if_canceled")),
	};

	if (!libkernel_funcs.dlsym)
	{
		DMibTraceSafe("No libkernel_funcs.dlsym found, malloc override not enabled!\n", 0);
		return false;
	}

	if (!libkernel_funcs.dlsym)
	{
		DMibTraceSafe("No libkernel_funcs.dlsym found, malloc override not enabled!\n", 0);
		return false;
	}

	void (*bootstrap_init)(void);
	(void * &)bootstrap_init = dlsym(RTLD_DEFAULT, "bootstrap_init");

	if (!bootstrap_init)
	{
		DMibTraceSafe("No bootstrap_init found, malloc override not enabled!\n", 0);
		return false;
	}

	void (* __libplatform_init)(void *future_use, const char *envp[], const char *apple[], const struct ProgramVars *vars);
	(void * &)__libplatform_init = dlsym(RTLD_DEFAULT, "__libplatform_init");

	if (!__libplatform_init)
	{
		DMibTraceSafe("No __libplatform_init found, malloc override not enabled!\n", 0);
		return false;
	}

	static const struct _libpthread_functions libpthread_funcs = {
		.version = 2,
		.exit = (void (*)(int))dlsym(RTLD_DEFAULT, "exit"),
		.malloc = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "malloc")),
		.free = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "free")),
	};

	if (!libpthread_funcs.exit)
	{
		DMibTraceSafe("No libpthread_funcs.exit found, malloc override not enabled!\n", 0);
		return false;
	}

	_libkernel_init(&libkernel_funcs, envp, apple, vars);
	__libplatform_init(NULL, envp, apple, vars);
	__pthread_init(&libpthread_funcs, envp, apple, vars);
	static const struct _libc_functions libc_funcs = {
		.version = 1,
		.libSystem_atfork_prepare = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "libSystem_atfork_prepare")),
		.libSystem_atfork_parent = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "libSystem_atfork_parent")),
		.libSystem_atfork_child = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "libSystem_atfork_child")),
		._dirhelper = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "_dirhelper")),
		.mach_init_old = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "mach_init")),
	};

	_libc_initializer(&libc_funcs, envp, apple, vars);

	return true;
}

bool fg_MalterlibSystem_InitMacOS1090(void *_pPThreadInit, char const* envp[], char const* apple[], const struct ProgramVars * vars)
{
	// This path is taken on macOS 10.9

	struct _libpthread_functions {
		/* Structure version 1. Subsequent versions must only add pointers! */
		unsigned long version;
		void (*exit)(int);
	};

	void (* __pthread_init)(const struct _libpthread_functions *libpthread_funcs, const char *envp[], const char *apple[], const struct ProgramVars *vars);
	(void * &)__pthread_init = _pPThreadInit;

	typedef const struct _libkernel_functions {
		/* Structure version 1. Subsequent versions must only add pointers! */
		unsigned long version;
		void* (*dlsym)(void*, const char*);
		void* (*malloc)(size_t);
		void  (*free)(void*);
		void* (*realloc)(void*, size_t);
		void  (*_pthread_exit_if_canceled)(int);
	} *_libkernel_functions_t;

	void (* _libkernel_init)(_libkernel_functions_t fns, const char *envp[], const char *apple[], const struct ProgramVars *vars);

	(void * &)_libkernel_init = dlsym(RTLD_DEFAULT, "__libkernel_init");

	if (!_libkernel_init)
		(void * &)_libkernel_init = dlsym(RTLD_DEFAULT, "_libkernel_init");

	if (!_libkernel_init)
	{
		DMibTraceSafe("No _libkernel_init found, malloc override not enabled!\n", 0);
		return false;
	}

	static const struct _libkernel_functions libkernel_funcs = {
		.version = 1,
		.dlsym = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "dlsym")),
		.malloc = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "malloc")),
		.free = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "free")),
		.realloc = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "realloc")),
		._pthread_exit_if_canceled = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "_pthread_exit_if_canceled")),
	};

	if (!libkernel_funcs.dlsym)
	{
		DMibTraceSafe("No libkernel_funcs.dlsym found, malloc override not enabled!\n", 0);
		return false;
	}

	if (!libkernel_funcs.dlsym)
	{
		DMibTraceSafe("No libkernel_funcs.dlsym found, malloc override not enabled!\n", 0);
		return false;
	}

	void (*bootstrap_init)(void);
	(void * &)bootstrap_init = dlsym(RTLD_DEFAULT, "bootstrap_init");

	if (!bootstrap_init)
	{
		DMibTraceSafe("No bootstrap_init found, malloc override not enabled!\n", 0);
		return false;
	}

	void (* __libplatform_init)(void *future_use, const char *envp[], const char *apple[], const struct ProgramVars *vars);
	(void * &)__libplatform_init = dlsym(RTLD_DEFAULT, "__libplatform_init");

	if (!__libplatform_init)
	{
		DMibTraceSafe("No __libplatform_init found, malloc override not enabled!\n", 0);
		return false;
	}

	static const struct _libpthread_functions libpthread_funcs = {
		.version = 1,
		.exit = (void (*)(int))dlsym(RTLD_DEFAULT, "exit"),
	};

	if (!libpthread_funcs.exit)
	{
		DMibTraceSafe("No libpthread_funcs.exit found, malloc override not enabled!\n", 0);
		return false;
	}
	_libkernel_init(&libkernel_funcs, envp, apple, vars);
	bootstrap_init();
	__libplatform_init(NULL, envp, apple, vars);
	__pthread_init(&libpthread_funcs, envp, apple, vars);

	return true;
}

bool fg_MalterlibSystem_InitMacOS10110(void *_pPThreadInit, char const* envp[], char const* apple[], const struct ProgramVars * vars)
{
	// This path is taken on macOS 10.11

	struct _libpthread_functions {
		/* Structure version 1. Subsequent versions must only add pointers! */
		unsigned long version;
		void (*exit)(int);
		void* (*malloc)(size_t);
		void  (*free)(void*);
	};

	void (* __pthread_init)(const struct _libpthread_functions *libpthread_funcs, const char *envp[], const char *apple[], const struct ProgramVars *vars);
	(void * &)__pthread_init = _pPThreadInit;

	typedef const struct _libkernel_functions {
		/* Structure version 1. Subsequent versions must only add pointers! */
		unsigned long version;
		void* (*dlsym)(void*, const char*);
		void* (*malloc)(size_t);
		void  (*free)(void*);
		void* (*realloc)(void*, size_t);
		void  (*_pthread_exit_if_canceled)(int);
	} *_libkernel_functions_t;

	typedef const struct _libc_functions {
		/* Structure version 1. Subsequent versions must only add pointers! */
		unsigned long version;
		void* (*libSystem_atfork_prepare)();
		void* (*libSystem_atfork_parent)();
		void* (*libSystem_atfork_child)();
		void* (*_dirhelper)();
		void* (*mach_init_old)();
	} *_libc_functions_t;

	void (* _libkernel_init)(_libkernel_functions_t fns, const char *envp[], const char *apple[], const struct ProgramVars *vars);

	(void * &)_libkernel_init = dlsym(RTLD_DEFAULT, "__libkernel_init");

	if (!_libkernel_init)
		(void * &)_libkernel_init = dlsym(RTLD_DEFAULT, "_libkernel_init");

	if (!_libkernel_init)
	{
		DMibTraceSafe("No _libkernel_init found, malloc override not enabled!\n", 0);
		return false;
	}

	void (* _libc_initializer)(_libc_functions_t fns, const char *envp[], const char *apple[], const struct ProgramVars *vars);

	(void * &)_libc_initializer = dlsym(RTLD_DEFAULT, "_libc_initializer");

	if (!_libc_initializer)
		(void * &)_libc_initializer = dlsym(RTLD_DEFAULT, "libc_initializer");

	if (!_libc_initializer)
	{
		DMibTraceSafe("No _libkernel_init found, malloc override not enabled!\n", 0);
		return false;
	}



	static const struct _libkernel_functions libkernel_funcs = {
		.version = 3,
		.dlsym = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "dlsym")),
		.malloc = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "malloc")),
		.free = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "free")),
		.realloc = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "realloc")),
		._pthread_exit_if_canceled = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "_pthread_exit_if_canceled")),
	};

	if (!libkernel_funcs.dlsym)
	{
		DMibTraceSafe("No libkernel_funcs.dlsym found, malloc override not enabled!\n", 0);
		return false;
	}

	if (!libkernel_funcs.dlsym)
	{
		DMibTraceSafe("No libkernel_funcs.dlsym found, malloc override not enabled!\n", 0);
		return false;
	}

	void (*bootstrap_init)(void);
	(void * &)bootstrap_init = dlsym(RTLD_DEFAULT, "bootstrap_init");

	if (!bootstrap_init)
	{
		DMibTraceSafe("No bootstrap_init found, malloc override not enabled!\n", 0);
		return false;
	}

	void (* __libplatform_init)(void *future_use, const char *envp[], const char *apple[], const struct ProgramVars *vars);
	(void * &)__libplatform_init = dlsym(RTLD_DEFAULT, "__libplatform_init");

	if (!__libplatform_init)
	{
		DMibTraceSafe("No __libplatform_init found, malloc override not enabled!\n", 0);
		return false;
	}

	static const struct _libpthread_functions libpthread_funcs = {
		.version = 2,
		.exit = (void (*)(int))dlsym(RTLD_DEFAULT, "exit"),
		.malloc = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "malloc")),
		.free = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "free")),
	};

	if (!libpthread_funcs.exit)
	{
		DMibTraceSafe("No libpthread_funcs.exit found, malloc override not enabled!\n", 0);
		return false;
	}

	_libkernel_init(&libkernel_funcs, envp, apple, vars);
	__libplatform_init(NULL, envp, apple, vars);
	__pthread_init(&libpthread_funcs, envp, apple, vars);
	static const struct _libc_functions libc_funcs = {
		.version = 1,
		.libSystem_atfork_prepare = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "libSystem_atfork_prepare")),
		.libSystem_atfork_parent = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "libSystem_atfork_parent")),
		.libSystem_atfork_child = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "libSystem_atfork_child")),
		._dirhelper = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "_dirhelper")),
		.mach_init_old = NMib::fg_AutoCCast(dlsym(RTLD_DEFAULT, "mach_init")),
	};

	_libc_initializer(&libc_funcs, envp, apple, vars);

	return true;
}

#endif
