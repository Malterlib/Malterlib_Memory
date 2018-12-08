// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::f_AddMessage(CMessage *_pMessage, EMessageType _MessageType)
	{
		while (1)
		{
			mint OldMessage = m_Messages.f_Load(NAtomic::EMemoryOrder_Relaxed);
			_pMessage->m_Next = OldMessage;

			mint Message = (mint)_pMessage | mint(_MessageType);
			if (m_Messages.f_CompareExchangeWeak(OldMessage, Message))
				break;
			yield_cpu;
		}
	}

	template <typename t_CParams>
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_ProcessMessageList(CNormalFreeList *_pFreeList, mint &o_MessageList)
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

			switch (FreeLinkType)
			{
			case EMessageType_FreeNormalBlock:
				{
					CMessage_FreeNormalBlock *pBlock = (CMessage_FreeNormalBlock *)pFreeBlock;
					//DMibDTrace("Freeing normal block: {}" DMibNewLine, pBlock);
					uint8 *pEndOfSlab = fg_AlignUp((uint8 *)pBlock + 1, t_CParams::mc_SlabSize);
					CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

					DMibFastCheck(pHeader->m_Magic == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic));
					TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
					fp_Free(pBlock, pSlab);
				}
				break;
			case EMessageType_FreeSmallBlock:
				{
					CMessage_FreeSmallBlock *pBlock = (CMessage_FreeSmallBlock *)pFreeBlock;
					//DMibDTrace("Freeing small block: {}" DMibNewLine, pBlock->m_pBlock);
					uint8 *pEndOfSlab = fg_AlignUp((uint8 *)pBlock->m_pBlock + 1, t_CParams::mc_SlabSize);
					CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

					DMibFastCheck(pHeader->m_Magic == NPrivate::fg_CalcMagic(pEndOfSlab, m_Magic));
					TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
					fp_Free(pBlock->m_pBlock, pSlab);
					m_pMemoryManager->f_Free(pBlock, sizeof(CMessage_FreeSmallBlock));
				}
				break;
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
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_ProcessMessages(CNormalFreeList *_pFreeList)
	{
		if (_pFreeList)
		{
			if (m_DeferredMessages)
			{
				if (fp_ProcessMessageList(_pFreeList, m_DeferredMessages))
					return true;
			}
			m_DeferredMessages = m_Messages.f_Exchange(0);
			return fp_ProcessMessageList(_pFreeList, m_DeferredMessages);
		}

		bool bDoneSomething = false;

		if (m_DeferredMessages)
		{
			bDoneSomething = true;
			fp_ProcessMessageList(_pFreeList, m_DeferredMessages);
		}

		if (auto Messages = m_Messages.f_Exchange(0))
		{
			bDoneSomething = true;
			fp_ProcessMessageList(_pFreeList, Messages);
		}

		return bDoneSomething;
	}

	template <typename t_CParams>
	inline_small void TCMemoryManagerArena<t_CParams>::fp_CheckMessages()
	{
		if (m_Messages.f_Load(NAtomic::EMemoryOrder_Relaxed))
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
				m_bRequestedCleanup = true;

				{
					DMibLock(m_pNumaArena->m_ArenasLock);
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
		if (m_Messages.f_Load(NAtomic::EMemoryOrder_Relaxed))
		{
			return fp_ProcessMessages(nullptr);
		}
		return false;
	}
}
