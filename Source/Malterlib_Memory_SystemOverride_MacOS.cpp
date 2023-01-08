// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#if defined(DMibConfig_OverrideSystemMalloc) || defined(DMibMemoryOverrideDll)

#include <Mib/Core/Core>
#include <Mib/Core/System>
#include <Mib/Instrumentation/FunctionHook>
#include <Mib/Memory/Pool>

#include <malloc/malloc.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <dlfcn.h>
#include <sys/utsname.h>
#include <sys/sysctl.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <mach/thread_status.h>
#include <mach/thread_state.h>
#include <mach/exc.h>
#include <string.h>
#include <errno.h>

#include "Malterlib_Memory_SystemOverride_MacOSInterpose.h"
#include "Malterlib_Memory_SystemOverride_MacOSMallocZone.h"

#include "Malterlib_Memory_SystemManager_Malterlib.h"

#if defined(DMibConfig_MemoryManager_UseMalterlib)
#	define DMemoryManagerIsSame
namespace NMib
{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	extern bool g_bMainHeapIsSmall;
	extern NMib::NStorage::TCAggregateSimple<CMemoryManagerSmall> g_MainHeapSmall;
#endif
	extern NMib::NStorage::TCAggregateSimple<CMemoryManagerMax> g_MainHeapMax;
	extern bool g_bMainHeapConstructed;
#if DMibEnableSafeCheck > 0
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	static auto &fg_MainHeapSmall()
	{
		DMibFastCheck(g_bMainHeapConstructed);
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		DMibFastCheck(g_bMainHeapIsSmall);
#endif
		return g_MainHeapSmall;
	}
	#define DMainHeapSmall fg_MainHeapSmall()
#endif

	static auto &fg_MainHeapMax()
	{
		DMibFastCheck(g_bMainHeapConstructed);
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		DMibFastCheck(!g_bMainHeapIsSmall);
#endif
		return g_MainHeapMax;
	}
	#define DMainHeapMax fg_MainHeapMax()
#else
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	#define DMainHeapSmall g_MainHeapSmall
#endif
	#define DMainHeapMax g_MainHeapMax
#endif

}
#endif

#define DMibOverrideErrorOutput(...) NMib::NSys::fg_DebugOutput((NMib::NStr::fg_Format<NMib::NStr::CFStr256>(__VA_ARGS__)).f_GetStr())

#if 0
#define DMibOverrideTrace DMibOverrideErrorOutput
#else
#define DMibOverrideTrace(...) (void)0
#endif

#if defined(DMibMemoryOverrideDll)
#define DMibMalterlibOverrideMallocExport
#else
#define DMibMalterlibOverrideMallocExport module_export
#endif

#if DMibConfig_MalterlibMemoryManager_Debug && DMibConfig_MalterlibMemoryManager_Debug_Features || DMibConfig_MalterlibMemoryManager_Debug_Features == 1
#	define DEnableDebugMemoryManager 1
#else
#	define DEnableDebugMemoryManager 0
#endif

extern "C" unsigned malloc_num_zones;

using namespace NMib;
using namespace NMib::NMemory;

//#define DEmulateCrash
#define DOptimizeSetJmp
#define DFullArenasForSecondary

extern "C"
{
#ifdef DMibMemoryOverrideDll
	extern COriginalFunctions g_OriginalFunctions;
#else
	COriginalFunctions g_OriginalFunctions;
#endif

	extern void fg_MalterlibSystem_InitHelper() __attribute__((weak_import));
}

EHeapDebugFlag g_DebugFlags = EHeapDebugFlag_None;

void fg_MalterlibMallocOverrideInit()
{
	if (fg_MalterlibSystem_InitHelper)
		fg_MalterlibSystem_InitHelper();
}

void fg_MalterlibMallocOverrideEnable();
void fg_MalterlibMallocOverrideDisable();

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

	// These are here so dynamic linking works on OSes where these are not defined
#if !defined(DMibMemoryOverrideDll)
	assure_used module_export void __attribute__((weak)) __malloc_init(const char *apple[])
	{
	}
	assure_used module_export void __attribute__((weak)) __libc_init(const struct ProgramVars *vars, void (*atfork_prepare)(void),void (*atfork_parent)(void),void (*atfork_child)(void),const char *apple[])
	{
	}
#endif
}

namespace NMib
{
	namespace NSys
	{
		void fg_CreateSystemMalloc(bool _bProvideDestroySystem);
		void fg_CreateSystemVersion();
		void fg_MalterlibSystem_ForkPrepare();
		void fg_MalterlibSystem_ForkParent();
		void fg_MalterlibSystem_ForkChild();

#ifdef DMalterlibMemoryOverrideMacOSInitBeforeLibSystemSupport
		constinit NStorage::TCAggregateSimple<NInstrumentation::CMHook> g_FunctionHooks = {DAggregateInit};
#endif

		bool g_bAtExitCalled = false;
	}
}


void fg_MalterlibMallocOverride_AtExitCalled()
{
	NSys::g_bAtExitCalled = true;
}

#ifdef DMalterlibMemoryOverrideMacOSInitBeforeLibSystemSupport
bool fg_MalterlibSystem_InitMacOS1060();
bool fg_MalterlibSystem_InitMacOS1070(void *_pPThreadInit);
bool fg_MalterlibSystem_InitMacOS1090(void *_pPThreadInit, char const* envp[], char const* apple[], const ProgramVars * vars);
bool fg_MalterlibSystem_InitMacOS10100(void *_pPThreadInit, char const* envp[], char const* apple[], const ProgramVars * vars);
bool fg_MalterlibSystem_InitMacOS10110(void *_pPThreadInit, char const* envp[], char const* apple[], const ProgramVars * vars);
#endif

namespace
{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	struct CMemoryManagerZoneSmall
	{
		CMemoryManagerZoneSmall(CMemoryManagerConfig const &_Config)
#if DMibConfig_Memory_Shims_Enable
			: m_MemoryManager("macOS Zone", _Config)
#else
			: m_MemoryManager(_Config)
#endif
		{
		}

		malloc_zone_t_10_7 m_MallocZone;
		CMemoryManagerSmall m_MemoryManager;
		DMibListLinkDS_Link(CMemoryManagerZoneSmall, m_Link);
		malloc_zone_t *f_GetMallocZone()
		{
			return (malloc_zone_t *)(&m_MallocZone);
		}
	};

	static_assert(__builtin_offsetof(CMemoryManagerZoneSmall, m_MallocZone) == 0);
#endif

	struct CMemoryManagerZoneMax
	{
		CMemoryManagerZoneMax(CMemoryManagerConfig const &_Config)
#if DMibConfig_Memory_Shims_Enable
			: m_MemoryManager("macOS Zone", _Config)
#else
			: m_MemoryManager(_Config)
#endif
		{
		}

		malloc_zone_t_10_7 m_MallocZone;
		CMemoryManagerMax m_MemoryManager;
		DMibListLinkDS_Link(CMemoryManagerZoneMax, m_Link);
		malloc_zone_t *f_GetMallocZone()
		{
			return (malloc_zone_t *)(&m_MallocZone);
		}
	};

	static_assert(__builtin_offsetof(CMemoryManagerZoneMax, m_MallocZone) == 0);

	struct CLowLevelGlobalState
	{
		CLowLevelGlobalState()
		{
			m_iThreadLocal = NSys::fg_Thread_AllocLocal();
			m_iThreadLocalReentrant = NSys::fg_Thread_AllocLocal();
		}

		~CLowLevelGlobalState()
		{
			NSys::fg_Thread_FreeLocal(m_iThreadLocal);
			NSys::fg_Thread_FreeLocal(m_iThreadLocalReentrant);
		}

		jmp_buf *f_GetJumpBuffer()
		{
			return (jmp_buf *)NSys::fg_Thread_GetLocal(m_iThreadLocal);
		}

		void f_SetJumpBuffer(jmp_buf *_pBuffer)
		{
			return NSys::fg_Thread_SetLocal(m_iThreadLocal, _pBuffer);
		}

		mint f_GetRentrant()
		{
			return (mint)NSys::fg_Thread_GetLocal(m_iThreadLocalReentrant);
		}

		void f_IncRentrant()
		{
			NSys::fg_Thread_SetLocal(m_iThreadLocalReentrant, (void *)(f_GetRentrant() + 1));
		}

		void f_DecRentrant()
		{
			NSys::fg_Thread_SetLocal(m_iThreadLocalReentrant, (void *)(f_GetRentrant() - 1));
		}

		mint m_iThreadLocal;
		mint m_iThreadLocalReentrant;
	};

	constinit NStorage::TCAggregateSimple<CLowLevelGlobalState> g_LowLevelGlobalState = {DAggregateInit};

	struct CExceptionParameters
	{
		mach_msg_type_number_t m_nTypes = 0;
		exception_mask_t m_Masks[EXC_TYPES_COUNT];
		mach_port_t m_Ports[EXC_TYPES_COUNT];
		exception_behavior_t m_Behaviors[EXC_TYPES_COUNT];
		thread_state_flavor_t m_Flavors[EXC_TYPES_COUNT];
	};

#ifdef  __MigPackStructs
#pragma pack(4)
#endif

	struct CExceptionMessage
	{
		mach_msg_header_t m_Header;
		/* start of the kernel processed data */
		mach_msg_body_t m_Body;
		mach_msg_port_descriptor_t m_Thread;
		mach_msg_port_descriptor_t m_Task;
		/* end of the kernel processed data */
		NDR_record_t m_NdrRecord;
		exception_type_t m_Exception;
		mach_msg_type_number_t m_CodeCnt;
		int64_t m_Code[2];
		int m_Flavor;
		mach_msg_type_number_t m_OldStateCnt;
		natural_t old_state[1296];
	};

	struct CExceptionReplyMessage
	{
		mach_msg_header_t m_Header;
		NDR_record_t m_NdrRecord;
		kern_return_t m_ReturnCode;
		int m_Flavor;
		mach_msg_type_number_t m_NewStateCnt;
		natural_t m_NewState[1296];
	};

#ifdef  __MigPackStructs
#pragma pack()
#endif

	struct CExceptionHandlingState
	{
		~CExceptionHandlingState();

		void f_Abort();
		bool f_InstallHandler();
		bool f_UnInstallHandler();
		void f_HandleMessages();

		mach_port_t m_ExceptionHandlingPort = 0;
		CExceptionParameters m_PreviousParams;
		NThread::CEvent m_InstalledEvent;
	};

	struct CGlobalState
	{
		NThread::CMutualManyRead m_ZoneListLock;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		DMibListLinkDS_List(CMemoryManagerZoneSmall, m_Link) m_ZoneListSmall;
#endif
		DMibListLinkDS_List(CMemoryManagerZoneMax, m_Link) m_ZoneListMax;
		NContainer::TCVector<malloc_zone_t *> m_ForeignZones;
		malloc_zone_t **m_pForeignZones = nullptr;
		mint m_nForeignZones = 0;
#ifdef DMemoryManagerIsSame
		NMib::NThread::CMutualAggregate m_ForkLock;
		mint m_ForkedCount = 0;
		bool m_Unforked = false;
		sigset_t m_ForkSigMask;
#endif

		NStorage::TCSharedPointer<CExceptionHandlingState> m_pExceptionHandlingState;
		NStorage::TCUniquePointer<NThread::CThreadObject> m_pExceptionHandlingThread;
	};

	constinit NStorage::TCAggregateSimple<CGlobalState> g_GlobalState = {DAggregateInit};
}

namespace
{
	uint32 g_RunningUnderRosetta = 2;

	inline_never bool fg_RunningUnderRosettaUpdate()
	{
		int RunningUnderRosetta = 0;
		size_t Size = sizeof(RunningUnderRosetta);
		sysctlbyname("sysctl.proc_translated", &RunningUnderRosetta, &Size, nullptr, 0);
		g_RunningUnderRosetta = !!RunningUnderRosetta;

		return !!g_RunningUnderRosetta;
	}

	inline_always bool fg_RunningUnderRosetta()
	{
		return false;

		if (g_RunningUnderRosetta < 2)
			return !!g_RunningUnderRosetta;

		return fg_RunningUnderRosettaUpdate();
	}
}

extern "C"
{
#ifdef DMalterlibMemoryOverrideMacOSInitBeforeLibSystemSupport
	void * (* malloc_reenter)(size_t _Size) = nullptr;
#endif

	bool g_MalterlibMallocOveridden = false;
	bool g_MalterlibMallocOveriddenInstalled = false;
	bool g_MalterlibMallocOveriddenInterposersInstalled = false;

	bool g_MalterlibMallocBeforeMallocCalled = false;
	bool g_MalterlibMallocAfterMallocCalled = false;

	mint g_nMallocZonesBeforeMallocInit = 0;
	mint g_nMallocZonesAfterMallocInit = 0;

#ifdef DMalterlibMemoryOverrideMacOSInitBeforeLibSystemSupport
	void fg_InterposeOverride();
	void fg_InterposeOverrideUnhook();
#endif

	void fg_MalterlibSystem_DestroyLate()
	{
		g_bMemoryManagerNeededAfterDestroy = true;
		fg_MalterlibMallocOverrideDisable();
		NSys::fg_DestroySystem();
	}
}

void fg_ExitReenter(int _Value)
{
	if (g_OriginalFunctions.__exit)
		return g_OriginalFunctions.__exit(_Value);
	else if (g_OriginalFunctions._exit)
		return g_OriginalFunctions._exit(_Value);
}


void fg_MalterlibMallocOverride_PreDestroyNonTrackedMemoryManager()
{
	if (!g_MalterlibMallocOveriddenInterposersInstalled)
		return;
#ifdef DMalterlibMemoryOverrideMacOSInitBeforeLibSystemSupport
	NSys::g_FunctionHooks->f_Suspend();
	if (CSystem::ms_PlatformVersion < 10'11'00)
	{
		if (malloc_reenter)
		{
			NSys::g_FunctionHooks->f_Unhook((void **)&malloc_reenter);
			malloc_reenter = nullptr;
		}
	}
	if (!g_bMemoryManagerNeededAfterDestroy)
		fg_InterposeOverrideUnhook();
	NSys::g_FunctionHooks->f_Resume();
	if (!g_bMemoryManagerNeededAfterDestroy)
		NSys::g_FunctionHooks.f_Destruct();
#endif
}

extern "C"
{
#ifdef DMalterlibMemoryOverrideMacOSInitBeforeLibSystemSupport
	void *fg_MalterlibSystem_Hooked_Malloc(size_t _Size)
	{
		DMibOverrideTrace("fg_MalterlibSystem_Hooked_Malloc!\n");
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
			NSys::g_FunctionHooks->f_Unhook((void **)&malloc_reenter); // Unhook malloc while assuming no other threads are calling in here simultaneously
			malloc_reenter = nullptr;
		}
		return pRet;
	}
