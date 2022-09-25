// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	template <typename... tfp_CAllocator>
	TCMemoryManager<t_CParams>::TCMemoryManager(CMemoryManagerConfig const &_Config, tfp_CAllocator &&..._Params)
		: t_CParams::CNotifier::CGlobal(*this)
		, m_Magic(_Config.m_Magic)
		, m_nMaxArenas(fg_Min(fg_RoundPowerOfTwoUp(_Config.m_nMaxArenas), t_CParams::mc_MaxArenas))
		, m_HeapChunks(NMemory::CAllocatorConstructTag(), this)
		, m_Allocator(fg_Forward<tfp_CAllocator>(_Params)...)
		, m_LocalArena
		(
			[this]() -> NThread::CThreadLocalInterface::CSafeAllocMemory
			{
				ENumaNode NumaNode = ENumaNode_Default;
				if (m_LocalNumaNode.f_IsValid()) // Don't init if we don't need to
					NumaNode = (*m_LocalNumaNode).m_Node;

				TCMemoryManagerNumaArena<t_CParams> *pNumaArena;
				{
					DMibLock(m_NumaArenasLock);
					pNumaArena = m_NumaArenas.f_FindEqual(NumaNode);
					if (!pNumaArena)
					{
						pNumaArena =
							new(m_Allocator.f_Alloc(sizeof(TCMemoryManagerNumaArena<t_CParams>), EAllocationFlag_WillFreeWithSize | t_CParams::mc_AllocationFlags, NumaNode))
							TCMemoryManagerNumaArena<t_CParams>(NumaNode, this, m_Magic, m_nMaxArenas != 0)
						;
						m_NumaArenas.f_Insert(pNumaArena);
					}
				}

				return {pNumaArena->m_PoolThreadLocal.f_GetBlock(), sizeof(TCMemoryManagerThreadLocal<t_CParams>)};
			}
			, [this](NThread::CThreadLocalInterface::CSafeAllocMemory const &_Memory) -> void
			{
				ENumaNode NumaNode = ENumaNode_Default;
				if (m_LocalNumaNode.f_IsValid()) // Don't init if we don't need to
					NumaNode = (*m_LocalNumaNode).m_Node;

				TCMemoryManagerNumaArena<t_CParams> *pNumaArena;
				{
					DMibLock(m_NumaArenasLock);
					pNumaArena = m_NumaArenas.f_FindEqual(NumaNode);
				}

				DMibFastCheck(pNumaArena);

				pNumaArena->m_PoolThreadLocal.f_ReturnBlock(_Memory.m_pMemory);
			}
			, [this](TCMemoryManagerThreadLocal<t_CParams> *_pParentArena, void *_pMemory, bool _bMove) -> TCMemoryManagerThreadLocal<t_CParams> *
			{
				ENumaNode NumaNode = ENumaNode_Default;
				if (m_LocalNumaNode.f_IsValid()) // Don't init if we don't need to
					NumaNode = (*m_LocalNumaNode).m_Node;

				TCMemoryManagerNumaArena<t_CParams> *pNumaArena;
				{
					DMibLock(m_NumaArenasLock);
					pNumaArena = m_NumaArenas.f_FindEqual(NumaNode);
				}

				DMibFastCheck(pNumaArena);

				if (_bMove)
					return new (_pMemory) TCMemoryManagerThreadLocal<t_CParams>(fg_Move(*_pParentArena), pNumaArena);
				else
					return new (_pMemory) TCMemoryManagerThreadLocal<t_CParams>(pNumaArena);
			}
			, [](TCMemoryManagerThreadLocal<t_CParams> *_pArena)
			{
				TCMemoryManagerNumaArena<t_CParams> *pNumaArena = _pArena->m_pNumaArena;
				pNumaArena->m_PoolThreadLocal.f_Delete(_pArena);
			}
		)
		, m_LocalNumaNode
		(
			[this]() -> NThread::CThreadLocalInterface::CSafeAllocMemory
			{
				return {this->m_LocalNumaNodePool.f_GetBlock(), sizeof(CLocalNumaNode)};
			}
			, [this](NThread::CThreadLocalInterface::CSafeAllocMemory const &_Memory) -> void
			{
				return this->m_LocalNumaNodePool.f_ReturnBlock(_Memory.m_pMemory);
			}
			, [] (CLocalNumaNode *_pParent, void *_pMemory, bool _bMove) -> CLocalNumaNode *
			{
				if (_pParent)
					return new (_pMemory) CLocalNumaNode(*_pParent);
				else
					return new (_pMemory) CLocalNumaNode();
			}
			, [this] (CLocalNumaNode *_pData)
			{
				this->m_LocalNumaNodePool.f_Delete(_pData);
			}
		)
	{
		static_assert(fg_RoundPowerOfTwoUp(t_CParams::mc_MaxArenas) == t_CParams::mc_MaxArenas);
		ICMemoryManagerReturnCheckout::m_Version = ECMemoryManagerReturnCheckoutVersion;
	}

