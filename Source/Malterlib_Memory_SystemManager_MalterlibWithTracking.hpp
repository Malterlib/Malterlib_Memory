// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include "Malterlib_Memory_SystemManager_MalterlibWithTracking.h"

namespace NMib
{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	bool g_bMainHeapIsSmall = false;
	constinit NMib::NStorage::TCAggregateSimple<CMemoryManagerSmall> g_MainHeapSmall = {DAggregateInit};
#endif
	constinit NMib::NStorage::TCAggregateSimple<CMemoryManagerMax> g_MainHeapMax = {DAggregateInit};
	constinit bool g_bMainHeapConstructed = false;

	constinit NMib::NStorage::TCAggregateSimple<CMemoryManagerNonTracked> g_NonTrackedHeap = {DAggregateInit};
	constinit bool g_bNonTrackedHeapConstructed = false;

#if DMibEnableSafeCheck > 0

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
	static auto &fg_MainHeapSmall()
	{
		DMibFastCheck(g_bMainHeapConstructed);
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		DMibFastCheck(g_bMainHeapIsSmall);
#endif
		return g_MainHeapSmall;
	}
	#define DMainHeapSmall fg_MainHeapSmall()
#endif

	static auto &fg_MainHeapMax()
	{
		DMibFastCheck(g_bMainHeapConstructed);
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		DMibFastCheck(!g_bMainHeapIsSmall);
#endif
		return g_MainHeapMax;
	}
	#define DMainHeapMax fg_MainHeapMax()

	static auto &fg_NonTrackedHeap()
	{
		DMibFastCheck(g_bNonTrackedHeapConstructed);
		return g_NonTrackedHeap;
	}
	#define DNonTrackedHeap fg_NonTrackedHeap()
#else
	#define DNonTrackedHeap g_NonTrackedHeap
	#define DMainHeapSmall g_MainHeapSmall
	#define DMainHeapMax g_MainHeapMax
#endif

	constinit NMib::NThread::CMutualAggregate g_MemoryManagerForkLock = {DAggregateInit};
	constinit mint g_MemoryManagerForkedCount = 0;
	constinit bool g_MemoryManagerUnforked = false;
}

namespace NMib::NMemory
{
#if DMibConfig_Memory_Shims_Lightweight
	CReportMemoryLightweight *fg_ReportMemoryLightweightTo(CReportMemoryLightweight *_pMemoryReporter)
	{
		if (!g_bMainHeapConstructed)
			return nullptr;
		DNonTrackedHeap->f_ReportMemoryTo(_pMemoryReporter);
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_ReportMemoryTo(_pMemoryReporter);
		else
#endif
			return DMainHeapMax->f_ReportMemoryTo(_pMemoryReporter);
	}

	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeGetFlags()
	{
		if (!g_bMainHeapConstructed)
			return EMemoryReportLightweightScopeFlag_None;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_GetLightweightScopeFlags();
		else
#endif
			return DMainHeapMax->f_GetLightweightScopeFlags();
	}

	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeSetFlags(EMemoryReportLightweightScopeFlag _Flags)
	{
		if (!g_bMainHeapConstructed)
			return EMemoryReportLightweightScopeFlag_None;
		DNonTrackedHeap->f_SetLightweightScopeFlags(_Flags);
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_SetLightweightScopeFlags(_Flags);
		else
#endif
			return DMainHeapMax->f_SetLightweightScopeFlags(_Flags);
	}

	EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeAddFlags(EMemoryReportLightweightScopeFlag _Flags)
	{
		if (!g_bMainHeapConstructed)
			return EMemoryReportLightweightScopeFlag_None;
		DNonTrackedHeap->f_AddLightweightScopeFlags(_Flags);
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_AddLightweightScopeFlags(_Flags);
		else
#endif
			return DMainHeapMax->f_AddLightweightScopeFlags(_Flags);
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
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_GarbageCollect(true);
			else
#endif
				DMainHeapMax->f_GarbageCollect(true);
			DNonTrackedHeap->f_GarbageCollect(true);
		}

		inline_always static void DMibCrossmoduleAPI fs_CreateMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
			mint PageSize = NSys::fg_Mem_PageSize();

			if (PageSize > CMemoryManagerMax::CParams::mc_SubSlabSize)
			{
				NSys::fg_DebugOutput
					(
						(
							NStr::CFStr256::CFormat("System page size {} is larger than the maximum supported page size of {}\n")
							<< PageSize
							<< CMemoryManagerMax::CParams::mc_SubSlabSize
						).f_GetStr().f_GetStr()
					)
				;
				DMibPDebugBreak;
			}
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			else if (PageSize <= CMemoryManagerSmall::CParams::mc_SubSlabSize)
				g_bMainHeapIsSmall = true;
#endif


#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				g_MainHeapSmall.f_Construct("Main memory manager", CMemoryManagerConfig());
			else
#endif
				g_MainHeapMax.f_Construct("Main memory manager", CMemoryManagerConfig());

