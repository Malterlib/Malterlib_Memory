// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

namespace NMib
{
	namespace NMem
	{

		/***************************************************************************************************\
		|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
		| Alloc																								|
		|___________________________________________________________________________________________________|
		\***************************************************************************************************/

		template <class t_CHeapParams>
		void *TCHeap<t_CHeapParams>::f_Alloc(mint &_Size)
		{
//				DMibTrace("sizeof(CBlock) {}" DMibNewLine, sizeof(CBlock));
			mint Size = _Size;
			Size = Size + EBlockWholeSize;
			if (Size < ESmallestBlock)
				Size = ESmallestBlock;

			// Align the size to the current alignment
			Size = ((Size + EAlignAdd) & (mint)EAlignAnd) ;
			DMibFastCheck(!(Size & (~((mint)EAlignAnd)))); // "Size is not aligned right"

			if (EMaxCachedBlockSize > 0)
			{
				if (Size <= EMaxCachedBlockSize)
				{
					//mint Place = ((Size - (ESmallestBlock)) >> EAlignBits);
					//mint ListSize = sizeof(m_aCachedBlocks) / sizeof(m_aCachedBlocks[0]);
					//DMibTrace("{}, {}" DMibNewLine, Place << ListSize);
					DMibFastCheck( ((Size - (ESmallestBlock)) >> EAlignBits) < sizeof(m_aCachedBlocks) / sizeof(m_aCachedBlocks[0])); // "Must fit in cache list"
					//DMibFastCheck(Place < ListSize) // "Must fit in cache list"
					CBlock_Cached *pBlock = m_aCachedBlocks[(Size - (ESmallestBlock)) >> EAlignBits].f_Pop();
					if (pBlock)
					{
						void *pMem = ((uint8 *)pBlock + EBlockPreSize);
						Size = pBlock->f_GetSizeNormal();
						if (CFillDebug::EDoFills)
						{
							// Check that noone has overwritten us
							CFillDebug::fs_CheckFree((uint8 *)pBlock + sizeof(CBlock_Cached), Size - sizeof(CBlock_Cached));
							// This block is allocated
							CFillDebug::fs_FillGuard((uint8 *)pMem - EBlockPreGuardOffset, EBlockPreGuardSize);
							CFillDebug::fs_FillAllocated(pMem, Size - EBlockWholeSize);
							CFillDebug::fs_FillGuard((uint8 *)pBlock + (Size - EBlockPostGuardSize), EBlockPostGuardSize);
						}
						_Size = Size - EBlockWholeSize;
						return pMem;
					}
				}
			}

			void *pMem = fp_AllocInternal(Size);
			_Size = Size - EBlockWholeSize;
			return pMem;
		}

		template <class t_CHeapParams>
		void *TCHeap<t_CHeapParams>::fp_AllocInternalFromBlock(mint &_Size, CFreeBlockBucket *_pFreeBlockBucket, mint _PrevSize, mint _Alignment)
		{
			return fp_AllocInternalFromBlockInline<true>(_Size, _pFreeBlockBucket, _PrevSize, _Alignment);
		}
		
