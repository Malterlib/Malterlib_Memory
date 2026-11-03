// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once
namespace NMib::NMemory
{
	template <typename t_CParams>
	TCMemoryManagerNumaArena<t_CParams>::TCMemoryManagerNumaArena(ENumaNode _NumaNode, TCMemoryManager<t_CParams> *_pMemoryManager, uint64 _Magic, bool _bLimitedArenas)
		: mp_pMemoryManager(_pMemoryManager)
		, mp_Heap(_pMemoryManager, this)
		, mp_NumaNode(_NumaNode)
		, mp_PoolThreadLocal(_NumaNode)
		, mp_Pool(_NumaNode)
		, mp_Magic(_Magic)
		, mp_BackgroundCleanup(this)
		, mp_RequestedCleanup(0)
		, mp_Random(NMisc::fg_GetHighEntropyRandomInteger<uint32>(), NMisc::fg_GetHighEntropyRandomInteger<uint32>(), NMisc::fg_GetHighEntropyRandomInteger<uint32>())
		, mp_bLimitedArenas(_bLimitedArenas)
	{
	}

	template <typename t_CParams>
	TCMemoryManagerNumaArena<t_CParams>::~TCMemoryManagerNumaArena()
	{
		mp_BackgroundCleanup.f_StopThread();

		while (auto pArena = mp_Arenas.f_Pop())
			mp_Pool.f_Delete(pArena);

		while (auto pSlab = mp_FreeSlabs.f_Pop())
		{
			if constexpr (TCMemoryManagerArena<t_CParams>::mc_bEnableCallbacks)
				mp_pMemoryManager->f_OnCommit(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);

			pSlab->~TCMemoryManagerSlabShared<t_CParams>();

			mp_pMemoryManager->mp_Allocator.f_Free(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);
		}
	}

	template <typename t_CParams>
	TCMemoryManagerArena<t_CParams> *TCMemoryManagerNumaArena<t_CParams>::f_NewArena()
	{
		TCMemoryManagerArena<t_CParams> *pArena;
		{
			DMibLock(mp_FreeArenasLock);
			pArena = mp_FreeArenas.f_Pop();
		}

		if (!pArena)
		{
			pArena = mp_Pool.f_New(mp_pMemoryManager, mp_Magic, mp_NumaNode, this, mp_bLimitedArenas);
			DMibLock(mp_ArenasLock);
			mp_Arenas.f_Insert(pArena);
		}

		if (pArena->mp_LockState.m_Lock.f_TryLockNoSanitize())
			return pArena;

		++pArena->mp_LockState.m_LockContended;
		pArena->mp_LockState.m_Lock.f_LockNoSanitize();
		--pArena->mp_LockState.m_LockContended;

		return pArena;
	}