			g_bMainHeapConstructed = true;
			{
				// Make sure the code for checking out manager is included
				auto MemoryManagerCheckout = NMib::fg_GetSys()->f_MemoryManager_Checkout();
				NSys::fg_Compiler_MakeActive(1, &MemoryManagerCheckout);
			}
		}
		inline_always static CMemoryManagerCheckout DMibCrossmoduleAPI fs_MemoryManager_Checkout(CMemoryManagerCrossModule *_pModule)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				return DMainHeapSmall->f_CheckoutVirtual();
			else
#endif
				return DMainHeapMax->f_CheckoutVirtual();
		}

		inline_always static void DMibCrossmoduleAPI fs_DestroyMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
#		if DEnableDebugMemoryManager
			{
				if (!CSystem::ms_bDisableMemoryManagerLeakReport)
				{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
					if (g_bMainHeapIsSmall)
						DMainHeapSmall->f_ReportLeaks();
					else
#endif
						DMainHeapMax->f_ReportLeaks();
				}
			}
#		endif
			{
				if (!g_bMemoryManagerNeededAfterDestroy)
				{
#				ifdef DMibConfig_HeapNeverDestroyed
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
					if (g_bMainHeapIsSmall)
						DMainHeapSmall->f_DestroyThreadLocals(); // For debug checks later
					else
#endif
						DMainHeapMax->f_DestroyThreadLocals(); // For debug checks later
#				else
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
					if (g_bMainHeapIsSmall)
						DMainHeapSmall.f_Destruct();
					else
#endif
						DMainHeapMax.f_Destruct();
#				endif
				}
			}
		}

		inline_always static bool DMibCrossmoduleAPI fs_MemoryManager_Check(CMemoryManagerCrossModule *_pModule, bool _bBreak)
		{
			bool bRet = true;
#			if DEnableDebugMemoryManager
			{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if (g_bMainHeapIsSmall)
				{
					if (!DMainHeapSmall->f_CheckAll(_bBreak ? EMemoryManagerCheckFlag_Break : EMemoryManagerCheckFlag_None))
						bRet = false;
				}
				else
#endif
				{
					if (!DMainHeapMax->f_CheckAll(_bBreak ? EMemoryManagerCheckFlag_Break : EMemoryManagerCheckFlag_None))
						bRet = false;
				}

				if (!DNonTrackedHeap->f_CheckAll(_bBreak ? EMemoryManagerCheckFlag_Break : EMemoryManagerCheckFlag_None))
					bRet = false;
			}
#			endif
			return bRet;
		}

		inline_always static bool DMibCrossmoduleAPI fs_ReportingLeaks(CMemoryManagerCrossModule *_pModule)
		{
#		if DEnableDebugMemoryManager
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				return DMainHeapSmall->f_ReportingLeaks();
			else
#endif
				return DMainHeapMax->f_ReportingLeaks();
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

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_CheckoutManual();
			else
#endif
				DMainHeapMax->f_CheckoutManual();
			DNonTrackedHeap->f_CheckoutManual();

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_Lock();
			else
#endif
				DMainHeapMax->f_Lock();

			DNonTrackedHeap->f_Lock();

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_PrepareFork();
			else
#endif
				DMainHeapMax->f_PrepareFork();
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
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_ForkedParent();
			else
#endif
				DMainHeapMax->f_ForkedParent();

			DNonTrackedHeap->f_Unlock();
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_Unlock();
			else
#endif
				DMainHeapMax->f_Unlock();

			DNonTrackedHeap->f_CheckinManual();
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_CheckinManual();
			else
#endif
				DMainHeapMax->f_CheckinManual();

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
			DNonTrackedHeap->f_Unlock();

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_ForkedChild();
			else
#endif
				DMainHeapMax->f_ForkedChild();

#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_Unlock();
			else
#endif
				DMainHeapMax->f_Unlock();

			DNonTrackedHeap->f_CheckinManual();
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_CheckinManual();
			else
#endif
				DMainHeapMax->f_CheckinManual();

			g_MemoryManagerForkLock.f_Unlock();
		}

		inline_always static void DMibCrossmoduleAPI fs_MemoryManager_DestroyThreads(CMemoryManagerCrossModule *_pModule)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_DestroyCleanupThreads();
			else
