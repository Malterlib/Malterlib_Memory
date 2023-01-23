// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include "Malterlib_Memory_MemoryManager_BackgroundCleanup.h"

namespace NMib::NMemory
{
	template <typename t_CParams>
	struct TCMemoryManager;

	template <typename t_CParams>
	struct TCMemoryManagerArena;

	template <typename t_CParams, uint32 t_SlabType>
	struct TCMemoryManagerSlab;

	template <typename t_CParams>
	class TCMemoryManagerArenaHeap;

	template <typename t_CParams>
	struct TCMemoryManagerLimitedTemporaryReturn
	{
		TCMemoryManagerLimitedTemporaryReturn(TCMemoryManagerLimitedTemporaryReturn const &_Other) = delete;
		TCMemoryManagerLimitedTemporaryReturn &operator = (TCMemoryManagerLimitedTemporaryReturn const &_Other) = delete;
		TCMemoryManagerLimitedTemporaryReturn(TCMemoryManagerLimitedTemporaryReturn &&_Other);
		TCMemoryManagerLimitedTemporaryReturn(TCMemoryManagerThreadLocal<t_CParams> *_pThreadLocal);
		TCMemoryManagerLimitedTemporaryReturn(TCMemoryManager<t_CParams> &_Manager);
		TCMemoryManagerLimitedTemporaryReturn &operator = (TCMemoryManagerLimitedTemporaryReturn &&_Other);
		~TCMemoryManagerLimitedTemporaryReturn();

	private:
		TCMemoryManagerThreadLocal<t_CParams> *mp_pThreadLocal = nullptr;
	};

	template <typename t_CParams>
	struct align_cacheline TCMemoryManagerThreadLocal
	{
		TCMemoryManagerArena<t_CParams> *m_pArena = nullptr;
		mint m_Reentrant = 0;
		TCMemoryManagerArena<t_CParams> *m_pPreferredArena = nullptr;
		mint m_TemporaryReturnCheckoutCount = 0;
		TCMemoryManagerNumaArena<t_CParams> *m_pNumaArena;
		bool m_bLazyCheckout = false;
		bool m_bLimited = false;
#if DMibEnableSafeCheck > 0
		bool m_bInLightCheckout = false;
#endif
#if DMibConfig_Memory_Shims_Lightweight
		CReportMemoryLightweight *m_pLightweightReporter = nullptr;
		EMemoryReportLightweightScopeFlag m_LightweightScopeFlags = EMemoryReportLightweightScopeFlag_None;
#endif
#if DMibConfig_Memory_CustomThreadLocal
		TCAutoClear<void *> m_pCustom[DMibConfig_Memory_CustomThreadLocal];
#endif
		uint32 m_RandomIndex = 0;
		NMisc::CRandomShiftRNG m_LimitedRandom;

		struct CRentrantScope
		{
			CRentrantScope(TCMemoryManagerThreadLocal *_pThreadLocal);
			~CRentrantScope();
			CRentrantScope(CRentrantScope &&_Other);

			CRentrantScope(CRentrantScope const &_Other) = delete;
			CRentrantScope &operator =(CRentrantScope &&_Other) = delete;
			CRentrantScope &operator =(CRentrantScope const &_Other) = delete;

			TCMemoryManagerThreadLocal *m_pThreadLocal;
		};

	public:
		TCMemoryManagerThreadLocal(TCMemoryManagerThreadLocal const &_Other) = delete;
		TCMemoryManagerThreadLocal(TCMemoryManagerThreadLocal &&_Other, TCMemoryManagerNumaArena<t_CParams> *_pNumaArena);
		TCMemoryManagerThreadLocal(TCMemoryManagerNumaArena<t_CParams> *_pNumaArena);
		~TCMemoryManagerThreadLocal();

		CRentrantScope f_Reentrant();

		void f_ForkedChild();
		void f_ReturnCheckout();
		void f_ReturnCheckoutLight();
		void f_TemporaryReturn();
		void f_TemporaryGetBack();
		void f_TakeOwnership();
		void f_RelinquishOwnership();
		void f_GarbageCollectLocalArena(bool _bDecommit);

#if DMibConfig_Memory_Shims_Lightweight
		inline_never void f_TrackAlloc(mint _Size);
		inline_never void f_TrackFree(mint _Size);
#endif
	};

