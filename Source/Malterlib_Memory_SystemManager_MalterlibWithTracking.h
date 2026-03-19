// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include "Malterlib_Memory_MemoryManager.hpp"
#include "Malterlib_Memory_MemoryManager_Tracked.h"

#if DMibConfig_MalterlibMemoryManager_Debug && DMibConfig_MalterlibMemoryManager_Debug_Features || DMibConfig_MalterlibMemoryManager_Debug_Features == 1
#	define DEnableDebugMemoryManager 1
#else
#	define DEnableDebugMemoryManager 0
#endif

#if DEnableDebugMemoryManager
#	include "Malterlib_Memory_MemoryManager_Debug.h"
#endif

#include "Malterlib_Memory_Reporter_CategoriesInterface.h"

namespace NMib
{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	struct CMemoryManagerParamsSmallOverrides : public NMemory::CDefaultMemoryManagerParams
	{
		static constexpr umint mc_SubSlabSize = 4096;
		using CAllocator = CMainHeapVirtualAllocator;
		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_MainHeap;
	};

	struct CMemoryManagerParamsSmall : public NMemory::TCMemoryManagerParams<CMemoryManagerParamsSmallOverrides>
	{
	};
#endif
	struct CMemoryManagerParamsMaxOverrides : public NMemory::CDefaultMemoryManagerParams
	{
		using CAllocator = CMainHeapVirtualAllocator;
		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_MainHeap;
	};

	struct CMemoryManagerParamsMax : public NMemory::TCMemoryManagerParams<CMemoryManagerParamsMaxOverrides>
	{
	};

#ifdef DMibNeedDebugException
	static constexpr bool gc_bHasExceptions = true;
#else
	static constexpr bool gc_bHasExceptions = false;
#endif


#	if DEnableDebugMemoryManager
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
		using CMemoryManagerWithDebugSmall = NMemory::TCMemoryManagerDebug<CMemoryManagerParamsSmall, gc_bHasExceptions, CMemoryManagerDebugOptions>;
#endif
		using CMemoryManagerWithDebugMax = NMemory::TCMemoryManagerDebug<CMemoryManagerParamsMax, gc_bHasExceptions, CMemoryManagerDebugOptions>;
#	else
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		using CMemoryManagerWithDebugSmall = NMemory::TCMemoryManager<CMemoryManagerParamsSmall>;
#endif
		using CMemoryManagerWithDebugMax = NMemory::TCMemoryManager<CMemoryManagerParamsMax>;
#	endif

#if !DMibConfig_MemoryManager_Stats_EnableCategories
	namespace NMemory
	{
		using CTrackedAllocationInfo = void;
	}
#endif

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	using CMemoryManagerSmall = NMemory::TCMemoryManagerTracked<CMemoryManagerWithDebugSmall, NMemory::CTrackedAllocationInfo> ;
#endif
	using CMemoryManagerMax = NMemory::TCMemoryManagerTracked<CMemoryManagerWithDebugMax, NMemory::CTrackedAllocationInfo>;

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	extern bool g_bMainHeapIsSmall;
	extern NMib::NStorage::TCAggregateSimple<CMemoryManagerSmall> g_MainHeapSmall;
#endif
	extern NMib::NStorage::TCAggregateSimple<CMemoryManagerMax> g_MainHeapMax;

	struct CMemoryManagerNonTrackedParams : public CMemoryManagerParamsMax
	{
		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_NonTrackedMainHeap;
		using CAllocator = NMemory::CAllocator_VirtualNoTracking;
		static constexpr bool mc_bBackgroundCleanup = false; // Threading potentially recursive allocations
	};

#if DEnableDebugMemoryManager
	struct CMemoryManagerNonTrackedDebugOptions : public CMemoryManagerDebugOptions
	{
		enum
		{
			mc_bCanAllocateNonTracked = false // Threading potentially recursive allocations
#ifdef DPlatformFamily_Linux
			, mc_StackTraceDepth = 0
#endif
		};
	};
	using CMemoryManagerNonTracked = NMemory::TCMemoryManagerDebug<CMemoryManagerNonTrackedParams, false, CMemoryManagerNonTrackedDebugOptions> ;
#else
	using CMemoryManagerNonTracked = NMemory::TCMemoryManager<CMemoryManagerNonTrackedParams>;
#endif

	extern NMib::NStorage::TCAggregateSimple<CMemoryManagerNonTracked> g_NonTrackedHeap;
}
