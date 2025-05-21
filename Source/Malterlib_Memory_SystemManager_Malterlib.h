// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#if DMibConfig_Memory_Shims_Enable
	#include "Malterlib_Memory_SystemManager_MalterlibWithTracking.h"
#else

#include "Malterlib_Memory_MemoryManager.hpp"

#if DMibConfig_MalterlibMemoryManager_Debug && DMibConfig_MalterlibMemoryManager_Debug_Features || DMibConfig_MalterlibMemoryManager_Debug_Features == 1
#	define DEnableDebugMemoryManager 1
#else
#	define DEnableDebugMemoryManager 0
#endif

#if DEnableDebugMemoryManager
#	include "Malterlib_Memory_MemoryManager_Debug.h"
#endif

namespace NMib
{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	struct CMemoryManagerParamsSmallOverrides : NMemory::CDefaultMemoryManagerParams
	{
		static constexpr mint mc_SubSlabSize = 4096;
		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_MainHeap;
		using CAllocator = CMainHeapVirtualAllocator;
	};

	struct CMemoryManagerParamsSmall : public NMemory::TCMemoryManagerParams<CMemoryManagerParamsSmallOverrides>
	{
	};
#endif
	struct CMemoryManagerParamsMaxOverrides : public NMemory::CDefaultMemoryManagerParams
	{
		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_MainHeap;
		using CAllocator = CMainHeapVirtualAllocator;
	};

	struct CMemoryManagerParamsMax : public NMemory::TCMemoryManagerParams<CMemoryManagerParamsMaxOverrides>
	{
	};

#ifdef DMibNeedDebugException
	static constexpr bool gc_bHasExceptions = true;
#else
	static constexpr bool gc_bHasExceptions = false;
#endif

#if DEnableDebugMemoryManager
		struct CMemoryManagerDebugOptions : public NMemory::CMemoryManagerDebugOptionsDefault
		{
			enum
			{
				EDummy
	#if !DMibConfig_MalterlibMemoryManager_Debug_EnableFreedGuards
				, mc_bCheckModifyAfterFree	= false
	#endif
	#if !DMibConfig_MalterlibMemoryManager_Debug_EnableAllocatedFill
				, mc_bFillAllocated			= false
	#endif
	#if !DMibConfig_MalterlibMemoryManager_Debug_EnableStackTrace
				, mc_StackTraceDepth		= 0
	#endif
	#if !DMibConfig_MalterlibMemoryManager_Debug_EnableEdgeGuards
				, mc_nPreGuardBytes			= 0
				, mc_nPostGuardBytes		= 0
	#endif
				, mc_bCanAllocateNonTracked = false // Threading potentially recursive allocations
	#if !DMibConfig_MalterlibMemoryManager_Debug_EnableFreeValidation
				, mc_bFreeValidation		= false
	#endif
	#if !DMibConfig_MalterlibMemoryManager_Debug_EnableFreeValidation && !DMibConfig_MalterlibMemoryManager_Debug_EnableMemoryLeaks
				, mc_bEnumeration			= false
	#endif
	#if  !DMibConfig_MalterlibMemoryManager_Debug_EnableMemoryLeaks
				, mc_bTraceLeaks			= false
	#endif
	#if  DMibConfig_MalterlibMemoryManager_Debug_EnableAssertOnMemoryLeak
				, mc_bAssertOnMemoryLeak	= true
	#endif
			};
		};
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		using CMemoryManagerSmall = NMemory::TCMemoryManagerDebug<CMemoryManagerParamsSmall, gc_bHasExceptions, CMemoryManagerDebugOptions>;
#endif
		using CMemoryManagerMax = NMemory::TCMemoryManagerDebug<CMemoryManagerParamsMax, gc_bHasExceptions, CMemoryManagerDebugOptions>;
#	else
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		using CMemoryManagerSmall = NMemory::TCMemoryManager<CMemoryManagerParamsSmall>;
#endif
		using CMemoryManagerMax = NMemory::TCMemoryManager<CMemoryManagerParamsMax>;
#	endif
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	extern bool g_bMainHeapIsSmall;
	extern NMib::NStorage::TCAggregateSimple<CMemoryManagerSmall> g_MainHeapSmall;
#endif
	extern NMib::NStorage::TCAggregateSimple<CMemoryManagerMax> g_MainHeapMax;
}

#endif
