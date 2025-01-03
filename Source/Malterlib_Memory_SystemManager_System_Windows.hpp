// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#ifdef DPlatformFamily_macOS
#include <malloc/malloc.h>
#endif

#ifdef DPlatformFamily_Windows
#include <windows.h>
#endif

HANDLE g_pMainHeap = nullptr;

namespace NMib
{
	ch8 const* g_pMemoryManagerName = "Windows system memory manager";
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
#ifndef DMibSanitizerEnabled_Address
		//g_pMainHeap = HeapCreate(0, 0);
		g_pMainHeap = GetProcessHeap();
		ULONG Enable = 3;
		HeapSetInformation(g_pMainHeap, HeapCompatibilityInformation, &Enable, sizeof(Enable));
#endif
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
	{
		//if (fg_GetSys()->f_IsDll())
		//	HeapDestroy(g_pMainHeap);
	}

	struct alignas(16) CAllocationHeader
	{
		uint8 *m_pMemory = nullptr;
		mint m_Size = 0;
	};

	namespace
	{
		inline_always mint fg_Local_Size(void *_pMemory)
		{
			if (!_pMemory)
				return 0;

			CAllocationHeader *pHeader = ((CAllocationHeader *)_pMemory) - 1;

			mint Ret = (pHeader->m_pMemory + pHeader->m_Size) - (uint8 *)_pMemory;
			return Ret;
		}

		inline_always mint fg_Local_SizeFull(void *_pMemory)
		{
			if (!_pMemory)
				return 0;

			CAllocationHeader *pHeader = ((CAllocationHeader *)_pMemory) - 1;

			return pHeader->m_Size;
		}

		inline_always void *fg_Local_Alloc(mint _Size, mint _Alignment)
		{
			mint Alignment = fg_AlignUp(_Alignment, 16);
			mint Size = fg_AlignUp(_Size, Alignment) + sizeof(CAllocationHeader) + (Alignment - 16);
#ifdef DMibSanitizerEnabled_Address
			uint8 *pMemory = (uint8 *)malloc(Size);
#else
			uint8 *pMemory = (uint8 *)HeapAlloc(g_pMainHeap, 0, Size);
#endif

			uint8 *pReturn = fg_AlignUp(pMemory + sizeof(CAllocationHeader), Alignment);
			new (pReturn - sizeof(CAllocationHeader)) CAllocationHeader{.m_pMemory = pMemory, .m_Size = Size};

			return pReturn;
		}

		inline_always void *fg_Local_AllocWithSize(mint &_Size, mint _Alignment)
		{
			mint Alignment = fg_AlignUp(_Alignment, 16);
			mint Size = fg_AlignUp(_Size, Alignment) + sizeof(CAllocationHeader) + (Alignment - 16);
#ifdef DMibSanitizerEnabled_Address
			uint8 *pMemory = (uint8 *)malloc(Size);
			Size = _msize(pMemory);
#else
			uint8 *pMemory = (uint8 *)HeapAlloc(g_pMainHeap, 0, Size);
			Size = HeapSize(g_pMainHeap, 0, pMemory);
#endif
			uint8 *pReturn = fg_AlignUp(pMemory + sizeof(CAllocationHeader), Alignment);
			new (pReturn - sizeof(CAllocationHeader)) CAllocationHeader{.m_pMemory = pMemory, .m_Size = Size};

			_Size = (pMemory + Size) - pReturn;

			return pReturn;
		}

		inline_always void *fg_Local_ReAlloc(void *_pOld, mint _Size, mint _Alignment)
		{
			if (!_pOld)
				return fg_Local_Alloc(_Size, _Alignment);

			CAllocationHeader *pOldHeader = ((CAllocationHeader *)_pOld) - 1;
			auto OldHeader = *pOldHeader;

			mint Alignment = fg_AlignUp(_Alignment, 16);
			mint Size = fg_AlignUp(_Size, Alignment) + sizeof(CAllocationHeader) + (Alignment - 16);
#ifdef DMibSanitizerEnabled_Address
			uint8 *pMemory = (uint8 *)realloc(OldHeader.m_pMemory, Size);
#else
			uint8 *pMemory = (uint8 *)HeapReAlloc(g_pMainHeap, 0, OldHeader.m_pMemory, Size);
#endif
			uint8 *pReturn = fg_AlignUp(pMemory + sizeof(CAllocationHeader), Alignment);
			new (pReturn - sizeof(CAllocationHeader)) CAllocationHeader{.m_pMemory = pMemory, .m_Size = Size};

			return pReturn;
		}

		inline_always void *fg_Local_ReAllocWithSize(void *_pOld, mint &_Size, mint _Alignment)
		{
			if (!_pOld)
				return fg_Local_AllocWithSize(_Size, _Alignment);

			CAllocationHeader *pOldHeader = ((CAllocationHeader *)_pOld) - 1;
			auto OldHeader = *pOldHeader;

			mint Alignment = fg_AlignUp(_Alignment, 16);
			mint Size = fg_AlignUp(_Size, Alignment) + sizeof(CAllocationHeader) + (Alignment - 16);
#ifdef DMibSanitizerEnabled_Address
			uint8 *pMemory = (uint8 *)realloc(OldHeader.m_pMemory, Size);
			Size = _msize(pMemory);
#else
			uint8 *pMemory = (uint8 *)HeapReAlloc(g_pMainHeap, 0, OldHeader.m_pMemory, Size);
			Size = HeapSize(g_pMainHeap, 0, pMemory);
#endif

			uint8 *pReturn = fg_AlignUp(pMemory + sizeof(CAllocationHeader), Alignment);
			new (pReturn - sizeof(CAllocationHeader)) CAllocationHeader{.m_pMemory = pMemory, .m_Size = Size};

			return pReturn;
		}