#if DMibConfig_Memory_Shims_Lightweight
	template <typename t_CParams>
	CReportMemoryLightweight *TCMemoryManager<t_CParams>::f_ReportMemoryTo(CReportMemoryLightweight *_pMemoryReporter)
	{
		if (m_bThreadLocalsDestroyed)
			return nullptr;
		auto &ThreadLocal = *m_LocalArena;
		auto pOld = ThreadLocal.m_pLightweightReporter;
		ThreadLocal.m_pLightweightReporter = _pMemoryReporter;
		return pOld;
	}

	template <typename t_CParams>
	EMemoryReportLightweightScopeFlag TCMemoryManager<t_CParams>::f_GetLightweightScopeFlags()
	{
		if (m_bThreadLocalsDestroyed || fg_GetSys()->f_ThreadDestroyed())
			return EMemoryReportLightweightScopeFlag_None;
		auto &ThreadLocal = *m_LocalArena;
		return ThreadLocal.m_LightweightScopeFlags;
	}

	template <typename t_CParams>
	EMemoryReportLightweightScopeFlag TCMemoryManager<t_CParams>::f_SetLightweightScopeFlags(EMemoryReportLightweightScopeFlag _Flags)
	{
		if (m_bThreadLocalsDestroyed || fg_GetSys()->f_ThreadDestroyed())
			return EMemoryReportLightweightScopeFlag_None;
		auto &ThreadLocal = *m_LocalArena;
		auto Old = ThreadLocal.m_LightweightScopeFlags;
		ThreadLocal.m_LightweightScopeFlags = _Flags;
		return Old;
	}

	template <typename t_CParams>
	EMemoryReportLightweightScopeFlag TCMemoryManager<t_CParams>::f_AddLightweightScopeFlags(EMemoryReportLightweightScopeFlag _Flags)
	{
		if (m_bThreadLocalsDestroyed || fg_GetSys()->f_ThreadDestroyed())
			return EMemoryReportLightweightScopeFlag_None;
		auto &ThreadLocal = *m_LocalArena;
		auto Old = ThreadLocal.m_LightweightScopeFlags;
		ThreadLocal.m_LightweightScopeFlags |= _Flags;
		return Old;
	}

	template <typename t_CParams>
	inline_always void TCMemoryManager<t_CParams>::fp_TrackAlloc(mint _Size)
	{
		auto *pLocalArena = m_LocalArena.f_TryGet();
		if (fsp_ShouldTrackAlloc(pLocalArena))
			pLocalArena->f_TrackAlloc(_Size);
	}

	template <typename t_CParams>
	inline_always bool TCMemoryManager<t_CParams>::fsp_ShouldTrackAlloc(TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena)
	{
		if (_pLocalArena && _pLocalArena->m_pLightweightReporter)
			return true;
		return false;
	}
#endif

#if DMibConfig_Memory_CustomThreadLocal
	template <typename t_CParams>
	void *TCMemoryManager<t_CParams>::f_GetCustomThreadLocal(mint _Index)
	{
		auto *pLocalArena = m_LocalArena.f_TryGet();
		if (pLocalArena)
			return pLocalArena->m_pCustom[_Index];
		return nullptr;
	}

	template <typename t_CParams>
	void *TCMemoryManager<t_CParams>::f_SetCustomThreadLocal(mint _Index, void *_pCustom)
	{
		auto *pLocalArena = m_LocalArena.f_TryGet();
		if (pLocalArena)
		{
			void *pOld = pLocalArena->m_pCustom[_Index];
			pLocalArena->m_pCustom[_Index] = _pCustom;
			return pOld;
		}
		else
			return nullptr;
	}
