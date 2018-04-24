// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>
#include "Malterlib_Memory_SystemManager_CrossModule.h"

namespace NMib
{
	
	NMem::ICMemoryManagerCrossModule *g_pLocalCrossModuleInterface = nullptr;
	
	void CSystem::fp_CreateNonTrackedMemoryManager()
	{
		using namespace NMem;

		DMibFastCheck(fg_GetSys()->f_IsDll());
		
		g_pLocalCrossModuleInterface = (NMem::ICMemoryManagerCrossModule *)NSys::fg_Process_GetCrossModuleMemoryManagerInterface();
		
		if (g_pLocalCrossModuleInterface)
		{
			g_pLocalCrossModuleInterface->f_Register(&NMem::g_CrossModule, NMem::EMemoryManagerCrossModule_Version);
			if (NMem::g_CrossModule.m_Version < 0x102)
			{
				NMem::g_CrossModule.m_fReportingLeaks = [](CMemoryManagerCrossModule *_pModule) -> bool
					{
						return false;
					}
				;
			}

			if (NMem::g_CrossModule.m_Version < 0x103)
			{
				NMem::g_CrossModule.m_fAlloc = [](CMemoryManagerCrossModule *_pModule, mint _Size) -> void *
					{
						return NMem::g_CrossModule.m_fAllocWithSize(&g_CrossModule, _Size);
					}
				;
				NMem::g_CrossModule.m_fAllocInitZero = [](CMemoryManagerCrossModule *_pModule, mint _Size) -> void *
					{
						return NMem::g_CrossModule.m_fAllocInitZeroWithSize(&g_CrossModule, _Size);
					}
				;
				NMem::g_CrossModule.m_fAllocAligned = [](CMemoryManagerCrossModule *_pModule, mint _Size, mint _Align) -> void *
					{
						return NMem::g_CrossModule.m_fAllocAlignedWithSize(&g_CrossModule, _Size, _Align);
					}
				;

				NMem::g_CrossModule.m_fNonTracked_Alloc = [](CMemoryManagerCrossModule *_pModule, mint _Size) -> void *
					{
						return NMem::g_CrossModule.m_fNonTracked_AllocWithSize(&g_CrossModule, _Size);
					}
				;
				NMem::g_CrossModule.m_fNonTracked_AllocAligned = [](CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment) -> void *
					{
						return NMem::g_CrossModule.m_fNonTracked_AllocAlignedWithSize(&g_CrossModule, _Size, _Alignment);
					}
				;

				NMem::g_CrossModule.m_fReallocDebug = [](CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags) -> void *
					{
						return NMem::g_CrossModule.m_fReallocNoOldDebug(&g_CrossModule, _pMemory, _Size, _pFile, _Line, _Flags);
					}
				;
				NMem::g_CrossModule.m_fResizeDebug = [](CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags) -> void *
					{
						return NMem::g_CrossModule.m_fResizeNoOldDebug(&g_CrossModule, _pMemory, _Size, _pFile, _Line, _Flags);
					}
				;
				NMem::g_CrossModule.m_fRealloc = [](CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags) -> void *
					{
						return NMem::g_CrossModule.m_fReallocNoOld(&g_CrossModule, _pMemory, _Size);
					}
				;
				NMem::g_CrossModule.m_fResize = [](CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags) -> void *
					{
						return NMem::g_CrossModule.m_fResizeNoOld(&g_CrossModule, _pMemory, _Size);
					}
				;

				NMem::g_CrossModule.m_fNonTracked_Realloc = [](CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags) -> void *
					{
						return NMem::g_CrossModule.m_fNonTracked_ReallocNoOld(&g_CrossModule, _pMem, _Size);
					}
				;

				NMem::g_CrossModule.m_fNonTracked_Resize = [](CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags) -> void *
					{
						return NMem::g_CrossModule.m_fNonTracked_ResizeNoOld(&g_CrossModule, _pMem, _Size);
					}
				;

				NMem::g_CrossModule.m_fNonTracked_ReallocDebug = [](CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags) -> void *
					{
						return NMem::g_CrossModule.m_fNonTracked_ReallocNoOldDebug(&g_CrossModule, _pMem, _Size, _pFile, _Line, _Flags);
					}
				;

				NMem::g_CrossModule.m_fNonTracked_ResizeDebug = [](CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags) -> void *
					{
						return NMem::g_CrossModule.m_fNonTracked_ResizeNoOldDebug(&g_CrossModule, _pMem, _Size, _pFile, _Line, _Flags);
					}
				;

				NMem::g_CrossModule.m_fFree = [](CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
					{
						return NMem::g_CrossModule.m_fFreeNoSize(&g_CrossModule, _pMemory);
					}
				;

				NMem::g_CrossModule.m_fNonTracked_Free = [](CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
					{
						return NMem::g_CrossModule.m_fNonTracked_FreeNoSize(&g_CrossModule, _pMemory);
					}
				;
			}
		}
		
		NMem::g_CrossModule.m_fCreateNonTrackedMemoryManager(&NMem::g_CrossModule);
	}
	
	void CSystem::fp_DestroyNonTrackedMemoryManager()
	{
		NMem::g_CrossModule.m_fDestroyNonTrackedMemoryManager(&NMem::g_CrossModule);
		
		if (g_pLocalCrossModuleInterface)
			g_pLocalCrossModuleInterface->f_Unregister(&NMem::g_CrossModule);
	}
	
	void CSystem::f_MemoryManager_GarbageCollect()
	{
		NMem::g_CrossModule.m_fMemoryManager_GarbageCollect(&NMem::g_CrossModule);
	}
	
	void CSystem::fp_CreateMemoryManager()
	{
		NMem::g_CrossModule.m_fCreateMemoryManager(&NMem::g_CrossModule);
	}
	NMem::CMemoryManagerCheckout CSystem::f_MemoryManager_Checkout()
	{
		return NMem::g_CrossModule.m_fMemoryManager_Checkout(&NMem::g_CrossModule);
	}	
	
	void CSystem::fp_DestroyMemoryManager()
	{
		return NMem::g_CrossModule.m_fDestroyMemoryManager(&NMem::g_CrossModule);
	}
	
	bool CSystem::f_MemoryManager_Check(bool _bBreak)
	{
		return NMem::g_CrossModule.m_fMemoryManager_Check(&NMem::g_CrossModule, _bBreak);
	}

	bool CSystem::f_MemoryManager_ReportingLeaks()
	{
		return NMem::g_CrossModule.m_fReportingLeaks(&NMem::g_CrossModule);
	}

	void CSystem::f_MemoryManager_PrepareFork()
	{
		return NMem::g_CrossModule.m_fMemoryManager_PrepareFork(&NMem::g_CrossModule);
	}
	
	void CSystem::f_MemoryManager_ForkedParent()
	{
		return NMem::g_CrossModule.m_fMemoryManager_ForkedParent(&NMem::g_CrossModule);
	}
	
	void CSystem::f_MemoryManager_ForkedChild()
	{
		return NMem::g_CrossModule.m_fMemoryManager_ForkedChild(&NMem::g_CrossModule);
	}
	
	void CSystem::f_MemoryManager_DestroyThreads()
	{
		return NMem::g_CrossModule.m_fMemoryManager_DestroyThreads(&NMem::g_CrossModule);
	}
	
	void CSystem::f_MemoryManager_CanStartThreads()
	{
		return NMem::g_CrossModule.m_fMemoryManager_CanStartThreads(&NMem::g_CrossModule);
	}	

	void CSystem::f_MemoryManager_SetNumaNode(ENumaNode _NumaNode)
	{
		return NMem::g_CrossModule.m_fMemoryManager_SetNumaNode(&NMem::g_CrossModule, _NumaNode);
	}

	void CSystem::f_MemoryManager_OnThreadCreated(mint _ThreadID, mint _ParentID)
	{
		return NMem::g_CrossModule.m_fMemoryManager_OnThreadCreated(&NMem::g_CrossModule, _ThreadID, _ParentID);
	}


	
	namespace NMem
	{
		void *fg_AllocWithSize(mint &_Size)
		{
			return g_CrossModule.m_fAllocWithSize(&g_CrossModule, _Size);
		}

		void *fg_Alloc(mint _Size)
		{
			return g_CrossModule.m_fAlloc(&g_CrossModule, _Size);
		}

		void *fg_AllocInitZeroWithSize(mint &_Size)
		{
			return g_CrossModule.m_fAllocInitZeroWithSize(&g_CrossModule, _Size);
		}

		void *fg_AllocInitZero(mint _Size)
		{
			return g_CrossModule.m_fAllocInitZero(&g_CrossModule, _Size);
		}

		void *fg_AllocAlignedWithSize(mint &_Size, mint _Alignment)
		{
			return g_CrossModule.m_fAllocAlignedWithSize(&g_CrossModule, _Size, _Alignment);
		}

		void *fg_AllocAligned(mint _Size, mint _Alignment)
		{
			return g_CrossModule.m_fAllocAligned(&g_CrossModule, _Size, _Alignment);
		}

		bool DMibCrossmoduleAPI fg_CallBatchFunctor(void *_pContext, void * _pAlloc, mint _Size)
		{
			NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const *pFunctor = fg_AutoStaticCast(_pContext);
			return (*pFunctor)(_pAlloc, _Size);
		}
		
		void fg_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			return g_CrossModule.m_fAllocBatch(&g_CrossModule, _Size, _Alignment, &fg_CallBatchFunctor, (void *)&_Functor);
		}
		
		void fg_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return g_CrossModule.m_fAllocBatchDebug(&g_CrossModule, _Size, _Alignment, &fg_CallBatchFunctor, (void *)&_Functor, _pFile, _Line, _Flags);
		}

#		if DMibConfig_MalterlibMemoryManager_Debug
			void *fg_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return g_CrossModule.m_fAllocWithSizeDebug(&g_CrossModule, _Size, _pFile, _Line, _Flags);
			}
			void *fg_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return g_CrossModule.m_fAllocWithSizeDebug(&g_CrossModule, _Size, _pFile, _Line, _Flags);
			}
			void *fg_AllocAlignedWithSizeDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return g_CrossModule.m_fAllocAlignedWithSizeDebug(&g_CrossModule, _Size, _Alignment, _pFile, _Line, _Flags);
			}
			void *fg_AllocAlignedDebug(mint _Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return g_CrossModule.m_fAllocAlignedWithSizeDebug(&g_CrossModule, _Size, _Alignment, _pFile, _Line, _Flags);
			}
			void *fg_ReallocDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
			{
				return g_CrossModule.m_fReallocDebug(&g_CrossModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
			}
			void *fg_ResizeDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
			{
				return g_CrossModule.m_fResizeDebug(&g_CrossModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
			}
#		endif

		void *fg_Realloc(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			return g_CrossModule.m_fRealloc(&g_CrossModule, _pMemory, _Size, _OldSize, _AllocFlags);
		}

		void *fg_Resize(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			return g_CrossModule.m_fResize(&g_CrossModule, _pMemory, _Size, _OldSize, _AllocFlags);
		}

		void fg_FreeNoSize(void *_pMemory)
		{
			return g_CrossModule.m_fFreeNoSize(&g_CrossModule, _pMemory);
		}

		void fg_Free(void *_pMemory, mint _Size)
		{
			return g_CrossModule.m_fFree(&g_CrossModule, _pMemory, _Size);
		}

		mint fg_Size(const void *_pMemory)
		{
			return g_CrossModule.m_fSize(&g_CrossModule, _pMemory);
		}

		mint fg_TrySize(const void *_pMemory)
		{
			return g_CrossModule.m_fTrySize(&g_CrossModule, _pMemory);
		}

		mint fg_SizePadded(mint _Size)
		{
			return g_CrossModule.m_fSizePadded(&g_CrossModule, _Size);
		}
		
		fp32 fg_Overhead(void const *_pMemory)
		{
			return g_CrossModule.m_fOverhead(&g_CrossModule, _pMemory);
		}
		
		mint fg_Granularity()
		{
			return g_CrossModule.m_fGranularity(&g_CrossModule);
		}

		bool fg_ContainsBlock(void const* _pPtr)
		{
			DMibPDebugBreak; // Cannot be implemented efficiently
			return false;
		}

		void fg_DemandProtection()
		{
			return g_CrossModule.m_fDemandProtection(&g_CrossModule);
		}
	}
	
	///
	/// Nontracked heap
	/// ===============

	namespace NMem
	{
		mint CAllocator_NonTrackedHeap::f_GranularityAlloc(bint _bLargePages)
		{
			return g_CrossModule.m_fNonTracked_Granularity(&g_CrossModule);
		}

		only_parameters_aliased mint CAllocator_NonTrackedHeap::f_Size(void *_pMemory)
		{
			return g_CrossModule.m_fNonTracked_Size(&g_CrossModule, _pMemory);
		}

		only_parameters_aliased mint CAllocator_NonTrackedHeap::f_TrySize(void *_pMemory)
		{
			return g_CrossModule.m_fNonTracked_TrySize(&g_CrossModule, _pMemory);
		}

		mint CAllocator_NonTrackedHeap::f_SizePadded(mint _Size)
		{
			return g_CrossModule.m_fNonTracked_SizePadded(&g_CrossModule, _Size);
		}

		fp32 CAllocator_NonTrackedHeap::f_Overhead(void const *_pMemory) // Number of bytes overhead for block
		{
			return g_CrossModule.m_fNonTracked_Overhead(&g_CrossModule, _pMemory);
		}
		
		only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return g_CrossModule.m_fNonTracked_AllocWithSizeDebug(&g_CrossModule, _Size, _pFile, _Line, _Flags);
		}

		only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_ReallocDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return g_CrossModule.m_fNonTracked_ReallocDebug(&g_CrossModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
		}

		only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_ResizeDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return g_CrossModule.m_fNonTracked_ResizeDebug(&g_CrossModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
		}

		only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocAlignedWithSizeDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return g_CrossModule.m_fNonTracked_AllocAlignedWithSizeDebug(&g_CrossModule, _Size, _Alignment, _pFile, _Line, _Flags);
		}
		
		only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return g_CrossModule.m_fNonTracked_AllocWithSize(&g_CrossModule, _Size);
		}

		only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_Alloc(mint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return g_CrossModule.m_fNonTracked_Alloc(&g_CrossModule, _Size);
		}

		only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocAlignedWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return g_CrossModule.m_fNonTracked_AllocAlignedWithSize(&g_CrossModule, _Size, _Alignment);
		}

		only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return g_CrossModule.m_fNonTracked_AllocAligned(&g_CrossModule, _Size, _Alignment);
		}

		only_parameters_aliased void CAllocator_NonTrackedHeap::f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return g_CrossModule.m_fNonTracked_AllocBatch(&g_CrossModule, _Size, _Alignment, &fg_CallBatchFunctor, (void *)&_Functor);
		}			

		only_parameters_aliased void CAllocator_NonTrackedHeap::f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return g_CrossModule.m_fNonTracked_AllocBatchDebug(&g_CrossModule, _Size, _Alignment, &fg_CallBatchFunctor, (void *)&_Functor, _pFile, _Line, _Flags);
		}			
		
		only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_Realloc(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return g_CrossModule.m_fNonTracked_Realloc(&g_CrossModule, _pMemory, _Size, _OldSize, _AllocFlags);
		}

		only_parameters_aliased void *CAllocator_NonTrackedHeap::f_Resize(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return g_CrossModule.m_fNonTracked_Resize(&g_CrossModule, _pMemory, _Size, _OldSize, _AllocFlags);
		}

		only_parameters_aliased void CAllocator_NonTrackedHeap::f_Free(void *_pMemory, mint _Size)
		{
			return g_CrossModule.m_fNonTracked_Free(&g_CrossModule, _pMemory, _Size);
		}

		only_parameters_aliased void CAllocator_NonTrackedHeap::f_FreeNoSize(void *_pMemory)
		{
			return g_CrossModule.m_fNonTracked_FreeNoSize(&g_CrossModule, _pMemory);
		}

	} // Namespace NMem
}
