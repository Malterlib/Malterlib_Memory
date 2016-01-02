// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

namespace NMib
{
	namespace NMem
	{

		/***************************************************************************************************\
		|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
		| Misc																								|
		|___________________________________________________________________________________________________|
		\***************************************************************************************************/

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::fp_FillFreeBlock(CBlock_Free *_pFreeBlock)
		{
			if (!f_CanCommit() && CFillDebug::EDoFills)
			{					
				mint Size = _pFreeBlock->f_GetSize();
				if (_pFreeBlock->f_Type() == EBlockType_FreeExtended)
					CFillDebug::fs_FillFree((uint8 *)_pFreeBlock + EBlockFreeExtendedSizeStart, Size - EBlockFreeExtendedSize);
				else if (_pFreeBlock->f_Type() == EBlockType_Free)
					CFillDebug::fs_FillFree((uint8 *)_pFreeBlock + sizeof(CBlock_Free), Size - sizeof(CBlock_Free));
				else
					DMibPDebugBreak;
			}
		}
		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::fp_CheckFreeBlock(CBlock_Free *_pFreeBlock, mint _Size)
		{
			if (!f_CanCommit() && CFillDebug::EDoFills)
			{					
				mint Size = _Size;
				if (_pFreeBlock->f_Type() == EBlockType_FreeExtended)
					CFillDebug::fs_CheckFree((uint8 *)_pFreeBlock + EBlockFreeExtendedSizeStart, Size - EBlockFreeExtendedSize);
				else if (_pFreeBlock->f_Type() == EBlockType_Free)
					CFillDebug::fs_CheckFree((uint8 *)_pFreeBlock + sizeof(CBlock_Free), Size - sizeof(CBlock_Free));
				else
					DMibPDebugBreak;
			}
		}
		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::fp_CheckAllocatedBlock(CBlock *_pBlock)
		{
			if (CFillDebug::EDoFills)
			{	
				void *pMem = (uint8 *)_pBlock + EBlockPreSize;
				mint Size = _pBlock->f_GetSizeNormal();
				// Check that noone has overwritten us

				CFillDebug::fs_CheckPreGuard((uint8 *)pMem - EBlockPreGuardOffset, EBlockPreGuardSize);
				CFillDebug::fs_CheckPostGuard((uint8 *)_pBlock + (Size - EBlockPostGuardSize), EBlockPostGuardSize);
			}
		}

	}
}