#endif

	template <typename t_CParams>
	TCMemoryManager<t_CParams>::~TCMemoryManager()
	{
		f_DestroyCleanupThreads();

		// Remove all big blocks
		{
			DMibMemLightweightTrackDisableScope;

			m_HeapChunks.f_Clear();

			for (auto &Arena : m_NumaArenas)
			{
				Arena.m_Heap.f_Destroy();
			}
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

		m_bThreadLocalsDestroyed = true;
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
		m_bThreadLocalsDestroyed = true;
		m_LocalArena.f_Destroy();
		m_LocalNumaNode.f_Destroy();
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::fp_ProcessArenaMessages()
	{
		auto &ThreadLocal = *m_LocalArena;
		auto ReentrantScope = ThreadLocal.f_Reentrant();

		bool bProcessed = true;
		while (bProcessed)
		{
			bProcessed = false;
			for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
			{
				DMibUnlock(m_NumaArenasLock); // Arenas should only be added to m_NumaArenas, so this should be safe

				bool bDeferred = false;
				if (iNumaArena->f_ProcessArenaMessages(false, bDeferred))
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
			DMibUnlock(m_NumaArenasLock); // Arenas should only be added to m_NumaArenas, so this should be safe
			DMibLock(iNumaArena->m_ArenasLock);
			for (auto iArena = iNumaArena->m_Arenas.f_GetIterator(); iArena;)
			{
				auto pArena = &*iArena;
				++iArena;
				DMibUnlock(iNumaArena->m_ArenasLock);
				if (pArena != ThreadLocal.m_pArena)
					pArena->m_Lock.f_LockNoSanitize();

				_Functor(pArena);

				if (pArena != ThreadLocal.m_pArena)
					pArena->m_Lock.f_UnlockNoSanitize();
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
			DMibUnlock(m_NumaArenasLock); // Arenas should only be added to m_NumaArenas, so this should be safe
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
			DMibUnlock(m_NumaArenasLock); // Arenas should only be added to m_NumaArenas, so this should be safe
			{
				DMibLock(iNumaArena->m_FreeSlabsLock);
				nSlabs += iNumaArena->m_FreeSlabs.f_GetLen();
			}

			{
				DMibLock(iNumaArena->m_ArenasLock);
				for (auto iArena = iNumaArena->m_Arenas.f_GetIterator(); iArena; ++iArena)
					nSlabs += iArena->f_GetNumFreeSlabs();
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
			DMibUnlock(m_NumaArenasLock); // Arenas should only be added to m_NumaArenas, so this should be safe
			while (iNumaArena->f_GarbageCollect({TCLimitsInt<int64>::mc_Max, TCLimitsInt<int64>::mc_Max}, _bDecommit, false) != TCLimitsInt<int64>::mc_Max)
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
			{
				DMibUnlock(m_NumaArenasLock); // Arenas should only be added to m_NumaArenas, so this should be safe
				iNumaArena->f_CanStartThreads();
			}
		}
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::f_ForceStartCleanupThreads()
	{
		{
			DMibLock(m_NumaArenasLock);
			for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
			{
				DMibUnlock(m_NumaArenasLock); // Arenas should only be added to m_NumaArenas, so this should be safe
				iNumaArena->f_ForceStartCleanupThread();
			}
		}
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::fp_EnumHeaps(NFunction::TCFunctionNoAlloc<void (TCMemoryManagerArenaHeap<t_CParams> *)> const &_Functor)
	{
		DMibLock(m_NumaArenasLock);
		for (auto iNumaArena = m_NumaArenas.f_GetIterator(); iNumaArena; ++iNumaArena)
		{
			DMibUnlock(m_NumaArenasLock); // Arenas should only be added to m_NumaArenas, so this should be safe
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
			DMibUnlock(m_NumaArenasLock); // Arenas should only be added to m_NumaArenas, so this should be safe
			DMibLock(iNumaArena->m_Heap);
			_Functor(&(iNumaArena->m_Heap));
		}
	}

	template <typename t_CParams>
	bool TCMemoryManager<t_CParams>::f_CheckFree(EMemoryManagerCheckFlag _Flags)
	{
		bool bError = false;
		fp_EnumArenas
			(
				[&](TCMemoryManagerArena<t_CParams> * _pArena)
				{
					if (_pArena->f_CheckFree(_Flags))
						bError = true;
				}
				, false
			)
		;

		fp_EnumHeaps
			(
				[&](TCMemoryManagerArenaHeap<t_CParams> * _pHeap)
				{
					if (_pHeap->f_CheckFree(_Flags))
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

		if (m_bCanDoLazyCheckout)
			f_LazyReturnCheckout();

		m_LocalArena.f_ReinitForThread();
	}

	template <typename t_CParams>
	inline_never void *TCMemoryManager<t_CParams>::fp_AllocWithCheckout(mint &_Size, TCMemoryManagerThreadLocal<t_CParams> &_LocalArena)
	{
		if (m_bCanDoLazyCheckout && !_LocalArena.m_TemporaryReturnCheckoutCount) [[unlikely]]
		{
			fp_CheckoutHelper(_LocalArena)->m_CheckoutCount.f_FetchAdd(1, NAtomic::EMemoryOrder_Relaxed);
			DMibFastCheck(fg_GetSys()->f_ThreadCreated());
			_LocalArena.m_bLazyCheckout = true;
			return f_AllocAlignedWithSize(_Size, 1);
		}
		auto Checkout = fp_Checkout(_LocalArena);
		return f_AllocAlignedWithSize(_Size, 1);
	}

	template <typename t_CParams>
	inline_never void *TCMemoryManager<t_CParams>::fp_AllocWithTempCheckout(mint &_Size)
	{
		TCMemoryManagerThreadLocal<t_CParams> TempArena{fp_GetAnyNumaArena()};
		fp_CheckoutHelper(TempArena);
		auto Cleanup = g_OnScopeExit / [&]
			{
				DMibFastCheck(!TempArena.m_bLazyCheckout);
				TempArena.f_ReturnCheckoutLight();
			}
		;
		return TempArena.m_pArena->f_AllocWithSize(_Size);
	}

	template <typename t_CParams>
	inline_never void TCMemoryManager<t_CParams>::fp_AllocBatchWithCheckout(mint _Size, TCMemoryManagerThreadLocal<t_CParams> &_LocalArena, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
	{
		if (m_bCanDoLazyCheckout && !_LocalArena.m_TemporaryReturnCheckoutCount) [[unlikely]]
		{
			fp_CheckoutHelper(_LocalArena)->m_CheckoutCount.f_FetchAdd(1, NAtomic::EMemoryOrder_Relaxed);
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
		auto Cleanup = g_OnScopeExit / [&]
			{
				DMibFastCheck(!TempArena.m_bLazyCheckout);
				TempArena.f_ReturnCheckoutLight();
			}
		;
		TempArena.m_pArena->f_AllocBatch(_Size, _Functor);
	}

	template <typename t_CParams>
	inline_never void *TCMemoryManager<t_CParams>::f_Realloc(void * _pMemory, mint &_Size, mint _OldSize)
	{
		return f_ReallocInline(_pMemory, _Size, _OldSize);
	}

	template <typename t_CParams>
	inline_always void *TCMemoryManager<t_CParams>::f_ReallocInline(void * _pMemory, mint &_Size, mint _OldSize)
	{
		if (_pMemory)
		{
			mint NewSize = f_SizePadded(_Size);
			mint Size = _OldSize ? _OldSize : f_Size(_pMemory);

			if (NewSize == f_SizePadded(Size))
				return _pMemory;

			void *pMemory = f_AllocAlignedWithSize(_Size, 1);
			f_Free(_pMemory, Size);
			return pMemory;
		}
		return f_AllocAlignedWithSize(_Size, 1);
	}

	template <typename t_CParams>
	inline_never void *TCMemoryManager<t_CParams>::f_Resize(void * _pMemory, mint &_Size, mint _OldSize)
	{
		return f_ResizeInline(_pMemory, _Size, _OldSize);
	}

	template <typename t_CParams>
	inline_always void *TCMemoryManager<t_CParams>::f_ResizeInline(void * _pMemory, mint &_Size, mint _OldSize)
	{
		if (_pMemory)
		{
			mint Size = _OldSize ? _OldSize : f_Size(_pMemory);
			mint NewSize = f_SizePadded(_Size);
			if (NewSize == f_SizePadded(Size))
				return _pMemory;

			void *pMemory = f_AllocAlignedWithSize(_Size, 1);

			fg_MemCopy(pMemory, _pMemory, fg_Min(_Size, Size));

			if (_pMemory)
				f_Free(_pMemory, Size);

			return pMemory;
		}
		return f_AllocAlignedWithSize(_Size, 1);
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

		if (pHeader->f_GetMagic() == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic))
		{
			TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
			return pSlab->m_pArena->f_Size(_pMemory, pSlab);
		}

		{
			TCMemoryManagerArenaHeapChunk<t_CParams> const *pChunk;
			bool bIsChunk;
			{
				DMibLockRead(m_HeapChunksLock);
				pChunk = m_HeapChunks.f_FindLargestLessThanEqual(_pMemory);
				bIsChunk = pChunk && (uint8 *)_pMemory < pChunk->f_GetEndAddress();
			}
			if (bIsChunk)
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

		if (pHeader->f_GetMagic() == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic))
		{
			TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
			return pSlab->m_pMemoryManager;
		}

		{
			TCMemoryManagerArenaHeapChunk<t_CParams> const *pChunk;
			{
				DMibLockRead(m_HeapChunksLock);
				pChunk = m_HeapChunks.f_FindLargestLessThanEqual(_pMemory);
				if (pChunk && (uint8 *)_pMemory < pChunk->f_GetEndAddress())
					return this;
			}
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

		if (pHeader->f_GetMagic() == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic))
		{
			TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
			return pSlab->m_pArena->f_Size(_pMemory, pSlab);
		}

		{
			TCMemoryManagerArenaHeapChunk<t_CParams> const *pChunk;
			bool bIsChunk;
			{
				DMibLockRead(m_HeapChunksLock);
				pChunk = m_HeapChunks.f_FindLargestLessThanEqual(_pMemory);
				bIsChunk = pChunk && (uint8 *)_pMemory < pChunk->f_GetEndAddress();
			}
			if (bIsChunk)
				return pChunk->f_GetHeap()->f_Size(_pMemory, pChunk);
		}

		return m_Allocator.f_TrySize(_pMemory);
	}

	template <typename t_CParams>
	fp32 TCMemoryManager<t_CParams>::f_Overhead(void const * _pMemory)
	{
		uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pMemory + 1, t_CParams::mc_SlabSize);
		CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

		if (pHeader->f_GetMagic() == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic))
		{
			TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
			return pSlab->m_pArena->f_Overhead(_pMemory, pSlab);
		}

		{
			TCMemoryManagerArenaHeapChunk<t_CParams> const *pChunk;
			bool bIsChunk;
			{
				DMibLockRead(m_HeapChunksLock);
				pChunk = m_HeapChunks.f_FindLargestLessThanEqual(_pMemory);
				bIsChunk = pChunk && (uint8 *)_pMemory < pChunk->f_GetEndAddress();
			}
			if (bIsChunk)
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
		m_HeapChunksLock.f_PrepareFork();
		for (auto &Arena : m_NumaArenas)
			Arena.m_BackgroundCleanup.f_PrepareFork();
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::f_ForkedChild()
	{
		auto &ThreadLocal = *m_LocalArena;
		DMibFastCheck(ThreadLocal.m_pArena);
		ThreadLocal.f_ForkedChild();

		for (auto &Arena : m_NumaArenas)
		{
			Arena.m_BackgroundCleanup.f_ForkedChild();;
			Arena.m_PoolThreadLocal.f_ForkedChildLocked();
			Arena.m_Pool.f_ForkedChildLocked();
			Arena.m_FreeSlabsLock.f_ForkedChildLocked();
			Arena.m_ArenasLock.f_ForkedChildLocked();
		}
		m_NumaArenasLock.f_ForkedChildLocked();
		m_HeapChunksLock.f_ForkedChild();
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::f_ForkedParent()
	{
		for (auto &Arena : m_NumaArenas)
			Arena.m_BackgroundCleanup.f_ForkedParent();
		m_HeapChunksLock.f_ForkedParent();
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::f_Lock()
	{
		m_NumaArenasLock.f_Lock();
		for (auto &Arena : m_NumaArenas)
			Arena.m_BackgroundCleanup.f_Lock();

		m_HeapChunksLock.f_Lock();
		for (auto &Arena : m_NumaArenas)
		{
			Arena.m_ArenasLock.f_Lock();
			Arena.m_FreeSlabsLock.f_Lock();
			Arena.m_Pool.f_Lock();
			Arena.m_PoolThreadLocal.f_Lock();
		}
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::f_Unlock()
	{
		for (auto &Arena : m_NumaArenas)
		{
			Arena.m_PoolThreadLocal.f_Unlock();
			Arena.m_Pool.f_Unlock();
			Arena.m_FreeSlabsLock.f_Unlock();
			Arena.m_ArenasLock.f_Unlock();
		}
		m_HeapChunksLock.f_Unlock();
		for (auto &Arena : m_NumaArenas)
			Arena.m_BackgroundCleanup.f_Unlock();
		m_NumaArenasLock.f_Unlock();
	}

	template <typename t_CParams>
	inline_always void *TCMemoryManager<t_CParams>::f_AllocWithSize(mint &_Size)
	{
		return f_AllocAlignedWithSize(_Size, 1);
	}

	template <typename t_CParams>
	inline_always void *TCMemoryManager<t_CParams>::f_Alloc(mint _Size)
	{
		return f_AllocAligned(_Size, 1);
	}

	template <typename t_CParams>
	inline_always void *TCMemoryManager<t_CParams>::f_AllocInline(mint _Size)
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
	inline_always void *TCMemoryManager<t_CParams>::f_AllocWithSizeInline(mint &_Size)
	{
		return f_AllocAlignedWithSize(_Size, 1);
	}

	template <typename t_CParams>
	inline_never void *TCMemoryManager<t_CParams>::f_AllocAlignedWithSize(mint &_Size, mint _Alignment)
	{
		return f_AllocAlignedWithSizeInline(_Size, _Alignment);
	}

	template <typename t_CParams>
	inline_always void *TCMemoryManager<t_CParams>::f_AllocAligned(mint _Size, mint _Alignment)
	{
		return f_AllocAlignedInline(_Size, _Alignment);
	}

	template <typename t_CParams>
	inline_never void TCMemoryManager<t_CParams>::fp_AllocBatchSlowPath(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
	{
		auto *pLocalArena = m_LocalArena.f_TryGet();
		if (_Size <= t_CParams::mc_MaxSlabAllocSize)
		{
			if (!pLocalArena) [[unlikely]]
			{
				if (fg_GetSys()->f_ThreadDestroyed()) [[unlikely]]
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
		if (!pLocalArena) [[unlikely]]
		{
			if (fg_GetSys()->f_ThreadDestroyed()) [[unlikely]]
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
				auto pAlloc = pNumaArena->m_Heap.f_AllocAlignedWithSize(Size, _Alignment);
				if (!_Functor(pAlloc, Size))
					break;
			}
			return;
		}
		while (true)
		{
			mint Size = _Size;
			DMibMemLightweightTrack
				(
					{
						if (fsp_ShouldTrackAlloc(pLocalArena))
							pLocalArena->f_TrackAlloc(m_Allocator.f_SizePadded(_Size));
					}
				)
			;
			auto pRet = m_Allocator.f_AllocAlignedWithSize(_Size, _Alignment, t_CParams::mc_AllocationFlags, pNumaArena->m_NumaNode);
			if constexpr (mc_EnableCallbacks)
				this->f_OnAlloc((uint8 *)pRet, _Size);
			if (!_Functor(pRet, Size))
				break;
		}
	}

	template <typename t_CParams>
	inline_never void *TCMemoryManager<t_CParams>::fp_AllocAlignedSlowPath(mint &_Size, mint _Alignment)
	{
		auto *pLocalArena = m_LocalArena.f_TryGet();
		if (_Size <= t_CParams::mc_MaxSlabAllocSize)
		{
			if (!pLocalArena) [[unlikely]]
			{
				if (fg_GetSys()->f_ThreadDestroyed()) [[unlikely]]
					return fp_AllocWithTempCheckout(_Size);
				pLocalArena = &(*m_LocalArena);
			}
			auto &LocalArena = *pLocalArena;
			auto ReentrantScope = LocalArena.f_Reentrant();
			if (LocalArena.m_pArena)
				return LocalArena.m_pArena->f_AllocWithSize(_Size);
			return fp_AllocWithCheckout(_Size, LocalArena);
		}

		TCMemoryManagerNumaArena<t_CParams> *pNumaArena;
		if (!pLocalArena) [[unlikely]]
		{
			if (fg_GetSys()->f_ThreadDestroyed()) [[unlikely]]
				pNumaArena = fp_GetAnyNumaArena();
			else
				pNumaArena = m_LocalArena->m_pNumaArena;
		}
		else
			pNumaArena = pLocalArena->m_pNumaArena;

		if (_Size <= t_CParams::mc_MaxHeapAllocSize)
			return pNumaArena->m_Heap.f_AllocAlignedWithSize(_Size, _Alignment);

		DMibMemLightweightTrack
			(
				{
					if (fsp_ShouldTrackAlloc(pLocalArena))
						pLocalArena->f_TrackAlloc(m_Allocator.f_SizePadded(_Size));
				}
			)
		;
		auto pRet = m_Allocator.f_AllocAlignedWithSize(_Size, _Alignment, t_CParams::mc_AllocationFlags, pNumaArena->m_NumaNode);
		if constexpr (mc_EnableCallbacks)
			this->f_OnAlloc((uint8 *)pRet, _Size);
		return pRet;
	}

	template <typename t_CParams>
	inline_always void *TCMemoryManager<t_CParams>::f_AllocAlignedInline(mint _Size, mint _Alignment)
	{
		_Size = fg_AlignUp(_Size, _Alignment);

		if (_Size <= t_CParams::mc_MaxSlabAllocSize) [[likely]]
		{
			auto *pLocalArena = m_LocalArena.f_TryGet();
			if (!pLocalArena) [[unlikely]]
				goto l_SlowPath;
			auto &LocalArena = *pLocalArena;
			auto ReentrantScope = LocalArena.f_Reentrant();
			if (LocalArena.m_pArena) [[likely]]
				return LocalArena.m_pArena->f_AllocWithSize(_Size);
			return fp_AllocWithCheckout(_Size, LocalArena);
		}

	l_SlowPath:
		return fp_AllocAlignedSlowPath(_Size, _Alignment);
	}

	template <typename t_CParams>
	inline_always void *TCMemoryManager<t_CParams>::f_AllocAlignedWithSizeInline(mint &_Size, mint _Alignment)
	{
		_Size = fg_AlignUp(_Size, _Alignment);

		if (_Size <= t_CParams::mc_MaxSlabAllocSize) [[likely]]
		{
			auto *pLocalArena = m_LocalArena.f_TryGet();
			if (!pLocalArena) [[unlikely]]
				goto l_SlowPath;
			auto &LocalArena = *pLocalArena;
			auto ReentrantScope = LocalArena.f_Reentrant();
			if (LocalArena.m_pArena) [[likely]]
				return LocalArena.m_pArena->f_AllocWithSize(_Size);
			return fp_AllocWithCheckout(_Size, LocalArena);
		}

	l_SlowPath:
		return fp_AllocAlignedSlowPath(_Size, _Alignment);
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::f_AllocBatch(mint _Size, mint _Alignment, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
	{
		_Size = fg_AlignUp(_Size, _Alignment);

		if (_Size <= t_CParams::mc_MaxSlabAllocSize) [[likely]]
		{
			auto *pLocalArena = m_LocalArena.f_TryGet();
			if (!pLocalArena) [[unlikely]]
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
	inline_never void TCMemoryManager<t_CParams>::f_Free(void *_pMemory, mint _Size)
	{
		return f_FreeInline(_pMemory, _Size);
	}

	template <typename t_CParams>
	inline_never void TCMemoryManager<t_CParams>::f_FreeNoSize(void *_pMemory)
	{
		return f_FreeNoSizeInline(_pMemory);
	}

	template <typename t_CParams>
	inline_never void TCMemoryManager<t_CParams>::fp_FreeSlowPath(void * _pMemory, mint _Size)
	{
		uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pMemory + 1, t_CParams::mc_SlabSize);
		CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

		if ((_Size != 0 && _Size <= t_CParams::mc_MaxSlabAllocSize) || (_Size == 0 && pHeader->f_GetMagic() == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic))) [[likely]]
		{
			DMibFastCheck(pHeader->f_GetMagic() == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic));

			TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);

			auto *pLocalArena = m_LocalArena.f_TryGet();
			if (!pLocalArena) [[unlikely]]
			{
				if (fg_GetSys()->f_ThreadDestroyed()) [[unlikely]]
				{
					pSlab->m_pArena->f_FreeOtherThread(_pMemory, pSlab, nullptr);
					return;
				}
				pLocalArena = &(*m_LocalArena);
			}

			auto &LocalArena = *pLocalArena;

			DMibMemLightweightTrack
				(
					{
						if (fsp_ShouldTrackAlloc(&LocalArena))
							LocalArena.f_TrackFree(pSlab->m_pArena->f_Size(_pMemory, pSlab));
					}
				)
			;

			auto ReentrantScope = LocalArena.f_Reentrant();
			if (pSlab->m_pArena == LocalArena.m_pArena)
				pSlab->m_pArena->f_FreeThisThread(_pMemory, pSlab);
			else
				pSlab->m_pArena->f_FreeOtherThread(_pMemory, pSlab, &LocalArena);
			return;
		}

		if (_Size == 0 || _Size <= t_CParams::mc_MaxHeapAllocSize)
		{
			TCMemoryManagerArenaHeapChunk<t_CParams> *pChunk;
			bool bIsChunk;
			{
				DMibLockRead(m_HeapChunksLock);
				pChunk = m_HeapChunks.f_FindLargestLessThanEqual(_pMemory);
				bIsChunk = pChunk && (uint8 *)_pMemory < pChunk->f_GetEndAddress();
			}

   			if (bIsChunk)
			{
				DMibMemLightweightTrack
					(
						{
							auto *pLocalArena = m_LocalArena.f_TryGet();
							if (fsp_ShouldTrackAlloc(pLocalArena))
								pLocalArena->f_TrackFree(pChunk->f_GetHeap()->f_Size(_pMemory, pChunk));
						}
					)
				;
				pChunk->f_GetHeap()->f_Free(_pMemory, pChunk);
				return;
			}
			else
			{
				DMibFastCheck(_Size == 0);
			}
		}

		if constexpr (mc_EnableCallbacks)
			this->f_OnFree((uint8 *)_pMemory);

		DMibMemLightweightTrack
			(
				{
					auto *pLocalArena = m_LocalArena.f_TryGet();
					if (fsp_ShouldTrackAlloc(pLocalArena))
						pLocalArena->f_TrackFree(m_Allocator.f_Size(_pMemory));
				}
			)
		;

		if (_Size)
			m_Allocator.f_Free(_pMemory, _Size);
		else
			m_Allocator.f_FreeNoSize(_pMemory);
	}

	template <typename t_CParams>
	inline_always void TCMemoryManager<t_CParams>::f_FreeInline(void *_pMemory, mint _Size)
	{
		if (!_pMemory)
			return;

		DMibFastCheck(_Size != 0);

		if (_Size <= t_CParams::mc_MaxSlabAllocSize) [[likely]]
		{
			uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pMemory + 1, t_CParams::mc_SlabSize);
			CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

			DMibFastCheck(pHeader->f_GetMagic() == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic));

			TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);

			auto *pLocalArena = m_LocalArena.f_TryGet();
			if (!pLocalArena) [[unlikely]]
				goto l_SlowPath;
			auto &LocalArena = *pLocalArena;

			DMibMemLightweightTrack
				(
					{
						if (fsp_ShouldTrackAlloc(&LocalArena))
							LocalArena.f_TrackFree(pSlab->m_pArena->f_Size(_pMemory, pSlab));
					}
				)
			;

			auto ReentrantScope = LocalArena.f_Reentrant();
			if (pSlab->m_pArena == LocalArena.m_pArena)
				pSlab->m_pArena->f_FreeThisThread(_pMemory, pSlab);
			else
				pSlab->m_pArena->f_FreeOtherThread(_pMemory, pSlab, &LocalArena);
			return;
		}

	l_SlowPath:
		fp_FreeSlowPath(_pMemory, _Size);
	}

	template <typename t_CParams>
	inline_always void TCMemoryManager<t_CParams>::f_FreeNoSizeInline(void *_pMemory)
	{
		if (!_pMemory)
			return;

		uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pMemory + 1, t_CParams::mc_SlabSize);
		CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

		if (pHeader->f_GetMagic() == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic)) [[likely]]
		{
			TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);

			auto *pLocalArena = m_LocalArena.f_TryGet();
			if (!pLocalArena) [[unlikely]]
				goto l_SlowPath;
			auto &LocalArena = *pLocalArena;

			DMibMemLightweightTrack
				(
					{
						if (fsp_ShouldTrackAlloc(&LocalArena))
							LocalArena.f_TrackFree(pSlab->m_pArena->f_Size(_pMemory, pSlab));
					}
				)
			;

			auto ReentrantScope = LocalArena.f_Reentrant();
			if (pSlab->m_pArena == LocalArena.m_pArena)
				pSlab->m_pArena->f_FreeThisThread(_pMemory, pSlab);
			else
				pSlab->m_pArena->f_FreeOtherThread(_pMemory, pSlab, &LocalArena);
			return;
		}

	l_SlowPath:
		fp_FreeSlowPath(_pMemory, 0);
	}

	///
	/// Checkout
	/// ========

	template <typename t_CParams>
	inline_always TCMemoryManagerCheckoutLight<t_CParams> TCMemoryManager<t_CParams>::fp_Checkout(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal)
	{
		DMibFastCheck(!_ThreadLocal.m_bInLightCheckout);
		DMibFastCheck(!_ThreadLocal.m_pArena);
		fp_CheckoutHelper(_ThreadLocal);

#	if DMibEnableSafeCheck > 0
		_ThreadLocal.m_bInLightCheckout = true;
#endif
		return this;
	}

	template <typename t_CParams>
	inline_always TCMemoryManagerArena<t_CParams> *TCMemoryManager<t_CParams>::fp_CheckoutHelper(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal)
	{
		DMibFastCheck(!_ThreadLocal.m_pArena);
		auto pArena = _ThreadLocal.m_pPreferredArena;
		if (pArena) [[likely]]
		{
			if (!pArena->m_Lock.f_TryLockNoSanitize()) [[unlikely]]
			{
				if (m_nMaxArenas) [[unlikely]]
					return fp_CheckoutHelperSlowPath(_ThreadLocal);
				else
				{
					++pArena->m_LockContended;
					pArena->m_Lock.f_LockNoSanitize();
					--pArena->m_LockContended;
				}
			}

			_ThreadLocal.m_pArena = pArena;
			return pArena;
		}

		return fp_CheckoutHelperSlowPath(_ThreadLocal);
	}

	template <typename t_CParams>
	inline_never TCMemoryManagerArena<t_CParams> *TCMemoryManager<t_CParams>::fp_CheckoutHelperSlowPath(TCMemoryManagerThreadLocal<t_CParams> &_ThreadLocal)
	{
		auto pNumaArena = _ThreadLocal.m_pNumaArena;

		if (m_nMaxArenas == 0)
		{
			auto pArena = pNumaArena->f_NewArena();
			_ThreadLocal.m_pArena = pArena;
			_ThreadLocal.m_pPreferredArena = pArena;
			return pArena;
		}

		if (_ThreadLocal.m_pPreferredArena) [[likely]]
		{
			mint iLimitedArena = _ThreadLocal.m_pPreferredArena->m_iLimitedArena;
			auto &LimitedArena = pNumaArena->m_LimitedArenas[iLimitedArena];
			auto *pArena = LimitedArena.f_Load(NAtomic::EMemoryOrder_Relaxed);
			if (pArena->m_Lock.f_TryLockNoSanitize()) [[likely]]
			{
				_ThreadLocal.m_pArena = pArena;
				return pArena;
			}
			if (t_CParams::mc_MaxArenas == 1 || m_nMaxArenas == 1)
			{
				++pArena->m_LockContended;
				pArena->m_Lock.f_LockNoSanitize();
				--pArena->m_LockContended;

				_ThreadLocal.m_pArena = pArena;
				return pArena;
			}
		}

		mint iLimitedArena;
		if (t_CParams::mc_MaxArenas == 1 || m_nMaxArenas == 1)
			iLimitedArena = 0;
		else
			iLimitedArena = _ThreadLocal.m_LimitedRandom.template f_GetValue<uint32>() & (m_nMaxArenas - 1);

		auto &LimitedArena = pNumaArena->m_LimitedArenas[iLimitedArena];
		auto *pArena = LimitedArena.f_Load(NAtomic::EMemoryOrder_Relaxed);
		if (!pArena) [[unlikely]]
		{
			DMibLock(pNumaArena->m_LimitedArenasCreateLock);
			pArena = LimitedArena.f_Load();
			if (!pArena)
			{
				pArena = pNumaArena->f_NewArena();
				pArena->m_iLimitedArena = iLimitedArena;
				_ThreadLocal.m_pArena = pArena;
				LimitedArena.f_Store(pArena);
				return pArena;
			}
		}

		if (pArena->m_Lock.f_TryLockNoSanitize()) [[likely]]
		{
			_ThreadLocal.m_pArena = pArena;
			return pArena;
		}
		else
		{
			++pArena->m_LockContended;
			pArena->m_Lock.f_LockNoSanitize();
			--pArena->m_LockContended;
		}

		_ThreadLocal.m_pArena = pArena;
		return pArena;
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::f_CheckoutManual()
	{
		auto &ThreadLocal = *m_LocalArena;
		if (ThreadLocal.m_pArena)
			ThreadLocal.m_pArena->m_CheckoutCount.f_FetchAdd(1, NAtomic::EMemoryOrder_Relaxed);
		else
			fp_CheckoutHelper(ThreadLocal)->m_CheckoutCount.f_FetchAdd(1, NAtomic::EMemoryOrder_Relaxed);
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
	bool TCMemoryManager<t_CParams>::f_IsCheckedOut()
	{
		if (m_bThreadLocalsDestroyed || fg_GetSys()->f_ThreadDestroyed())
			return false;

		auto &ThreadLocal = *m_LocalArena;
		return !!ThreadLocal.m_pArena;
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::f_LazyReturnCheckout()
	{
		if (!m_LocalArena.f_IsValid()) [[unlikely]]
			return;
		auto &ThreadLocal = *m_LocalArena;
		if (ThreadLocal.m_bLazyCheckout && !ThreadLocal.m_Reentrant)
		{
			ThreadLocal.m_bLazyCheckout = false;
			ThreadLocal.f_ReturnCheckout();
		}
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::f_LazyReturnCheckoutPrevent()
	{
		if (!m_LocalArena.f_IsValid()) [[unlikely]]
			return;
		auto &ThreadLocal = *m_LocalArena;
		++ThreadLocal.m_TemporaryReturnCheckoutCount;
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::f_LazyReturnCheckoutAllow()
	{
		if (!m_LocalArena.f_IsValid()) [[unlikely]]
			return;
		auto &ThreadLocal = *m_LocalArena;
		--ThreadLocal.m_TemporaryReturnCheckoutCount;
	}

	template <typename t_CParams>
	void TCMemoryManager<t_CParams>::f_CanDoLazyCheckout()
	{
		if (m_nMaxArenas != 0)
			return;

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
	TCMemoryManagerCheckout<t_CParams> TCMemoryManager<t_CParams>::f_CheckoutForce()
	{
		if (m_bThreadLocalsDestroyed || fg_GetSys()->f_ThreadDestroyed())
			return nullptr;

		auto &ThreadLocal = *m_LocalArena;
		if (ThreadLocal.m_pArena)
			ThreadLocal.m_pArena->m_CheckoutCount.f_FetchAdd(1, NAtomic::EMemoryOrder_Relaxed);
		else
			fp_CheckoutHelper(ThreadLocal)->m_CheckoutCount.f_FetchAdd(1, NAtomic::EMemoryOrder_Relaxed);

		return this;
	}
	
	template <typename t_CParams>
	TCMemoryManagerCheckout<t_CParams> TCMemoryManager<t_CParams>::f_Checkout()
	{
		if (m_nMaxArenas != 0 || m_bThreadLocalsDestroyed || fg_GetSys()->f_ThreadDestroyed())
			return nullptr;

		auto &ThreadLocal = *m_LocalArena;
		if (ThreadLocal.m_pArena)
			ThreadLocal.m_pArena->m_CheckoutCount.f_FetchAdd(1, NAtomic::EMemoryOrder_Relaxed);
		else
			fp_CheckoutHelper(ThreadLocal)->m_CheckoutCount.f_FetchAdd(1, NAtomic::EMemoryOrder_Relaxed);

		return this;
	}

	template <typename t_CParams>
	CMemoryManagerCheckout TCMemoryManager<t_CParams>::f_CheckoutVirtual()
	{
		if (m_nMaxArenas != 0)
			return nullptr;

		auto &ThreadLocal = *m_LocalArena;
		if (ThreadLocal.m_pArena)
			ThreadLocal.m_pArena->m_CheckoutCount.f_FetchAdd(1, NAtomic::EMemoryOrder_Relaxed);
		else
			fp_CheckoutHelper(ThreadLocal)->m_CheckoutCount.f_FetchAdd(1, NAtomic::EMemoryOrder_Relaxed);

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

	template <typename t_CParams>
	bool TCMemoryManagerCheckout<t_CParams>::f_IsValid() const
	{
		return !!m_pMemoryManager;
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
