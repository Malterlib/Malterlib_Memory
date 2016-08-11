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

	struct CMemoryManagerParams : public NMem::CDefaultMemoryManagerParams
	{
		typedef CMainHeapVirtualAllocator CAllocator;
		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_MainHeap;
	};

#	if DEnableDebugMemoryManager
		struct CMemoryManagerDebugOptions : public NMem::CMemoryManagerDebugOptionsDefault
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
		typedef NMem::TCMemoryManagerDebug<CMemoryManagerParams, false, CMemoryManagerDebugOptions> CMemoryManagerWithDebug;
#	else
		typedef NMem::TCMemoryManager<CMemoryManagerParams> CMemoryManagerWithDebug;
#	endif

#if !DMibConfig_MemoryManager_Stats_EnableCategories
	namespace NMem
	{
		typedef void CTrackedAllocationInfo;
	}
#endif
	
	typedef NMem::TCMemoryManagerTracked<CMemoryManagerWithDebug, NMem::CTrackedAllocationInfo> CMemoryManager;
	
	extern NMib::NAggregate::TCAggregateSimple<CMemoryManager> g_MainHeap;
	
	struct CMemoryManagerNonTrackedParams : public CMemoryManagerParams
	{
		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_NonTrackedMainHeap;
		typedef NMem::CAllocator_VirtualNoTracking CAllocator;
		static constexpr bool mc_bBackgroundCleanup = false; // Threading potentially recursive allocations
	};

#if DEnableDebugMemoryManager
	struct CMemoryManagerNonTrackedDebugOptions : public CMemoryManagerDebugOptions
	{
		enum
		{
			mc_bCanAllocateNonTracked = false // Threading potentially recursive allocations
		};
	};
	typedef NMem::TCMemoryManagerDebug<CMemoryManagerNonTrackedParams, false, CMemoryManagerNonTrackedDebugOptions> CMemoryManagerNonTracked;
#else
	typedef NMem::TCMemoryManager<CMemoryManagerNonTrackedParams> CMemoryManagerNonTracked;
#endif
	
	extern NMib::NAggregate::TCAggregateSimple<CMemoryManagerNonTracked> g_NonTrackedHeap;
}
