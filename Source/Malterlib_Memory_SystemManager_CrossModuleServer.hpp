// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>
#include "Malterlib_Memory_SystemManager_CrossModule.h"

namespace NMib
{
	struct CMemoryManagerCrossModuleInterfaceServer : NMemory::ICMemoryManagerCrossModule
	{
		void f_Register(NMemory::CMemoryManagerCrossModule *_pModule, uint32 _Version) override;
		void f_Unregister(NMemory::CMemoryManagerCrossModule *_pModule) override;
	};

	constinit NStorage::TCAggregateSimple<CMemoryManagerCrossModuleInterfaceServer> g_CrossModuleInterfaceServer = {DAggregateInit};

	void CSystem::fp_CreateNonTrackedMemoryManager()
	{
#if DMibConfig_MemoryManager_CrossModule_Enable
#if !defined(DMibMemoryOverrideDll)
		DMibFastCheck(!fg_GetSys()->f_IsDll());
#endif
#endif
		NMemory::CCrossModuleImplementationExtra::fs_CreateNonTrackedMemoryManager(&NMemory::g_CrossModule);
	}

	void CSystem::fp_DestroyNonTrackedMemoryManager()
	{
		NMemory::CCrossModuleImplementationExtra::fs_DestroyNonTrackedMemoryManager(&NMemory::g_CrossModule);
	}

	void CSystem::f_MemoryManager_GarbageCollect()
	{
		NMemory::CCrossModuleImplementationExtra::fs_MemoryManager_GarbageCollect(&NMemory::g_CrossModule);
	}

	void CSystem::fp_CreateMemoryManager()
	{
		NMemory::CCrossModuleImplementationExtra::fs_CreateMemoryManager(&NMemory::g_CrossModule);

#if !defined(DMibDynamicLibrary) || defined(DMibMemoryOverrideDll)
		g_CrossModuleInterfaceServer.f_Construct();
		NSys::fg_Process_SetCrossModuleMemoryManagerInterface(&*g_CrossModuleInterfaceServer);
#endif
	}
	NMemory::CMemoryManagerCheckout CSystem::f_MemoryManager_Checkout()
	{
		return NMemory::CCrossModuleImplementationExtra::fs_MemoryManager_Checkout(&NMemory::g_CrossModule);
	}

	void CSystem::fp_DestroyMemoryManager()
	{
		NSys::fg_PreDestroyHeap();
		return NMemory::CCrossModuleImplementationExtra::fs_DestroyMemoryManager(&NMemory::g_CrossModule);
	}

	bool CSystem::f_MemoryManager_Check(bool _bBreak)
	{
		return NMemory::CCrossModuleImplementationExtra::fs_MemoryManager_Check(&NMemory::g_CrossModule, _bBreak);
	}

	bool CSystem::f_MemoryManager_ReportingLeaks()
	{
		return NMemory::CCrossModuleImplementationExtra::fs_ReportingLeaks(&NMemory::g_CrossModule);
	}

	void CSystem::f_MemoryManager_PrepareFork()
	{
		NSys::fg_Mem_PrepareFork();
		NMemory::CCrossModuleImplementationExtra::fs_MemoryManager_PrepareFork(&NMemory::g_CrossModule);
	}

	void CSystem::f_MemoryManager_ForkedParent()
	{
		NMemory::CCrossModuleImplementationExtra::fs_MemoryManager_ForkedParent(&NMemory::g_CrossModule);
		NSys::fg_Mem_ForkedParent();
	}

	void CSystem::f_MemoryManager_ForkedChild()
	{
		NMemory::CCrossModuleImplementationExtra::fs_MemoryManager_ForkedChild(&NMemory::g_CrossModule);
		NSys::fg_Mem_ForkedChild();
	}

	void CSystem::f_MemoryManager_DestroyThreads()
	{
		return NMemory::CCrossModuleImplementationExtra::fs_MemoryManager_DestroyThreads(&NMemory::g_CrossModule);
	}

	void CSystem::f_MemoryManager_CanStartThreads()
	{
		return NMemory::CCrossModuleImplementationExtra::fs_MemoryManager_CanStrartThreads(&NMemory::g_CrossModule);
	}



