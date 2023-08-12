// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::f_AddMessage(CMessage *_pMessage, EMessageType _MessageType, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena)
	{
		DMibFastCheck(_pMessage);
		DMibFastCheck(!((mint)_pMessage & mint(3)));

		mint RandomIndex;
		if (_pLocalArena)
			RandomIndex = _pLocalArena->m_RandomIndex & (mc_MessagesSpread - 1);
		else
			RandomIndex = m_Magic & (mc_MessagesSpread - 1);

		auto &Messages = m_SpreadMessages[RandomIndex].m_Messages;
		bool bWasEmpty = false;
		while (1)
		{
			mint OldMessage = Messages.f_Load(NAtomic::EMemoryOrder_Relaxed);
			_pMessage->m_Next = OldMessage;
			bWasEmpty = !OldMessage;

			mint Message = (mint)_pMessage | mint(_MessageType);
			if (Messages.f_CompareExchangeWeak(OldMessage, Message))
				break;

			yield_cpu;
		}

		if (!bWasEmpty)
			return;

		m_MessagesAvailable.f_FetchAdd(1);
	}

	template <typename t_CParams>
	template <bool tf_bAbortable, bool tf_bFreeList>
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_ProcessMessageList
		(
			mint &o_MessageList
			, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena
 			, CMemoryManagerSubSlab_NormalFreeList *_pFreeList
			, smint &_nToProcess
		)
	{
		mint Messages = o_MessageList;

		while (Messages)
		{
			EMessageType FreeLinkType = (EMessageType)(Messages & 3);
			CMessage *pFreeBlock = (CMessage *)(Messages & (~mint(3)));
			mint NextMessage = pFreeBlock->m_Next;

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
					fp_Free(pBlock, pSlab);
				}
				else
				{
					DMibFastCheck(FreeLinkType == EMessageType_FreeSmallBlock);
					CMessage_FreeSmallBlock *pBlock = (CMessage_FreeSmallBlock *)pFreeBlock;
					uint8 *pEndOfSlab = fg_AlignUp((uint8 *)pBlock->m_pBlock + 1, t_CParams::mc_SlabSize);
					CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

					DMibFastCheck(pHeader->f_GetMagic() == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic));
					TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
					fp_Free(pBlock->m_pBlock, pSlab);
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
				fp_Free(pBlock, pSlab);
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

		for (auto &Messages : m_DeferredMessages)
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

		if (m_MessagesAvailable.f_Exchange(0))
		{
			mint iMessages = 0;
			for (auto &Messages : m_SpreadMessages)
			{
				m_DeferredMessages[iMessages] = Messages.m_Messages.f_Exchange(0);
				++iMessages;
			}
		}

		bool bNeedProcess = false;
		auto Cleanup = g_OnScopeExit / [&]
			{
				if (bNeedProcess)
					fp_RequestCleanup();
			}
		;

		for (auto &Messages : m_DeferredMessages)
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

		return bDoneSomething;
	}

	template <typename t_CParams>
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_ProcessMessages(CMemoryManagerSubSlab_NormalFreeList *_pFreeList)
	{
		if (_pFreeList)
		{
			smint nToProcess = 128;

			for (auto &Messages : m_DeferredMessages)
			{
				if (Messages)
				{
					if (fp_ProcessMessageList<false, true>(Messages, nullptr, _pFreeList, nToProcess))
						return true;
					else if (nToProcess <= 0)
						return false;
				}
			}
			if (m_MessagesAvailable.f_Exchange(0))
			{
				bool bNeedProcess = false;
				auto Cleanup = g_OnScopeExit / [&]
					{
						if (bNeedProcess)
							fp_RequestCleanup();
					}
				;

				mint iMessages = 0;
				for (auto &Messages : m_SpreadMessages)
				{
					m_DeferredMessages[iMessages] = Messages.m_Messages.f_Exchange(0);
					++iMessages;
				}
				for (auto &Messages : m_DeferredMessages)
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
			}
			return false;
		}

		bool bDoneSomething = false;

		for (auto &Messages : m_DeferredMessages)
		{
			if (Messages)
			{
				bDoneSomething = true;
				smint nToProcess = 0;
				fp_ProcessMessageList<false, false>(Messages, nullptr, nullptr, nToProcess);
			}
		}

		if (m_MessagesAvailable.f_Exchange(0))
		{
			bDoneSomething = true;
			for (auto &Messages : m_SpreadMessages)
			{
				auto ToProcess = Messages.m_Messages.f_Exchange(0);
				smint nToProcess = 0;
				fp_ProcessMessageList<false, false>(ToProcess, nullptr, nullptr, nToProcess);
			}
		}

		return bDoneSomething;
	}

	template <typename t_CParams>
	inline_small void TCMemoryManagerArena<t_CParams>::fp_CheckMessages()
	{
		fp_ProcessMessages(nullptr);
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
		bool bNeedCleanup = m_MessagesAvailable.f_Load(NAtomic::EMemoryOrder_Relaxed) != 0;
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
