// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once
namespace NMib::NMemory
{
	template <typename t_CParams>
	TCMemoryManagerNumaArena<t_CParams>::TCMemoryManagerNumaArena(ENumaNode _NumaNode, TCMemoryManager<t_CParams> *_pMemoryManager, uint64 _Magic, bool _bLimitedArenas)
		: m_pMemoryManager(_pMemoryManager)
		, m_Heap(_pMemoryManager, this)
		, m_NumaNode(_NumaNode)
		, m_PoolThreadLocal(_NumaNode)
		, m_Pool(_NumaNode)
		, m_Magic(_Magic)
		, m_BackgroundCleanup(this)
		, m_RequestedCleanup(0)
		, m_Random(NMisc::fg_GetHighEntropyRandomInteger<uint32>(), NMisc::fg_GetHighEntropyRandomInteger<uint32>(), NMisc::fg_GetHighEntropyRandomInteger<uint32>())
		, m_bLimitedArenas(_bLimitedArenas)
	{
	}

	template <typename t_CParams>
	TCMemoryManagerNumaArena<t_CParams>::~TCMemoryManagerNumaArena()
	{
		m_BackgroundCleanup.f_StopThread();

		while (auto pArena = m_Arenas.f_Pop())
			m_Pool.f_Delete(pArena);

		while (auto pSlab = m_FreeSlabs.f_Pop())
		{
			if constexpr (TCMemoryManagerArena<t_CParams>::mc_EnableCallbacks)
				m_pMemoryManager->f_OnCommit(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);

			pSlab->~TCMemoryManagerSlabShared<t_CParams>();

			m_pMemoryManager->m_Allocator.f_Free(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);
		}
	}

