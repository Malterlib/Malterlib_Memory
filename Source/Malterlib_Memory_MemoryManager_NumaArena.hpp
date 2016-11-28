// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once
namespace NMib
{
	namespace NMem
	{
		template <typename t_CParams>
		TCMemoryManagerNumaArena<t_CParams>::TCMemoryManagerNumaArena(ENumaNode _NumaNode, TCMemoryManager<t_CParams> *_pMemoryManager, uint64 _Magic)
			: m_pMemoryManager(_pMemoryManager)
			, m_Heap(_pMemoryManager, this)
			, m_NumaNode(_NumaNode)
			, m_PoolThreadLocal(_NumaNode)
			, m_Pool(_NumaNode)
			, m_Magic(_Magic)
			, m_BackgroundCleanup(this)
			, m_RequestedCleanup(0)
			, m_nArenas(0)
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
				pSlab->~TCMemoryManagerSlabShared<t_CParams>();
				m_pMemoryManager->m_Allocator.f_Free(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);
			}
		}

		template <typename t_CParams>
		TCMemoryManagerArena<t_CParams> *TCMemoryManagerNumaArena<t_CParams>::f_NewArena()
		{
			ENumaNode NumaNode = m_NumaNode;
			auto pArena = m_Pool.f_New(m_pMemoryManager, m_Magic, NumaNode, this);
			return pArena;
		}

		template <typename t_CParams>
		void TCMemoryManagerNumaArena<t_CParams>::f_CanStartThreads()
		{
			m_BackgroundCleanup.f_CanStartThreads();
		}

		template <typename t_CParams>
		void TCMemoryManagerNumaArena<t_CParams>::f_ArenaAvailable(TCMemoryManagerArena<t_CParams> * _pArena)
		{
			m_ArenaAvailableEvent.f_Signal();
		}
		
		template <typename t_CParams>
		void TCMemoryManagerNumaArena<t_CParams>::f_OnNeedCleanup()
		{
			if (t_CParams::mc_bBackgroundCleanup)
				m_BackgroundCleanup.f_OnNeedCleanup();
		}		
		
		template <typename t_CParams>
		int64 TCMemoryManagerNumaArena<t_CParams>::f_GetTimestamp() const
		{
			if (t_CParams::mc_bBackgroundCleanup)
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
			if (t_CParams::mc_bBackgroundCleanup && bNeedCleanup)
				m_BackgroundCleanup.f_OnNeedCleanup();
		}		

		template <typename t_CParams>
		int64 TCMemoryManagerNumaArena<t_CParams>::f_GarbageCollect(int64 _Timestamp, bool _bDecommit, bool _bHasNumaArenasLock, bool _bForceCleanup)
		{
			int64 EarliestTimestamp = TCLimitsInt<int64>::mc_Max;
			
			bool bIncremental = _Timestamp != TCLimitsInt<int64>::mc_Max;
			
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
					int64 NextCleanup = m_Heap.f_GarbageCollect(_Timestamp);
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
						int64 NextCleanup = m_Heap.f_DecommitDeferred(_Timestamp);
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
				f_ProcessArenaMessages(bIncremental, bDeferred, _bHasNumaArenasLock);
				if (bDeferred)
				{
					EarliestTimestamp = fg_Min(EarliestTimestamp, _Timestamp); // Directly request another go
					NewCleanup |= ENumaArenaCleanup_ProcessMessages;
				}
			}

			{
				DMibLock(m_ArenasLock);
				auto fProcessArena = [&](TCMemoryManagerArena<t_CParams> &_Arena) -> bool 
					{
						bool bRemove = false;
						auto pArena = &_Arena;
						if (pArena != ThreadLocal.m_pArena)
						{
							if (bIncremental)
							{
								auto Locked = pArena->m_Locked.f_FetchOr(EArenaLockFlag_Cleanup, NAtomic::EMemoryOrder_Acquire);
								if (Locked != EArenaLockFlag_None)
								{
									EarliestTimestamp = fg_Min(EarliestTimestamp, _Timestamp); // Directly request another go
									return bRemove; // Already locked by some other thread, just leave it be
								}
							}
							else
							{
								DMibUnlock(m_ArenasLock);
								if (_bHasNumaArenasLock)
								{
									DMibUnlock(m_pMemoryManager->m_NumaArenasLock);
									while (pArena->m_Locked.f_FetchOr(EArenaLockFlag_Normal, NAtomic::EMemoryOrder_Acquire) != EArenaLockFlag_None)
										NSys::fg_Thread_SmallestSleep();
								}
								else
								{
									while (pArena->m_Locked.f_FetchOr(EArenaLockFlag_Normal, NAtomic::EMemoryOrder_Acquire) != EArenaLockFlag_None)
										NSys::fg_Thread_SmallestSleep();
								}
							}
						}
						
						int64 ArenaEarliestTimestamp = TCLimitsInt<int64>::mc_Max;
						
						ArenaEarliestTimestamp = fg_Min(EarliestTimestamp, pArena->f_GarbageCollect(RequestedCleanup, _Timestamp));
						if (_bDecommit)
						{
							ArenaEarliestTimestamp = fg_Min(EarliestTimestamp, pArena->f_DecommitDeferred(_Timestamp));
							
							if (ArenaEarliestTimestamp == TCLimitsInt<int64>::mc_Max)
							{
								// Fully garbage collected, remove from list
								bRemove = true;
								pArena->m_bRequestedCleanup = false;
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
						{
							auto LockResult = pArena->m_Locked.f_Exchange(EArenaLockFlag_None, NAtomic::EMemoryOrder_Release);
							if (LockResult & EArenaLockFlag_Waiting)
								f_ArenaAvailable(pArena);
						}
						return bRemove;
					}
				;
				if (!_bForceCleanup)
				{
					bool bTryAgain = true;
					while (bTryAgain)
					{
						bTryAgain = false;
						for (auto pArena = m_ArenasNeedCleanup.f_GetFirst(); pArena; )
						{
							if (!pArena->m_CleanupLink.f_IsInList())
							{
								// Another thread collected garbage
								bTryAgain = true;
								break;
							}
							auto *pNext = m_ArenasNeedCleanup.fs_GetNext(pArena);
							if (fProcessArena(*pArena))
								pArena->m_CleanupLink.f_Unlink();
							
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
					
					if (pFreeSlab->m_Link0.f_IsAloneInList())
						break; // Always save one free slab
					
					if (pFreeSlab->m_FreeTimestamp > _Timestamp)
					{
						EarliestTimestamp = fg_Min(EarliestTimestamp, pFreeSlab->m_FreeTimestamp);
						bAnotherCleanup = true;
						continue;
					}									

					pFreeSlab->~TCMemoryManagerSlabShared<t_CParams>();
					m_pMemoryManager->m_Allocator.f_Free(pFreeSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);
				}
				
				if (_bDecommit)
				{
					for (auto iSlab = m_FreeSlabsNeedingDecommit.f_GetIterator(); iSlab; )
					{
						auto pSlab = &*iSlab;
						++iSlab;
						
						if (pSlab->m_NeedDecommitTimestamp > _Timestamp)
						{
							EarliestTimestamp = fg_Min(EarliestTimestamp, pSlab->m_NeedDecommitTimestamp);
							bAnotherCleanup = true;
							continue;
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
				EarliestTimestamp = fg_Min(EarliestTimestamp, _Timestamp); // Directly request another go
			
			return EarliestTimestamp;
		}		
		
		template <typename t_CParams>
		bool TCMemoryManagerNumaArena<t_CParams>::f_ProcessArenaMessages(bool _bIncremental, bool & _oDeferred, bool _bHasNumaArenasLock)
		{
			auto &ThreadLocal = *m_pMemoryManager->m_LocalArena;
			DMibFastCheck(ThreadLocal.m_Reentrant);
			
			bool bOneProcessed = false;
			bool bProcessed = true;
			while (bProcessed)
			{
				bProcessed = false;
				DMibLock(m_ArenasLock);
				for (auto iArena = m_Arenas.f_GetIterator(); iArena; ++iArena)
				{
					auto pArena = &*iArena;
					if (pArena != ThreadLocal.m_pArena)
					{
						if (_bIncremental)
						{
							if (pArena->m_Locked.f_FetchOr(EArenaLockFlag_Cleanup, NAtomic::EMemoryOrder_Acquire) != EArenaLockFlag_None)
							{
								_oDeferred = true;
								continue; // Just give up if this arena is already locked by someone else
							}
						}
						else
						{
							DMibUnlock(m_ArenasLock);
							if (_bHasNumaArenasLock)
							{
								DMibUnlock(m_pMemoryManager->m_NumaArenasLock);
								while (pArena->m_Locked.f_FetchOr(EArenaLockFlag_Normal, NAtomic::EMemoryOrder_Acquire) != EArenaLockFlag_None)
									NSys::fg_Thread_SmallestSleep();
							}
							else
							{
								while (pArena->m_Locked.f_FetchOr(EArenaLockFlag_Normal, NAtomic::EMemoryOrder_Acquire) != EArenaLockFlag_None)
									NSys::fg_Thread_SmallestSleep();
							}
						}
					}

					if (pArena->f_ProcessMessages())
					{
						bProcessed = true;
						bOneProcessed = true;
					}
					bool bNeedCleanup = pArena->fp_CheckCleanup();
					
					if (pArena != ThreadLocal.m_pArena)
					{
						auto LockResult = pArena->m_Locked.f_Exchange(EArenaLockFlag_None, NAtomic::EMemoryOrder_Release);
						if (LockResult & EArenaLockFlag_Waiting)
							f_ArenaAvailable(pArena);
						if ((LockResult & EArenaLockFlag_Cleanup) || bNeedCleanup)
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
			, m_pArena(nullptr)
			, m_pPreferredArena(nullptr)
			, m_TemporaryReturnCheckoutCount(0)
		{
		}

		/// This happens when we are recreating the thread local for use in a new numa node
		/// We need to return the checkin and check it out for the new numa node
		template <typename t_CParams>
		TCMemoryManagerThreadLocal<t_CParams>::TCMemoryManagerThreadLocal(TCMemoryManagerThreadLocal && _Other, TCMemoryManagerNumaArena<t_CParams> *_pNumaArena)
			: m_pNumaArena(_pNumaArena)
			, m_pArena(nullptr)
			, m_pPreferredArena(nullptr)
			, m_TemporaryReturnCheckoutCount(_Other.m_TemporaryReturnCheckoutCount)
		{
			if (_Other.m_pArena)
			{
				// The other arena is checked out, we need to checkout a new arena here
				m_pArena = _Other.m_pArena->m_pMemoryManager->fp_CheckoutHelper(*this);
				m_pArena->m_CheckoutCount = _Other.m_pArena->m_CheckoutCount;

				_Other.m_pArena->m_CheckoutCount = 1;
				_Other.m_pArena->f_ReturnCheckout();
				_Other.m_pArena = nullptr;
				_Other.m_pPreferredArena = nullptr;

			}

		}

		template <typename t_CParams>
		TCMemoryManagerThreadLocal<t_CParams>::~TCMemoryManagerThreadLocal()
		{
			DMibFastCheck(!m_Reentrant);
			
			if (m_bOwnArena && m_pPreferredArena)
			{
				mint Owned = m_pPreferredArena->m_pNextArena.f_FetchAnd(~mint(1));
				DMibFastCheck(Owned & 1);
				(void)Owned;
			}
			
			if (m_bLazyCheckout)
			{
				m_bLazyCheckout = false;
				f_ReturnCheckout();
			}
			
			DMibFastCheck(!m_pArena); // No arenas should be checked out here
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
				DMibFastCheck(!m_bOwnArena || !m_pPreferredArena || m_pPreferredArena == pArena);
				m_pPreferredArena = pArena;				
			}
		}		

		template <typename t_CParams>
		void TCMemoryManagerThreadLocal<t_CParams>::f_ReturnCheckoutLight()
		{
			auto pArena = m_pArena;
			DMibFastCheck(pArena);
			pArena->f_ReturnCheckoutLight();
			m_pArena = nullptr;
			DMibFastCheck(!m_bOwnArena || !m_pPreferredArena || m_pPreferredArena == pArena);
			m_pPreferredArena = pArena;				
		}

		template <typename t_CParams>
		void TCMemoryManagerThreadLocal<t_CParams>::f_TemporaryReturn()
		{
			auto pArena = m_pArena;
			DMibFastCheck(pArena);
			DMibFastCheck(m_TemporaryReturnCheckoutCount == 0);
			m_TemporaryReturnCheckoutCount = pArena->m_CheckoutCount;
			pArena->m_CheckoutCount = 1;
			pArena->f_ReturnCheckout();
			m_pArena = nullptr;
			DMibFastCheck(!m_bOwnArena || !m_pPreferredArena || m_pPreferredArena == pArena);
			m_pPreferredArena = pArena;				
			
		}

		template <typename t_CParams>
		void TCMemoryManagerThreadLocal<t_CParams>::f_TemporaryGetBack()
		{
			DMibFastCheck(!m_pArena);
			m_pArena = m_pNumaArena->m_pMemoryManager->fp_CheckoutHelper(*this);
			DMibFastCheck(m_TemporaryReturnCheckoutCount != 0);
			m_pArena->m_CheckoutCount = m_TemporaryReturnCheckoutCount;
			m_TemporaryReturnCheckoutCount = 0;
		}

		template <typename t_CParams>
		void TCMemoryManagerThreadLocal<t_CParams>::f_TakeOwnership()
		{
			DMibFastCheck(m_pArena);
			DMibFastCheck(!(m_pArena->m_pNextArena.f_Load() & 1));
			if ((m_pArena->m_pNextArena.f_FetchOr(1) & 1) == 0)
				m_bOwnArena = true;
		}

		template <typename t_CParams>
		void TCMemoryManagerThreadLocal<t_CParams>::f_RelinquishOwnership()
		{
			DMibFastCheck(m_pArena->m_pNextArena.f_Load() & 1);
			if (m_pArena->m_pNextArena.f_FetchAnd(mint(~mint(1))) & 1)
				m_bOwnArena = false;
		}

		template <typename t_CParams>
		void TCMemoryManagerThreadLocal<t_CParams>::f_GarbageCollectLocalArena(bool _bDecommit)
		{
			m_pArena->fp_GarbageCollectFull();

			if (_bDecommit)
			{
				ENumaArenaCleanup RequestedCleanup = ENumaArenaCleanup_None;
				m_pArena->f_GarbageCollect(RequestedCleanup, 0);
				m_pArena->f_DecommitDeferred(0);
				if (RequestedCleanup)
					m_pNumaArena->f_RequestCleanup(RequestedCleanup);
			}
		}
	}
}