	template <typename t_CParams>
	struct align_cacheline TCMemoryManagerNumaArena
	{
	public:
		struct CCompare
		{
			inline_small ENumaNode operator () (TCMemoryManagerNumaArena const &_Node) const
			{
				return _Node.m_NumaNode;
			}
		};

		NIntrusive::TCAVLLink<> m_Link;

	public:
		TCMemoryManagerNumaArena(ENumaNode _NumaNode, TCMemoryManager<t_CParams> *_pMemoryManager, uint64 _Magic, bool _bLimitedArenas);
		~TCMemoryManagerNumaArena();

		TCMemoryManagerArena<t_CParams> *f_NewArena();

		void f_OnNeedCleanup();

		int64 f_GetTimestamp() const;

		void f_RequestCleanup(ENumaArenaCleanup _Cleanup);
		void f_RequestCleanupWeak(ENumaArenaCleanup _Cleanup);

		int64 f_GarbageCollect(CMemoryManagerGarbageOptions const &_GarbageOptions, bool _bDecommit, bool _bForceCleanup);

		bool f_ProcessArenaMessages(bool _bIncremental, bool &o_Deferred);

		void f_CanStartThreads();
		void f_ForceStartCleanupThread();

	private:
		template <typename t_CParams2>
		friend struct TCMemoryManager;

		template <typename t_CParams2>
		friend struct TCMemoryManagerArena;

		template <typename t_CParams2, uint32 t_SlabType2>
		friend struct TCMemoryManagerSlab;

		template <typename t_CParams2>
		friend class TCMemoryManagerArenaHeap;

		template <typename t_CParams2>
		friend struct TCMemoryManagerNumaArenaBackgroundCleanup;

		template <typename t_CParams2>
		friend struct TCMemoryManagerThreadLocal;

		template <typename t_CParams2>
		friend struct TCMemoryManagerLimitedTemporaryReturn;

	private:
		TCPool<TCMemoryManagerArena<t_CParams>, 16, NThread::CLowLevelLock, NMemory::CPoolType_Freeable, typename t_CParams::CAllocator> m_Pool;
		TCPool<TCMemoryManagerThreadLocal<t_CParams>, 128, NThread::CLowLevelLock, NMemory::CPoolType_Freeable, typename t_CParams::CAllocator> m_PoolThreadLocal;
		TCMemoryManager<t_CParams> *m_pMemoryManager;

		TCMemoryManagerArenaHeap<t_CParams> m_Heap;

		align_cacheline NMib::NThread::CLowLevelLock m_ArenasLock;
		DMibListLinkDS_List_FromTemplate(TCMemoryManagerArena<t_CParams>, m_NumaArenaLink) m_Arenas;

		align_cacheline NMib::NThread::CLowLevelLock m_FreeArenasLock;
		DMibListLinkDS_List_FromTemplate(TCMemoryManagerArena<t_CParams>, m_FreeArenasLink) m_FreeArenas;

		align_cacheline NMib::NThread::CLowLevelLock m_ArenasNeedCleanupLock;
		DMibListLinkDS_List_FromTemplate(TCMemoryManagerArena<t_CParams>, m_CleanupLink) m_ArenasNeedCleanup;

		align_cacheline NMib::NThread::CLowLevelLock m_LimitedArenasCreateLock;
		NAtomic::TCAtomic<TCMemoryManagerArena<t_CParams> *> m_LimitedArenas[t_CParams::mc_MaxArenas] = {};

		align_cacheline NMib::NThread::CLowLevelContendedLock m_FreeSlabsLock;
		DMibListLinkDS_List_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_Link) m_FreeSlabs;
		DMibListLinkDS_List_FromTemplate(TCMemoryManagerSlabShared<t_CParams>, m_LinkNeedDecommit) m_FreeSlabsNeedingDecommit;

		TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams> m_BackgroundCleanup;

		NThread::CLowLevelLock m_RandomLock;
		NMisc::CRandomShiftRNG m_Random;

		uint64 m_Magic;
		ENumaNode m_NumaNode;
		bool m_bLimitedArenas;

		align_cacheline NAtomic::TCAtomic<uint32> m_RequestedCleanup;
	};
}
