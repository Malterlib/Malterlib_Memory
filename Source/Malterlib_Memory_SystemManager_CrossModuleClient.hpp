// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>
#include "Malterlib_Memory_SystemManager_CrossModule.h"

namespace NMib
{
	NMemory::ICMemoryManagerCrossModule *g_pLocalCrossModuleInterface = nullptr;

	DMibSuppressUndefinedSanitizer void CSystem::fp_CreateNonTrackedMemoryManager()
	{
		using namespace NMemory;

		DMibFastCheck(fg_GetSys()->f_IsDll());

		g_pLocalCrossModuleInterface = (NMemory::ICMemoryManagerCrossModule *)NSys::fg_Process_GetCrossModuleMemoryManagerInterface();

		if (g_pLocalCrossModuleInterface)
		{
			g_pLocalCrossModuleInterface->f_Register(&NMemory::g_CrossModule, NMemory::EMemoryManagerCrossModule_Version);
			if (NMemory::g_CrossModule.m_Version < 0x102)
			{
				NMemory::g_CrossModule.m_fReportingLeaks = [](CMemoryManagerCrossModule *_pModule) DMibSuppressUndefinedSanitizer -> bool
					{
						return false;
					}
				;
			}

			if (NMemory::g_CrossModule.m_Version < 0x103)
			{
				NMemory::g_CrossModule.m_fAlloc = [](CMemoryManagerCrossModule *_pModule, umint _Size) DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fAllocWithSize(&g_CrossModule, _Size);
					}
				;
				NMemory::g_CrossModule.m_fAllocInitZero = [](CMemoryManagerCrossModule *_pModule, umint _Size) DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fAllocInitZeroWithSize(&g_CrossModule, _Size);
					}
				;
				NMemory::g_CrossModule.m_fAllocAligned = [](CMemoryManagerCrossModule *_pModule, umint _Size, umint _Align) DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fAllocAlignedWithSize(&g_CrossModule, _Size, _Align);
					}
				;

