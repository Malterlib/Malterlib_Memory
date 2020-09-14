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
		static constexpr mint mc_SubSlabSize = 4096;
		typedef CMainHeapVirtualAllocator CAllocator;
		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_MainHeap;
	};

	struct CMemoryManagerParamsSmall : public NMemory::TCMemoryManagerParams<CMemoryManagerParamsSmallOverrides>
	{
	};
#endif
	struct CMemoryManagerParamsMaxOverrides : public NMemory::CDefaultMemoryManagerParams
	{
		typedef CMainHeapVirtualAllocator CAllocator;
		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_MainHeap;
	};

	struct CMemoryManagerParamsMax : public NMemory::TCMemoryManagerParams<CMemoryManagerParamsMaxOverrides>
	{
	};

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
			};
		};
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		typedef NMemory::TCMemoryManagerDebug<CMemoryManagerParamsSmall, false, CMemoryManagerDebugOptions> CMemoryManagerWithDebugSmall;
#endif
		typedef NMemory::TCMemoryManagerDebug<CMemoryManagerParamsMax, false, CMemoryManagerDebugOptions> CMemoryManagerWithDebugMax;
#	else
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		typedef NMemory::TCMemoryManager<CMemoryManagerParamsSmall> CMemoryManagerWithDebugSmall;
#endif
		typedef NMemory::TCMemoryManager<CMemoryManagerParamsMax> CMemoryManagerWithDebugMax;
#	endif

#if !DMibConfig_MemoryManager_Stats_EnableCategories
	namespace NMemory
	{
		typedef void CTrackedAllocationInfo;
	}
#endif

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	typedef NMemory::TCMemoryManagerTracked<CMemoryManagerWithDebugSmall, NMemory::CTrackedAllocationInfo> CMemoryManagerSmall;
#endif
	typedef NMemory::TCMemoryManagerTracked<CMemoryManagerWithDebugMax, NMemory::CTrackedAllocationInfo> CMemoryManagerMax;

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	extern bool g_bMainHeapIsSmall;
	extern NMib::NStorage::TCAggregateSimple<CMemoryManagerSmall> g_MainHeapSmall;
#endif
	extern NMib::NStorage::TCAggregateSimple<CMemoryManagerMax> g_MainHeapMax;

	struct CMemoryManagerNonTrackedParams : public CMemoryManagerParamsMax
	{
		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_NonTrackedMainHeap;
		typedef NMemory::CAllocator_VirtualNoTracking CAllocator;
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
	typedef NMemory::TCMemoryManagerDebug<CMemoryManagerNonTrackedParams, false, CMemoryManagerNonTrackedDebugOptions> CMemoryManagerNonTracked;
#else
	typedef NMemory::TCMemoryManager<CMemoryManagerNonTrackedParams> CMemoryManagerNonTracked;
#endif

	extern NMib::NStorage::TCAggregateSimple<CMemoryManagerNonTracked> g_NonTrackedHeap;
}
