// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>

namespace NMib
{
	namespace NMem
	{
		enum
		{
			EMemoryManagerCrossModule_Version = 0x103
		};
		
		struct CMemoryManagerCrossModule;

		struct CCrossModuleImplementationDefaults
		{
			static constexpr bool mc_bSizePenalty = true;

			static void fs_MemoryManager_GarbageCollect(CMemoryManagerCrossModule *_pModule);
			static void fs_CreateMemoryManager(CMemoryManagerCrossModule *_pModule);
			static CMemoryManagerCheckout fs_MemoryManager_Checkout(CMemoryManagerCrossModule *_pModule);
			static void fs_DestroyMemoryManager(CMemoryManagerCrossModule *_pModule);
			static bool fs_MemoryManager_Check(CMemoryManagerCrossModule *_pModule, bool _bBreak);
			static void fs_MemoryManager_PrepareFork(CMemoryManagerCrossModule *_pModule);
			static void fs_MemoryManager_ForkedParent(CMemoryManagerCrossModule *_pModule);
			static void fs_MemoryManager_ForkedChild(CMemoryManagerCrossModule *_pModule);
			static void fs_MemoryManager_DestroyThreads(CMemoryManagerCrossModule *_pModule);
			static void fs_MemoryManager_CanStrartThreads(CMemoryManagerCrossModule *_pModule);
			static void fs_MemoryManager_SetNumaNode(CMemoryManagerCrossModule *_pModule, ENumaNode _NumaNode);
			static void fs_MemoryManager_OnThreadCreated(CMemoryManagerCrossModule *_pModule, mint _ThreadID, mint _ParentID);
			static void fs_DemandProtection(CMemoryManagerCrossModule *_pModule);

			static void fs_AllocBatchInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);
			static void fs_AllocBatch(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext);
			static void fs_AllocBatchDebugInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			static void fs_AllocBatchDebug(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);

