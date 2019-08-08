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
	inline_always void DMibCrossmoduleAPI CCrossModuleImplementationDefaults::fs_MemoryManager_CanStrartThreads(CMemoryManagerCrossModule *_pModule)
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

	CMemoryManagerCrossModule g_CrossModule
		=
		{
			EMemoryManagerCrossModule_Version
			, 0 // Reserved
			, 1 // Reserved
			, 2 // Reserved
			, 3 // Reserved
			, 4 // Reserved
			, 5 // Reserved
			, 6 // Reserved
			, 7 // Reserved
			, 8 // Reserved
			, 9 // Reserved
			, 10 // Reserved
			, 11 // Reserved
			, 12 // Reserved
			, 13 // Reserved
			, 14 // Reserved
			, 15 // Reserved
			, &CCrossModuleImplementationExtra::fs_CreateNonTrackedMemoryManager
			, &CCrossModuleImplementationExtra::fs_DestroyNonTrackedMemoryManager
			, &CCrossModuleImplementationExtra::fs_MemoryManager_GarbageCollect
			, &CCrossModuleImplementationExtra::fs_CreateMemoryManager
			, &CCrossModuleImplementationExtra::fs_MemoryManager_Checkout
			, &CCrossModuleImplementationExtra::fs_DestroyMemoryManager
			, &CCrossModuleImplementationExtra::fs_MemoryManager_Check
			, &CCrossModuleImplementationExtra::fs_MemoryManager_PrepareFork
			, &CCrossModuleImplementationExtra::fs_MemoryManager_ForkedParent
			, &CCrossModuleImplementationExtra::fs_MemoryManager_ForkedChild
			, &CCrossModuleImplementationExtra::fs_MemoryManager_DestroyThreads
			, &CCrossModuleImplementationExtra::fs_MemoryManager_CanStrartThreads
			, &CCrossModuleImplementationExtra::fs_MemoryManager_SetNumaNode
			, &CCrossModuleImplementationExtra::fs_MemoryManager_OnThreadCreated
			, &CCrossModuleImplementationExtra::fs_DemandProtection

			, &CCrossModuleImplementationExtra::fs_AllocWithSize
			, &CCrossModuleImplementationExtra::fs_AllocInitZeroWithSize
			, &CCrossModuleImplementationExtra::fs_AllocAlignedWithSize
			, &CCrossModuleImplementationExtra::fs_AllocBatch
			, &CCrossModuleImplementationExtra::fs_AllocBatchDebug
			, &CCrossModuleImplementationExtra::fs_AllocWithSizeDebug
			, &CCrossModuleImplementationExtra::fs_AllocAlignedWithSizeDebug
			, nullptr // &CCrossModuleImplementationExtra::fs_ReallocNoOldDebug
			, nullptr // &CCrossModuleImplementationExtra::fs_ResizeNoOldDebug
			, &fg_OldRealloc
			, &fg_OldResize
			, &CCrossModuleImplementationExtra::fs_FreeNoSize
			, &CCrossModuleImplementationExtra::fs_Size
			, &CCrossModuleImplementationExtra::fs_TrySize
			, &CCrossModuleImplementationExtra::fs_SizePadded
			, &CCrossModuleImplementationExtra::fs_Overhead
			, &CCrossModuleImplementationExtra::fs_Granularity

			, &CCrossModuleImplementationExtra::fs_NonTracked_Granularity
			, &CCrossModuleImplementationExtra::fs_NonTracked_Size
			, &CCrossModuleImplementationExtra::fs_NonTracked_TrySize
			, &CCrossModuleImplementationExtra::fs_NonTracked_SizePadded
			, &CCrossModuleImplementationExtra::fs_NonTracked_Overhead
			, &CCrossModuleImplementationExtra::fs_NonTracked_AllocWithSize
			, &CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedWithSize
			, &CCrossModuleImplementationExtra::fs_NonTracked_AllocBatch
			, &CCrossModuleImplementationExtra::fs_NonTracked_AllocBatchDebug
			, &fg_OldNonTrackedRealloc
			, &fg_OldNonTrackedResize
			, &CCrossModuleImplementationExtra::fs_NonTracked_FreeNoSize
			, &CCrossModuleImplementationExtra::fs_NonTracked_AllocWithSizeDebug
			, &CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedWithSizeDebug
			, nullptr // &CCrossModuleImplementationExtra::fs_NonTracked_ReallocDebug
			, nullptr // &CCrossModuleImplementationExtra::fs_NonTracked_ResizeDebug

			, &CCrossModuleImplementationExtra::fs_ReportingLeaks

			, &CCrossModuleImplementationExtra::fs_Alloc
			, &CCrossModuleImplementationExtra::fs_AllocInitZero
			, &CCrossModuleImplementationExtra::fs_AllocAligned

			, &CCrossModuleImplementationExtra::fs_NonTracked_Alloc
			, &CCrossModuleImplementationExtra::fs_NonTracked_AllocAligned

			, &CCrossModuleImplementationExtra::fs_ReallocDebug
			, &CCrossModuleImplementationExtra::fs_ResizeDebug
			, &CCrossModuleImplementationExtra::fs_Realloc
			, &CCrossModuleImplementationExtra::fs_Resize

			, &CCrossModuleImplementationExtra::fs_NonTracked_Realloc
			, &CCrossModuleImplementationExtra::fs_NonTracked_Resize
			, &CCrossModuleImplementationExtra::fs_NonTracked_ReallocDebug
			, &CCrossModuleImplementationExtra::fs_NonTracked_ResizeDebug

			, &CCrossModuleImplementationExtra::fs_Free
			, &CCrossModuleImplementationExtra::fs_NonTracked_Free
		}
	;
}

#if defined(DMibDynamicLibrary) && DMibConfig_MemoryManager_CrossModule_Enable && !defined(DMibMemoryOverrideDll)
	#include "Malterlib_Memory_SystemManager_CrossModuleClient.hpp"
#else
	#include "Malterlib_Memory_SystemManager_CrossModuleServer.hpp"
#endif
