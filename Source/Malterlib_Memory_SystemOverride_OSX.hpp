// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include <Mib/Instrumentation/FunctionHook>

#include <malloc/malloc.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <mach/vm_map.h>
#include <mach/mach_host.h>
#include <mach/task.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/utsname.h>
#include <mach-o/dyld.h>

//#define DEmulateCrash
#define DOptimizeSetJmp

extern "C"
{
	extern void fg_MalterlibSystem_InitHelper() __attribute__((weak_import));
}

#ifdef DMibConfig_CheckOverrideMemoryLeaks
	constexpr NMib::EHeapDebugFlag gc_DebugFlags = NMib::EHeapDebugFlag_None;
#else
	constexpr NMib::EHeapDebugFlag gc_DebugFlags = NMib::EHeapDebugFlag_Ignore;
#endif

void fg_MalterlibMallocOverrideInit()
{
	if (fg_MalterlibSystem_InitHelper)
		fg_MalterlibSystem_InitHelper();
}

using namespace NMib::NMem;

void fg_MalterlibMallocOverrideEnable();

extern "C" int mach_init(void);

extern "C" 
{
	struct ProgramVars
	{
		const void*		mh;
		int*			NXArgcPtr;
		const char***	NXArgvPtr;
		const char***	environPtr;
		const char**	__prognamePtr;
	};
	extern void _mach_fork_child(void) __attribute__((weak_import));
}

namespace NMib
{
	namespace NSys
	{
		void fg_CreateSystemMalloc(bool _bProvideDestroySystem);
		void fg_MalterlibSystem_ForkPrepare();
		void fg_MalterlibSystem_ForkParent();
		void fg_MalterlibSystem_ForkChild();
		
		NAggregate::TCAggregateSimple<NInstrumentation::CMHook> g_FunctionHooks = {DAggregateInit};

		uint64 g_AllocMagic = 0;
		bool g_bAtExitCalled = false;
	}
}


void fg_MalterlibMallocOverride_AtExitCalled()
{
	NMib::NSys::g_bAtExitCalled = true;
}

namespace
{
	bool fg_MalterlibSystem_InitOSX1060();
	bool fg_MalterlibSystem_InitOSX1070(void *_pPThreadInit);
	bool fg_MalterlibSystem_InitOSX1090(void *_pPThreadInit, char const* envp[], char const* apple[], const ProgramVars * vars);
	bool fg_MalterlibSystem_InitOSX10100(void *_pPThreadInit, char const* envp[], char const* apple[], const ProgramVars * vars);
	bool fg_MalterlibSystem_InitOSX10110(void *_pPThreadInit, char const* envp[], char const* apple[], const ProgramVars * vars);
	
	void (*bootstrap_init)(void) = nullptr;

	class CAllocatorIgnore : public NMib::NMem::CAllocator_Heap
	{
	public:
		only_parameters_aliased return_not_aliased static void *f_AllocAligned(mint &_Size, mint _Alignment, NMib::EAllocationFlag _AllocFlags = NMib::EAllocationFlag_None, NMib::ENumaNode _NumaNode = NMib::ENumaNode_Default)
		{
			return NMib::NMem::CAllocator_Heap::f_AllocAlignedDebug(_Size, _Alignment, DMibPFile, DMibPLine, NMib::EHeapDebugFlag_Ignore, _AllocFlags, _NumaNode);
		}
	};
	
	struct CThreadLocal
	{
		zbool m_bReentrant;
		jmp_buf m_ReturnJump;
	};
	
	NMib::NAggregate::TCAggregateSimple<NMib::NThread::TCThreadLocal<CThreadLocal, CAllocatorIgnore>> g_ThreadLocal = {DAggregateInit};
	
}