#endif

	void fg_MalterlibSystem_Hooked_Exit(int _ExitCode)
	{
		if (NSys::g_bAtExitCalled) // If atexit has not been called this is an abnormal exit so don't try to do any cleanup
			fg_MalterlibSystem_DestroyLate();
		return fg_ExitReenter(_ExitCode);
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib___exit(int _ExitCode)
	{
		return fg_MalterlibSystem_Hooked_Exit(_ExitCode);
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__exit(int _ExitCode)
	{
		fg_MalterlibSystem_Hooked_Exit(_ExitCode);
	}

	bool fg_InstallAllocInterposers_GetReentries(bool _bNeedMalloc)
	{
#ifdef DMalterlibMemoryOverrideMacOSInitBeforeLibSystemSupport
		if (_bNeedMalloc && CSystem::ms_PlatformVersion < 10'11'00)
		{
			(void * &)malloc_reenter = dlsym(RTLD_DEFAULT, "malloc");

			if (!malloc_reenter)
			{
				DMibOverrideTrace("No malloc found, malloc override not enabled!\n", 0);
				return false;
			}
		}
#endif
		return true;
	}

	void fg_InstallAllocInterposers(bool _bNeedMalloc)
	{
		DMibOverrideTrace("fg_InstallAllocInterposers!\n");

		g_LowLevelGlobalState.f_Construct();
		NSys::fg_CreateSystemMalloc(true);
		g_GlobalState.f_Construct();

#ifdef DMalterlibMemoryOverrideMacOSInitBeforeLibSystemSupport
		NSys::g_FunctionHooks.f_Construct();
		NSys::g_FunctionHooks->f_Suspend();
		if (_bNeedMalloc && CSystem::ms_PlatformVersion < 10'11'00)
		{
			if (!NSys::g_FunctionHooks->f_SetHook((void **)&malloc_reenter, (void *)&fg_MalterlibSystem_Hooked_Malloc))
			{
				DMibOverrideTrace("Failed to hook malloc, aborting!\n", 0);
				DMibPDebugBreak;
			}
		}
		if (!NSys::g_FunctionHooks->f_SetHook((void **)&exit_reenter, (void *)&fg_MalterlibSystem_Hooked_Exit))
		{
			DMibOverrideTrace("Failed to hook exit, aborting!\n", 0);
			DMibPDebugBreak;
		}

		fg_InterposeOverride();

		NSys::g_FunctionHooks->f_Resume();
#endif

		g_MalterlibMallocOveriddenInterposersInstalled = true;

#ifdef DMemoryManagerIsSame
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			DMainHeapSmall->f_CanDoLazyCheckout();
		else
#endif
			DMainHeapMax->f_CanDoLazyCheckout();
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_MalterlibSystem_InitAfterMalloc()
	{
		if (g_MalterlibMallocAfterMallocCalled)
			return;
		g_MalterlibMallocAfterMallocCalled = true;

		g_nMallocZonesAfterMallocInit = malloc_num_zones;

		DMibOverrideTrace("fg_MalterlibSystem_InitAfterMalloc()\n");
		NSys::fg_CreateSystemVersion();

		if (g_MalterlibMallocOveriddenInterposersInstalled)
		{
			DMibOverrideTrace("fg_MalterlibSystem_InitAfterMalloc - g_MalterlibMallocOveriddenInterposersInstalled altready!\n");
			return;
		}

		if (!fg_InstallAllocInterposers_GetReentries(false))
		{
			fg_MalterlibMallocOverrideEnable();
			return; // Memory already allocated
		}

		fg_InstallAllocInterposers(false);
		fg_MalterlibMallocOverrideEnable();
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_MalterlibSystem_InitEarly(COriginalFunctions const &_Functions, int argc, char const* argv[], char const* envp[], char const* apple[], const ProgramVars * vars)
	{
#ifndef DMibMemoryOverrideDll
		if (!g_MalterlibMallocBeforeMallocCalled)
			g_OriginalFunctions = _Functions;
#endif

		NSys::fg_CreateSystemVersion();

		if (fg_RunningUnderRosetta())
		{
			if (NSys::fg_System_BeingDebugged())
			{
				DMibOverrideTrace("fg_System_BeingDebugged!\n");
				return;
			}
		}

		DMibOverrideTrace("fg_MalterlibSystem_InitEarly!\n");
#ifndef DMalterlibMemoryOverrideMacOSInitBeforeLibSystemSupport
		fg_MalterlibMallocOverrideEnable();
		return;
#else
#ifdef DArchitecture_x86
		// Not debugged for this arch
		return;
#endif

		if (g_MalterlibMallocOveriddenInterposersInstalled)
		{
			DMibOverrideTrace("fg_MalterlibSystem_InitEarly - g_MalterlibMallocOveriddenInterposersInstalled altready!\n");
			return;
		}

		struct utsname un;
		int Res = uname(&un);
		int Major = 0;
		if (Res >= 0)
		{
			NStr::CFStr256 VersionString;
			VersionString = un.release;
			int Minor = 0;
			int Fix = 0;

			Major = fg_GetStrSep(VersionString, ".").f_ToInt(0);
			Minor = fg_GetStrSep(VersionString, ".").f_ToInt(0);
			Fix = fg_GetStrSep(VersionString, ".").f_ToInt(0);

			if (Major >= 15)
			{
				DMibOverrideTrace("fg_MalterlibSystem_InitEarly - Use interposers instead!\n");
				return;
			}
			if (Major > 16)
				return; // Don't try to override on macOS that we don't yet know about as this is likely to fail
		}

		auto MemoryStats = mstats();
		if (MemoryStats.bytes_total)
		{
			DMibOverrideTrace("MemoryStats.bytes_total give up in init early!\n");
			return; // Something else got inbetween
		}

		if (envp)
		{
			for (mint i = 0; envp[i]; ++i)
			{
				if (NStr::fg_StrStartsWith(envp[i], "DYLD_INSERT_LIBRARIES"))
				{
					//DMibOverrideTrace("Malloc override disabled because of: {}\n", envp[i]);
					return;
				}

				if (NStr::fg_StrStartsWith(envp[i], "MalterlibMallocOverrideDisable"))
				{
					//DMibOverrideTrace("Malloc override disabled because of: MalterlibMallocOverrideDisable\n", 0);
					return;
				}
			}
		}

		auto nImages = _dyld_image_count();
		for (auto i = 0; i < nImages; ++i)
		{
			auto pName = _dyld_get_image_name(i);
			//DMibOverrideTrace("Image: {}\n", pName);
			if (NStr::fg_StrFindNoCase(pName, "/libBacktraceRecording.dylib") >= 0)
			{
				DMibOverrideTrace("Malloc override disabled because of: libBacktraceRecording.dylib\n", 0);
				return;
			}
			if (NStr::fg_StrFindNoCase(pName, "/libsimshim.dylib") >= 0)
			{
				DMibOverrideTrace("Malloc override disabled because of: libsimshim.dylib\n", 0);
				return;
			}
		}

		if (!fg_InstallAllocInterposers_GetReentries(true))
			return;

		void *pthread_init = dlsym(RTLD_DEFAULT, "__pthread_init");

		if (pthread_init)
		{
			if (Major == 15)
			{
				if (!fg_MalterlibSystem_InitMacOS10110(pthread_init, envp, apple, vars))
					return;
			}
			else if (Major == 14)
			{
				if (!fg_MalterlibSystem_InitMacOS10100(pthread_init, envp, apple, vars))
					return;
			}
			else
			{
				if (!fg_MalterlibSystem_InitMacOS1090(pthread_init, envp, apple, vars))
					return;
			}
		}
		else
		{
			pthread_init = dlsym(RTLD_DEFAULT, "pthread_init");

			if (pthread_init)
			{
				if (!fg_MalterlibSystem_InitMacOS1070(pthread_init))
					return;
			}
			else
			{
				if (!fg_MalterlibSystem_InitMacOS1060())
					return;
			}
		}

		fg_InstallAllocInterposers(true);
#endif
	}
}

bool fg_MalterlibMallocOverride_Enabled()
{
	return g_MalterlibMallocOveriddenInstalled;
}

malloc_zone_t_10_7 g_OriginalMallocs;
malloc_zone_t_10_7 *g_pDefaultZone = nullptr;

//#define DMibMacOSOverrideZoneCheck(_Z) DMibFastCheck((_malloc_zone_t_10_7 *)_Z == g_pDefaultZone)
#define DMibMacOSOverrideZoneCheck(_Z)

struct sigaction g_OldSignalHandlerBus;
struct sigaction g_OldSignalHandlerSegv;

void fg_HandleCrashSignalBus(int signal)
{
	auto *pJumpBuffer = g_LowLevelGlobalState->f_GetJumpBuffer();
	if (pJumpBuffer)
	{
#ifdef DOptimizeSetJmp
		// Unblock this signal so it's called again the next time
		sigset_t ToUnblock;
		sigemptyset (&ToUnblock);
		sigaddset(&ToUnblock, signal);
		sigprocmask(SIG_UNBLOCK, &ToUnblock, nullptr);
		_longjmp(*pJumpBuffer, 1);
#else
		longjmp(*pJumpBuffer, 1);
#endif
	}

	struct sigaction Old;
	sigaction(signal, &g_OldSignalHandlerBus, &Old);
	raise(signal);
}

void fg_HandleCrashSignalSegv(int signal)
{
	auto *pJumpBuffer = g_LowLevelGlobalState->f_GetJumpBuffer();
	if (pJumpBuffer)
	{
#ifdef DOptimizeSetJmp
		// Unblock this signal so it's called again the next time
		sigset_t ToUnblock;
		sigemptyset (&ToUnblock);
		sigaddset(&ToUnblock, signal);
		sigprocmask(SIG_UNBLOCK, &ToUnblock, nullptr);
		_longjmp(*pJumpBuffer, 1);
#else
		longjmp(*pJumpBuffer, 1);
#endif
	}

	struct sigaction Old;
	sigaction(signal, &g_OldSignalHandlerSegv, &Old);
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
	g_bInstalled = NSys::fg_Thread_GetCurrentUID();
#endif
}

void fg_MalterlibMallocOverrideInit_UninstallHandler()
{
	struct sigaction DummyHandler;

	if (sigaction(SIGBUS, &g_OldSignalHandlerBus, &DummyHandler) != 0)
	{
		[[maybe_unused]] auto Error = errno;
		DMibPDebugBreak;
	}

	if (sigaction(SIGSEGV, &g_OldSignalHandlerSegv, &DummyHandler) != 0)
	{
		[[maybe_unused]] auto Error = errno;
		DMibPDebugBreak;
	}

#ifdef DEmulateCrash
	g_bInstalled = 0;
#endif
}

extern "C"
{
#if !defined(DMibMemoryOverrideDll)
	assure_used DMibMalterlibOverrideMallocExport bool breakpad_should_handle_exception(pthread_t _pThread)
	{
		if (!g_MalterlibMallocOveriddenInstalled)
			return true;

		auto *pJumpBuffer = (jmp_buf *)NSys::fg_Thread_GetLocal((mint)_pThread, g_LowLevelGlobalState->m_iThreadLocal);
		if (pJumpBuffer)
			return false;
		else
			return true;
	}
#endif
}

size_t fg_Malterlib_zone_size(struct _malloc_zone_t *_pZone, const void *ptr) /* returns the size of a block or 0 if not in this zone; must be fast, especially for negative answers */
{
	if (!ptr)
		return 0;

	uint8 *pMalterlibAlloc = (uint8 *)ptr;

	auto &State = *g_LowLevelGlobalState;
	jmp_buf JumpBuffer;
	DMibFastCheck(!State.f_GetJumpBuffer());
	State.f_SetJumpBuffer(&JumpBuffer);
	auto Cleanup = fg_OnScopeExit
		(
			[&]()
			{
				State.f_SetJumpBuffer(nullptr);
			}
		)
	;

#ifdef DEmulateCrash
	if (g_bInstalled == NSys::fg_Thread_GetCurrentUID())
	{
		DMibConOut2("Will crash({}) = {}\n", (void *)NSys::fg_Thread_GetCurrentUID(), &State);
		pMalterlibAlloc = nullptr;
	}
#endif

#ifdef DOptimizeSetJmp
	if (_setjmp(JumpBuffer))
#else
	if (setjmp(JumpBuffer))
#endif
	{
#ifdef DEmulateCrash
		pMalterlibAlloc = (uint8 *)ptr;
#else
		return 0; // Access violation accessing header, this is not our block
#endif
	}

#ifdef DMemoryManagerIsSame
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		return DMainHeapSmall->f_TrySize(pMalterlibAlloc);
	else
#endif
		return DMainHeapMax->f_TrySize(pMalterlibAlloc);
#else
	return fg_TrySize(pMalterlibAlloc);
#endif
}

void fg_Malterlib_zone_free(struct _malloc_zone_t *_pZone, void *ptr)
{
	if (!ptr)
		return;
	DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	DMibMacOSOverrideZoneCheck(_pZone);
	uint8 *pMalterlibAlloc = (uint8 *)ptr;
#ifdef DMemoryManagerIsSame
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		return DMainHeapSmall->f_FreeNoSize(pMalterlibAlloc);
	else
#endif
		return DMainHeapMax->f_FreeNoSize(pMalterlibAlloc);
#else
	return fg_FreeNoSize(pMalterlibAlloc);
#endif
}

void fg_Malterlib_zone_free_definite_size(struct _malloc_zone_t *_pZone, void *ptr, size_t size)
{
	if (!ptr)
		return;
	DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	DMibMacOSOverrideZoneCheck(_pZone);
	uint8 *pMalterlibAlloc = (uint8 *)ptr;
#ifdef DMemoryManagerIsSame
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		return DMainHeapSmall->f_Free(pMalterlibAlloc, size);
	else
#endif
		return DMainHeapMax->f_Free(pMalterlibAlloc, size);
#else
	return fg_Free(pMalterlibAlloc, size);
#endif
}

size_t fg_Malterlib_zone_pressure_relief(struct _malloc_zone_t *_pZone, size_t goal)
{
	fg_GetSys()->f_MemoryManager_GarbageCollect();
	return 0;
}

#define DAlignSizeMacOS(d_Size) fg_AlignUp(fg_Max(d_Size, 1), 16)
//#define DAlignSizeMacOS(d_Size) d_Size

void *fg_Malterlib_zone_malloc(struct _malloc_zone_t *_pZone, size_t size)
{
	DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	DMibMacOSOverrideZoneCheck(_pZone);

	mint Size = DAlignSizeMacOS(size);

#ifdef DMemoryManagerIsSame
#if DEnableDebugMemoryManager
	uint8 * pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
	else
#endif
		pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
#else
	uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_Alloc(Size);
	else
#endif
		pMalterlibAlloc = (uint8 *)DMainHeapMax->f_Alloc(Size);
#endif
#else
#if DEnableDebugMemoryManager
	uint8 *pMalterlibAlloc = (uint8 *)fg_AllocDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
#else
	uint8 *pMalterlibAlloc = (uint8 *)fg_Alloc(Size);
#endif
#endif

	return pMalterlibAlloc;
}

unsigned fg_Malterlib_zone_batch_malloc(struct _malloc_zone_t *_pZone, size_t size, void **results, unsigned num_requested)
{
	DMibMacOSOverrideZoneCheck(_pZone);
	if (num_requested)
		return 0;
	DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	mint Size = DAlignSizeMacOS(size);

	mint nAllocated = 0;
#ifdef DMemoryManagerIsSame
#if DEnableDebugMemoryManager
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
	{
		DMainHeapSmall->f_AllocBatchDebug
			(
				Size
				, 1
				, [&](void *_pAlloc, mint _Size) -> bool
				{
					results[nAllocated] = _pAlloc;
					++nAllocated;
					return nAllocated < num_requested;
				}
				, DMibPFile
				, DMibPLine
				, g_DebugFlags
			)
		;
	}
	else
#endif
	{
		DMainHeapMax->f_AllocBatchDebug
			(
				Size
				, 1
				, [&](void *_pAlloc, mint _Size) -> bool
				{
					results[nAllocated] = _pAlloc;
					++nAllocated;
					return nAllocated < num_requested;
				}
				, DMibPFile
				, DMibPLine
				, g_DebugFlags
			)
		;
	}
#else
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
	{
		DMainHeapSmall->f_AllocBatch
			(
				Size
				, 1
				, [&](void *_pAlloc, mint _Size) -> bool
				{
					results[nAllocated] = _pAlloc;
					++nAllocated;
					return nAllocated < num_requested;
				}
			)
		;
	}
	else
#endif
	{
		DMainHeapMax->f_AllocBatch
			(
				Size
				, 1
				, [&](void *_pAlloc, mint _Size) -> bool
				{
					results[nAllocated] = _pAlloc;
					++nAllocated;
					return nAllocated < num_requested;
				}
			)
		;
	}
#endif
#else
#if DEnableDebugMemoryManager
	fg_AllocBatchDebug
		(
			Size
			, 1
			, [&](void *_pAlloc, mint _Size) -> bool
			{
				results[nAllocated] = _pAlloc;
				++nAllocated;
				return nAllocated < num_requested;
			}
			, DMibPFile
			, DMibPLine
			, g_DebugFlags
		)
	;
#else
	fg_AllocBatch
		(
			Size
			, 1
			, [&](void *_pAlloc, mint _Size) -> bool
			{
				results[nAllocated] = _pAlloc;
				++nAllocated;
				return nAllocated < num_requested;
			}
		)
	;
#endif
#endif

	return num_requested;
}

void fg_Malterlib_zone_batch_free(struct _malloc_zone_t *_pZone, void **to_be_freed, unsigned num_to_be_freed)
{
	if (num_to_be_freed == 0)
		return;
	DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);

#ifdef DMemoryManagerIsSame
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
	{
		auto &MainHeap = *DMainHeapSmall;
		auto Checkout = fg_GetSys()->f_MemoryManager_Checkout();
		for (mint iToFree = 0; iToFree < num_to_be_freed; ++iToFree)
			return MainHeap.f_FreeNoSize(to_be_freed[iToFree]);
	}
	else
#endif
	{
		auto &MainHeap = *DMainHeapMax;
		auto Checkout = fg_GetSys()->f_MemoryManager_Checkout();
		for (mint iToFree = 0; iToFree < num_to_be_freed; ++iToFree)
			return MainHeap.f_FreeNoSize(to_be_freed[iToFree]);
	}
#else
	auto Checkout = fg_GetSys()->f_MemoryManager_Checkout();
	for (mint iToFree = 0; iToFree < num_to_be_freed; ++iToFree)
		fg_FreeNoSize(to_be_freed[iToFree]);
#endif
}

void *fg_Malterlib_zone_calloc(struct _malloc_zone_t *_pZone, size_t num_items, size_t size) /* same as malloc, but block returned is set to zero */
{
	DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	DMibMacOSOverrideZoneCheck(_pZone);
	mint Size = DAlignSizeMacOS(size * num_items);
#ifdef DMemoryManagerIsSame
#if DEnableDebugMemoryManager
	uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
	else
#endif
		pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
#else
	uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_Alloc(Size);
	else
#endif
		pMalterlibAlloc = (uint8 *)DMainHeapMax->f_Alloc(Size);
#endif
#else
#if DEnableDebugMemoryManager
	uint8 *pMalterlibAlloc = (uint8 *)fg_AllocDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
#else
	uint8 *pMalterlibAlloc = (uint8 *)fg_Alloc(Size);
#endif
#endif
	fg_MemClear(pMalterlibAlloc, Size);
	return pMalterlibAlloc;
}
/* aligned memory allocation. The callback may be NULL. Present in version >= 5. */
void *fg_Malterlib_zone_memalign(struct _malloc_zone_t *_pZone, size_t alignment, size_t size)
{
	DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	DMibMacOSOverrideZoneCheck(_pZone);

	mint Size = size;
#ifdef DMemoryManagerIsSame
#if DEnableDebugMemoryManager
	uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAlignedWithSizeDebug(Size, alignment, DMibPFile, DMibPLine, g_DebugFlags);
	else
#endif
		pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAlignedWithSizeDebug(Size, alignment, DMibPFile, DMibPLine, g_DebugFlags);
#else
	uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAligned(Size, alignment);
	else
#endif
		pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAligned(Size, alignment);
#endif
#else
#if DEnableDebugMemoryManager
	uint8 *pMalterlibAlloc = (uint8 *)fg_AllocAlignedDebug(Size, alignment, DMibPFile, DMibPLine, g_DebugFlags);
#else
	uint8 *pMalterlibAlloc = (uint8 *)fg_AllocAligned(Size, alignment);
#endif
#endif

	return pMalterlibAlloc;
}

void *fg_Malterlib_zone_valloc(struct _malloc_zone_t *_pZone, size_t size) /* same as malloc, but block returned is set to zero and is guaranteed to be page aligned */
{
	DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	DMibMacOSOverrideZoneCheck(_pZone);

	mint Alignment = NSys::NPrivate::g_PageSize;
	mint Size = fg_AlignUp(fg_Max(size, 1), Alignment);
#ifdef DMemoryManagerIsSame
#if DEnableDebugMemoryManager
	uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAlignedWithSizeDebug(Size, Alignment, DMibPFile, DMibPLine, g_DebugFlags);
	else
#endif
		pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAlignedWithSizeDebug(Size, Alignment, DMibPFile, DMibPLine, g_DebugFlags);
#else
	uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAligned(Size, Alignment);
	else
#endif
		pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAligned(Size, Alignment);
#endif
#else
#if DEnableDebugMemoryManager
	uint8 *pMalterlibAlloc = (uint8 *)fg_AllocAlignedDebug(Size, Alignment, DMibPFile, DMibPLine, g_DebugFlags);
#else
	uint8 *pMalterlibAlloc = (uint8 *)fg_AllocAligned(Size, Alignment);
#endif
#endif

	fg_MemClear(pMalterlibAlloc, Size);
	return pMalterlibAlloc;
}

void *fg_Malterlib_zone_realloc(struct _malloc_zone_t *_pZone, void *ptr, size_t size)
{
	DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	DMibMacOSOverrideZoneCheck(_pZone);

	uint8 *pMalterlibAlloc = (uint8 *)ptr;

	mint Size = DAlignSizeMacOS(size);

#ifdef DMemoryManagerIsSame
#if DEnableDebugMemoryManager
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_ResizeDebug(pMalterlibAlloc, Size, 0, DMibPFile, DMibPLine, g_DebugFlags);
	else
#endif
		pMalterlibAlloc = (uint8 *)DMainHeapMax->f_ResizeDebug(pMalterlibAlloc, Size, 0, DMibPFile, DMibPLine, g_DebugFlags);
#else
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_Resize(pMalterlibAlloc, Size, 0);
	else
#endif
		pMalterlibAlloc = (uint8 *)DMainHeapMax->f_Resize(pMalterlibAlloc, Size, 0);
#endif
#else
#if DEnableDebugMemoryManager
	pMalterlibAlloc = (uint8 *)fg_ResizeDebug(pMalterlibAlloc, Size, 0, DMibPFile, DMibPLine, g_DebugFlags);
#else
	pMalterlibAlloc = (uint8 *)fg_Resize(pMalterlibAlloc, Size, 0, EAllocationFlag_SizeNotNeeded);
#endif
#endif
	return pMalterlibAlloc;
}

void fg_Malterlib_zone_destroy(struct _malloc_zone_t *_pZone) /* zone is destroyed and all memory reclaimed */
{
	DMibPDebugBreak; // Sholud not get here
}

size_t fg_Malterlib_zone_good_size(malloc_zone_t *zone, size_t size) /* zone is destroyed and all memory reclaimed */
{
#ifdef DMemoryManagerIsSame
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	if (g_bMainHeapIsSmall)
		return DMainHeapSmall->f_SizePadded(size);
	else
#endif
		return DMainHeapMax->f_SizePadded(size);
#else
	return fg_SizePadded(size);
#endif
}

malloc_introspection_t_10_7 g_MalterlibMallocZoneIntrospection =
	{
		[](task_t task, void *, unsigned type_mask, vm_address_t zone_address, memory_reader_t reader, vm_range_recorder_t recorder) -> kern_return_t /* enumerates all the malloc pointers in use */
		{
			return KERN_FAILURE;
		}
		, &fg_Malterlib_zone_good_size
		, [](malloc_zone_t *zone) -> boolean_t /* Consistency checker */
		{
			return true;
		}
		, [](malloc_zone_t *zone, boolean_t verbose) /* Prints zone  */
		{
		}
		, [](malloc_zone_t *zone, void *address) /* Enables logging of activity */
		{
		}
		, [](malloc_zone_t *zone) /* Forces locking zone */
		{
		}
		, [](malloc_zone_t *zone) /* Forces unlocking zone */
		{
		}
		, [](malloc_zone_t *zone, malloc_statistics_t *stats) /* Fills statistics */
		{
			fg_MemClear(*stats);
		}
		, [](malloc_zone_t *zone) -> boolean_t /* Are any zone locks held */
		{
			return false;
		}
		, [](malloc_zone_t *zone) -> boolean_t
		{
			return false;
		}
		, [](malloc_zone_t *zone)
		{
		}
		, [](malloc_zone_t *zone, void *memory)
		{
		}
	#ifdef __BLOCKS__
		, [](malloc_zone_t *zone, void (^report_discharged)(void *memory, void *info))
		{
		}
	#else
		, nullptr
	#endif /* __BLOCKS__ */
	}
;

malloc_zone_t_10_7 g_MalterlibMallocZone =
	{
		nullptr
		, nullptr
		, &fg_Malterlib_zone_size
		, &fg_Malterlib_zone_malloc
		, &fg_Malterlib_zone_calloc
		, &fg_Malterlib_zone_valloc
		, &fg_Malterlib_zone_free
		, &fg_Malterlib_zone_realloc
		, &fg_Malterlib_zone_destroy
		, nullptr // "DefaultMallocZone" // "Malterlib malloc zone"
		, &fg_Malterlib_zone_batch_malloc
		, &fg_Malterlib_zone_batch_free
		, &g_MalterlibMallocZoneIntrospection
		, 8
		, &fg_Malterlib_zone_memalign
		, &fg_Malterlib_zone_free_definite_size
		, &fg_Malterlib_zone_pressure_relief
	}
;

extern "C" malloc_zone_t **malloc_zones;
extern "C" bool g_bForeignZone = false;
extern "C" bool g_bHasForeignZones = false;
extern "C" bool g_bOnlyDefaultZone = true;

extern bool g_bRegisteredAtFork;

void fg_Override_PrepareFork();
void fg_Override_PrepareForkDummy();
void fg_Override_ForkedChild();
void fg_Override_ForkedParent();

void fg_MalterlibMallocOverrideEnable()
{
	if (g_MalterlibMallocOveriddenInstalled)
		return;
	g_MalterlibMallocOveriddenInstalled = true;

	if (!g_MalterlibMallocOveriddenInterposersInstalled)
	{
		g_MalterlibMallocOveriddenInterposersInstalled = true;
		g_bMemoryManagerNeededAfterDestroy = true;

		if (fg_InstallAllocInterposers_GetReentries(false))
			fg_InstallAllocInterposers(false);
		else
		{
			g_LowLevelGlobalState.f_Construct();
			NSys::fg_CreateSystemMalloc(false);
			g_GlobalState.f_Construct();
		}
	}

#if !defined(DMibConfig_MemoryManager_UseMalterlib) && !defined(DMibConfig_MemoryManager_UseOverwriteCheck) && !defined(DMibConfig_MemoryManager_UseSystem)
	return;
#endif

	if (CSystem::ms_PlatformVersion >= 10'16'00)
	{
		if (malloc_num_zones > g_nMallocZonesAfterMallocInit || g_nMallocZonesBeforeMallocInit > 0)
		{
			DMibOverrideTrace("FOREIGN\n");
			g_bForeignZone = true; // Some other manager got inbetwee
			g_bOnlyDefaultZone = false;
		}
	}
	else
	{
		auto MemoryStats = mstats();
		if (MemoryStats.bytes_total)
		{
			DMibOverrideTrace("FOREIGN\n");
			g_bForeignZone = true; // Some other manager got inbetwee
			g_bOnlyDefaultZone = false;
		}
	}

	DMibOverrideTrace("Installing ZOONE\n");
	fg_MalterlibMallocOverrideInit_ReinstallHandler();
#ifdef DMibConfig_CheckOverrideMemoryLeaks
	if (CSystem::ms_PlatformVersion >= 10'11'00)
		g_DebugFlags = EHeapDebugFlag_None;
#endif

#ifdef DMemoryManagerIsSame
	if (g_bForeignZone)
#endif
	{
		if (CSystem::ms_PlatformVersion >= 10'12'00)
		{
			malloc_destroy_zone(malloc_create_zone(0, 0));
			malloc_zone_t_10_7 *pDefaultZone = (malloc_zone_t_10_7 *)malloc_default_zone();
			g_pDefaultZone = pDefaultZone;
			g_OriginalMallocs = *pDefaultZone;
			malloc_zone_register((malloc_zone_t *)&g_MalterlibMallocZone);
			*pDefaultZone = g_MalterlibMallocZone;

			if (malloc_zones && malloc_num_zones > 1 && malloc_zones[0]->malloc != &fg_Malterlib_zone_malloc)
			{
				auto pDefaultRealZone = malloc_zones[0];
				malloc_zone_unregister(pDefaultRealZone);
				malloc_zone_register(pDefaultRealZone);
			}
		}
		else
		{
			malloc_zone_t_10_7 *pDefaultZone = (malloc_zone_t_10_7 *)malloc_default_zone();
			g_pDefaultZone = pDefaultZone;
			malloc_zone_unregister((malloc_zone_t *)g_pDefaultZone);
			malloc_zone_register((malloc_zone_t *)&g_MalterlibMallocZone);
			malloc_zone_register((malloc_zone_t *)g_pDefaultZone);
		}
	}

	malloc_set_zone_name((malloc_zone_t *)&g_MalterlibMallocZone, "MalterlibMemoryManager");
	DMibFastCheck(!g_bRegisteredAtFork);
	if (!g_bRegisteredAtFork)
	{
		g_bRegisteredAtFork = true;
		if (g_OriginalFunctions._malloc_fork_prepare && g_OriginalFunctions._malloc_fork_parent && g_OriginalFunctions._malloc_fork_child)
			;//pthread_atfork(&fg_Override_PrepareForkDummy, &fg_Override_ForkedParent, &fg_Override_ForkedChild);
		else
			pthread_atfork(&fg_Override_PrepareFork, &fg_Override_ForkedParent, &fg_Override_ForkedChild);
	}
}

void fg_MalterlibMallocOverrideDisable()
{
	if (!g_pDefaultZone)
		return;
	if (CSystem::ms_PlatformVersion >= 10'07'00)
		malloc_set_zone_name((malloc_zone_t *)&g_MalterlibMallocZone, nullptr);
#if 0
	if (CSystem::ms_PlatformVersion >= 10'12'00)
	{
		malloc_zone_unregister((malloc_zone_t *)&g_MalterlibMallocZone);
		*g_pDefaultZone = g_OriginalMallocs;
	}
	else
	{
		malloc_zone_unregister((malloc_zone_t *)&g_MalterlibMallocZone);
	}
#endif
}

namespace
{
#ifdef DArchitecture_x86
	using CThreadState = i386_thread_state_t;
	static constexpr int gc_StateFlavor = x86_THREAD_STATE32;
	static constexpr int gc_StateCount = x86_THREAD_STATE32_COUNT;
#elif defined(DArchitecture_x64)
	using CThreadState = x86_thread_state64_t;
	static constexpr int gc_StateFlavor = x86_THREAD_STATE64;
	static constexpr int gc_StateCount = x86_THREAD_STATE64_COUNT;
#elif defined(DArchitecture_ppc32)
	using CThreadState = ppc_thread_state_t;
	static constexpr int gc_StateFlavor = PPC_THREAD_STATE32;
	static constexpr int gc_StateCount = PPC_THREAD_STATE32_COUNT;
#elif defined(DArchitecture_ppc64)
	using CThreadState = ppc_thread_state64_t;
	static constexpr int gc_StateFlavor = PPC_THREAD_STATE64;
	static constexpr int gc_StateCount = PPC_THREAD_STATE64_COUNT;
#elif defined(DArchitecture_arm)
	using CThreadState = arm_thread_state_t;
	static constexpr int gc_StateFlavor = ARM_THREAD_STATE32;
	static constexpr int gc_StateCount = ARM_THREAD_STATE32_COUNT;
#elif defined(DArchitecture_arm64) || defined(DArchitecture_arm64e)
	using CThreadState = _STRUCT_ARM_THREAD_STATE64;
	static constexpr int gc_StateFlavor = ARM_THREAD_STATE64;
	static constexpr int gc_StateCount = ARM_THREAD_STATE64_COUNT;
#else
	#error "Implement this"
#endif
}

extern "C"
{
	boolean_t exc_server(mach_msg_header_t* request, mach_msg_header_t* reply);

	module_export assure_used kern_return_t catch_exception_raise_state_identity
		(
			mach_port_t _ExceptionPort
			, mach_port_t _Thread
			, mach_port_t _Task
			, exception_type_t _Exception
			, const exception_data_t _pCodes
			, mach_msg_type_number_t _CodesCount
			, int *_pFlavor
			, const thread_state_t _pOldState
			, mach_msg_type_number_t _OldStatesCount
			, thread_state_t _pNewState
			, mach_msg_type_number_t *_pNewStateCount
		)
	{
		if (_Task != mach_task_self())
			return KERN_FAILURE;

		auto pThread = pthread_from_mach_thread_np(_Thread);

		if (!g_MalterlibMallocOveriddenInstalled)
			return true;

		auto *pJumpBuffer = (jmp_buf *)NSys::fg_Thread_GetLocal((mint)pThread, g_LowLevelGlobalState->m_iThreadLocal);
		if (pJumpBuffer)
		{
			if (*_pFlavor != gc_StateFlavor)
				return KERN_FAILURE;

			if (_OldStatesCount != gc_StateCount)
				return KERN_FAILURE;

			[[maybe_unused]] CThreadState *pOldState = (CThreadState *)_pOldState;
			CThreadState *pNewState = (CThreadState *)_pNewState;
			*pNewState = *pOldState;
			*_pNewStateCount = gc_StateCount;

#if defined(DArchitecture_arm64e)
			pNewState->__x[0] = (mint)(void *)*pJumpBuffer;
			pNewState->__x[1] = 1;
			__darwin_arm_thread_state64_set_pc_fptr(*pNewState, (void *)&_longjmp);
#elif defined(DArchitecture_arm64)
			pNewState->__x[0] = (mint)(void *)*pJumpBuffer;
			pNewState->__x[1] = 1;
			pNewState->__pc = (mint)(void *)&_longjmp;
#elif defined(DArchitecture_x64)
			pNewState->__rdi = (mint)(void *)*pJumpBuffer;
			pNewState->__rsi = 1;
			pNewState->__rip = (mint)(void *)&_longjmp;
#else
#	error "Implement this"
#endif
			return KERN_SUCCESS;
		}

		auto &State = *g_GlobalState;

		if (!State.m_pExceptionHandlingState)
			return KERN_FAILURE;

		auto &ExceptionHandlingState = *State.m_pExceptionHandlingState;

		smint iNextHandler = -1;
		for (mint i = 0; i < ExceptionHandlingState.m_PreviousParams.m_nTypes; ++i)
		{
			if (ExceptionHandlingState.m_PreviousParams.m_Masks[i] & (1 << _Exception))
			{
				iNextHandler = i;
				break;
			}
		}

		if (iNextHandler == -1)
			return KERN_FAILURE;

		auto TargetPort = ExceptionHandlingState.m_PreviousParams.m_Ports[iNextHandler];
		auto TargetBehavior = ExceptionHandlingState.m_PreviousParams.m_Behaviors[iNextHandler];

		switch (TargetBehavior)
		{
		case EXCEPTION_DEFAULT:
			return exception_raise(TargetPort, _Thread, _Task, _Exception, _pCodes, _CodesCount);
		case EXCEPTION_STATE:
			return exception_raise_state(TargetPort, _Exception, _pCodes, _CodesCount, _pFlavor, _pOldState, _OldStatesCount, _pNewState, _pNewStateCount);
		case EXCEPTION_STATE_IDENTITY:
			return exception_raise_state_identity(TargetPort, _Thread, _Task, _Exception, _pCodes, _CodesCount, _pFlavor, _pOldState, _OldStatesCount, _pNewState, _pNewStateCount);
		default:
			return KERN_FAILURE;
		}

		return KERN_FAILURE;
	}
}

namespace
{
	CExceptionHandlingState::~CExceptionHandlingState()
	{
		if (m_ExceptionHandlingPort)
			mach_port_deallocate(mach_task_self(), m_ExceptionHandlingPort);
	}

	void CExceptionHandlingState::f_Abort()
	{
		CExceptionMessage Message;
		fg_MemClear(Message);

		Message.m_Header.msgh_id = 1;
		Message.m_Header.msgh_size = sizeof(Message);
		Message.m_Header.msgh_remote_port = m_ExceptionHandlingPort;
		Message.m_Header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, MACH_MSG_TYPE_MAKE_SEND_ONCE);

		mach_msg
			(
				&Message.m_Header
				, MACH_SEND_MSG | MACH_SEND_TIMEOUT
				, Message.m_Header.msgh_size
				, 0
				, 0
				, MACH_MSG_TIMEOUT_NONE
				, MACH_PORT_NULL
			)
		;
	}

	bool CExceptionHandlingState::f_InstallHandler()
	{
		mach_port_t CurrentTask = mach_task_self();

		kern_return_t Result = mach_port_allocate(CurrentTask, MACH_PORT_RIGHT_RECEIVE, &m_ExceptionHandlingPort);
		if (Result != KERN_SUCCESS)
		{
			DMibConErrOut("Failed to install exception handler (mach_port_allocate): {}\n", Result);
			return false;
		}

		Result = mach_port_insert_right(CurrentTask, m_ExceptionHandlingPort, m_ExceptionHandlingPort, MACH_MSG_TYPE_MAKE_SEND);
		if (Result != KERN_SUCCESS)
		{
			DMibConErrOut("Failed to install exception handler (mach_port_insert_right): {}\n", Result);
			return false;
		}

		Result = task_swap_exception_ports
			(
				CurrentTask
				, EXC_MASK_BAD_ACCESS
				, m_ExceptionHandlingPort
				, EXCEPTION_STATE_IDENTITY
				, gc_StateFlavor
				, m_PreviousParams.m_Masks
				, &m_PreviousParams.m_nTypes
				, m_PreviousParams.m_Ports
				, m_PreviousParams.m_Behaviors
				, m_PreviousParams.m_Flavors
			)
		;

		if (Result != KERN_SUCCESS)
		{
			DMibConErrOut("Failed to install exception handler (task_swap_exception_ports): {}\n", Result);
			return false;
		}

		return true;
	}

	bool CExceptionHandlingState::f_UnInstallHandler()
	{
		mach_port_t CurrentTask = mach_task_self();
		bool bReturn = true;

		for (mint i = 0; i < m_PreviousParams.m_nTypes; ++i)
		{
			kern_return_t Result = task_set_exception_ports
				(
					CurrentTask
					, m_PreviousParams.m_Masks[i]
					, m_PreviousParams.m_Ports[i]
					, m_PreviousParams.m_Behaviors[i]
					, m_PreviousParams.m_Flavors[i]
				)
			;
			if (Result != KERN_SUCCESS)
			{
				bReturn = false;
				DMibConErrOut2("Failed to uninstall exception handler (task_set_exception_ports, {}): {}\n", i, Result);
			}
		}

		return bReturn;
	}

	void CExceptionHandlingState::f_HandleMessages()
	{
		CExceptionMessage Message;

		while (true)
		{
			Message.m_Header.msgh_local_port = m_ExceptionHandlingPort;
			Message.m_Header.msgh_size = sizeof(Message);
			kern_return_t Result = mach_msg
				(
					&Message.m_Header
					,MACH_RCV_MSG | MACH_RCV_LARGE
					, 0
					, Message.m_Header.msgh_size
					, m_ExceptionHandlingPort
					, MACH_MSG_TIMEOUT_NONE
					, MACH_PORT_NULL
				)
			;

			if (Result != KERN_SUCCESS)
				return;

			if (!Message.m_Exception)
			{
				if (Message.m_Header.msgh_id == 1)
					return;
			}
			else
			{
				if (Message.m_Task.name != mach_task_self())
					continue; // Not our process, could have been forked

				CExceptionReplyMessage Reply;
				Reply.m_Header.msgh_size = sizeof(Reply);
				if (!exc_server(&Message.m_Header, &Reply.m_Header))
					return;

				// Send a reply and exit
				mach_msg
					(
						&Reply.m_Header
						, MACH_SEND_MSG
						, Reply.m_Header.msgh_size
						, 0
						, MACH_PORT_NULL
						, MACH_MSG_TIMEOUT_NONE
						, MACH_PORT_NULL
					)
				;
			}
		}
	}
}

void fg_MalterlibMallocOverride_DestroyThreads()
{
	if (!g_MalterlibMallocOveriddenInterposersInstalled)
		return;
	auto &State = *g_GlobalState;
	{
		DMibLockRead(State.m_ZoneListLock);
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
		{
			for (auto &Zone : State.m_ZoneListSmall)
				Zone.m_MemoryManager.f_DestroyCleanupThreads();
		}
		else
#endif
		{
			for (auto &Zone : State.m_ZoneListMax)
				Zone.m_MemoryManager.f_DestroyCleanupThreads();
		}
	}

	if (State.m_pExceptionHandlingState)
	{
		State.m_pExceptionHandlingState->f_Abort();
		State.m_pExceptionHandlingState.f_Clear();
	}

	if (State.m_pExceptionHandlingThread)
	{
		State.m_pExceptionHandlingThread->f_Stop();
		State.m_pExceptionHandlingThread.f_Clear();
	}
}

void fg_MalterlibMallocOverride_CanStartThreads()
{
	if (!g_MalterlibMallocOveriddenInterposersInstalled)
		return;

	auto &State = *g_GlobalState;
	{
		DMibLockRead(State.m_ZoneListLock);
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
		{
			for (auto &Zone : State.m_ZoneListSmall)
				Zone.m_MemoryManager.f_CanStartThreads();
		}
		else
#endif
		{
			for (auto &Zone : State.m_ZoneListMax)
				Zone.m_MemoryManager.f_CanStartThreads();
		}
	}

	if (!fg_RunningUnderRosetta() || (fg_RunningUnderRosetta() && !NSys::fg_System_BeingDebugged()))
	{
		State.m_pExceptionHandlingState = fg_Construct();
		State.m_pExceptionHandlingThread = NThread::CThreadObject::fs_StartThread
			(
				[pExceptionState = State.m_pExceptionHandlingState](NThread::CThreadObject *_pThread) mutable -> aint
				{
					auto pExceptionStateMove = fg_Move(pExceptionState);
					auto &ExceptionState = *pExceptionStateMove;

					if (!ExceptionState.f_InstallHandler())
					{
						ExceptionState.m_InstalledEvent.f_SetSignaled();
						return 0;
					}

					ExceptionState.m_InstalledEvent.f_SetSignaled();

					fg_MalterlibMallocOverrideInit_UninstallHandler();

					ExceptionState.f_HandleMessages();
					ExceptionState.f_UnInstallHandler();

					fg_MalterlibMallocOverrideInit_ReinstallHandler();

					return 0;
				}
				, "Memory manager override exception handling"
			)
		;
		State.m_pExceptionHandlingState->m_InstalledEvent.f_Wait();
	}
}

void NMib::NSys::fg_Mem_DisableLazyReturnCheckout()
{
	if (!g_MalterlibMallocOveriddenInterposersInstalled)
		return;
	auto &State = *g_LowLevelGlobalState;
	State.f_IncRentrant();
}

void NMib::NSys::fg_Mem_EnableLazyReturnCheckout()
{
	if (!g_MalterlibMallocOveriddenInterposersInstalled)
		return;
	auto &State = *g_LowLevelGlobalState;
	State.f_DecRentrant();
}

void fg_Override_PrepareFork()
{
	NSys::fg_MalterlibSystem_ForkPrepare();
}

void fg_Override_PrepareForkDummy()
{
}

assure_used extern "C" DMibMalterlibOverrideMallocExport void fg_Malterlib__malloc_fork_prepare()
{
	fg_Override_PrepareFork();

	if (g_OriginalFunctions._malloc_fork_prepare)
		g_OriginalFunctions._malloc_fork_prepare();
}

assure_used extern "C" DMibMalterlibOverrideMallocExport void fg_Malterlib__malloc_fork_child()
{
	fg_Override_ForkedChild();

	if (g_OriginalFunctions._malloc_fork_child)
		g_OriginalFunctions._malloc_fork_child();
}

void fg_Override_ForkedChild()
{
	NSys::fg_MalterlibSystem_ForkChild();
}

assure_used extern "C" DMibMalterlibOverrideMallocExport void fg_Malterlib__malloc_fork_parent()
{
	fg_Override_ForkedParent();

	if (g_OriginalFunctions._malloc_fork_parent)
		g_OriginalFunctions._malloc_fork_parent();
}

void NMib::NSys::fg_Mem_PrepareFork()
{
	if (!g_MalterlibMallocOveriddenInterposersInstalled)
		return;
	do
	{
		auto &LowLevelState = *g_LowLevelGlobalState;
		auto &State = *g_GlobalState;
		{
			DMibLockRead(State.m_ZoneListLock);
		}
		{
			DMibLock(State.m_ZoneListLock);
		}

		if (State.m_pExceptionHandlingThread)
			State.m_pExceptionHandlingThread->f_PrepareFork();

		LowLevelState.f_IncRentrant();
#ifdef DMemoryManagerIsSame
		State.m_ForkLock.f_Lock();
		if (++State.m_ForkedCount > 1)
			break;
		State.m_ForkLock.f_PrepareFork();

		State.m_Unforked = false;

		State.m_ZoneListLock.f_Lock();
		State.m_ZoneListLock.f_PrepareFork();

		if (!g_bOnlyDefaultZone)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
			{
				for (auto &Zone : State.m_ZoneListSmall)
				{
					Zone.m_MemoryManager.f_CheckoutManual();
					Zone.m_MemoryManager.f_Lock();
					Zone.m_MemoryManager.f_PrepareFork();
				}
			}
			else
#endif
			{
				for (auto &Zone : State.m_ZoneListMax)
				{
					Zone.m_MemoryManager.f_CheckoutManual();
					Zone.m_MemoryManager.f_Lock();
					Zone.m_MemoryManager.f_PrepareFork();
				}
			}
		}
		sigset_t NewMask;
		sigfillset(&NewMask);
		pthread_sigmask(SIG_SETMASK, &NewMask, &State.m_ForkSigMask);
#endif
	}
	while (false)
		;
}
void NMib::NSys::fg_Mem_ForkedChild()
{
	if (!g_MalterlibMallocOveriddenInterposersInstalled)
		return;
	do
	{
		auto &LowLevelState = *g_LowLevelGlobalState;
		auto &State = *g_GlobalState;
#ifdef DMemoryManagerIsSame
		--State.m_ForkedCount;
		if (State.m_Unforked)
		{
			State.m_ForkLock.f_Unlock();
			break;
		}
		State.m_ZoneListLock.f_ForkedChild();
		State.m_ForkLock.f_ForkedChild();
		State.m_Unforked = true;
		if (!g_bOnlyDefaultZone)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
			{
				for (auto &Zone : State.m_ZoneListSmall)
				{
					Zone.m_MemoryManager.f_ForkedChild();
					Zone.m_MemoryManager.f_CheckinManual();
					Zone.m_MemoryManager.f_Unlock();
				}
			}
			else
#endif
			{
				for (auto &Zone : State.m_ZoneListMax)
				{
					Zone.m_MemoryManager.f_ForkedChild();
					Zone.m_MemoryManager.f_CheckinManual();
					Zone.m_MemoryManager.f_Unlock();
				}
			}
		}
		State.m_ZoneListLock.f_Unlock();
#endif

		if (State.m_pExceptionHandlingThread)
			State.m_pExceptionHandlingThread->f_ForkedChild();

		LowLevelState.f_DecRentrant();
#ifdef DMemoryManagerIsSame
		pthread_sigmask(SIG_SETMASK, &State.m_ForkSigMask, nullptr);
		State.m_ForkLock.f_Unlock();
#endif
	}
	while (false)
		;
}
void NMib::NSys::fg_Mem_ForkedParent()
{
	if (!g_MalterlibMallocOveriddenInterposersInstalled)
		return;
	do
	{
		auto &LowLevelState = *g_LowLevelGlobalState;
		auto &State = *g_GlobalState;
#ifdef DMemoryManagerIsSame
		--State.m_ForkedCount;
		if (State.m_Unforked)
		{
			State.m_ForkLock.f_Unlock();
			break;
		}
		State.m_ZoneListLock.f_ForkedParent();
		State.m_ForkLock.f_ForkedParent();
		State.m_Unforked = true;
		if (!g_bOnlyDefaultZone)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
			{
				for (auto &Zone : State.m_ZoneListSmall)
				{
					Zone.m_MemoryManager.f_ForkedParent();
					Zone.m_MemoryManager.f_CheckinManual();
					Zone.m_MemoryManager.f_Unlock();
				}
			}
			else
#endif
			{
				for (auto &Zone : State.m_ZoneListMax)
				{
					Zone.m_MemoryManager.f_ForkedParent();
					Zone.m_MemoryManager.f_CheckinManual();
					Zone.m_MemoryManager.f_Unlock();
				}
			}
		}
		State.m_ZoneListLock.f_Unlock();
#endif

		if (State.m_pExceptionHandlingThread)
			State.m_pExceptionHandlingThread->f_ForkedParent();

		LowLevelState.f_DecRentrant();
#ifdef DMemoryManagerIsSame
		pthread_sigmask(SIG_SETMASK, &State.m_ForkSigMask, nullptr);
		State.m_ForkLock.f_Unlock();
#endif
	}
	while (false)
		;
}

