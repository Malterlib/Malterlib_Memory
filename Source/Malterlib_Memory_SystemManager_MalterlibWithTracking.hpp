// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include "Malterlib_Memory_SystemManager_MalterlibWithTracking.h"

namespace NMib
{
	NMib::NStorage::TCAggregateSimple<CMemoryManager> g_MainHeap = {DAggregateInit};
	bool g_bMainHeapConstructed = false;

	NMib::NStorage::TCAggregateSimple<CMemoryManagerNonTracked> g_NonTrackedHeap = {DAggregateInit};
	bool g_bNonTrackedHeapConstructed = false;

#if DMibEnableSafeCheck > 0
	static auto &fg_NonTrackedHeap()
	{
		DMibFastCheck(g_bNonTrackedHeapConstructed);
		return g_NonTrackedHeap;
	}
	static auto &fg_MainHeap()
	{
		DMibFastCheck(g_bMainHeapConstructed);
		return g_MainHeap;
	}
	#define DNonTrackedHeap fg_NonTrackedHeap()
	#define DMainHeap fg_MainHeap()
#else
	#define DNonTrackedHeap g_NonTrackedHeap
	#define DMainHeap g_MainHeap
#endif

	NMib::NThread::CMutualAggregate g_MemoryManagerForkLock = {DAggregateInit};
	mint g_MemoryManagerForkedCount = 0;
	bool g_MemoryManagerUnforked = false;
}

namespace NMib::NMemory
{
#if DMibConfig_Memory_Shims_Lightweight
	CReportMemoryLightweight *fg_ReportMemoryLightweightTo(CReportMemoryLightweight *_pMemoryReporter)
	{
		if (!g_bMainHeapConstructed)
			return nullptr;
		DNonTrackedHeap->f_ReportMemoryTo(_pMemoryReporter);
		return DMainHeap->f_ReportMemoryTo(_pMemoryReporter);
	}

	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeGetFlags()
	{
		if (!g_bMainHeapConstructed)
			return EMemoryReportLightweightScopeFlag_None;
		return DMainHeap->f_GetLightweightScopeFlags();
	}

	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeSetFlags(EMemoryReportLightweightScopeFlag _Flags)
	{
		if (!g_bMainHeapConstructed)
			return EMemoryReportLightweightScopeFlag_None;
		DNonTrackedHeap->f_SetLightweightScopeFlags(_Flags);
		return DMainHeap->f_SetLightweightScopeFlags(_Flags);
	}

	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeAddFlags(EMemoryReportLightweightScopeFlag _Flags)
	{
		if (!g_bMainHeapConstructed)
			return EMemoryReportLightweightScopeFlag_None;
		DNonTrackedHeap->f_AddLightweightScopeFlags(_Flags);
		return DMainHeap->f_AddLightweightScopeFlags(_Flags);
	}
#endif

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_CreateNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
	{
		g_NonTrackedHeap.f_Construct(CMemoryManagerConfig());
		g_bNonTrackedHeapConstructed = true;
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
	{
#		if DEnableDebugMemoryManager
			if (!CSystem::ms_bDisableMemoryManagerLeakReport)
				DNonTrackedHeap->f_ReportLeaks();
#		endif
		if (!g_bMemoryManagerNeededAfterDestroy)
		{
#			ifdef DMibConfig_HeapNeverDestroyed
				DNonTrackedHeap->f_DestroyThreadLocals(); // For debug checks later
#			else
				DNonTrackedHeap.f_Destruct();
				g_MemoryManagerForkLock.f_Destruct();
#			endif
		}
	}

	struct CCrossModuleImplementationExtra : public CCrossModuleImplementation
	{
		static constexpr bool mc_bSizePenalty = false;

		inline_always static void DMibCrossmoduleAPI fs_MemoryManager_GarbageCollect(CMemoryManagerCrossModule *_pModule)
		{
			DMainHeap->f_GarbageCollect(true);
			DNonTrackedHeap->f_GarbageCollect(true);
		}

		inline_always static void DMibCrossmoduleAPI fs_CreateMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
			g_MainHeap.f_Construct("Main memory manager", CMemoryManagerConfig());
			g_bMainHeapConstructed = true;
			{
				// Make sure the code for checking out manager is included
				auto MemoryManagerCheckout = NMib::fg_GetSys()->f_MemoryManager_Checkout();
				NSys::fg_Compiler_MakeActive(1, &MemoryManagerCheckout);
			}
		}
		inline_always static CMemoryManagerCheckout DMibCrossmoduleAPI fs_MemoryManager_Checkout(CMemoryManagerCrossModule *_pModule)
		{
			return DMainHeap->f_CheckoutVirtual();
		}

		inline_always static void DMibCrossmoduleAPI fs_DestroyMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
#		if DEnableDebugMemoryManager
			{
				if (!CSystem::ms_bDisableMemoryManagerLeakReport)
					DMainHeap->f_ReportLeaks();
			}
#		endif
			{
				if (!g_bMemoryManagerNeededAfterDestroy)
				{
#				ifdef DMibConfig_HeapNeverDestroyed
					DMainHeap->f_DestroyThreadLocals(); // For debug checks later
#				else
					DMainHeap.f_Destruct();
#				endif
				}
			}
		}

