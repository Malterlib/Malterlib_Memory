// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

namespace NMib
{
	namespace NMem
	{

		/***************************************************************************************************\
		|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
		| Free block																						|
		|___________________________________________________________________________________________________|
		\***************************************************************************************************/

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::fp_InitFreeBlock(CBlock_Free *pFreeBlock, CBlock *pNextBlock, CBlock *pPrevBlock)
		{
			mint FreeMemorySize = ((uint8 *)pNextBlock - (uint8 *)pFreeBlock);
			DMibFastCheck(FreeMemorySize >= sizeof(CBlock_FreeExtended)); // "The chunk you added was too small"

			// We add a start and an end block to allow size operation to be less complex

			bint bNeedExtended = (ENeedExtendedBlocks && ((t_CHeapParams::EbOptimizeForSize && FreeMemorySize >= sizeof(CBlock_FreeExtendedBucket)) || (!t_CHeapParams::EbOptimizeForSize && FreeMemorySize >= (mint)ELargestBlock)));

			// Block in middle
//				pFreeBlock->m_FreeLink.f_Construct();
			if (bNeedExtended)
			{
				if (t_CHeapParams::EbOptimizeForSize)
				{
					((CBlock_FreeExtendedBucket *)pFreeBlock)->f_Construct();
				}
				else
				{
					((CBlock_FreeExtended *)pFreeBlock)->f_Construct();
				}
			}
			else
			{
				pFreeBlock->f_Construct();
			}

			if (bNeedExtended)
			{
				pFreeBlock->f_SetAll(0, ((uint8 *)pFreeBlock - (uint8 *)pPrevBlock) >> EAlignBits, EBlockType_FreeExtended);
				((CBlock_FreeExtendedBase *)pFreeBlock)->m_pNext = pNextBlock;
				pNextBlock->f_SetPrev(0);
				TCDynamicPtr<CPtrHolder, CBlock> *pFreeBlockEndTag = (TCDynamicPtr<CPtrHolder, CBlock> *)((uint8 *)pFreeBlock + FreeMemorySize - EBlockFreeExtendedSizeEnd);
				*pFreeBlockEndTag = (CBlock *)pFreeBlock;
			}
			else
			{
				pFreeBlock->f_SetAll(((uint8 *)pNextBlock - (uint8 *)pFreeBlock) >> EAlignBits, ((uint8 *)pFreeBlock - (uint8 *)pPrevBlock) >> EAlignBits, EBlockType_Free);
				pNextBlock->f_SetPrev(((uint8 *)pNextBlock - (uint8 *)pFreeBlock) >> EAlignBits);
			}
	
			CFreeBlockBucket *pFreeBlockBucket;

			mint FindSize = FreeMemorySize;
			if (FindSize > (mint)ELargestBlock)
				FindSize = (mint)ELargestBlock;

			pFreeBlockBucket = fp_GetBlockBucketExact(FindSize);
				
			fp_FillFreeBlock(pFreeBlock);

			if (!pFreeBlockBucket)
			{
				if (bNeedExtended && t_CHeapParams::EbOptimizeForSize)
				{
					pFreeBlockBucket = &((CBlock_FreeExtendedBucket *)pFreeBlock)->m_FreeBlockBucket;
				}
				else
				{
					pFreeBlockBucket = fp_NewFreeBlockBucket(FindSize);
				}

				pFreeBlockBucket->f_Construct(pFreeBlock, FindSize, this);
			}
			else
			{
				pFreeBlockBucket->m_FreeBlocks.f_Push(pFreeBlock);
			}
		}

