// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <mimalloc.h>

namespace NMib
{
	ch8 const* g_pMemoryManagerName = "mimalloc memory manager";
}

#define DUseAlignedAlloc 1

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
	void DMibCrossmoduleAPI CCrossModuleImplementation::fs_CreateNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
	{
		mi_process_init();
	}

	void DMibCrossmoduleAPI CCrossModuleImplementation::fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
	{
#if !defined(DMibMemoryOverrideDll)
		if (fg_GetSys()->f_IsDll())
			mi_process_done();
		else
#endif
		{
#ifndef DMibConfig_HeapNeverDestroyed
			if (!g_bMemoryManagerNeededAfterDestroy)
				mi_process_done();
#endif
		}
	}

	struct CCrossModuleImplementationExtra : public CCrossModuleImplementation
	{
		static constexpr bool mc_SupportsNonTracked = true;
		static constexpr bool mc_SupportsDebug = false;
		inline_always static void DMibCrossmoduleAPI fs_CreateMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
			DMibMemoryReportAllocatorName(g_pMemoryManagerName, g_pMemoryManagerName);
			mi_process_init();
		}

		inline_always static void DMibCrossmoduleAPI fs_DestroyMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
			DMibMemoryReportAllocatorDelete(g_pMemoryManagerName, g_pMemoryManagerName);
		}

		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			auto *pRet = mi_malloc(_Size);
			_Size = mi_usable_size(pRet);
			return pRet;
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
		{
#if DUseAlignedAlloc
			auto *pRet = mi_malloc_aligned(_Size, _Alignment);
#else
			auto *pRet = mi_malloc(_Size);
#endif
			_Size = mi_usable_size(pRet);
			return pRet;
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			auto *pRet = mi_malloc(_Size);
			return pRet;
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment)
		{
#if DUseAlignedAlloc
			auto *pRet = mi_malloc_aligned(_Size, _Alignment);
#else
			auto *pRet = mi_malloc(_Size);
#endif
			return pRet;
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			auto *pRet = mi_reallocf(_pMemory, _Size);
			if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
				_Size = mi_usable_size(pRet);
			return pRet;
		}

		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			auto *pRet = mi_reallocf(_pMemory, _Size);
			if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
				_Size = mi_usable_size(pRet);
			return pRet;
		}

		inline_always static void DMibCrossmoduleAPI fs_NonTracked_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
		{
			mi_free(_pMemory);
		}

		inline_always static void DMibCrossmoduleAPI fs_NonTracked_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			mi_free(_pMemory);
		}

		inline_always static mint DMibCrossmoduleAPI fs_NonTracked_Size(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			return mi_usable_size((void *)_pMemory);
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
			auto *pRet = mi_malloc(_Size);
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
#if DUseAlignedAlloc
			auto *pRet = mi_malloc_aligned(_Size, _Alignment);
#else
			auto *pRet = mi_malloc(_Size);
#endif
			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, _Alignment, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}
	};

	void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		auto *pRet = mi_malloc(_Size);
		_Size = mi_usable_size(pRet);
		DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
		return pRet;
	}

	void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocInitZeroWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
		return fg_MemClear(fs_AllocWithSize(_pModule, _Size), _Size);
	}

	void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
#if DUseAlignedAlloc
		auto *pRet = mi_malloc_aligned(_Size, _Alignment);
#else
		auto *pRet = mi_malloc(_Size);
#endif
		_Size = mi_usable_size(pRet);
		DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, _Alignment, RequestedSize, _Size, 0.0, nullptr);
		return pRet;
	}

	void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		DMibMemoryReportExpression(mint Size = _OldSize ? fs_SizePadded(_pModule, _OldSize) : fs_Size(_pModule, _pMemory));
		auto *pRet = mi_reallocf(_pMemory, _Size);
		if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
			_Size = mi_usable_size(pRet);
		DMibMemoryReportRealloc(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
		return pRet;
	}

	void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		DMibMemoryReportExpression(mint Size = _OldSize ? fs_SizePadded(_pModule, _OldSize) : fs_Size(_pModule, _pMemory));
		auto *pRet = mi_reallocf(_pMemory, _Size);
		if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
			_Size = mi_usable_size(pRet);
		DMibMemoryReportResize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
		return pRet;
	}

	void DMibCrossmoduleAPI CCrossModuleImplementation::fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
	{
		if (!_pMemory)
			return;
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(Size, fs_SizePadded(_pModule, _Size));
		mi_free(_pMemory);
		DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
	}

	void DMibCrossmoduleAPI CCrossModuleImplementation::fs_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
	{
		if (!_pMemory)
			return;
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(Size, mi_usable_size(_pMemory));
		mi_free(_pMemory);
		DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
	}

	mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		mint Ret = mi_usable_size((void *)_pMemory);
		DMibMemoryReportGetSize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Ret, nullptr);
		return Ret;
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_TrySize(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
	{
		DMibPDebugBreak; // Not supported
		return 0;
	}

	mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
	{
		return mi_good_size(_Size);
	}

	fp32 DMibCrossmoduleAPI CCrossModuleImplementation::fs_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory)
	{
		return 0.0;
	}

	mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Granularity(CMemoryManagerCrossModule *_pModule)
	{
		return 16;
	}
}
