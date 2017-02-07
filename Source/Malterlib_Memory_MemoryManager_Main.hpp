// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{
		template <typename t_CParams>
		template <typename... tfp_CAllocator>
		TCMemoryManager<t_CParams>::TCMemoryManager(CMemoryManagerConfig const &_Config, tfp_CAllocator &&..._Params)
			: t_CParams::CNotifier::CGlobal(*this)
			, m_Magic(_Config.m_Magic)
			, m_nMaxArenas(_Config.m_nMaxArenas)
			, m_nArenas(0)
			, m_HeapChunks(NMem::CAllocatorConstructTag(), this)
			, m_Allocator(fg_Forward<tfp_CAllocator>(_Params)...)
			, m_LocalArena
			(
				[this] (TCMemoryManagerThreadLocal<t_CParams> *_pParentArena, bool _bMove) -> TCMemoryManagerThreadLocal<t_CParams> *
				{
					ENumaNode NumaNode = ENumaNode_Default;
					if (m_LocalNumaNode.f_IsValid()) // Don't init if we don't need to
						NumaNode = (*m_LocalNumaNode).m_Node;
					else if (_pParentArena)
						NumaNode = _pParentArena->m_pNumaArena->m_NumaNode;
					TCMemoryManagerNumaArena<t_CParams> *pNumaArena;
					{
						DMibLock(m_NumaArenasLock);
						pNumaArena = m_NumaArenas.f_FindEqual(NumaNode);
						if (!pNumaArena)
						{
							mint Size = sizeof(TCMemoryManagerNumaArena<t_CParams>);
							pNumaArena = 
								new(m_Allocator.f_Alloc(Size, EAllocationFlag_WillFreeWithSize | t_CParams::mc_AllocationFlags, NumaNode)) 
								TCMemoryManagerNumaArena<t_CParams>(NumaNode, this, m_Magic)
							;
							m_NumaArenas.f_Insert(pNumaArena);
						}
					}

					if (_bMove)
					{
						auto pArena = pNumaArena->m_PoolThreadLocal.f_New(fg_Move(*_pParentArena), pNumaArena);
						return pArena;
					}
					else
					{
						auto pArena = pNumaArena->m_PoolThreadLocal.f_New(pNumaArena);
						return pArena;
					}
				}	
				, [this] (TCMemoryManagerThreadLocal<t_CParams> *_pArena)
				{
					TCMemoryManagerNumaArena<t_CParams> *pNumaArena = _pArena->m_pNumaArena;
					pNumaArena->m_PoolThreadLocal.f_Delete(_pArena);
				}
			)
			, m_LocalNumaNode
			(
				[this] (CLocalNumaNode *_pParent, bool _bMove) -> CLocalNumaNode *
				{
					if (_pParent)
						return this->m_LocalNumaNodePool.f_New(*_pParent);
					else
						return this->m_LocalNumaNodePool.f_New();
				}	
				, [this] (CLocalNumaNode *_pData)
				{
					this->m_LocalNumaNodePool.f_Delete(_pData);
				}
			)
		{
			ICMemoryManagerReturnCheckout::m_Version = ECMemoryManagerReturnCheckoutVersion;
		}
		
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_SetMaxArenas(mint _nArenas)
		{
			DMibLock(m_nArenasLock);
			m_nMaxArenas = _nArenas;
		}

		template <typename t_CParams>
		TCMemoryManager<t_CParams>::~TCMemoryManager()
		{
			// Remove all big blocks
			m_HeapChunks.f_Clear();
			
			for (auto &Arena : m_NumaArenas)
			{
				Arena.m_Heap.f_Destroy();
			}
			
			bool bDoneSomething = true;
			while (bDoneSomething)
			{
				bDoneSomething = false;

				for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
				{
					for (auto iArena = iNumaArena->m_Arenas.f_GetIterator(); iArena; ++iArena)
					{
						if (iArena->f_ProcessMessages())
							bDoneSomething = true;
					}
				}
			}
			
			m_LocalArena.f_Destroy();
			m_LocalNumaNode.f_Destroy();
			
			while (auto pArena = m_NumaArenas.f_GetRoot())
			{
				m_NumaArenas.f_Remove(pArena);
				pArena->~TCMemoryManagerNumaArena<t_CParams>();
				m_Allocator.f_Free(pArena, sizeof(TCMemoryManagerNumaArena<t_CParams>));
			}
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_DestroyThreadLocals()
		{
			m_LocalArena.f_Destroy();
			m_LocalNumaNode.f_Destroy();
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::fp_ProcessArenaMessages()
		{
			DMibFastCheck(m_NumaArenasLock.f_OwnsLock());

			auto &ThreadLocal = *m_LocalArena;
			auto ReentrantScope = ThreadLocal.f_Reentrant();

			bool bProcessed = true;
			while (bProcessed)
			{
				bProcessed = false;
				for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
				{
					bool bDeferred = false;
					if (iNumaArena->f_ProcessArenaMessages(false, bDeferred, true))
						bProcessed = true;
				}
			}			
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::fp_EnumArenas(NFunction::TCFunctionNoAlloc<void (TCMemoryManagerArena<t_CParams> *)> const &_Functor, bool _bCleanup)
		{
			auto &ThreadLocal = *m_LocalArena;
			auto ReentrantScope = ThreadLocal.f_Reentrant();

			DMibLock(m_NumaArenasLock);
			if (_bCleanup)
				fp_ProcessArenaMessages();
			for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
			{
				DMibLock(iNumaArena->m_ArenasLock);
				for (auto iArena = iNumaArena->m_Arenas.f_GetIterator(); iArena; ++iArena)
				{
					auto pArena = &*iArena;
					if (pArena != ThreadLocal.m_pArena)
					{
						while (pArena->m_Locked.f_FetchOr(EArenaLockFlag_Normal, NAtomic::EMemoryOrder_Acquire) != EArenaLockFlag_None)
						{
							DMibUnlock(iNumaArena->m_ArenasLock);
							{
								DMibUnlock(m_NumaArenasLock);
								NSys::fg_Thread_SmallestSleep();
							}
						}
					}
						
					_Functor(pArena);
					
					if (pArena != ThreadLocal.m_pArena)
					{
						auto LockResult = pArena->m_Locked.f_Exchange(EArenaLockFlag_None, NAtomic::EMemoryOrder_Release);
						if (LockResult & EArenaLockFlag_Waiting)
							iNumaArena->f_ArenaAvailable(pArena);
						if (LockResult & EArenaLockFlag_Cleanup)
							iNumaArena->f_OnNeedCleanup();
					}
				}
			}
		}
		
		template <typename t_CParams>
		mint TCMemoryManager<t_CParams>::f_GetNumUsedSlabs()
		{
			mint nSlabs = 0;
			DMibLock(m_NumaArenasLock);
			for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
			{
				DMibLock(iNumaArena->m_ArenasLock);
				for (auto iArena = iNumaArena->m_Arenas.f_GetIterator(); iArena; ++iArena)
					nSlabs += iArena->f_GetNumUsedSlabs();
			}
			
			{
				DMibLockRead(m_HeapChunksLock);
				nSlabs += m_HeapChunks.f_GetLen();
			}
			
			return nSlabs;
		}

		template <typename t_CParams>
		mint TCMemoryManager<t_CParams>::f_GetNumFreeSlabs()
		{
			mint nSlabs = 0;
			DMibLock(m_NumaArenasLock);
			for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
			{
				{
					DMibLock(iNumaArena->m_FreeSlabsLock);
					nSlabs += iNumaArena->m_FreeSlabs.f_GetLen();
				}

				{
					DMibLock(iNumaArena->m_ArenasLock);
					
					for (auto iArena = iNumaArena->m_Arenas.f_GetIterator(); iArena; ++iArena)
					{
						nSlabs += iArena->f_GetNumFreeSlabs();
					}
				}
			}
			
			return nSlabs;
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_GarbageCollect(bool _bDecommit)
		{
			// We need to garbage collect the heaps first as they generate frees to arenas
			DMibLock(m_NumaArenasLock);
			for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
			{
				while (iNumaArena->f_GarbageCollect(TCLimitsInt<int64>::mc_Max, _bDecommit, true, false) != TCLimitsInt<int64>::mc_Max)
					;
			}
			
		}
		
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_WaitForBackgroundCleanup()
		{
			while (true)
			{
				bool bDone = true;
				{
					DMibLock(m_NumaArenasLock);
					for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
					{
						if (!iNumaArena->m_BackgroundCleanup.f_IsWaiting())
						{
							bDone = false;
							break;
						}
					}
				}
				
				if (bDone)
					break;
				
				NSys::fg_Thread_SmallestSleep();
			}
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_DestroyCleanupThreads()
		{
			{
				DMibLock(m_NumaArenasLock);
				for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
				{
					DMibUnlock(m_NumaArenasLock);
					iNumaArena->m_BackgroundCleanup.f_StopThread();
				}
			}
		}
		
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_CanStartThreads()
		{
			{
				DMibLock(m_NumaArenasLock);
				for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
					iNumaArena->f_CanStartThreads();
			}
		}
		
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::fp_EnumHeaps(NFunction::TCFunctionNoAlloc<void (TCMemoryManagerArenaHeap<t_CParams> *)> const &_Functor)
		{
			DMibLock(m_NumaArenasLock);
			for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
			{
				DMibLock(iNumaArena->m_Heap);
				_Functor(&(iNumaArena->m_Heap));
			}
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_EnumArenas(NFunction::TCFunctionNoAlloc<void (typename t_CParams::CNotifier::CArena *)> const &_Functor)
		{
			fp_EnumArenas
				(
					[&](typename t_CParams::CNotifier::CArena *_pArena)
					{
						_Functor(_pArena);
					}
					, true
				)
			;
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_EnumGlobal(NFunction::TCFunctionNoAlloc<void (typename t_CParams::CNotifier::CGlobal *)> const &_Functor)
		{
			_Functor(this);
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_EnumHeaps(NFunction::TCFunctionNoAlloc<void (typename t_CParams::CNotifier::CHeap *)> const &_Functor)
		{
			DMibLock(m_NumaArenasLock);
			for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
			{
				DMibLock(iNumaArena->m_Heap);
				_Functor(&(iNumaArena->m_Heap));
			}
		}
		
		template <typename t_CParams>
		bool TCMemoryManager<t_CParams>::f_CheckFree(bool _bBreak)
		{
			bool bError = false;
			fp_EnumArenas
				(
					[&](TCMemoryManagerArena<t_CParams> * _pArena)
					{
						if (_pArena->f_CheckFree(_bBreak))
							bError = true;
					}
					, false
				)
			;
			
			fp_EnumHeaps
				(
					[&](TCMemoryManagerArenaHeap<t_CParams> * _pHeap)
					{
						if (_pHeap->f_CheckFree(_bBreak))
							bError = true;
					}
				)
			;
			
			return bError;
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_SetNumaNode(ENumaNode _NumaNode)
		{
			(*m_LocalNumaNode).m_Node = _NumaNode;
			m_LocalArena.f_ReinitForThread();
		}

		template <typename t_CParams>
		inline_never void *TCMemoryManager<t_CParams>::fp_AllocWithCheckout(mint &_Size, TCMemoryManagerThreadLocal<t_CParams> &_LocalArena)
		{
			if (unlikely(m_bCanDoLazyCheckout))
			{
				++fp_CheckoutHelper(_LocalArena)->m_CheckoutCount;
				DMibFastCheck(fg_GetSys()->f_ThreadCreated());
				_LocalArena.m_bLazyCheckout = true;
				return f_AllocAligned(_Size, 1);
			}
			auto Checkout = fp_Checkout(_LocalArena);
			return f_AllocAligned(_Size, 1);
		}

		template <typename t_CParams>
		inline_never void *TCMemoryManager<t_CParams>::fp_AllocWithTempCheckout(mint &_Size)
		{
			TCMemoryManagerThreadLocal<t_CParams> TempArena{fp_GetAnyNumaArena()};
			fp_CheckoutHelper(TempArena);
			auto Cleanup = g_OnScopeExit > [&]
				{
					DMibFastCheck(!TempArena.m_bLazyCheckout);
					TempArena.f_ReturnCheckoutLight();
				}
			;
			return TempArena.m_pArena->f_Alloc(_Size);
		}
		
		template <typename t_CParams>
		inline_never void TCMemoryManager<t_CParams>::fp_AllocBatchWithCheckout(mint _Size, TCMemoryManagerThreadLocal<t_CParams> &_LocalArena, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			if (unlikely(m_bCanDoLazyCheckout))
			{
				++fp_CheckoutHelper(_LocalArena)->m_CheckoutCount;
				DMibFastCheck(fg_GetSys()->f_ThreadCreated());
				_LocalArena.m_bLazyCheckout = true;
				return f_AllocBatch(_Size, 1, _Functor); 
			}
			auto Checkout = fp_Checkout(_LocalArena);
			return f_AllocBatch(_Size, 1, _Functor);
		}
		
		template <typename t_CParams>
		inline_never void TCMemoryManager<t_CParams>::fp_AllocBatchWithTempCheckout(mint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			TCMemoryManagerThreadLocal<t_CParams> TempArena{fp_GetAnyNumaArena()};
			fp_CheckoutHelper(TempArena);
			auto Cleanup = g_OnScopeExit > [&]
				{
					DMibFastCheck(!TempArena.m_bLazyCheckout);
					TempArena.f_ReturnCheckoutLight();
				}
			;
			TempArena.m_pArena->f_AllocBatch(_Size, _Functor);
		}

		template <typename t_CParams>
		inline_never void *TCMemoryManager<t_CParams>::f_Realloc(void * _pMemory, mint &_Size)
		{
			return f_ReallocInline(_pMemory, _Size);
		}

		template <typename t_CParams>
		inline_always void *TCMemoryManager<t_CParams>::f_ReallocInline(void * _pMemory, mint &_Size)
		{
			if (_pMemory)
			{
				mint NewSize = f_SizePadded(_Size);
				mint Size = f_Size(_pMemory);
				
				if (NewSize == Size)
					return _pMemory;
				
				void *pMemory = f_AllocAligned(_Size, 1);
				f_Free(_pMemory);
				return pMemory;
			}
			return f_AllocAligned(_Size, 1);
		}
		
		template <typename t_CParams>
		inline_never void *TCMemoryManager<t_CParams>::f_Resize(void * _pMemory, mint &_Size)
		{
			return f_ResizeInline(_pMemory, _Size);
		}

		template <typename t_CParams>
		inline_always void *TCMemoryManager<t_CParams>::f_ResizeInline(void * _pMemory, mint &_Size)
		{
			if (_pMemory)
			{
				mint Size = f_Size(_pMemory);
				mint NewSize = f_SizePadded(_Size);
				if (NewSize == Size)
					return _pMemory;

				void *pMemory = f_AllocAligned(_Size, 1);

				fg_MemCopy(pMemory, _pMemory, fg_Min(_Size, Size));
				
				if (_pMemory)
					f_Free(_pMemory);
				
				return pMemory;
			}
			return f_AllocAligned(_Size, 1);
		}
		
		template <typename t_CParams>
		inline_never mint TCMemoryManager<t_CParams>::f_Size(void const * _pMemory) const
		{
			return f_SizeInline(_pMemory);
		}

		template <typename t_CParams>
		inline_always mint TCMemoryManager<t_CParams>::f_SizeInline(void const * _pMemory) const
		{
			uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pMemory + 1, t_CParams::mc_SlabSize);
			CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

			if (pHeader->m_Magic == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic))
			{
				TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
				return pSlab->m_pArena->f_Size(_pMemory, pSlab);
			}
			
			{
				TCMemoryManagerArenaHeapChunk<t_CParams> const *pChunk;
				{
					DMibLockRead(m_HeapChunksLock);
					pChunk = m_HeapChunks.f_FindLargestLessThanEqual(_pMemory);
				}
				
				if (pChunk && (uint8 *)_pMemory < pChunk->f_GetEndAddress())
					return pChunk->f_GetHeap()->f_Size(_pMemory, pChunk);
			}
			
			return m_Allocator.f_Size(_pMemory);
		}
		
		template <typename t_CParams>
		uint64 TCMemoryManager<t_CParams>::f_GetMagic() const
		{
			return m_Magic;
		}

		template <typename t_CParams>
		TCMemoryManager<t_CParams> *TCMemoryManager<t_CParams>::f_GetMemoryManager(void const *_pMemory)
		{
			uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pMemory + 1, t_CParams::mc_SlabSize);
			CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

			if (pHeader->m_Magic == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic))
			{ 
				TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
				return pSlab->m_pMemoryManager;
			}
			
			{
				TCMemoryManagerArenaHeapChunk<t_CParams> const *pChunk;
				{
					DMibLockRead(m_HeapChunksLock);
					pChunk = m_HeapChunks.f_FindLargestLessThanEqual(_pMemory);
				}
				
				if (pChunk && (uint8 *)_pMemory < pChunk->f_GetEndAddress())
					return this;
			}
			if (m_Allocator.f_TrySize(_pMemory))
				return this;
			return nullptr;
		}		

		template <typename t_CParams>
		inline_always mint TCMemoryManager<t_CParams>::f_TrySize(void const * _pMemory) const
		{
			uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pMemory + 1, t_CParams::mc_SlabSize);
			CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

			if (pHeader->m_Magic == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic))
			{
				TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
				return pSlab->m_pArena->f_Size(_pMemory, pSlab);
			}
			
			{
				TCMemoryManagerArenaHeapChunk<t_CParams> const *pChunk;
				{
					DMibLockRead(m_HeapChunksLock);
					pChunk = m_HeapChunks.f_FindLargestLessThanEqual(_pMemory);
				}
				
				if (pChunk && (uint8 *)_pMemory < pChunk->f_GetEndAddress())
					return pChunk->f_GetHeap()->f_Size(_pMemory, pChunk);
			}
			
			return m_Allocator.f_TrySize(_pMemory);
		}
		
		template <typename t_CParams>
		fp32 TCMemoryManager<t_CParams>::f_Overhead(void const * _pMemory)
		{
			uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pMemory + 1, t_CParams::mc_SlabSize);
			CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

			if (pHeader->m_Magic == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic))
			{
				TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
				return pSlab->m_pArena->f_Overhead(_pMemory, pSlab);
			}
			
			{
				TCMemoryManagerArenaHeapChunk<t_CParams> const *pChunk;
				{
					DMibLockRead(m_HeapChunksLock);
					pChunk = m_HeapChunks.f_FindLargestLessThanEqual(_pMemory);
				}
				
				if (pChunk && (uint8 *)_pMemory < pChunk->f_GetEndAddress())
					return pChunk->f_GetHeap()->f_Overhead(_pMemory, pChunk);
			}
			
			return m_Allocator.f_Overhead(_pMemory);
		}
		
		template <typename t_CParams>
		bool TCMemoryManager<t_CParams>::f_ContainsBlock(void const * _pMemory)
		{
			DMibPDebugBreak; // Not implemented
			
			return false;
		}
		
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_PrepareFork()
		{
			auto &ThreadLocal = *m_LocalArena;
			auto pNumaArena = ThreadLocal.m_pNumaArena;

			m_NumaArenasLock.f_PrepareFork();
			m_HeapChunksLock.f_PrepareFork();
			pNumaArena->m_ArenasLock.f_PrepareFork();
			pNumaArena->m_FreeSlabsLock.f_PrepareFork();
			pNumaArena->m_Pool.f_PrepareFork();
			pNumaArena->m_PoolThreadLocal.f_PrepareFork();
			pNumaArena->m_BackgroundCleanup.f_PrepareFork();
		}
		
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_ForkedChild()
		{
			auto &ThreadLocal = *m_LocalArena;
			auto pNumaArena = ThreadLocal.m_pNumaArena;

			pNumaArena->m_BackgroundCleanup.f_ForkedChild();;
			pNumaArena->m_PoolThreadLocal.f_ForkedChild();
			pNumaArena->m_Pool.f_ForkedChild();
			pNumaArena->m_FreeSlabsLock.f_ForkedChild();
			pNumaArena->m_ArenasLock.f_ForkedChild();
			m_HeapChunksLock.f_ForkedChild();
			m_NumaArenasLock.f_ForkedChild();

		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_ForkedParent()
		{
			auto &ThreadLocal = *m_LocalArena;
			auto pNumaArena = ThreadLocal.m_pNumaArena;
			
			pNumaArena->m_BackgroundCleanup.f_ForkedParent();
			pNumaArena->m_PoolThreadLocal.f_ForkedParent();
			pNumaArena->m_Pool.f_ForkedParent();
			pNumaArena->m_FreeSlabsLock.f_ForkedParent();
			pNumaArena->m_ArenasLock.f_ForkedParent();
			m_HeapChunksLock.f_ForkedParent();
			m_NumaArenasLock.f_ForkedParent();
			
			
		}
		
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_Lock()
		{
			auto &ThreadLocal = *m_LocalArena;
			auto pNumaArena = ThreadLocal.m_pNumaArena;

			m_NumaArenasLock.f_Lock();
			m_HeapChunksLock.f_Lock();
			
			pNumaArena->m_ArenasLock.f_Lock();
			pNumaArena->m_FreeSlabsLock.f_Lock();
			pNumaArena->m_Pool.f_Lock();
			pNumaArena->m_PoolThreadLocal.f_Lock();
		}
		
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_Unlock()
		{
			auto &ThreadLocal = *m_LocalArena;
			auto pNumaArena = ThreadLocal.m_pNumaArena;

			pNumaArena->m_PoolThreadLocal.f_Unlock();
			pNumaArena->m_Pool.f_Unlock();
			pNumaArena->m_FreeSlabsLock.f_Unlock();
			pNumaArena->m_ArenasLock.f_Unlock();

			m_HeapChunksLock.f_Unlock();
			m_NumaArenasLock.f_Unlock();
			
		}
		
		template <typename t_CParams>
		inline_always void *TCMemoryManager<t_CParams>::f_Alloc(mint &_Size)
		{
			return f_AllocAligned(_Size, 1);
		}

		template <typename t_CParams>
		TCMemoryManagerNumaArena<t_CParams> *TCMemoryManager<t_CParams>::fp_GetAnyNumaArena()
		{
			DMibLock(m_NumaArenasLock);
			return m_NumaArenas.f_FindSmallest();
		}
		
		template <typename t_CParams>
		inline_always void *TCMemoryManager<t_CParams>::f_AllocInline(mint &_Size)
		{
			return f_AllocAligned(_Size, 1);
		}

		template <typename t_CParams>
		inline_never void *TCMemoryManager<t_CParams>::f_AllocAligned(mint &_Size, mint _Alignment)
		{
			return f_AllocAlignedInline(_Size, _Alignment);
		}
		
		template <typename t_CParams>
		inline_never void TCMemoryManager<t_CParams>::fp_AllocBatchSlowPath(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			auto *pLocalArena = m_LocalArena.f_TryGet();
			if (_Size <= t_CParams::mc_MaxSlabAllocSize)
			{
				if (unlikely(!pLocalArena))
				{
					if (unlikely(fg_GetSys()->f_ThreadDestroyed()))
						return fp_AllocBatchWithTempCheckout(_Size, _Functor);
					pLocalArena = &(*m_LocalArena);
				}
				auto &LocalArena = *pLocalArena;
				auto ReentrantScope = LocalArena.f_Reentrant();
				if (LocalArena.m_pArena)
					return LocalArena.m_pArena->f_AllocBatch(_Size, _Functor);
				return fp_AllocBatchWithCheckout(_Size, LocalArena, _Functor);
			}
			
			TCMemoryManagerNumaArena<t_CParams> *pNumaArena;
			if (unlikely(!pLocalArena))
			{
				if (unlikely(fg_GetSys()->f_ThreadDestroyed()))
					pNumaArena = fp_GetAnyNumaArena();
				else
					pNumaArena = m_LocalArena->m_pNumaArena; 
			}
			else
				pNumaArena = pLocalArena->m_pNumaArena;

			if (_Size <= t_CParams::mc_MaxHeapAllocSize)
			{
				while (true)
				{
					mint Size = _Size;
					auto pAlloc = pNumaArena->m_Heap.f_AllocAligned(Size, _Alignment);
					if (!_Functor(pAlloc, Size))
						break;
				}				
				return;
			}
			while (true)
			{
				mint Size = _Size;
				auto pRet = m_Allocator.f_AllocAligned(_Size, _Alignment, t_CParams::mc_AllocationFlags, pNumaArena->m_NumaNode);
				if (this->mc_EnableCallbacks)
					this->f_OnAlloc((uint8 *)pRet, _Size);
				if (!_Functor(pRet, Size))
					break;
			}				
		}
		
		template <typename t_CParams>
		inline_never void *TCMemoryManager<t_CParams>::fp_AllocAlignedSlowPath(mint & _Size, mint _Alignment)
		{
			auto *pLocalArena = m_LocalArena.f_TryGet();
			if (_Size <= t_CParams::mc_MaxSlabAllocSize)
			{
				if (unlikely(!pLocalArena))
				{
					if (unlikely(fg_GetSys()->f_ThreadDestroyed()))
						return fp_AllocWithTempCheckout(_Size);
					pLocalArena = &(*m_LocalArena);
				}
				auto &LocalArena = *pLocalArena;
				auto ReentrantScope = LocalArena.f_Reentrant();
				if (LocalArena.m_pArena)
					return LocalArena.m_pArena->f_Alloc(_Size);
				return fp_AllocWithCheckout(_Size, LocalArena);
			}
			
			TCMemoryManagerNumaArena<t_CParams> *pNumaArena;
			if (unlikely(!pLocalArena))
			{
				if (unlikely(fg_GetSys()->f_ThreadDestroyed()))
					pNumaArena = fp_GetAnyNumaArena();
				else
					pNumaArena = m_LocalArena->m_pNumaArena; 
			}
			else
				pNumaArena = pLocalArena->m_pNumaArena;
			
			if (_Size <= t_CParams::mc_MaxHeapAllocSize)
				return pNumaArena->m_Heap.f_AllocAligned(_Size, _Alignment);
			auto pRet = m_Allocator.f_AllocAligned(_Size, _Alignment, t_CParams::mc_AllocationFlags, pNumaArena->m_NumaNode);
			if (this->mc_EnableCallbacks)
				this->f_OnAlloc((uint8 *)pRet, _Size);
			return pRet;
		}
		
		template <typename t_CParams>
		inline_always void *TCMemoryManager<t_CParams>::f_AllocAlignedInline(mint &_Size, mint _Alignment)
		{
			_Size = fg_AlignUp(_Size, _Alignment);
			
			if (likely(_Size <= t_CParams::mc_MaxSlabAllocSize))
			{
				auto *pLocalArena = m_LocalArena.f_TryGet();
				if (unlikely(!pLocalArena))
					goto l_SlowPath;
				auto &LocalArena = *pLocalArena;
				auto ReentrantScope = LocalArena.f_Reentrant();
				if (likely(LocalArena.m_pArena))
					return LocalArena.m_pArena->f_Alloc(_Size);
				return fp_AllocWithCheckout(_Size, LocalArena);
			}
			
		l_SlowPath:
			return fp_AllocAlignedSlowPath(_Size, _Alignment);
		}
		
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			_Size = fg_AlignUp(_Size, _Alignment);
			
			if (likely(_Size <= t_CParams::mc_MaxSlabAllocSize))
			{
				auto *pLocalArena = m_LocalArena.f_TryGet();
				if (unlikely(!pLocalArena))
					goto l_SlowPath;
				auto &LocalArena = *pLocalArena;
				auto ReentrantScope = LocalArena.f_Reentrant();
				if (LocalArena.m_pArena)
					return LocalArena.m_pArena->f_AllocBatch(_Size, _Functor);
				return fp_AllocBatchWithCheckout(_Size, LocalArena, _Functor);
			}
			
		l_SlowPath:
			return fp_AllocBatchSlowPath(_Size, _Alignment, _Functor);
		}

		template <typename t_CParams>
		mint TCMemoryManager<t_CParams>::f_SizePadded(mint _Size)
		{
			if (_Size <= t_CParams::mc_MaxSlabAllocSize)
				return TCMemoryManagerArena<t_CParams>::fs_GetAllocSize(_Size);
			else if (_Size <= t_CParams::mc_MaxHeapAllocSize)
				return TCMemoryManagerArenaHeap<t_CParams>::fs_GetAllocSize(_Size);
			else
				return m_Allocator.f_SizePadded(_Size);
		}

		template <typename t_CParams>
		inline_never void TCMemoryManager<t_CParams>::f_Free(void *_pMemory)
		{
			return f_FreeInline(_pMemory);
		}

		template <typename t_CParams>
		inline_never void TCMemoryManager<t_CParams>::fp_FreeSlowPath(void * _pMemory)
		{
			uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pMemory + 1, t_CParams::mc_SlabSize);
			CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

			if (likely(pHeader->m_Magic == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic)))
			{
				TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
				
				auto *pLocalArena = m_LocalArena.f_TryGet();
				if (unlikely(!pLocalArena))
				{
					if (unlikely(fg_GetSys()->f_ThreadDestroyed()))
					{
						pSlab->m_pArena->f_FreeOtherThread(_pMemory, pSlab, nullptr);
						return;
					}
					pLocalArena = &(*m_LocalArena);
				}
					
				auto &LocalArena = *pLocalArena;
				auto ReentrantScope = LocalArena.f_Reentrant();
				if (pSlab->m_pArena == LocalArena.m_pArena)
					pSlab->m_pArena->f_FreeThisThread(_pMemory, pSlab);
				else
					pSlab->m_pArena->f_FreeOtherThread(_pMemory, pSlab, &LocalArena);
				return;
			}
			
			{
				TCMemoryManagerArenaHeapChunk<t_CParams> *pChunk;
				{
					DMibLockRead(m_HeapChunksLock);
					pChunk = m_HeapChunks.f_FindLargestLessThanEqual(_pMemory);
				}
				
				if (pChunk && (uint8 *)_pMemory < pChunk->f_GetEndAddress())
				{
					pChunk->f_GetHeap()->f_Free(_pMemory, pChunk);
					return;
				}
			}
			
			if (this->mc_EnableCallbacks)
				this->f_OnFree((uint8 *)_pMemory);
			m_Allocator.f_Free(_pMemory);
		}

		template <typename t_CParams>
		inline_always void TCMemoryManager<t_CParams>::f_FreeInline(void *_pMemory)
		{
			if (!_pMemory)
				return;

			uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pMemory + 1, t_CParams::mc_SlabSize);
			CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

			if (likely(pHeader->m_Magic == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic)))
			{
				TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
				
				auto *pLocalArena = m_LocalArena.f_TryGet();
				if (unlikely(!pLocalArena))
					goto l_SlowPath;
				auto &LocalArena = *pLocalArena;
				auto ReentrantScope = LocalArena.f_Reentrant();
				if (pSlab->m_pArena == LocalArena.m_pArena)
					pSlab->m_pArena->f_FreeThisThread(_pMemory, pSlab);
				else
					pSlab->m_pArena->f_FreeOtherThread(_pMemory, pSlab, &LocalArena);
				return;
			}
			
		l_SlowPath:
			fp_FreeSlowPath(_pMemory);
		}

		///
		/// Checkout
		/// ========

		template <typename t_CParams>
		inline_always TCMemoryManagerCheckoutLight<t_CParams> TCMemoryManager<t_CParams>::fp_Checkout(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal)
		{
			DMibFastCheck(!_ThreadLocal.m_pArena);
			fp_CheckoutHelper(_ThreadLocal);
			return this;
		}

		template <typename t_CParams>
		inline_never TCMemoryManagerArena<t_CParams> *TCMemoryManager<t_CParams>::fp_CheckoutHelperWaitForCleanup(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal)
		{
			while (true)
			{
				auto pArena = _ThreadLocal.m_pPreferredArena;
				auto CheckoutResult = pArena->m_Locked.f_FetchOr(EArenaLockFlag_Normal, NAtomic::EMemoryOrder_Acquire);
				if (CheckoutResult == EArenaLockFlag_None)
				{
					if (pArena->m_pOwningThreadLocal && pArena->m_pOwningThreadLocal != &_ThreadLocal)
					{
						pArena->f_ReturnCheckoutLight();
						_ThreadLocal.m_pPreferredArena = nullptr;
						return fp_CheckoutHelperSlowPath(_ThreadLocal);
					}

					_ThreadLocal.m_pArena = pArena;
					return pArena;
				}
				else if (!_ThreadLocal.m_bOwnArena && pArena->m_CheckoutCount > 0) // Another checkout got inbetween
					return fp_CheckoutHelperSlowPath(_ThreadLocal);
				yield_cpu;
			}
		}
		
		template <typename t_CParams>
		inline_always TCMemoryManagerArena<t_CParams> *TCMemoryManager<t_CParams>::fp_CheckoutHelper(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal)
		{
			DMibFastCheck(!_ThreadLocal.m_pArena);
			auto pArena = _ThreadLocal.m_pPreferredArena;
			if (pArena)
			{
				auto CheckoutResult = pArena->m_Locked.f_FetchOr(EArenaLockFlag_Normal, NAtomic::EMemoryOrder_Acquire);
				if (CheckoutResult == EArenaLockFlag_None)
				{
					if (pArena->m_pOwningThreadLocal && pArena->m_pOwningThreadLocal != &_ThreadLocal)
					{
						pArena->f_ReturnCheckoutLight();
						_ThreadLocal.m_pPreferredArena = nullptr;
						return fp_CheckoutHelperSlowPath(_ThreadLocal);
					}
					_ThreadLocal.m_pArena = pArena;
					return pArena;
				}
				else if (CheckoutResult == EArenaLockFlag_Cleanup || _ThreadLocal.m_bOwnArena)
					return fp_CheckoutHelperWaitForCleanup(_ThreadLocal);
			}
			return fp_CheckoutHelperSlowPath(_ThreadLocal);
		}
	
		template <typename t_CParams>
		inline_never TCMemoryManagerArena<t_CParams> *TCMemoryManager<t_CParams>::fp_CheckoutHelperWaitUnlock(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal)
		{
			auto pNumaArena = _ThreadLocal.m_pNumaArena;

			while (true)
			{
				TCMemoryManagerArena<t_CParams> *pArena;
				pArena = pNumaArena->m_pFirstArena.f_Load();
				while (pArena)
				{
					mint NextArena = pArena->m_pNextArena.f_Load(NAtomic::EMemoryOrder_Acquire);
					if ((!(NextArena & 1)) && pArena->m_Locked.f_FetchOr(EArenaLockFlag_Waiting, NAtomic::EMemoryOrder_Acquire) == EArenaLockFlag_None)
					{
						if (pArena->m_pOwningThreadLocal && pArena->m_pOwningThreadLocal != &_ThreadLocal)
							pArena->f_ReturnCheckoutLight();
						else
						{
							_ThreadLocal.m_pArena = pArena;
							return pArena;
						}
					}
					
					pArena = (TCMemoryManagerArena<t_CParams> *)(NextArena & mint(~mint(1)));
				}				
				
				pNumaArena->m_ArenaAvailableEvent.f_Wait();
			}			
		}
		
		template <typename t_CParams>
		inline_never TCMemoryManagerArena<t_CParams> *TCMemoryManager<t_CParams>::fp_CheckoutHelperSlowPath(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal)
		{
			do
			{
				TCMemoryManagerArena<t_CParams> *pArena;
				
				auto pNumaArena = _ThreadLocal.m_pNumaArena;

				for (mint i = 0; i < 2; ++i)
				{
					mint nArenas = m_nArenas; // This is racy, so we can get > 0 arenas while pArena is still nullptr
					pArena = _ThreadLocal.m_pPreferredArena;
					if (!pArena)
						pArena = pNumaArena->m_pFirstArena.f_Load();
					
					for (mint i = 0; i < nArenas && pArena; ++i)
					{
						mint NextArena = pArena->m_pNextArena.f_Load(NAtomic::EMemoryOrder_Relaxed);
						if ((!(NextArena & 1)) && pArena->m_Locked.f_FetchOr(EArenaLockFlag_Normal, NAtomic::EMemoryOrder_Acquire) == EArenaLockFlag_None)
						{
							if (pArena->m_pOwningThreadLocal && pArena->m_pOwningThreadLocal != &_ThreadLocal)
								pArena->f_ReturnCheckoutLight();
							else
							{
								_ThreadLocal.m_pArena = pArena;
								return pArena;
							}
						}
						pArena = (TCMemoryManagerArena<t_CParams> *)(NextArena & mint(~mint(1)));
						if (!pArena)
							pArena = pNumaArena->m_pFirstArena.f_Load();
					}
				}
				
				
				bool bWait = false;
				{
					DMibLock(m_nArenasLock);
					if (m_nArenas >= m_nMaxArenas && pNumaArena->m_nArenas != 0)
						bWait = true;
					else
					{
						++pNumaArena->m_nArenas;
						++m_nArenas;
					}
				}
				
				if (bWait)
					return fp_CheckoutHelperWaitUnlock(_ThreadLocal);
				
				{
					DMibLock(pNumaArena->m_ArenasLock);
					
					pArena = pNumaArena->f_NewArena();
					pArena->m_Locked.f_Exchange(EArenaLockFlag_Normal, NAtomic::EMemoryOrder_Acquire); // Set lock for us
					pNumaArena->m_Arenas.f_Insert(pArena);
				}
				{
					while (1)
					{
						// Put in list
						TCMemoryManagerArena<t_CParams> *pOldFirst = (TCMemoryManagerArena<t_CParams> *)pNumaArena->m_pFirstArena.f_Load(NAtomic::EMemoryOrder_Acquire);
						DMibFastCheck(!(pArena->m_pNextArena.f_Load() & 1));
						pArena->m_pNextArena.f_Store((mint)pOldFirst, NAtomic::EMemoryOrder_Release);
						if (pNumaArena->m_pFirstArena.f_CompareExchangeWeak(pOldFirst, pArena, NAtomic::EMemoryOrder_SequentiallyConsistent))
							break;
					}
				}
				_ThreadLocal.m_pArena = pArena;
				return pArena;
			}
			while (false)
				;
			
			return nullptr;
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_CheckoutManual()
		{
			auto &ThreadLocal = *m_LocalArena;
			if (ThreadLocal.m_pArena)
				++ThreadLocal.m_pArena->m_CheckoutCount;
			else
				++fp_CheckoutHelper(ThreadLocal)->m_CheckoutCount;
		}
		
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_CheckinManual()
		{
			auto &ThreadLocal = *m_LocalArena;
			DMibFastCheck(ThreadLocal.m_pArena);
			
			ThreadLocal.f_ReturnCheckout();
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_CheckinManualLight()
		{
			auto &ThreadLocal = *m_LocalArena;
			DMibFastCheck(ThreadLocal.m_pArena);
			
			ThreadLocal.f_ReturnCheckoutLight();
		}
		
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_LazyReturnCheckout()
		{
			if (unlikely(!m_LocalArena.f_IsValid()))
				return;
			auto &ThreadLocal = *m_LocalArena;
			if (ThreadLocal.m_bLazyCheckout && !ThreadLocal.m_Reentrant)
			{
				ThreadLocal.m_bLazyCheckout = false;
				ThreadLocal.f_ReturnCheckout();
			}
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_CanDoLazyCheckout()
		{
			m_bCanDoLazyCheckout = true;
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_ReturnCheckoutVirtual()
		{
			auto &ThreadLocal = *m_LocalArena;
			DMibFastCheck(ThreadLocal.m_pArena);
			
			ThreadLocal.f_ReturnCheckout();
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_TemporaryReturn()
		{
			auto &ThreadLocal = *m_LocalArena;
			ThreadLocal.f_TemporaryReturn();
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_TemporaryGetBack()
		{
			auto &ThreadLocal = *m_LocalArena;
			ThreadLocal.f_TemporaryGetBack();
		}
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_TakeOwnership()
		{
			auto &ThreadLocal = *m_LocalArena;
			ThreadLocal.f_TakeOwnership();
		}

		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_RelinquishOwnership()
		{
			auto &ThreadLocal = *m_LocalArena;
			ThreadLocal.f_RelinquishOwnership();
		}
		
		template <typename t_CParams>
		void TCMemoryManager<t_CParams>::f_GarbageCollectLocalArena(bool _bDecommit)
		{
			auto &ThreadLocal = *m_LocalArena;
			ThreadLocal.f_GarbageCollectLocalArena(_bDecommit);
		}

		template <typename t_CParams>
		TCMemoryManagerCheckout<t_CParams> TCMemoryManager<t_CParams>::f_Checkout()
		{
			auto &ThreadLocal = *m_LocalArena;
			if (ThreadLocal.m_pArena)
				++ThreadLocal.m_pArena->m_CheckoutCount;
			else
				++fp_CheckoutHelper(ThreadLocal)->m_CheckoutCount;
			
			return this;
		}

		template <typename t_CParams>
		CMemoryManagerCheckout TCMemoryManager<t_CParams>::f_CheckoutVirtual()
		{
			auto &ThreadLocal = *m_LocalArena;
			if (ThreadLocal.m_pArena)
				++ThreadLocal.m_pArena->m_CheckoutCount;
			else
				++fp_CheckoutHelper(ThreadLocal)->m_CheckoutCount;
			
			return this;
		}

		///
		/// Checkout
		/// ========
		
		template <typename t_CParams>
		TCMemoryManagerCheckout<t_CParams>::TCMemoryManagerCheckout(TCMemoryManagerCheckout &&_Other)
			: m_pMemoryManager(_Other.m_pMemoryManager)
		{
			_Other.m_pMemoryManager = nullptr;
		}

		template <typename t_CParams>
		TCMemoryManagerCheckout<t_CParams> &TCMemoryManagerCheckout<t_CParams>::operator = (TCMemoryManagerCheckout<t_CParams> &&_Other)
		{
			if (m_pMemoryManager)
				m_pMemoryManager->f_CheckinManual();
			
			m_pMemoryManager = _Other.m_pMemoryManager;
			_Other.m_pMemoryManager = nullptr;
			return *this;
		}

		template <typename t_CParams>
		TCMemoryManagerCheckout<t_CParams>::TCMemoryManagerCheckout(TCMemoryManager<t_CParams> *_pMemoryManager)
			: m_pMemoryManager(_pMemoryManager)
		{
					
		}

		template <typename t_CParams>
		TCMemoryManagerCheckout<t_CParams>::~TCMemoryManagerCheckout()
		{
			if (m_pMemoryManager)
				m_pMemoryManager->f_CheckinManual();
		}
		
		///
		/// Light checkout
		/// ==============
		
		
		template <typename t_CParams>
		TCMemoryManagerCheckoutLight<t_CParams>::TCMemoryManagerCheckoutLight(TCMemoryManagerCheckoutLight &&_Other)
			: m_pMemoryManager(_Other.m_pMemoryManager)
		{
			_Other.m_pMemoryManager = nullptr;
		}

		template <typename t_CParams>
		TCMemoryManagerCheckoutLight<t_CParams> &TCMemoryManagerCheckoutLight<t_CParams>::operator = (TCMemoryManagerCheckoutLight<t_CParams> &&_Other)
		{
			if (m_pMemoryManager)
				m_pMemoryManager->f_CheckinManualLight();
			
			m_pMemoryManager = _Other.m_pMemoryManager;
			_Other.m_pMemoryManager = nullptr;
			return *this;
		}

		template <typename t_CParams>
		TCMemoryManagerCheckoutLight<t_CParams>::TCMemoryManagerCheckoutLight(TCMemoryManager<t_CParams> *_pMemoryManager)
			: m_pMemoryManager(_pMemoryManager)
		{
					
		}

		template <typename t_CParams>
		TCMemoryManagerCheckoutLight<t_CParams>::~TCMemoryManagerCheckoutLight()
		{
			if (m_pMemoryManager)
				m_pMemoryManager->f_CheckinManualLight();
		}
		
	}
}
