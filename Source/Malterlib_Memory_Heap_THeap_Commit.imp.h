// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

namespace NMib
{
	namespace NMem
	{

		/***************************************************************************************************\
		|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
		| Commit																							|
		|___________________________________________________________________________________________________|
		\***************************************************************************************************/

		template <class t_CHeapParams>
		inline_small mint TCHeap<t_CHeapParams>::fsp_CommitSize_FreeStart(mint _BlockSize)
		{
			if (!ENeedExtendedBlocks)
			{
				return sizeof(CBlock_Free);
			}
			else
			{

				if (t_CHeapParams::EbOptimizeForSize)
				{
					if (_BlockSize >= sizeof(CBlock_FreeExtendedBucket))
					{
						return EBlockFreeExtendedSizeStart;
					}
					else 
						return sizeof(CBlock_Free);
				}
				else
				{
					if (_BlockSize >= (mint)ELargestBlock)
					{
						return EBlockFreeExtendedSizeStart;
					}
					else 
						return sizeof(CBlock_Free);
				}
			}
		}

		template <class t_CHeapParams>
		inline_small mint TCHeap<t_CHeapParams>::fsp_CommitSize_FreeExtended()
		{
			if (!ENeedExtendedBlocks)
			{
				return sizeof(CBlock_Free);
			}
			else
			{
				if (t_CHeapParams::EbOptimizeForSize)
				{
					return sizeof(CBlock_FreeExtendedBucket);
				}
				else
				{
					return sizeof(CBlock_FreeExtended);
				}
			}
		}

		template <class t_CHeapParams>
		inline_small mint TCHeap<t_CHeapParams>::fsp_CommitSize_FreeExtendedStart()
		{
			if (!ENeedExtendedBlocks)
			{
				return sizeof(CBlock_Free);
			}
			else
			{
				if (t_CHeapParams::EbOptimizeForSize)
				{
					return EBlockFreeExtendedSizeStart;
				}
				else
				{
					return EBlockFreeExtendedSizeStart;
				}
			}
		}

		template <class t_CHeapParams>
		inline_small mint TCHeap<t_CHeapParams>::fsp_CommitSize_FreeExtendedEnd()
		{
			if (!ENeedExtendedBlocks)
			{
				return 0;
			}
			else
			{
				if (t_CHeapParams::EbOptimizeForSize)
				{
					return EBlockFreeExtendedSizeEnd;
				}
				else
				{
					return EBlockFreeExtendedSizeEnd;
				}
			}
		}

		template <class t_CHeapParams>
		inline_small void TCHeap<t_CHeapParams>::fsp_CommitRange(CChunk *_pChunk, uint8 *_pStart, uint8 *_pEnd)
		{
			_pStart = fp_AlignDown(_pStart);
			_pEnd = fp_AlignUp(_pEnd);
			if (_pEnd > _pStart)
			{
				_pChunk->f_Commit(_pStart, _pEnd - _pStart);
			}
		}

		template <class t_CHeapParams>
		inline_small void TCHeap<t_CHeapParams>::fsp_CommitRangeAlreadyCommitted(CChunk *_pChunk, uint8 *_pStart, uint8 *_pEnd, uint8 *_pAlreadyCommited)
		{
			_pStart = fp_AlignDown(_pStart);
			_pEnd = fp_AlignUp(_pEnd);
			uint8 *pAlreadyCommitedStart = fp_AlignDown(_pAlreadyCommited);
			uint8 *pAlreadyCommitedEnd = fp_AlignUp(_pAlreadyCommited);
			if (_pEnd > pAlreadyCommitedStart && _pEnd <= pAlreadyCommitedEnd)
			{
				_pEnd = pAlreadyCommitedStart;
			}
			if (_pStart >= pAlreadyCommitedStart && _pStart < pAlreadyCommitedEnd)
			{
				_pStart = pAlreadyCommitedEnd;
			}
			if (_pEnd > _pStart)
			{
				_pChunk->f_Commit(_pStart, _pEnd - _pStart);
			}
		}

