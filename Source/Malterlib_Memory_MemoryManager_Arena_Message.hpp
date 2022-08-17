// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::f_AddMessage(CMessage *_pMessage, EMessageType _MessageType, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena)
	{
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
			if (!OldMessage)
				bWasEmpty = true;

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
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_ProcessMessageList(CMemoryManagerSubSlab_NormalFreeList *_pFreeList, mint &o_MessageList)
	{
		mint Messages = o_MessageList;

		smint nToProcess = TCLimitsInt<smint>::mc_Max;

		bool bFound;

		if (_pFreeList)
		{
			bFound = false;
			nToProcess = 1024;
		}
		else
			bFound = true;

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

			if (!bFound)
			{
				if (!_pFreeList->f_IsEmpty())
					bFound = true;
				--nToProcess;
			}
			else if (--nToProcess <= 0)
				break;
		}
		o_MessageList = Messages;
		return bFound;
	}

	template <typename t_CParams>
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_ProcessMessages(CMemoryManagerSubSlab_NormalFreeList *_pFreeList)
	{
		if (_pFreeList)
		{
			for (auto &Messages : m_DeferredMessages)
			{
				if (Messages)
				{
					if (fp_ProcessMessageList(_pFreeList, Messages))
						return true;
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
				for (auto &Messages : m_DeferredMessages)
				{
					if (Messages)
					{
						if (fp_ProcessMessageList(_pFreeList, Messages))
							return true;
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
				fp_ProcessMessageList(nullptr, Messages);
			}
		}

		if (m_MessagesAvailable.f_Exchange(0))
		{
			bDoneSomething = true;
			for (auto &Messages : m_SpreadMessages)
			{
				auto ToProcess = Messages.m_Messages.f_Exchange(0);
				fp_ProcessMessageList(nullptr, ToProcess);
			}
		}

		return bDoneSomething;
	}

	template <typename t_CParams>
	inline_small void TCMemoryManagerArena<t_CParams>::fp_CheckMessages()
	{
		if (m_MessagesAvailable.f_Load(NAtomic::EMemoryOrder_Relaxed))
		{
			fp_ProcessMessages(nullptr);
		}
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
		bool bNeedCleanup = false;
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
		if (m_MessagesAvailable.f_Load(NAtomic::EMemoryOrder_Relaxed))
		{
			return fp_ProcessMessages(nullptr);
		}
		return false;
	}
}