		template <class t_CHeapParams>
		template <bool tf_bAlign>
		inline_always void *TCHeap<t_CHeapParams>::fp_AllocInternalFromBlockInline(mint &_Size, CFreeBlockBucket *_pFreeBlockBucket, mint _PrevSize, mint _Alignment)
		{
			static const bint bTreeOpt0 = !t_CHeapParams::EbOptimizeForSize && t_CHeapParams::CSizeHolder::EStoresSize && t_CHeapParams::ETreeOpt0;
			static const bint bTreeOpt1 = !t_CHeapParams::EbOptimizeForSize && t_CHeapParams::CSizeHolder::EStoresSize && t_CHeapParams::ETreeOpt1;
			
			auto *pFreeBlockBucket = _pFreeBlockBucket;
			mint PrevSize = _PrevSize;
			mint Size = _Size;
			CBlock_Free* pFreeBlock = pFreeBlockBucket->m_FreeBlocks.f_GetFirst();
			CChunk *pChunk = nullptr;

			if (f_CanCommit())
				pChunk = (CChunk *)m_pChunksTree->f_FindLargestLessThanEqual(pFreeBlock);

			mint BlockSize = pFreeBlock->f_GetSize();
			uint32 Type = pFreeBlock->f_Type();
			fp_CheckFreeBlock(pFreeBlock, Size);

			CBlock *pBlock = (CBlock*)pFreeBlock;
			mint LeftOverBeforeSize = 0;
			if (tf_bAlign)
			{
				uint8 *pStart = (uint8 *)pFreeBlock + EBlockPreSize;
				uint8 *pAlignedStart = fg_AlignUp(pStart, _Alignment);
				pBlock = (CBlock *)(pAlignedStart - EBlockPreSize);
				LeftOverBeforeSize = (uint8 *)pBlock - (uint8 *)pFreeBlock;
				if (LeftOverBeforeSize && LeftOverBeforeSize < ESmallestBlock)
				{
					while (LeftOverBeforeSize < ESmallestBlock)
					{
						pAlignedStart += _Alignment;
						pBlock = (CBlock *)(pAlignedStart - EBlockPreSize);
						LeftOverBeforeSize = (uint8 *)pBlock - (uint8 *)pFreeBlock;
					}
				}
				DMibFastCheck(BlockSize >= LeftOverBeforeSize);
				BlockSize -= LeftOverBeforeSize;
			}
			CBlock *pNextBlock = pFreeBlock->f_GetNext();
			CBlock_Free *pLeftOverBlock = (CBlock_Free *)(((uint8 *)pBlock) + Size);
			//DMibFastCheck(((uint8 *)pLeftOverBlock + sizeof(CBlock_Free)) < (uint8 *)pNextBlock || );
			DMibFastCheck((uint8 *)pNextBlock == (uint8 *)pBlock + BlockSize); // "The block data and the block bucket data does not correspond. The heap is corrupt"

			DMibFastCheck(BlockSize >= Size);
			mint LeftOverSize = BlockSize - Size;

			DMibFastCheck((LeftOverSize & (~((mint)EAlignAnd))) == 0); // "Left over size has to be aligned"

			if (tf_bAlign && f_CanCommit() && LeftOverBeforeSize > 0)
			{
				if
					(
						ENeedExtendedBlocks
						&&
						(
							(t_CHeapParams::EbOptimizeForSize && LeftOverBeforeSize >= sizeof(CBlock_FreeExtendedBucket))
							|| (!t_CHeapParams::EbOptimizeForSize && LeftOverBeforeSize >= (mint)ELargestBlock)
						)
					)
				{
					if (t_CHeapParams::EbOptimizeForSize)
						fsp_CommitRangeAlreadyCommitted(pChunk, (uint8 *)pBlock, (uint8 *)pBlock + sizeof(CBlock_FreeExtendedBucket), (uint8 *)pFreeBlock);
					else
						fsp_CommitRangeAlreadyCommitted(pChunk, (uint8 *)pBlock, (uint8 *)pBlock + sizeof(CBlock_FreeExtended), (uint8 *)pFreeBlock);
				}
				else if (LeftOverBeforeSize >= ESmallestBlock)
				{
					fsp_CommitRangeAlreadyCommitted(pChunk, (uint8 *)pBlock, (uint8 *)pBlock + sizeof(CBlock_Free), (uint8 *)pFreeBlock);
				}
				else
				{
					DMibFastCheck(false); // Internal error
				}
			}

			if
				(
					ENeedExtendedBlocks
					&&
					(
						(t_CHeapParams::EbOptimizeForSize && LeftOverSize >= sizeof(CBlock_FreeExtendedBucket))
						|| (!t_CHeapParams::EbOptimizeForSize && LeftOverSize >= (mint)ELargestBlock)
					)
				)
			{
				// We can let the block bucket reside in the free block itself

				if (f_CanCommit())
				{
					fsp_CommitBlockNextExtended(pChunk, pBlock, pNextBlock, pLeftOverBlock);
				}

				// We need to untie the free block after we tried to commit, otherwise an exception in commit will leave the heap invalid
				fp_UntieFreeBlock<!bTreeOpt0>(pFreeBlock);

				pLeftOverBlock->f_SetAll(0, Size >> EAlignBits, EBlockType_FreeExtended);
				((CBlock_FreeExtendedBase *)pLeftOverBlock)->m_pNext = pNextBlock;
				pNextBlock->f_SetPrev(0);

				// Set ThisPtr
				*(((TCDynamicPtr<CPtrHolder, CBlock> *)(((uint8 *)pLeftOverBlock) + LeftOverSize)) - 1) = pLeftOverBlock;

				if (t_CHeapParams::EbOptimizeForSize)
				{
					((CBlock_FreeExtendedBucket *)pLeftOverBlock)->f_Construct();

					mint FindSize = LeftOverSize;
					if (FindSize > (mint)ELargestBlock)
						FindSize = (mint)ELargestBlock;

					// Check to see if it already exists a bucket
					pFreeBlockBucket = fp_GetBlockBucketExact(FindSize);
					if (!pFreeBlockBucket)
					{
						pFreeBlockBucket = &((CBlock_FreeExtendedBucket *)pLeftOverBlock)->m_FreeBlockBucket;
						pFreeBlockBucket->f_Construct(pLeftOverBlock, FindSize, this);
					}
					else
						pFreeBlockBucket->m_FreeBlocks.f_Push(pLeftOverBlock);
				}
				else
				{
					((CBlock_FreeExtended *)pLeftOverBlock)->f_Construct();
					
					if (bTreeOpt0)
						pFreeBlockBucket->m_FreeBlocks.f_Push(pLeftOverBlock); // The block was larger than ELargestBlock and still is, lets just put it back
					else
					{
						mint FindSize = ELargestBlock;
						CFreeBlockBucket *pFreeOld = fp_GetBlockBucketExact(FindSize);;
						// Check to see if it already exists a bucket
						if (pFreeOld)
							pFreeOld->m_FreeBlocks.f_Push(pLeftOverBlock);
						else
						{
							pFreeBlockBucket = fp_NewFreeBlockBucket(FindSize);
							pFreeBlockBucket->f_Construct(pLeftOverBlock, FindSize, this);
						}
					}
				}
			}
			else if (LeftOverSize >= ESmallestBlock)
			{
				if (f_CanCommit())
				{
					if (ENeedExtendedBlocks && Type == EBlockType_FreeExtended)
						fsp_CommitBlockNextExtended(pChunk, pBlock, pNextBlock, pLeftOverBlock);
					else
						fsp_CommitBlockNextFree(pChunk, pBlock, pNextBlock, pLeftOverBlock);
				}

				// We need to untie the free block after we tried to commit, otherwise an exception in commit will leave the heap invalid
				fp_UntieFreeBlock<!bTreeOpt0>(pFreeBlock);

				pLeftOverBlock->f_SetAll(LeftOverSize >> EAlignBits, Size >> EAlignBits, EBlockType_Free);
				pNextBlock->f_SetPrev(LeftOverSize >> EAlignBits);

				pLeftOverBlock->f_Construct();

				if (EFreeSizeBucketTreeThresholdBits > 0 && LeftOverSize <= 1 << EFreeSizeBucketTreeThresholdBits)
				{
					PrevSize = ~mint(0);
				}

				if (bTreeOpt1 && LeftOverSize > PrevSize && pFreeBlockBucket->m_FreeBlocks.f_IsEmpty())
				{
					pFreeBlockBucket->f_SetSize(LeftOverSize);
					pFreeBlockBucket->m_FreeBlocks.f_Push(pLeftOverBlock);
				}
				else
				{
					CFreeBlockBucket *pFreeOld = fp_GetBlockBucketExact(LeftOverSize);
					// Check to see if it already exists a bucket
					if (!pFreeOld)
					{
						if (bTreeOpt0 && pFreeBlockBucket->m_FreeBlocks.f_IsEmpty())
						{
							pFreeBlockBucket->f_Destruct(this);
							pFreeBlockBucket->f_AddBlockBucket(this, LeftOverSize);
							pFreeBlockBucket->m_FreeBlocks.f_Push(pLeftOverBlock);
						}
						else
						{
							pFreeBlockBucket = fp_NewFreeBlockBucket(LeftOverSize);
							pFreeBlockBucket->f_Construct(pLeftOverBlock, LeftOverSize, this);
						}
					}
					else
					{
						if (bTreeOpt0 && pFreeBlockBucket != pFreeOld && pFreeBlockBucket->m_FreeBlocks.f_IsEmpty())
						{
							pFreeBlockBucket->f_Destruct(this);
							fp_DeleteFreeBlockBucket(pFreeBlockBucket);
						}
						pFreeOld->m_FreeBlocks.f_Push(pLeftOverBlock);
					}
				}
				if (Type == EBlockType_FreeExtended) // Changed from free extended to free
					fp_FillFreeBlock(pLeftOverBlock);
			}
			else
			{
				if (f_CanCommit())
				{
					if (ENeedExtendedBlocks && Type == EBlockType_FreeExtended)
						fsp_CommitBlockExtended(pChunk, pBlock, pNextBlock);
					else
						fsp_CommitBlockNormal(pChunk, pBlock, pNextBlock);
				}

				// We need to untie the free block after we tried to commit, otherwise an exception in commit will leave the heap invalid
				fp_UntieFreeBlock<!bTreeOpt0>(pFreeBlock);

				if (bTreeOpt0 && pFreeBlockBucket->m_FreeBlocks.f_IsEmpty())
				{
					pFreeBlockBucket->f_Destruct(this);
					fp_DeleteFreeBlockBucket(pFreeBlockBucket);
				}

				DMibFastCheck(BlockSize < (mint)ELargestBlock); // "Overflow error will occur"

				pNextBlock->f_SetPrev(BlockSize >> EAlignBits);
				_Size = BlockSize;
				Size = BlockSize;
			}

			if (tf_bAlign && LeftOverBeforeSize > 0)
			{
				pBlock->f_SetAll(Size >> EAlignBits, LeftOverBeforeSize >> EAlignBits, EBlockType_Normal);

				if
					(
						ENeedExtendedBlocks
						&&
						(
							(t_CHeapParams::EbOptimizeForSize && LeftOverBeforeSize >= sizeof(CBlock_FreeExtendedBucket))
							|| (!t_CHeapParams::EbOptimizeForSize && LeftOverBeforeSize >= (mint)ELargestBlock)
						)
					)
				{

					pFreeBlock->f_SetNext(LeftOverBeforeSize >> EAlignBits);
					pFreeBlock->f_SetType(EBlockType_FreeExtended);

					((CBlock_FreeExtendedBase *)pFreeBlock)->m_pNext = pNextBlock;
					pBlock->f_SetPrev(0);

					// Set ThisPtr
					*(((TCDynamicPtr<CPtrHolder, CBlock> *)(((uint8 *)pFreeBlock) + LeftOverBeforeSize)) - 1) = pFreeBlock;

					if (t_CHeapParams::EbOptimizeForSize)
					{
						((CBlock_FreeExtendedBucket *)pFreeBlock)->f_Construct();

						mint FindSize = LeftOverBeforeSize;
						if (FindSize > (mint)ELargestBlock)
							FindSize = (mint)ELargestBlock;

						// Check to see if it already exists a bucket
						auto pFreeBlockBucket = fp_GetBlockBucketExact(FindSize);
						if (!pFreeBlockBucket)
						{
							pFreeBlockBucket = &((CBlock_FreeExtendedBucket *)pFreeBlock)->m_FreeBlockBucket;
							pFreeBlockBucket->f_Construct(pFreeBlock, FindSize, this);
						}
						else
							pFreeBlockBucket->m_FreeBlocks.f_Push(pFreeBlock);
					}
					else
					{
						((CBlock_FreeExtended *)pFreeBlock)->f_Construct();
					
						mint FindSize = ELargestBlock;
						CFreeBlockBucket *pFreeBlockBucket = fp_GetBlockBucketExact(FindSize);;
						// Check to see if it already exists a bucket
						if (pFreeBlockBucket)
							pFreeBlockBucket->m_FreeBlocks.f_Push(pFreeBlock);
						else
						{
							pFreeBlockBucket = fp_NewFreeBlockBucket(FindSize);
							pFreeBlockBucket->f_Construct(pFreeBlock, FindSize, this);
						}
					}
				}
				else if (LeftOverBeforeSize >= ESmallestBlock)
				{
					pFreeBlock->f_SetNext(LeftOverBeforeSize >> EAlignBits);
					pFreeBlock->f_SetType(EBlockType_Free);

					pFreeBlock->f_Construct();

					CFreeBlockBucket *pFreeBlockBucket = fp_GetBlockBucketExact(LeftOverBeforeSize);
					// Check to see if it already exists a bucket
					if (!pFreeBlockBucket)
					{
						pFreeBlockBucket = fp_NewFreeBlockBucket(LeftOverBeforeSize);
						pFreeBlockBucket->f_Construct(pFreeBlock, LeftOverBeforeSize, this);
					}
					else
						pFreeBlockBucket->m_FreeBlocks.f_Push(pFreeBlock);
				}
				else
				{
					DMibFastCheck(false); // "Must not get here"
				}
				fp_FillFreeBlock(pFreeBlock);
			}
			else
			{
				pBlock->f_SetType(EBlockType_Normal);
				pBlock->f_SetNext(Size >> EAlignBits);
			}

			void *pRet = ((uint8 *)pBlock + EBlockPreSize);
			if (CFillDebug::EDoFills)
			{
				mint Size = pBlock->f_GetSizeNormal();
				CFillDebug::fs_FillGuard((uint8 *)pRet - EBlockPreGuardOffset, EBlockPreGuardSize);
				CFillDebug::fs_FillAllocated(pRet, Size - EBlockWholeSize);
				CFillDebug::fs_FillGuard((uint8 *)pBlock + (Size - EBlockPostGuardSize), EBlockPostGuardSize);
			}
			return pRet;
		}