extern "C"
{
	void * (* malloc_reenter)(size_t _Size) = nullptr;
	
	bool g_MalterlibMallocOveridden = false;
	bool g_MalterlibMallocOveriddenSuccess = false;


	void (* exit_reenter)(int) __attribute__((noreturn));
	
	void fg_MalterlibSystem_DestroyLate()
	{
		NMib::NSys::g_FunctionHooks->f_Suspend();
		if (malloc_reenter)
		{
			NMib::NSys::g_FunctionHooks->f_Unhook((void **)&malloc_reenter);
			malloc_reenter = nullptr;
		}
		NMib::NSys::g_FunctionHooks->f_Unhook((void **)&exit_reenter);
		NMib::NSys::g_FunctionHooks->f_Resume();
		
		NMib::NSys::g_FunctionHooks.f_Destruct();
		NMib::g_bMemoryManagerNeededAfterDestroy = true;
		NMib::NSys::fg_DestroySystem();
	}

	void *fg_MalterlibSystem_Hooked_Malloc(size_t _Size)
	{
		bool bClear = false;
		if (!g_MalterlibMallocOveridden)
		{
			g_MalterlibMallocOveridden = true;
			fg_MalterlibMallocOverrideEnable();
			if (malloc_reenter)
				bClear = true;
		}
		void *pRet = malloc_reenter(_Size);

		if (bClear)
		{
			NMib::NSys::g_FunctionHooks->f_Unhook((void **)&malloc_reenter); // Unhook malloc while assuming no other threads are calling in here simultaneously
			malloc_reenter = nullptr;
		}
		return pRet;
	}

	void fg_MalterlibSystem_Hooked_Exit(int _ExitCode)
	{
		if (NMib::NSys::g_bAtExitCalled) // If atexit has not been called this is an abnormal exit so don't try to do any cleanup
			fg_MalterlibSystem_DestroyLate();
		return exit_reenter(_ExitCode);
	}

	assure_used module_export void fg_MalterlibSystem_InitEarly(int argc, char const* argv[], char const* envp[], char const* apple[], const ProgramVars * vars)
	{
#ifdef DArchitecture_x86
		// Not debugged for this arch
		return;
#endif
		
		if (NMib::NSys::fg_System_BeingDebugged())
			return;
		
		if (envp)
		{
			for (mint i = 0; envp[i]; ++i)
			{
				if (NMib::NStr::fg_StrStartsWith(envp[i], "DYLD_INSERT_LIBRARIES"))
				{
					//DMibTraceSafe("Malloc override disabled because of: {}\n", envp[i]);
					return;
				}
				 
				if (NMib::NStr::fg_StrStartsWith(envp[i], "MalterlibMallocOverrideDisable"))
				{
					//DMibTraceSafe("Malloc override disabled because of: MalterlibMallocOverrideDisable\n", 0);
					return;
				}
			}
		}
		
		auto nImages = _dyld_image_count();
		for (auto i = 0; i < nImages; ++i)
		{
			auto pName = _dyld_get_image_name(i);
			//DMibTraceSafe("Image: {}\n", pName);
			if (NMib::NStr::fg_StrFindNoCase(pName, "/libBacktraceRecording.dylib") >= 0)
			{
				//DMibTraceSafe("Malloc override disabled because of: libBacktraceRecording.dylib\n", 0);
				return;
			}
			if (NMib::NStr::fg_StrFindNoCase(pName, "/libsimshim.dylib") >= 0)
			{
				//DMibTraceSafe("Malloc override disabled because of: libsimshim.dylib\n", 0);
				return;
			}
		}
		
		if (NMib::NSys::fg_System_BeingDebugged())
			return;
		
		struct utsname un;
		int Res = uname(&un);
		int Major = 0;
		if (Res >= 0)
		{
			NMib::NStr::CFStr256 VersionString;
			VersionString = un.release; 
			int Minor = 0;
			int Fix = 0;

			Major = fg_GetStrSep(VersionString, ".").f_ToInt(0);
			Minor = fg_GetStrSep(VersionString, ".").f_ToInt(0);
			Fix = fg_GetStrSep(VersionString, ".").f_ToInt(0);
			
			if (Major > 15)
				return; // Don't try to override on OSX that we don't yet know about as this is likely to fail
		}
		
 		
		(void * &)malloc_reenter = dlsym(RTLD_DEFAULT, "malloc");
		
		if (!malloc_reenter)
		{
			DMibTraceSafe("No malloc found, malloc override not enabled!\n", 0);
			return;
		}
		
		(void * &)exit_reenter = dlsym(RTLD_DEFAULT, "__exit");
		
		if (!exit_reenter)
			(void * &)exit_reenter = dlsym(RTLD_DEFAULT, "_exit");
		
		if (!exit_reenter)
		{
			DMibTraceSafe("No __exit found, malloc override not enabled!\n", 0);
			return;
		}
		
		void *pthread_init = dlsym(RTLD_DEFAULT, "__pthread_init");
		
		if (pthread_init)
		{
			if (Major == 15)
			{
				if (!fg_MalterlibSystem_InitOSX10110(pthread_init, envp, apple, vars))
					return;
			}
			else if (Major == 14)
			{
				if (!fg_MalterlibSystem_InitOSX10100(pthread_init, envp, apple, vars))
					return;
			}
			else
			{
				if (!fg_MalterlibSystem_InitOSX1090(pthread_init, envp, apple, vars))
					return;
			}
		}
		else
		{
			pthread_init = dlsym(RTLD_DEFAULT, "pthread_init");
			
			if (pthread_init)
			{
				if (!fg_MalterlibSystem_InitOSX1070(pthread_init))
					return;
			}
			else
			{
				if (!fg_MalterlibSystem_InitOSX1060())
					return;
			}
		}

		NMib::NSys::fg_CreateSystemMalloc(true);

		NMib::NSys::g_AllocMagic = NMib::NMisc::fg_GetHighEntropyRandomInteger<uint64>();
		g_ThreadLocal.f_Construct();
		
		NMib::NSys::g_FunctionHooks.f_Construct();

		g_MalterlibMallocOveriddenSuccess = true;
		
		NMib::NSys::g_FunctionHooks->f_Suspend();
		if (!NMib::NSys::g_FunctionHooks->f_SetHook((void **)&malloc_reenter, (void *)&fg_MalterlibSystem_Hooked_Malloc))
		{
			DMibTraceSafe("Failed to hook malloc, aborting!\n", 0);
			DMibPDebugBreak;
		}
		if (!NMib::NSys::g_FunctionHooks->f_SetHook((void **)&exit_reenter, (void *)&fg_MalterlibSystem_Hooked_Exit))
		{
			DMibTraceSafe("Failed to hook exit, aborting!\n", 0);
			DMibPDebugBreak;
		}

		NMib::NSys::g_FunctionHooks->f_Resume();
	}

}