				NMemory::g_CrossModule.m_fNonTracked_Alloc = [](CMemoryManagerCrossModule *_pModule, umint _Size) DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fNonTracked_AllocWithSize(&g_CrossModule, _Size);
					}
				;
				NMemory::g_CrossModule.m_fNonTracked_AllocAligned = [](CMemoryManagerCrossModule *_pModule, umint _Size, umint _Alignment) DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fNonTracked_AllocAlignedWithSize(&g_CrossModule, _Size, _Alignment);
					}
				;

				NMemory::g_CrossModule.m_fReallocDebug = []
					(
						CMemoryManagerCrossModule *_pModule
						, void *_pMemory
						 , umint &_Size
						 , umint _OldSize
						 , const ch8 *_pFile
						 , aint _Line
						 , EHeapDebugFlag _Flags
						 , EAllocationFlag _AllocFlags
					) DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fReallocNoOldDebug(&g_CrossModule, _pMemory, _Size, _pFile, _Line, _Flags);
					}
				;
				NMemory::g_CrossModule.m_fResizeDebug = []
					(
						CMemoryManagerCrossModule *_pModule
						, void *_pMemory
						, umint &_Size
						, umint _OldSize
						, const ch8 *_pFile
						, aint _Line
						, EHeapDebugFlag _Flags
						, EAllocationFlag _AllocFlags
					) DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fResizeNoOldDebug(&g_CrossModule, _pMemory, _Size, _pFile, _Line, _Flags);
					}
				;
				NMemory::g_CrossModule.m_fRealloc = [](CMemoryManagerCrossModule *_pModule, void *_pMemory, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags)
					DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fReallocNoOld(&g_CrossModule, _pMemory, _Size);
					}
				;
				NMemory::g_CrossModule.m_fResize = [](CMemoryManagerCrossModule *_pModule, void *_pMemory, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags)
					DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fResizeNoOld(&g_CrossModule, _pMemory, _Size);
					}
				;

				NMemory::g_CrossModule.m_fNonTracked_Realloc = [](CMemoryManagerCrossModule *_pModule, void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags)
					DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fNonTracked_ReallocNoOld(&g_CrossModule, _pMem, _Size);
					}
				;

				NMemory::g_CrossModule.m_fNonTracked_Resize = [](CMemoryManagerCrossModule *_pModule, void *_pMem, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags)
					DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fNonTracked_ResizeNoOld(&g_CrossModule, _pMem, _Size);
					}
				;

				NMemory::g_CrossModule.m_fNonTracked_ReallocDebug = []
					(
						CMemoryManagerCrossModule *_pModule
						, void *_pMem
						, umint &_Size
						, umint _OldSize
						, const ch8 *_pFile
						, aint _Line
						, EHeapDebugFlag _Flags
						, EAllocationFlag _AllocFlags
					) DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fNonTracked_ReallocNoOldDebug(&g_CrossModule, _pMem, _Size, _pFile, _Line, _Flags);
					}
				;

				NMemory::g_CrossModule.m_fNonTracked_ResizeDebug = []
					(
						CMemoryManagerCrossModule *_pModule
						, void *_pMem
						, umint &_Size
						, umint _OldSize
						, const ch8 *_pFile
						, aint _Line
						, EHeapDebugFlag _Flags
						, EAllocationFlag _AllocFlags
					) DMibSuppressUndefinedSanitizer -> void *
					{
						return NMemory::g_CrossModule.m_fNonTracked_ResizeNoOldDebug(&g_CrossModule, _pMem, _Size, _pFile, _Line, _Flags);
					}
				;

				NMemory::g_CrossModule.m_fFree = [](CMemoryManagerCrossModule *_pModule, void *_pMemory, umint _Size) DMibSuppressUndefinedSanitizer
					{
						return NMemory::g_CrossModule.m_fFreeNoSize(&g_CrossModule, _pMemory);
					}
				;

				NMemory::g_CrossModule.m_fNonTracked_Free = [](CMemoryManagerCrossModule *_pModule, void *_pMemory, umint _Size) DMibSuppressUndefinedSanitizer
					{
						return NMemory::g_CrossModule.m_fNonTracked_FreeNoSize(&g_CrossModule, _pMemory);
					}
				;
			}

			if (NMemory::g_CrossModule.m_Version < 0x104)
			{
				NMemory::g_CrossModule.m_fAllocHasDeterministicSize = [](CMemoryManagerCrossModule *_pModule) DMibSuppressUndefinedSanitizer
					{
						return false;
					}
				;
			}

			if (NMemory::g_CrossModule.m_Version < 0x105)
			{
				NMemory::g_CrossModule.m_fMemoryManagerFeatures = [](CMemoryManagerCrossModule *_pModule) DMibSuppressUndefinedSanitizer -> EMemoryManagerFeatureFlag
					{
						return EMemoryManagerFeatureFlag_None;
					}
				;
			}
		}

		NMemory::g_CrossModule.m_fCreateNonTrackedMemoryManager(&NMemory::g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer void CSystem::fp_DestroyNonTrackedMemoryManager()
	{
		NMemory::g_CrossModule.m_fDestroyNonTrackedMemoryManager(&NMemory::g_CrossModule);

		if (g_pLocalCrossModuleInterface)
			g_pLocalCrossModuleInterface->f_Unregister(&NMemory::g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer void CSystem::f_MemoryManager_GarbageCollect()
	{
		NMemory::g_CrossModule.m_fMemoryManager_GarbageCollect(&NMemory::g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer void CSystem::fp_CreateMemoryManager()
	{
		NMemory::g_CrossModule.m_fCreateMemoryManager(&NMemory::g_CrossModule);
	}
	DMibSuppressUndefinedSanitizer NMemory::CMemoryManagerCheckout CSystem::f_MemoryManager_Checkout()
	{
		return NMemory::g_CrossModule.m_fMemoryManager_Checkout(&NMemory::g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer void CSystem::fp_DestroyMemoryManager()
	{
		return NMemory::g_CrossModule.m_fDestroyMemoryManager(&NMemory::g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer bool CSystem::f_MemoryManager_Check(bool _bBreak)
	{
		return NMemory::g_CrossModule.m_fMemoryManager_Check(&NMemory::g_CrossModule, _bBreak);
	}

	DMibSuppressUndefinedSanitizer bool CSystem::f_MemoryManager_ReportingLeaks()
	{
		return NMemory::g_CrossModule.m_fReportingLeaks(&NMemory::g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer void CSystem::f_MemoryManager_PrepareFork()
	{
		return NMemory::g_CrossModule.m_fMemoryManager_PrepareFork(&NMemory::g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer void CSystem::f_MemoryManager_ForkedParent()
	{
		return NMemory::g_CrossModule.m_fMemoryManager_ForkedParent(&NMemory::g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer void CSystem::f_MemoryManager_ForkedChild()
	{
		return NMemory::g_CrossModule.m_fMemoryManager_ForkedChild(&NMemory::g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer void CSystem::f_MemoryManager_DestroyThreads()
	{
		return NMemory::g_CrossModule.m_fMemoryManager_DestroyThreads(&NMemory::g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer void CSystem::f_MemoryManager_CanStartThreads()
	{
		return NMemory::g_CrossModule.m_fMemoryManager_CanStartThreads(&NMemory::g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer void CSystem::f_MemoryManager_SetNumaNode(ENumaNode _NumaNode)
	{
		return NMemory::g_CrossModule.m_fMemoryManager_SetNumaNode(&NMemory::g_CrossModule, _NumaNode);
	}

	DMibSuppressUndefinedSanitizer void CSystem::f_MemoryManager_OnThreadCreated(umint _ThreadID, umint _ParentID)
	{
		return NMemory::g_CrossModule.m_fMemoryManager_OnThreadCreated(&NMemory::g_CrossModule, _ThreadID, _ParentID);
	}
}

namespace NMib::NMemory
{
	DMibSuppressUndefinedSanitizer void *fg_AllocWithSize(umint &_Size)
	{
		return g_CrossModule.m_fAllocWithSize(&g_CrossModule, _Size);
	}

	DMibSuppressUndefinedSanitizer void *fg_Alloc(umint _Size)
	{
		return g_CrossModule.m_fAlloc(&g_CrossModule, _Size);
	}

	DMibSuppressUndefinedSanitizer void *fg_AllocInitZeroWithSize(umint &_Size)
	{
		return g_CrossModule.m_fAllocInitZeroWithSize(&g_CrossModule, _Size);
	}

	DMibSuppressUndefinedSanitizer void *fg_AllocInitZero(umint _Size)
	{
		return g_CrossModule.m_fAllocInitZero(&g_CrossModule, _Size);
	}

	DMibSuppressUndefinedSanitizer void *fg_AllocAlignedWithSize(umint &_Size, umint _Alignment)
	{
		return g_CrossModule.m_fAllocAlignedWithSize(&g_CrossModule, _Size, _Alignment);
	}

	DMibSuppressUndefinedSanitizer void *fg_AllocAligned(umint _Size, umint _Alignment)
	{
		return g_CrossModule.m_fAllocAligned(&g_CrossModule, _Size, _Alignment);
	}

	bool DMibCrossmoduleAPI fg_CallBatchFunctor(void *_pContext, void * _pAlloc, umint _Size)
	{
		NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const *pFunctor = fg_AutoStaticCast(_pContext);
		return (*pFunctor)(_pAlloc, _Size);
	}

	DMibSuppressUndefinedSanitizer void fg_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor)
	{
		return g_CrossModule.m_fAllocBatch(&g_CrossModule, _Size, _Alignment, &fg_CallBatchFunctor, (void *)&_Functor);
	}

	DMibSuppressUndefinedSanitizer bool fg_AllocHasDeterministicSize()
	{
		return g_CrossModule.m_fAllocHasDeterministicSize(&g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer EMemoryManagerFeatureFlag fg_MemoryManagerFeatures()
	{
		return g_CrossModule.m_fMemoryManagerFeatures(&g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer void fg_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		return g_CrossModule.m_fAllocBatchDebug(&g_CrossModule, _Size, _Alignment, &fg_CallBatchFunctor, (void *)&_Functor, _pFile, _Line, _Flags);
	}

#	if DMibConfig_MalterlibMemoryManager_Debug
		DMibSuppressUndefinedSanitizer void *fg_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return g_CrossModule.m_fAllocWithSizeDebug(&g_CrossModule, _Size, _pFile, _Line, _Flags);
		}
		DMibSuppressUndefinedSanitizer void *fg_AllocDebug(umint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return g_CrossModule.m_fAllocWithSizeDebug(&g_CrossModule, _Size, _pFile, _Line, _Flags);
		}
		DMibSuppressUndefinedSanitizer void *fg_AllocAlignedWithSizeDebug(umint &_Size, umint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return g_CrossModule.m_fAllocAlignedWithSizeDebug(&g_CrossModule, _Size, _Alignment, _pFile, _Line, _Flags);
		}
		DMibSuppressUndefinedSanitizer void *fg_AllocAlignedDebug(umint _Size, umint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return g_CrossModule.m_fAllocAlignedWithSizeDebug(&g_CrossModule, _Size, _Alignment, _pFile, _Line, _Flags);
		}
		DMibSuppressUndefinedSanitizer void *fg_ReallocDebug(void *_pMemory, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
		{
			return g_CrossModule.m_fReallocDebug(&g_CrossModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
		}
		DMibSuppressUndefinedSanitizer void *fg_ResizeDebug(void *_pMemory, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
		{
			return g_CrossModule.m_fResizeDebug(&g_CrossModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
		}
#	endif

	DMibSuppressUndefinedSanitizer void *fg_Realloc(void *_pMemory, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags)
	{
		return g_CrossModule.m_fRealloc(&g_CrossModule, _pMemory, _Size, _OldSize, _AllocFlags);
	}

	DMibSuppressUndefinedSanitizer void *fg_Resize(void *_pMemory, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags)
	{
		return g_CrossModule.m_fResize(&g_CrossModule, _pMemory, _Size, _OldSize, _AllocFlags);
	}

	DMibSuppressUndefinedSanitizer void fg_FreeNoSize(void *_pMemory)
	{
		return g_CrossModule.m_fFreeNoSize(&g_CrossModule, _pMemory);
	}

	DMibSuppressUndefinedSanitizer void fg_Free(void *_pMemory, umint _Size)
	{
		return g_CrossModule.m_fFree(&g_CrossModule, _pMemory, _Size);
	}

	DMibSuppressUndefinedSanitizer umint fg_Size(const void *_pMemory)
	{
		return g_CrossModule.m_fSize(&g_CrossModule, _pMemory);
	}

	DMibSuppressUndefinedSanitizer umint fg_TrySize(const void *_pMemory)
	{
		return g_CrossModule.m_fTrySize(&g_CrossModule, _pMemory);
	}

	DMibSuppressUndefinedSanitizer umint fg_SizePadded(umint _Size)
	{
		return g_CrossModule.m_fSizePadded(&g_CrossModule, _Size);
	}

	DMibSuppressUndefinedSanitizer fp32 fg_Overhead(void const *_pMemory)
	{
		return g_CrossModule.m_fOverhead(&g_CrossModule, _pMemory);
	}

	DMibSuppressUndefinedSanitizer umint fg_Granularity()
	{
		return g_CrossModule.m_fGranularity(&g_CrossModule);
	}

	bool fg_ContainsBlock(void const* _pPtr)
	{
		DMibPDebugBreak; // Cannot be implemented efficiently
		return false;
	}

	DMibSuppressUndefinedSanitizer void fg_DemandProtection()
	{
		return g_CrossModule.m_fDemandProtection(&g_CrossModule);
	}
}

///
/// Nontracked heap
/// ===============

namespace NMib::NMemory
{
	DMibSuppressUndefinedSanitizer umint CAllocator_NonTrackedHeap::f_GranularityAlloc(bool _bLargePages)
	{
		return g_CrossModule.m_fNonTracked_Granularity(&g_CrossModule);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased umint CAllocator_NonTrackedHeap::f_Size(void *_pMemory)
	{
		return g_CrossModule.m_fNonTracked_Size(&g_CrossModule, _pMemory);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased umint CAllocator_NonTrackedHeap::f_TrySize(void *_pMemory)
	{
		return g_CrossModule.m_fNonTracked_TrySize(&g_CrossModule, _pMemory);
	}

	DMibSuppressUndefinedSanitizer umint CAllocator_NonTrackedHeap::f_SizePadded(umint _Size)
	{
		return g_CrossModule.m_fNonTracked_SizePadded(&g_CrossModule, _Size);
	}

	DMibSuppressUndefinedSanitizer fp32 CAllocator_NonTrackedHeap::f_Overhead(void const *_pMemory) // Number of bytes overhead for block
	{
		return g_CrossModule.m_fNonTracked_Overhead(&g_CrossModule, _pMemory);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocWithSizeDebug(umint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return g_CrossModule.m_fNonTracked_AllocWithSizeDebug(&g_CrossModule, _Size, _pFile, _Line, _Flags);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_ReallocDebug(void *_pMemory, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return g_CrossModule.m_fNonTracked_ReallocDebug(&g_CrossModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_ResizeDebug(void *_pMemory, umint &_Size, umint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return g_CrossModule.m_fNonTracked_ResizeDebug(&g_CrossModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocAlignedWithSizeDebug(umint &_Size, umint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return g_CrossModule.m_fNonTracked_AllocAlignedWithSizeDebug(&g_CrossModule, _Size, _Alignment, _pFile, _Line, _Flags);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocWithSize(umint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return g_CrossModule.m_fNonTracked_AllocWithSize(&g_CrossModule, _Size);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_Alloc(umint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return g_CrossModule.m_fNonTracked_Alloc(&g_CrossModule, _Size);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocAlignedWithSize(umint &_Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return g_CrossModule.m_fNonTracked_AllocAlignedWithSize(&g_CrossModule, _Size, _Alignment);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocAligned(umint _Size, umint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return g_CrossModule.m_fNonTracked_AllocAligned(&g_CrossModule, _Size, _Alignment);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased void CAllocator_NonTrackedHeap::f_AllocBatch(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return g_CrossModule.m_fNonTracked_AllocBatch(&g_CrossModule, _Size, _Alignment, &fg_CallBatchFunctor, (void *)&_Functor);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased void CAllocator_NonTrackedHeap::f_AllocBatchDebug(umint _Size, umint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return g_CrossModule.m_fNonTracked_AllocBatchDebug(&g_CrossModule, _Size, _Alignment, &fg_CallBatchFunctor, (void *)&_Functor, _pFile, _Line, _Flags);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_Realloc(void *_pMemory, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return g_CrossModule.m_fNonTracked_Realloc(&g_CrossModule, _pMemory, _Size, _OldSize, _AllocFlags);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased void *CAllocator_NonTrackedHeap::f_Resize(void *_pMemory, umint &_Size, umint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return g_CrossModule.m_fNonTracked_Resize(&g_CrossModule, _pMemory, _Size, _OldSize, _AllocFlags);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased void CAllocator_NonTrackedHeap::f_Free(void *_pMemory, umint _Size)
	{
		return g_CrossModule.m_fNonTracked_Free(&g_CrossModule, _pMemory, _Size);
	}

	DMibSuppressUndefinedSanitizer only_parameters_aliased void CAllocator_NonTrackedHeap::f_FreeNoSize(void *_pMemory)
	{
		return g_CrossModule.m_fNonTracked_FreeNoSize(&g_CrossModule, _pMemory);
	}
}