		template <class t_CHeapParams>
		template <bint t_bRemove>
		void TCHeap<t_CHeapParams>::fp_UntieFreeBlock(CBlock_Free* _pFreeBlock)
		{
			DMibFastCheck(_pFreeBlock->m_FreeLink.f_IsInList()); // "Must be free"
			if (ENeedExtendedBlocks && t_CHeapParams::EbOptimizeForSize && _pFreeBlock->f_Type() == EBlockType_FreeExtended)
			{
				// We have an extended block

				if (_pFreeBlock->m_FreeLink.f_IsAloneInList())
				{
					// The block is the only one in the list so we need to take care of the associated free block bucket
						
					CFreeBlockBucket *pFreeBlockBucket = DMibGetParent(CFreeBlockBucket, m_FreeBlocks.m_Link, _pFreeBlock->m_FreeLink.fp_GetPrevNotList());
					DMibFastCheck(pFreeBlockBucket == &((CBlock_FreeExtendedBucket *)_pFreeBlock)->m_FreeBlockBucket); // "Must be in this block"

					pFreeBlockBucket->f_Destruct(this);
						
					// We must remove ourselfs from tree before we remove the last link
					DMibFastCheck(!((CBlock_FreeExtendedBucket *)_pFreeBlock)->m_FreeBlockBucket.m_FreeBlockBucketsLink.f_IsInTree()); // "Must not be in tree now"

					pFreeBlockBucket->m_FreeBlocks.f_Remove(_pFreeBlock);
					DMibFastCheck(pFreeBlockBucket->m_FreeBlocks.f_IsEmpty()); // "The list must be empty now, or an error has occurred"

				}
				else
				{
					_pFreeBlock->m_FreeLink.f_Unlink();
					CBlock_FreeExtendedBucket *pFreeBlockExtended = ((CBlock_FreeExtendedBucket *)_pFreeBlock);

					mint Size = _pFreeBlock->f_GetSize();
					if (pFreeBlockExtended->m_FreeBlockBucket.f_BucketUsed(this, Size))
					{
						// The Sizeclass is in use for this block, we will have to transfer it to another block
						CFreeBlockBucket *pFreeBlockBucket = &((CBlock_FreeExtendedBucket *)_pFreeBlock)->m_FreeBlockBucket;
//							DMibFastCheck(_pFreeBlock->f_GetSize() == pFreeBlockBucket->m_Size); // "humms"
						CFreeBlockBucket *pNewFreeBlockBucket = &((CBlock_FreeExtendedBucket *)pFreeBlockBucket->m_FreeBlocks.f_GetLast())->m_FreeBlockBucket;

						DMibFastCheck(!pNewFreeBlockBucket->m_FreeBlockBucketsLink.f_IsInTree()); // "humms"
						//DMibFastCheck(pNewFreeBlockBucket->m_FreeBlocks.f_IsEmpty()); // "humms"
						// This operation is a little expensive, maybe we can get in down to one find in the future by finding the prev item and just linking it in place
						// To do: Create a function in the tree so you can transfer the link to a new location

						pFreeBlockBucket->f_Destruct(this, Size);
						// Transfer list so the new treelink knows the size
						pNewFreeBlockBucket->f_Transfer(pFreeBlockBucket, this);
						pNewFreeBlockBucket->f_AddBlockBucket(this, Size);
                    }
				}
			}
			else
			{					
				if (t_CHeapParams::EbOptimizeForSize)
				{
					DMibFastCheck((_pFreeBlock->f_Type() == EBlockType_Free)); // "To untie a block it has to be free"
				}
				else
				{
					DMibFastCheck((_pFreeBlock->f_Type() == EBlockType_Free || _pFreeBlock->f_Type() == EBlockType_FreeExtended)); // "To untie a block it has to be free"
				}

				if (t_bRemove && _pFreeBlock->m_FreeLink.f_IsAloneInList())
				{
					// The block is the only one in the list so we need to take care of the associated free block bucket
						
					CFreeBlockBucket *pFreeBlockBucket = DMibGetParent(CFreeBlockBucket, m_FreeBlocks.m_Link, _pFreeBlock->m_FreeLink.fp_GetPrevNotList());

					pFreeBlockBucket->f_Destruct(this);
					_pFreeBlock->m_FreeLink.f_Unlink();
					DMibFastCheck(pFreeBlockBucket->m_FreeBlocks.f_IsEmpty()); // "The list must be empty now, or an error has occurred"

					// Return it to the pool of buckets
					fp_DeleteFreeBlockBucket(pFreeBlockBucket);
				}
				else
				{
					_pFreeBlock->m_FreeLink.f_Unlink();
				}

			}
		}

