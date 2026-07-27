// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::f_AddMessage(CMessage *_pMessage, EMessageType _MessageType, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena)
	{
		DMibFastCheck(_pMessage);
		DMibFastCheck(!((umint)_pMessage & umint(3)));

		umint RandomIndex;
		if (_pLocalArena)
			RandomIndex = _pLocalArena->m_RandomIndex & (mc_MessagesSpread - 1);
		else
			RandomIndex = m_Magic & (mc_MessagesSpread - 1);

		auto &Messages = m_MessageState.m_SpreadMessages[RandomIndex].m_Messages;
		bool bWasEmpty = false;
		while (1)
		{
			umint OldMessage = Messages.f_Load(NAtomic::gc_MemoryOrder_Relaxed);
			_pMessage->m_Next = OldMessage;
			bWasEmpty = !OldMessage;

			umint Message = (umint)_pMessage | umint(_MessageType);
			if (Messages.f_CompareExchangeWeak(OldMessage, Message))
				break;

			yield_cpu;
		}

		if (!bWasEmpty)
			return;

		m_MessageState.m_MessagesAvailable.f_FetchAdd(1);
	}

	template <typename t_CParams>
	inline_always bool TCMemoryManagerArena<t_CParams>::fp_HasRemoteFrees() const
	{
		if constexpr (mc_bUseRemoteFreeReap)
			return m_RemoteFreeState.m_RemoteFreeSlabs.f_Load(NAtomic::gc_MemoryOrder_Relaxed) != 0;
		else
			return false;
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_PushRemoteFreeSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab)
	{
		auto &StackNext = _pSlab->m_pRemoteFreeNotify->m_StackNext;
		while (1)
		{
			umint OldHead = m_RemoteFreeState.m_RemoteFreeSlabs.f_Load(NAtomic::gc_MemoryOrder_Relaxed);
			StackNext.f_Store(OldHead, NAtomic::gc_MemoryOrder_Relaxed);
			if (m_RemoteFreeState.m_RemoteFreeSlabs.f_CompareExchangeWeak(OldHead, (umint)_pSlab, NAtomic::gc_MemoryOrder_Release))
				break;

			yield_cpu;
		}

		if constexpr (mc_bUseFreeMessages)
			m_MessageState.m_MessagesAvailable.f_FetchAdd(1);
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::fp_ReapRemoteSubSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab, umint _iSubSlab)
	{
		if constexpr (mc_bSubSlabStore && t_CParams::mc_bReapInCleanup)
		{
			umint SlabType = _pSlab->m_SlabType;
			umint SlabBucket = _pSlab->f_GetSubSlabDataType()[_iSubSlab].m_Type - 2;

			umint MinSize = t_CParams::mc_SlabTypeFast[SlabType].m_SubSlabMultiplier * t_CParams::mc_SubSlabSize;
			umint SlabBucketStart = umint(1) << SlabBucket;
			umint AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));
			uint8 *pBase = _pSlab->f_GetSlabStart() + _iSubSlab * MinSize;

			// Multi-sub-slab single blocks never use the free store; the pending state only
			// carries the drain of their one allocation
			bool bSingleBlock = AlignedSize > MinSize;

			auto &FreeState = _pSlab->m_pSubSlabFreeState[_iSubSlab];
			auto &Remote = _pSlab->m_pSubSlabRemoteFree[_iSubSlab];

			umint nFreed = 0;
			bool bWasExhausted;

			if constexpr (mc_bSubSlabBitmaps)
			{
				uint64 HeaderBits = Remote.m_Bits.f_Exchange(0, NAtomic::gc_MemoryOrder_Acquire);
				if (!HeaderBits)
					return false; // Stale summary bit from an earlier reap round

				bWasExhausted = FreeState.m_Bits == 0;

				umint nBlocks = 0;
				if constexpr (t_CParams::mc_bReapDenseBitmaps)
				{
					if (!bSingleBlock)
						nBlocks = fsp_GetSubSlabBlocks(SlabType, SlabBucket);
				}

				if (nBlocks > 64)
				{
					// Dense sub-slab: the header slot summarizes the in-band pending words
					umint nWords = (nBlocks + 63) / 64;
					auto pPendingWords = (NAtomic::TCAtomic<uint64> *)(pBase + fsp_BitmapRemoteWordsOffset(nWords));
					uint64 *pFreeWords = (uint64 *)pBase;
					uint64 NewSummary = 0;

					for (uint64 Summary = HeaderBits; Summary; Summary &= Summary - 1)
					{
						umint iWord = NMib::fg_GetLowestBitSetNoZero(Summary);
						uint64 Pending = pPendingWords[iWord].f_Exchange(0, NAtomic::gc_MemoryOrder_Acquire);
						if (!Pending)
							continue; // Stale word bit from an earlier reap round

						DMibFastCheck(!(pFreeWords[iWord] & Pending)); // Double free

						nFreed += fg_NumBitsSet(Pending);

						if constexpr (mc_EnableCallbacks)
						{
							for (uint64 Bits = Pending; Bits; Bits &= Bits - 1)
							{
								uint8 *pBlock = pBase + (iWord * 64 + NMib::fg_GetLowestBitSetNoZero(Bits)) * AlignedSize;
								this->f_OnFree(pBlock);
								this->f_OnFillFree(pBlock, AlignedSize);
							}
						}

						pFreeWords[iWord] |= Pending;
						NewSummary |= uint64(1) << iWord;

						// With notifier callbacks the loop calls out, and a callback that frees
						// into this sub-slab must not observe a non-empty word whose summary bit
						// is still missing; the free path treats that pairing as an invariant
						if constexpr (mc_EnableCallbacks)
							FreeState.m_Bits |= NewSummary;
					}

					if (nFreed == 0)
						return false; // Everything was stale

					// One update of the header line instead of one per dirty word: as far as
					// the compiler knows the in-band word stores above may alias it
					FreeState.m_Bits |= NewSummary;
				}
				else
				{
					if (!bSingleBlock)
						DMibFastCheck(!(FreeState.m_Bits & HeaderBits)); // Double free

					nFreed = fg_NumBitsSet(HeaderBits);

					if constexpr (mc_EnableCallbacks)
					{
						for (uint64 Bits = HeaderBits; Bits; Bits &= Bits - 1)
						{
							uint8 *pBlock = pBase + NMib::fg_GetLowestBitSetNoZero(Bits) * AlignedSize;
							this->f_OnFree(pBlock);
							this->f_OnFillFree(pBlock, AlignedSize);
						}
					}

					if (!bSingleBlock)
						FreeState.m_Bits |= HeaderBits;
				}
			}
			else
			{
				uint32 Head = Remote.m_Head.f_Exchange(0, NAtomic::gc_MemoryOrder_Acquire);
				if (!Head)
					return false; // Stale summary bit from an earlier reap round

				bWasExhausted
					= FreeState.m_pFreeHead == nullptr
					&& FreeState.m_CarveOffset >= FreeState.m_CarveEnd
				;

				uint16 Offset = uint16(Head - 1);
				while (1)
				{
					uint8 *pBlock = pBase + umint(Offset) * 4;
					uint16 Next = *(uint16 *)pBlock;

					++nFreed;
					if constexpr (mc_EnableCallbacks)
					{
						this->f_OnFree(pBlock);

						// A single block goes pending as a whole and its remote chain word was
						// already consumed above, so the whole block is filled; chained blocks
						// keep the pointer sized metadata region unprotected because the chain
						// conversion below writes a full pointer over the block start
						if (bSingleBlock)
							this->f_OnFillFree(pBlock, AlignedSize);
						else
							this->f_OnFillFree(pBlock + sizeof(void *), AlignedSize - sizeof(void *));
					}

					if (!bSingleBlock)
					{
						// Convert from the remote uint16 links to the local pointer chain; the
						// uint16 next was already read above so the wider store is safe
						*(void **)pBlock = FreeState.m_pFreeHead;
						FreeState.m_pFreeHead = pBlock;
					}

					if (Next == CMemoryManagerSubSlabFreeState::mc_ChainEnd)
						break;

					Offset = Next;
				}
			}

			if (!bSingleBlock && bWasExhausted)
			{
				// A drained sub-slab can still be cached as current (exhaustion is only
				// discovered by the next allocation); the merged blocks are reachable through
				// the free store, so it must not be listed as well
				bool bList = fp_GetCurrentSubSlabSlot(SlabType, SlabBucket, AlignedSize)->m_pFreeState != &FreeState;

				if constexpr (!mc_bSubSlabBitmaps)
				{
					// Single-block sub-slabs are never listed; they are reclaimed through the
					// pending path below
					bList = bList && umint(FreeState.m_CarveEnd - FreeState.m_CarveStart) * 4 > AlignedSize;
				}

				if (bList)
				{
					// The sub-slab was exhausted so it is neither listed nor current; remote
					// frees are cold, append at the tail like message-processed frees
					if constexpr (t_CParams::mc_bGlobalAddressOrder)
						m_bSubSlabListsDirty = true;
					fp_GetNormalFreeList(SlabType, SlabBucket, AlignedSize)->f_UnsafeInsertLast(&_pSlab->m_pSubSlabNodes[_iSubSlab]);
				}
			}

			auto &Data = _pSlab->f_GetSubSlabDataAlloc()[_iSubSlab];

			DMibFastCheck(Data.m_nAllocs >= nFreed);
			Data.m_nAllocs = decltype(Data.m_nAllocs)(Data.m_nAllocs - nFreed);

			if (Data.m_nAllocs == 0) [[unlikely]]
			{
				if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_NoCleanup) != 0)
				{
				}
				else if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_Allocs) != 0)
				{
					if constexpr (t_CParams::mc_MaxPendingSubSlabs)
					{
						if (_pSlab->m_nPendingSubSlabs >= t_CParams::mc_MaxPendingSubSlabs)
							return fp_FreeSubSlab(_pSlab, uint32(_iSubSlab));
					}

					_pSlab->f_SetPendingBit(_iSubSlab);

					fp_SlabHasGarbage(_pSlab);

					++_pSlab->m_nPendingSubSlabs;
				}
				else
					return fp_FreeSubSlab(_pSlab, uint32(_iSubSlab)); // _pSlab is potentially invalid when this returns true
			}

			return false;
		}
		else
			return false;
	}

	template <typename t_CParams>
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_ReapRemoteSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab)
	{
		if constexpr (mc_bSubSlabStore && t_CParams::mc_bReapInCleanup)
		{
			uint64 Top = _pSlab->m_pRemoteFreeNotify->m_SummaryTop.f_Exchange(0, NAtomic::gc_MemoryOrder_Acquire);

			for (; Top; Top &= Top - 1)
			{
				umint iWord = NMib::fg_GetLowestBitSetNoZero(Top);
				uint64 Summary = _pSlab->m_pRemoteFreeSummary[iWord].f_Exchange(0, NAtomic::gc_MemoryOrder_Acquire);

				for (; Summary; Summary &= Summary - 1)
				{
					umint iSubSlab = iWord * 64 + NMib::fg_GetLowestBitSetNoZero(Summary);
					if (fp_ReapRemoteSubSlab(_pSlab, iSubSlab))
					{
						// The slab went fully free and left through the free lists; remaining
						// summary bits must be stale because pending frees imply allocations, so
						// stop touching the slab
						return true;
					}
				}
			}

			return false;
		}
		else
			return false;
	}

	template <typename t_CParams>
	template <bool tf_bAbortable>
	bool TCMemoryManagerArena<t_CParams>::fp_ReapRemoteFrees(TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena, CNormalFreeStoreList *_pFreeList)
	{
		if constexpr (mc_bSubSlabStore && t_CParams::mc_bReapInCleanup)
		{
			bool bDoneSomething = false;

			while (1)
			{
				if constexpr (tf_bAbortable)
				{
					if (f_IsContended(_pLocalArena))
					{
						if (m_RemoteFreeState.m_RemoteFreeSlabs.f_Load(NAtomic::gc_MemoryOrder_Relaxed))
							fp_RequestCleanup();
						return bDoneSomething;
					}
				}

				if (_pFreeList && !_pFreeList->f_IsEmpty())
					return bDoneSomething;

				// Single consumer: slabs are only popped while holding the arena. A slab cannot
				// be pushed again while still in the stack (pushes are gated on the summary top
				// word's empty-to-pending transition), so the pop cannot suffer ABA.
				TCMemoryManagerSlabShared<t_CParams> *pSlab;
				while (1)
				{
					umint OldHead = m_RemoteFreeState.m_RemoteFreeSlabs.f_Load(NAtomic::gc_MemoryOrder_Acquire);
					if (!OldHead)
						return bDoneSomething;

					pSlab = (TCMemoryManagerSlabShared<t_CParams> *)OldHead;
					umint Next = pSlab->m_pRemoteFreeNotify->m_StackNext.f_Load(NAtomic::gc_MemoryOrder_Relaxed);
					if (m_RemoteFreeState.m_RemoteFreeSlabs.f_CompareExchangeWeak(OldHead, Next, NAtomic::gc_MemoryOrder_Acquire))
						break;

					yield_cpu;
				}

				bDoneSomething = true;
				fp_ReapRemoteSlab(pSlab);
			}
		}
		else
			return false;
	}

	template <typename t_CParams>
	template <bool tf_bAbortable, bool tf_bFreeList>
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_ProcessMessageList
		(
			umint &o_MessageList
			, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena
 			, CNormalFreeStoreList *_pFreeList
			, smint &_nToProcess
		)
	{
		umint Messages = o_MessageList;

		while (Messages)
		{
			EMessageType FreeLinkType = (EMessageType)(Messages & 3);
			CMessage *pFreeBlock = (CMessage *)(Messages & (~umint(3)));
			umint NextMessage = pFreeBlock->m_Next;

			NAtomic::fg_CompilerFence();

			if constexpr (t_CParams::mc_bUseSmallSizes)
			{
				if (FreeLinkType == EMessageType_FreeNormalBlock) [[likely]]
				{
					CMessage_FreeNormalBlock *pBlock = (CMessage_FreeNormalBlock *)pFreeBlock;
					uint8 *pEndOfSlab = fg_AlignUp((uint8 *)pBlock + 1, t_CParams::mc_SlabSize);
					CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

					DMibFastCheck(pHeader->f_GetMagic() == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic));
					TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
					fp_FreeFromMessage(pBlock, pSlab);
				}
				else
				{
					DMibFastCheck(FreeLinkType == EMessageType_FreeSmallBlock);
					CMessage_FreeSmallBlock *pBlock = (CMessage_FreeSmallBlock *)pFreeBlock;
					uint8 *pEndOfSlab = fg_AlignUp((uint8 *)pBlock->m_pBlock + 1, t_CParams::mc_SlabSize);
					CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

					DMibFastCheck(pHeader->f_GetMagic() == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic));
					TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
					fp_FreeFromMessage(pBlock->m_pBlock, pSlab);
					m_pMemoryManager->f_Free(pBlock, sizeof(CMessage_FreeSmallBlock));
				}
			}
			else
			{
				CMessage_FreeNormalBlock *pBlock = (CMessage_FreeNormalBlock *)pFreeBlock;
				uint8 *pEndOfSlab = fg_AlignUp((uint8 *)pBlock + 1, t_CParams::mc_SlabSize);
				CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

				DMibFastCheck(pHeader->f_GetMagic() == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic));
				TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
				fp_FreeFromMessage(pBlock, pSlab);
			}

			Messages = NextMessage;

			if constexpr (tf_bAbortable)
			{
				if (f_IsContended(_pLocalArena))
					break;
			}

			if constexpr (tf_bFreeList)
			{
				if (!_pFreeList->f_IsEmpty())
					break;

				if (--_nToProcess <= 0)
					break;
			}
		}
		o_MessageList = Messages;
		if constexpr (tf_bFreeList)
			return !_pFreeList->f_IsEmpty();
		return false;
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::fp_ProcessMessagesAbortable(bool &o_bAborted, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena)
	{
		bool bDoneSomething = false;

		if constexpr (mc_bUseFreeMessages)
		{
			for (auto &Messages : m_MessageState.m_DeferredMessages)
			{
				if (Messages)
				{
					bDoneSomething = true;
					smint nToProcess = 0;
					fp_ProcessMessageList<true, false>(Messages, _pLocalArena, nullptr, nToProcess);
					if (f_IsContended(_pLocalArena))
					{
						o_bAborted = true;
						return bDoneSomething;
					}
				}
			}

			if (m_MessageState.m_MessagesAvailable.f_Exchange(0))
			{
				umint iMessages = 0;
				for (auto &Messages : m_MessageState.m_SpreadMessages)
				{
					m_MessageState.m_DeferredMessages[iMessages] = Messages.m_Messages.f_Exchange(0);
					++iMessages;
				}
			}
		}

		bool bNeedProcess = false;
		auto Cleanup = g_OnScopeExit / [&]
			{
				if (bNeedProcess || fp_HasRemoteFrees())
					fp_RequestCleanup();
			}
		;

		if constexpr (mc_bUseFreeMessages)
		{
			for (auto &Messages : m_MessageState.m_DeferredMessages)
			{
				if (Messages)
				{
					bDoneSomething = true;
					smint nToProcess = 0;
					fp_ProcessMessageList<true, false>(Messages, _pLocalArena, nullptr, nToProcess);
					if (Messages)
						bNeedProcess = true;
					if (f_IsContended(_pLocalArena))
					{
						o_bAborted = true;
						return bDoneSomething;
					}
				}
			}
		}

		if (fp_ReapRemoteFrees<true>(_pLocalArena, nullptr))
			bDoneSomething = true;
		if (f_IsContended(_pLocalArena))
			o_bAborted = true;

		return bDoneSomething;
	}

	template <typename t_CParams>
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_ProcessMessages(CNormalFreeStoreList *_pFreeList)
	{
		if (_pFreeList)
		{
			if constexpr (mc_bUseFreeMessages)
			{
				smint nToProcess = 128;

				for (auto &Messages : m_MessageState.m_DeferredMessages)
				{
					if (Messages)
					{
						if (fp_ProcessMessageList<false, true>(Messages, nullptr, _pFreeList, nToProcess))
							return true;
						else if (nToProcess <= 0)
							return false;
					}
				}
				if (m_MessageState.m_MessagesAvailable.f_Exchange(0))
				{
					bool bNeedProcess = false;
					auto Cleanup = g_OnScopeExit / [&]
						{
							if (bNeedProcess || fp_HasRemoteFrees())
								fp_RequestCleanup();
						}
					;

					umint iMessages = 0;
					for (auto &Messages : m_MessageState.m_SpreadMessages)
					{
						m_MessageState.m_DeferredMessages[iMessages] = Messages.m_Messages.f_Exchange(0);
						++iMessages;
					}
					for (auto &Messages : m_MessageState.m_DeferredMessages)
					{
						if (Messages)
						{
							auto bProcessResult = fp_ProcessMessageList<false, true>(Messages, nullptr, _pFreeList, nToProcess);
							if (Messages)
								bNeedProcess = true;

							if (bProcessResult)
								return true;
							else if (nToProcess <= 0)
								return false;
						}
					}

					if constexpr (mc_bSubSlabStore && t_CParams::mc_bReapInCleanup)
					{
						fp_ReapRemoteFrees<false>(nullptr, _pFreeList);
						return !_pFreeList->f_IsEmpty();
					}
				}
			}

			if constexpr (mc_bSubSlabStore && t_CParams::mc_bReapInCleanup)
			{
				fp_ReapRemoteFrees<false>(nullptr, _pFreeList);
				return !_pFreeList->f_IsEmpty();
			}

			return false;
		}

		bool bDoneSomething = false;

		if constexpr (mc_bUseFreeMessages)
		{
			for (auto &Messages : m_MessageState.m_DeferredMessages)
			{
				if (Messages)
				{
					bDoneSomething = true;
					smint nToProcess = 0;
					fp_ProcessMessageList<false, false>(Messages, nullptr, nullptr, nToProcess);
				}
			}

			if (m_MessageState.m_MessagesAvailable.f_Exchange(0))
			{
				bDoneSomething = true;
				for (auto &Messages : m_MessageState.m_SpreadMessages)
				{
					auto ToProcess = Messages.m_Messages.f_Exchange(0);
					smint nToProcess = 0;
					fp_ProcessMessageList<false, false>(ToProcess, nullptr, nullptr, nToProcess);
				}
			}
		}

		if (fp_ReapRemoteFrees<false>(nullptr, nullptr))
			bDoneSomething = true;

		return bDoneSomething;
	}

	template <typename t_CParams>
	inline_small void TCMemoryManagerArena<t_CParams>::fp_CheckMessages()
	{
		fp_ProcessMessages(nullptr);
	}

	template <typename t_CParams>
	inline_small bool TCMemoryManagerArena<t_CParams>::f_HasPendingMessages() const
	{
		if constexpr (mc_bUseFreeMessages)
		{
			if (m_MessageState.m_MessagesAvailable.f_Load(NAtomic::gc_MemoryOrder_Relaxed))
				return true;
		}

		if (fp_HasRemoteFrees())
			return true;

		if constexpr (mc_bUseFreeMessages)
		{
			for (auto &Messages : m_MessageState.m_DeferredMessages)
			{
				if (Messages)
					return true;
			}
		}

		return false;
	}

	template <typename t_CParams>
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_CheckCleanupNumaFree()
	{
		bool bNeedCleanup = false;
		if (m_bWantNumaFreeSlabsCleanup)
		{
			m_bWantNumaFreeSlabsCleanup = false;
			bNeedCleanup = true;
			m_pNumaArena->f_RequestCleanup(ENumaArenaCleanup_FreeSlabs);
		}
		return bNeedCleanup;
	}

	template <typename t_CParams>
	inline_small bool TCMemoryManagerArena<t_CParams>::fp_CheckCleanup()
	{
		bool bNeedCleanup;
		if constexpr (mc_bUseFreeMessages)
			bNeedCleanup = m_MessageState.m_MessagesAvailable.f_Load(NAtomic::gc_MemoryOrder_Relaxed) != 0;
		else
			bNeedCleanup = fp_HasRemoteFrees();
		if (m_bWantCleanup)
		{
			m_bWantCleanup = false;
			if (!m_bRequestedCleanup)
			{
				{
					DMibLock(m_pNumaArena->m_ArenasNeedCleanupLock);
					m_bRequestedCleanup = true;
					m_pNumaArena->m_ArenasNeedCleanup.f_InsertFirst(this);
				}
				bNeedCleanup = true;
			}
		}
		return bNeedCleanup;
	}

	template <typename t_CParams>
	inline_small bool TCMemoryManagerArena<t_CParams>::f_ProcessMessages()
	{
		return fp_ProcessMessages(nullptr);
	}

	template <typename t_CParams>
	inline_small bool TCMemoryManagerArena<t_CParams>::f_ProcessMessagesAbortable(bool &o_bAborted, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena)
	{
		return fp_ProcessMessagesAbortable(o_bAborted, _pLocalArena);
	}
}