void fg_Override_ForkedParent()
{
	NSys::fg_MalterlibSystem_ForkParent();
}

namespace
{
	struct CInvalidRegionCacheThreadLocal
	{
		struct CRegionData
		{
			uint32 m_Type;

			bool operator == (CRegionData const &_Right) const
			{
				return m_Type == _Right.m_Type;
			}
		};

		using CRegionAllocator = TCStaticPoolAllocator<NContainer::TCMapNode<mint, NContainer::TCRegionData<mint, CRegionData>>, 128, CAllocator_VirtualNoTracking>;

		NContainer::TCRegions<mint, CRegionData, CRegionAllocator> m_Regions;
	};

	constinit NStorage::TCAggregate<NThread::TCThreadLocal<CInvalidRegionCacheThreadLocal>> g_InvalidRegionCache = {DAggregateInit};

	int g_bIsBeingDebugged = -1;

	inline_never void fg_UpdateBeingDebugged()
	{
		g_bIsBeingDebugged = NSys::fg_System_BeingDebugged();
	}

	inline_never bool fg_IsInvalidRegionSlowPath(const void *_pMemory)
	{
		if (g_InvalidRegionCache.f_WasDestructed() || !g_bCreatedSystem || fg_GetSys()->f_ThreadDestroyed() || !fg_GetSys()->f_ThreadCreated())
			return false;

		auto &Cache = **g_InvalidRegionCache;
		vm_address_t Address = (mint)_pMemory;
		auto pData = Cache.m_Regions.f_GetData(Address);
		if (pData)
			return pData->m_Type < 240 || pData->m_Type > 255;

		vm_size_t Size;
		natural_t Depth = 0;
		mach_msg_type_number_t InfoCount = VM_REGION_SUBMAP_SHORT_INFO_COUNT_64;
		vm_region_submap_short_info_64 Info;
		auto Return = vm_region_recurse_64(mach_task_self(), &Address, &Size, &Depth, (vm_region_recurse_info_t)&Info, &InfoCount);
        if (Return)
			return false;

		Cache.m_Regions.f_MakeRegion
			(
				Address
				, Address + Size
				, [&](CInvalidRegionCacheThreadLocal::CRegionData &_Data)
				{
					_Data.m_Type = Info.user_tag;
				}
			)
		;

		return Info.user_tag < 240 || Info.user_tag > 255;
	}

