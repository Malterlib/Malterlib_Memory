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
	struct CMemoryManagerParams : public NMem::CDefaultMemoryManagerParams
	{
		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_MainHeap;
		typedef CMainHeapVirtualAllocator CAllocator;
	};

#if DEnableDebugMemoryManager
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
			};
		};
		using CMemoryManager = NMem::TCMemoryManagerDebug<CMemoryManagerParams, false, CMemoryManagerDebugOptions>;
#	else
		using CMemoryManager = NMem::TCMemoryManager<CMemoryManagerParams>;
#	endif
	extern NMib::NAggregate::TCAggregateSimple<CMemoryManager> g_MainHeap;
}

#endif