typedef struct _malloc_zone_t_10_7 {
    /* Only zone implementors should depend on the layout of this structure;
	 Regular callers should use the access functions below */
    void	*reserved1;	/* RESERVED FOR CFAllocator DO NOT USE */
    void	*reserved2;	/* RESERVED FOR CFAllocator DO NOT USE */
    size_t 	(*size)(struct _malloc_zone_t *zone, const void *ptr); /* returns the size of a block or 0 if not in this zone; must be fast, especially for negative answers */
    void 	*(*malloc)(struct _malloc_zone_t *zone, size_t size);
    void 	*(*calloc)(struct _malloc_zone_t *zone, size_t num_items, size_t size); /* same as malloc, but block returned is set to zero */
    void 	*(*valloc)(struct _malloc_zone_t *zone, size_t size); /* same as malloc, but block returned is set to zero and is guaranteed to be page aligned */
    void 	(*free)(struct _malloc_zone_t *zone, void *ptr);
    void 	*(*realloc)(struct _malloc_zone_t *zone, void *ptr, size_t size);
    void 	(*destroy)(struct _malloc_zone_t *zone); /* zone is destroyed and all memory reclaimed */
    const char	*zone_name;
	
    /* Optional batch callbacks; these may be NULL */
    unsigned	(*batch_malloc)(struct _malloc_zone_t *zone, size_t size, void **results, unsigned num_requested); /* given a size, returns pointers capable of holding that size; returns the number of pointers allocated (maybe 0 or less than num_requested) */
    void	(*batch_free)(struct _malloc_zone_t *zone, void **to_be_freed, unsigned num_to_be_freed); /* frees all the pointers in to_be_freed; note that to_be_freed may be overwritten during the process */
	
    struct malloc_introspection_t	*introspect;
    unsigned	version;
    
    /* aligned memory allocation. The callback may be NULL. Present in version >= 5. */
    void *(*memalign)(struct _malloc_zone_t *zone, size_t alignment, size_t size);
    
    /* free a pointer known to be in zone and known to have the given size. The callback may be NULL. Present in version >= 6.*/
    void (*free_definite_size)(struct _malloc_zone_t *zone, void *ptr, size_t size);
	
    /* Empty out caches in the face of memory pressure. The callback may be NULL. Present in version >= 8. */
    size_t 	(*pressure_relief)(struct _malloc_zone_t *zone, size_t goal);
} malloc_zone_t_10_7;