		inline_always void fg_Local_Free(void *_pMemory)
		{
			if (!_pMemory)
				return;

			CAllocationHeader *pHeader = ((CAllocationHeader *)_pMemory) - 1;

#ifdef DMibSanitizerEnabled_Address
			free(pHeader->m_pMemory);
#else
			HeapFree(g_pMainHeap, 0, pHeader->m_pMemory);
#endif
		}

		inline_always mint fg_Local_Size(void const *_pMemory)
		{
			if (!_pMemory)
				return 0;

			CAllocationHeader *pHeader = ((CAllocationHeader *)_pMemory) - 1;

			return (pHeader->m_pMemory + pHeader->m_Size) - (uint8 *)_pMemory;
		}
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
			return fg_Local_AllocWithSize(_Size, 16);
		}

		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
		{
			return fg_Local_AllocWithSize(_Size, _Alignment);
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return fg_Local_Alloc(_Size, 16);
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment)
		{
			return fg_Local_Alloc(_Size, _Alignment);
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			if (_AllocFlags & EAllocationFlag_SizeNotNeeded)
				return fg_Local_ReAlloc(_pMemory, _Size, 16);
			else
				return fg_Local_ReAllocWithSize(_pMemory, _Size, 16);
		}

		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			if (_AllocFlags & EAllocationFlag_SizeNotNeeded)
				return fg_Local_ReAlloc(_pMemory, _Size, 16);
			else
				return fg_Local_ReAllocWithSize(_pMemory, _Size, 16);
		}

		inline_always static void DMibCrossmoduleAPI fs_NonTracked_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
		{
			fg_Local_Free(_pMemory);
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
			fg_Local_Free(_pMemory);
		}

		inline_always static mint DMibCrossmoduleAPI fs_NonTracked_Size(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			return fg_Local_Size(_pMemory);
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

			auto *pReturn = fg_Local_Alloc(_Size, 16);

			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pReturn, 16, RequestedSize, _Size, fg_Local_SizeFull(pReturn) - RequestedSize, nullptr);

			return pReturn;
		}

		inline_always static void * DMibCrossmoduleAPI fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return fg_MemClear(fs_Alloc(_pModule, _Size), _Size);
		}

		inline_always static void * DMibCrossmoduleAPI fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment)
		{
			DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
			DMibMemoryReportSaveVar(RequestedSize, _Size);

			auto *pReturn = fg_Local_Alloc(_Size, _Alignment);

			DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pReturn, _Alignment, RequestedSize, _Size, fg_Local_SizeFull(pReturn) - RequestedSize, nullptr);

			return pReturn;
		}
	};

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);

		auto *pReturn = fg_Local_AllocWithSize(_Size, 16);

		DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pReturn, 16, RequestedSize, _Size, fg_Local_SizeFull(pReturn) - RequestedSize, nullptr);

		return pReturn;
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocInitZeroWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
		return fg_MemClear(fs_AllocWithSize(_pModule, _Size), _Size);
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);

		auto *pReturn = fg_Local_AllocWithSize(_Size, _Alignment);

		DMibMemoryReportAlloc(g_pMemoryManagerName, g_pMemoryManagerName, pReturn, _Alignment, RequestedSize, _Size, fg_Local_SizeFull(pReturn) - RequestedSize, nullptr);

		return pReturn;
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		DMibMemoryReportExpression(mint OldSize = fg_Local_Size(_pMemory));

		void *pReturn;
		if (_AllocFlags & EAllocationFlag_SizeNotNeeded)
			pReturn = fg_Local_ReAlloc(_pMemory, _Size, 16);
		else
			pReturn = fg_Local_ReAllocWithSize(_pMemory, _Size, 16);

		DMibMemoryReportRealloc(g_pMemoryManagerName, g_pMemoryManagerName, pReturn, OldSize, nullptr, pReturn, 0, RequestedSize, _Size, fg_Local_SizeFull(pReturn) - RequestedSize, nullptr);

		return pReturn;
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(RequestedSize, _Size);
		DMibMemoryReportExpression(mint OldSize = fg_Local_Size(_pMemory));

		void *pReturn;
		if (_AllocFlags & EAllocationFlag_SizeNotNeeded)
			pReturn = fg_Local_ReAlloc(_pMemory, _Size, 16);
		else
			pReturn = fg_Local_ReAllocWithSize(_pMemory, _Size, 16);

		DMibMemoryReportRealloc(g_pMemoryManagerName, g_pMemoryManagerName, pReturn, OldSize, nullptr, pReturn, 0, RequestedSize, _Size, fg_Local_SizeFull(pReturn) - RequestedSize, nullptr);

		return pReturn;
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(Size, fg_Local_Size(_pMemory));

		fg_Local_Free(_pMemory);

		DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);
		DMibMemoryReportSaveVar(Size, fg_Local_Size(_pMemory));

		fg_Local_Free(_pMemory);

		DMibMemoryReportFree(g_pMemoryManagerName, g_pMemoryManagerName, _pMemory, Size, nullptr);
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
	{
		DMibMemoryGoingToReportScope(g_pMemoryManagerName, true);

		auto Ret = fg_Local_Size(_pMemory);

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
		return fg_AlignUp(_Size, 16);
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
