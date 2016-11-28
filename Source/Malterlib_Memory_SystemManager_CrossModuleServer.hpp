// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>
#include "Malterlib_Memory_SystemManager_CrossModule.h"

namespace NMib
{
	
	struct CMemoryManagerCrossModuleInterfaceServer : NMem::ICMemoryManagerCrossModule
	{
		void f_Register(NMem::CMemoryManagerCrossModule *_pModule, uint32 _Version) override;
		void f_Unregister(NMem::CMemoryManagerCrossModule *_pModule) override;
	};
	
	NAggregate::TCAggregateSimple<CMemoryManagerCrossModuleInterfaceServer> g_CrossModuleInterfaceServer = {DAggregateInit};

	void CSystem::fp_CreateNonTrackedMemoryManager()
	{
#if DMibConfig_MemoryManager_CrossModule_Enable
#if !defined(DMibMemoryOverrideDll)
		DMibFastCheck(!fg_GetSys()->f_IsDll());
#endif
#endif
		NMem::CCrossModuleImplementationExtra::fs_CreateNonTrackedMemoryManager(&NMem::g_CrossModule);
	}
	
	void CSystem::fp_DestroyNonTrackedMemoryManager()
	{
		NMem::CCrossModuleImplementationExtra::fs_DestroyNonTrackedMemoryManager(&NMem::g_CrossModule);
	}
	
	void CSystem::f_MemoryManager_GarbageCollect()
	{
		NMem::CCrossModuleImplementationExtra::fs_MemoryManager_GarbageCollect(&NMem::g_CrossModule);
	}
	
	void CSystem::fp_CreateMemoryManager()
	{
		NMem::CCrossModuleImplementationExtra::fs_CreateMemoryManager(&NMem::g_CrossModule);

#if !defined(DMibDynamicLibrary) || defined(DMibMemoryOverrideDll)
		g_CrossModuleInterfaceServer.f_Construct();
		NSys::fg_Process_SetCrossModuleMemoryManagerInterface(&*g_CrossModuleInterfaceServer);
#endif
	}
	NMem::CMemoryManagerCheckout CSystem::f_MemoryManager_Checkout()
	{
		return NMem::CCrossModuleImplementationExtra::fs_MemoryManager_Checkout(&NMem::g_CrossModule);
	}	
	
	void CSystem::fp_DestroyMemoryManager()
	{
		NSys::fg_PreDestroyHeap();
		return NMem::CCrossModuleImplementationExtra::fs_DestroyMemoryManager(&NMem::g_CrossModule);
	}
	
	bool CSystem::f_MemoryManager_Check(bool _bBreak)
	{
		return NMem::CCrossModuleImplementationExtra::fs_MemoryManager_Check(&NMem::g_CrossModule, _bBreak);
	}

	bool CSystem::f_MemoryManager_ReportingLeaks()
	{
		return NMem::CCrossModuleImplementationExtra::fs_ReportingLeaks(&NMem::g_CrossModule);
	}

	void CSystem::f_MemoryManager_PrepareFork()
	{
		return NMem::CCrossModuleImplementationExtra::fs_MemoryManager_PrepareFork(&NMem::g_CrossModule);
	}
	
	void CSystem::f_MemoryManager_ForkedParent()
	{
		return NMem::CCrossModuleImplementationExtra::fs_MemoryManager_ForkedParent(&NMem::g_CrossModule);
	}
	
	void CSystem::f_MemoryManager_ForkedChild()
	{
		return NMem::CCrossModuleImplementationExtra::fs_MemoryManager_ForkedChild(&NMem::g_CrossModule);
	}
	
	void CSystem::f_MemoryManager_DestroyThreads()
	{
		return NMem::CCrossModuleImplementationExtra::fs_MemoryManager_DestroyThreads(&NMem::g_CrossModule);
	}
	
	void CSystem::f_MemoryManager_CanStartThreads()
	{
		return NMem::CCrossModuleImplementationExtra::fs_MemoryManager_CanStrartThreads(&NMem::g_CrossModule);
	}
	
	

	void CSystem::f_MemoryManager_SetNumaNode(ENumaNode _NumaNode)
	{
		return NMem::CCrossModuleImplementationExtra::fs_MemoryManager_SetNumaNode(&NMem::g_CrossModule, _NumaNode);
	}