	[[maybe_unused]] inline_always bool fg_IsInvalidRegion(const void *_pMemory)
	{
		if ((uint8 *)_pMemory != fg_AlignUp((uint8 *)_pMemory, 16))
			return true; // Unaligned memory can never be OK

		if (!fg_RunningUnderRosetta())
			return false;

		if (g_bIsBeingDebugged < 0) [[unlikely]]
			fg_UpdateBeingDebugged();

		if (!g_bIsBeingDebugged) [[likely]]
			return false;

		return fg_IsInvalidRegionSlowPath(_pMemory);
	}
}

namespace
{
#ifdef DMemoryManagerIsSame

	bool fg_LazyReturnCheckout()
	{
		if (!g_MalterlibMallocOveriddenInterposersInstalled)
			return false;
		auto &LowLevelState = *g_LowLevelGlobalState;
		auto &State = *g_GlobalState;
#ifdef DMemoryManagerIsSame
		if (LowLevelState.f_GetRentrant())
			return false;

		LowLevelState.f_IncRentrant();
		auto Cleanup = g_OnScopeExit / [&]
			{
				LowLevelState.f_DecRentrant();
			}
		;
#ifdef DFullArenasForSecondary
		if (!g_bOnlyDefaultZone)
		{
			DMibLockRead(State.m_ZoneListLock);
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
			{
				for (auto &Zone : State.m_ZoneListSmall)
					Zone.m_MemoryManager.f_LazyReturnCheckout();
			}
			else
#endif
			{
				for (auto &Zone : State.m_ZoneListMax)
					Zone.m_MemoryManager.f_LazyReturnCheckout();
			}
		}
#endif
#endif
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			DMainHeapSmall->f_LazyReturnCheckout();
		else
#endif
			DMainHeapMax->f_LazyReturnCheckout();

		return true;
	}

