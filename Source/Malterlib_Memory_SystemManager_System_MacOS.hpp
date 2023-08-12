// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <malloc/malloc.h>

namespace NMib
{
	ch8 const* g_pMemoryManagerName = "macOS system memory manager";
	
	malloc_zone_t *g_DefaultZone = nullptr;
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
		g_DefaultZone = malloc_default_zone();
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
	{
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
			void *pRet = g_DefaultZone->malloc(g_DefaultZone, _Size);
			_Size = g_DefaultZone->size(g_DefaultZone, pRet);
			return pRet;
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
		{
			if (_Alignment <= 16)
				return fs_NonTracked_AllocWithSize(_pModule, _Size);

			void *pRet = g_DefaultZone->memalign(g_DefaultZone, _Alignment, _Size);
			DMibFastCheck(fg_AlignUp((uint8 *)pRet, _Alignment) == (uint8 *)pRet);
			_Size = g_DefaultZone->size(g_DefaultZone, pRet);
			return pRet;
		}

		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			void *pRet = g_DefaultZone->malloc(g_DefaultZone, _Size);
			return pRet;
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment)
		{
			if (_Alignment <= 16)
				return fs_NonTracked_Alloc(_pModule, _Size);

			void *pRet = g_DefaultZone->memalign(g_DefaultZone, _Alignment, _Size);
			DMibFastCheck(fg_AlignUp((uint8 *)pRet, _Alignment) == (uint8 *)pRet);
			return pRet;
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			void *pRet = g_DefaultZone->realloc(g_DefaultZone, _pMemory, _Size);
			if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
				_Size = g_DefaultZone->size(g_DefaultZone, pRet);
			return pRet;
		}

		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			void *pRet = g_DefaultZone->realloc(g_DefaultZone, _pMemory, _Size);
			if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
				_Size = g_DefaultZone->size(g_DefaultZone, pRet);
			return pRet;
		}

		inline_always static void DMibCrossmoduleAPI fs_NonTracked_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
		{
			g_DefaultZone->free(g_DefaultZone, _pMemory);
		}

		inline_always static bool DMibCrossmoduleAPI fs_AllocHasDeterministicSize(CMemoryManagerCrossModule *_pModule)
		{
			return false;
		}

		inline_always static EMemoryManagerFeatureFlag DMibCrossmoduleAPI fs_MemoryManagerFeatures(CMemoryManagerCrossModule *_pModule)
		{
			return EMemoryManagerFeatureFlag_None;
		}

		inline_always static void DMibCrossmoduleAPI fs_NonTracked_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			g_DefaultZone->free(g_DefaultZone, _pMemory);
		}

		inline_always static mint DMibCrossmoduleAPI fs_NonTracked_Size(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			return g_DefaultZone->size(g_DefaultZone, _pMemory);
		}

		inline_always static mint DMibCrossmoduleAPI fs_NonTracked_TrySize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			return g_DefaultZone->size(g_DefaultZone, _pMemory);
		}

		inline_always static void * DMibCrossmoduleAPI fs_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			void *pRet = g_DefaultZone->malloc(g_DefaultZone, _Size);
			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

		inline_always static void * DMibCrossmoduleAPI fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return fg_MemClear(fs_Alloc(_pModule, _Size), _Size);
		}

		inline_always static void * DMibCrossmoduleAPI fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment)
		{
			if (_Alignment <= 16)
				return fs_Alloc(_pModule, _Size);

			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);
			void *pRet = g_DefaultZone->memalign(g_DefaultZone, _Alignment, _Size);
			DMibFastCheck(fg_AlignUp((uint8 *)pRet, _Alignment) == (uint8 *)pRet);
			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, _Alignment, RequestedSize, _Size, 0.0, nullptr);
			return pRet;
		}

	};
	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = g_DefaultZone->malloc(g_DefaultZone, _Size);
		_Size = g_DefaultZone->size(g_DefaultZone, pRet);
		DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
		return pRet;
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocInitZeroWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
		return fg_MemClear(fs_AllocWithSize(_pModule, _Size), _Size);
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
	{
		if (_Alignment <= 16)
			return fs_AllocWithSize(_pModule, _Size);

		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		void *pRet = g_DefaultZone->memalign(g_DefaultZone, _Alignment, _Size);
		DMibFastCheck(fg_AlignUp((uint8 *)pRet, _Alignment) == (uint8 *)pRet);
		_Size = g_DefaultZone->size(g_DefaultZone, pRet);
		DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pRet, _Alignment, RequestedSize, _Size, 0.0, nullptr);
		return pRet;
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		DMibMemoryReportExpression(mint Size = _OldSize ? fs_SizePadded(_pModule, _OldSize) : fs_Size(_pModule, _pMemory));
		void *pRet = g_DefaultZone->realloc(g_DefaultZone, _pMemory, _Size);
		if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
			_Size = g_DefaultZone->size(g_DefaultZone, pRet);
		DMibMemoryReportRealloc(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
		return pRet;
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		DMibMemoryReportExpression(mint Size = _OldSize ? fs_SizePadded(_pModule, _OldSize) : fs_Size(_pModule, _pMemory));
		void *pRet = g_DefaultZone->realloc(g_DefaultZone, _pMemory, _Size);
		if (!(_AllocFlags & EAllocationFlag_SizeNotNeeded))
			_Size = g_DefaultZone->size(g_DefaultZone, pRet);
		DMibMemoryReportResize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr, pRet, 0, RequestedSize, _Size, 0.0, nullptr);
		return pRet;
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
	{
		if (!_pMemory)
			return;

		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(Size, fs_SizePadded(_pModule, _Size));
		//if (g_DefaultZone->version >= 6 && g_DefaultZone->free_definite_size)
		//	g_DefaultZone->free_definite_size(g_DefaultZone, _pMemory, _Size);
		//else
			g_DefaultZone->free(g_DefaultZone, _pMemory);

		DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
	{
		if (!_pMemory)
			return;

		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(Size, g_DefaultZone->size(g_DefaultZone, _pMemory));
		g_DefaultZone->free(g_DefaultZone, _pMemory);
		DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		mint Ret = g_DefaultZone->size(g_DefaultZone, _pMemory);
		DMibMemoryReportGetSize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Ret, nullptr);
		return Ret;
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_TrySize(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		mint Ret = g_DefaultZone->size(g_DefaultZone, _pMemory);
		DMibMemoryReportGetSize(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Ret, nullptr);
		return Ret;
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
	{
		return malloc_good_size(_Size);
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