	void CSystem::f_MemoryManager_OnThreadCreated(mint _ThreadID, mint _ParentID)
	{
		return NMem::CCrossModuleImplementationExtra::fs_MemoryManager_OnThreadCreated(&NMem::g_CrossModule, _ThreadID, _ParentID);
	}
	
	namespace NMem
	{
		DMibMemory_MemoryManagerExport void * fg_Alloc(mint &_Size)
		{
			return CCrossModuleImplementationExtra::fs_Alloc(&g_CrossModule, _Size);
		}

		DMibMemory_MemoryManagerExport void * fg_AllocNoSize(mint _Size)
		{
			return CCrossModuleImplementationExtra::fs_Alloc(&g_CrossModule, _Size);
		}

		DMibMemory_MemoryManagerExport void * fg_AllocInitZero(mint &_Size)
		{
			return CCrossModuleImplementationExtra::fs_AllocInitZero(&g_CrossModule, _Size);
		}
		
		DMibMemory_MemoryManagerExport void * fg_AllocInitZeroNoSize(mint _Size)
		{
			return CCrossModuleImplementationExtra::fs_AllocInitZero(&g_CrossModule, _Size);
		}

		DMibMemory_MemoryManagerExport void * fg_AllocAligned(mint &_Size, mint _Alignment)
		{
			return CCrossModuleImplementationExtra::fs_AllocAligned(&g_CrossModule, _Size, _Alignment);
		}
		
		DMibMemory_MemoryManagerExport void  fg_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			return CCrossModuleImplementationExtra::fs_AllocBatchInternal(&g_CrossModule, _Size, _Alignment, _Functor);
		}		

#		if DMibConfig_MalterlibMemoryManager_Debug
			DMibMemory_MemoryManagerExport void * fg_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return CCrossModuleImplementationExtra::fs_AllocDebug(&g_CrossModule, _Size, _pFile, _Line, _Flags);
			}
			DMibMemory_MemoryManagerExport void * fg_AllocAlignedDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return CCrossModuleImplementationExtra::fs_AllocAlignedDebug(&g_CrossModule, _Size, _Alignment, _pFile, _Line, _Flags);
			}
			DMibMemory_MemoryManagerExport void * fg_ReallocDebug(void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return CCrossModuleImplementationExtra::fs_ReallocDebug(&g_CrossModule, _pMemory, _Size, _pFile, _Line, _Flags);
			}
			DMibMemory_MemoryManagerExport void * fg_ResizeDebug(void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return CCrossModuleImplementationExtra::fs_ResizeDebug(&g_CrossModule, _pMemory, _Size, _pFile, _Line, _Flags);
			}
			DMibMemory_MemoryManagerExport void fg_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return CCrossModuleImplementationExtra::fs_AllocBatchDebugInternal(&g_CrossModule, _Size, _Alignment, _Functor, _pFile, _Line, _Flags);
			}