	template <typename tf_CMemoryManagerZone>
	mint fg_Malterlib_Safe_GetSize(tf_CMemoryManagerZone *_pZone, const void *_pMemory)
	{
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;

		auto &LowLevelState = *g_LowLevelGlobalState;
		jmp_buf JumpBuffer;
		DMibFastCheck(!LowLevelState.f_GetJumpBuffer());
		LowLevelState.f_SetJumpBuffer(&JumpBuffer);
		auto Cleanup = fg_OnScopeExit
			(
				[&]()
				{
					LowLevelState.f_SetJumpBuffer(nullptr);
				}
			)
		;
	#ifdef DOptimizeSetJmp
		if (_setjmp(JumpBuffer))
	#else
		if (setjmp(JumpBuffer))
	#endif
			return 0; // Access violation accessing header, this is not our block
		return _pZone->m_MemoryManager.f_TrySize(pMalterlibAlloc);
	}

	template <typename tf_CMemoryManager>
	tf_CMemoryManager *fg_Malterlib_Safe_GetDefaultMemoryManager(void const *_pMemory)
	{
		auto &LowLevelState = *g_LowLevelGlobalState;
		jmp_buf JumpBuffer;
		DMibFastCheck(!LowLevelState.f_GetJumpBuffer());
		LowLevelState.f_SetJumpBuffer(&JumpBuffer);
		auto Cleanup = fg_OnScopeExit
			(
				[&]()
				{
					LowLevelState.f_SetJumpBuffer(nullptr);
				}
			)
		;

	#ifdef DOptimizeSetJmp
		if (_setjmp(JumpBuffer))
	#else
		if (setjmp(JumpBuffer))
	#endif
			return nullptr;

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if constexpr (NTraits::TCIsSame<tf_CMemoryManager, CMemoryManagerSmall>::mc_Value)
			return DMainHeapSmall->f_GetMemoryManager(_pMemory);
		else
#endif
			return DMainHeapMax->f_GetMemoryManager(_pMemory);
	}

	template <typename tf_CMemoryManager>
	tf_CMemoryManager *fg_Malterlib_Safe_GetOtherMemoryManager(void const *_pMemory)
	{
		auto &LowLevelState = *g_LowLevelGlobalState;
		auto &State = *g_GlobalState;
		jmp_buf JumpBuffer;
		DMibFastCheck(!LowLevelState.f_GetJumpBuffer());
		LowLevelState.f_SetJumpBuffer(&JumpBuffer);
		auto Cleanup = fg_OnScopeExit
			(
				[&]()
				{
					LowLevelState.f_SetJumpBuffer(nullptr);
				}
			)
		;

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if constexpr (NTraits::TCIsSame<tf_CMemoryManager, CMemoryManagerSmall>::mc_Value)
		{
			DMibLockRead(State.m_ZoneListLock);
			auto iZone = State.m_ZoneListSmall.f_GetIterator();

		#ifdef DOptimizeSetJmp
			if (_setjmp(JumpBuffer))
		#else
			if (setjmp(JumpBuffer))
		#endif
				return nullptr;

			for (; iZone; ++iZone)
			{
				auto pMemoryManager = iZone->m_MemoryManager.f_GetMemoryManager(_pMemory);
				if (pMemoryManager)
					return pMemoryManager;
			}
			return nullptr;
		}
		else
#endif
		{
			DMibLockRead(State.m_ZoneListLock);
			auto iZone = State.m_ZoneListMax.f_GetIterator();

		#ifdef DOptimizeSetJmp
			if (_setjmp(JumpBuffer))
		#else
			if (setjmp(JumpBuffer))
		#endif
				return nullptr;

			for (; iZone; ++iZone)
			{
				auto pMemoryManager = iZone->m_MemoryManager.f_GetMemoryManager(_pMemory);
				if (pMemoryManager)
					return pMemoryManager;
			}
			return nullptr;
		}
	}
	template <typename tf_CMemoryManager>
	inline_always tf_CMemoryManager *fg_Malterlib_Safe_GetMemoryManager(void const *_pMemory)
	{
		auto *pManager = fg_Malterlib_Safe_GetDefaultMemoryManager<tf_CMemoryManager>(_pMemory);
		if (pManager) [[likely]]
			return pManager;
		return fg_Malterlib_Safe_GetOtherMemoryManager<tf_CMemoryManager>(_pMemory);
	}

	template <typename tf_CMemoryManager>
	inline_always malloc_zone_t *fg_Malterlib_ZoneFromMemoryManager(tf_CMemoryManager *_pMemoryManager)
	{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if constexpr (NTraits::TCIsSame<tf_CMemoryManager, CMemoryManagerSmall>::mc_Value)
		{
			if (_pMemoryManager == &(*DMainHeapSmall))
				return (malloc_zone_t *)&g_MalterlibMallocZone;
			return (malloc_zone_t *)&(((CMemoryManagerZoneSmall *)((uint8 *)_pMemoryManager - DMibPOffsetOf(CMemoryManagerZoneSmall, m_MemoryManager)))->m_MallocZone);
		}
		else
#endif
		{
			if (_pMemoryManager == &(*DMainHeapMax))
				return (malloc_zone_t *)&g_MalterlibMallocZone;
			return (malloc_zone_t *)&(((CMemoryManagerZoneMax *)((uint8 *)_pMemoryManager - DMibPOffsetOf(CMemoryManagerZoneMax, m_MemoryManager)))->m_MallocZone);
		}
	}

