// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include "Malterlib_Memory_Heap.h"

namespace NMib
{

	enum
	{
#ifdef DPlatformFamily_OSX
		// OSX needs 16 byte alignment to handle SSE
		EMemoryManagerAlignmentCalc = DMibGetHighestBitSet(sizeof(mint)*2) - 1
		, EMemoryManagerAlignment = EMemoryManagerAlignmentCalc < 4 ? 4 : EMemoryManagerAlignmentCalc
#else
		EMemoryManagerAlignment = DMibGetHighestBitSet(sizeof(mint)*2) - 1
#endif
	};
	
#if DMibConfig_MalterlibMemoryManager_Debug && DMibConfig_MalterlibMemoryManager_Debug_Features || DMibConfig_MalterlibMemoryManager_Debug_Features == 1
#	define DEnableDebugMemoryManager 1
#else
#	define DEnableDebugMemoryManager 0
#endif

	
		
#	if DEnableDebugMemoryManager
		class CHeapParams : public NMem::TCHeapParamsDebug<4096*1024, NMem::CAllocator_VirtualNoCommit, 2, EMemoryManagerAlignment, NMib::NThread::CMutualNoRecurse>
		{
		public:
			//typedef NMem::CHeap_FillNoDebug CFillDebug;
		};
		typedef NMib::NMem::TCHeap_CombinedDebug< CHeapParams > CMainHeap;
#	else
		typedef NMib::NMem::TCHeap_Combined<-1, NMem::TCHeapParams<4096*1024, CMainHeapVirtualAllocator, 2, EMemoryManagerAlignment, NMib::NThread::CMutualNoRecurse> > CMainHeap;
#	endif

	NMib::NAggregate::TCAggregateSimple<CMainHeap> g_MainHeap = {DAggregateInit};
		
#	if DEnableDebugMemoryManager
		class CHeapParamsNonTracked : public NMem::TCHeapParamsDebug<64*1024, NMem::CAllocator_VirtualNoTracking, 2, EMemoryManagerAlignment, NMib::NThread::CMutualNoRecurse>
		{
		public:
		
		//typedef NMem::CHeap_FillNoDebug CFillDebug;
		
		};
		typedef NMib::NMem::TCHeap_CombinedDebug< CHeapParamsNonTracked > CNonTrackedHeap;
#	else
		typedef NMib::NMem::TCHeap_StandAlone<NMem::TCHeapParams<64*1024, NMem::CAllocator_VirtualNoTracking, 2, EMemoryManagerAlignment, NMib::NThread::CMutualNoRecurse> > CNonTrackedHeap;
#	endif
	NMib::NAggregate::TCAggregateSimple<CNonTrackedHeap> g_NonTrackedHeap = {DAggregateInit};
	NMib::NThread::CMutualAggregate g_MemoryManagerForkLock = {DAggregateInit};
	mint g_MemoryManagerForkedCount = 0;
	bool g_MemoryManagerUnforked = false;

	namespace NMem
	{
		inline_always void CCrossModuleImplementation::fs_CreateNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
			g_NonTrackedHeap.f_Construct((ch8 const *)nullptr);
		}
	
		inline_always void CCrossModuleImplementation::fs_DestroyNonTrackedMemoryManager(CMemoryManagerCrossModule *_pModule)
		{
	#		ifndef DMibConfig_HeapNeverDestroyed
			g_NonTrackedHeap.f_Destruct();
			g_MemoryManagerForkLock.f_Destruct();
#endif
		}
		struct CCrossModuleImplementationExtra : public CCrossModuleImplementation
		{
			inline_always static void fs_CreateMemoryManager(CMemoryManagerCrossModule *_pModule)
			{
				g_MainHeap.f_Construct("Main Heap");
			}
	
			inline_always static void fs_DestroyMemoryManager(CMemoryManagerCrossModule *_pModule)
			{
		#		ifndef DMibConfig_HeapNeverDestroyed
					if (!CSystem::ms_bDisableMemoryManagerLeakReport)
						g_MainHeap->f_TraceLeaks(true);
					g_MainHeap.f_Destruct();
		#		else
					if (!CSystem::ms_bDisableMemoryManagerLeakReport)
						g_MainHeap->f_TraceLeaks(false);
		#		endif
			}
	
			inline_always static bool fs_MemoryManager_Check(CMemoryManagerCrossModule *_pModule, bool _bBreak)
			{
				return g_MainHeap->f_CheckHeap(_bBreak);
			}

			inline_always static void fs_MemoryManager_PrepareFork(CMemoryManagerCrossModule *_pModule)
			{
				g_MemoryManagerForkLock.f_Lock();
				if (++g_MemoryManagerForkedCount > 1)
					return;
				g_MemoryManagerForkLock.f_PrepareFork();
		
				g_MemoryManagerUnforked = false;
				
				g_MainHeap->f_Lock();
				g_MainHeap->f_PrepareFork();
				g_NonTrackedHeap->f_Lock();
				g_NonTrackedHeap->f_PrepareFork();
			}
	
			inline_always static void fs_MemoryManager_ForkedParent(CMemoryManagerCrossModule *_pModule)
			{
				--g_MemoryManagerForkedCount;
				if (g_MemoryManagerUnforked)
				{
					g_MemoryManagerForkLock.f_Unlock();
					return;
				}
				g_MemoryManagerForkLock.f_ForkedParent();
				g_MemoryManagerUnforked = true;
				
				g_NonTrackedHeap->f_ForkedParent();
				g_NonTrackedHeap->f_Unlock();
				g_MainHeap->f_ForkedParent();
				g_MainHeap->f_Unlock();

				g_MemoryManagerForkLock.f_Unlock();
			}
	