malloc_zone_t_10_7 g_OriginalMallocs;
malloc_zone_t_10_7 *g_pDefaultZone = nullptr;

#define DMibOSXOverrideZoneCheck(_Z) DMibFastCheck((_malloc_zone_t_10_7 *)_Z == g_pDefaultZone)


extern "C" unsigned malloc_num_zones;
extern "C" unsigned malloc_num_zones_allocated;

struct sigaction g_OldSignalHandlerBus;
struct sigaction g_OldSignalHandlerSegv;

void fg_HandleCrashSignalBus(int signal)
{
	auto pThreadLocal = g_ThreadLocal->f_TryGet();
	if (pThreadLocal && pThreadLocal->m_bReentrant)
	{
#ifdef DOptimizeSetJmp
		// Unblock this signal so it's called again the next time
		sigset_t ToUnblock;
		sigemptyset (&ToUnblock);
		sigaddset(&ToUnblock, signal);
		sigprocmask(SIG_UNBLOCK, &ToUnblock, nullptr);
		_longjmp(pThreadLocal->m_ReturnJump, 1);
#else
		longjmp(pThreadLocal->m_ReturnJump, 1);
#endif
	}

	struct sigaction Old;
	sigaction(signal, &g_OldSignalHandlerBus, &Old);
	raise(signal);
}

void fg_HandleCrashSignalSegv(int signal)
{
	auto pThreadLocal = g_ThreadLocal->f_TryGet();
	if (pThreadLocal && pThreadLocal->m_bReentrant)
	{
#ifdef DOptimizeSetJmp
		// Unblock this signal so it's called again the next time
		sigset_t ToUnblock;
		sigemptyset (&ToUnblock);
		sigaddset(&ToUnblock, signal);
		sigprocmask(SIG_UNBLOCK, &ToUnblock, nullptr);
		_longjmp(pThreadLocal->m_ReturnJump, 1);
#else
		longjmp(pThreadLocal->m_ReturnJump, 1);
#endif
	}

	struct sigaction Old;
	sigaction(signal, &g_OldSignalHandlerBus, &Old);
	raise(signal);
}

#ifdef DEmulateCrash
mint g_bInstalled = 0;
#endif

void fg_MalterlibMallocOverrideInit_ReinstallHandler()
{
	struct sigaction NewHandler;
	fg_MemClear(NewHandler);
	NewHandler.sa_handler = fg_HandleCrashSignalBus;
	NewHandler.sa_flags = 0;
	
	sigaction(SIGBUS, &NewHandler, &g_OldSignalHandlerBus);

	NewHandler.sa_handler = fg_HandleCrashSignalSegv;
	sigaction(SIGSEGV, &NewHandler, &g_OldSignalHandlerSegv);
#ifdef DEmulateCrash
	g_bInstalled = NMib::NSys::fg_Thread_GetCurrentUID();
#endif
}

