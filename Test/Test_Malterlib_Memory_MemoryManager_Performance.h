// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#if defined(DPlatformFamily_Windows)
	#define DMibPWindowsTest
#endif

#ifndef DMibPWindowsTest
#	undef DMemoryManagerTestEnable_WindowsDefault
#	undef DMemoryManagerTestEnable_WindowsLF
#	undef DMemoryManagerTestEnable_PtMalloc
#	undef DMemoryManagerTestEnable_LLAlloc
#	undef DMemoryManagerTestEnable_TcMalloc
#endif

#if DMibPPtrBits != 64 
#	undef DMemoryManagerTestEnable_TcMalloc
#endif

#ifdef DMemoryManagerTestEnable_MalterlibOld
#include "../../Memory/Source/Malterlib_Memory_Heap.h"
#endif

#ifdef DMemoryManagerTestEnable_MalterlibNew
#include <Mib/Memory/MemoryManager>
#include <Mib/Memory/MemoryManagerDebug>
#include <Mib/Memory/MemoryManagerTracked>
#endif

#ifdef DMemoryManagerTestEnable_DlMalloc
#include "../../../SDK/DLMalloc/dlmalloc_singlethreaded.h"
#include "../../../SDK/DLMalloc/dlmalloc_singlethreadedclear.h"
#endif

#ifdef DMemoryManagerTestEnable_DlMallocMultiThreaded
#include "../../../SDK/DLMalloc/dlmalloc_multithreaded.h"
#include "../../../SDK/DLMalloc/dlmalloc_multithreadedclear.h"
#endif

#ifdef DMemoryManagerTestEnable_PtMalloc
#include "../../../SDK/PTMalloc/ptmalloc_cpp.h"
#endif

#ifdef DMemoryManagerTestEnable_TcMalloc
#include <google/tcmalloc.h>
#endif

#ifdef DMemoryManagerTestEnable_LLAlloc
#include "../../../SDK/LLAlloc/ll_alloc.h"
#endif

#if defined(DMemoryManagerTestEnable_WindowsDefault) || defined(DMemoryManagerTestEnable_WindowsLF)
#include <windows.h>
#endif

#ifdef DMemoryManagerTestEnable_StdLib
#include <stdlib.h>
#endif

#ifdef DMemoryManagerTestEnable_OSX
#include <malloc/malloc.h>
#endif

#include <mutex>

namespace
{

	enum 
	{
		EUseFast = true
	};

	class CMalterlibMemoryDummy
	{
		NMib::NThread::TCThreadLocal<NMib::NContainer::TCVector<uint8>> m_Memory;
		NMib::NThread::TCThreadLocal
			<
				uint8 *
				, NMib::NMem::CAllocator_Heap
				, (NMib::NThread::EThreadLocalFlag)(constenum(NMib::NThread::EThreadLocalFlag_AlwaysCreated) | constenum(NMib::NThread::EThreadLocalFlag_FastThreadLocal))
			>
			m_pMemory
		;
		mint m_MaxSize;
	public:
		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			m_MaxSize = _MaxSize;
			return true;
		}

		void f_InitThread()
		{
			(*m_Memory).f_SetLen(m_MaxSize * 2);
			(*m_pMemory) = (*m_Memory).f_GetArray();
		}

		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			
			return NMib::fg_AlignUp((*m_pMemory), _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return (*m_pMemory);
		}