	// Call with mp_LimitedArenasCreateLock held and before the slab and list locks, which arena operations take while holding an arena
	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_LockLimitedArenas(TCMemoryManagerArena<t_CParams> const *_pCheckedOut)
	{
		for (auto &LimitedArena : mp_LimitedArenas)
		{
			auto *pArena = LimitedArena.f_Load(NAtomic::gc_MemoryOrder_Relaxed);
			if (!pArena || pArena == _pCheckedOut)
				continue;

			if (pArena->mp_LockState.m_Lock.f_TryLockNoSanitize())
				continue;

			++pArena->mp_LockState.m_LockContended;
			pArena->mp_LockState.m_Lock.f_LockNoSanitize();
			--pArena->mp_LockState.m_LockContended;
		}
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_UnlockLimitedArenas(TCMemoryManagerArena<t_CParams> const *_pCheckedOut)
	{
		for (auto &LimitedArena : mp_LimitedArenas)
		{
			auto *pArena = LimitedArena.f_Load(NAtomic::gc_MemoryOrder_Relaxed);
			if (pArena && pArena != _pCheckedOut)
				pArena->mp_LockState.m_Lock.f_UnlockNoSanitize();
		}
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_ForkedChildLimitedArenas(TCMemoryManagerArena<t_CParams> const *_pCheckedOut)
	{
		for (auto &LimitedArena : mp_LimitedArenas)
		{
			auto *pArena = LimitedArena.f_Load(NAtomic::gc_MemoryOrder_Relaxed);
			if (pArena && pArena != _pCheckedOut)
				pArena->f_ForkedChild();
		}
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_CanStartThreads()
	{
		mp_BackgroundCleanup.f_CanStartThreads();
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_ForceStartCleanupThread()
	{
		mp_BackgroundCleanup.f_ForceStartThread();
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_OnNeedCleanup()
	{
		if constexpr (t_CParams::mc_bBackgroundCleanup)
			mp_BackgroundCleanup.f_OnNeedCleanup();
	}

	template <typename t_CParams>
	int64 TCMemoryManagerNumaArena<t_CParams>::fs_GetTimestamp()
	{
		if constexpr (t_CParams::mc_bBackgroundCleanup)
			return TCMemoryManagerNumaArenaBackgroundCleanup<t_CParams>::fs_GetTimestamp();
		return 0;
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_RequestCleanupWeak(ENumaArenaCleanup _Cleanup)
	{
		if (!(mp_RequestedCleanup.f_Load(NAtomic::gc_MemoryOrder_Relaxed) & _Cleanup))
			f_RequestCleanup(_Cleanup);
	}

	template <typename t_CParams>
	void TCMemoryManagerNumaArena<t_CParams>::f_RequestCleanup(ENumaArenaCleanup _Cleanup)
	{
		bool bNeedCleanup = !mp_RequestedCleanup.f_FetchOr(_Cleanup);
		if constexpr (t_CParams::mc_bBackgroundCleanup)
		{
			if (bNeedCleanup)
				mp_BackgroundCleanup.f_OnNeedCleanup();
		}
	}

	template <typename t_CParams>
	int64 TCMemoryManagerNumaArena<t_CParams>::f_GarbageCollect(CMemoryManagerGarbageOptions const &_GarbageOptions, bool _bDecommit, bool _bForceCleanup)
	{
		int64 EarliestTimestamp = TCLimitsInt<int64>::mc_Max;

		bool bIncremental = _GarbageOptions.m_Timestamp != (TCLimitsInt<int64>::mc_Max - 1);

		auto &ThreadLocal = *mp_pMemoryManager->mp_LocalArena;

		auto bOldForcing = ThreadLocal.m_bForcingCleanup;
		ThreadLocal.m_bForcingCleanup = _bForceCleanup;
		auto Cleanup = g_OnScopeExit / [&]
			{
				ThreadLocal.m_bForcingCleanup = bOldForcing;
			}
		;

		auto ReentrantScope = ThreadLocal.f_Reentrant();

		ENumaArenaCleanup RequestedCleanup = (ENumaArenaCleanup)mp_RequestedCleanup.f_Exchange(0);
		if (_bForceCleanup)
			RequestedCleanup |= ENumaArenaCleanup_HeapGarbage | ENumaArenaCleanup_HeapCommit | ENumaArenaCleanup_ProcessMessages | ENumaArenaCleanup_FreeSlabs;

		ENumaArenaCleanup NewCleanup = ENumaArenaCleanup_None;
		if (RequestedCleanup & (ENumaArenaCleanup_HeapGarbage | ENumaArenaCleanup_HeapCommit))
		{
			DMibLock(mp_Heap);
			if (RequestedCleanup & ENumaArenaCleanup_HeapGarbage)
			{
				int64 NextCleanup = mp_Heap.f_GarbageCollect(_GarbageOptions.m_Timestamp);
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
					int64 NextCleanup = mp_Heap.f_DecommitDeferred(_GarbageOptions.m_TimestampDecommit);
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
							if (!pArena->mp_LockState.m_Lock.f_TryLockNoSanitize())
							{
								EarliestTimestamp = fg_Min(EarliestTimestamp, _GarbageOptions.m_Timestamp); // Directly request another go
								return; // Already locked by some other thread, just leave it be
							}
						}
						else
							pArena->mp_LockState.m_Lock.f_LockNoSanitize();
					}

					int64 ArenaEarliestTimestamp = TCLimitsInt<int64>::mc_Max;

					ArenaEarliestTimestamp = fg_Min(EarliestTimestamp, pArena->f_GarbageCollect(RequestedCleanup, _GarbageOptions.m_Timestamp, &ThreadLocal));

					if (pArena->mp_bWantCleanup)
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
							pArena->mp_bRequestedCleanup = false;
							DMibLock(mp_ArenasNeedCleanupLock);
							pArena->m_CleanupLink.f_Unlink();
						}
					}
					EarliestTimestamp = fg_Min(EarliestTimestamp, ArenaEarliestTimestamp);

					if (pArena->mp_bWantNumaFreeSlabsCleanup)
					{
						pArena->mp_bWantNumaFreeSlabsCleanup = false;
						if (!(RequestedCleanup & ENumaArenaCleanup_FreeSlabs))
							RequestedCleanup |= ENumaArenaCleanup_FreeSlabs;
					}

					if (pArena != ThreadLocal.m_pArena)
						pArena->mp_LockState.m_Lock.f_UnlockNoSanitize();
				}
			;
			if (!_bForceCleanup)
			{
				bool bTryAgain = true;
				while (bTryAgain)
				{
					bTryAgain = false;
					DMibLock(mp_ArenasNeedCleanupLock);
					for (auto pArena = mp_ArenasNeedCleanup.f_GetFirst(); pArena;)
					{
						if (!pArena->m_CleanupLink.f_IsInList())
						{
							// Another thread collected garbage
							bTryAgain = true;
							break;
						}
						auto *pNext = mp_ArenasNeedCleanup.fs_GetNext(pArena);
						DMibUnlock(mp_ArenasNeedCleanupLock);
						fProcessArena(*pArena);
						pArena = pNext;
					}
				}
			}
			else
			{
				for (auto iArena = mp_Arenas.f_GetIterator(); iArena; ++iArena)
					fProcessArena(*iArena);
			}
		}

		if (RequestedCleanup & ENumaArenaCleanup_FreeSlabs)
		{
			DMibLock(mp_FreeSlabsLock);

			bool bAnotherCleanup = false;
			for (auto iFreeSlab = mp_FreeSlabs.f_GetIterator(); iFreeSlab;)
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

				if (_GarbageOptions.m_TimestampDecommit != (TCLimitsInt<int64>::mc_Max - 1) && mp_FreeSlabsLock.f_Contended())
				{
					EarliestTimestamp = fg_Min(EarliestTimestamp, pFreeSlab->m_FreeTimestamp);
					bAnotherCleanup = true;
					break;
				}

				if constexpr (TCMemoryManagerArena<t_CParams>::mc_bEnableCallbacks)
					mp_pMemoryManager->f_OnCommit(pFreeSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);

				pFreeSlab->~TCMemoryManagerSlabShared<t_CParams>();
				mp_pMemoryManager->mp_Allocator.f_Free(pFreeSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);
			}

			if (_bDecommit)
			{
				for (auto iSlab = mp_FreeSlabsNeedingDecommit.f_GetIterator(); iSlab;)
				{
					auto pSlab = &*iSlab;
					++iSlab;

					if (pSlab->m_NeedDecommitTimestamp > _GarbageOptions.m_TimestampDecommit)
					{
						EarliestTimestamp = fg_Min(EarliestTimestamp, pSlab->m_NeedDecommitTimestamp);
						bAnotherCleanup = true;
						continue;
					}

					if (_GarbageOptions.m_TimestampDecommit != (TCLimitsInt<int64>::mc_Max - 1) && mp_FreeSlabsLock.f_Contended())
					{
						EarliestTimestamp = fg_Min(EarliestTimestamp, pSlab->m_NeedDecommitTimestamp);
						bAnotherCleanup = true;
						break;
					}

					pSlab->f_DecommitDeferred();
				}
			}
			else if (!mp_FreeSlabsNeedingDecommit.f_IsEmpty())
				bAnotherCleanup = true;

			if (bAnotherCleanup)
				NewCleanup |= ENumaArenaCleanup_FreeSlabs;
		}

		if (mp_RequestedCleanup.f_FetchOr(NewCleanup))
			EarliestTimestamp = fg_Min(EarliestTimestamp, _GarbageOptions.m_Timestamp); // Directly request another go

		return EarliestTimestamp;
	}

	template <typename t_CParams>
	bool TCMemoryManagerNumaArena<t_CParams>::f_ProcessArenaMessages(bool _bIncremental, bool &o_bDeferred)
	{
		auto &ThreadLocal = *mp_pMemoryManager->mp_LocalArena;
		DMibFastCheck(ThreadLocal.m_Reentrant);

		bool bOneProcessed = false;
		bool bProcessed = true;
		while (bProcessed)
		{
			bProcessed = false;
			DMibLock(mp_ArenasLock);
			for (auto iArena = mp_Arenas.f_GetIterator(); iArena;)
			{
				auto pArena = &*iArena;
				++iArena;
				DMibUnlock(mp_ArenasLock);
				if (pArena != ThreadLocal.m_pArena)
				{
					if (_bIncremental)
					{
						if (!pArena->mp_LockState.m_Lock.f_TryLockNoSanitize())
						{
							o_bDeferred = true;
							continue; // Just give up if this arena is already locked by someone else
						}
					}
					else
						pArena->mp_LockState.m_Lock.f_LockNoSanitize();
				}

				if (_bIncremental)
				{
					if (pArena->f_ProcessMessagesAbortable(o_bDeferred, &ThreadLocal))
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
					pArena->mp_LockState.m_Lock.f_UnlockNoSanitize();
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
		, m_bLimited(_pNumaArena->mp_bLimitedArenas)
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

		DMibLock(_pNumaArena->mp_RandomLock);
		m_RandomIndex = _pNumaArena->mp_Random.template f_GetValue<uint32>();
	}

	/// This happens when we are recreating the thread local for use in a new numa node
	/// We need to return the checkin and check it out for the new numa node
	template <typename t_CParams>
	TCMemoryManagerThreadLocal<t_CParams>::TCMemoryManagerThreadLocal(TCMemoryManagerThreadLocal &&_Other, TCMemoryManagerNumaArena<t_CParams> *_pNumaArena)
		: m_pNumaArena(_pNumaArena)
		, m_TemporaryReturnCheckoutCount(_Other.m_TemporaryReturnCheckoutCount)
		, m_bLimited(_pNumaArena->mp_bLimitedArenas)
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
			m_pArena = _Other.m_pArena->mp_pMemoryManager->fp_CheckoutHelper(*this);
			m_pArena->mp_LockState.m_CheckoutCount.f_Store(_Other.m_pArena->mp_LockState.m_CheckoutCount.f_Load(NAtomic::gc_MemoryOrder_Relaxed), NAtomic::gc_MemoryOrder_Relaxed);

			_Other.m_pArena->mp_LockState.m_CheckoutCount.f_Store(1, NAtomic::gc_MemoryOrder_Relaxed);
			_Other.m_pArena->f_ReturnCheckout();
			_Other.m_pArena = nullptr;
			DMibFastCheck(!_Other.m_bInLightCheckout);
			_Other.m_pPreferredArena = nullptr;
		}

		DMibLock(_pNumaArena->mp_RandomLock);
		m_RandomIndex = _pNumaArena->mp_Random.template f_GetValue<uint32>();
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
			DMibLock(m_pNumaArena->mp_FreeArenasLock);
			m_pNumaArena->mp_FreeArenas.f_Insert(m_pPreferredArena);
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
		m_TemporaryReturnCheckoutCount = pArena->mp_LockState.m_CheckoutCount.f_Load(NAtomic::gc_MemoryOrder_Relaxed);
		pArena->mp_LockState.m_CheckoutCount.f_Store(1, NAtomic::gc_MemoryOrder_Relaxed);
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
		m_pArena = m_pNumaArena->mp_pMemoryManager->fp_CheckoutHelper(*this);
		DMibFastCheck(m_TemporaryReturnCheckoutCount != 0);
		m_pArena->mp_LockState.m_CheckoutCount.f_Store(m_TemporaryReturnCheckoutCount, NAtomic::gc_MemoryOrder_Relaxed);
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
	bool TCMemoryManagerThreadLocal<t_CParams>::f_GarbageCollectLocalArenaIfPending()
	{
		if (!m_pArena->fp_HasPendingMessages())
			return false;

		// Consolidating fully free sub-slabs into contiguous free-list runs restores the
		// sequential allocation pattern of a fresh sub-slab, which is where the throughput of
		// the owner-side collect comes from. Sub-slab release is not needed for that, so most
		// collects skip it, but the owner must still release periodically: the background
		// cleanup thread rarely wins the arena lock against a busy thread, and memory creeps
		// if release is left to it alone.
		if ((++m_nPendingGarbageCollects & (DMibConfig_Memory_EagerOwnerGC ? 127 : 15)) == 0)
			m_pArena->fp_GarbageCollectFull(true);
		else
			m_pArena->fp_GarbageCollectConsolidate();
		return true;
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

	template <typename t_CParams>
	TCMemoryManagerLimitedTemporaryReturn<t_CParams>::TCMemoryManagerLimitedTemporaryReturn(TCMemoryManagerLimitedTemporaryReturn &&_Other)
		: mp_pThreadLocal(fg_Exchange(_Other.mp_pThreadLocal, nullptr))
	{
	}

	template <typename t_CParams>
	TCMemoryManagerLimitedTemporaryReturn<t_CParams>::TCMemoryManagerLimitedTemporaryReturn(TCMemoryManager<t_CParams> &_Manager)
		: TCMemoryManagerLimitedTemporaryReturn(_Manager.mp_LocalArena.f_TryGet())
	{
	}

	template <typename t_CParams>
	TCMemoryManagerLimitedTemporaryReturn<t_CParams>::TCMemoryManagerLimitedTemporaryReturn(TCMemoryManagerThreadLocal<t_CParams> *_pThreadLocal)
	{
		if (_pThreadLocal && _pThreadLocal->m_pArena && _pThreadLocal->m_pNumaArena->mp_bLimitedArenas)
		{
			_pThreadLocal->f_TemporaryReturn();
			mp_pThreadLocal = _pThreadLocal;
		}
	}

	template <typename t_CParams>
	TCMemoryManagerLimitedTemporaryReturn<t_CParams>::~TCMemoryManagerLimitedTemporaryReturn()
	{
		if (mp_pThreadLocal)
			mp_pThreadLocal->f_TemporaryGetBack();
	}

	template <typename t_CParams>
	TCMemoryManagerLimitedTemporaryReturn<t_CParams> &TCMemoryManagerLimitedTemporaryReturn<t_CParams>::operator = (TCMemoryManagerLimitedTemporaryReturn<t_CParams> &&_Other)
	{
		mp_pThreadLocal = fg_Exchange(_Other.mp_pThreadLocal, nullptr);
		return *this;
	}

#if DMibConfig_Memory_Shims_Lightweight
	template <typename t_CParams>
	inline_never void TCMemoryManagerThreadLocal<t_CParams>::f_TrackAlloc(umint _Size)
	{
		DMibFastCheck(m_pLightweightReporter);
		m_pLightweightReporter->f_Alloc(_Size);
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerThreadLocal<t_CParams>::f_TrackFree(umint _Size)
	{
		DMibFastCheck(m_pLightweightReporter);
		m_pLightweightReporter->f_Free(_Size);
	}
#endif
}