		template <class t_CHeapParams>
		inline_small void TCHeap<t_CHeapParams>::fsp_DecommitRangeAligned(CChunk *_pChunk, uint8 *_pStart, uint8 *_pEnd)
		{
			if (_pEnd > _pStart)
			{
				_pChunk->f_Commit(_pStart, _pEnd - _pStart);
			}
		}

		template <class t_CHeapParams>
		inline_small void TCHeap<t_CHeapParams>::fsp_CommitDualRange(CChunk *_pChunk, uint8 *_pStart, uint8 *_pEnd, uint8 *_pStart2, uint8 *_pEnd2)
		{
			_pStart = fp_AlignDown(_pStart);
			_pEnd = fp_AlignUp(_pEnd);
			_pStart2 = fp_AlignDown(_pStart2);
			_pEnd2 = fp_AlignUp(_pEnd2);
			if (_pStart2 > _pEnd)
			{
				// Two blocks
				if (_pEnd > _pStart)
					_pChunk->f_Commit(_pStart, _pEnd - _pStart);

				if (_pEnd2 > _pStart2)
					_pChunk->f_Commit(_pStart2, _pEnd2 - _pStart2);
			}
			else if (_pEnd2 > _pEnd)
			{
				// Block overlap
				if (_pEnd2 > _pStart)
					_pChunk->f_Commit(_pStart, _pEnd2 - _pStart);
			}
			else
			{
				// No second block
				if (_pEnd > _pStart)
					_pChunk->f_Commit(_pStart, _pEnd - _pStart);
			}
		}

		template <class t_CHeapParams>
		inline_small void TCHeap<t_CHeapParams>::fsp_CommitDualRangeAlreadyCommitted(CChunk *_pChunk, uint8 *_pStart, uint8 *_pEnd, uint8 *_pStart2, uint8 *_pEnd2, uint8 *_pAlreadyCommited)
		{
			_pStart = fp_AlignDown(_pStart);
			_pEnd = fp_AlignUp(_pEnd);
			_pStart2 = fp_AlignDown(_pStart2);
			_pEnd2 = fp_AlignUp(_pEnd2);
			uint8 *pAlreadyCommitedStart = fp_AlignDown(_pAlreadyCommited);
			uint8 *pAlreadyCommitedEnd = fp_AlignUp(_pAlreadyCommited);
			if (_pEnd > pAlreadyCommitedStart && _pEnd <= pAlreadyCommitedEnd)
			{
				_pEnd = pAlreadyCommitedStart;
			}
			if (_pStart >= pAlreadyCommitedStart && _pStart < pAlreadyCommitedEnd)
			{
				_pStart = pAlreadyCommitedEnd;
			}
			if (_pEnd2 > pAlreadyCommitedStart && _pEnd2 <= pAlreadyCommitedEnd)
			{
				_pEnd2 = pAlreadyCommitedStart;
			}
			if (_pStart2 >= pAlreadyCommitedStart && _pStart2 < pAlreadyCommitedEnd)
			{
				_pStart2 = pAlreadyCommitedEnd;
			}

			if (_pStart2 > _pEnd)
			{
				// Two blocks
				if (_pEnd > _pStart)
					_pChunk->f_Commit(_pStart, _pEnd - _pStart);

				if (_pEnd2 > _pStart2)
					_pChunk->f_Commit(_pStart2, _pEnd2 - _pStart2);
			}
			else if (_pEnd2 > _pEnd)
			{
				// Block overlap
				if (_pEnd2 > _pStart)
					_pChunk->f_Commit(_pStart, _pEnd2 - _pStart);
			}
			else
			{
				// No second block
				if (_pEnd > _pStart)
					_pChunk->f_Commit(_pStart, _pEnd - _pStart);
			}
		}

