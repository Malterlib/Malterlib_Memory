// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

namespace NMib
{
	namespace NMem
	{


		/***************************************************************************************************\
		|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
		| Heap Chunk																						|
		|___________________________________________________________________________________________________|
		\***************************************************************************************************/

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::CChunk::fp_Destroy(typename TCHeapChunk<CAllocator>::CTree &_Tree)
		{
			if (m_pHeap->m_pFreeChunk == this)
			{
				m_pHeap->m_pFreeChunk = nullptr;
			}

			CBlock *pBlock = f_GetFirstBlock();
			DMibFastCheck(m_SubChunks.f_FindSmallest()->m_pStart == f_GetBasePointer()); // "Heap inconsistency"

			DMibFastCheck(pBlock->f_Type() == EBlockType_Edge); // "Heap must start with an edgeblock"
			pBlock = pBlock->f_GetNext();

			// Untie all free blocks
			while (pBlock->f_Type() != EBlockType_Edge)
			{
				if (pBlock->f_Type() != EBlockType_Normal)
				{
					m_pHeap->template fp_UntieFreeBlock<true>((CBlock_Free *)pBlock);
				}
				else
				{
#ifdef DMibDebug
//					NSys::fg_DebugOutput("MemoryLeak" DMibNewLine);
#endif
					// MemoryLeak
				}

				pBlock = pBlock->f_GetNext();
			}			

			TCHeapChunk<CAllocator>::f_Destroy(_Tree);

			{
				CSubChunk *pSubChunk = m_SubChunks.f_GetRoot();
				while (pSubChunk)
				{
					m_SubChunks.f_Remove(pSubChunk);
					CAllocator::f_Free(pSubChunk->m_pStart, pSubChunk->m_Size);
					m_pHeap->m_PoolSubChunks.f_Delete(pSubChunk);
					pSubChunk = m_SubChunks.f_GetRoot();
				}
			}

			// Return space
			m_pHeap->m_PoolChunks.f_Delete(this);
		}

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::CChunk::f_Trim()
		{
			// The whole chunk must be free
			// Trim at start of chunk first as to not glide

			m_pHeap->m_pChunksTree->f_Remove(this);
			DMibFastCheck(f_GetFirstBlock()->f_GetNext()->f_GetNext()->f_Type() == EBlockType_Edge); // "The whole chunk must be free"

			// Remove sub chunks until we only have one left
			bint bFreedChunk = false;
			CSubChunk *pSubChunk;

			if (mc_AllocationFlags & EAllocationFlag_LocationUp)
				pSubChunk = m_SubChunks.f_FindLargest();
			else 
				pSubChunk = m_SubChunks.f_FindSmallest();

			while (!m_SubChunks.f_HasOneMember())
			{
				bFreedChunk = true;
				TCHeapChunk<CAllocator>::m_Size -= pSubChunk->m_Size;
				m_SubChunks.f_Remove(pSubChunk);
				CAllocator::f_Free(pSubChunk->m_pStart, pSubChunk->m_Size);
				m_pHeap->m_PoolSubChunks.f_Delete(pSubChunk);

				if (mc_AllocationFlags & EAllocationFlag_LocationUp)
					pSubChunk = m_SubChunks.f_FindLargest();
				else 
					pSubChunk = m_SubChunks.f_FindSmallest();
			}

			TCHeapChunk<CAllocator>::m_pBase = pSubChunk->m_pStart;
			m_pHeap->m_pChunksTree->f_Insert(this);

			CBlock *pBlock = f_GetFirstBlock();
			CBlock *pLastBlock = f_GetLastBlock();
			CBlock_FreeExtended *pFreeBlock = (CBlock_FreeExtended *)((uint8 *)pBlock + EBlockMinNext);

			if (f_CanCommit() && bFreedChunk)
			{
				// ToDo: Check that we really need to do this
				if (!(mc_AllocationFlags & EAllocationFlag_LocationUp))
				{
					TCHeap::fsp_CommitDualRange(this, (uint8 *)pBlock, (uint8 *)pBlock + EBlockNonAllocSize, (uint8 *)pBlock + EBlockMinNext, 
						(uint8 *)pBlock + EBlockMinNext + TCHeap::fsp_CommitSize_FreeExtendedStart());
				}
				else
				{
					TCHeap::fsp_CommitRange(this, (uint8 *)pLastBlock - TCHeap::fsp_CommitSize_FreeExtendedEnd(), (uint8 *)f_GetEndPointer());
				}
			}

			// Setup first block

			pBlock->f_SetAll(EBlockMinNext >> EAlignBits, 0, EBlockType_Edge);

			if (ENeedExtendedBlocks)
				pLastBlock->f_SetAll(0,0,EBlockType_Edge);
			else
				pLastBlock->f_SetAll(0,((uint8 *)pLastBlock - (uint8 *)pFreeBlock) >> EAlignBits,EBlockType_Edge);

			m_pHeap->fp_InitFreeBlock(pFreeBlock, pLastBlock, pBlock);
		}

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::CChunk::f_TrimEnd()
		{
//					m_pHeap->m_pChunksTree->f_Remove(this);
			CBlock *pFreeBlock = f_GetLastBlock()->f_GetPrev();
			CSubChunk *pSubChunk = m_SubChunks.f_FindLargest();
			while ((mint)(void *)pSubChunk->m_pStart > (mint)pFreeBlock + (TCHeap::fsp_CommitSize_FreeExtended() + EBlockNonAllocSize))
			{				
				// Remove sub chunks until we only have one left
				TCHeapChunk<CAllocator>::m_Size -= pSubChunk->m_Size;
				m_SubChunks.f_Remove(pSubChunk);
				CAllocator::f_Free(pSubChunk->m_pStart, pSubChunk->m_Size);
				m_pHeap->m_PoolSubChunks.f_Delete(pSubChunk);
				pSubChunk = m_SubChunks.f_FindLargest();
			}
//					m_pHeap->m_pChunksTree->f_Insert(this);

			// Setup blocks
			CBlock *pNextBlock = f_GetLastBlock();
			if (f_CanCommit())
			{
				// ToDo: Check that we really need to do this
				TCHeap::fsp_CommitRangeAlreadyCommitted(this, (uint8 *)pNextBlock - TCHeap::fsp_CommitSize_FreeExtendedEnd(), (uint8 *)f_GetEndPointer(), (uint8 *)pFreeBlock);
			}

			CBlock *pPrevBlock = pFreeBlock->f_GetPrev();
			if (ENeedExtendedBlocks)
				pNextBlock->f_SetAll(0,0,EBlockType_Edge);
			else
				pNextBlock->f_SetAll(0,((uint8 *)pNextBlock - (uint8 *)pFreeBlock) >> EAlignBits,EBlockType_Edge);
			m_pHeap->fp_InitFreeBlock((CBlock_FreeExtendedBase *)pFreeBlock, pNextBlock, pPrevBlock);
		}

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::CChunk::f_TrimStart()
		{
			m_pHeap->m_pChunksTree->f_Remove(this);
			CBlock *pNextBlock = f_GetFirstBlock()->f_GetNext()->f_GetNext();
#					if DMibEnableSafeCheck > 0
				mint SizeStart = TCHeapChunk<CAllocator>::m_Size;
#					endif

			CSubChunk *pSubChunk = m_SubChunks.f_FindSmallest();
			mint ExtraNeededSize = (EBlockFirstAdd - EBlockNonAllocSize) + EBlockMinNext + TCHeap<t_CHeapParams>::fsp_CommitSize_FreeExtended();
			while ((mint)(void *)pSubChunk->m_pStart + pSubChunk->m_Size < ((mint)pNextBlock - ExtraNeededSize))
			{				
				// Remove sub chunks until we only have one left
				TCHeapChunk<CAllocator>::m_Size -= pSubChunk->m_Size;
				m_SubChunks.f_Remove(pSubChunk);
				CAllocator::f_Free(pSubChunk->m_pStart, pSubChunk->m_Size);
				m_pHeap->m_PoolSubChunks.f_Delete(pSubChunk);
				pSubChunk = m_SubChunks.f_FindSmallest();
			}
			DMibFastCheck(TCHeapChunk<CAllocator>::m_Size != SizeStart); // ""
			TCHeapChunk<CAllocator>::m_pBase = pSubChunk->m_pStart;
			m_pHeap->m_pChunksTree->f_Insert(this);

			CBlock *pPrevBlock = f_GetFirstBlock();
			CBlock *pFreeBlock = (CBlock *)((uint8 *)pPrevBlock + EBlockMinNext);
			if (f_CanCommit())
			{
				// ToDo: Check that we really need to do this
				TCHeap::fsp_CommitDualRangeAlreadyCommitted(this, (uint8 *)pPrevBlock, (uint8 *)pPrevBlock + EBlockNonAllocSize, (uint8 *)pPrevBlock + EBlockMinNext, 
					(uint8 *)pPrevBlock + EBlockMinNext + TCHeap::fsp_CommitSize_FreeExtendedStart(), (uint8 *)pNextBlock);
			}

			// Setup blocks
			pPrevBlock->f_SetAll(EBlockMinNext >> EAlignBits, 0, EBlockType_Edge);
			m_pHeap->fp_InitFreeBlock((CBlock_FreeExtendedBase *)pFreeBlock, pNextBlock, pPrevBlock);
		}

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::CChunk::f_Commit(void *_pMem, mint _Size)
		{
			if (_Size <= CAllocator::f_GranularityCommit(ELargePages)) // Only one page, we cannot span multiple pages
			{
				CAllocator::f_Commit(_pMem, _Size);
			}
			else
			{
				CSubChunkIter Iter;
				Iter.f_InitForSearch(m_SubChunks);
				typename CSubChunk::CDynamicPtr Ptr;
				Ptr = _pMem;
				if (Iter.f_FindLargestLessThanEqualForward(Ptr))
				{
					uint8 *pMem = (uint8 *)_pMem;
					while (Iter && _Size)
					{
						CSubChunk *pChunk = Iter;
						DMibFastCheck(pMem >= (uint8 *)(void *)pChunk->m_pStart && pMem + CAllocator::f_GranularityCommit(ELargePages) <= (uint8 *)(void *)pChunk->m_pStart + pChunk->m_Size);
						mint ThisChunkSize = fg_Min(_Size, pChunk->m_Size - (pMem - (uint8 *)(void *)pChunk->m_pStart));
		
						CAllocator::f_Commit(pMem, ThisChunkSize);
						_Size -= ThisChunkSize;
						pMem += ThisChunkSize;
						++Iter;
					}
				}
				DMibFastCheck(_Size == 0); // "All memory should be committed"
			}
		}

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::CChunk::f_Decommit(void *_pMem, mint _Size)
		{
			if (_Size <= CAllocator::f_GranularityCommit(ELargePages)) // Only one page, we cannot span multiple pages
			{
				CAllocator::f_Decommit(_pMem, _Size);
			}
			else
			{
				CSubChunkIter Iter;
				Iter.f_InitForSearch(m_SubChunks);
				typename CSubChunk::CDynamicPtr Ptr;
				Ptr = _pMem;
				if (Iter.f_FindLargestLessThanEqualForward(Ptr))
				{
					uint8 *pMem = (uint8 *)_pMem;
					while (Iter && _Size)
					{
						CSubChunk *pChunk = Iter;
						DMibFastCheck(pMem >= (uint8 *)(void *)pChunk->m_pStart && pMem + CAllocator::f_GranularityCommit(ELargePages) <= (uint8 *)(void *)pChunk->m_pStart + pChunk->m_Size);
						mint ThisChunkSize = fg_Min(_Size, pChunk->m_Size - (pMem - (uint8 *)(void *)pChunk->m_pStart));
		
						CAllocator::f_Decommit(pMem, ThisChunkSize);
						_Size -= ThisChunkSize;
						pMem += ThisChunkSize;
						++Iter;
					}
				}
				DMibFastCheck(_Size == 0); // "All memory should be decommitted"
			}
		}

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::fp_AddChunk(void *_pChunk, mint _ChunkSize)
		{
			if (!_pChunk)
				return;
			// We need this size so we can fit two edge blocks and a FreeExtended block in the chunk. Althogh this small a chunk should not be used

			// First check if we can append this pool to another pool

			// Try with lower first

			const void *pChunkConst = _pChunk;
			CChunk* pChunk = (CChunk *)m_pChunksTree->f_FindSmallestGreaterThanEqual(pChunkConst);
			CBlock* pPrevBlock = nullptr;
			CBlock* pNextBlock = nullptr;
			CBlock_Free *pFreeBlock;
			if (pChunk && (uint8*)pChunk->f_GetBasePointer() == ((uint8*)_pChunk + _ChunkSize) && pChunk->m_pHeap == this)
			{
				m_pChunksTree->f_Remove(pChunk);
				// Commit start of block
				// We have a fit
				// Knit in the chunk at bottom of chunk
				CSubChunk *pSubChunk = m_PoolSubChunks.f_New();

				// Set up a new chunk
				pSubChunk->m_pStart = (void *)_pChunk;
				pSubChunk->m_Size = _ChunkSize;
				pChunk->m_SubChunks.f_Insert(pSubChunk);

				CBlock *pLastBase = pChunk->f_GetFirstBlock();
				pChunk->m_pBase = _pChunk;
				pChunk->m_Size += _ChunkSize;
				pPrevBlock = pChunk->f_GetFirstBlock();

				if (f_CanCommit())
				{
					fsp_CommitDualRange(pChunk, (uint8 *)pPrevBlock, (uint8 *)pPrevBlock + EBlockNonAllocSize, 
						(uint8 *)pPrevBlock + EBlockMinNext, (uint8 *)pPrevBlock + EBlockMinNext + fsp_CommitSize_FreeExtendedStart());
				}

				pPrevBlock->f_SetAll(EBlockMinNext >> EAlignBits, 0, EBlockType_Edge);
				pFreeBlock = (CBlock_Free *)((uint8 *)pPrevBlock + EBlockMinNext);

				DMibFastCheck(pLastBase->f_Type() == EBlockType_Edge); // "Must be an edgeblock"

				CBlock *pNext = pLastBase->f_GetNextNormal();

				if (pNext->f_IsFree())
				{
					pNextBlock = pNext->f_GetNext();

					fp_UntieFreeBlock<true>((CBlock_Free *)pNext);

					if (f_CanCommit())
					{
						// Decommit last base, and start of free block
						uint8 *pStart = fp_AlignDown((uint8 *)pLastBase);
						uint8 *pEnd = fp_AlignUp((uint8 *)pLastBase + EBlockNonAllocSize);
						if (pEnd > (uint8 *)pNextBlock)
							pEnd = fp_AlignDown((uint8 *)pNextBlock);

						uint8 *pStart2 = fp_AlignDown((uint8 *)pNext);
						uint8 *pEnd2 = fp_AlignUp((uint8 *)pNext + fsp_CommitSize_FreeStart(pNext->f_GetSize()));
						if (pEnd2 > (uint8 *)pNextBlock)
							pEnd2 = fp_AlignDown((uint8 *)pNextBlock);

						fsp_DecommitDualRangeAligned(pChunk, pStart, pEnd, pStart2, pEnd2);
					}
				}				
				else
				{
					if (f_CanCommit())
					{
						uint8 *pStart = fp_AlignDown((uint8 *)pLastBase);
						uint8 *pEnd = fp_AlignUp((uint8 *)pLastBase + EBlockNonAllocSize);
						if (pEnd > (uint8 *)pFreeBlock)
							pEnd = fp_AlignDown((uint8 *)pFreeBlock);

						fsp_DecommitRangeAligned(pChunk, pStart, pEnd);
					}

					pNextBlock = pNext;
				}

				m_pChunksTree->f_Insert(pChunk);
			}
			else
			{
				// Try with higher
				pChunkConst = _pChunk;
				pChunk = (CChunk *)m_pChunksTree->f_FindLargestLessThanEqual(pChunkConst);

				if (pChunk && (uint8*)pChunk->f_GetEndPointer() == ((uint8*)_pChunk) && pChunk->m_pHeap == this)
				{
					m_pChunksTree->f_Remove(pChunk);
					// We have a fit
					// Knit in the sub chunk at top of chunk
					CSubChunk *pSubChunk = m_PoolSubChunks.f_New();

					// Set up a new chunk
					pSubChunk->m_pStart = _pChunk;
					pSubChunk->m_Size = _ChunkSize;
					pChunk->m_SubChunks.f_Insert(pSubChunk);

					CBlock *pLastEnd = pChunk->f_GetLastBlock();
					pChunk->m_Size += _ChunkSize;
					pNextBlock = pChunk->f_GetLastBlock();

					if (f_CanCommit())
					{
						// Commit End
						fsp_CommitRange(pChunk, (uint8 *)pChunk->f_GetLastBlock() - fsp_CommitSize_FreeExtendedEnd(), (uint8 *)pChunk->f_GetLastBlock() + EBlockNonAllocSize);
					}

					DMibFastCheck(pLastEnd->f_Type() == EBlockType_Edge); // "Must be an edgeblock"

					CBlock *pPrev = pLastEnd->f_GetPrevNormal();

					if (pPrev->f_IsFree())
					{
						pPrevBlock = pPrev->f_GetPrev();

						fp_UntieFreeBlock<true>((CBlock_Free *)pPrev);
						pFreeBlock = (CBlock_Free *)pPrev;

						if (f_CanCommit())
						{
							{
								uint8 *pCommitMem;
								if (ENeedExtendedBlocks && pPrev->f_Type() == EBlockType_FreeExtended)
									pCommitMem = fp_AlignDown(((uint8*)pLastEnd) - EBlockFreeExtendedSizeEnd);
								else
									pCommitMem = fp_AlignDown((uint8 *)pLastEnd);

								if (pCommitMem < ((uint8*)pPrev + sizeof(CBlock_Free)))
								{
									pCommitMem = fp_AlignUp((uint8*)pPrev + sizeof(CBlock_Free));
								}
										
								uint8 *pBlockEnd = fp_AlignUp((uint8 *)pLastEnd + EBlockNonAllocSize);
								if (pBlockEnd > (uint8 *)pNextBlock)
								{
									pBlockEnd = fp_AlignDown((uint8*)pNextBlock);
								}

								if (pBlockEnd > pCommitMem)
								{
									pChunk->f_Decommit(pCommitMem, pBlockEnd - pCommitMem);
								}
							}
							// Check if we need to commit end of block
							if (ENeedExtendedBlocks && pPrev->f_Type() == EBlockType_Free)
							{
								uint8 *pCommitStart = fp_AlignUp((uint8 *)pFreeBlock + sizeof(CBlock_Free));
								uint8 *pCommitEnd = fp_AlignUp((uint8 *)pFreeBlock + EBlockFreeExtendedSizeStart);

								if (pCommitEnd > pCommitStart)
									pChunk->f_Commit(pCommitStart, pCommitEnd - pCommitStart);
							}
						}
					}				
					else
					{
						if (f_CanCommit())
						{
 							uint8 *pStartBlock = fp_AlignDown((uint8*)pLastEnd + EBlockNonAllocSize);
							uint8 *pEndBlock = fp_AlignUp((uint8*)pLastEnd + EBlockFreeExtendedSizeStart);
							pChunk->f_Commit(pStartBlock, pEndBlock - pStartBlock);
						}

						pFreeBlock = (CBlock_Free *)pLastEnd;
						pPrevBlock = pPrev;
					}
					if (ENeedExtendedBlocks)
						pNextBlock->f_SetAll(0,0,EBlockType_Edge);
					else
						pNextBlock->f_SetAll(0,((uint8 *)pNextBlock - (uint8 *)pFreeBlock) >> EAlignBits,EBlockType_Edge);
					m_pChunksTree->f_Insert(pChunk);
				}
				else
				{
					if (f_CanCommit())
					{
						mint ToCommitStart = fp_AlignUp(EBlockFreeExtendedSizeStart + EBlockNonAllocSize);
						pChunk->f_Commit(_pChunk, ToCommitStart);
						if (_ChunkSize > CAllocator::f_GranularityCommit(ELargePages))
						{
							// We need to commit at end also
							void *pEndBlock = fp_AlignDown((uint8 *)_pChunk + _ChunkSize - EBlockNonAllocSize - EBlockFreeExtendedSizeEnd);
							mint ToCommitEnd = fp_AlignUp(((uint8*)_pChunk + _ChunkSize) - (uint8*)pEndBlock);
							pChunk->f_Commit(pEndBlock, ToCommitEnd);
						}
					}

					pChunk = m_PoolChunks.f_New();
					pChunk->m_pHeap = this;
					pChunk->m_pBase = _pChunk;
					pChunk->m_Size = _ChunkSize;

					CSubChunk *pSubChunk = m_PoolSubChunks.f_New();

					pSubChunk->m_pStart = _pChunk;
					pSubChunk->m_Size = _ChunkSize;
					pChunk->m_SubChunks.f_Insert(pSubChunk);

					pPrevBlock = pChunk->f_GetFirstBlock();
					pPrevBlock->f_SetAll(EBlockMinNext >> EAlignBits, 0, EBlockType_Edge);
					pFreeBlock = (CBlock_Free *)((uint8 *)pPrevBlock + EBlockMinNext);

					pNextBlock = (CBlock *)(((uint8 *)_pChunk) + _ChunkSize - EBlockNonAllocSize);
					if (ENeedExtendedBlocks)
						pNextBlock->f_SetAll(0,0,EBlockType_Edge);
					else
						pNextBlock->f_SetAll(0,((uint8 *)pNextBlock - (uint8 *)pFreeBlock) >> EAlignBits,EBlockType_Edge);
					m_pChunksTree->f_Insert(pChunk);
				}
			}

			fp_InitFreeBlock(pFreeBlock, pNextBlock, pPrevBlock);
		}


	}
}
