// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

#include "Malterlib_Memory_SystemManager_CrossModule.h"

namespace NMib
{
	namespace NMem
	{
		inline_always void CCrossModuleImplementationDefaults::fs_MemoryManager_GarbageCollect(CMemoryManagerCrossModule *_pModule)
		{
		}
		inline_always void CCrossModuleImplementationDefaults::fs_CreateMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
		}
		inline_always CMemoryManagerCheckout CCrossModuleImplementationDefaults::fs_MemoryManager_Checkout(CMemoryManagerCrossModule *_pModule)
		{
			return nullptr;
		}
		inline_always void CCrossModuleImplementationDefaults::fs_DestroyMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
		}
		inline_always bool CCrossModuleImplementationDefaults::fs_MemoryManager_Check(CMemoryManagerCrossModule *_pModule, bool _bBreak)
		{
			return true;
		}
		inline_always void CCrossModuleImplementationDefaults::fs_MemoryManager_PrepareFork(CMemoryManagerCrossModule *_pModule)
		{
		}
		inline_always void CCrossModuleImplementationDefaults::fs_MemoryManager_ForkedParent(CMemoryManagerCrossModule *_pModule)
		{
		}
		inline_always void CCrossModuleImplementationDefaults::fs_MemoryManager_ForkedChild(CMemoryManagerCrossModule *_pModule)
		{
		}
		inline_always void CCrossModuleImplementationDefaults::fs_MemoryManager_DestroyThreads(CMemoryManagerCrossModule *_pModule)
		{
		}
		inline_always void CCrossModuleImplementationDefaults::fs_MemoryManager_CanStrartThreads(CMemoryManagerCrossModule *_pModule)
		{
		}		
		inline_always void CCrossModuleImplementationDefaults::fs_MemoryManager_SetNumaNode(CMemoryManagerCrossModule *_pModule, ENumaNode _NumaNode)
		{
		}
		inline_always void CCrossModuleImplementationDefaults::fs_MemoryManager_OnThreadCreated(CMemoryManagerCrossModule *_pModule, mint _ThreadID, mint _ParentID)
		{
		}
		inline_always void CCrossModuleImplementationDefaults::fs_DemandProtection(CMemoryManagerCrossModule *_pModule)
		{
		}
		
		inline_always void * CCrossModuleImplementationDefaults::fs_AllocDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return CCrossModuleImplementationExtra::fs_Alloc(_pModule, _Size);
		}
		inline_always void * CCrossModuleImplementationDefaults::fs_AllocAlignedDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return CCrossModuleImplementationExtra::fs_AllocAligned(_pModule, _Size, _Alignment);
		}
		inline_always void * CCrossModuleImplementationDefaults::fs_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return CCrossModuleImplementationExtra::fs_Realloc(_pModule, _pMemory, _Size);
		}
		inline_always void * CCrossModuleImplementationDefaults::fs_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return CCrossModuleImplementationExtra::fs_Resize(_pModule, _pMemory, _Size);
		}
		
		inline_always mint CCrossModuleImplementationDefaults::fs_NonTracked_Granularity(CMemoryManagerCrossModule *_pModule)
		{
			return CCrossModuleImplementationExtra::fs_Granularity(_pModule);
		}
		inline_always mint CCrossModuleImplementationDefaults::fs_NonTracked_Size(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			return CCrossModuleImplementationExtra::fs_Size(_pModule, _pMemory);
		}
		inline_always mint CCrossModuleImplementationDefaults::fs_NonTracked_TrySize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			return CCrossModuleImplementationExtra::fs_TrySize(_pModule, _pMemory);
		}
		inline_always mint CCrossModuleImplementationDefaults::fs_NonTracked_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return CCrossModuleImplementationExtra::fs_SizePadded(_pModule, _Size);
		}
		inline_always fp32 CCrossModuleImplementationDefaults::fs_NonTracked_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory)
		{
			return CCrossModuleImplementationExtra::fs_Overhead(_pModule, _pMemory);
		}
		
		inline_always void * CCrossModuleImplementationDefaults::fs_NonTracked_Alloc(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return CCrossModuleImplementationExtra::fs_Alloc(_pModule, _Size);
		}
		inline_always void * CCrossModuleImplementationDefaults::fs_NonTracked_AllocAligned(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
		{
			return CCrossModuleImplementationExtra::fs_AllocAligned(_pModule, _Size, _Alignment);
		}
		inline_always void * CCrossModuleImplementationDefaults::fs_NonTracked_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
		{
			return CCrossModuleImplementationExtra::fs_Realloc(_pModule, _pMemory, _Size);
		}
		inline_always void * CCrossModuleImplementationDefaults::fs_NonTracked_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
		{
			return CCrossModuleImplementationExtra::fs_Resize(_pModule, _pMemory, _Size);
		}
		inline_always void CCrossModuleImplementationDefaults::fs_NonTracked_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			return CCrossModuleImplementationExtra::fs_Free(_pModule, _pMemory);
		}
		inline_always void * CCrossModuleImplementationDefaults::fs_NonTracked_AllocDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			if (CCrossModuleImplementationExtra::mc_SupportsNonTracked)
				return CCrossModuleImplementationExtra::fs_NonTracked_Alloc(_pModule, _Size);
			else
				return CCrossModuleImplementationExtra::fs_AllocDebug(_pModule, _Size, _pFile, _Line, _Flags);
		}
		inline_always void * CCrossModuleImplementationDefaults::fs_NonTracked_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			if (CCrossModuleImplementationExtra::mc_SupportsNonTracked)
				return CCrossModuleImplementationExtra::fs_NonTracked_Realloc(_pModule, _pMemory, _Size);
			else
				return CCrossModuleImplementationExtra::fs_ReallocDebug(_pModule, _pMemory, _Size, _pFile, _Line, _Flags);
		}
		inline_always void * CCrossModuleImplementationDefaults::fs_NonTracked_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			if (CCrossModuleImplementationExtra::mc_SupportsNonTracked)
				return CCrossModuleImplementationExtra::fs_NonTracked_Resize(_pModule, _pMemory, _Size);
			else
				return CCrossModuleImplementationExtra::fs_ResizeDebug(_pModule, _pMemory, _Size, _pFile, _Line, _Flags);
		}
		inline_always void * CCrossModuleImplementationDefaults::fs_NonTracked_AllocAlignedDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			if (CCrossModuleImplementationExtra::mc_SupportsNonTracked)
				return CCrossModuleImplementationExtra::fs_NonTracked_AllocAligned(_pModule, _Size, _Alignment);
			else
				return CCrossModuleImplementationExtra::fs_AllocAlignedDebug(_pModule, _Size, _Alignment, _pFile, _Line, _Flags);
		}
		
		inline_always void CCrossModuleImplementationDefaults::fs_AllocBatchInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			while (true)
			{
				mint Size = _Size;
				void * pMem = CCrossModuleImplementationExtra::fs_AllocAligned(_pModule, Size, _Alignment);
				if (!_Functor(pMem, Size))
					break;
			}
		}
		inline_always void CCrossModuleImplementationDefaults::fs_AllocBatch(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext)
		{
			while (true)
			{
				mint Size = _Size;
				void * pMem = CCrossModuleImplementationExtra::fs_AllocAligned(_pModule, Size, _Alignment);
				if (!_fCallBatchFunctor(_pContext, pMem, Size))
					break;
			}
		}
		inline_always void CCrossModuleImplementationDefaults::fs_AllocBatchDebugInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			while (true)
			{
				mint Size = _Size;
				void * pMem = CCrossModuleImplementationExtra::fs_AllocAlignedDebug(_pModule, Size, _Alignment, _pFile, _Line, _Flags);
				if (!_Functor(pMem, Size))
					break;
			}
		}
		inline_always void CCrossModuleImplementationDefaults::fs_AllocBatchDebug(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			while (true)
			{
				mint Size = _Size;
				void * pMem = CCrossModuleImplementationExtra::fs_AllocAlignedDebug(_pModule, Size, _Alignment, _pFile, _Line, _Flags);
				if (!_fCallBatchFunctor(_pContext, pMem, Size))
					break;
			}
		}
		
		inline_always void CCrossModuleImplementationDefaults::fs_NonTracked_AllocBatchInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			while (true)
			{
				mint Size = _Size;
				void * pMem = CCrossModuleImplementationExtra::fs_NonTracked_AllocAligned(_pModule, Size, _Alignment);
				if (!_Functor(pMem, Size))
					break;
			}
		}
		inline_always void CCrossModuleImplementationDefaults::fs_NonTracked_AllocBatch(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext)
		{
			while (true)
			{
				mint Size = _Size;
				void * pMem = CCrossModuleImplementationExtra::fs_NonTracked_AllocAligned(_pModule, Size, _Alignment);
				if (!_fCallBatchFunctor(_pContext, pMem, Size))
					break;
			}
		}

		inline_always void CCrossModuleImplementationDefaults::fs_NonTracked_AllocBatchDebugInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			while (true)
			{
				mint Size = _Size;
				void * pMem = CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedDebug(_pModule, Size, _Alignment, _pFile, _Line, _Flags);
				if (!_Functor(pMem, Size))
					break;
			}
		}
		inline_always void CCrossModuleImplementationDefaults::fs_NonTracked_AllocBatchDebug(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			while (true)
			{
				mint Size = _Size;
				void * pMem = CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedDebug(_pModule, Size, _Alignment, _pFile, _Line, _Flags);
				if (!_fCallBatchFunctor(_pContext, pMem, Size))
					break;
			}
		}
		
		inline_always bool CCrossModuleImplementationDefaults::fs_ReportingLeaks(CMemoryManagerCrossModule *_pModule)
		{
			return false;
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
				
				, &CCrossModuleImplementationExtra::fs_Alloc
				, &CCrossModuleImplementationExtra::fs_AllocInitZero
				, &CCrossModuleImplementationExtra::fs_AllocAligned
				, &CCrossModuleImplementationExtra::fs_AllocBatch
				, &CCrossModuleImplementationExtra::fs_AllocBatchDebug
				, &CCrossModuleImplementationExtra::fs_AllocDebug
				, &CCrossModuleImplementationExtra::fs_AllocAlignedDebug
				, &CCrossModuleImplementationExtra::fs_ReallocDebug
				, &CCrossModuleImplementationExtra::fs_ResizeDebug
				, &CCrossModuleImplementationExtra::fs_Realloc
				, &CCrossModuleImplementationExtra::fs_Resize
				, &CCrossModuleImplementationExtra::fs_Free
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
				, &CCrossModuleImplementationExtra::fs_NonTracked_Alloc
				, &CCrossModuleImplementationExtra::fs_NonTracked_AllocAligned
				, &CCrossModuleImplementationExtra::fs_NonTracked_AllocBatch
				, &CCrossModuleImplementationExtra::fs_NonTracked_AllocBatchDebug
				, &CCrossModuleImplementationExtra::fs_NonTracked_Realloc
				, &CCrossModuleImplementationExtra::fs_NonTracked_Resize
				, &CCrossModuleImplementationExtra::fs_NonTracked_Free
				, &CCrossModuleImplementationExtra::fs_NonTracked_AllocDebug
				, &CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedDebug
				, &CCrossModuleImplementationExtra::fs_NonTracked_ReallocDebug
				, &CCrossModuleImplementationExtra::fs_NonTracked_ResizeDebug
				
				, &CCrossModuleImplementationExtra::fs_ReportingLeaks
			}
		;
	}
}

#if defined(DMibDynamicLibrary) && DMibConfig_MemoryManager_CrossModule_Enable && !defined(DMibMemoryOverrideDll)
	#include "Malterlib_Memory_SystemManager_CrossModuleClient.hpp"
#else
	#include "Malterlib_Memory_SystemManager_CrossModuleServer.hpp"
#endif
