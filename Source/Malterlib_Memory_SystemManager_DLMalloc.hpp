// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#define USE_DL_PREFIX 1
#define INSECURE 1
#define MSPACES 1
#define ONLY_MSPACES 1
#define USE_SPIN_LOCKS 1
#define NO_MALLINFO 1
#define USE_LOCKS 1
#define DLMALLOC_EXPORT

#include <stddef.h>   /* for size_t */

#include "../SDK/DLMalloc/malloc.h"


#include "../SDK/DLMalloc/malloc.c"


mspace g_MainHeap;

namespace NMib
{
	ch8 const* g_pMemoryManagerName = "DLMalloc memory manager";
}
	
namespace NMib::NMemory
{
#if DMibConfig_Memory_Shims_Lightweight
	CReportMemoryLightweight *fg_ReportMemoryLightweightTo(CReportMemoryLightweight *_pMemoryReporter)
	{
		return nullptr;
	}

	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeGetFlags()
	{
		return EMemoryReportLightweightScopeFlag_None;
	}

	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeSetFlags(EMemoryReportLightweightScopeFlag _Flags)
	{
		return EMemoryReportLightweightScopeFlag_None;
	}

	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeAddFlags(EMemoryReportLightweightScopeFlag _Flags)
	{
		return EMemoryReportLightweightScopeFlag_None;
	}
#endif
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_CreateNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
	{
		g_MainHeap = create_mspace(0, true);
		mspace_mallopt(M_TRIM_THRESHOLD, 4096*1024*2);
		mspace_track_large_chunks(g_MainHeap, true);
//		NAllocator_DlMalloc::mspace_mallopt(M_MMAP_THRESHOLD, NMib::TCLimitsInt<int>::mc_Max);
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
	{
		if (fg_GetSys()->f_IsDll())
			destroy_mspace(g_MainHeap);
	}
	struct CCrossModuleImplementationExtra : public CCrossModuleImplementation
	{
		static constexpr bool mc_SupportsNonTracked = true;
		static constexpr bool mc_SupportsDebug = false;
		inline_always static void DMibCrossmoduleAPI fs_CreateMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
			DMibMemoryReportAllocatorName(g_pMemoryManagerName, g_pMemoryManagerName);
		}

		inline_always static void DMibCrossmoduleAPI fs_DestroyMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
			DMibMemoryReportAllocatorDelete(g_pMemoryManagerName, g_pMemoryManagerName);
		}

		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			void *pRet = mspace_malloc(g_MainHeap, _Size);
			_Size = fs_NonTracked_Size(_pModule, pRet);
			return pRet;
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
		{
			void *pRet = mspace_memalign(g_MainHeap, _Alignment, _Size);
			DMibFastCheck(fg_AlignUp((uint8 *)pRet, _Alignment) == (uint8 *)pRet);
			_Size = fs_NonTracked_Size(_pModule, pRet);
			return pRet;
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			void *pRet = mspace_malloc(g_MainHeap, _Size);
			return pRet;
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment)
		{
			void *pRet = mspace_memalign(g_MainHeap, _Alignment, _Size);
			DMibFastCheck(fg_AlignUp((uint8 *)pRet, _Alignment) == (uint8 *)pRet);
			return pRet;
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			void *pRet = mspace_realloc(g_MainHeap, _pMemory, _Size);
			if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
				_Size = fs_NonTracked_Size(_pModule, pRet);
			return pRet;
		}

		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			void *pRet = mspace_realloc(g_MainHeap, _pMemory, _Size);
			if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
				_Size = fs_NonTracked_Size(_pModule, pRet);
			return pRet;
		}

		inline_always static void DMibCrossmoduleAPI fs_NonTracked_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
		{
			mspace_free(g_MainHeap, _pMemory);
		}

		inline_always static void DMibCrossmoduleAPI fs_NonTracked_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			mspace_free(g_MainHeap, _pMemory);
		}

		inline_always static mint DMibCrossmoduleAPI fs_NonTracked_Size(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			return mspace_usable_size((void *)_pMemory);
		}

		inline_always static mint DMibCrossmoduleAPI fs_NonTracked_TrySize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			DMibPDebugBreak; // Not supported
			return 0;
		}

		inline_always static void * DMibCrossmoduleAPI fs_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			void *pRet = mspace_malloc(g_MainHeap, _Size);
			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		inline_always static void * DMibCrossmoduleAPI fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return fg_MemClear(fs_Alloc(_pModule, _Size), _Size);
		}

		inline_always static void * DMibCrossmoduleAPI fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			void *pRet = mspace_memalign(g_MainHeap, _Alignment, _Size);
			DMibFastCheck(fg_AlignUp((uint8 *)pRet, _Alignment) == (uint8 *)pRet);
			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, _Alignment, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}
	};

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = mspace_malloc(g_MainHeap, _Size);
		_Size = fs_Size(_pModule, pRet);
		DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
		return pRet;
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocInitZeroWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
		return fg_MemClear(fs_AllocWithSize(_pModule, _Size), _Size);
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = mspace_memalign(g_MainHeap, _Alignment, _Size);
		DMibFastCheck(fg_AlignUp((uint8 *)pRet, _Alignment) == (uint8 *)pRet);
		_Size = fs_Size(_pModule, pRet);
		DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, _Alignment, RequestedSize, _Size, 0.0, nullptr);
		return pRet;
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		DMibMemoryReportExpression(mint Size = _OldSize ? fs_SizePadded(_pModule, _OldSize) : fs_Size(_pModule, _pMemory));
		void *pRet = mspace_realloc(g_MainHeap, _pMemory, _Size);
		if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
			_Size = fs_Size(_pModule, pRet);
		DMibMemoryReportRealloc(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
		return pRet;
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		DMibMemoryReportExpression(mint Size = _OldSize ? fs_SizePadded(_pModule, _OldSize) : fs_Size(_pModule, _pMemory));
		void *pRet = mspace_realloc(g_MainHeap, _pMemory, _Size);
		if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
			_Size = fs_Size(_pModule, pRet);
		DMibMemoryReportResize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
		return pRet;
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
	{
		if (!_pMemory)
			return;
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(Size, fs_SizePadded(_pModule, _Size);
		mspace_free(g_MainHeap, _pMemory);
		DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
	{
		if (!_pMemory)
			return;
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(Size, mspace_usable_size((void *)_pMemory));
		mspace_free(g_MainHeap, _pMemory);
		DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		mint Ret = mspace_usable_size((void *)_pMemory);
		DMibMemoryReportGetSize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Ret, nullptr);
		return Ret;
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_TrySize(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
	{
		DMibPDebugBreak; // Not supported
		return 0;
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
	{
		return fg_AlignUp(_Size, fs_Granularity(_pModule));
	}

	inline_always fp32 DMibCrossmoduleAPI CCrossModuleImplementation::fs_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory)
	{
		return 0.0;
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Granularity(CMemoryManagerCrossModule *_pModule)
	{
		return 16;
	}
}