		inline_small void f_Free(void *_pMem)
		{
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};

#ifdef DMemoryManagerTestEnable_MalterlibNew
	class CMalterlibMemoryMalterlibNew
	{
		NMib::NMem::TCMemoryManager<NMib::NMem::CDefaultMemoryManagerParams> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlibNew()
		{
		}
		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}
		
		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			m_MemoryManager.f_Free(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
	
	class CMalterlibMemoryMalterlibNew_Debug
	{
		NMib::NMem::TCMemoryManagerDebug<NMib::NMem::CDefaultMemoryManagerParams, false> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlibNew_Debug()
		{
		}
		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}
		
		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			m_MemoryManager.f_Free(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};

	
	class CMalterlibMemoryMalterlibNew_Tracked
	{
		NMib::NMem::TCMemoryManagerTracked<NMib::NMem::TCMemoryManager<NMib::NMem::CDefaultMemoryManagerParams>> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlibNew_Tracked()
			: m_MemoryManager("Test tracked manager")
		{
		}
		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}
		
		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			m_MemoryManager.f_Free(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};

	class CMalterlibMemoryMalterlibNew_NoCleanup
	{
		struct CParams : public NMib::NMem::CDefaultMemoryManagerParams
		{
			static constexpr NMib::NMem::EDeferCleanup mc_DeferCleanup
				= NMib::NMem::EDeferCleanup
				(
					constenum(NMib::NMem::EDeferCleanup_NoCleanup)
					| constenum(NMib::NMem::EDeferCleanup_OneSizeBlocks)
					| constenum(NMib::NMem::EDeferCleanup_Commit)
					| constenum(NMib::NMem::EDeferCleanup_Allocs)
				)
			;
		};
		
		NMib::NMem::TCMemoryManager<CParams> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlibNew_NoCleanup()
		{
		}
		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}
		
		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			m_MemoryManager.f_Free(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
	class CMalterlibMemoryMalterlibNew_NoDeferCleanup
	{
		struct CParams : public NMib::NMem::CDefaultMemoryManagerParams
		{
			static constexpr NMib::NMem::EDeferCleanup mc_DeferCleanup = NMib::NMem::EDeferCleanup_None;
		};
		
		NMib::NMem::TCMemoryManager<CParams> m_MemoryManager;
	public:
		
		CMalterlibMemoryMalterlibNew_NoDeferCleanup()
		{
		}
		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}
		
		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			m_MemoryManager.f_Free(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
	class CMalterlibMemoryMalterlibNew_NoCheckout
	{
		NMib::NMem::TCMemoryManager<NMib::NMem::CDefaultMemoryManagerParams> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlibNew_NoCheckout()
		{
		}
		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}
		
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			m_MemoryManager.f_Free(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
	class CMalterlibMemoryMalterlibNew_NoCommit
	{
		NMib::NMem::TCMemoryManager<NMib::NMem::CDefaultMemoryManagerParams_NoCommit> m_MemoryManager;
	public:
		CMalterlibMemoryMalterlibNew_NoCommit()
		{
		}
		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			return true;
		}
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
			m_MemoryManager.f_SetNumaNode(_Node);
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}
		
		auto f_Checkout() -> decltype(m_MemoryManager.f_Checkout())
		{
			return m_MemoryManager.f_Checkout();
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_MemoryManager.f_AllocAligned(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_MemoryManager.f_Alloc(_Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			m_MemoryManager.f_Free(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
#endif

	//#define VirtualLimit 1024*1024
	#define VirtualLimit -1

#ifdef DMemoryManagerTestEnable_MalterlibOld
	class CMutualMutex
	{
		std::mutex m_Mutex;
	public:
		void f_Lock()
		{
			m_Mutex.lock();
		}
		void f_Unlock()
		{
			m_Mutex.unlock();
		}
	};
	
	struct CTicketLockShared
	{
		union UTicket
		{
			NMib::NAtomic::TCAtomicAggregate<mint> m_Lock;
			struct CSplit
			{
				NMib::NAtomic::TCAtomicAggregate<uint32> m_TicketRelease;
				NMib::NAtomic::TCAtomicAggregate<uint32> m_Ticket;
			} m_Split;
			struct CSplitDebug
			{
				uint32 m_TicketRelease;
				uint32 m_Ticket;
			} m_SplitDebug;
			NMib::NTraits::TCAlign<uint8, DMibPMemoryCacheLineSize>::CType m_AlignedPadding;
			
		};
		
		UTicket m_Lock;
		
		static constexpr mint mc_Split = sizeof(mint) * 8 / 2;
		static constexpr mint mc_Lower = DMibBitRangeTyped(0, mc_Split - 1, mint);
		inline_never CTicketLockShared()
		{
			m_Lock.m_Lock.f_Store(1);
		}
		
		inline_never void f_Lock()
		{
			mint Old = m_Lock.m_Lock.f_FetchAdd(mint(1) << mc_Split, NMib::NAtomic::EMemoryOrder_Acquire);
			mint Ticket = (Old >> mc_Split) + 1;
			mint TicketRelease = Old & mc_Lower;
			
			if (Ticket != TicketRelease)
			{
				while (Ticket != TicketRelease)
				{
					for (volatile int i = 0; i < 32; ++i)
					{
						yield_cpu;
						NMib::NAtomic::fg_CompilerFence(NMib::NAtomic::EMemoryOrder_Relaxed);
					}
					TicketRelease = m_Lock.m_Split.m_TicketRelease.f_Load(NMib::NAtomic::EMemoryOrder_Relaxed);
				}
				NMib::NAtomic::fg_MemoryFence(NMib::NAtomic::EMemoryOrder_Acquire);
			}
		}
		
		inline_never void f_Unlock()
		{
			m_Lock.m_Split.m_TicketRelease.f_FetchAdd(1, NMib::NAtomic::EMemoryOrder_Release);
		}
	};

	struct CTicketLock
	{
		union UTicket
		{
			NMib::NAtomic::TCAtomicAggregate<mint> m_Lock;
			NMib::NTraits::TCAlign<uint8, DMibPMemoryCacheLineSize>::CType m_AlignedPadding;
			
			UTicket()
			{
			}
		};
		union URelease
		{
			NMib::NAtomic::TCAtomicAggregate<mint> m_Lock;
			NMib::NTraits::TCAlign<uint8, DMibPMemoryCacheLineSize>::CType m_AlignedPadding;
			
			URelease()
			{
			}
		};
		
		UTicket m_Ticket;
		URelease m_Release;
		
		CTicketLock()
		{
			m_Ticket.m_Lock.f_Store(0);
			m_Release.m_Lock.f_Store(1);
		}
		
		inline_never void f_Lock()
		{
			mint Ticket = m_Ticket.m_Lock.f_FetchAdd(1, NMib::NAtomic::EMemoryOrder_Acquire) + 1;
			mint TicketRelease = m_Release.m_Lock.f_Load(NMib::NAtomic::EMemoryOrder_Acquire);
//			mint YieldCount = 511;
//			mint Count = 0;
			while (Ticket != TicketRelease)
			{
				yield_cpu;
/*				for (volatile int i = 0; i < 16; ++i)
				{
					yield_cpu;
					NMib::NAtomic::fg_CompilerFence(NMib::NAtomic::EMemoryOrder_Relaxed);
				}*/
				
/*				if ((++Count & YieldCount) == 0)
				{
					// sched_yield();
				}*/
				TicketRelease = m_Release.m_Lock.f_Load(NMib::NAtomic::EMemoryOrder_Acquire);
			}
		}
		
		inline_never void f_Unlock()
		{
			m_Release.m_Lock.f_FetchAdd(1, NMib::NAtomic::EMemoryOrder_Release);
		}
	};
	
	static_assert(sizeof(CTicketLockShared) == DMibPMemoryCacheLineSize, "Alignm");
	static_assert(sizeof(CTicketLock) == DMibPMemoryCacheLineSize * 2, "Alignm");
	
	
	template <class t_CAllocator, aint t_bClearHeaps, bint t_bLock = false, aint t_AlignBits = NMib::TCHighestBitSetCorrect<aint, sizeof(void *)>::mc_Value, bint t_bUseFast = EUseFast>
	class CHeapParams : public NMib::NMem::CHeapDefaultParams
	{
	public:
		static_assert((1 << t_AlignBits) >= int(sizeof(void *)), "");
		enum
		{
			EGrowSize = 4*1024*1024
			,ENumChunkFreeThreshold = t_bClearHeaps
			,EAlignBits = t_AlignBits
			,EbOptimizeForSize = 0
			,EBlockCacheSize = (64 + sizeof(mint)) * t_bUseFast
			,ECacheFreeThreshold = 256
			,EFreeSizeBucketTreeThresholdBits = 9
			,ETreeOpt0 = 1
			,ETreeOpt1 = 1
			,ETreeOpt2 = 1
			,ETreeOpt3 = 1
			,EAccurateBucketCache = 0
			,ELargePages = 0
		};
		typedef t_CAllocator CAllocator;
		typedef NMib::NMem::CTCHeap_SizeHolderFast CSizeHolder;
		typedef typename NMib::TCChooseType<t_bLock, CTicketLock, NMib::NThread::CNoLock>::CType CLock;
	};


	template <class t_CAllocator, aint t_bClearHeaps, bint t_bLock = false>
	class CHeapParamsSmall : public CHeapParams<t_CAllocator, t_bClearHeaps, t_bLock>
	{
	public:
		enum
		{
			EbOptimizeForSize = 1
		};
		typedef NMib::NMem::CTCHeap_SizeHolderSmall CSizeHolder;
	};

	typedef NMib::NMem::TCHeap_StandAlone<CHeapParams<NMib::NMem::CAllocator_VirtualNoCommit, 2> > CHeap_StandAlone_Commit0_Clear1_Lock0;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParams<NMib::NMem::CAllocator_VirtualNoCommit, 0> > CHeap_StandAlone_Commit0_Clear0_Lock0;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParams<NMib::NMem::CAllocator_VirtualNoCommit, 2> > CHeap_Combined_Commit0_Clear1_Lock0;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParams<NMib::NMem::CAllocator_VirtualNoCommit, 0> > CHeap_Combined_Commit0_Clear0_Lock0;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParams<NMib::NMem::CAllocator_Virtual, 2> > CHeap_StandAlone_Commit1_Clear1_Lock0;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParams<NMib::NMem::CAllocator_Virtual, 0> > CHeap_StandAlone_Commit1_Clear0_Lock0;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParams<NMib::NMem::CAllocator_Virtual, 2> > CHeap_Combined_Commit1_Clear1_Lock0;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParams<NMib::NMem::CAllocator_Virtual, 0> > CHeap_Combined_Commit1_Clear0_Lock0;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParamsSmall<NMib::NMem::CAllocator_VirtualNoCommit, 2> > CHeapSmall_StandAlone_Commit0_Clear1_Lock0;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParamsSmall<NMib::NMem::CAllocator_VirtualNoCommit, 0> > CHeapSmall_StandAlone_Commit0_Clear0_Lock0;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParamsSmall<NMib::NMem::CAllocator_VirtualNoCommit, 2> > CHeapSmall_Combined_Commit0_Clear1_Lock0;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParamsSmall<NMib::NMem::CAllocator_VirtualNoCommit, 0> > CHeapSmall_Combined_Commit0_Clear0_Lock0;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParamsSmall<NMib::NMem::CAllocator_Virtual, 2> > CHeapSmall_StandAlone_Commit1_Clear1_Lock0;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParamsSmall<NMib::NMem::CAllocator_Virtual, 0> > CHeapSmall_StandAlone_Commit1_Clear0_Lock0;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParamsSmall<NMib::NMem::CAllocator_Virtual, 2> > CHeapSmall_Combined_Commit1_Clear1_Lock0;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParamsSmall<NMib::NMem::CAllocator_Virtual, 0> > CHeapSmall_Combined_Commit1_Clear0_Lock0;

	typedef NMib::NMem::TCHeap_StandAlone<CHeapParams<NMib::NMem::CAllocator_VirtualNoCommit, 2, true> > CHeap_StandAlone_Commit0_Clear1_Lock1;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParams<NMib::NMem::CAllocator_VirtualNoCommit, 0, true> > CHeap_StandAlone_Commit0_Clear0_Lock1;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParams<NMib::NMem::CAllocator_VirtualNoCommit, 2, true> > CHeap_Combined_Commit0_Clear1_Lock1;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParams<NMib::NMem::CAllocator_VirtualNoCommit, 0, true> > CHeap_Combined_Commit0_Clear0_Lock1;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParams<NMib::NMem::CAllocator_Virtual, 2, true> > CHeap_StandAlone_Commit1_Clear1_Lock1;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParams<NMib::NMem::CAllocator_Virtual, 0, true> > CHeap_StandAlone_Commit1_Clear0_Lock1;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParams<NMib::NMem::CAllocator_Virtual, 2, true> > CHeap_Combined_Commit1_Clear1_Lock1;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParams<NMib::NMem::CAllocator_Virtual, 0, true> > CHeap_Combined_Commit1_Clear0_Lock1;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParamsSmall<NMib::NMem::CAllocator_VirtualNoCommit, 2, true> > CHeapSmall_StandAlone_Commit0_Clear1_Lock1;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParamsSmall<NMib::NMem::CAllocator_VirtualNoCommit, 0, true> > CHeapSmall_StandAlone_Commit0_Clear0_Lock1;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParamsSmall<NMib::NMem::CAllocator_VirtualNoCommit, 2, true> > CHeapSmall_Combined_Commit0_Clear1_Lock1;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParamsSmall<NMib::NMem::CAllocator_VirtualNoCommit, 0, true> > CHeapSmall_Combined_Commit0_Clear0_Lock1;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParamsSmall<NMib::NMem::CAllocator_Virtual, 2, true> > CHeapSmall_StandAlone_Commit1_Clear1_Lock1;
	typedef NMib::NMem::TCHeap_StandAlone<CHeapParamsSmall<NMib::NMem::CAllocator_Virtual, 0, true> > CHeapSmall_StandAlone_Commit1_Clear0_Lock1;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParamsSmall<NMib::NMem::CAllocator_Virtual, 2, true> > CHeapSmall_Combined_Commit1_Clear1_Lock1;
	typedef NMib::NMem::TCHeap_Combined<VirtualLimit, CHeapParamsSmall<NMib::NMem::CAllocator_Virtual, 0, true> > CHeapSmall_Combined_Commit1_Clear0_Lock1;

	template <typename t_CHeap>
	class TCMalterlibMemoryImp
	{
	public:

		t_CHeap m_Heap;

		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			if (NMib::NTraits::TCIsSame<typename t_CHeap::CHeapParams::CLock, NMib::NThread::CNoLock>::mc_Value && _nThreads > 1)
				return false;
			return _nThreads <= 3;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			if (NMib::NTraits::TCIsSame<typename t_CHeap::CHeapParams::CLock, NMib::NThread::CNoLock>::mc_Value && _nThreads > 1)
				return false;
			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_Heap.f_Alloc(_Size);
	#		ifdef DebugHeap
				m_Heap.f_CheckHeap(true);
	#		endif
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return m_Heap.f_AllocAligned(_Size, _Alignment);
	#		ifdef DebugHeap
				m_Heap.f_CheckHeap(true);
	#		endif
		}

		inline_small void f_Free(void *_pMem)
		{
			m_Heap.f_Free(_pMem);
	#		ifdef DebugHeap
				m_Heap.f_CheckHeap(true);
	#		endif

		}

		void f_CheckHeap()
		{
			m_Heap.f_CheckHeap(true);
		}

		void f_Clear()
		{
	//		m_Heap.f_Clear();
	#		ifdef DebugHeap
	//			m_Heap.f_CheckHeap(true);
	#		endif
		}

	};
#endif
	


#ifdef DMemoryManagerTestEnable_PtMalloc
	class CMalterlibMemoryPtMalloc
	{
	public:
		CMalterlibMemoryPtMalloc()
		{
		}
		~CMalterlibMemoryPtMalloc()
		{
		}

		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			if (_bAlignment)
				return false;
			return _nThreads <= 4;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			NAllocator_PtMalloc::mallopt(M_TRIM_THRESHOLD, -1);

			if (VirtualLimit < 0)
				NAllocator_PtMalloc::mallopt(M_MMAP_THRESHOLD, NMib::TCLimitsInt<int>::mc_Max);
			else
				NAllocator_PtMalloc::mallopt(M_MMAP_THRESHOLD, VirtualLimit);
			// Do init from one thread
			NAllocator_PtMalloc::free(NAllocator_PtMalloc::malloc(1));
			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return nullptr;
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return NAllocator_PtMalloc::malloc(_Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			NAllocator_PtMalloc::free(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
#endif


#ifdef DMemoryManagerTestEnable_TcMalloc
	class CMalterlibMemoryTcMalloc
	{
	public:
		CMalterlibMemoryTcMalloc()
		{
		}
		~CMalterlibMemoryTcMalloc()
		{
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			if (_bAlignment)
				return false;
			return _nThreads <= 4;
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			tc_free(tc_malloc(1));
			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return nullptr;
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return tc_malloc(_Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			tc_free(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
#endif



#ifdef DMemoryManagerTestEnable_LLAlloc
	class CMalterlibMemoryLLAlloc
	{
	public:
		CMalterlibMemoryLLAlloc()
		{
		}
		~CMalterlibMemoryLLAlloc()
		{
		}

		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			if (_bAlignment)
				return false; // Seems to be a bug in llalloc for aligned blocks
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			llalloc_free(llalloc_malloc(1));
			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return llalloc_memalign(_Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return llalloc_malloc(_Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			llalloc_free(_pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
#endif


#ifdef DMemoryManagerTestEnable_DlMalloc
	class CMalterlibMemoryDlMalloc
	{
		NAllocator_DlMalloc::mspace m_Heap;
	public:

		CMalterlibMemoryDlMalloc()
			: m_Heap(nullptr)
		{
		}
		~CMalterlibMemoryDlMalloc()
		{
			if (m_Heap)
				NAllocator_DlMalloc::destroy_mspace(m_Heap);
		}

		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			return _nThreads == 1;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			if (_nThreads > 1)
				return false;
			m_Heap = NAllocator_DlMalloc::create_mspace(0, false);
			NAllocator_DlMalloc::mspace_mallopt(M_TRIM_THRESHOLD, -1);

			if (VirtualLimit < 0)
				NAllocator_DlMalloc::mspace_mallopt(M_MMAP_THRESHOLD, NMib::TCLimitsInt<int>::mc_Max);
			else
				NAllocator_DlMalloc::mspace_mallopt(M_MMAP_THRESHOLD, VirtualLimit);
			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return NAllocator_DlMalloc::mspace_malloc(m_Heap, _Size);
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return NAllocator_DlMalloc::mspace_memalign(m_Heap, _Size, _Alignment);
		}

		inline_small void f_Free(void *_pMem)
		{
			NAllocator_DlMalloc::mspace_free(m_Heap, _pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};

	class CMalterlibMemoryDlMallocClear
	{
		NAllocator_DlMalloc::mspace m_Heap;
	public:

		CMalterlibMemoryDlMallocClear()
			: m_Heap(nullptr)
		{
		}
		~CMalterlibMemoryDlMallocClear()
		{
			if (m_Heap)
				NAllocator_DlMalloc::destroy_mspace(m_Heap);
		}

		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			return _nThreads == 1;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			if (_nThreads > 1)
				return false;
			m_Heap = NAllocator_DlMalloc::create_mspace(0, false);
			NAllocator_DlMalloc::mspace_mallopt(M_TRIM_THRESHOLD, 4096*1024*2);

			if (VirtualLimit < 0)
				NAllocator_DlMalloc::mspace_mallopt(M_MMAP_THRESHOLD, NMib::TCLimitsInt<int>::mc_Max);
			else
				NAllocator_DlMalloc::mspace_mallopt(M_MMAP_THRESHOLD, VirtualLimit);
			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return NAllocator_DlMalloc::mspace_malloc(m_Heap, _Size);
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return NAllocator_DlMalloc::mspace_memalign(m_Heap, _Size, _Alignment);
		}


		inline_small void f_Free(void *_pMem)
		{
			NAllocator_DlMalloc::mspace_free(m_Heap, _pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_DlMallocMultiThreaded

	class CMalterlibMemoryDlMallocMultiThreaded
	{
		NAllocator_DlMallocMultiThreaded::mspace m_Heap;
	public:
		CMalterlibMemoryDlMallocMultiThreaded()
			: m_Heap(nullptr)
		{
		}
		~CMalterlibMemoryDlMallocMultiThreaded()
		{
			if (m_Heap)
				NAllocator_DlMallocMultiThreaded::destroy_mspace(m_Heap);
		}

		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			return _nThreads <= 3;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			m_Heap = NAllocator_DlMallocMultiThreaded::create_mspace(0, true);
			NAllocator_DlMallocMultiThreaded::mspace_mallopt(M_TRIM_THRESHOLD, -1);

			if (VirtualLimit < 0)
				NAllocator_DlMallocMultiThreaded::mspace_mallopt(M_MMAP_THRESHOLD, NMib::TCLimitsInt<int>::mc_Max);
			else
				NAllocator_DlMallocMultiThreaded::mspace_mallopt(M_MMAP_THRESHOLD, VirtualLimit);
			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return NAllocator_DlMallocMultiThreaded::mspace_memalign(m_Heap, _Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return NAllocator_DlMallocMultiThreaded::mspace_malloc(m_Heap, _Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			NAllocator_DlMallocMultiThreaded::mspace_free(m_Heap, _pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
	class CMalterlibMemoryDlMallocMultiThreadedClear
	{
		NAllocator_DlMallocMultiThreaded::mspace m_Heap;
	public:
		CMalterlibMemoryDlMallocMultiThreadedClear()
			: m_Heap(nullptr)
		{
		}
		~CMalterlibMemoryDlMallocMultiThreadedClear()
		{
			if (m_Heap)
				NAllocator_DlMallocMultiThreaded::destroy_mspace(m_Heap);
		}

		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			return _nThreads <= 3;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			m_Heap = NAllocator_DlMallocMultiThreaded::create_mspace(0, true);
			NAllocator_DlMallocMultiThreaded::mspace_mallopt(M_TRIM_THRESHOLD, 4096*1024*2);

			if (VirtualLimit < 0)
				NAllocator_DlMallocMultiThreaded::mspace_mallopt(M_MMAP_THRESHOLD, NMib::TCLimitsInt<int>::mc_Max);
			else
				NAllocator_DlMallocMultiThreaded::mspace_mallopt(M_MMAP_THRESHOLD, VirtualLimit);

			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return NAllocator_DlMallocMultiThreaded::mspace_memalign(m_Heap, _Size, _Alignment);
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return NAllocator_DlMallocMultiThreaded::mspace_malloc(m_Heap, _Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			NAllocator_DlMallocMultiThreaded::mspace_free(m_Heap, _pMem);
		}

		void f_Clear()
		{

		}
		void f_CheckHeap()
		{
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_OSX
	class CMalterlibMemoryOSX
	{
		malloc_zone_t *m_pMallocZone;
	public:
		CMalterlibMemoryOSX()
			: m_pMallocZone(nullptr)
		{
		}
		
		~CMalterlibMemoryOSX()
		{
			if (m_pMallocZone)
			{
				malloc_destroy_zone(m_pMallocZone);
				m_pMallocZone = nullptr;
			}
		}
		
		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
#ifdef DPlatform_OSX10_5
			if (_bAlignment)
				return false;
#endif
			return true;
		}
		
		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}
		
		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			m_pMallocZone = malloc_create_zone(1024*1024, 0);
			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}
		
		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
#ifndef DPlatform_OSX10_5
			_Alignment = NMib::fg_Max(sizeof(void *), _Alignment);
			return m_pMallocZone->memalign(m_pMallocZone, _Alignment, _Size);
#else
			return nullptr;
#endif
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return m_pMallocZone->malloc(m_pMallocZone, _Size);
		}
		
		inline_small void f_Free(void *_pMem)
		{
			m_pMallocZone->free(m_pMallocZone, _pMem);
		}
		
		void f_Clear()
		{
		}
		void f_CheckHeap()
		{
		}
		NMib::NStr::CStr GetDesc()
		{
			NMib::NStr::CStr Str = "StdLib";
			return Str;
		}
	};
#endif
	
#ifdef DMemoryManagerTestEnable_StdLib
	class CMalterlibMemoryStdLib
	{
		NAllocator_DlMallocMultiThreaded::mspace m_Heap;
	public:
		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			if (_bAlignment)
				return false;
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return nullptr;
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return malloc(_Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			free(_pMem);
		}

		void f_Clear()
		{
		}
		void f_CheckHeap()
		{
		}
		NMib::NStr::CStr GetDesc()
		{
			NMib::NStr::CStr Str = "StdLib";
			return Str;
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_WindowsDefault
	template <bint t_bClear, bint t_bMultiThreaded>
	class TCMalterlibMemoryWindows
	{
	public:
		TCMalterlibMemoryWindows()
			: m_Heap(nullptr)
		{
		}
		~TCMalterlibMemoryWindows()
		{
			if (m_Heap)
				HeapDestroy(m_Heap);
		}
		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			if (_bAlignment)
				return false;
			return _nThreads <= 3;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		HANDLE m_Heap;
		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			if (_nThreads > 1 && !t_bMultiThreaded)
				return false;
			m_Heap = HeapCreate(t_bMultiThreaded ? 0 : HEAP_NO_SERIALIZE, 0, 0);
			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return nullptr;
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return HeapAlloc(m_Heap, 0, _Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			HeapFree(m_Heap, 0, _pMem);
		}

		void f_Clear()
		{
			if (t_bClear)
			{
				if (m_Heap)
					HeapDestroy(m_Heap);
				m_Heap = HeapCreate(t_bMultiThreaded ? 0 : HEAP_NO_SERIALIZE, 0, 0);
			}
		}
		void f_CheckHeap()
		{
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_WindowsLF
	template <bint t_bClear>
	class TCMalterlibMemoryWindowsLF
	{
	public:
		TCMalterlibMemoryWindowsLF()
			: m_Heap(nullptr)
		{
		}
		~TCMalterlibMemoryWindowsLF()
		{
			if (m_Heap)
				HeapDestroy(m_Heap);
		}
		HANDLE m_Heap;

		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			if (_bAlignment)
				return false;
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			f_Create();
			return true;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		void f_Create()
		{
			m_Heap = HeapCreate(0, 0, 0);
			ULONG Enable = 3;
			HeapSetInformation(m_Heap, HeapCompatibilityInformation, &Enable, sizeof(Enable));
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return nullptr;
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return HeapAlloc(m_Heap, 0, _Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			HeapFree(m_Heap, 0, _pMem);
		}

		void f_Clear()
		{
			if (t_bClear)
			{
				if (m_Heap)
					HeapDestroy(m_Heap);
				f_Create();
			}
		}
		void f_CheckHeap()
		{
		}
	};
#endif

#ifdef DMemoryManagerTestEnable_Virtual
	class CMalterlibMemoryVirtual
	{
	public:

		static bint fs_ShouldRun(mint _nThreads, bint _bAlignment)
		{
			if (_bAlignment)
				return false;
			return true;
		}

		void f_SetNumaNode(NMib::ENumaNode _Node)
		{
		}

		bint f_Init(mint _nThreads, mint _MaxSize)
		{
			return _nThreads <= 2;
		}
		void f_InitThread()
		{
		}
		int f_Checkout()
		{
			return 0;
		}

		inline_small void *f_AllocAligned(mint _Size, mint _Alignment)
		{
			return nullptr;
		}

		inline_small void *f_Alloc(mint _Size)
		{
			return NMib::NMem::CAllocator_Virtual::f_Alloc(_Size);
		}

		inline_small void f_Free(void *_pMem)
		{
			NMib::NMem::CAllocator_Virtual::f_Free(_pMem);
		}

		void f_Clear()
		{
		}
		void f_CheckHeap()
		{
		}

	};
#endif

}