#endif
				DMainHeapMax->f_DestroyCleanupThreads();
			DNonTrackedHeap->f_DestroyCleanupThreads();
		}

		inline_always static void DMibCrossmoduleAPI fs_MemoryManager_CanStrartThreads(CMemoryManagerCrossModule *_pModule)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_CanStartThreads();
			else
#endif
				DMainHeapMax->f_CanStartThreads();
			DNonTrackedHeap->f_CanStartThreads();
		}

		inline_always static void DMibCrossmoduleAPI fs_MemoryManager_SetNumaNode(CMemoryManagerCrossModule *_pModule, ENumaNode _NumaNode)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_SetNumaNode(_NumaNode);
			else
#endif
				DMainHeapMax->f_SetNumaNode(_NumaNode);
			// Don't bother with non-tracked heap as that should not be used a lot
			//DNonTrackedHeap->f_SetNumaNode(_NumaNode);
		}
		static constexpr bool mc_SupportsNonTracked = true;
#		if DEnableDebugMemoryManager
			static constexpr bool mc_SupportsDebug = true;
			inline_always static void * DMibCrossmoduleAPI fs_AllocWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if (g_bMainHeapIsSmall)
					return DMainHeapSmall->f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags);
				else
#endif
					return DMainHeapMax->f_AllocWithSizeDebug(_Size, _pFile, _Line, _Flags);
			}
			inline_always static void * DMibCrossmoduleAPI fs_AllocAlignedWithSizeDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if (g_bMainHeapIsSmall)
					return DMainHeapSmall->f_AllocAlignedWithSizeDebug(_Size, _Align, _pFile, _Line, _Flags);
				else
#endif
					return DMainHeapMax->f_AllocAlignedWithSizeDebug(_Size, _Align, _pFile, _Line, _Flags);
			}
			inline_always static void * DMibCrossmoduleAPI fs_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
			{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if (g_bMainHeapIsSmall)
					return DMainHeapSmall->f_ReallocDebug(_pMemory, _Size, _OldSize, _pFile, _Line, _Flags);
				else
#endif
					return DMainHeapMax->f_ReallocDebug(_pMemory, _Size, _OldSize, _pFile, _Line, _Flags);
			}
			inline_always static void * DMibCrossmoduleAPI fs_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags, EAllocationFlag _AllocFlags)
			{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if (g_bMainHeapIsSmall)
					return DMainHeapSmall->f_ResizeDebug(_pMemory, _Size, _OldSize, _pFile, _Line, _Flags);
				else
#endif
					return DMainHeapMax->f_ResizeDebug(_pMemory, _Size, _OldSize, _pFile, _Line, _Flags);
			}
			inline_always static void DMibCrossmoduleAPI fs_AllocBatchDebugInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if (g_bMainHeapIsSmall)
					DMainHeapSmall->f_AllocBatchDebug(_Size, _Alignment, _Functor, _pFile, _Line, _Flags);
				else
