// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include "Malterlib_Memory_DebugMemoryManager.h"

namespace NMib
{
	
#if DMibPPtrBits == 32
	//#warning "Overwrite memory manager not suiteable for 32 bit executables as you are likely to run out of memory"
#endif

#define DMibGuardRandom

//#define DMibGuardBefore

	/*
#if	defined(DMibGuardRandom)
	NAggregate::TCAggregateSimple<NMib::NMem::TCDebugMemoryManager<NMib::NMem::EDebugMemoryManager_CheckRandom>> g_DebugMemoryManager = {0};
#elif defined(DMibGuardBefore)
	NAggregate::TCAggregateSimple<NMib::NMem::TCDebugMemoryManager<NMib::NMem::EDebugMemoryManager_None>> g_DebugMemoryManager = {0};
#else
	NAggregate::TCAggregateSimple<NMib::NMem::TCDebugMemoryManager<NMib::NMem::EDebugMemoryManager_CheckUpper>> g_DebugMemoryManager = {0};
#endif
*/
//		NAggregate::TCAggregateSimple<NMib::NMem::TCDebugMemoryManager<int(NMib::NMem::EDebugMemoryManager_CheckUpper) | int(NMib::NMem::EDebugMemoryManager_ProtectOnDemand)>> g_DebugMemoryManager = {0};

#if 1
	NAggregate::TCAggregateSimple<NMib::NMem::TCDebugMemoryManager<(NMib::NMem::EDebugMemoryManager)int(NMib::NMem::EDebugMemoryManager_CheckUpper)>> g_DebugMemoryManager = {DAggregateInit};
#else
	NAggregate::TCAggregateSimple<NMib::NMem::TCDebugMemoryManager<(NMib::NMem::EDebugMemoryManager)int(0)>> g_DebugMemoryManager = {DAggregateInit};
#endif

	NMib::NThread::CMutualAggregate g_MemoryManagerForkLock = {DAggregateInit};
	mint g_MemoryManagerForkedCount = 0;
	bool g_MemoryManagerUnforked = false;

	NThread::TMutual<NThread::CEventAutoResetAggregate, true> &fg_GetDebugMemoryManagerLock()
	{
		return g_DebugMemoryManager->f_GetLock();
	}
	
	namespace NMem
	{
#if DMibConfig_Memory_Shims_Lightweight
		CReportMemoryLightweight *fg_ReportMemoryLightweightTo(CReportMemoryLightweight *_pMemoryReporter)
		{
			return g_DebugMemoryManager->f_ReportMemoryLightweightTo(_pMemoryReporter);
		}

		EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeGetFlags()
		{
			return g_DebugMemoryManager->f_GetLightweightScopeFlags();
		}

		EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeSetFlags(EMemoryReportLightweightScopeFlag _Flags)
		{
			return g_DebugMemoryManager->f_SetLightweightScopeFlags(_Flags);
		}

		EMemoryReportLightweightScopeFlag fg_MemoryLightweightScopeAddFlags(EMemoryReportLightweightScopeFlag _Flags)
		{
			return g_DebugMemoryManager->f_AddLightweightScopeFlags(_Flags);
		}
#endif
		inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_CreateNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
			g_DebugMemoryManager.f_Construct();
		}
		
		inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
	#		ifndef DMibConfig_HeapNeverDestroyed
			if (!g_bMemoryManagerNeededAfterDestroy)
			{
				g_DebugMemoryManager.f_Destruct();
				g_MemoryManagerForkLock.f_Destruct();
			}
	#		endif
		}
		struct CCrossModuleImplementationExtra : public CCrossModuleImplementation
		{
			static constexpr bool mc_SupportsNonTracked = false;
			static constexpr bool mc_SupportsDebug = false;

			inline_always static void DMibCrossmoduleAPI fs_MemoryManager_PrepareFork(CMemoryManagerCrossModule *_pModule)
			{
				g_MemoryManagerForkLock.f_Lock();
				if (++g_MemoryManagerForkedCount > 1)
					return;
				g_MemoryManagerForkLock.f_PrepareFork();
		
				g_MemoryManagerUnforked = false;
				
				g_DebugMemoryManager->f_Lock();
				g_DebugMemoryManager->f_PrepareFork();
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

				g_DebugMemoryManager->f_ForkedParent();
				g_DebugMemoryManager->f_Unlock();

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

				g_DebugMemoryManager->f_ForkedChild();
				g_DebugMemoryManager->f_Unlock();

				g_MemoryManagerForkLock.f_Unlock();
			}
			
			inline_always static void DMibCrossmoduleAPI fs_DemandProtection(CMemoryManagerCrossModule *_pModule)
			{
				g_DebugMemoryManager->f_DemandProtection();
			}
		};
		
		inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return g_DebugMemoryManager->f_AllocWithSize(_Size, 1);
		}

		inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocInitZeroWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return fg_MemClear(g_DebugMemoryManager->f_AllocWithSize(_Size, 0), _Size);
		}

		inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_AllocAlignedWithSize(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Align)
		{
			return g_DebugMemoryManager->f_AllocWithSize(_Size, _Align);
		}

		inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			if (_OldSize)
				fs_Free(_pModule, _pMemory, _OldSize);
			else
				fs_FreeNoSize(_pModule, _pMemory);
			return fs_AllocWithSize(_pModule, _Size);
		}

		inline_always void * DMibCrossmoduleAPI CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, mint _OldSize, EAllocationFlag _AllocFlags)
		{
			void *pRet = fs_AllocWithSize(_pModule, _Size);
			mint OldSize = _OldSize ? _OldSize : fs_Size(_pModule, _pMemory);
			fg_MemCopy(pRet, _pMemory, fg_Min(OldSize, _Size));
			fs_Free(_pModule, _pMemory, OldSize);
			return pRet;
		}

		inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint _Size)
		{
			g_DebugMemoryManager->f_Free(_pMemory, _Size);
		}

		inline_always void DMibCrossmoduleAPI CCrossModuleImplementation::fs_FreeNoSize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			g_DebugMemoryManager->f_FreeNoSize(_pMemory);
		}

		inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
		{
			return g_DebugMemoryManager->f_Size(_pMemory);
		}

		inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_TrySize(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
		{
			return g_DebugMemoryManager->f_TrySize(_pMemory);
		}
		
		inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_Granularity(CMemoryManagerCrossModule *_pModule)
		{
			return 1;
		}
		
		inline_always mint DMibCrossmoduleAPI CCrossModuleImplementation::fs_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return fg_AlignUp(_Size, fs_Granularity(_pModule));
		}
		
		inline_always fp32 DMibCrossmoduleAPI CCrossModuleImplementation::fs_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory)
		{
			return g_DebugMemoryManager->f_Overhead(_pMemory);
		}
	}
} // Namespace NMib