	void CSystem::f_MemoryManager_SetNumaNode(ENumaNode _NumaNode)
	{
		return NMemory::CCrossModuleImplementationExtra::fs_MemoryManager_SetNumaNode(&NMemory::g_CrossModule, _NumaNode);
	}

	void CSystem::f_MemoryManager_OnThreadCreated(mint _ThreadID, mint _ParentID)
	{
		return NMemory::CCrossModuleImplementationExtra::fs_MemoryManager_OnThreadCreated(&NMemory::g_CrossModule, _ThreadID, _ParentID);
	}
}

namespace NMib::NMemory
{
	DMibMemory_MemoryManagerExport void * fg_AllocWithSize(mint &_Size)
	{
		return CCrossModuleImplementationExtra::fs_AllocWithSize(&g_CrossModule, _Size);
	}

	inline_always_lto DMibMemory_MemoryManagerExport void * fg_Alloc(mint _Size)
	{
		return CCrossModuleImplementationExtra::fs_Alloc(&g_CrossModule, _Size);
	}

	DMibMemory_MemoryManagerExport void * fg_AllocInitZeroWithSize(mint &_Size)
	{
		return CCrossModuleImplementationExtra::fs_AllocInitZeroWithSize(&g_CrossModule, _Size);
	}

	DMibMemory_MemoryManagerExport void * fg_AllocInitZero(mint _Size)
	{
		return CCrossModuleImplementationExtra::fs_AllocInitZero(&g_CrossModule, _Size);
	}

	DMibMemory_MemoryManagerExport void * fg_AllocAlignedWithSize(mint &_Size, mint _Alignment)
	{
		return CCrossModuleImplementationExtra::fs_AllocAlignedWithSize(&g_CrossModule, _Size, _Alignment);
	}

	inline_always_lto DMibMemory_MemoryManagerExport void * fg_AllocAligned(mint _Size, mint _Alignment)
	{
#if defined(DPlatformFamily_Linux) && !defined(DMibInitInPreInitArray)
		if (!NMib::g_bCreatedSystem)
			NMib::NSys::fg_CreateSystem();
#endif
		return CCrossModuleImplementationExtra::fs_AllocAligned(&g_CrossModule, _Size, _Alignment);
	}

	DMibMemory_MemoryManagerExport void fg_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
	{
		return CCrossModuleImplementationExtra::fs_AllocBatchInternal(&g_CrossModule, _Size, _Alignment, _Functor);
	}

	bool fg_AllocHasDeterministicSize()
	{
		return CCrossModuleImplementationExtra::fs_AllocHasDeterministicSize(&g_CrossModule);
	}