extern "C"
{
	assure_used module_export bool breakpad_should_handle_exception(pthread_t _pThread)
	{
		if (!g_MalterlibMallocOveriddenSuccess)
			return true;
		
		auto *pThreadLocal = g_ThreadLocal->f_TryGetForThread((mint)_pThread);
		if (pThreadLocal)
			return !pThreadLocal->m_bReentrant;
		else
			return true;
	}
}


size_t fg_Malterlib_size(struct _malloc_zone_t *zone, const void *ptr) /* returns the size of a block or 0 if not in this zone; must be fast, especially for negative answers */
{
	if (!ptr)
		return 0;
	
	uint8 *pMalterlibAlloc = (uint8 *)ptr;
	
	auto &ThreadLocal = **g_ThreadLocal;
	DMibFastCheck(!ThreadLocal.m_bReentrant);
	ThreadLocal.m_bReentrant = true;
	auto Cleanup = NMib::fg_OnScopeExit
		(
			[&]()
			{
				ThreadLocal.m_bReentrant = false;
			}
		)
	;
	
#ifdef DEmulateCrash
	if (g_bInstalled == NMib::NSys::fg_Thread_GetCurrentUID())
	{
		DConOut("Will crash({}) = {}\n", (void *)NMib::NSys::fg_Thread_GetCurrentUID() << &ThreadLocal);
		pMalterlibAlloc = nullptr;
	}
#endif
	
#ifdef DOptimizeSetJmp
	if (_setjmp(ThreadLocal.m_ReturnJump))
#else
	if (setjmp(ThreadLocal.m_ReturnJump))
#endif
	{
#ifdef DEmulateCrash
		pMalterlibAlloc = (uint8 *)ptr;
#else
		return 0; // Access violation accessing header, this is not our block
#endif
	}
	
	return NMib::NMem::fg_TrySize(pMalterlibAlloc);
}

void fg_Malterlib_free(struct _malloc_zone_t *zone, void *ptr)
{
	if (!ptr)
		return;
	DMibOSXOverrideZoneCheck(zone);
	uint8 *pMalterlibAlloc = (uint8 *)ptr;
	return NMib::NMem::fg_Free(pMalterlibAlloc);
}

void *fg_Malterlib_malloc(struct _malloc_zone_t *zone, size_t size)
{
	DMibOSXOverrideZoneCheck(zone);

	mint Size = NMib::fg_AlignUp(NMib::fg_Max(size, 1), 16);
	
#if DMibConfig_MalterlibMemoryManager_Debug
	uint8 *pMalterlibAlloc = (uint8 *)NMib::NMem::fg_AllocDebug(Size, DMibPFile, DMibPLine, gc_DebugFlags);
#else
	uint8 *pMalterlibAlloc = (uint8 *)NMib::NMem::fg_Alloc(Size);
#endif
	
	return pMalterlibAlloc;
}
void *fg_Malterlib_calloc(struct _malloc_zone_t *zone, size_t num_items, size_t size) /* same as malloc, but block returned is set to zero */
{
	DMibOSXOverrideZoneCheck(zone);
	mint Size = NMib::fg_AlignUp(NMib::fg_Max(size * num_items, 1), 16);
#if DMibConfig_MalterlibMemoryManager_Debug
	uint8 *pMalterlibAlloc = (uint8 *)NMib::NMem::fg_AllocDebug(Size, DMibPFile, DMibPLine, gc_DebugFlags);
#else
	uint8 *pMalterlibAlloc = (uint8 *)NMib::NMem::fg_Alloc(Size);
#endif
	fg_MemClear(pMalterlibAlloc, Size);
	return pMalterlibAlloc;
}
/* aligned memory allocation. The callback may be NULL. Present in version >= 5. */
void *fg_Malterlib_memalign(struct _malloc_zone_t *zone, size_t alignment, size_t size)
{
	DMibOSXOverrideZoneCheck(zone);

	mint Size = NMib::fg_AlignUp(NMib::fg_Max(size, 1), alignment);
#if DMibConfig_MalterlibMemoryManager_Debug
	uint8 *pMalterlibAlloc = (uint8 *)NMib::NMem::fg_AllocAlignedDebug(Size, alignment, DMibPFile, DMibPLine, gc_DebugFlags);
#else
	uint8 *pMalterlibAlloc = (uint8 *)NMib::NMem::fg_AllocAligned(Size, alignment);
#endif

	return pMalterlibAlloc;
}