		inline_always static bool DMibCrossmoduleAPI fs_MemoryManager_Check(CMemoryManagerCrossModule *_pModule, bool _bBreak)
		{
			bool bRet = true;
#			if DEnableDebugMemoryManager
			{
				if (!DMainHeap->f_CheckAll(_bBreak))
					bRet = false;
				if (!DNonTrackedHeap->f_CheckAll(_bBreak))
					bRet = false;
			}
#			endif
			return bRet;
		}

		inline_always static bool DMibCrossmoduleAPI fs_ReportingLeaks(CMemoryManagerCrossModule *_pModule)
		{
#		if DEnableDebugMemoryManager
				return DMainHeap->f_ReportingLeaks();
#		else
				return false;
#		endif
		}
		inline_always static void DMibCrossmoduleAPI fs_MemoryManager_PrepareFork(CMemoryManagerCrossModule *_pModule)
		{
			g_MemoryManagerForkLock.f_Lock();
			if (++g_MemoryManagerForkedCount > 1)
				return;
			g_MemoryManagerForkLock.f_PrepareFork();

			g_MemoryManagerUnforked = false;

			DMainHeap->f_Lock();
			DNonTrackedHeap->f_Lock();

			DMainHeap->f_CheckoutManual();
			DNonTrackedHeap->f_CheckoutManual();

			DMainHeap->f_PrepareFork();
			DNonTrackedHeap->f_PrepareFork();
		}

		inline_always static void DMibCrossmoduleAPI fs_MemoryManager_ForkedParent(CMemoryManagerCrossModule *_pModule)
		{
			--g_MemoryManagerForkedCount;
			if (g_MemoryManagerUnforked)
			{
				g_MemoryManagerForkLock.f_Unlock();
				return;
			}
			g_MemoryManagerForkLock.f_ForkedParent();
			g_MemoryManagerUnforked = true;
			DNonTrackedHeap->f_ForkedParent();
			DMainHeap->f_ForkedParent();

			DNonTrackedHeap->f_CheckinManual();
			DMainHeap->f_CheckinManual();

			DNonTrackedHeap->f_Unlock();
			DMainHeap->f_Unlock();

			g_MemoryManagerForkLock.f_Unlock();
		}

		inline_always static void DMibCrossmoduleAPI fs_MemoryManager_ForkedChild(CMemoryManagerCrossModule *_pModule)
		{
			--g_MemoryManagerForkedCount;
			if (g_MemoryManagerUnforked)
			{
				g_MemoryManagerForkLock.f_Unlock();
				return;
			}
			g_MemoryManagerForkLock.f_ForkedChild();
			g_MemoryManagerUnforked = true;
			DNonTrackedHeap->f_ForkedChild();
			DMainHeap->f_ForkedChild();

			DNonTrackedHeap->f_CheckinManual();
			DMainHeap->f_CheckinManual();

			DNonTrackedHeap->f_Unlock();
			DMainHeap->f_Unlock();

			g_MemoryManagerForkLock.f_Unlock();
		}

		inline_always static void DMibCrossmoduleAPI fs_MemoryManager_DestroyThreads(CMemoryManagerCrossModule *_pModule)
		{
			DMainHeap->f_DestroyCleanupThreads();
			DNonTrackedHeap->f_DestroyCleanupThreads();
		}