	EMemoryManagerFeatureFlag fg_MemoryManagerFeatures()
	{
		return CCrossModuleImplementationExtra::fs_MemoryManagerFeatures(&g_CrossModule);
	}

#	if DMibConfig_MalterlibMemoryManager_Debug
		DMibMemory_MemoryManagerExport void * fg_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return CCrossModuleImplementationExtra::fs_AllocWithSizeDebug(&g_CrossModule, _Size, _pFile, _Line, _Flags);
		}
		DMibMemory_MemoryManagerExport void * fg_AllocDebug(mint _Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return CCrossModuleImplementationExtra::fs_AllocWithSizeDebug(&g_CrossModule, _Size, _pFile, _Line, _Flags);
		}
		DMibMemory_MemoryManagerExport void * fg_AllocAlignedWithSizeDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return CCrossModuleImplementationExtra::fs_AllocAlignedWithSizeDebug(&g_CrossModule, _Size, _Alignment, _pFile, _Line, _Flags);
		}
		DMibMemory_MemoryManagerExport void * fg_AllocAlignedDebug(mint _Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return CCrossModuleImplementationExtra::fs_AllocAlignedWithSizeDebug(&g_CrossModule, _Size, _Alignment, _pFile, _Line, _Flags);
		}
		DMibMemory_MemoryManagerExport void * fg_ReallocDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
		{
			return CCrossModuleImplementationExtra::fs_ReallocDebug(&g_CrossModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
		}
		DMibMemory_MemoryManagerExport void * fg_ResizeDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
		{
			return CCrossModuleImplementationExtra::fs_ResizeDebug(&g_CrossModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
		}
		DMibMemory_MemoryManagerExport void fg_AllocBatchDebug(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
		{
			return CCrossModuleImplementationExtra::fs_AllocBatchDebugInternal(&g_CrossModule, _Size, _Alignment, _Functor, _pFile, _Line, _Flags);
		}
#	endif

	DMibMemory_MemoryManagerExport void * fg_Realloc(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		return CCrossModuleImplementationExtra::fs_Realloc(&g_CrossModule, _pMemory, _Size, _OldSize, _AllocFlags);
	}

	DMibMemory_MemoryManagerExport void * fg_Resize(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		return CCrossModuleImplementationExtra::fs_Resize(&g_CrossModule, _pMemory, _Size, _OldSize, _AllocFlags);
	}

	inline_always_lto DMibMemory_MemoryManagerExport void fg_Free(void *_pMemory, mint _Size)
	{
		return CCrossModuleImplementationExtra::fs_Free(&g_CrossModule, _pMemory, _Size);
	}

	inline_always_lto DMibMemory_MemoryManagerExport void fg_FreeNoSize(void *_pMemory)
	{
		return CCrossModuleImplementationExtra::fs_FreeNoSize(&g_CrossModule, _pMemory);
	}

	inline_always_lto DMibMemory_MemoryManagerExport mint fg_Size(const void *_pMemory)
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

namespace NMib::NMemory
{
	mint CAllocator_NonTrackedHeap::f_GranularityAlloc(bool _bLargePages)
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

	only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocWithSizeDebug(mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_AllocWithSizeDebug(&g_CrossModule, _Size, _pFile, _Line, _Flags);
	}

	only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_ReallocDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_ReallocDebug(&g_CrossModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
	}

	only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_ResizeDebug(void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_ResizeDebug(&g_CrossModule,_pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
	}

	only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocAlignedWithSizeDebug(mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedWithSizeDebug(&g_CrossModule, _Size, _Alignment, _pFile, _Line, _Flags);
	}

	only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocWithSize(mint &_Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_AllocWithSize(&g_CrossModule, _Size);
	}

	only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocAlignedWithSize(mint &_Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedWithSize(&g_CrossModule, _Size, _Alignment);
	}

	only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_Alloc(mint _Size, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_Alloc(&g_CrossModule, _Size);
	}

	only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_AllocAligned(mint _Size, mint _Alignment, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
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

	only_parameters_aliased malloc_like void *CAllocator_NonTrackedHeap::f_Realloc(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_Realloc(&g_CrossModule, _pMemory, _Size, _OldSize, _AllocFlags);
	}

	only_parameters_aliased void *CAllocator_NonTrackedHeap::f_Resize(void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags, ENumaNode _NumaNode)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_Resize(&g_CrossModule, _pMemory, _Size, _OldSize, _AllocFlags);
	}

	only_parameters_aliased void CAllocator_NonTrackedHeap::f_Free(void *_pMemory, mint _Size)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_Free(&g_CrossModule, _pMemory, _Size);
	}

	only_parameters_aliased void CAllocator_NonTrackedHeap::f_FreeNoSize(void *_pMemory)
	{
		return CCrossModuleImplementationExtra::fs_NonTracked_FreeNoSize(&g_CrossModule, _pMemory);
	}
}

namespace NMib::NMemory::NCrossModuleServer
{
	void DMibCrossmoduleAPI fg_VoidDummy(NMemory::CMemoryManagerCrossModule *_pModule)
	{
	}
	void * DMibCrossmoduleAPI fg_AllocWithSizeDebug(NMemory::CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_AllocWithSizeDebug(_pModule, _Size, _pFile, _Line, _Flags);
	}
	void * DMibCrossmoduleAPI fg_AllocAlignedWithSizeDebug(NMemory::CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_AllocAlignedWithSizeDebug(_pModule, _Size, _Alignment, _pFile, _Line, _Flags);
	}
	void * DMibCrossmoduleAPI fg_ReallocNoOldDebug(NMemory::CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_ReallocDebug(_pModule, _pMemory, _Size, 0, _pFile, _Line, _Flags, EAllocationFlag_None);
	}
	void * DMibCrossmoduleAPI fg_ReallocDebug
		(
			NMemory::CMemoryManagerCrossModule *_pModule
			, void *_pMemory
			, mint &_Size
			, mint _OldSize
			, const ch8 *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
		)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_ReallocDebug(_pModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
	}
	void * DMibCrossmoduleAPI fg_ResizeNoOldDebug(NMemory::CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_ResizeDebug(_pModule, _pMemory, _Size, 0, _pFile, _Line, _Flags, EAllocationFlag_None);
	}
	void * DMibCrossmoduleAPI fg_ResizeDebug
		(
			NMemory::CMemoryManagerCrossModule *_pModule
			, void *_pMemory
			, mint &_Size
			, mint _OldSize
			, const ch8 *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
		)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_ResizeDebug(_pModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
	}
	void * DMibCrossmoduleAPI fg_NonTracked_AllocDebug(NMemory::CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_NonTracked_AllocWithSizeDebug(_pModule, _Size, _pFile, _Line, _Flags);
	}
	void * DMibCrossmoduleAPI fg_NonTracked_ReallocNoOldDebug(NMemory::CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_NonTracked_ReallocDebug(_pModule, _pMemory, _Size, 0, _pFile, _Line, _Flags, EAllocationFlag_None);
	}
	void * DMibCrossmoduleAPI fg_NonTracked_ReallocDebug
		(
			NMemory::CMemoryManagerCrossModule *_pModule
			, void *_pMemory
			, mint &_Size
			, mint _OldSize
			, const ch8 *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
		)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_NonTracked_ReallocDebug(_pModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
	}
	void * DMibCrossmoduleAPI fg_NonTracked_ResizeNoOldDebug(NMemory::CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_NonTracked_ResizeDebug(_pModule, _pMemory, _Size, 0, _pFile, _Line, _Flags, EAllocationFlag_None);
	}
	void * DMibCrossmoduleAPI fg_NonTracked_ResizeDebug
		(
			NMemory::CMemoryManagerCrossModule *_pModule
			, void *_pMemory
			, mint &_Size
			, mint _OldSize
			, const ch8 *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
			, EAllocationFlag _AllocFlags
		)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_NonTracked_ResizeDebug(_pModule, _pMemory, _Size, _OldSize, _pFile, _Line, _Flags, _AllocFlags);
	}
	void * DMibCrossmoduleAPI fg_NonTracked_AllocAlignedDebug
		(
			NMemory::CMemoryManagerCrossModule *_pModule
			, mint &_Size
			, mint _Alignment
			, const ch8 *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
		)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_NonTracked_AllocAlignedWithSizeDebug(_pModule, _Size, _Alignment, _pFile, _Line, _Flags);
	}

	void DMibCrossmoduleAPI fg_AllocBatchDebug
		(
			NMemory::CMemoryManagerCrossModule *_pModule
			, mint _Size
			, mint _Alignment
			, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size)
			, void * _pContext
			, const ch8 *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
		)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_AllocBatchDebug(_pModule, _Size, _Alignment, _fCallBatchFunctor, _pContext, _pFile, _Line, _Flags);
	}

	void DMibCrossmoduleAPI fg_NonTracked_AllocBatchDebug
		(
			NMemory::CMemoryManagerCrossModule *_pModule
			, mint _Size
			, mint _Alignment
			, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size)
			, void * _pContext
			, const ch8 *_pFile
			, aint _Line
			, EHeapDebugFlag _Flags
		)
	{
		_pFile = nullptr; // We need to zero this out as it will be invalid after dll in unloaded
		return NMemory::CCrossModuleImplementationExtra::fs_NonTracked_AllocBatchDebug(_pModule, _Size, _Alignment, _fCallBatchFunctor, _pContext, _pFile, _Line, _Flags);
	}

	void DMibCrossmoduleAPI fg_MemoryManager_OnThreadCreated(NMemory::CMemoryManagerCrossModule *_pModule, mint _ThreadID, mint _ParentID)
	{
		fg_GetSys()->f_ThreadLocalCreateThread(_ThreadID, _ParentID);
	}
}