		template <class t_CHeapParams>
		inline_always typename TCHeap<t_CHeapParams>::CFreeBlockBucket *TCHeap<t_CHeapParams>::fp_GetFreeBlockBucketInline(mint _Size, mint &_PrevSize)
		{
			mint Size = _Size;
			static const bint bTreeOpt1 = !t_CHeapParams::EbOptimizeForSize && t_CHeapParams::CSizeHolder::EStoresSize && t_CHeapParams::ETreeOpt1;
			mint MaxSize = (mint)ELargestBlock;
			MaxSize -= ((mint)1) << EAlignBits;
			if (Size > MaxSize)
			{
				DMibFastCheck(false); // "Trying to allocate to big a block"
				return nullptr;
			}
			
			if (EMaxCachedBlockSize > 0)
			{
				if (Size > (mint)t_CHeapParams::ECacheFreeThreshold)
				{
					fp_FreeBlockCache();
				}
			}
			
			// First try to find a free block that is larger than the the requested size, but as small as there is
			CFreeBlockBucket *pFreeBlockBucket;
			mint PrevSize = ~mint(0);
			
			if (bTreeOpt1)
				pFreeBlockBucket = fp_GetBlockBucket(Size, PrevSize);
			else
				pFreeBlockBucket = fp_GetBlockBucket(Size);
			
			if (!pFreeBlockBucket)
			{
				// No such block exists, allocate new memory for it
				mint SizeToVirtualAlloc = NMib::fg_AlignUp((mint)t_CHeapParams::EGrowSize, CAllocator::f_GranularityAlloc(ELargePages));
				
				mint CanFit = SizeToVirtualAlloc - (EBlockMinNext + EBlockNonAllocSize + (EBlockFirstAdd - EBlockNonAllocSize));
				if (CanFit < Size)
					SizeToVirtualAlloc = NMib::fg_AlignUp(NMib::fg_AlignUp(Size + (EBlockMinNext + EBlockNonAllocSize + (EBlockFirstAdd - EBlockNonAllocSize)), (mint)t_CHeapParams::EGrowSize), CAllocator::f_GranularityAlloc(ELargePages));
				
				if (f_CanCommit())
					fp_AddChunk(CAllocator::f_Alloc(SizeToVirtualAlloc, mc_AllocationFlags | EAllocationFlag_WillFreeWithSize | EAllocationFlag_NoCommit | (ELargePages ? EAllocationFlag_LargePages : EAllocationFlag_None)), SizeToVirtualAlloc);
				else
					fp_AddChunk(CAllocator::f_Alloc(SizeToVirtualAlloc, mc_AllocationFlags | EAllocationFlag_WillFreeWithSize | (ELargePages ? EAllocationFlag_LargePages : EAllocationFlag_None)), SizeToVirtualAlloc);
				if (bTreeOpt1)
					pFreeBlockBucket = fp_GetBlockBucket(Size, PrevSize);
				else
					pFreeBlockBucket = fp_GetBlockBucket(Size);
				if (!pFreeBlockBucket)
					return nullptr;
			}
			
			_PrevSize = PrevSize;
			
			return pFreeBlockBucket;
		}

		template <class t_CHeapParams>
		typename TCHeap<t_CHeapParams>::CFreeBlockBucket *TCHeap<t_CHeapParams>::fp_GetFreeBlockBucket(mint _Size, mint &_PrevSize)
		{
			return fp_GetFreeBlockBucketInline( _Size, _PrevSize);
		}
		
		template <class t_CHeapParams>
		void *TCHeap<t_CHeapParams>::fp_AllocInternal(mint &_Size)
		{
			mint PrevSize;
			CFreeBlockBucket *pFreeBlockBucket = fp_GetFreeBlockBucketInline(_Size, PrevSize);
			if (!pFreeBlockBucket)
				return nullptr;

			return fp_AllocInternalFromBlockInline<false>(_Size, pFreeBlockBucket, PrevSize, EAlign);
		}

	}
}