	template <typename t_CParams>
	TCMemoryManagerArena<t_CParams> *TCMemoryManagerNumaArena<t_CParams>::f_NewArena()
	{
		TCMemoryManagerArena<t_CParams> *pArena;
		{
			DMibLock(m_FreeArenasLock);
			pArena = m_FreeArenas.f_Pop();
		}

		if (!pArena)
		{
			pArena = m_Pool.f_New(m_pMemoryManager, m_Magic, m_NumaNode, this, m_bLimitedArenas);
			DMibLock(m_ArenasLock);
			m_Arenas.f_Insert(pArena);
		}

		if (pArena->m_Lock.f_TryLockNoSanitize())
			return pArena;

		++pArena->m_LockContended;
		pArena->m_Lock.f_LockNoSanitize();
		--pArena->m_LockContended;

		return pArena;
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_CanStartThreads()
	{
		m_BackgroundCleanup.f_CanStartThreads();
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_ForceStartCleanupThread()
	{
		m_BackgroundCleanup.f_ForceStartThread();
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_OnNeedCleanup()
	{
		if constexpr (t_CParams::mc_bBackgroundCleanup)
			m_BackgroundCleanup.f_OnNeedCleanup();
	}

	template <typename t_CParams>
	int64 TCMemoryManagerNumaArena<t_CParams>::f_GetTimestamp() const
	{
		if constexpr (t_CParams::mc_bBackgroundCleanup)
			return m_BackgroundCleanup.f_GetTimestamp();
		return 0;
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_RequestCleanupWeak(ENumaArenaCleanup _Cleanup)
	{
		if (!(m_RequestedCleanup.f_Load(NAtomic::EMemoryOrder_Relaxed) & _Cleanup))
			f_RequestCleanup(_Cleanup);
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_RequestCleanup(ENumaArenaCleanup _Cleanup)
	{
		bool bNeedCleanup = !m_RequestedCleanup.f_FetchOr(_Cleanup);
		if constexpr (t_CParams::mc_bBackgroundCleanup)
		{
			if (bNeedCleanup)
				m_BackgroundCleanup.f_OnNeedCleanup();
		}
	}

	template <typename t_CParams>
	int64 TCMemoryManagerNumaArena<t_CParams>::f_GarbageCollect(CMemoryManagerGarbageOptions const &_GarbageOptions, bool _bDecommit, bool _bForceCleanup)
	{
		int64 EarliestTimestamp = TCLimitsInt<int64>::mc_Max;

		bool bIncremental = _GarbageOptions.m_Timestamp != TCLimitsInt<int64>::mc_Max;

		auto &ThreadLocal = *m_pMemoryManager->m_LocalArena;
		auto ReentrantScope = ThreadLocal.f_Reentrant();

		ENumaArenaCleanup RequestedCleanup = (ENumaArenaCleanup)m_RequestedCleanup.f_Exchange(0);
		if (_bForceCleanup)
			RequestedCleanup |= ENumaArenaCleanup_HeapGarbage | ENumaArenaCleanup_HeapCommit | ENumaArenaCleanup_ProcessMessages | ENumaArenaCleanup_FreeSlabs;

		ENumaArenaCleanup NewCleanup = ENumaArenaCleanup_None;
		if (RequestedCleanup & (ENumaArenaCleanup_HeapGarbage | ENumaArenaCleanup_HeapCommit))
		{
			DMibLock(m_Heap);
			if (RequestedCleanup & ENumaArenaCleanup_HeapGarbage)
			{
				int64 NextCleanup = m_Heap.f_GarbageCollect(_GarbageOptions.m_Timestamp);
				if (NextCleanup != TCLimitsInt<int64>::mc_Max)
					NewCleanup |= ENumaArenaCleanup_HeapGarbage;
				EarliestTimestamp = fg_Min(EarliestTimestamp, NextCleanup);
			}
			if (RequestedCleanup & ENumaArenaCleanup_HeapCommit)
			{
				if (!_bDecommit)
					NewCleanup |= ENumaArenaCleanup_HeapCommit;
				else
				{
					int64 NextCleanup = m_Heap.f_DecommitDeferred(_GarbageOptions.m_TimestampDecommit);
					if (NextCleanup != TCLimitsInt<int64>::mc_Max)
						NewCleanup |= ENumaArenaCleanup_HeapCommit;
					EarliestTimestamp = fg_Min(EarliestTimestamp, NextCleanup);
				}
			}
		}

		// Make sure other thread frees are processed
		if (RequestedCleanup & ENumaArenaCleanup_ProcessMessages)
		{
			bool bDeferred = false;
			f_ProcessArenaMessages(bIncremental, bDeferred);
			if (bDeferred)
			{
				EarliestTimestamp = fg_Min(EarliestTimestamp, _GarbageOptions.m_Timestamp); // Directly request another go
				NewCleanup |= ENumaArenaCleanup_ProcessMessages;
			}
		}

		{
			auto fProcessArena = [&](TCMemoryManagerArena<t_CParams> &_Arena)
				{
					auto pArena = &_Arena;
					if (pArena != ThreadLocal.m_pArena)
					{
						if (bIncremental)
						{
							if (!pArena->m_Lock.f_TryLockNoSanitize())
							{
								EarliestTimestamp = fg_Min(EarliestTimestamp, _GarbageOptions.m_Timestamp); // Directly request another go
								return; // Already locked by some other thread, just leave it be
							}
						}
						else
							pArena->m_Lock.f_LockNoSanitize();
					}

					int64 ArenaEarliestTimestamp = TCLimitsInt<int64>::mc_Max;

					ArenaEarliestTimestamp = fg_Min(EarliestTimestamp, pArena->f_GarbageCollect(RequestedCleanup, _GarbageOptions.m_Timestamp, &ThreadLocal));

					if (pArena->m_bWantCleanup)
					{
						ArenaEarliestTimestamp = fg_Min(ArenaEarliestTimestamp, _GarbageOptions.m_Timestamp); // Directly request another go
						NewCleanup |= ENumaArenaCleanup_ProcessMessages;
					}

					if (_bDecommit)
					{
						ArenaEarliestTimestamp = fg_Min(ArenaEarliestTimestamp, pArena->f_DecommitDeferred(_GarbageOptions.m_TimestampDecommit));

						if (ArenaEarliestTimestamp == TCLimitsInt<int64>::mc_Max)
						{
							// Fully garbage collected, remove from list
							pArena->m_bRequestedCleanup = false;
							DMibLock(m_ArenasNeedCleanupLock);
							pArena->m_CleanupLink.f_Unlink();
						}
					}
					EarliestTimestamp = fg_Min(EarliestTimestamp, ArenaEarliestTimestamp);

					if (pArena->m_bWantNumaFreeSlabsCleanup)
					{
						pArena->m_bWantNumaFreeSlabsCleanup = false;
						if (!(RequestedCleanup & ENumaArenaCleanup_FreeSlabs))
							RequestedCleanup |= ENumaArenaCleanup_FreeSlabs;
					}

					if (pArena != ThreadLocal.m_pArena)
						pArena->m_Lock.f_UnlockNoSanitize();
				}
			;
			if (!_bForceCleanup)
			{
				bool bTryAgain = true;
				while (bTryAgain)
				{
					bTryAgain = false;
					DMibLock(m_ArenasNeedCleanupLock);
					for (auto pArena = m_ArenasNeedCleanup.f_GetFirst(); pArena; )
					{
						if (!pArena->m_CleanupLink.f_IsInList())
						{
							// Another thread collected garbage
							bTryAgain = true;
							break;
						}
						auto *pNext = m_ArenasNeedCleanup.fs_GetNext(pArena);
						DMibUnlock(m_ArenasNeedCleanupLock);
						fProcessArena(*pArena);
						pArena = pNext;
					}
				}
			}
			else
			{
				for (auto iArena = m_Arenas.f_GetIterator(); iArena; ++iArena)
					fProcessArena(*iArena);
			}
		}

		if (RequestedCleanup & ENumaArenaCleanup_FreeSlabs)
		{
			DMibLock(m_FreeSlabsLock);

			bool bAnotherCleanup = false;
			for (auto iFreeSlab = m_FreeSlabs.f_GetIterator(); iFreeSlab; )
			{
				auto pFreeSlab = &*iFreeSlab;
				++iFreeSlab;

				if (pFreeSlab->m_Link.f_IsAloneInList())
					break; // Always save one free slab

				if (pFreeSlab->m_FreeTimestamp > _GarbageOptions.m_TimestampDecommit)
				{
					EarliestTimestamp = fg_Min(EarliestTimestamp, pFreeSlab->m_FreeTimestamp);
					bAnotherCleanup = true;
					continue;
				}

				if (_GarbageOptions.m_TimestampDecommit != TCLimitsInt<int64>::mc_Max && m_FreeSlabsLock.f_Contended())
				{
					EarliestTimestamp = fg_Min(EarliestTimestamp, pFreeSlab->m_FreeTimestamp);
					bAnotherCleanup = true;
					break;
				}

				if constexpr (TCMemoryManagerArena<t_CParams>::mc_EnableCallbacks)
					m_pMemoryManager->f_OnCommit(pFreeSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);

				pFreeSlab->~TCMemoryManagerSlabShared<t_CParams>();
				m_pMemoryManager->m_Allocator.f_Free(pFreeSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);
			}

			if (_bDecommit)
			{
				for (auto iSlab = m_FreeSlabsNeedingDecommit.f_GetIterator(); iSlab; )
				{
					auto pSlab = &*iSlab;
					++iSlab;

					if (pSlab->m_NeedDecommitTimestamp > _GarbageOptions.m_TimestampDecommit)
					{
						EarliestTimestamp = fg_Min(EarliestTimestamp, pSlab->m_NeedDecommitTimestamp);
						bAnotherCleanup = true;
						continue;
					}

					if (_GarbageOptions.m_TimestampDecommit != TCLimitsInt<int64>::mc_Max && m_FreeSlabsLock.f_Contended())
					{
						EarliestTimestamp = fg_Min(EarliestTimestamp, pSlab->m_NeedDecommitTimestamp);
						bAnotherCleanup = true;
						break;
					}

					pSlab->f_DecommitDeferred();
				}
			}
			else if (!m_FreeSlabsNeedingDecommit.f_IsEmpty())
				bAnotherCleanup = true;

			if (bAnotherCleanup)
				NewCleanup |= ENumaArenaCleanup_FreeSlabs;
		}

		if (m_RequestedCleanup.f_FetchOr(NewCleanup))
			EarliestTimestamp = fg_Min(EarliestTimestamp, _GarbageOptions.m_Timestamp); // Directly request another go

		return EarliestTimestamp;
	}

	template <typename t_CParams>
	bool TCMemoryManagerNumaArena<t_CParams>::f_ProcessArenaMessages(bool _bIncremental, bool &o_Deferred)
	{
		auto &ThreadLocal = *m_pMemoryManager->m_LocalArena;
		DMibFastCheck(ThreadLocal.m_Reentrant);

		bool bOneProcessed = false;
		bool bProcessed = true;
		while (bProcessed)
		{
			bProcessed = false;
			DMibLock(m_ArenasLock);
			for (auto iArena = m_Arenas.f_GetIterator(); iArena;)
			{
				auto pArena = &*iArena;
				++iArena;
				DMibUnlock(m_ArenasLock);
				if (pArena != ThreadLocal.m_pArena)
				{
					if (_bIncremental)
					{
						if (!pArena->m_Lock.f_TryLockNoSanitize())
						{
							o_Deferred = true;
							continue; // Just give up if this arena is already locked by someone else
						}
					}
					else
						pArena->m_Lock.f_LockNoSanitize();
				}

				if (_bIncremental)
				{
					if (pArena->f_ProcessMessagesAbortable(o_Deferred, &ThreadLocal))
					{
						bProcessed = true;
						bOneProcessed = true;
					}
				}
				else
				{
					if (pArena->f_ProcessMessages())
					{
						bProcessed = true;
						bOneProcessed = true;
					}
				}
				bool bNeedCleanup = pArena->fp_CheckCleanup();

				if (pArena != ThreadLocal.m_pArena)
				{
					pArena->m_Lock.f_UnlockNoSanitize();
					if (bNeedCleanup)
						f_OnNeedCleanup();
				}
			}

			if (_bIncremental)
				break;
		}
		return bOneProcessed;
	}

	///
	/// Thread local
	/// ============

	template <typename t_CParams>
	TCMemoryManagerThreadLocal<t_CParams>::TCMemoryManagerThreadLocal(TCMemoryManagerNumaArena<t_CParams> *_pNumaArena)
		: m_pNumaArena(_pNumaArena)
		, m_bLimited(_pNumaArena->m_bLimitedArenas)
	{
		if (m_bLimited)
		{
			m_LimitedRandom = NMisc::CRandomShiftRNG
				(
					NMisc::fg_GetHighEntropyRandomInteger<uint32>()
					, NMisc::fg_GetHighEntropyRandomInteger<uint32>()
					, NMisc::fg_GetHighEntropyRandomInteger<uint32>()
				)
			;
		}

		DMibLock(_pNumaArena->m_RandomLock);
		m_RandomIndex = _pNumaArena->m_Random.template f_GetValue<uint32>();
	}

	/// This happens when we are recreating the thread local for use in a new numa node
	/// We need to return the checkin and check it out for the new numa node
	template <typename t_CParams>
	TCMemoryManagerThreadLocal<t_CParams>::TCMemoryManagerThreadLocal(TCMemoryManagerThreadLocal && _Other, TCMemoryManagerNumaArena<t_CParams> *_pNumaArena)
		: m_pNumaArena(_pNumaArena)
		, m_TemporaryReturnCheckoutCount(_Other.m_TemporaryReturnCheckoutCount)
		, m_bLimited(_pNumaArena->m_bLimitedArenas)
	{
		if (m_bLimited)
		{
			m_LimitedRandom = NMisc::CRandomShiftRNG
				(
					NMisc::fg_GetHighEntropyRandomInteger<uint32>()
					, NMisc::fg_GetHighEntropyRandomInteger<uint32>()
					, NMisc::fg_GetHighEntropyRandomInteger<uint32>()
				)
			;
		}

		if (_Other.m_pArena)
		{
			// The other arena is checked out, we need to checkout a new arena here
			m_pArena = _Other.m_pArena->m_pMemoryManager->fp_CheckoutHelper(*this);
			m_pArena->m_CheckoutCount.f_Store(_Other.m_pArena->m_CheckoutCount.f_Load(NAtomic::EMemoryOrder_Relaxed), NAtomic::EMemoryOrder_Relaxed);

			_Other.m_pArena->m_CheckoutCount.f_Store(1, NAtomic::EMemoryOrder_Relaxed);
			_Other.m_pArena->f_ReturnCheckout();
			_Other.m_pArena = nullptr;
			DMibFastCheck(!_Other.m_bInLightCheckout);
			_Other.m_pPreferredArena = nullptr;
		}

		DMibLock(_pNumaArena->m_RandomLock);
		m_RandomIndex = _pNumaArena->m_Random.template f_GetValue<uint32>();
	}

	template <typename t_CParams>
	TCMemoryManagerThreadLocal<t_CParams>::~TCMemoryManagerThreadLocal()
	{
		DMibFastCheck(!m_Reentrant);

		if (m_bLazyCheckout)
		{
			m_bLazyCheckout = false;
			f_ReturnCheckout();
		}

		DMibFastCheck(!m_pArena); // No arenas should be checked out here

		if (!m_bLimited && m_pPreferredArena)
		{
			DMibLock(m_pNumaArena->m_FreeArenasLock);
			m_pNumaArena->m_FreeArenas.f_Insert(m_pPreferredArena);
		}
	}

	template <typename t_CParams>
	inline_always TCMemoryManagerThreadLocal<t_CParams>::CRentrantScope::CRentrantScope(TCMemoryManagerThreadLocal *_pThreadLocal)
		: m_pThreadLocal(_pThreadLocal)
	{
		++_pThreadLocal->m_Reentrant;
	}

	template <typename t_CParams>
	inline_always TCMemoryManagerThreadLocal<t_CParams>::CRentrantScope::~CRentrantScope()
	{
		DMibFastCheck(m_pThreadLocal->m_Reentrant);
		--m_pThreadLocal->m_Reentrant;
	}

	template <typename t_CParams>
	inline_always TCMemoryManagerThreadLocal<t_CParams>::CRentrantScope::CRentrantScope(CRentrantScope &&_Other)
		: m_pThreadLocal(_Other.m_pThreadLocal)
	{
		++m_pThreadLocal->m_Reentrant;
	}

	template <typename t_CParams>
	inline_always auto TCMemoryManagerThreadLocal<t_CParams>::f_Reentrant() -> CRentrantScope
	{
		return CRentrantScope(this);
	}

	template <typename t_CParams>
	void TCMemoryManagerThreadLocal<t_CParams>::f_ReturnCheckout()
	{
		auto pArena = m_pArena;
		DMibFastCheck(pArena);
		if (pArena->f_ReturnCheckout())
		{
			m_pArena = nullptr;
			DMibFastCheck(!m_bInLightCheckout);
			DMibFastCheck(m_bLimited || !m_pPreferredArena || m_pPreferredArena == pArena);
			m_pPreferredArena = pArena;
		}
	}

	template <typename t_CParams>
	void TCMemoryManagerThreadLocal<t_CParams>::f_ForkedChild()
	{
		auto pArena = m_pArena;
		DMibFastCheck(pArena);
		pArena->f_ForkedChild();
	}

	template <typename t_CParams>
	void TCMemoryManagerThreadLocal<t_CParams>::f_ReturnCheckoutLight()
	{
		auto pArena = m_pArena;
		DMibFastCheck(pArena);
		pArena->f_ReturnCheckoutLight();
		m_pArena = nullptr;
#if DMibEnableSafeCheck > 0
		m_bInLightCheckout = false;
#endif
		DMibFastCheck(m_bLimited || !m_pPreferredArena || m_pPreferredArena == pArena);
		m_pPreferredArena = pArena;
	}

	template <typename t_CParams>
	void TCMemoryManagerThreadLocal<t_CParams>::f_TemporaryReturn()
	{
		auto pArena = m_pArena;
		DMibFastCheck(pArena);
		DMibFastCheck(m_TemporaryReturnCheckoutCount == 0);
		m_TemporaryReturnCheckoutCount = pArena->m_CheckoutCount.f_Load(NAtomic::EMemoryOrder_Relaxed);
		pArena->m_CheckoutCount.f_Store(1, NAtomic::EMemoryOrder_Relaxed);
		pArena->f_ReturnCheckout();
		DMibFastCheck(!m_bInLightCheckout);
		m_pArena = nullptr;
		DMibFastCheck(m_bLimited || !m_pPreferredArena || m_pPreferredArena == pArena);
		m_pPreferredArena = pArena;

	}

	template <typename t_CParams>
	void TCMemoryManagerThreadLocal<t_CParams>::f_TemporaryGetBack()
	{
		DMibFastCheck(!m_pArena);
		m_pArena = m_pNumaArena->m_pMemoryManager->fp_CheckoutHelper(*this);
		DMibFastCheck(m_TemporaryReturnCheckoutCount != 0);
		m_pArena->m_CheckoutCount.f_Store(m_TemporaryReturnCheckoutCount, NAtomic::EMemoryOrder_Relaxed);
		m_TemporaryReturnCheckoutCount = 0;
	}

	template <typename t_CParams>
	void TCMemoryManagerThreadLocal<t_CParams>::f_TakeOwnership()
	{
	}

	template <typename t_CParams>
	void TCMemoryManagerThreadLocal<t_CParams>::f_RelinquishOwnership()
	{
	}

	template <typename t_CParams>
	void TCMemoryManagerThreadLocal<t_CParams>::f_GarbageCollectLocalArena(bool _bDecommit)
	{
		m_pArena->fp_GarbageCollectFull();

		if (_bDecommit)
		{
			ENumaArenaCleanup RequestedCleanup = ENumaArenaCleanup_None;
			m_pArena->f_GarbageCollect(RequestedCleanup, 0, this);
			m_pArena->f_DecommitDeferred(0);
			if (RequestedCleanup)
				m_pNumaArena->f_RequestCleanup(RequestedCleanup);
		}
	}

#if DMibConfig_Memory_Shims_Lightweight
	template <typename t_CParams>
	inline_never void TCMemoryManagerThreadLocal<t_CParams>::f_TrackAlloc(mint _Size)
	{
		DMibFastCheck(m_pLightweightReporter);
		m_pLightweightReporter->f_Alloc(_Size);
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerThreadLocal<t_CParams>::f_TrackFree(mint _Size)
	{
		DMibFastCheck(m_pLightweightReporter);
		m_pLightweightReporter->f_Free(_Size);
	}
#endif
}