		inline_always static void DMibCrossmoduleAPI fs_MemoryManager_CanStrartThreads(CMemoryManagerCrossModule *_pModule)
		{
			DMainHeap->f_CanStartThreads();
			DNonTrackedHeap->f_CanStartThreads();
		}

		inline_always static void DMibCrossmoduleAPI fs_MemoryManager_SetNumaNode(CMemoryManagerCrossModule *_pModule, ENumaNode _NumaNode)
		{
			DMainHeap->f_SetNumaNode(_NumaNode);
			// Don't bother with non-tracked heap as that should not be used a lot
			//DNonTrackedHeap->f_SetNumaNode(_NumaNode);
		}
		static constexpr bool mc_SupportsNonTracked = true;
#		if DEnableDebugMemoryManager
			static constexpr bool mc_SupportsDebug = true;
			inline_always static void * DMibCrossmoduleAPI fs_AllocWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return DMainHeap->f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags);
			}
			inline_always static void * DMibCrossmoduleAPI fs_AllocAlignedWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return DMainHeap->f_AllocAlignedWithSizeDebug(_Size, _Align, _pFile, _Line, _Flags);
			}
			inline_always static void * DMibCrossmoduleAPI fs_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
			{
				return DMainHeap->f_ReallocDebug(_pMemory, _Size, _OldSize, _pFile, _Line, _Flags);
			}
			inline_always static void * DMibCrossmoduleAPI fs_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
			{
				return DMainHeap->f_ResizeDebug(_pMemory, _Size, _OldSize, _pFile, _Line, _Flags);
			}
			inline_always static void DMibCrossmoduleAPI fs_AllocBatchDebugInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				DMainHeap->f_AllocBatchDebug(_Size, _Alignment, _Functor, _pFile, _Line, _Flags);
			}
			inline_always static void DMibCrossmoduleAPI fs_AllocBatchDebug(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				DMainHeap->f_AllocBatchDebug
					(
						_Size
						, _Alignment
						, [&](void * _pAlloc, mint _Size)
						{
							return _fCallBatchFunctor(_pContext, _pAlloc, _Size);
						}
						, _pFile
						, _Line
						, _Flags
					)
				;
			}
#		else
			static constexpr bool mc_SupportsDebug = false;
#		endif

		inline_always static mint DMibCrossmoduleAPI fs_NonTracked_Granularity(CMemoryManagerCrossModule *_pModule)
		{
			return 1;
		}

		inline_always static mint DMibCrossmoduleAPI fs_NonTracked_Size(CMemoryManagerCrossModule *_pModule, void *_pBlock)
		{
			return DNonTrackedHeap->f_Size(_pBlock);
		}

		inline_always static mint DMibCrossmoduleAPI fs_NonTracked_TrySize(CMemoryManagerCrossModule *_pModule, void *_pBlock)
		{
			return DNonTrackedHeap->f_TrySize(_pBlock);
		}

		inline_always static mint DMibCrossmoduleAPI fs_NonTracked_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return DNonTrackedHeap->f_SizePadded(_Size);
		}

		inline_always static fp32 DMibCrossmoduleAPI fs_NonTracked_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pBlock) // Number of bytes overhead for block
		{
			return DNonTrackedHeap->f_Overhead(_pBlock);
		}

		inline_always static void DMibCrossmoduleAPI fs_AllocBatchInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			DMainHeap->f_AllocBatch(_Size, _Alignment, _Functor);
		}
		inline_always static void DMibCrossmoduleAPI fs_AllocBatch(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext)
		{
			DMainHeap->f_AllocBatch
				(
					_Size
					, _Alignment
					, [&](void * _pAlloc, mint _Size)
					{
						return _fCallBatchFunctor(_pContext, _pAlloc, _Size);
					}
				)
			;
		}
		inline_always static void DMibCrossmoduleAPI fs_NonTracked_AllocBatchInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			DMainHeap->f_AllocBatch(_Size, _Alignment, _Functor);
		}
		inline_always static void DMibCrossmoduleAPI fs_NonTracked_AllocBatch(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext)
		{
			DMainHeap->f_AllocBatch
				(
					_Size
					, _Alignment
					, [&](void * _pAlloc, mint _Size)
					{
						return _fCallBatchFunctor(_pContext, _pAlloc, _Size);
					}
				)
			;
		}

