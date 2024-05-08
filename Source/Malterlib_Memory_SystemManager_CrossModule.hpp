// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

#include "Malterlib_Memory_SystemManager_CrossModule.h"

namespace NMib::NMemory
{
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_MemoryManager_GarbageCollect(CMemoryManagerCrossModule *_pModule)
	{
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_CreateMemoryManager(CMemoryManagerCrossModule *_pModule)
	{
	}
	inline_always CMemoryManagerCheckout DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_MemoryManager_Checkout(CMemoryManagerCrossModule *_pModule)
	{
		return nullptr;
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_DestroyMemoryManager(CMemoryManagerCrossModule *_pModule)
	{
	}
	inline_always bool DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_MemoryManager_Check(CMemoryManagerCrossModule *_pModule, bool _bBreak)
	{
		return true;
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_MemoryManager_PrepareFork(CMemoryManagerCrossModule *_pModule)
	{
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_MemoryManager_ForkedParent(CMemoryManagerCrossModule *_pModule)
	{
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_MemoryManager_ForkedChild(CMemoryManagerCrossModule *_pModule)
	{
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_MemoryManager_DestroyThreads(CMemoryManagerCrossModule *_pModule)
	{
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_MemoryManager_CanStartThreads(CMemoryManagerCrossModule *_pModule)
	{
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_MemoryManager_SetNumaNode(CMemoryManagerCrossModule *_pModule, ENumaNode _NumaNode)
	{
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_MemoryManager_OnThreadCreated(CMemoryManagerCrossModule *_pModule, mint _ThreadID, mint _ParentID)
	{
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_DemandProtection(CMemoryManagerCrossModule *_pModule)
	{
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_AllocWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		return CCrossModuleImplementationExtra::fs_AllocWithSize(_pModule, _Size);
	}
	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_AllocAlignedWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		return CCrossModuleImplementationExtra::fs_AllocAlignedWithSize(_pModule, _Size, _Alignment);
	}
	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
	{
		return CCrossModuleImplementationExtra::fs_Realloc(_pModule, _pMemory, _Size, _OldSize, _AllocFlags);
	}
	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
	{
		return CCrossModuleImplementationExtra::fs_Resize(_pModule, _pMemory, _Size, _OldSize, _AllocFlags);
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_Granularity(CMemoryManagerCrossModule *_pModule)
	{
		return CCrossModuleImplementationExtra::fs_Granularity(_pModule);
	}
	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_Size(CMemoryManagerCrossModule *_pModule, void *_pMemory)
	{
		return CCrossModuleImplementationExtra::fs_Size(_pModule, _pMemory);
	}
	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_TrySize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
	{
		return CCrossModuleImplementationExtra::fs_TrySize(_pModule, _pMemory);
	}
	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
	{
		return CCrossModuleImplementationExtra::fs_SizePadded(_pModule, _Size);
	}
	inline_always fp32 DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory)
	{
		return CCrossModuleImplementationExtra::fs_Overhead(_pModule, _pMemory);
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
		return CCrossModuleImplementationExtra::fs_AllocWithSize(_pModule, _Size);
	}
	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
	{
		return CCrossModuleImplementationExtra::fs_AllocAlignedWithSize(_pModule, _Size, _Alignment);
	}
	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		return CCrossModuleImplementationExtra::fs_Realloc(_pModule, _pMemory, _Size, _OldSize, _AllocFlags);
	}
	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		return CCrossModuleImplementationExtra::fs_Resize(_pModule, _pMemory, _Size, _OldSize, _AllocFlags);
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
	{
		return CCrossModuleImplementationExtra::fs_Free(_pModule, _pMemory, _Size);
	}

	inline_always bool DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_AllocHasDeterministicSize(CMemoryManagerCrossModule *_pModule)
	{
		return CCrossModuleImplementationExtra::fs_AllocHasDeterministicSize(_pModule);
	}

	EMemoryManagerFeatureFlag DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_MemoryManagerFeatures(CMemoryManagerCrossModule *_pModule)
	{
		return CCrossModuleImplementationExtra::fs_MemoryManagerFeatures(_pModule);
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
	{
		return CCrossModuleImplementationExtra::fs_FreeNoSize(_pModule, _pMemory);
	}
	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_AllocWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		if constexpr (CCrossModuleImplementationExtra::mc_SupportsNonTracked)
			return CCrossModuleImplementationExtra::fs_NonTracked_AllocWithSize(_pModule, _Size);
		else
			return CCrossModuleImplementationExtra::fs_AllocWithSizeDebug(_pModule, _Size, _pFile, _Line, _Flags);
	}
	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
	{
		if constexpr (CCrossModuleImplementationExtra::mc_SupportsNonTracked)
			return CCrossModuleImplementationExtra::fs_NonTracked_Realloc(_pModule, _pMemory, _Size, _OldSize, _AllocFlags);
		else
			return CCrossModuleImplementationExtra::fs_ReallocDebug(_pModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
	}
	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
	{
		if constexpr (CCrossModuleImplementationExtra::mc_SupportsNonTracked)
			return CCrossModuleImplementationExtra::fs_NonTracked_Resize(_pModule, _pMemory, _Size, _OldSize, _AllocFlags);
		else
			return CCrossModuleImplementationExtra::fs_ResizeDebug(_pModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
	}
	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_AllocAlignedWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		if constexpr (CCrossModuleImplementationExtra::mc_SupportsNonTracked)
			return CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedWithSize(_pModule, _Size, _Alignment);
		else
			return CCrossModuleImplementationExtra::fs_AllocAlignedWithSizeDebug(_pModule, _Size, _Alignment, _pFile, _Line, _Flags);
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_AllocBatchInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
	{
		while (true)
		{
			mint Size = _Size;
			void * pMem;
			if constexpr (CCrossModuleImplementationExtra::mc_bSizePenalty)
				pMem = CCrossModuleImplementationExtra::fs_AllocAligned(_pModule, Size, _Alignment);
			else
				pMem = CCrossModuleImplementationExtra::fs_AllocAlignedWithSize(_pModule, Size, _Alignment);
			if (!_Functor(pMem, Size))
				break;
		}
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_AllocBatch(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext)
	{
		while (true)
		{
			mint Size = _Size;
			void *pMem;
			if constexpr (CCrossModuleImplementationExtra::mc_bSizePenalty)
				pMem = CCrossModuleImplementationExtra::fs_AllocAligned(_pModule, Size, _Alignment);
			else
				pMem = CCrossModuleImplementationExtra::fs_AllocAlignedWithSize(_pModule, Size, _Alignment);
			if (!_fCallBatchFunctor(_pContext, pMem, Size))
				break;
		}
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_AllocBatchDebugInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		while (true)
		{
			mint Size = _Size;
			void *pMem;
			pMem = CCrossModuleImplementationExtra::fs_AllocAlignedWithSizeDebug(_pModule, Size, _Alignment, _pFile, _Line, _Flags);
			if (!_Functor(pMem, Size))
				break;
		}
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_AllocBatchDebug(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		while (true)
		{
			mint Size = _Size;
			void *pMem = CCrossModuleImplementationExtra::fs_AllocAlignedWithSizeDebug(_pModule, Size, _Alignment, _pFile, _Line, _Flags);
			if (!_fCallBatchFunctor(_pContext, pMem, Size))
				break;
		}
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_AllocBatchInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
	{
		while (true)
		{
			mint Size = _Size;
			void *pMem;
			if constexpr (CCrossModuleImplementationExtra::mc_bSizePenalty)
				pMem = CCrossModuleImplementationExtra::fs_NonTracked_AllocAligned(_pModule, Size, _Alignment);
			else
				pMem = CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedWithSize(_pModule, Size, _Alignment);
			if (!_Functor(pMem, Size))
				break;
		}
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_AllocBatch(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext)
	{
		while (true)
		{
			mint Size = _Size;
			void *pMem;
			if constexpr (CCrossModuleImplementationExtra::mc_bSizePenalty)
				pMem = CCrossModuleImplementationExtra::fs_NonTracked_AllocAligned(_pModule, Size, _Alignment);
			else
				pMem = CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedWithSize(_pModule, Size, _Alignment);
			if (!_fCallBatchFunctor(_pContext, pMem, Size))
				break;
		}
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_AllocBatchDebugInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		while (true)
		{
			mint Size = _Size;
			void *pMem = CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedWithSizeDebug(_pModule, Size, _Alignment, _pFile, _Line, _Flags);
			if (!_Functor(pMem, Size))
				break;
		}
	}
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_AllocBatchDebug(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		while (true)
		{
			mint Size = _Size;
			void *pMem = CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedWithSizeDebug(_pModule, Size, _Alignment, _pFile, _Line, _Flags);
			if (!_fCallBatchFunctor(_pContext, pMem, Size))
				break;
		}
	}

	inline_always bool DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_ReportingLeaks(CMemoryManagerCrossModule *_pModule)
	{
		return false;
	}

	void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
	{
		return CCrossModuleImplementation::fs_AllocWithSize(_pModule, _Size);
	}

	void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint _Size)
	{
		return CCrossModuleImplementation::fs_AllocInitZeroWithSize(_pModule, _Size);
	}

	void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Align)
	{
		return CCrossModuleImplementation::fs_AllocAlignedWithSize(_pModule, _Size, _Align);
	}

	void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
	{
		return CCrossModuleImplementation::fs_NonTracked_AllocWithSize(_pModule, _Size);
	}

	void * DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_NonTracked_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment)
	{
		return CCrossModuleImplementation::fs_NonTracked_AllocAlignedWithSize(_pModule, _Size, _Alignment);
	}

	static void * DMibCrossmoduleAPI fg_OldRealloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
	{
		return CCrossModuleImplementationExtra::fs_Realloc(_pModule, _pMemory, _Size, 0, EAllocationFlag_None);
	}
	static void * DMibCrossmoduleAPI fg_OldResize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
	{
		return CCrossModuleImplementationExtra::fs_Resize(_pModule, _pMemory, _Size, 0, EAllocationFlag_None);
	}
	static void * DMibCrossmoduleAPI fg_OldNonTrackedRealloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_Realloc(_pModule, _pMemory, _Size, 0, EAllocationFlag_None);
	}
	static void * DMibCrossmoduleAPI fg_OldNonTrackedResize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_Resize(_pModule, _pMemory, _Size, 0, EAllocationFlag_None);
	}

	CMemoryManagerCrossModule g_CrossModule =
		{
			.m_Version = EMemoryManagerCrossModule_Version
			, .m_Reserved = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}
			, .m_fCreateNonTrackedMemoryManager = &CCrossModuleImplementationExtra::fs_CreateNonTrackedMemoryManager
			, .m_fDestroyNonTrackedMemoryManager = &CCrossModuleImplementationExtra::fs_DestroyNonTrackedMemoryManager
			, .m_fMemoryManager_GarbageCollect = &CCrossModuleImplementationExtra::fs_MemoryManager_GarbageCollect
			, .m_fCreateMemoryManager = &CCrossModuleImplementationExtra::fs_CreateMemoryManager
			, .m_fMemoryManager_Checkout = &CCrossModuleImplementationExtra::fs_MemoryManager_Checkout
			, .m_fDestroyMemoryManager = &CCrossModuleImplementationExtra::fs_DestroyMemoryManager
			, .m_fMemoryManager_Check = &CCrossModuleImplementationExtra::fs_MemoryManager_Check
			, .m_fMemoryManager_PrepareFork = &CCrossModuleImplementationExtra::fs_MemoryManager_PrepareFork
			, .m_fMemoryManager_ForkedParent = &CCrossModuleImplementationExtra::fs_MemoryManager_ForkedParent
			, .m_fMemoryManager_ForkedChild = &CCrossModuleImplementationExtra::fs_MemoryManager_ForkedChild
			, .m_fMemoryManager_DestroyThreads = &CCrossModuleImplementationExtra::fs_MemoryManager_DestroyThreads
			, .m_fMemoryManager_CanStartThreads = &CCrossModuleImplementationExtra::fs_MemoryManager_CanStartThreads
			, .m_fMemoryManager_SetNumaNode = &CCrossModuleImplementationExtra::fs_MemoryManager_SetNumaNode
			, .m_fMemoryManager_OnThreadCreated = &CCrossModuleImplementationExtra::fs_MemoryManager_OnThreadCreated
			, .m_fDemandProtection = &CCrossModuleImplementationExtra::fs_DemandProtection
			, .m_fAllocWithSize = &CCrossModuleImplementationExtra::fs_AllocWithSize
			, .m_fAllocInitZeroWithSize = &CCrossModuleImplementationExtra::fs_AllocInitZeroWithSize
			, .m_fAllocAlignedWithSize = &CCrossModuleImplementationExtra::fs_AllocAlignedWithSize
			, .m_fAllocBatch = &CCrossModuleImplementationExtra::fs_AllocBatch
			, .m_fAllocBatchDebug = &CCrossModuleImplementationExtra::fs_AllocBatchDebug
			, .m_fAllocWithSizeDebug = &CCrossModuleImplementationExtra::fs_AllocWithSizeDebug
			, .m_fAllocAlignedWithSizeDebug = &CCrossModuleImplementationExtra::fs_AllocAlignedWithSizeDebug
			, .m_fReallocNoOldDebug = nullptr // &CCrossModuleImplementationExtra::fs_ReallocNoOldDebug
			, .m_fResizeNoOldDebug = nullptr // &CCrossModuleImplementationExtra::fs_ResizeNoOldDebug
			, .m_fReallocNoOld = &fg_OldRealloc
			, .m_fResizeNoOld = &fg_OldResize
			, .m_fFreeNoSize = &CCrossModuleImplementationExtra::fs_FreeNoSize
			, .m_fSize = &CCrossModuleImplementationExtra::fs_Size
			, .m_fTrySize = &CCrossModuleImplementationExtra::fs_TrySize
			, .m_fSizePadded = &CCrossModuleImplementationExtra::fs_SizePadded
			, .m_fOverhead = &CCrossModuleImplementationExtra::fs_Overhead
			, .m_fGranularity = &CCrossModuleImplementationExtra::fs_Granularity
			, .m_fNonTracked_Granularity = &CCrossModuleImplementationExtra::fs_NonTracked_Granularity
			, .m_fNonTracked_Size = &CCrossModuleImplementationExtra::fs_NonTracked_Size
			, .m_fNonTracked_TrySize = &CCrossModuleImplementationExtra::fs_NonTracked_TrySize
			, .m_fNonTracked_SizePadded = &CCrossModuleImplementationExtra::fs_NonTracked_SizePadded
			, .m_fNonTracked_Overhead = &CCrossModuleImplementationExtra::fs_NonTracked_Overhead
			, .m_fNonTracked_AllocWithSize = &CCrossModuleImplementationExtra::fs_NonTracked_AllocWithSize
			, .m_fNonTracked_AllocAlignedWithSize = &CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedWithSize
			, .m_fNonTracked_AllocBatch = &CCrossModuleImplementationExtra::fs_NonTracked_AllocBatch
			, .m_fNonTracked_AllocBatchDebug = &CCrossModuleImplementationExtra::fs_NonTracked_AllocBatchDebug
			, .m_fNonTracked_ReallocNoOld = &fg_OldNonTrackedRealloc
			, .m_fNonTracked_ResizeNoOld = &fg_OldNonTrackedResize
			, .m_fNonTracked_FreeNoSize = &CCrossModuleImplementationExtra::fs_NonTracked_FreeNoSize
			, .m_fNonTracked_AllocWithSizeDebug = &CCrossModuleImplementationExtra::fs_NonTracked_AllocWithSizeDebug
			, .m_fNonTracked_AllocAlignedWithSizeDebug = &CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedWithSizeDebug
			, .m_fNonTracked_ReallocNoOldDebug = nullptr // &CCrossModuleImplementationExtra::fs_NonTracked_ReallocDebug
			, .m_fNonTracked_ResizeNoOldDebug = nullptr // &CCrossModuleImplementationExtra::fs_NonTracked_ResizeDebug
			, .m_fReportingLeaks = &CCrossModuleImplementationExtra::fs_ReportingLeaks
			, .m_fAlloc = &CCrossModuleImplementationExtra::fs_Alloc
			, .m_fAllocInitZero = &CCrossModuleImplementationExtra::fs_AllocInitZero
			, .m_fAllocAligned = &CCrossModuleImplementationExtra::fs_AllocAligned
			, .m_fNonTracked_Alloc = &CCrossModuleImplementationExtra::fs_NonTracked_Alloc
			, .m_fNonTracked_AllocAligned = &CCrossModuleImplementationExtra::fs_NonTracked_AllocAligned
			, .m_fReallocDebug = &CCrossModuleImplementationExtra::fs_ReallocDebug
			, .m_fResizeDebug = &CCrossModuleImplementationExtra::fs_ResizeDebug
			, .m_fRealloc = &CCrossModuleImplementationExtra::fs_Realloc
			, .m_fResize = &CCrossModuleImplementationExtra::fs_Resize
			, .m_fNonTracked_Realloc = &CCrossModuleImplementationExtra::fs_NonTracked_Realloc
			, .m_fNonTracked_Resize = &CCrossModuleImplementationExtra::fs_NonTracked_Resize
			, .m_fNonTracked_ReallocDebug = &CCrossModuleImplementationExtra::fs_NonTracked_ReallocDebug
			, .m_fNonTracked_ResizeDebug = &CCrossModuleImplementationExtra::fs_NonTracked_ResizeDebug
			, .m_fFree = &CCrossModuleImplementationExtra::fs_Free
			, .m_fNonTracked_Free = &CCrossModuleImplementationExtra::fs_NonTracked_Free
			, .m_fAllocHasDeterministicSize = &CCrossModuleImplementationExtra::fs_AllocHasDeterministicSize
			, .m_fMemoryManagerFeatures = &CCrossModuleImplementationExtra::fs_MemoryManagerFeatures
		}
	;
}

#if defined(DMibDynamicLibrary) && DMibConfig_MemoryManager_CrossModule_Enable && !defined(DMibMemoryOverrideDll)
	#include "Malterlib_Memory_SystemManager_CrossModuleClient.hpp"
#else
	#include "Malterlib_Memory_SystemManager_CrossModuleServer.hpp"
#endif
