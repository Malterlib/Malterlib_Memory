// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

namespace NMib
{
	namespace NMem
	{


		/***************************************************************************************************\
		|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
		| Free																								|
		|___________________________________________________________________________________________________|
		\***************************************************************************************************/

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::fp_FreeBlockCache()
		{
			if (m_bHaveFastChunks)
			{
				m_bHaveFastChunks = false;

				for (mint i = 0; i < ECachedBlocks; ++i)
				{
					while (CBlock_Cached *pBlock = m_aCachedBlocks[i].f_Pop())
					{
						if (f_CanCommit())
							fp_FreeInternal((CChunk *)m_pChunksTree->f_FindLargestLessThanEqual(pBlock), pBlock);
						else
							fp_FreeInternal(nullptr, pBlock);
					}
				}
			}
		}

		template <class t_CHeapParams>
		inline_small void TCHeap<t_CHeapParams>::fp_Free(CChunk *_pChunk, void *_pMem)
		{

			if (!_pMem)
				return;

			CBlock_Cached *pBlock = (CBlock_Cached *)(((uint8 *)_pMem) - EBlockPreSize);
			mint FinalSize = pBlock->f_GetSizeNormal();

			if (EMaxCachedBlockSize > 0)
			{
				if (FinalSize <= EMaxCachedBlockSize)
				{
					m_bHaveFastChunks = true;
					DMibFastCheck(((FinalSize - (ESmallestBlock)) >> EAlignBits) < sizeof(m_aCachedBlocks) / sizeof(m_aCachedBlocks[0])); // "Must fit in cache list"
						
					if (CFillDebug::EDoFills)
					{
						mint Size = FinalSize;
						// Check that noone has overwritten us
						CFillDebug::fs_CheckPreGuard((uint8 *)_pMem - EBlockPreGuardOffset, EBlockPreGuardSize);
						CFillDebug::fs_CheckPostGuard((uint8 *)pBlock + (Size - EBlockPostGuardSize), EBlockPostGuardSize);
						// This block is free
						CFillDebug::fs_FillFree((uint8 *)pBlock + sizeof(CBlock_Cached), Size - sizeof(CBlock_Cached));
					}

					m_aCachedBlocks[(FinalSize - (ESmallestBlock)) >> EAlignBits].f_Push(pBlock);
					return;
				}
			}

			fp_FreeInternal(_pChunk, pBlock);
		}