#endif
					DMainHeapMax->f_AllocBatchDebug(_Size, _Alignment, _Functor, _pFile, _Line, _Flags);
			}
			inline_always static void DMibCrossmoduleAPI fs_AllocBatchDebug(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
				if (g_bMainHeapIsSmall)
				{
					DMainHeapSmall->f_AllocBatchDebug
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
				else
#endif
				{
					DMainHeapMax->f_AllocBatchDebug
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
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_AllocBatch(_Size, _Alignment, _Functor);
			else
#endif
				DMainHeapMax->f_AllocBatch(_Size, _Alignment, _Functor);
		}
		inline_always static void DMibCrossmoduleAPI fs_AllocBatch(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
			{
				DMainHeapSmall->f_AllocBatch
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
			else
#endif
			{
				DMainHeapMax->f_AllocBatch
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
		}
		inline_always static void DMibCrossmoduleAPI fs_NonTracked_AllocBatchInternal(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				DMainHeapSmall->f_AllocBatch(_Size, _Alignment, _Functor);
			else
#endif
				DMainHeapMax->f_AllocBatch(_Size, _Alignment, _Functor);
		}
		inline_always static void DMibCrossmoduleAPI fs_NonTracked_AllocBatch(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Alignment, bool (DMibCrossmoduleAPI * _fCallBatchFunctor)(void *_pContext, void * _pAlloc, mint _Size), void * _pContext)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
			{
				DMainHeapSmall->f_AllocBatch
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
			else
#endif
			{
				DMainHeapMax->f_AllocBatch
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

		inline_always static bool DMibCrossmoduleAPI fs_AllocHasDeterministicSize(CMemoryManagerCrossModule *_pModule)
		{
#if DEnableDebugMemoryManager
			return false;
#else
			return true;
#endif
		}

		inline_always static EMemoryManagerFeatureFlag DMibCrossmoduleAPI fs_MemoryManagerFeatures(CMemoryManagerCrossModule *_pModule)
		{
			auto Return = EMemoryManagerFeatureFlag_None;

			if constexpr (CMemoryManagerDebugOptions::mc_bTraceLeaks)
				Return |= EMemoryManagerFeatureFlag_TraceLeaks;

			if constexpr (CMemoryManagerDebugOptions::mc_StackTraceDepth > 0)
				Return |= EMemoryManagerFeatureFlag_StackTrace;

			if constexpr (CMemoryManagerDebugOptions::mc_bCheckModifyAfterFree)
				Return |= EMemoryManagerFeatureFlag_CheckModifyAfterFree;

			if constexpr (CMemoryManagerDebugOptions::mc_nPreGuardBytes > 0)
				Return |= EMemoryManagerFeatureFlag_PreGuard;

			if constexpr (CMemoryManagerDebugOptions::mc_nPostGuardBytes > 0)
				Return |= EMemoryManagerFeatureFlag_PostGuard;

			if constexpr (CMemoryManagerDebugOptions::mc_bFreeValidation)
				Return |= EMemoryManagerFeatureFlag_FreeValidation;

			if constexpr (CMemoryManagerDebugOptions::mc_bEnumeration)
				Return |= EMemoryManagerFeatureFlag_Enumeration;

			if constexpr (CMemoryManagerDebugOptions::mc_bAssertOnMemoryLeak)
				Return |= EMemoryManagerFeatureFlag_AssertOnMemoryLeak;

			return Return;
		}

		inline_always static void DMibCrossmoduleAPI fs_NonTracked_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pBlock)
		{
			return DNonTrackedHeap->f_FreeNoSize(_pBlock);
		}

		inline_never static void * DMibCrossmoduleAPI fs_Alloc(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				return DMainHeapSmall->f_Alloc(_Size);
			else
#endif
				return DMainHeapMax->f_Alloc(_Size);
		}

		inline_always static void * DMibCrossmoduleAPI fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			void *pMem;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				pMem = DMainHeapSmall->f_AllocAligned(_Size, 1);
			else
#endif
				pMem = DMainHeapMax->f_AllocAligned(_Size, 1);
			return fg_MemClear(pMem, _Size);
		}

		inline_always static void * DMibCrossmoduleAPI fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint _Size, mint _Align)
		{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
			if (g_bMainHeapIsSmall)
				return DMainHeapSmall->f_AllocAligned(_Size, _Align);
			else
#endif
				return DMainHeapMax->f_AllocAligned(_Size, _Align);
		}
	};

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_AllocWithSize(_Size);
		else
#endif
			return DMainHeapMax->f_AllocWithSize(_Size);
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocInitZeroWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
	{
		void *pMem;
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			pMem = DMainHeapSmall->f_AllocWithSize(_Size);
		else
#endif
			pMem = DMainHeapMax->f_AllocWithSize(_Size);
		return fg_MemClear(pMem, _Size);
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align)
	{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_AllocAlignedWithSize(_Size, _Align);
		else
#endif
			return DMainHeapMax->f_AllocAlignedWithSize(_Size, _Align);
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_Realloc(_pMemory, _Size, _OldSize);
		else
#endif
			return DMainHeapMax->f_Realloc(_pMemory, _Size, _OldSize);
	}

	inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
	{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_Resize(_pMemory, _Size, _OldSize);
		else
#endif
			return DMainHeapMax->f_Resize(_pMemory, _Size, _OldSize);
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
	{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			DMainHeapSmall->f_Free(_pMemory, _Size);
		else
#endif
			DMainHeapMax->f_Free(_pMemory, _Size);
	}

	inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
	{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			DMainHeapSmall->f_FreeNoSize(_pMemory);
		else
#endif
			DMainHeapMax->f_FreeNoSize(_pMemory);
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
	{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_Size(_pMemory);
		else
#endif
			return DMainHeapMax->f_Size(_pMemory);
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_TrySize(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
	{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_TrySize(_pMemory);
		else
#endif
			return DMainHeapMax->f_TrySize(_pMemory);
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
	{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_SizePadded(_Size);
		else
#endif
			return DMainHeapMax->f_SizePadded(_Size);
	}

	inline_always fp32 DMibCrossmoduleAPI CCrossModuleImplementation::fs_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory)
	{
#if DMibConfig_MalterlibMemoryManager_NeedDualPageSize
		if (g_bMainHeapIsSmall)
			return DMainHeapSmall->f_Overhead(_pMemory);
		else
#endif
			return DMainHeapMax->f_Overhead(_pMemory);
	}

	inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Granularity(CMemoryManagerCrossModule *_pModule)
	{
		return 1;
	}
}