void *fg_Malterlib_valloc(struct _malloc_zone_t *zone, size_t size) /* same as malloc, but block returned is set to zero and is guaranteed to be page aligned */
{
	DMibOSXOverrideZoneCheck(zone);

	mint Size = NMib::fg_AlignUp(NMib::fg_Max(size, 1), NMib::NSys::NPrivate::g_PageSize);
#if DMibConfig_MalterlibMemoryManager_Debug
	uint8 *pMalterlibAlloc = (uint8 *)NMib::NMem::fg_AllocAlignedDebug(Size, NMib::NSys::NPrivate::g_PageSize, DMibPFile, DMibPLine, gc_DebugFlags);
#else
	uint8 *pMalterlibAlloc = (uint8 *)NMib::NMem::fg_AllocAligned(Size, NMib::NSys::NPrivate::g_PageSize);
#endif

	fg_MemClear(pMalterlibAlloc, Size);
	return pMalterlibAlloc;
}
void *fg_Malterlib_realloc(struct _malloc_zone_t *zone, void *ptr, size_t size)
{
	DMibOSXOverrideZoneCheck(zone);

	uint8 *pMalterlibAlloc = (uint8 *)ptr;

	mint Size = NMib::fg_AlignUp(NMib::fg_Max(size, 1), 16);
	
#if DMibConfig_MalterlibMemoryManager_Debug
	pMalterlibAlloc = (uint8 *)NMib::NMem::fg_ResizeDebug(pMalterlibAlloc, Size, DMibPFile, DMibPLine, gc_DebugFlags);
#else
	pMalterlibAlloc = (uint8 *)NMib::NMem::fg_Resize(pMalterlibAlloc, Size);
#endif
	return pMalterlibAlloc;
}
void fg_Malterlib_destroy(struct _malloc_zone_t *zone) /* zone is destroyed and all memory reclaimed */
{
	DMibPDebugBreak; // Sholud not get here
}

void fg_MalterlibMallocOverrideEnable()
{
	fg_MalterlibMallocOverrideInit_ReinstallHandler();

	malloc_zone_t_10_7 *pDefaultZone = (malloc_zone_t_10_7 *)malloc_default_zone();
	g_pDefaultZone = pDefaultZone;
	
	g_OriginalMallocs = *pDefaultZone;
	
	if (pDefaultZone->version >= 8)
		vm_protect(mach_task_self(), (uintptr_t)pDefaultZone, sizeof(malloc_zone_t_10_7), 0, VM_PROT_READ | VM_PROT_WRITE);//remove the write protection
	
    pDefaultZone->size = &fg_Malterlib_size;
    pDefaultZone->malloc = fg_Malterlib_malloc;
    pDefaultZone->calloc = fg_Malterlib_calloc;
    pDefaultZone->valloc = fg_Malterlib_valloc;
    pDefaultZone->free = fg_Malterlib_free;
    pDefaultZone->realloc = fg_Malterlib_realloc;
    pDefaultZone->destroy = fg_Malterlib_destroy;
	
	pDefaultZone->batch_malloc = nullptr;
    pDefaultZone->batch_free = nullptr;
	if (pDefaultZone->version >= 6)
		pDefaultZone->free_definite_size = nullptr;
	if (pDefaultZone->version >= 8)
		pDefaultZone->pressure_relief = nullptr;

    /* aligned memory allocation. The callback may be NULL. Present in version >= 5. */
	if (pDefaultZone->version >= 5)
		pDefaultZone->memalign = &fg_Malterlib_memalign;
	
	if (pDefaultZone->version >= 8)
		vm_protect(mach_task_self(), (uintptr_t)pDefaultZone, sizeof(malloc_zone_t_10_7), 0, VM_PROT_READ);//put the write protection back
	
	pthread_atfork(&NMib::NSys::fg_MalterlibSystem_ForkPrepare, &NMib::NSys::fg_MalterlibSystem_ForkParent, &NMib::NSys::fg_MalterlibSystem_ForkChild);
	
}