		template <class t_CHeapParams>
		inline_small void TCHeap<t_CHeapParams>::f_Free(void *_pMem)
		{

			if (!_pMem)
				return;

			CBlock_Cached *pBlock = (CBlock_Cached *)(((uint8 *)_pMem) - EBlockPreSize);
			mint FinalSize = pBlock->f_GetSizeNormal();

			if (EMaxCachedBlockSize > 0)
			{
				if (FinalSize <= EMaxCachedBlockSize)
				{
					m_bHaveFastChunks = true;
					DMibFastCheck(((FinalSize - (ESmallestBlock)) >> EAlignBits) < sizeof(m_aCachedBlocks) / sizeof(m_aCachedBlocks[0])); // "Must fit in cache list"
					//pBlock->m_CacheLink.f_Construct();

					if (CFillDebug::EDoFills)
					{
						mint Size = FinalSize;
						// Check that noone has overwritten us
						CFillDebug::fs_CheckPreGuard((uint8 *)_pMem - EBlockPreGuardOffset, EBlockPreGuardSize);
						CFillDebug::fs_CheckPostGuard((uint8 *)pBlock + (Size - EBlockPostGuardSize), EBlockPostGuardSize);
						// This block is free
						CFillDebug::fs_FillFree((uint8 *)pBlock + sizeof(CBlock_Cached), Size - sizeof(CBlock_Cached));
					}

					m_aCachedBlocks[(FinalSize - (ESmallestBlock)) >> EAlignBits].f_Push(pBlock);
					return;
				}
			}

			if (f_CanCommit())
				fp_FreeInternal((CChunk *)m_pChunksTree->f_FindLargestLessThanEqual(pBlock), pBlock);
			else
				fp_FreeInternal(nullptr, pBlock);
		}

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::fp_FreeInternal(CChunk *_pChunk, CBlock *_pBlock)
		{
			DMibFastCheck(_pBlock->f_Type() == EBlockType_Normal); // "Already freed?"

			fp_CheckAllocatedBlock(_pBlock);

			static const bint bTreeOpt1 = !t_CHeapParams::EbOptimizeForSize && t_CHeapParams::CSizeHolder::EStoresSize && t_CHeapParams::ETreeOpt3;
			static const bint bTreeOpt0 = (!t_CHeapParams::EbOptimizeForSize && t_CHeapParams::CSizeHolder::EStoresSize && t_CHeapParams::ETreeOpt2) || bTreeOpt1;

			CBlock *pBlock = _pBlock;
			mint FinalSize = pBlock->f_GetSizeNormal();
			if (EMaxCachedBlockSize > 0 && 0)
			{
				if (FinalSize > (mint)t_CHeapParams::ECacheFreeThreshold)
				{
					fp_FreeBlockCache();					
				}
			}				mint OriginalSize = FinalSize;
			CBlock *pPrev = pBlock->f_GetPrev();
			CBlock *pNext = pBlock->f_GetNextNormal();
			CBlock *pLastBlock = pBlock;

			CBlock_Free *pFinalBlock = (CBlock_Free *)pBlock;

			uint8 *pDecommit1Start = nullptr;
			uint8 *pDecommit1End = nullptr;
				
			if (f_CanCommit())
			{
				pDecommit1Start = fp_AlignUp(((uint8*)pBlock) + sizeof(CBlock_Free));
				pDecommit1End = fp_AlignDown((uint8 *)pBlock->f_GetNextNormal());
			}

			CFreeBlockBucket *pSavedBlockBucket = nullptr;

			if (pPrev->f_IsFree())
			{
				FinalSize += pPrev->f_GetSize();
				pFinalBlock = (CBlock_Free *)pPrev;
				if (bTreeOpt0)
					pSavedBlockBucket = fp_UntieFreeBlockRet<0>((CBlock_Free *)pPrev);
				else
					fp_UntieFreeBlock<1>((CBlock_Free *)pPrev);

				if (f_CanCommit())
				{
					uint8 *pCommit = fp_AlignDown((uint8 *)pBlock);

					if (pCommit < ((uint8*)pPrev + sizeof(CBlock_Free)))
						pCommit = fp_AlignUp((uint8*)pPrev + sizeof(CBlock_Free));

					if (pCommit < pDecommit1Start)
						pDecommit1Start = pCommit;
				}

			}

			if (pNext->f_IsFree())
			{
				FinalSize += pNext->f_GetSize();
				if (bTreeOpt0)
				{
					if (pSavedBlockBucket)
					{
						CFreeBlockBucket *pSavedBlockBucket2 = fp_UntieFreeBlockRet<0>((CBlock_Free *)pNext);
						if (pSavedBlockBucket2)
						{
							if (pSavedBlockBucket2->f_GetSize() > pSavedBlockBucket->f_GetSize())
							{
								// Delete old saved bucket
								pSavedBlockBucket->f_Destruct(this);
								fp_DeleteFreeBlockBucket(pSavedBlockBucket);
								pSavedBlockBucket = pSavedBlockBucket2;
							}
							else
							{
								// Delete old saved bucket
								pSavedBlockBucket2->f_Destruct(this);
								fp_DeleteFreeBlockBucket(pSavedBlockBucket2);
							}
						}
					}
					else
						pSavedBlockBucket = fp_UntieFreeBlockRet<0>((CBlock_Free *)pNext);
				}
				else
					fp_UntieFreeBlock<1>((CBlock_Free *)pNext);

				pLastBlock = pNext;

				if (f_CanCommit())
				{
					uint8 *pCommit;

					if (ENeedExtendedBlocks && pNext->f_Type() == EBlockType_FreeExtended)
						pCommit = fp_AlignUp(((uint8 *)pNext) + EBlockFreeExtendedSizeStart);
					else
						pCommit = fp_AlignUp(((uint8 *)pNext) + sizeof(CBlock_Free));

					if (pCommit > (uint8 *)pFinalBlock + FinalSize)
						pCommit = fp_AlignDown((uint8 *)pFinalBlock + FinalSize);

					if (pCommit > pDecommit1End)
						pDecommit1End = pCommit;
				}
			}


			CBlock *pNextAllocated = (CBlock *)((uint8 *)pFinalBlock + FinalSize);

			DMibFastCheck((FinalSize & (~((mint)EAlignAnd))) == 0); // "Left over size has to be aligned"

			if (ENeedExtendedBlocks && ((t_CHeapParams::EbOptimizeForSize && FinalSize >= sizeof(CBlock_FreeExtendedBucket)) || (!t_CHeapParams::EbOptimizeForSize && FinalSize >= (mint)ELargestBlock)))
			{
				// We can let the block bucket reside in the free block itself
				TCDynamicPtr<CPtrHolder, CBlock> *pExtendedPlace = (((TCDynamicPtr<CPtrHolder, CBlock> *)(((uint8 *)pFinalBlock) + FinalSize)) - 1);

				if (f_CanCommit())
				{
					if (pDecommit1Start < ((uint8*)pFinalBlock) + EBlockFreeExtendedSizeStart)
						pDecommit1Start = fp_AlignUp(((uint8*)pFinalBlock) + EBlockFreeExtendedSizeStart);

					if (pDecommit1End > (uint8 *)pExtendedPlace)
						pDecommit1End = fp_AlignDown((uint8 *)pExtendedPlace);
					if (pFinalBlock != pBlock && pFinalBlock->f_Type() != EBlockType_FreeExtended)
					{
						uint8 *pCommit1 = fp_AlignUp((uint8 *)pFinalBlock + sizeof(CBlock_Free));
						uint8 *pCommit2 = fp_AlignUp((uint8 *)pFinalBlock + EBlockFreeExtendedSizeStart);
						uint8 *pCommit3 = fp_AlignDown((uint8 *)pBlock);
						if (pCommit3 < pCommit2)
							pCommit2 = pCommit3;
						if (pCommit1 < pCommit2)
							_pChunk->f_Commit(pCommit1, pCommit2 - pCommit1);
					}
					if (pLastBlock != pBlock && pLastBlock->f_Type() != EBlockType_FreeExtended)
					{
						uint8 *pCommit1 = fp_AlignDown((uint8 *)pExtendedPlace);
						uint8 *pCommit2 = fp_AlignDown((uint8 *)pFinalBlock + FinalSize);
						uint8 *pCommit3 = fp_AlignUp((uint8 *)pLastBlock + sizeof(CBlock_Free));
						if (pCommit3 > pCommit1)
							pCommit1 = pCommit3;
						if (pCommit1 < pCommit2)
							_pChunk->f_Commit(pCommit1, pCommit2 - pCommit1);
					}
				}

				((CBlock_FreeExtendedBase *)pFinalBlock)->m_pNext = pNextAllocated;
				pNextAllocated->f_SetPrev(0);
				pFinalBlock->f_SetType(EBlockType_FreeExtended);

				// Set ThisPtr
				*pExtendedPlace = pFinalBlock;

				if (t_CHeapParams::EbOptimizeForSize)
				{
					((CBlock_FreeExtendedBucket *)pFinalBlock)->f_Construct();

					mint FindSize = FinalSize;
					if (FindSize > (mint)ELargestBlock)
						FindSize = (mint)ELargestBlock;
					// Check to see if it already exists a bucket
					CFreeBlockBucket *pFreeBlockBucket = fp_GetBlockBucketExact(FindSize);
					if (!pFreeBlockBucket)
					{
						pFreeBlockBucket = &((CBlock_FreeExtendedBucket *)pFinalBlock)->m_FreeBlockBucket;
						pFreeBlockBucket->f_Construct(pFinalBlock, FindSize, this);
					}
					else
					{
						pFreeBlockBucket->m_FreeBlocks.f_Push(pFinalBlock);
					}
				}
				else
				{
					((CBlock_FreeExtended *)pFinalBlock)->f_Construct();

					mint FindSize = FinalSize;
					if (FindSize > (mint)ELargestBlock)
						FindSize = (mint)ELargestBlock;
					// Check to see if it already exists a bucket
					if (bTreeOpt1 && pSavedBlockBucket)
					{
						CFreeBlockBucket *pPrev = nullptr;
						CFreeBlockBucket *pFreeBlockBucket = fp_GetBlockBucket(FindSize, pPrev);
						if (pFreeBlockBucket && pFreeBlockBucket->f_GetSize() == FindSize)
						{
							if (pSavedBlockBucket != pFreeBlockBucket)
							{
								pSavedBlockBucket->f_Destruct(this);
								fp_DeleteFreeBlockBucket(pSavedBlockBucket);
							}
							pFreeBlockBucket->m_FreeBlocks.f_Push(pFinalBlock);
						}
						else
						{
							if (pPrev == pSavedBlockBucket)
							{
								pSavedBlockBucket->f_SetSize(FindSize);
								pSavedBlockBucket->m_FreeBlocks.f_Push(pFinalBlock);
							}
							else
							{
								pSavedBlockBucket->f_Destruct(this);
								pSavedBlockBucket->f_Construct(pFinalBlock, FindSize, this);
							}
						}
					}
					else
					{

						CFreeBlockBucket *pFreeBlockBucket = fp_GetBlockBucketExact(FindSize);
						if (!pFreeBlockBucket)
						{
							if ((bTreeOpt0 && !bTreeOpt1) && pSavedBlockBucket)
							{
								// Delete old saved bucket
								pSavedBlockBucket->f_Destruct(this);
								pFreeBlockBucket = pSavedBlockBucket;
							}
							else
								pFreeBlockBucket = fp_NewFreeBlockBucket(FindSize);
							pFreeBlockBucket->f_Construct(pFinalBlock, FindSize, this);
						}
						else
						{
							if ((bTreeOpt0 && !bTreeOpt1) && pSavedBlockBucket && pFreeBlockBucket != pSavedBlockBucket)
							{
								// Delete old saved bucket
								pSavedBlockBucket->f_Destruct(this);
								fp_DeleteFreeBlockBucket(pSavedBlockBucket);
							}
							pFreeBlockBucket->m_FreeBlocks.f_Push(pFinalBlock);
						}
					}
				}
			}
			else
			{
				DMibFastCheck(FinalSize >= ESmallestBlock); // "Block has to fit"
				// We can let the block bucket reside in the free block itself					 

				pFinalBlock->f_SetType(EBlockType_Free);
				pFinalBlock->f_SetNext(FinalSize >> EAlignBits);
				pNextAllocated->f_SetPrev(FinalSize >> EAlignBits);

				pFinalBlock->f_Construct();

				if (bTreeOpt1 && pSavedBlockBucket)
				{
					CFreeBlockBucket *pPrev = nullptr;
					CFreeBlockBucket *pFreeBlockBucket = fp_GetBlockBucket(FinalSize, pPrev);
					if (pFreeBlockBucket && pFreeBlockBucket->f_GetSize() == FinalSize)
					{
						pSavedBlockBucket->f_Destruct(this);
						fp_DeleteFreeBlockBucket(pSavedBlockBucket);
						pFreeBlockBucket->m_FreeBlocks.f_Push(pFinalBlock);
					}
					else
					{
						if (pPrev == pSavedBlockBucket)
						{
							pSavedBlockBucket->f_SetSize(FinalSize);
							pSavedBlockBucket->m_FreeBlocks.f_Push(pFinalBlock);
						}
						else
						{
							pSavedBlockBucket->f_Destruct(this);
							pSavedBlockBucket->f_Construct(pFinalBlock, FinalSize, this);
						}
					}
				}
				else
				{
					// Check to see if it already exists a bucket
					CFreeBlockBucket *pFreeBlockBucket = fp_GetBlockBucketExact(FinalSize);
					if (!pFreeBlockBucket)
					{
						if ((bTreeOpt0 && !bTreeOpt1) && pSavedBlockBucket)
						{
							// Delete old saved bucket
							pSavedBlockBucket->f_Destruct(this);
							pFreeBlockBucket = pSavedBlockBucket;
						}
						else
							pFreeBlockBucket = fp_NewFreeBlockBucket(FinalSize);
						pFreeBlockBucket->f_Construct(pFinalBlock, FinalSize, this);
					}
					else
					{
						if ((bTreeOpt0 && !bTreeOpt1) && pSavedBlockBucket)
						{
							// Delete old saved bucket
							pSavedBlockBucket->f_Destruct(this);
							fp_DeleteFreeBlockBucket(pSavedBlockBucket);
						}
						pFreeBlockBucket->m_FreeBlocks.f_Push(pFinalBlock);
					}
				}
			}

			if (FinalSize == OriginalSize)
				fp_FillFreeBlock(pFinalBlock);
			else
			{
				if (!f_CanCommit() && CFillDebug::EDoFills)
				{					
					mint Size = OriginalSize;

					if (pFinalBlock == pPrev)
					{
						if (pLastBlock == pBlock)
						{
							if (pFinalBlock->f_Type() == EBlockType_FreeExtended)
							{
								uint8 *pStart = fg_Max((uint8 *)pBlock - EBlockFreeExtendedSizeEnd, (uint8 *)pFinalBlock + EBlockFreeExtendedSizeStart);
								uint8 *pEnd = (uint8 *)pBlock + OriginalSize - EBlockFreeExtendedSizeEnd;
								CFillDebug::fs_FillFree(pStart, pEnd - pStart);
							}
							else if (pFinalBlock->f_Type() == EBlockType_Free)
							{
								uint8 *pStart = (uint8 *)pBlock;
								uint8 *pEnd = (uint8 *)pBlock + OriginalSize;
								CFillDebug::fs_FillFree(pStart, pEnd - pStart);
							}
							else
								DMibPDebugBreak;
						}
						else
						{
							if (pFinalBlock->f_Type() == EBlockType_FreeExtended)
							{
								uint8 *pStart = fg_Max((uint8 *)pBlock - EBlockFreeExtendedSizeEnd, (uint8 *)pFinalBlock + EBlockFreeExtendedSizeStart);
								uint8 *pEnd = fg_Min((uint8 *)pBlock + OriginalSize + EBlockFreeExtendedSizeStart, (uint8 *)pFinalBlock + FinalSize - EBlockFreeExtendedSizeEnd);
								CFillDebug::fs_FillFree(pStart, pEnd - pStart);
							}
							else if (pFinalBlock->f_Type() == EBlockType_Free)
							{
								uint8 *pStart = (uint8 *)pBlock;
								uint8 *pEnd = fg_Min((uint8 *)pBlock + OriginalSize + sizeof(CBlock_Free), (uint8 *)pFinalBlock + FinalSize);
								CFillDebug::fs_FillFree(pStart, pEnd - pStart);
							}
							else
								DMibPDebugBreak;
						}
					}
					else
					{
						if (pFinalBlock->f_Type() == EBlockType_FreeExtended)
							CFillDebug::fs_FillFree((uint8 *)pFinalBlock + EBlockFreeExtendedSizeStart, fg_Min(Size - EBlockFreeExtendedSizeStart + EBlockFreeExtendedSize, FinalSize - EBlockFreeExtendedSize));
						else if (pFinalBlock->f_Type() == EBlockType_Free)
							CFillDebug::fs_FillFree((uint8 *)pFinalBlock + sizeof(CBlock_Free), fg_Min(Size - sizeof(CBlock_Free) + sizeof(CBlock_Free), FinalSize - sizeof(CBlock_Free)));
						else
							DMibPDebugBreak;
					}
				}
			}


			if (f_CanCommit())
			{
				if (pDecommit1Start < pDecommit1End)
				{
					_pChunk->f_Decommit(pDecommit1Start, pDecommit1End - pDecommit1Start);
				}
			}

			if (t_CHeapParams::ENumChunkFreeThreshold)
			{
				if (pNextAllocated->f_Type() == EBlockType_Edge) // We added space to the last block 
				{
					if (pFinalBlock->f_GetPrev()->f_Type() == EBlockType_Edge) // Whole chunk is free
					{
						const void *pChunkMem = pFinalBlock;
						CChunk *pChunk = (CChunk *)m_pChunksTree->f_FindLargestLessThanEqual(pChunkMem);
						DMibFastCheck((mint)pChunk->f_GetBasePointer() <= (mint)pChunkMem && (mint)pChunk->f_GetEndPointer() > (mint)pChunkMem && pChunk->m_pHeap == this); // "Got a chunk that isn't ours"
						if (m_pFreeChunk && pChunk != m_pFreeChunk)
						{
							if (m_pFreeChunk->f_GetFirstBlock()->f_GetNext()->f_GetNext()->f_Type() == EBlockType_Edge)
							{
								// The chunk is still free destroy our chunk
								pChunk->f_Destroy(*m_pChunksTree);
								return;
							}
						}
						fp_UntieFreeBlock<true>(pFinalBlock);
						// Make the chunk one subchunk large
						pChunk->f_Trim();
						// Replace the free chunk with us
						m_pFreeChunk = pChunk;
					}
					else // Only do this if we have close to 2 subchunks free, this is to stop from doing this in a flickering manner
					{
						mint FreeSize = pFinalBlock->f_GetSize();
						if (FreeSize > (NMib::fg_AlignUp((mint)t_CHeapParams::EGrowSize, CAllocator::f_GranularityAlloc(ELargePages))) * t_CHeapParams::ENumChunkFreeThreshold)
						{
							const void *pChunkMem = pFinalBlock;
							CChunk *pChunk = (CChunk *)m_pChunksTree->f_FindLargestLessThanEqual(pChunkMem);
							mint MinChunkSize = pChunk->m_SubChunks.f_FindLargest()->m_Size + (TCHeap::fsp_CommitSize_FreeExtended() + EBlockNonAllocSize);

							if (FreeSize >= MinChunkSize)
							{
								// Get the chunk
								fp_UntieFreeBlock<true>(pFinalBlock);
								pChunk->f_TrimEnd();
							}
						}
					}
				}
				else if (pFinalBlock->f_GetPrev()->f_Type() == EBlockType_Edge)	// We added space to the first block
				{
					mint FreeSize = pFinalBlock->f_GetSize();
					// Only do this if we have close to 2 subchunks free, this is to stop from doing this in a flickering manner
					if (FreeSize > (NMib::fg_AlignUp((mint)t_CHeapParams::EGrowSize, CAllocator::f_GranularityAlloc(ELargePages))) * t_CHeapParams::ENumChunkFreeThreshold)
					{
						const void *pChunkMem = pFinalBlock;
						CChunk *pChunk = (CChunk *)m_pChunksTree->f_FindLargestLessThanEqual(pChunkMem);
						mint ExtraNeededSize = (EBlockFirstAdd - EBlockNonAllocSize) + EBlockMinNext + TCHeap<t_CHeapParams>::fsp_CommitSize_FreeExtended();
						mint MinChunkSize = pChunk->m_SubChunks.f_FindSmallest()->m_Size + ExtraNeededSize;
						if (FreeSize >= MinChunkSize)
						{
							// Get the chunk
							fp_UntieFreeBlock<true>(pFinalBlock);
							pChunk->f_TrimStart();
						}
					}
				}
			}
		}

	}
}