namespace NMib
{
	void CMemoryManagerCrossModuleInterfaceServer::f_Register(NMemory::CMemoryManagerCrossModule *_pModule, uint32 _Version)
	{
		using namespace NMemory;
		_pModule->m_Version = NMemory::g_CrossModule.m_Version;
		_pModule->m_fCreateNonTrackedMemoryManager = &NCrossModuleServer::fg_VoidDummy;
		_pModule->m_fDestroyNonTrackedMemoryManager = &NCrossModuleServer::fg_VoidDummy;
		_pModule->m_fMemoryManager_GarbageCollect = NMemory::g_CrossModule.m_fMemoryManager_GarbageCollect;
		_pModule->m_fCreateMemoryManager = &NCrossModuleServer::fg_VoidDummy;
		_pModule->m_fMemoryManager_Checkout = NMemory::g_CrossModule.m_fMemoryManager_Checkout;
		_pModule->m_fDestroyMemoryManager = &NCrossModuleServer::fg_VoidDummy;
		_pModule->m_fMemoryManager_Check = NMemory::g_CrossModule.m_fMemoryManager_Check;
		_pModule->m_fMemoryManager_PrepareFork = NMemory::g_CrossModule.m_fMemoryManager_PrepareFork;
		_pModule->m_fMemoryManager_ForkedParent = NMemory::g_CrossModule.m_fMemoryManager_ForkedParent;
		_pModule->m_fMemoryManager_ForkedChild = NMemory::g_CrossModule.m_fMemoryManager_ForkedChild;
		_pModule->m_fMemoryManager_DestroyThreads = &NCrossModuleServer::fg_VoidDummy;
		_pModule->m_fMemoryManager_CanStartThreads = &NCrossModuleServer::fg_VoidDummy;
		_pModule->m_fMemoryManager_SetNumaNode = NMemory::g_CrossModule.m_fMemoryManager_SetNumaNode;
		_pModule->m_fMemoryManager_OnThreadCreated = &NCrossModuleServer::fg_MemoryManager_OnThreadCreated;
		_pModule->m_fDemandProtection = NMemory::g_CrossModule.m_fDemandProtection;

		_pModule->m_fAllocWithSize = NMemory::g_CrossModule.m_fAllocWithSize;
		_pModule->m_fAllocInitZeroWithSize = NMemory::g_CrossModule.m_fAllocInitZeroWithSize;
		_pModule->m_fAllocAlignedWithSize = NMemory::g_CrossModule.m_fAllocAlignedWithSize;
		_pModule->m_fAllocBatch = NMemory::g_CrossModule.m_fAllocBatch;
		_pModule->m_fAllocBatchDebug = &NCrossModuleServer::fg_AllocBatchDebug;
		_pModule->m_fAllocWithSizeDebug = &NCrossModuleServer::fg_AllocWithSizeDebug;
		_pModule->m_fAllocAlignedWithSizeDebug = &NCrossModuleServer::fg_AllocAlignedWithSizeDebug;
		_pModule->m_fReallocNoOldDebug = &NCrossModuleServer::fg_ReallocNoOldDebug;
		_pModule->m_fResizeNoOldDebug = &NCrossModuleServer::fg_ResizeNoOldDebug;
		_pModule->m_fReallocNoOld = NMemory::g_CrossModule.m_fReallocNoOld;
		_pModule->m_fResizeNoOld = NMemory::g_CrossModule.m_fResizeNoOld;
		_pModule->m_fFreeNoSize = NMemory::g_CrossModule.m_fFreeNoSize;
		_pModule->m_fSize = NMemory::g_CrossModule.m_fSize;
		_pModule->m_fTrySize = NMemory::g_CrossModule.m_fTrySize;
		_pModule->m_fSizePadded = NMemory::g_CrossModule.m_fSizePadded;
		_pModule->m_fOverhead = NMemory::g_CrossModule.m_fOverhead;
		_pModule->m_fGranularity = NMemory::g_CrossModule.m_fGranularity;

		_pModule->m_fNonTracked_Granularity = NMemory::g_CrossModule.m_fNonTracked_Granularity;
		_pModule->m_fNonTracked_Size = NMemory::g_CrossModule.m_fNonTracked_Size;
		_pModule->m_fNonTracked_TrySize = NMemory::g_CrossModule.m_fNonTracked_TrySize;
		_pModule->m_fNonTracked_SizePadded = NMemory::g_CrossModule.m_fNonTracked_SizePadded;
		_pModule->m_fNonTracked_Overhead = NMemory::g_CrossModule.m_fNonTracked_Overhead;
		_pModule->m_fNonTracked_AllocWithSize = NMemory::g_CrossModule.m_fNonTracked_AllocWithSize;
		_pModule->m_fNonTracked_AllocAlignedWithSize = NMemory::g_CrossModule.m_fNonTracked_AllocAlignedWithSize;
		_pModule->m_fNonTracked_AllocBatch = NMemory::g_CrossModule.m_fNonTracked_AllocBatch;
		_pModule->m_fNonTracked_AllocBatchDebug = &NCrossModuleServer::fg_NonTracked_AllocBatchDebug;
		_pModule->m_fNonTracked_ReallocNoOld = NMemory::g_CrossModule.m_fNonTracked_ReallocNoOld;
		_pModule->m_fNonTracked_ResizeNoOld = NMemory::g_CrossModule.m_fNonTracked_ResizeNoOld;
		_pModule->m_fNonTracked_FreeNoSize = NMemory::g_CrossModule.m_fNonTracked_FreeNoSize;
		_pModule->m_fNonTracked_AllocWithSizeDebug = &NCrossModuleServer::fg_NonTracked_AllocDebug;
		_pModule->m_fNonTracked_AllocAlignedWithSizeDebug = &NCrossModuleServer::fg_NonTracked_AllocAlignedDebug;
		_pModule->m_fNonTracked_ReallocNoOldDebug = &NCrossModuleServer::fg_NonTracked_ReallocNoOldDebug;
		_pModule->m_fNonTracked_ResizeNoOldDebug = &NCrossModuleServer::fg_NonTracked_ResizeNoOldDebug;

		if (_Version >= 0x102)
			_pModule->m_fReportingLeaks = NMemory::g_CrossModule.m_fReportingLeaks;

		if (_Version >= 0x103)
		{
			_pModule->m_fAlloc = NMemory::g_CrossModule.m_fAlloc;
			_pModule->m_fAllocInitZero = NMemory::g_CrossModule.m_fAllocInitZero;
			_pModule->m_fAllocAligned = NMemory::g_CrossModule.m_fAllocAligned;

			_pModule->m_fNonTracked_Alloc = NMemory::g_CrossModule.m_fNonTracked_Alloc;
			_pModule->m_fNonTracked_AllocAligned = NMemory::g_CrossModule.m_fNonTracked_AllocAligned;

			_pModule->m_fReallocDebug = &NCrossModuleServer::fg_ReallocDebug;
			_pModule->m_fResizeDebug = &NCrossModuleServer::fg_ResizeDebug;
			_pModule->m_fRealloc = NMemory::g_CrossModule.m_fRealloc;
			_pModule->m_fResize = NMemory::g_CrossModule.m_fResize;

			_pModule->m_fNonTracked_Realloc = NMemory::g_CrossModule.m_fNonTracked_Realloc;
			_pModule->m_fNonTracked_Resize = NMemory::g_CrossModule.m_fNonTracked_Resize;
			_pModule->m_fNonTracked_ReallocDebug = &NCrossModuleServer::fg_NonTracked_ReallocDebug;
			_pModule->m_fNonTracked_ResizeDebug = &NCrossModuleServer::fg_NonTracked_ResizeDebug;

			_pModule->m_fFree = NMemory::g_CrossModule.m_fFree;
			_pModule->m_fNonTracked_Free = NMemory::g_CrossModule.m_fNonTracked_Free;
		}

		if (_Version >= 0x104)
			_pModule->m_fAllocHasDeterministicSize = NMemory::g_CrossModule.m_fAllocHasDeterministicSize;

		if (_Version >= 0x105)
			_pModule->m_fMemoryManagerFeatures = NMemory::g_CrossModule.m_fMemoryManagerFeatures;
	}

	void CMemoryManagerCrossModuleInterfaceServer::f_Unregister(NMemory::CMemoryManagerCrossModule *_pModule)
	{
	}
}