#		if DEnableDebugMemoryManager
			inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return DNonTrackedHeap->f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags);
			}
			inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocAlignedWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return DNonTrackedHeap->f_AllocAlignedWithSizeDebug(_Size, _Align, _pFile, _Line, _Flags);
			}
			inline_always static void * DMibCrossmoduleAPI fs_NonTracked_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
			{
				return DNonTrackedHeap->f_ReallocDebug(_pMemory, _Size, _OldSize, _pFile, _Line, _Flags);
			}
			inline_always static void * DMibCrossmoduleAPI fs_NonTracked_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
			{
				return DNonTrackedHeap->f_ResizeDebug(_pMemory, _Size, _OldSize, _pFile, _Line, _Flags);
			}

			inline_always static void DMibCrossmoduleAPI fs_NonTracked_AllocBatchDebugInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				DNonTrackedHeap->f_AllocBatchDebug(_Size, _Alignment, _Functor, _pFile, _Line, _Flags);
			}
			inline_always static void DMibCrossmoduleAPI fs_NonTracked_AllocBatchDebug(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				DNonTrackedHeap->f_AllocBatchDebug
					(
						_Size
						, _Alignment
						, [&](void * _pAlloc, mint _Size)
						{
							return _fCallBatchFunctor(_pContext, _pAlloc, _Size);
						}
						, _pFile
						, _Line
						, _Flags
					)
				;
			}
#		endif

		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return DNonTrackedHeap->f_AllocWithSize(_Size);
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return DNonTrackedHeap->f_Alloc(_Size);
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
		{
			return DNonTrackedHeap->f_AllocAlignedWithSize(_Size, _Alignment);
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment)
		{
			return DNonTrackedHeap->f_AllocAligned(_Size, _Alignment);
		}
		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			return DNonTrackedHeap->f_Realloc(_pMem, _Size, _OldSize);
		}

		inline_always static void * DMibCrossmoduleAPI fs_NonTracked_Resize(CMemoryManagerCrossModule *_pModule, void *_pMem, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			return DNonTrackedHeap->f_Resize(_pMem, _Size, _OldSize);
		}

		inline_always static void DMibCrossmoduleAPI fs_NonTracked_Free(CMemoryManagerCrossModule *_pModule, void *_pBlock, mint _Size)
		{
			return DNonTrackedHeap->f_Free(_pBlock, _Size);
		}

		inline_always static void DMibCrossmoduleAPI fs_NonTracked_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pBlock)
		{
			return DNonTrackedHeap->f_FreeNoSize(_pBlock);
		}

		inline_never static void * DMibCrossmoduleAPI fs_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return DMainHeap->f_Alloc(_Size);
		}

		inline_always static void * DMibCrossmoduleAPI fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			auto *pMem = DMainHeap->f_AllocAligned(_Size, 1);
			return fg_MemClear(pMem, _Size);
		}

		inline_always static void * DMibCrossmoduleAPI fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Align)
		{
			return DMainHeap->f_AllocAligned(_Size, _Align);
		}
	};

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
		return DMainHeap->f_AllocWithSize(_Size);
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocInitZeroWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
		auto *pMem = DMainHeap->f_AllocWithSize(_Size);
		return fg_MemClear(pMem, _Size);
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align)
	{
		return DMainHeap->f_AllocAlignedWithSize(_Size, _Align);
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		return DMainHeap->f_Realloc(_pMemory, _Size, _OldSize);
	}


	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
		return DMainHeap->f_Resize(_pMemory, _Size, _OldSize);
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
	{
		DMainHeap->f_Free(_pMemory, _Size);
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
	{
		DMainHeap->f_FreeNoSize(_pMemory);
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
	{
		return DMainHeap->f_Size(_pMemory);
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_TrySize(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
	{
		return DMainHeap->f_TrySize(_pMemory);
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
	{
		return DMainHeap->f_SizePadded(_Size);
	}

	inline_always fp32 DMibCrossmoduleAPI CCrossModuleImplementation::fs_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory)
	{
		return DMainHeap->f_Overhead(_pMemory);
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Granularity(CMemoryManagerCrossModule *_pModule)
	{
		return 1;
	}
}