#		endif

		DMibMemory_MemoryManagerExport void * fg_Realloc(void *_pMemory, mint &_Size)
		{
			return CCrossModuleImplementationExtra::fs_Realloc(&g_CrossModule, _pMemory, _Size);
		}

		DMibMemory_MemoryManagerExport void * fg_Resize(void *_pMemory, mint &_Size)
		{
			return CCrossModuleImplementationExtra::fs_Resize(&g_CrossModule, _pMemory, _Size);
		}

		DMibMemory_MemoryManagerExport void fg_Free(void *_pMemory)
		{
			return CCrossModuleImplementationExtra::fs_Free(&g_CrossModule, _pMemory);
		}

		DMibMemory_MemoryManagerExport mint fg_Size(const void *_pMemory)
		{
			return CCrossModuleImplementationExtra::fs_Size(&g_CrossModule, _pMemory);
		}

		DMibMemory_MemoryManagerExport mint fg_TrySize(const void *_pMemory)
		{
			return CCrossModuleImplementationExtra::fs_TrySize(&g_CrossModule, _pMemory);
		}

		DMibMemory_MemoryManagerExport mint fg_SizePadded(mint _Size)
		{
			return CCrossModuleImplementationExtra::fs_SizePadded(&g_CrossModule, _Size);
		}
		
		DMibMemory_MemoryManagerExport fp32 fg_Overhead(void const *_pMemory)
		{
			return CCrossModuleImplementationExtra::fs_Overhead(&g_CrossModule, _pMemory);
		}
		
		DMibMemory_MemoryManagerExport mint fg_Granularity()
		{
			return CCrossModuleImplementationExtra::fs_Granularity(&g_CrossModule);
		}

		DMibMemory_MemoryManagerExport bool fg_ContainsBlock(void const* _pPtr)
		{
			DMibPDebugBreak; // Cannot be implemented efficiently
			return false;
		}
		DMibMemory_MemoryManagerExport void fg_DemandProtection()
		{
			return CCrossModuleImplementationExtra::fs_DemandProtection(&g_CrossModule);
		}
	}
	
	///
	/// Nontracked heap
	/// ===============

	namespace NMem
	{
		mint CAllocator_NonTrackedHeap::f_GranularityAlloc(bint _bLargePages)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_Granularity(&g_CrossModule);
		}

		only_parameters_aliased mint CAllocator_NonTrackedHeap::f_Size(void *_pMemory)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_Size(&g_CrossModule, _pMemory);
		}

		only_parameters_aliased mint CAllocator_NonTrackedHeap::f_TrySize(void *_pMemory)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_TrySize(&g_CrossModule, _pMemory);
		}

		mint CAllocator_NonTrackedHeap::f_SizePadded(mint _Size)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_SizePadded(&g_CrossModule, _Size);
		}

		fp32 CAllocator_NonTrackedHeap::f_Overhead(void const *_pMemory) // Number of bytes overhead for block
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_Overhead(&g_CrossModule, _pMemory);
		}
		
		only_parameters_aliased return_not_aliased void *CAllocator_NonTrackedHeap::f_AllocDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_AllocDebug(&g_CrossModule, _Size, _pFile, _Line, _Flags);
		}
		only_parameters_aliased return_not_aliased void *CAllocator_NonTrackedHeap::f_ReallocDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_ReallocDebug(&g_CrossModule, _pMemory, _Size, _pFile, _Line, _Flags);
		}
		only_parameters_aliased return_not_aliased void *CAllocator_NonTrackedHeap::f_ResizeDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_ResizeDebug(&g_CrossModule,_pMemory, _Size, _pFile, _Line, _Flags);
		}
	
		only_parameters_aliased return_not_aliased void *CAllocator_NonTrackedHeap::f_AllocAlignedDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedDebug(&g_CrossModule, _Size, _Alignment, _pFile, _Line, _Flags);
		}
		
		only_parameters_aliased return_not_aliased void *CAllocator_NonTrackedHeap::f_Alloc(mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_Alloc(&g_CrossModule, _Size);
		}
		only_parameters_aliased return_not_aliased void *CAllocator_NonTrackedHeap::f_AllocAligned(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_AllocAligned(&g_CrossModule, _Size, _Alignment);
		}
		only_parameters_aliased void CAllocator_NonTrackedHeap::f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_AllocBatchInternal(&g_CrossModule, _Size, _Alignment, _Functor);
		}			

		only_parameters_aliased void CAllocator_NonTrackedHeap::f_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_AllocBatchDebugInternal(&g_CrossModule, _Size, _Alignment, _Functor, _pFile, _Line, _Flags);
		}			
		
		only_parameters_aliased return_not_aliased void *CAllocator_NonTrackedHeap::f_Realloc(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_Realloc(&g_CrossModule, _pMemory, _Size);
		}

		only_parameters_aliased void *CAllocator_NonTrackedHeap::f_Resize(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_Resize(&g_CrossModule, _pMemory, _Size);
		}

		only_parameters_aliased void CAllocator_NonTrackedHeap::f_Free(void *_pMemory, mint _Size)
		{
			return CCrossModuleImplementationExtra::fs_NonTracked_Free(&g_CrossModule, _pMemory);
		}

	} // Namespace NMem

	namespace NCrossModuleServer
	{
		void fg_VoidDummy(NMem::CMemoryManagerCrossModule *_pModule)
		{
		}
		void * fg_AllocDebug(NMem::CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
			return NMem::CCrossModuleImplementationExtra::fs_AllocDebug(_pModule, _Size, _pFile, _Line, _Flags);
		}
		void * fg_AllocAlignedDebug(NMem::CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
			return NMem::CCrossModuleImplementationExtra::fs_AllocAlignedDebug(_pModule, _Size, _Alignment, _pFile, _Line, _Flags);
		}
		void * fg_ReallocDebug(NMem::CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
			return NMem::CCrossModuleImplementationExtra::fs_ReallocDebug(_pModule, _pMemory, _Size, _pFile, _Line, _Flags);
		}
		void * fg_ResizeDebug(NMem::CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
			return NMem::CCrossModuleImplementationExtra::fs_ResizeDebug(_pModule, _pMemory, _Size, _pFile, _Line, _Flags);
		}
		void * fg_NonTracked_AllocDebug(NMem::CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
			return NMem::CCrossModuleImplementationExtra::fs_NonTracked_AllocDebug(_pModule, _Size, _pFile, _Line, _Flags);
		}
		void * fg_NonTracked_ReallocDebug(NMem::CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
			return NMem::CCrossModuleImplementationExtra::fs_NonTracked_ReallocDebug(_pModule, _pMemory, _Size, _pFile, _Line, _Flags);
		}
		void * fg_NonTracked_ResizeDebug(NMem::CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
			return NMem::CCrossModuleImplementationExtra::fs_NonTracked_ResizeDebug(_pModule, _pMemory, _Size, _pFile, _Line, _Flags);
		}
		void * fg_NonTracked_AllocAlignedDebug(NMem::CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
			return NMem::CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedDebug(_pModule, _Size, _Alignment, _pFile, _Line, _Flags);
		}
		
		void fg_AllocBatchDebug(NMem::CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
			return NMem::CCrossModuleImplementationExtra::fs_AllocBatchDebug(_pModule, _Size, _Alignment, _fCallBatchFunctor, _pContext, _pFile, _Line, _Flags);
		}

		void fg_NonTracked_AllocBatchDebug(NMem::CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
			return NMem::CCrossModuleImplementationExtra::fs_NonTracked_AllocBatchDebug(_pModule, _Size, _Alignment, _fCallBatchFunctor, _pContext, _pFile, _Line, _Flags);
		}

		void fg_MemoryManager_OnThreadCreated(NMem::CMemoryManagerCrossModule *_pModule, mint _ThreadID, mint _ParentID)
		{
			fg_GetSys()->f_ThreadLocalCreateThread(_ThreadID, _ParentID);
		}
	}
	
	void CMemoryManagerCrossModuleInterfaceServer::f_Register(NMem::CMemoryManagerCrossModule *_pModule, uint32 _Version)
	{
		_pModule->m_Version = NMem::g_CrossModule.m_Version;
		_pModule->m_fCreateNonTrackedMemoryManager = &NCrossModuleServer::fg_VoidDummy;
		_pModule->m_fDestroyNonTrackedMemoryManager = &NCrossModuleServer::fg_VoidDummy;
		_pModule->m_fMemoryManager_GarbageCollect = NMem::g_CrossModule.m_fMemoryManager_GarbageCollect;
		_pModule->m_fCreateMemoryManager = &NCrossModuleServer::fg_VoidDummy;
		_pModule->m_fMemoryManager_Checkout = NMem::g_CrossModule.m_fMemoryManager_Checkout;
		_pModule->m_fDestroyMemoryManager = &NCrossModuleServer::fg_VoidDummy;
		_pModule->m_fMemoryManager_Check = NMem::g_CrossModule.m_fMemoryManager_Check;
		_pModule->m_fMemoryManager_PrepareFork = NMem::g_CrossModule.m_fMemoryManager_PrepareFork;
		_pModule->m_fMemoryManager_ForkedParent = NMem::g_CrossModule.m_fMemoryManager_ForkedParent;
		_pModule->m_fMemoryManager_ForkedChild = NMem::g_CrossModule.m_fMemoryManager_ForkedChild;
		_pModule->m_fMemoryManager_DestroyThreads = &NCrossModuleServer::fg_VoidDummy;
		_pModule->m_fMemoryManager_CanStartThreads = &NCrossModuleServer::fg_VoidDummy;
		_pModule->m_fMemoryManager_SetNumaNode = NMem::g_CrossModule.m_fMemoryManager_SetNumaNode;
		_pModule->m_fMemoryManager_OnThreadCreated = &NCrossModuleServer::fg_MemoryManager_OnThreadCreated;
		_pModule->m_fDemandProtection = NMem::g_CrossModule.m_fDemandProtection;
		
		_pModule->m_fAlloc = NMem::g_CrossModule.m_fAlloc;
		_pModule->m_fAllocInitZero = NMem::g_CrossModule.m_fAllocInitZero;
		_pModule->m_fAllocAligned = NMem::g_CrossModule.m_fAllocAligned;
		_pModule->m_fAllocBatch = NMem::g_CrossModule.m_fAllocBatch;
		_pModule->m_fAllocBatchDebug = &NCrossModuleServer::fg_AllocBatchDebug;
		_pModule->m_fAllocDebug = &NCrossModuleServer::fg_AllocDebug;
		_pModule->m_fAllocAlignedDebug = &NCrossModuleServer::fg_AllocAlignedDebug;
		_pModule->m_fReallocDebug = &NCrossModuleServer::fg_ReallocDebug;
		_pModule->m_fResizeDebug = &NCrossModuleServer::fg_ResizeDebug;
		_pModule->m_fRealloc = NMem::g_CrossModule.m_fRealloc;
		_pModule->m_fResize = NMem::g_CrossModule.m_fResize;
		_pModule->m_fFree = NMem::g_CrossModule.m_fFree;
		_pModule->m_fSize = NMem::g_CrossModule.m_fSize;
		_pModule->m_fTrySize = NMem::g_CrossModule.m_fTrySize;
		_pModule->m_fSizePadded = NMem::g_CrossModule.m_fSizePadded;
		_pModule->m_fOverhead = NMem::g_CrossModule.m_fOverhead;
		_pModule->m_fGranularity = NMem::g_CrossModule.m_fGranularity;
		
		_pModule->m_fNonTracked_Granularity = NMem::g_CrossModule.m_fNonTracked_Granularity;
		_pModule->m_fNonTracked_Size = NMem::g_CrossModule.m_fNonTracked_Size;
		_pModule->m_fNonTracked_TrySize = NMem::g_CrossModule.m_fNonTracked_TrySize;
		_pModule->m_fNonTracked_SizePadded = NMem::g_CrossModule.m_fNonTracked_SizePadded;
		_pModule->m_fNonTracked_Overhead = NMem::g_CrossModule.m_fNonTracked_Overhead;
		_pModule->m_fNonTracked_Alloc = NMem::g_CrossModule.m_fNonTracked_Alloc;
		_pModule->m_fNonTracked_AllocAligned = NMem::g_CrossModule.m_fNonTracked_AllocAligned;
		_pModule->m_fNonTracked_AllocBatch = NMem::g_CrossModule.m_fNonTracked_AllocBatch;
		_pModule->m_fNonTracked_AllocBatchDebug = &NCrossModuleServer::fg_NonTracked_AllocBatchDebug;
		_pModule->m_fNonTracked_Realloc = NMem::g_CrossModule.m_fNonTracked_Realloc;
		_pModule->m_fNonTracked_Resize = NMem::g_CrossModule.m_fNonTracked_Resize;
		_pModule->m_fNonTracked_Free = NMem::g_CrossModule.m_fNonTracked_Free;
		_pModule->m_fNonTracked_AllocDebug = &NCrossModuleServer::fg_NonTracked_AllocDebug;
		_pModule->m_fNonTracked_AllocAlignedDebug = &NCrossModuleServer::fg_NonTracked_AllocAlignedDebug;
		_pModule->m_fNonTracked_ReallocDebug = &NCrossModuleServer::fg_NonTracked_ReallocDebug;
		_pModule->m_fNonTracked_ResizeDebug = &NCrossModuleServer::fg_NonTracked_ResizeDebug;
		
		if (_Version >= 0x102)
			_pModule->m_fReportingLeaks = NMem::g_CrossModule.m_fReportingLeaks;
	}
	
	void CMemoryManagerCrossModuleInterfaceServer::f_Unregister(NMem::CMemoryManagerCrossModule *_pModule)
	{
	}	
}