			inline_always static void fs_MemoryManager_ForkedChild(CMemoryManagerCrossModule *_pModule)
			{
				--g_MemoryManagerForkedCount;
				if (g_MemoryManagerUnforked)
				{
					g_MemoryManagerForkLock.f_Unlock();
					return;
				}
				g_MemoryManagerForkLock.f_ForkedChild();
				g_MemoryManagerUnforked = true;
				
				g_NonTrackedHeap->f_ForkedChild();
				g_NonTrackedHeap->f_Unlock();
		
				g_MainHeap->f_ForkedChild();
				g_MainHeap->f_Unlock();

				g_MemoryManagerForkLock.f_Unlock();
			}

			static const bool mc_SupportsNonTracked = true;

#		if DEnableDebugMemoryManager
			static const bool mc_SupportsDebug = true;
			inline_always static void * fs_AllocDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return g_MainHeap->f_AllocDebug(_Size, _pFile, _Line, _Flags);
			}

			inline_always static void * fs_AllocAlignedDebug(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return g_MainHeap->f_AllocAlignedDebug(_Size, _Alignment, _pFile, _Line, _Flags);
			}
			inline_always static void * fs_ReallocDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return fg_Realloc(_pMemory, _Size);
			}
			inline_always static void * fs_ResizeDebug(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size, const ch8 *_pFile, aint _Line, EHeapDebugFlag _Flags)
			{
				return fg_Resize(_pMemory, _Size);
			}
#		else
			static const bool mc_SupportsDebug = false;
#		endif


			inline_always static mint fs_NonTracked_Granularity(CMemoryManagerCrossModule *_pModule)
			{
				return g_NonTrackedHeap->f_MaxGranularity();
			}

			inline_always static mint fs_NonTracked_Size(CMemoryManagerCrossModule *_pModule, void *_pMemory)
			{
				return g_NonTrackedHeap->f_Size(_pMemory);
			}

			inline_always static mint fs_NonTracked_TrySize(CMemoryManagerCrossModule *_pModule, void *_pMemory)
			{
				DMibPDebugBreak; // Not supported
				return 0;
			}

			inline_always static mint fs_NonTracked_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
			{
				return fg_AlignUp(_Size, g_NonTrackedHeap->f_MaxGranularity());
			}
		
			inline_always static fp32 fs_NonTracked_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory) // Number of bytes overhead for block
			{
				return g_NonTrackedHeap->f_Overhead(_pMemory);
			}
			inline_always static void *fs_NonTracked_Alloc(CMemoryManagerCrossModule *_pModule, mint &_Size)
			{
				return g_NonTrackedHeap->f_Alloc(_Size);
			}
			inline_always static void *fs_NonTracked_AllocAligned(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
			{
				return g_NonTrackedHeap->f_AllocAligned(_Size, _Alignment);
			}
			inline_always static void *fs_NonTracked_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
			{
				return g_NonTrackedHeap->f_Realloc(_pMemory, _Size);
			}

			inline_always static void *fs_NonTracked_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
			{
				return g_NonTrackedHeap->f_Resize(_pMemory, _Size);
			}

			inline_always static void fs_NonTracked_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory)
			{
				return g_NonTrackedHeap->f_Free(_pMemory);
			}
		};
	
		inline_always void * CCrossModuleImplementation::fs_Alloc(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return g_MainHeap->f_Alloc(_Size);
		}

		inline_always void * CCrossModuleImplementation::fs_AllocInitZero(CMemoryManagerCrossModule *_pModule, mint &_Size)
		{
			return fg_MemClear(g_MainHeap->f_Alloc(_Size), _Size);
		}

		inline_always void * CCrossModuleImplementation::fs_AllocAligned(CMemoryManagerCrossModule *_pModule, mint &_Size, mint _Alignment)
		{
			return g_MainHeap->f_AllocAligned(_Size, _Alignment);
		}


		inline_always void * CCrossModuleImplementation::fs_Realloc(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
		{
			return g_MainHeap->f_Realloc(_pMemory, _Size);
		}

		inline_always void * CCrossModuleImplementation::fs_Resize(CMemoryManagerCrossModule *_pModule, void *_pMemory, mint &_Size)
		{
			return g_MainHeap->f_Resize(_pMemory, _Size);
		}

		inline_always void CCrossModuleImplementation::fs_Free(CMemoryManagerCrossModule *_pModule, void *_pMemory)
		{
			g_MainHeap->f_Free(_pMemory);
		}

		inline_always mint CCrossModuleImplementation::fs_Size(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
		{
			return g_MainHeap->f_Size(_pMemory);
		}

		inline_always mint CCrossModuleImplementation::fs_TrySize(CMemoryManagerCrossModule *_pModule, const void *_pMemory)
		{
			DMibPDebugBreak; // Not supported
			return 0;
		}
		
		inline_always mint CCrossModuleImplementation::fs_SizePadded(CMemoryManagerCrossModule *_pModule, mint _Size)
		{
			return fg_AlignUp(_Size, g_MainHeap->f_MaxGranularity());
		}
		
		inline_always fp32 CCrossModuleImplementation::fs_Overhead(CMemoryManagerCrossModule *_pModule, void const *_pMemory)
		{
			return g_MainHeap->f_Overhead(_pMemory);
		}
		
		inline_always mint CCrossModuleImplementation::fs_Granularity(CMemoryManagerCrossModule *_pModule)
		{
			return g_MainHeap->f_MaxGranularity();
		}


	} // Namespace NMem
} // Namespace NMib