void fg_MalterlibMallocOverrideDisable()
{
	if (!g_pDefaultZone)
		return;
	malloc_zone_t_10_7 *pDefaultZone = g_pDefaultZone;
	g_pDefaultZone = nullptr;
	if (pDefaultZone->version >= 8)
		vm_protect(mach_task_self(), (uintptr_t)pDefaultZone, sizeof(malloc_zone_t_10_7), 0, VM_PROT_READ | VM_PROT_WRITE);//remove the write protection
	
	pDefaultZone->size = g_OriginalMallocs.size;
    pDefaultZone->malloc = g_OriginalMallocs.malloc;
    pDefaultZone->calloc = g_OriginalMallocs.calloc;
    pDefaultZone->valloc = g_OriginalMallocs.valloc;
    pDefaultZone->free = g_OriginalMallocs.free;
    pDefaultZone->realloc = g_OriginalMallocs.realloc;
    pDefaultZone->destroy = g_OriginalMallocs.destroy;

	pDefaultZone->batch_malloc = g_OriginalMallocs.batch_malloc;
    pDefaultZone->batch_free = g_OriginalMallocs.batch_free;
	if (pDefaultZone->version >= 6)
		pDefaultZone->free_definite_size = g_OriginalMallocs.free_definite_size;
	if (pDefaultZone->version >= 8)
		pDefaultZone->pressure_relief = g_OriginalMallocs.pressure_relief;

    /* aligned memory allocation. The callback may be NULL. Present in version >= 5. */
	if (pDefaultZone->version >= 5)
		pDefaultZone->memalign = g_OriginalMallocs.memalign;

	if (pDefaultZone->version >= 8)
		vm_protect(mach_task_self(), (uintptr_t)pDefaultZone, sizeof(malloc_zone_t_10_7), 0, VM_PROT_READ);//put the write protection back
	
}


namespace
{
	bool fg_MalterlibSystem_InitOSX1060()
	{
		void *pLibSystemInfo = dlsym(RTLD_DEFAULT, "__Libsystem_version");
		
		if (!pLibSystemInfo)
		{
			DMibTraceSafe("No __Libsystem_version found, malloc override not enabled!\n", 0);
			return false;
		}
		
		// This path is taken os OXS 10.6
		
		Dl_info Info;
		if (!dladdr((void *)(mint)pLibSystemInfo, &Info))
		{
			DMibTraceSafe("No dladdr for __Libsystem_version found, malloc override not enabled!\n", 0);
			return false;
		}

		void (* pthread_init)() = nullptr;

		uint8 *pStartAddress = (uint8 *)Info.dli_fbase;
		uint8 *pEndAddress = pStartAddress + 8*1024; // Go through 8 KB at max
		mint nLoop = 0;
		while (pStartAddress < pEndAddress)
		{	
			++nLoop;
			Dl_info Info;
			if (dladdr((void *)(mint)pStartAddress, &Info))
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
	
	bool fg_MalterlibSystem_InitOSX1070(void *_pPThreadInit)
	{
		void (* pthread_init)();
		(void * &)pthread_init = _pPThreadInit;
		
		// This code path is taken on OSX 10.7 and 10.8
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

	bool fg_MalterlibSystem_InitOSX10100(void *_pPThreadInit, char const* envp[], char const* apple[], const struct ProgramVars * vars)
	{
		// This path is taken on OSX 10.10

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

	bool fg_MalterlibSystem_InitOSX10110(void *_pPThreadInit, char const* envp[], char const* apple[], const struct ProgramVars * vars)
	{
		// This path is taken on OSX 10.11

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
	
	bool fg_MalterlibSystem_InitOSX1090(void *_pPThreadInit, char const* envp[], char const* apple[], const struct ProgramVars * vars)
	{
		// This path is taken on OSX 10.9

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
}