		template <class t_CHeapParams>
		inline_small void TCHeap<t_CHeapParams>::fsp_DecommitDualRangeAligned(CChunk *_pChunk, uint8 *_pStart, uint8 *_pEnd, uint8 *_pStart2, uint8 *_pEnd2)
		{
			if (_pStart2 > _pEnd)
			{
				// Two blocks
				if (_pEnd > _pStart)
					_pChunk->f_Decommit(_pStart, _pEnd - _pStart);

				if (_pEnd2 > _pStart2)
					_pChunk->f_Decommit(_pStart2, _pEnd2 - _pStart2);
			}
			else if (_pEnd2 > _pEnd)
			{
				// Block overlap
				if (_pEnd2 > _pStart)
					_pChunk->f_Decommit(_pStart, _pEnd2 - _pStart);
			}
			else
			{
				// No second block
				if (_pEnd > _pStart)
					_pChunk->f_Decommit(_pStart, _pEnd - _pStart);
			}
		}

		template <class t_CHeapParams>
		inline_medium void TCHeap<t_CHeapParams>::fsp_CommitBlockNextExtended(CChunk *_pChunk, CBlock* _pBlock, CBlock* _pNextBlockOld, CBlock* _pNextBlock)
		{
			DMibFastCheck(ENeedExtendedBlocks); // "Must not call here if we don't need extended free blocks"
			uint8 *pCommitMem = fp_AlignUp(((uint8*)_pBlock) + EBlockFreeExtendedSizeStart);
			uint8 *pCommitEnd = fp_AlignDown((uint8 *)_pNextBlockOld - EBlockFreeExtendedSizeEnd);
			uint8 *pBlockEnd = fp_AlignUp((uint8 *)_pNextBlock + EBlockFreeExtendedSizeStart);
			if (pCommitEnd < pBlockEnd)
				pBlockEnd = pCommitEnd;

			if (pBlockEnd > pCommitMem)
				_pChunk->f_Commit(pCommitMem, pBlockEnd - pCommitMem);
		}

		template <class t_CHeapParams>
		inline_medium void TCHeap<t_CHeapParams>::fsp_CommitBlockNextFree(CChunk *_pChunk, CBlock* _pBlock, CBlock* _pNextBlockOld, CBlock* _pNextBlock)
		{
			uint8 *pCommitMem = fp_AlignUp(((uint8*)_pBlock) + sizeof(CBlock_Free));
			uint8 *pCommitEnd = fp_AlignDown((uint8 *)_pNextBlockOld);
			uint8 *pBlockEnd = fp_AlignUp((uint8 *)_pNextBlock + sizeof(CBlock_Free));
			if (pCommitEnd < pBlockEnd)
				pBlockEnd = pCommitEnd;

			if (pBlockEnd > pCommitMem)
				_pChunk->f_Commit(pCommitMem, pBlockEnd - pCommitMem);
		}

		template <class t_CHeapParams>
		inline_medium void TCHeap<t_CHeapParams>::fsp_CommitBlockExtended(CChunk *_pChunk, CBlock* _pBlock, CBlock *_pNextBlock)
		{
			DMibFastCheck(ENeedExtendedBlocks); // "Must not call here if we don't need extended free blocks"
			uint8 *pCommitMem = fp_AlignUp(((uint8*)_pBlock) + EBlockFreeExtendedSizeStart);
			uint8 *pCommitEnd = fp_AlignDown((uint8 *)_pNextBlock - EBlockFreeExtendedSizeEnd);
			if (pCommitEnd > pCommitMem)
				_pChunk->f_Commit(pCommitMem, pCommitEnd - pCommitMem);
		}

		template <class t_CHeapParams>
		inline_medium void TCHeap<t_CHeapParams>::fsp_CommitBlockNormal(CChunk *_pChunk, CBlock* _pBlock, CBlock *_pNextBlock)
		{
			uint8 *pCommitMem = fp_AlignUp(((uint8*)_pBlock) + sizeof(CBlock_Free));
			uint8 *pCommitEnd = fp_AlignDown((uint8 *)_pNextBlock);
			if (pCommitEnd > pCommitMem)
				_pChunk->f_Commit(pCommitMem, pCommitEnd - pCommitMem);
		}

	}
}