		template <class t_CHeapParams>
		template <bint t_bRemove>
		typename TCHeap<t_CHeapParams>::CFreeBlockBucket *TCHeap<t_CHeapParams>::fp_UntieFreeBlockRet(CBlock_Free* _pFreeBlock)
		{
			DMibFastCheck(_pFreeBlock->m_FreeLink.f_IsInList()); // "Must be free"
			if (ENeedExtendedBlocks && t_CHeapParams::EbOptimizeForSize && _pFreeBlock->f_Type() == EBlockType_FreeExtended)
			{
				// We have an extended block

				if (_pFreeBlock->m_FreeLink.f_IsAloneInList())
				{
					// The block is the only one in the list so we need to take care of the associated free block bucket
						
					CFreeBlockBucket *pFreeBlockBucket = DMibGetParent(CFreeBlockBucket, m_FreeBlocks.m_Link, _pFreeBlock->m_FreeLink.fp_GetPrevNotList());
					DMibFastCheck(pFreeBlockBucket == &((CBlock_FreeExtendedBucket *)_pFreeBlock)->m_FreeBlockBucket); // "Must be in this block"

					pFreeBlockBucket->f_Destruct(this);
						
					// We must remove ourselfs from tree before we remove the last link
					DMibFastCheck(!((CBlock_FreeExtendedBucket *)_pFreeBlock)->m_FreeBlockBucket.m_FreeBlockBucketsLink.f_IsInTree()); // "Must not be in tree now"

					pFreeBlockBucket->m_FreeBlocks.f_Remove(_pFreeBlock);
					DMibFastCheck(pFreeBlockBucket->m_FreeBlocks.f_IsEmpty()); // "The list must be empty now, or an error has occurred"

				}
				else
				{
					_pFreeBlock->m_FreeLink.f_Unlink();
					CBlock_FreeExtendedBucket *pFreeBlockExtended = ((CBlock_FreeExtendedBucket *)_pFreeBlock);

					mint Size = _pFreeBlock->f_GetSize();
					if (pFreeBlockExtended->m_FreeBlockBucket.f_BucketUsed(this, Size))
					{
						// The Sizeclass is in use for this block, we will have to transfer it to another block
						CFreeBlockBucket *pFreeBlockBucket = &((CBlock_FreeExtendedBucket *)_pFreeBlock)->m_FreeBlockBucket;
//							DMibFastCheck(_pFreeBlock->f_GetSize() == pFreeBlockBucket->m_Size); // "humms"
						CFreeBlockBucket *pNewFreeBlockBucket = &((CBlock_FreeExtendedBucket *)pFreeBlockBucket->m_FreeBlocks.f_GetLast())->m_FreeBlockBucket;

						DMibFastCheck(!pNewFreeBlockBucket->m_FreeBlockBucketsLink.f_IsInTree()); // "humms"
						//DMibFastCheck(pNewFreeBlockBucket->m_FreeBlocks.f_IsEmpty()); // "humms"
						// This operation is a little expensive, maybe we can get in down to one find in the future by finding the prev item and just linking it in place
						// To do: Create a function in the tree so you can transfer the link to a new location

						pFreeBlockBucket->f_Destruct(this, Size);
						// Transfer list so the new treelink knows the size
						pNewFreeBlockBucket->f_Transfer(pFreeBlockBucket, this);
						pNewFreeBlockBucket->f_AddBlockBucket(this, Size);
                    }
				}
			}
			else
			{					
				if (t_CHeapParams::EbOptimizeForSize)
				{
					DMibFastCheck((_pFreeBlock->f_Type() == EBlockType_Free)); // "To untie a block it has to be free"
				}
				else
				{
					DMibFastCheck((_pFreeBlock->f_Type() == EBlockType_Free || _pFreeBlock->f_Type() == EBlockType_FreeExtended)); // "To untie a block it has to be free"
				}

				if (_pFreeBlock->m_FreeLink.f_IsAloneInList())
				{
					// The block is the only one in the list so we need to take care of the associated free block bucket
						
					CFreeBlockBucket *pFreeBlockBucket = DMibGetParent(CFreeBlockBucket, m_FreeBlocks.m_Link, _pFreeBlock->m_FreeLink.fp_GetPrevNotList());

					if (t_bRemove)
						pFreeBlockBucket->f_Destruct(this);
		
					_pFreeBlock->m_FreeLink.f_Unlink();
	
					if (t_bRemove)
					{
						DMibFastCheck(pFreeBlockBucket->m_FreeBlocks.f_IsEmpty()); // "The list must be empty now, or an error has occurred"

						// Return it to the pool of buckets
						fp_DeleteFreeBlockBucket(pFreeBlockBucket);
					}
					else
						return pFreeBlockBucket;
				}
				else
				{
					_pFreeBlock->m_FreeLink.f_Unlink();
				}

			}
			return nullptr;
		}


	}
}