	template <typename tf_CMemoryManager>
	tf_CMemoryManager *fg_Malterlib_GetMemoryManager(void const *_pMemory)
	{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if constexpr (NTraits::TCIsSame<tf_CMemoryManager, CMemoryManagerSmall>::mc_Value)
		{
			tf_CMemoryManager *pMemoryManager = DMainHeapSmall->f_GetMemoryManager(_pMemory);
			if (pMemoryManager) [[likely]]
				return pMemoryManager;
			auto &State = *g_GlobalState;
			DMibLockRead(State.m_ZoneListLock);
			for (auto &Zone : State.m_ZoneListSmall)
			{
				pMemoryManager = Zone.m_MemoryManager.f_GetMemoryManager(_pMemory);
				if (pMemoryManager)
					return pMemoryManager;
			}
			return nullptr;
		}
		else
#endif
		{
			tf_CMemoryManager *pMemoryManager = DMainHeapMax->f_GetMemoryManager(_pMemory);
			if (pMemoryManager) [[likely]]
				return pMemoryManager;
			auto &State = *g_GlobalState;
			DMibLockRead(State.m_ZoneListLock);
			for (auto &Zone : State.m_ZoneListMax)
			{
				pMemoryManager = Zone.m_MemoryManager.f_GetMemoryManager(_pMemory);
				if (pMemoryManager)
					return pMemoryManager;
			}
			return nullptr;
		}
	}

#endif


}
// Direct overrides
extern "C"
{

	assure_used DMibMalterlibOverrideMallocExport void fg_MalterlibSystem_InitBeforeMalloc(COriginalFunctions const &_Functions)
	{
		if (g_MalterlibMallocBeforeMallocCalled)
			return;
		g_MalterlibMallocBeforeMallocCalled = true;
		g_nMallocZonesBeforeMallocInit = malloc_num_zones;

#ifndef DMibMemoryOverrideDll
		g_OriginalFunctions = _Functions;
#endif
	}

	malloc_zone_t *fg_GetForeignZone(void const *_pMemory, mint *_pSize = nullptr)
	{
		auto &State = *g_GlobalState;
		for (mint iZone = 0; iZone < State.m_nForeignZones; ++iZone)
		{
			auto *pZone = State.m_pForeignZones[iZone];
			if (!pZone)
				continue;
			if (mint Size = pZone->size(pZone, _pMemory))
			{
				if (_pSize)
					*_pSize = Size;
				return pZone;
			}
		}
		return nullptr;
	}

	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib_malloc(size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		mint Size = DAlignSizeMacOS(_Size);
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
#if DEnableDebugMemoryManager
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
#else
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_Alloc(Size);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_Alloc(Size);
#endif
		return pMalterlibAlloc;
#else
		return g_OriginalFunctions.malloc(_Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib_valloc(size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		mint Alignment = NSys::NPrivate::g_PageSize;
		mint Size = DAlignSizeMacOS(_Size);
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	#if DEnableDebugMemoryManager
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAlignedWithSizeDebug(Size, Alignment, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAlignedWithSizeDebug(Size, Alignment, DMibPFile, DMibPLine, g_DebugFlags);
	#else
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAligned(Size, Alignment);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAligned(Size, Alignment);
	#endif
		return pMalterlibAlloc;
#else
		return g_OriginalFunctions.valloc(_Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib_calloc(size_t _NumItems, size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		mint Size = DAlignSizeMacOS(_Size * _NumItems);
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	#if DEnableDebugMemoryManager
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
	#else
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_Alloc(Size);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_Alloc(Size);
	#endif
		fg_MemClear(pMalterlibAlloc, Size);
		return pMalterlibAlloc;
#else
		return g_OriginalFunctions.calloc(_NumItems, _Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib_realloc(void *_pMemory, size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.realloc(_pMemory, _Size);
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
		mint Size = DAlignSizeMacOS(_Size);
		if (g_bOnlyDefaultZone || !_pMemory)
		{
			DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
#			if DEnableDebugMemoryManager
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if (g_bMainHeapIsSmall)
					return (uint8 *)DMainHeapSmall->f_ResizeDebug(pMalterlibAlloc, Size, 0, DMibPFile, DMibPLine, g_DebugFlags);
				else
#endif
					return (uint8 *)DMainHeapMax->f_ResizeDebug(pMalterlibAlloc, Size, 0, DMibPFile, DMibPLine, g_DebugFlags);
#			else
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if (g_bMainHeapIsSmall)
					return (uint8 *)DMainHeapSmall->f_Resize(pMalterlibAlloc, Size, 0);
				else
#endif
					return (uint8 *)DMainHeapMax->f_Resize(pMalterlibAlloc, Size, 0);
#			endif
		}
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
		{
			CMemoryManagerSmall *pMemoryManager;
			if (g_bHasForeignZones)
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
				if (!pMemoryManager)
				{
					auto pZone = fg_GetForeignZone(_pMemory);
					if (!pZone)
					{
						DMibOverrideErrorOutput("realloc failed beacuse no zone was found for pointer: {}\n", _pMemory);
						return nullptr;
					}
					return pZone->realloc(pZone, _pMemory, _Size);
				}
			}
			else
				pMemoryManager = fg_Malterlib_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
			DMibFastCheck(pMemoryManager);
			DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	#		if DEnableDebugMemoryManager
				return pMemoryManager->f_ResizeDebug(_pMemory, Size, 0, DMibPFile, DMibPLine, g_DebugFlags);
	#		else
				return pMemoryManager->f_Resize(_pMemory, Size, 0);
	#		endif
		}
		else
#endif
		{
			CMemoryManagerMax *pMemoryManager;
			if (g_bHasForeignZones)
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerMax>(_pMemory);
				if (!pMemoryManager)
				{
					auto pZone = fg_GetForeignZone(_pMemory);
					if (!pZone)
					{
						DMibOverrideErrorOutput("realloc failed beacuse no zone was found for pointer: {}\n", _pMemory);
						return nullptr;
					}
					return pZone->realloc(pZone, _pMemory, _Size);
				}
			}
			else
				pMemoryManager = fg_Malterlib_GetMemoryManager<CMemoryManagerMax>(_pMemory);
			DMibFastCheck(pMemoryManager);
			DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	#		if DEnableDebugMemoryManager
				return pMemoryManager->f_ResizeDebug(_pMemory, Size, 0, DMibPFile, DMibPLine, g_DebugFlags);
	#		else
				return pMemoryManager->f_Resize(_pMemory, Size, 0);
	#		endif
		}
#else
		return g_OriginalFunctions.realloc(_pMemory, _Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib_reallocf(void *_pMemory, size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.reallocf(_pMemory, _Size);
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
		mint Size = DAlignSizeMacOS(_Size);
		if (g_bOnlyDefaultZone || !_pMemory)
		{
			DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
#			if DEnableDebugMemoryManager
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if (g_bMainHeapIsSmall)
					return (uint8 *)DMainHeapSmall->f_ResizeDebug(pMalterlibAlloc, Size, 0, DMibPFile, DMibPLine, g_DebugFlags);
				else
#endif
					return (uint8 *)DMainHeapMax->f_ResizeDebug(pMalterlibAlloc, Size, 0, DMibPFile, DMibPLine, g_DebugFlags);
#			else
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if (g_bMainHeapIsSmall)
					return (uint8 *)DMainHeapSmall->f_Resize(pMalterlibAlloc, Size, 0);
				else
#endif
					return (uint8 *)DMainHeapMax->f_Resize(pMalterlibAlloc, Size, 0);
#			endif
		}
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
		{
			CMemoryManagerSmall *pMemoryManager;
			if (g_bHasForeignZones)
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
				if (!pMemoryManager)
				{
					auto pZone = fg_GetForeignZone(_pMemory);
					if (!pZone)
					{
						DMibOverrideErrorOutput("realloc failed beacuse no zone was found for pointer: {}\n", _pMemory);
						return nullptr;
					}
					return pZone->realloc(pZone, _pMemory, _Size);
				}
			}
			else
				pMemoryManager = fg_Malterlib_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
			DMibFastCheck(pMemoryManager);
			DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	#		if DEnableDebugMemoryManager
				return pMemoryManager->f_ResizeDebug(_pMemory, Size, 0, DMibPFile, DMibPLine, g_DebugFlags);
	#		else
				return pMemoryManager->f_Resize(_pMemory, Size, 0);
	#		endif
		}
		else
#endif
		{
			CMemoryManagerMax *pMemoryManager;
			if (g_bHasForeignZones)
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerMax>(_pMemory);
				if (!pMemoryManager)
				{
					auto pZone = fg_GetForeignZone(_pMemory);
					if (!pZone)
					{
						DMibOverrideErrorOutput("realloc failed beacuse no zone was found for pointer: {}\n", _pMemory);
						return nullptr;
					}
					return pZone->realloc(pZone, _pMemory, _Size);
				}
			}
			else
				pMemoryManager = fg_Malterlib_GetMemoryManager<CMemoryManagerMax>(_pMemory);
			DMibFastCheck(pMemoryManager);
			DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	#		if DEnableDebugMemoryManager
				return pMemoryManager->f_ResizeDebug(_pMemory, Size, 0, DMibPFile, DMibPLine, g_DebugFlags);
	#		else
				return pMemoryManager->f_Resize(_pMemory, Size, 0);
	#		endif
		}
#else
		return g_OriginalFunctions.reallocf(_pMemory, _Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_free(void *_pMemory)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.free(_pMemory);
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
		if (g_bOnlyDefaultZone)
		{
			DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
#ifdef DMemoryManagerIsSame
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				return DMainHeapSmall->f_FreeNoSize(pMalterlibAlloc);
			else
#endif
				return DMainHeapMax->f_FreeNoSize(pMalterlibAlloc);
#else
			return fg_FreeNoSize(pMalterlibAlloc);
#endif
		}
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
		{
			CMemoryManagerSmall *pMemoryManager;
			if (g_bHasForeignZones)
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
				if (!pMemoryManager)
				{
					auto pZone = fg_GetForeignZone(_pMemory);
					if (!pZone)
					{
						DMibOverrideErrorOutput("free failed beacuse no zone was found for pointer: {}\n", _pMemory);
						return;
					}
					return pZone->free(pZone, _pMemory);
				}
			}
			else
				pMemoryManager = fg_Malterlib_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
			DMibFastCheck(pMemoryManager);
			DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
			return pMemoryManager->f_FreeNoSize(_pMemory);
		}
		else
#endif
		{
			CMemoryManagerMax *pMemoryManager;
			if (g_bHasForeignZones)
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerMax>(_pMemory);
				if (!pMemoryManager)
				{
					auto pZone = fg_GetForeignZone(_pMemory);
					if (!pZone)
					{
						DMibOverrideErrorOutput("free failed beacuse no zone was found for pointer: {}\n", _pMemory);
						return;
					}
					return pZone->free(pZone, _pMemory);
				}
			}
			else
				pMemoryManager = fg_Malterlib_GetMemoryManager<CMemoryManagerMax>(_pMemory);
			DMibFastCheck(pMemoryManager);
			DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
			return pMemoryManager->f_FreeNoSize(_pMemory);
		}
#else
		return g_OriginalFunctions.free(_pMemory);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_vfree(void *_pMemory)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.vfree(_pMemory);
		if (g_bOnlyDefaultZone)
		{
			DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
#ifdef DMemoryManagerIsSame
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				return DMainHeapSmall->f_FreeNoSize(_pMemory);
			else
#endif
				return DMainHeapMax->f_FreeNoSize(_pMemory);
#else
			return fg_FreeNoSize((uint8 *)_pMemory);
#endif
		}
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
		{
			CMemoryManagerSmall *pMemoryManager;
			if (g_bHasForeignZones)
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
				if (!pMemoryManager)
				{
					auto pZone = fg_GetForeignZone(_pMemory);
					if (!pZone)
					{
						DMibOverrideErrorOutput("free failed beacuse no zone was found for pointer: {}\n", _pMemory);
						return;
					}
					return pZone->free(pZone, _pMemory);
				}
			}
			else
				pMemoryManager = fg_Malterlib_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
			DMibFastCheck(pMemoryManager);
			DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
			return pMemoryManager->f_FreeNoSize(_pMemory);
		}
		else
#endif
		{
			CMemoryManagerMax *pMemoryManager;
			if (g_bHasForeignZones)
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerMax>(_pMemory);
				if (!pMemoryManager)
				{
					auto pZone = fg_GetForeignZone(_pMemory);
					if (!pZone)
					{
						DMibOverrideErrorOutput("free failed beacuse no zone was found for pointer: {}\n", _pMemory);
						return;
					}
					return pZone->free(pZone, _pMemory);
				}
			}
			else
				pMemoryManager = fg_Malterlib_GetMemoryManager<CMemoryManagerMax>(_pMemory);
			DMibFastCheck(pMemoryManager);
			DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
			return pMemoryManager->f_FreeNoSize(_pMemory);
		}
#else
		return g_OriginalFunctions.vfree(_pMemory);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport int fg_Malterlib_posix_memalign(void **_pOutput, size_t _Alignment, size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		mint Size = DAlignSizeMacOS(_Size);
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	#if DEnableDebugMemoryManager
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAlignedWithSizeDebug(Size, _Alignment, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAlignedWithSizeDebug(Size, _Alignment, DMibPFile, DMibPLine, g_DebugFlags);
	#else
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAligned(Size, _Alignment);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAligned(Size, _Alignment);
	#endif
		*_pOutput = pMalterlibAlloc;
		return 0;
#else
		return g_OriginalFunctions.posix_memalign(_pOutput, _Alignment, _Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib_aligned_alloc(size_t _Alignment, size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		mint Size = DAlignSizeMacOS(_Size);
		DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
	#if DEnableDebugMemoryManager
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_AllocAlignedWithSizeDebug(Size, _Alignment, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			return DMainHeapMax->f_AllocAlignedWithSizeDebug(Size, _Alignment, DMibPFile, DMibPLine, g_DebugFlags);
	#else
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_AllocAligned(Size, _Alignment);
		else
#endif
			return DMainHeapMax->f_AllocAligned(Size, _Alignment);
	#endif
#else
		return g_OriginalFunctions.aligned_alloc(_Alignment, _Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport int fg_Malterlib_malloc_jumpstart(int _Value)
	{
#ifdef DMemoryManagerIsSame
		return 1;
#else
		return g_OriginalFunctions.malloc_jumpstart(_Value);
#endif
	}

	// operator new[](unsigned long)
	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib__Znam(size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		mint Size = DAlignSizeMacOS(_Size);
	#if DEnableDebugMemoryManager
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
	#else
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_Alloc(Size);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_Alloc(Size);
	#endif
		return pMalterlibAlloc;
#else
		return fg_Alloc(_Size);
#endif
	}

	// operator new(unsigned long)
	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib__Znwm(size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		mint Size = DAlignSizeMacOS(_Size);
	#if DEnableDebugMemoryManager
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
	#else
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_Alloc(Size);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_Alloc(Size);
	#endif
		return pMalterlibAlloc;
#else
		return fg_Alloc(_Size);
#endif
	}

	// operator new(unsigned long, std::nothrow_t const&)
	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib__ZnwmRKSt9nothrow_t(size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		mint Size = DAlignSizeMacOS(_Size);
	#if DEnableDebugMemoryManager
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
	#else
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_Alloc(Size);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_Alloc(Size);
	#endif
		return pMalterlibAlloc;
#else
		return fg_Alloc(_Size);
#endif
	}

	// operator new(unsigned long, std::align_val_t)
	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib__ZnwmSt11align_val_t(size_t _Size, size_t _Alignment)
	{
#ifdef DMemoryManagerIsSame
	#if DEnableDebugMemoryManager
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAlignedWithSizeDebug(_Size, (mint)_Alignment, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAlignedWithSizeDebug(_Size, (mint)_Alignment, DMibPFile, DMibPLine, g_DebugFlags);
	#else
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAligned(_Size, (mint)_Alignment);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAligned(_Size, (mint)_Alignment);
	#endif
		return pMalterlibAlloc;
#else
		return fg_AllocAligned(_Size, _Alignment);
#endif
	}

	// operator new(unsigned long, std::align_val_t, std::nothrow_t const&)
	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib__ZnwmSt11align_val_tRKSt9nothrow_t(size_t _Size, size_t _Alignment)
	{
#ifdef DMemoryManagerIsSame
	#if DEnableDebugMemoryManager
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAlignedWithSizeDebug(_Size, (mint)_Alignment, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAlignedWithSizeDebug(_Size, (mint)_Alignment, DMibPFile, DMibPLine, g_DebugFlags);
	#else
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAligned(_Size, (mint)_Alignment);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAligned(_Size, (mint)_Alignment);
	#endif
		return pMalterlibAlloc;
#else
		return fg_AllocAligned(_Size, _Alignment);
#endif
	}

	// operator new[](unsigned long, std::nothrow_t const&)
	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib__ZnamRKSt9nothrow_t(size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		mint Size = DAlignSizeMacOS(_Size);
	#if DEnableDebugMemoryManager
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocWithSizeDebug(Size, DMibPFile, DMibPLine, g_DebugFlags);
	#else
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_Alloc(Size);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_Alloc(Size);
	#endif
		return pMalterlibAlloc;
#else
		return fg_Alloc(_Size);
#endif
	}

	// operator new[](unsigned long, std::align_val_t)
	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib__ZnamSt11align_val_t(size_t _Size, size_t _Alignment)
	{
#ifdef DMemoryManagerIsSame
	#if DEnableDebugMemoryManager
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAlignedWithSizeDebug(_Size, (mint)_Alignment, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAlignedWithSizeDebug(_Size, (mint)_Alignment, DMibPFile, DMibPLine, g_DebugFlags);
	#else
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAligned(_Size, (mint)_Alignment);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAligned(_Size, (mint)_Alignment);
 	#endif
		return pMalterlibAlloc;
#else
		return fg_AllocAligned(_Size, _Alignment);
#endif
	}

	// operator new[](unsigned long, std::align_val_t, std::nothrow_t const&)
	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib__ZnamSt11align_val_tRKSt9nothrow_t(size_t _Size, size_t _Alignment)
	{
#ifdef DMemoryManagerIsSame
		mint Size = DAlignSizeMacOS(_Size);
	#if DEnableDebugMemoryManager
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAlignedWithSizeDebug(Size, (mint)_Alignment, DMibPFile, DMibPLine, g_DebugFlags);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAlignedWithSizeDebug(Size, (mint)_Alignment, DMibPFile, DMibPLine, g_DebugFlags);
	#else
		uint8 *pMalterlibAlloc;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMalterlibAlloc = (uint8 *)DMainHeapSmall->f_AllocAligned(Size, (mint)_Alignment);
		else
#endif
			pMalterlibAlloc = (uint8 *)DMainHeapMax->f_AllocAligned(Size, (mint)_Alignment);
	#endif
		return pMalterlibAlloc;
#else
		return fg_AllocAligned(_Size, _Alignment);
#endif
	}

	// operator delete[](void*)
	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__ZdaPv(void *_pMemory)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_FreeNoSize(pMalterlibAlloc);
		else
#endif
			return DMainHeapMax->f_FreeNoSize(pMalterlibAlloc);
#else
		return fg_FreeNoSize(_pMemory);
#endif
	}

	// operator delete(void*)
	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__ZdlPv(void *_pMemory)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_FreeNoSize(pMalterlibAlloc);
		else
#endif
			return DMainHeapMax->f_FreeNoSize(pMalterlibAlloc);
#else
		return fg_FreeNoSize(_pMemory);
#endif
	}

	// operator delete[](void*, std::nothrow_t const&)
	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__ZdaPvRKSt9nothrow_t(void *_pMemory)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_FreeNoSize(pMalterlibAlloc);
		else
#endif
			return DMainHeapMax->f_FreeNoSize(pMalterlibAlloc);
#else
		return fg_FreeNoSize(_pMemory);
#endif
	}

	//operator delete[](void*, std::align_val_t)
	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__ZdaPvSt11align_val_t(void *_pMemory, size_t _Alignment)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_FreeNoSize(pMalterlibAlloc);
		else
#endif
			return DMainHeapMax->f_FreeNoSize(pMalterlibAlloc);
#else
		return fg_FreeNoSize(_pMemory);
#endif
	}

	// operator delete[](void*, std::align_val_t, std::nothrow_t const&)
	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__ZdaPvSt11align_val_tRKSt9nothrow_t(void *_pMemory, size_t _Alignment)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_FreeNoSize(pMalterlibAlloc);
		else
#endif
			return DMainHeapMax->f_FreeNoSize(pMalterlibAlloc);
#else
		return fg_FreeNoSize(_pMemory);
#endif
	}

	// operator delete[](void*, unsigned long)
	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__ZdaPvm(void *_pMemory, size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_Free(pMalterlibAlloc, _Size);
		else
#endif
			return DMainHeapMax->f_Free(pMalterlibAlloc, _Size);
#else
		return fg_Free(_pMemory, _Size);
#endif
	}

	// operator delete[](void*, unsigned long, std::align_val_t)
	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__ZdaPvmSt11align_val_t(void *_pMemory, size_t _Size, size_t _Alignment)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		mint Size = fg_AlignUp(DAlignSizeMacOS(_Size), (mint)_Alignment);
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_Free(pMalterlibAlloc, Size);
		else
#endif
			return DMainHeapMax->f_Free(pMalterlibAlloc, Size);
#else
		return fg_Free(_pMemory, _Size);
#endif
	}

	// operator delete(void*, std::nothrow_t const&)
	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__ZdlPvRKSt9nothrow_t(void *_pMemory)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_FreeNoSize(pMalterlibAlloc);
		else
#endif
			return DMainHeapMax->f_FreeNoSize(pMalterlibAlloc);
#else
		return fg_FreeNoSize(_pMemory);
#endif
	}

	// operator delete(void*, std::align_val_t)
	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__ZdlPvSt11align_val_t(void *_pMemory, size_t _Alignment)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_FreeNoSize(pMalterlibAlloc);
		else
#endif
			return DMainHeapMax->f_FreeNoSize(pMalterlibAlloc);
#else
		return fg_FreeNoSize(_pMemory);
#endif
	}

	// operator delete(void*, std::align_val_t, std::nothrow_t const&)
	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__ZdlPvSt11align_val_tRKSt9nothrow_t(void *_pMemory, size_t _Alignment)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_FreeNoSize(pMalterlibAlloc);
		else
#endif
			return DMainHeapMax->f_FreeNoSize(pMalterlibAlloc);
#else
		return fg_FreeNoSize(_pMemory);
#endif
	}

	// operator delete(void*, unsigned long)
	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__ZdlPvm(void *_pMemory, size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_Free(pMalterlibAlloc, _Size);
		else
#endif
			return DMainHeapMax->f_Free(pMalterlibAlloc, _Size);
#else
		return fg_Free(_pMemory, _Size);
#endif
	}

	// operator delete(void*, unsigned long, std::align_val_t)
	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib__ZdlPvmSt11align_val_t(void *_pMemory, size_t _Size, size_t _Alignment)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return;
		mint Size = fg_AlignUp(DAlignSizeMacOS(_Size), (mint)_Alignment);
		uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_Free(pMalterlibAlloc, Size);
		else
#endif
			return DMainHeapMax->f_Free(pMalterlibAlloc, Size);
#else
		return fg_Free(_pMemory, _Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport size_t fg_Malterlib_malloc_size(const void *_pMemory)
	{
#ifdef DMemoryManagerIsSame
		if (!_pMemory)
			return 0;
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_size(_pMemory);
		if (g_bOnlyDefaultZone)
		{
			if (fg_IsInvalidRegion(_pMemory))
				return 0;

#ifdef DMemoryManagerIsSame
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_Size(_pMemory);
		else
#endif
			return DMainHeapMax->f_Size(_pMemory);
#else
			return fg_Size(_pMemory);
#endif
		}

		// Need safe because objc stupidly relies on being able to check if it's a real memory block
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
		{
			CMemoryManagerSmall *pMemoryManager;
			if (g_bHasForeignZones)
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
				if (!pMemoryManager)
				{
					mint Size;
					auto pZone = fg_GetForeignZone(_pMemory, &Size);
					if (!pZone)
						return 0;
					return Size;
				}
			}
			else
			{
				if (fg_IsInvalidRegion(_pMemory))
					return 0;
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
			}
			if (!pMemoryManager)
				return 0;
			return pMemoryManager->f_Size(_pMemory);
		}
		else
#endif
		{
			CMemoryManagerMax *pMemoryManager;
			if (g_bHasForeignZones)
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerMax>(_pMemory);
				if (!pMemoryManager)
				{
					mint Size;
					auto pZone = fg_GetForeignZone(_pMemory, &Size);
					if (!pZone)
						return 0;
					return Size;
				}
			}
			else
			{
				if (fg_IsInvalidRegion(_pMemory))
					return 0;
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerMax>(_pMemory);
			}
			if (!pMemoryManager)
				return 0;
			return pMemoryManager->f_Size(_pMemory);
		}
#else
		return g_OriginalFunctions.malloc_size(_pMemory);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport size_t fg_Malterlib_malloc_good_size(size_t _Size)
	{
#ifdef DMemoryManagerIsSame
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_SizePadded(_Size);
		else
#endif
			return DMainHeapMax->f_SizePadded(_Size);
#else
		return g_OriginalFunctions.malloc_good_size(_Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport malloc_zone_t *fg_Malterlib_malloc_default_zone()
	{
#ifdef DMemoryManagerIsSame
		return (malloc_zone_t *)&g_MalterlibMallocZone;
#else
		return g_OriginalFunctions.malloc_default_zone();
#endif
	}

	malloc_introspection_t_10_7 g_MalterlibMallocZoneZoneIntrospection =
		{
			[](task_t task, void *, unsigned type_mask, vm_address_t zone_address, memory_reader_t reader, vm_range_recorder_t recorder) -> kern_return_t /* enumerates all the malloc pointers in use */
			{
				return KERN_FAILURE;
			}
			, [](malloc_zone_t *_pZone, size_t _Size) -> size_t
			{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if (g_bMainHeapIsSmall)
				{
					auto *pZone = (CMemoryManagerZoneSmall *)_pZone;
					return pZone->m_MemoryManager.f_SizePadded(_Size);
				}
				else
#endif
				{
					auto *pZone = (CMemoryManagerZoneMax *)_pZone;
					return pZone->m_MemoryManager.f_SizePadded(_Size);
				}
			}
			, [](malloc_zone_t *_pZone) -> boolean_t /* Consistency checker */
			{
				return true;
			}
			, [](malloc_zone_t *_pZone, boolean_t verbose) /* Prints zone  */
			{
			}
			, [](malloc_zone_t *_pZone, void *address) /* Enables logging of activity */
			{
			}
			, [](malloc_zone_t *_pZone) /* Forces locking zone */
			{
			}
			, [](malloc_zone_t *_pZone) /* Forces unlocking zone */
			{
			}
			, [](malloc_zone_t *_pZone, malloc_statistics_t *_pStats) /* Fills statistics */
			{
				fg_MemClear(*_pStats);
			}
			, [](malloc_zone_t *_pZone) -> boolean_t /* Are any zone locks held */
			{
				return false;
			}
			, [](malloc_zone_t *_pZone) -> boolean_t
			{
				return false;
			}
			, [](malloc_zone_t *_pZone)
			{
			}
			, [](malloc_zone_t *_pZone, void *memory)
			{
			}
			, [](malloc_zone_t *_pZone, void (^report_discharged)(void *memory, void *info))
			{
			}
		}
	;

	assure_used DMibMalterlibOverrideMallocExport malloc_zone_t *fg_Malterlib_malloc_create_zone(vm_size_t start_size, unsigned flags)
	{
#ifdef DMemoryManagerIsSame
		auto fCreateZone = [&]<typename tf_CZone>()
			{
				CMemoryManagerConfig Config;
		#ifndef DFullArenasForSecondary
				Config.m_nMaxArenas = 1; // For these zones don't waste address space, chances are they will be single thread use anyways
		#endif
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if constexpr (NTraits::TCIsSame<tf_CZone, CMemoryManagerZoneSmall>::mc_Value)
					Config.m_Magic = DMainHeapSmall->f_GetMagic();
				else
#endif
					Config.m_Magic = DMainHeapMax->f_GetMagic();

				NStorage::TCUniquePointer<tf_CZone> pMemoryManager = fg_Construct(Config);

		#ifdef DFullArenasForSecondary
				pMemoryManager->m_MemoryManager.f_CanDoLazyCheckout();
		#endif


				pMemoryManager->m_MallocZone =
					{
						nullptr
						, nullptr
						, [](malloc_zone_t *_pZone, const void *_pMemory) -> size_t // size
						{
							if (!_pMemory)
								return 0;
							auto *pZone = (tf_CZone *)_pZone;
							if (g_bForeignZone) [[unlikely]]
								return fg_Malterlib_Safe_GetSize(pZone, _pMemory);
							return pZone->m_MemoryManager.f_TrySize(_pMemory);
						}
						, [](malloc_zone_t *_pZone, size_t _Size) -> void * // malloc
						{
							auto *pZone = (tf_CZone *)_pZone;
							mint Size = DAlignSizeMacOS(_Size);
							DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
							uint8 *pMalterlibAlloc = (uint8 *)pZone->m_MemoryManager.f_AllocAligned(Size, 1);
							return pMalterlibAlloc;
						}
						, [](malloc_zone_t *_pZone, size_t _nItems, size_t _Size) -> void *
						{
							auto *pZone = (tf_CZone *)_pZone;
							mint Size = DAlignSizeMacOS(_Size * _nItems);
							DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
							uint8 *pMalterlibAlloc = (uint8 *)pZone->m_MemoryManager.f_AllocAligned(Size, 1);
							fg_MemClear(pMalterlibAlloc, Size);
							return pMalterlibAlloc;
						}
						, [](malloc_zone_t *_pZone, size_t _Size) -> void *
						{
							auto *pZone = (tf_CZone *)_pZone;
							mint Size = fg_AlignUp(fg_Max(_Size, 1), NSys::NPrivate::g_PageSize);
							DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
							uint8 *pMalterlibAlloc = (uint8 *)pZone->m_MemoryManager.f_AllocAligned(Size, NSys::NPrivate::g_PageSize);
							fg_MemClear(pMalterlibAlloc, Size);
							return pMalterlibAlloc;
						}
						, [](malloc_zone_t *_pZone, void *_pMemory) -> void
						{
							if (!_pMemory)
								return;
							auto *pZone = (tf_CZone *)_pZone;
							uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
							return pZone->m_MemoryManager.f_FreeNoSize(pMalterlibAlloc);
						}
						, [](malloc_zone_t *_pZone, void *_pMemory, size_t _Size) -> void *
						{
							uint8 *pMalterlibAlloc = (uint8 *)_pMemory;
							mint Size = DAlignSizeMacOS(_Size);
							auto *pZone = (tf_CZone *)_pZone;
							DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
							pMalterlibAlloc = (uint8 *)pZone->m_MemoryManager.f_Resize(pMalterlibAlloc, Size, 0);
							return pMalterlibAlloc;
						}
						, [](malloc_zone_t *_pZone)
						{
							NStorage::TCUniquePointer<tf_CZone> pMemoryManager = fg_Explicit((tf_CZone *)_pZone);
							if (g_bForeignZone) [[unlikely]]
								g_OriginalFunctions.malloc_zone_unregister(pMemoryManager->f_GetMallocZone());
							else
							{
								auto &State = *g_GlobalState;
								DMibLock(State.m_ZoneListLock);
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
								if constexpr (NTraits::TCIsSame<tf_CZone, CMemoryManagerZoneSmall>::mc_Value)
								{
									State.m_ZoneListSmall.f_Remove(*pMemoryManager);
									g_bOnlyDefaultZone = State.m_ZoneListSmall.f_IsEmpty() && !g_bHasForeignZones;
								}
								else
#endif
								{
									State.m_ZoneListMax.f_Remove(*pMemoryManager);
									g_bOnlyDefaultZone = State.m_ZoneListMax.f_IsEmpty() && !g_bHasForeignZones;
								}
							}
							pMemoryManager.f_Clear();
						}
						, nullptr // "DefaultMallocZone" // "Malterlib malloc zone"
						, [](struct _malloc_zone_t *_pZone, size_t size, void **results, unsigned num_requested) -> unsigned
						{
							if (num_requested)
								return 0;
							mint Size = DAlignSizeMacOS(size);
							auto *pZone = (tf_CZone *)_pZone;
							DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);

							mint nAllocated = 0;
						#if DEnableDebugMemoryManager
							pZone->m_MemoryManager.f_AllocBatchDebug
								(
									Size
									, 1
									, [&](void *_pAlloc, mint _Size) -> bool
									{
										results[nAllocated] = _pAlloc;
										++nAllocated;
										return nAllocated < num_requested;
									}
									, DMibPFile
									, DMibPLine
									, g_DebugFlags
								)
							;
						#else
							pZone->m_MemoryManager.f_AllocBatch
								(
									Size
									, 1
									, [&](void *_pAlloc, mint _Size) -> bool
									{
										results[nAllocated] = _pAlloc;
										++nAllocated;
										return nAllocated < num_requested;
									}
								)
							;
						#endif

							return num_requested;
						}
						, [](struct _malloc_zone_t *_pZone, void **to_be_freed, unsigned num_to_be_freed)
						{
							if (num_to_be_freed == 0)
								return;
							DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
							auto *pZone = (tf_CZone *)_pZone;
							auto Checkout = fg_GetSys()->f_MemoryManager_Checkout();
							for (mint iToFree = 0; iToFree < num_to_be_freed; ++iToFree)
								pZone->m_MemoryManager.f_FreeNoSize(to_be_freed[iToFree]);
						}
						, &g_MalterlibMallocZoneZoneIntrospection
						, 8
						, [](struct _malloc_zone_t *_pZone, size_t alignment, size_t size) -> void *
						{
							auto *pZone = (tf_CZone *)_pZone;
							mint Size = size;
							DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
						#if DEnableDebugMemoryManager
							uint8 *pMalterlibAlloc = (uint8 *)pZone->m_MemoryManager.f_AllocAlignedWithSizeDebug(Size, alignment, DMibPFile, DMibPLine, g_DebugFlags);
						#else
							uint8 *pMalterlibAlloc = (uint8 *)pZone->m_MemoryManager.f_AllocAligned(Size, alignment);
						#endif

							return pMalterlibAlloc;
						}
						, [](struct _malloc_zone_t *_pZone, void *ptr, size_t size)
						{
							if (!ptr)
								return;
							DMibMemLightweightTrackAddFlagsLowLevelScope(EMemoryReportLightweightScopeFlag_InCScope);
							auto *pZone = (tf_CZone *)_pZone;
							uint8 *pMalterlibAlloc = (uint8 *)ptr;
							pZone->m_MemoryManager.f_Free(pMalterlibAlloc, size);
						}
						, [](struct _malloc_zone_t *_pZone, size_t goal) -> size_t
						{
							auto *pZone = (tf_CZone *)_pZone;
							pZone->m_MemoryManager.f_GarbageCollect(true);
							return 0;
						}
					}
				;

				if (g_bForeignZone) [[unlikely]]
					g_OriginalFunctions.malloc_zone_register(pMemoryManager->f_GetMallocZone());
				else
				{
					auto &State = *g_GlobalState;
					DMibLock(State.m_ZoneListLock);
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
					if constexpr (NTraits::TCIsSame<tf_CZone, CMemoryManagerZoneSmall>::mc_Value)
						State.m_ZoneListSmall.f_Insert(*pMemoryManager);
					else
#endif
						State.m_ZoneListMax.f_Insert(*pMemoryManager);
					g_bOnlyDefaultZone = false;
				}

				// return g_OriginalFunctions.malloc_create_zone(start_size, flags);
				return pMemoryManager.f_Detach()->f_GetMallocZone();
			}
		;

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return fCreateZone.operator ()<CMemoryManagerZoneSmall>();
		else
#endif
			return fCreateZone.operator ()<CMemoryManagerZoneMax>();
#else
		return g_OriginalFunctions.malloc_create_zone(start_size, flags);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_destroy_zone(malloc_zone_t *_pZone)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_destroy_zone(_pZone);
		_pZone->destroy(_pZone);
#else
		return g_OriginalFunctions.malloc_destroy_zone(_pZone);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib_malloc_zone_malloc(malloc_zone_t *_pZone, size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_malloc(_pZone, _Size);
		return _pZone->malloc(_pZone, _Size);
#else
		return g_OriginalFunctions.malloc_zone_malloc(_pZone, _Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib_malloc_zone_calloc(malloc_zone_t *_pZone, size_t _nItems, size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_calloc(_pZone, _nItems, _Size);
		return _pZone->calloc(_pZone, _nItems, _Size);
#else
		return g_OriginalFunctions.malloc_zone_calloc(_pZone, _nItems, _Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib_malloc_zone_valloc(malloc_zone_t *_pZone, size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_valloc(_pZone, _Size);
		return _pZone->valloc(_pZone, _Size);
#else
		return g_OriginalFunctions.malloc_zone_valloc(_pZone, _Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_zone_free(malloc_zone_t *_pZone, void *_pMemory)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_free(_pZone, _pMemory);
		return _pZone->free(_pZone, _pMemory);
#else
		return g_OriginalFunctions.malloc_zone_free(_pZone, _pMemory);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib_malloc_zone_realloc(malloc_zone_t *_pZone, void *_pMemory, size_t _Size)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_realloc(_pZone, _pMemory, _Size);
		return _pZone->realloc(_pZone, _pMemory, _Size);
#else
		return g_OriginalFunctions.malloc_zone_realloc(_pZone, _pMemory, _Size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport malloc_zone_t *fg_Malterlib_malloc_zone_from_ptr(void const *_pMemory)
	{
#ifdef DMemoryManagerIsSame
		if (_pMemory == nullptr)
			return nullptr;

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
		{
			CMemoryManagerSmall *pMemoryManager;
			if (g_bForeignZone) [[unlikely]]
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
				if (!pMemoryManager)
					return g_OriginalFunctions.malloc_zone_from_ptr(_pMemory);
			}

			if (g_bHasForeignZones) [[unlikely]]
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
				if (pMemoryManager)
					return fg_Malterlib_ZoneFromMemoryManager<CMemoryManagerSmall>(pMemoryManager);
				return fg_GetForeignZone(_pMemory);
			}
			else
			{
				// Unfortunately we need the safe variant here, because some macOS code sends in garbage here
				if ((uint8 *)_pMemory != fg_AlignUp((uint8 *)_pMemory, 16))
					return nullptr;
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerSmall>(_pMemory);
			}
			if (pMemoryManager)
				return fg_Malterlib_ZoneFromMemoryManager<CMemoryManagerSmall>(pMemoryManager);
			return nullptr;
		}
		else
#endif
		{
			CMemoryManagerMax *pMemoryManager;
			if (g_bForeignZone) [[unlikely]]
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerMax>(_pMemory);
				if (!pMemoryManager)
					return g_OriginalFunctions.malloc_zone_from_ptr(_pMemory);
			}

			if (g_bHasForeignZones) [[unlikely]]
			{
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerMax>(_pMemory);
				if (pMemoryManager)
					return fg_Malterlib_ZoneFromMemoryManager<CMemoryManagerMax>(pMemoryManager);
				return fg_GetForeignZone(_pMemory);
			}
			else
			{
				// Unfortunately we need the safe variant here, because some macOS code sends in garbage here
				if ((uint8 *)_pMemory != fg_AlignUp((uint8 *)_pMemory, 16))
					return nullptr;
				pMemoryManager = fg_Malterlib_Safe_GetMemoryManager<CMemoryManagerMax>(_pMemory);
			}
			if (pMemoryManager)
				return fg_Malterlib_ZoneFromMemoryManager<CMemoryManagerMax>(pMemoryManager);
			return nullptr;
		}
#else
		return g_OriginalFunctions.malloc_zone_from_ptr(_pMemory);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void *fg_Malterlib_malloc_zone_memalign(malloc_zone_t *_pZone, size_t alignment, size_t size)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_memalign(_pZone, alignment, size);
		return _pZone->memalign(_pZone, alignment, size);
#else
		return g_OriginalFunctions.malloc_zone_memalign(_pZone, alignment, size);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport unsigned fg_Malterlib_malloc_zone_batch_malloc(malloc_zone_t *_pZone, size_t size, void **results, unsigned num_requested)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_batch_malloc(_pZone, size, results, num_requested);
		return _pZone->batch_malloc(_pZone, size, results, num_requested);
#else
		return g_OriginalFunctions.malloc_zone_batch_malloc(_pZone, size, results, num_requested);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_zone_batch_free(malloc_zone_t *_pZone, void **to_be_freed, unsigned num)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_batch_free(_pZone, to_be_freed, num);
#else
		return g_OriginalFunctions.malloc_zone_batch_free(_pZone, to_be_freed, num);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport malloc_zone_t *fg_Malterlib_malloc_default_purgeable_zone(void)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_default_purgeable_zone();
		return (malloc_zone_t *)&g_MalterlibMallocZone;
#else
		return g_OriginalFunctions.malloc_default_purgeable_zone();
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_make_purgeable(void *ptr)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_make_purgeable(ptr);
#else
		return g_OriginalFunctions.malloc_make_purgeable(ptr);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport int fg_Malterlib_malloc_make_nonpurgeable(void *ptr)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_make_nonpurgeable(ptr);
		return 0;
#else
		return g_OriginalFunctions.malloc_make_nonpurgeable(ptr);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_zone_register(malloc_zone_t *_pZone)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_register(_pZone);
		if (_pZone == (malloc_zone_t *)&g_MalterlibMallocZone)
			return;
		auto &State = *g_GlobalState;
		DMibLock(State.m_ZoneListLock);
		if (!g_bHasForeignZones)
		{
			State.m_ForeignZones.f_SetLen(256);
			fg_MemClear(State.m_ForeignZones.f_GetArray(), State.m_ForeignZones.f_GetLen() * sizeof(malloc_zone_t *));
			State.m_pForeignZones = State.m_ForeignZones.f_GetArray();
		}
		if (State.m_nForeignZones == 256)
			DMibPDebugBreak; // Out of zones

		DMibOverrideTrace("FOREIGN zone registered\n");

		NMib::NAtomic::fg_MemoryFence();
		mint iNextZone = State.m_nForeignZones;
		NMib::NAtomic::fg_MemoryFence();
		State.m_pForeignZones[iNextZone] = _pZone;
		NMib::NAtomic::fg_MemoryFence();
		++State.m_nForeignZones;
		NMib::NAtomic::fg_MemoryFence();
		g_bHasForeignZones = true;
		g_bOnlyDefaultZone = false;
		NMib::NAtomic::fg_MemoryFence();
#else
		g_OriginalFunctions.malloc_zone_register(_pZone);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_zone_unregister(malloc_zone_t *_pZone)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_unregister(_pZone);
		if (_pZone == (malloc_zone_t *)&g_MalterlibMallocZone)
			return;
		auto &State = *g_GlobalState;
		DMibLock(State.m_ZoneListLock);
		if (!g_bHasForeignZones)
			return;

		mint iFoundZone = TCLimitsInt<mint>::mc_Max;
		for (mint iZone = 0; iZone < State.m_nForeignZones; ++iZone)
		{
			if (State.m_pForeignZones[iZone] == _pZone)
			{
				iFoundZone = iZone;
				break;
			}
		}
		if (iFoundZone == TCLimitsInt<mint>::mc_Max)
			return;

		if (State.m_nForeignZones == 1)
		{
			g_bHasForeignZones = false;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				g_bOnlyDefaultZone = State.m_ZoneListSmall.f_IsEmpty() && !g_bHasForeignZones;
			else
#endif
				g_bOnlyDefaultZone = State.m_ZoneListMax.f_IsEmpty() && !g_bHasForeignZones;
			State.m_nForeignZones = 0;
			State.m_pForeignZones[iFoundZone] = nullptr;
			return ;
		}

		mint iLastZone = State.m_nForeignZones - 1;
		NMib::NAtomic::fg_MemoryFence();
		State.m_pForeignZones[iFoundZone] = State.m_pForeignZones[iLastZone];
		NMib::NAtomic::fg_MemoryFence();
		--State.m_nForeignZones;
		NMib::NAtomic::fg_MemoryFence();
		State.m_pForeignZones[iLastZone] = nullptr;
		NMib::NAtomic::fg_MemoryFence();
#else
		return g_OriginalFunctions.malloc_zone_unregister(_pZone);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_set_zone_name(malloc_zone_t *_pZone, const char *name)
	{
#ifdef DMemoryManagerIsSame
		if (_pZone == (malloc_zone_t *)&g_MalterlibMallocZone)
		{
			_pZone->zone_name = name;
			return;
		}
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_set_zone_name(_pZone, name);
		_pZone->zone_name = strdup(name);
#else
		return g_OriginalFunctions.malloc_set_zone_name(_pZone, name);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport const char *fg_Malterlib_malloc_get_zone_name(malloc_zone_t *_pZone)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_get_zone_name(_pZone);
		return _pZone->zone_name;
#else
		return g_OriginalFunctions.malloc_get_zone_name(_pZone);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport size_t fg_Malterlib_malloc_zone_pressure_relief(malloc_zone_t *_pZone, size_t goal)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_pressure_relief(_pZone, goal);
		if (_pZone->pressure_relief)
			return _pZone->pressure_relief(_pZone, goal);
		return 0;
#else
		return g_OriginalFunctions.malloc_zone_pressure_relief(_pZone, goal);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport kern_return_t fg_Malterlib_malloc_get_all_zones(task_t task, memory_reader_t reader, vm_address_t **addresses, unsigned *count)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_get_all_zones(task, reader, addresses, count);
		return KERN_FAILURE;
#else
		return g_OriginalFunctions.malloc_get_all_zones(task, reader, addresses, count);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_zone_print_ptr_info(void *ptr)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_print_ptr_info(ptr);
#else
		return g_OriginalFunctions.malloc_zone_print_ptr_info(ptr);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport boolean_t fg_Malterlib_malloc_zone_check(malloc_zone_t *_pZone)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_check(_pZone);
		return true;
#else
		return g_OriginalFunctions.malloc_zone_check(_pZone);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_zone_print(malloc_zone_t *_pZone, boolean_t verbose)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_print(_pZone, verbose);
#else
		return g_OriginalFunctions.malloc_zone_print(_pZone, verbose);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_zone_statistics(malloc_zone_t *_pZone, malloc_statistics_t *stats)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_statistics(_pZone, stats);
#else
		return g_OriginalFunctions.malloc_zone_statistics(_pZone, stats);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_zone_log(malloc_zone_t *_pZone, void *address)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_log(_pZone, address);
#else
		return g_OriginalFunctions.malloc_zone_log(_pZone, address);
#endif
	}

	/*
	assure_used DMibMalterlibOverrideMallocExport struct mstats fg_Malterlib_mstats(void)
	{
		return g_OriginalFunctions.mstats();
	}*/

	assure_used DMibMalterlibOverrideMallocExport boolean_t fg_Malterlib_malloc_zone_enable_discharge_checking(malloc_zone_t *_pZone)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_enable_discharge_checking(_pZone);
		return false;
#else
		return g_OriginalFunctions.malloc_zone_enable_discharge_checking(_pZone);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_zone_disable_discharge_checking(malloc_zone_t *_pZone)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_disable_discharge_checking(_pZone);
#else
		return g_OriginalFunctions.malloc_zone_disable_discharge_checking(_pZone);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_zone_discharge(malloc_zone_t *_pZone, void *memory)
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_discharge(_pZone, memory);
#else
		return g_OriginalFunctions.malloc_zone_discharge(_pZone, memory);
#endif
	}

	assure_used DMibMalterlibOverrideMallocExport void fg_Malterlib_malloc_zone_enumerate_discharged_pointers(malloc_zone_t *_pZone, void (^report_discharged)(void *memory, void *info))
	{
#ifdef DMemoryManagerIsSame
		if (g_bForeignZone) [[unlikely]]
			return g_OriginalFunctions.malloc_zone_enumerate_discharged_pointers(_pZone, report_discharged);
#else
		return g_OriginalFunctions.malloc_zone_enumerate_discharged_pointers(_pZone, report_discharged);
#endif
	}

	constinit NAtomic::TCAtomic<mint> g_Sequence = {DAggregateInit};

	assure_used mach_msg_return_t DMibMalterlibOverrideMallocExport fg_Malterlib_mach_msg
		(
			mach_msg_header_t *msg,
			mach_msg_option_t option,
			mach_msg_size_t send_size,
			mach_msg_size_t rcv_size,
			mach_port_name_t rcv_name,
			mach_msg_timeout_t timeout,
			mach_port_name_t notify
		)
	{
#ifdef DMemoryManagerIsSame
		bool bSuccessLazyReturn = false;
		if ((option & MACH_RCV_MSG) && msg->msgh_remote_port == 0)
		{
			bSuccessLazyReturn = fg_LazyReturnCheckout();
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_LazyReturnCheckoutPrevent();
			else
#endif
				DMainHeapMax->f_LazyReturnCheckoutPrevent();
		}
#endif

		auto Return = g_OriginalFunctions.mach_msg(msg, option, send_size, rcv_size, rcv_name, timeout, notify);

#ifdef DMemoryManagerIsSame
		if (bSuccessLazyReturn)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_LazyReturnCheckoutAllow();
			else
#endif
				DMainHeapMax->f_LazyReturnCheckoutAllow();
		}
#endif

		return Return;
	}

	assure_used kern_return_t DMibMalterlibOverrideMallocExport fg_Malterlib_semaphore_timedwait(semaphore_t semaphore, mach_timespec_t wait_time)
	{
#ifdef DMemoryManagerIsSame
		fg_LazyReturnCheckout();
#endif
		return g_OriginalFunctions.semaphore_timedwait(semaphore, wait_time);
	}

	assure_used kern_return_t DMibMalterlibOverrideMallocExport fg_Malterlib_semaphore_wait(semaphore_t semaphore)
	{
#ifdef DMemoryManagerIsSame
		fg_LazyReturnCheckout();
#endif
		return g_OriginalFunctions.semaphore_wait(semaphore);
	}

	assure_used kern_return_t DMibMalterlibOverrideMallocExport fg_Malterlib_semaphore_wait_signal(semaphore_t wait_semaphore, semaphore_t signal_semaphore)
	{
#ifdef DMemoryManagerIsSame
		fg_LazyReturnCheckout();
#endif
		return g_OriginalFunctions.semaphore_wait_signal(wait_semaphore, signal_semaphore);
	}

	assure_used kern_return_t DMibMalterlibOverrideMallocExport fg_Malterlib_semaphore_timedwait_signal(semaphore_t wait_semaphore, semaphore_t signal_semaphore, mach_timespec_t wait_time)
	{
#ifdef DMemoryManagerIsSame
		fg_LazyReturnCheckout();
#endif
		return g_OriginalFunctions.semaphore_timedwait_signal(wait_semaphore, signal_semaphore, wait_time);
	}

	assure_used int DMibMalterlibOverrideMallocExport fg_Malterlib___workq_kernreturn(int options, user_addr_t item, int affinity, int prio)
	{
#ifdef DMemoryManagerIsSame
		fg_LazyReturnCheckout();
#endif
		return g_OriginalFunctions.__workq_kernreturn(options, item, affinity, prio);
	}

	assure_used uint32_t DMibMalterlibOverrideMallocExport fg_Malterlib___psynch_cvwait(user_addr_t cv, uint64_t cvlsgen, uint32_t cvugen, user_addr_t mutex, uint64_t mugen, uint32_t flags, int64_t sec, uint32_t nsec)
	{
#ifdef DMemoryManagerIsSame
		fg_LazyReturnCheckout();
#endif
		return g_OriginalFunctions.__psynch_cvwait(cv, cvlsgen, cvugen, mutex, mugen, flags, sec, nsec);
	}

	assure_used int DMibMalterlibOverrideMallocExport fg_Malterlib_kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout)
	{
#ifdef DMemoryManagerIsSame
		fg_LazyReturnCheckout();
#endif
		return g_OriginalFunctions.kevent(kq, changelist, nchanges, eventlist, nevents, timeout);
	}

	assure_used int DMibMalterlibOverrideMallocExport fg_Malterlib_kevent64(int kq, const struct kevent64_s *changelist, int nchanges, struct kevent64_s *eventlist, int nevents, unsigned int flags, const struct timespec *timeout)
	{
#ifdef DMemoryManagerIsSame
		fg_LazyReturnCheckout();
#endif
		return g_OriginalFunctions.kevent64(kq, changelist, nchanges, eventlist, nevents, flags, timeout);
	}
}

extern "C"
{
#ifdef DMalterlibMemoryOverrideMacOSInitBeforeLibSystemSupport
	void fg_InterposeOverride()
	{
#define DMibMemoryInterpose_Hooks

#define DMibMemoryInterpose(d_Return, d_Function, d_Args, ...) \
		if (!NSys::g_FunctionHooks->f_SetHook((void **)&(g_OriginalFunctions.d_Function), (void *)&fg_Malterlib_##d_Function))\
		{\
			DMibOverrideTrace("Failed to hook " #d_Function ", aborting!\n", 0);\
			DMibPDebugBreak;\
		}

#define DMibMemoryInterposeCpp1(d_Return, d_Function, ...)
#define DMibMemoryInterposeCpp2(d_Return, d_Function, ...)
#define DMibMemoryInterposeCpp3(d_Return, d_Function, ...)

#include "Malterlib_Memory_SystemOverride_MacOSInterposeFunctions.h"
	}

	void fg_InterposeOverrideUnhook()
	{
#define DMibMemoryInterpose(d_Return, d_Function, d_Args, ...) \
		if (!NSys::g_FunctionHooks->f_Unhook((void **)&(g_OriginalFunctions.d_Function)))\
		{\
			DMibOverrideTrace("Failed to unhook " #d_Function ", aborting!\n", 0);\
			DMibPDebugBreak;\
		}

#define DMibMemoryInterposeCpp1(d_Return, d_Function, ...)
#define DMibMemoryInterposeCpp2(d_Return, d_Function, ...)
#define DMibMemoryInterposeCpp3(d_Return, d_Function, ...)

#include "Malterlib_Memory_SystemOverride_MacOSInterposeFunctions.h"
#undef DMibMemoryInterpose_Hooks
	}
#endif
}

#endif