			static void *fs_AllocWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			static void *fs_AllocAlignedWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			static void *fs_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags);
			static void *fs_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags);
			
			static mint fs_NonTracked_Granularity(CMemoryManagerCrossModule *_pModule);
			static mint fs_NonTracked_Size(CMemoryManagerCrossModule *_pModule, void *_pBlock);
			static mint fs_NonTracked_TrySize(CMemoryManagerCrossModule *_pModule, void *_pBlock);
			static mint fs_NonTracked_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size);
			static fp32 fs_NonTracked_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pBlock);

			static void *fs_NonTracked_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size);
			static void *fs_NonTracked_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment);

			static void *fs_NonTracked_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags);
			static void *fs_NonTracked_Resize(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags);
			static void fs_NonTracked_Free(CMemoryManagerCrossModule *_pModule, void *_pBlock, mint _Size);
			static void fs_NonTracked_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pBlock);

			static void fs_NonTracked_AllocBatchInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor);
			static void fs_NonTracked_AllocBatch(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext);
			static void fs_NonTracked_AllocBatchDebugInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			static void fs_NonTracked_AllocBatchDebug(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);


			static void *fs_NonTracked_AllocWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			static void *fs_NonTracked_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags);
			static void *fs_NonTracked_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags);
			static void *fs_NonTracked_AllocAlignedWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);

			static bool fs_ReportingLeaks(CMemoryManagerCrossModule *_pModule);

			static void *fs_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size);
			static void *fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint _Size);
			static void *fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Align);

			static void *fs_NonTracked_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size);
			static void *fs_NonTracked_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment);
		};
		
		struct CCrossModuleImplementation : public CCrossModuleImplementationDefaults
		{
			static void fs_CreateNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule);
			static void fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule);
			
			static void *fs_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size);
			static void *fs_AllocInitZeroWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size);
			static void *fs_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align);

			static void *fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags);
			static void *fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags);
			static void fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size);
			static void fs_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory);
			static mint fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory);
			static mint fs_TrySize(CMemoryManagerCrossModule *_pModule, const void *_pMemory);
			static mint fs_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size);
			static fp32 fs_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory);
			static mint fs_Granularity(CMemoryManagerCrossModule *_pModule);
		};

		struct CMemoryManagerCrossModule
		{
			uint32 m_Version;
			mint m_Reserved[16]; // Space reserved for future use
			
			void (* m_fCreateNonTrackedMemoryManager)(CMemoryManagerCrossModule *_pModule);
			void (* m_fDestroyNonTrackedMemoryManager)(CMemoryManagerCrossModule *_pModule);
			void (* m_fMemoryManager_GarbageCollect)(CMemoryManagerCrossModule *_pModule);
			void (* m_fCreateMemoryManager)(CMemoryManagerCrossModule *_pModule);
			CMemoryManagerCheckout (* m_fMemoryManager_Checkout)(CMemoryManagerCrossModule *_pModule);
			void (* m_fDestroyMemoryManager)(CMemoryManagerCrossModule *_pModule);
			bool (* m_fMemoryManager_Check)(CMemoryManagerCrossModule *_pModule, bool _bBreak);
			void (* m_fMemoryManager_PrepareFork)(CMemoryManagerCrossModule *_pModule);
			void (* m_fMemoryManager_ForkedParent)(CMemoryManagerCrossModule *_pModule);
			void (* m_fMemoryManager_ForkedChild)(CMemoryManagerCrossModule *_pModule);
			void (* m_fMemoryManager_DestroyThreads)(CMemoryManagerCrossModule *_pModule);
			void (* m_fMemoryManager_CanStartThreads)(CMemoryManagerCrossModule *_pModule);
			void (* m_fMemoryManager_SetNumaNode)(CMemoryManagerCrossModule *_pModule, ENumaNode _NumaNode);
			void (* m_fMemoryManager_OnThreadCreated)(CMemoryManagerCrossModule *_pModule, mint _ThreadID, mint _ParentID);
			void (* m_fDemandProtection)(CMemoryManagerCrossModule *_pModule);
			
			void * (* m_fAllocWithSize)(CMemoryManagerCrossModule *_pModule, mint &_Size);
			void * (* m_fAllocInitZeroWithSize)(CMemoryManagerCrossModule *_pModule, mint &_Size);
			void * (* m_fAllocAlignedWithSize)(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align);
			void (* m_fAllocBatch)(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Align, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext);
			void (* m_fAllocBatchDebug)(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Align, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			void * (* m_fAllocWithSizeDebug)(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			void * (* m_fAllocAlignedWithSizeDebug)(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			void * (* m_fReallocNoOldDebug)(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			void * (* m_fResizeNoOldDebug)(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			void * (* m_fReallocNoOld)(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size);
			void * (* m_fResizeNoOld)(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size);
			void (* m_fFreeNoSize)(CMemoryManagerCrossModule *_pModule, void *_pMemory);
			mint (* m_fSize)(CMemoryManagerCrossModule *_pModule, const void *_pMemory);
			mint (* m_fTrySize)(CMemoryManagerCrossModule *_pModule, const void *_pMemory);
			mint (* m_fSizePadded)(CMemoryManagerCrossModule *_pModule, mint _Size);
			fp32 (* m_fOverhead)(CMemoryManagerCrossModule *_pModule, void const *_pMemory);
			mint (* m_fGranularity)(CMemoryManagerCrossModule *_pModule);
			
			mint (* m_fNonTracked_Granularity)(CMemoryManagerCrossModule *_pModule);
			mint (* m_fNonTracked_Size)(CMemoryManagerCrossModule *_pModule, void *_pBlock);
			mint (* m_fNonTracked_TrySize)(CMemoryManagerCrossModule *_pModule, void *_pBlock);
			mint (* m_fNonTracked_SizePadded)(CMemoryManagerCrossModule *_pModule, mint _Size);
			fp32 (* m_fNonTracked_Overhead)(CMemoryManagerCrossModule *_pModule, void const *_pBlock);
			void * (* m_fNonTracked_AllocWithSize)(CMemoryManagerCrossModule *_pModule, mint &_Size);
			void * (* m_fNonTracked_AllocAlignedWithSize)(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment);
			void (* m_fNonTracked_AllocBatch)(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Align, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext);
			void (* m_fNonTracked_AllocBatchDebug)(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Align, bool (* _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			void * (* m_fNonTracked_ReallocNoOld)(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size);
			void * (* m_fNonTracked_ResizeNoOld)(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size);
			void (* m_fNonTracked_FreeNoSize)(CMemoryManagerCrossModule *_pModule, void *_pBlock);
			void * (* m_fNonTracked_AllocWithSizeDebug)(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			void * (* m_fNonTracked_AllocAlignedWithSizeDebug)(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			void * (* m_fNonTracked_ReallocNoOldDebug)(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			void * (* m_fNonTracked_ResizeNoOldDebug)(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags);
			
			bool (* m_fReportingLeaks)(CMemoryManagerCrossModule *_pModule);

			void * (* m_fAlloc)(CMemoryManagerCrossModule *_pModule, mint _Size);
			void * (* m_fAllocInitZero)(CMemoryManagerCrossModule *_pModule, mint _Size);
			void * (* m_fAllocAligned)(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Align);

			void * (* m_fNonTracked_Alloc)(CMemoryManagerCrossModule *_pModule, mint _Size);
			void * (* m_fNonTracked_AllocAligned)(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment);

			void * (* m_fReallocDebug)(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags);
			void * (* m_fResizeDebug)(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags);
			void * (* m_fRealloc)(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags);
			void * (* m_fResize)(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags);

			void * (* m_fNonTracked_Realloc)(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags);
			void * (* m_fNonTracked_Resize)(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags);
			void * (* m_fNonTracked_ReallocDebug)(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags);
			void * (* m_fNonTracked_ResizeDebug)(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags);

			void (* m_fFree)(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size);
			void (* m_fNonTracked_Free)(CMemoryManagerCrossModule *_pModule, void *_pBlock, mint _Size);
		};
		
		struct ICMemoryManagerCrossModule
		{
			virtual void f_Register(CMemoryManagerCrossModule *_pModule, uint32 _Version) = 0;
			virtual void f_Unregister(CMemoryManagerCrossModule *_pModule) = 0;
		};
	}
}
