// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

namespace NMib
{
	namespace NMem
	{

		/***************************************************************************************************\
		|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
		| Alloc Aligned																						|
		|___________________________________________________________________________________________________|
		\***************************************************************************************************/

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::fp_FillBlock(CBlock *_pBlock)
		{
			// This block is allocated
			mint Size = _pBlock->f_GetSize();
			void *pMem = (void *)((mint)_pBlock + EBlockPreSize);
			CFillDebug::fs_FillGuard((uint8 *)pMem - EBlockPreGuardOffset, EBlockPreGuardSize);
			CFillDebug::fs_FillAllocated(pMem, Size - EBlockWholeSize);
			CFillDebug::fs_FillGuard((uint8 *)_pBlock + (Size - EBlockPostGuardSize), EBlockPostGuardSize);
		}
		
		template <class t_CHeapParams>
		inline_small void *TCHeap<t_CHeapParams>::f_AllocAligned(mint &_Size, mint _Align)
		{
			if (_Align <= EAlign)
				return f_Alloc(_Size);
			
			mint Size = _Size;
			Size = fg_AlignUp(fg_AlignUp(Size, _Align) + EBlockWholeSize + ESmallestBlock, EAlign);
			if (Size < ESmallestBlock)
				Size = ESmallestBlock;
			
			mint PrevSize;
			mint BlockBucketSize = Size + fg_Max(_Align * 2, mint(ESmallestBlock * 2));
			CFreeBlockBucket *pFreeBlockBucket = fp_GetFreeBlockBucketInline(BlockBucketSize, PrevSize);
			if (!pFreeBlockBucket)
				return nullptr;
			
			void *pMem = fp_AllocInternalFromBlockInline<true>(Size, pFreeBlockBucket, PrevSize, _Align);
			_Size = Size - EBlockWholeSize;
			return pMem;
			
			#if 0

			// First try to allocate the block at the right size + align and move the top of the block forward... hhmmm or do we want a free block before ourselfs all the time..
			mint AlignedSize = fg_AlignUp(_Size, _Align);
			mint FullSize = AlignedSize + _Align;
			void *pMem = f_Alloc(FullSize);
			if ((mint)pMem & (_Align - 1))
			{
				CBlock *pBlock = (CBlock *)(((uint8 *)pMem) - EBlockPreSize);
//				CBlock *pPrev = pBlock->f_GetPrev();
				CBlock *pNext = pBlock->f_GetNext();
				CBlock *pAlignedBlock = (CBlock *)(fg_AlignUp((mint)pBlock, _Align) - EBlockPreSize);
				CBlock *pPostBlock = (CBlock *)((mint)pAlignedBlock + AlignedSize);
				mint PreSize = (mint)pAlignedBlock - (mint)pBlock;
				mint PostSize = (mint)pNext - (mint)pPostBlock;
				DMibFastCheck(PreSize >= ESmallestBlock);
				
				pBlock->f_SetNext(PreSize >> EAlignBits);
				pBlock->f_SetType(EBlockType_Normal);
				
				pAlignedBlock->f_SetType(EBlockType_Normal);
				pAlignedBlock->f_SetPrev(PreSize >> EAlignBits);
				pAlignedBlock->f_SetNext(AlignedSize >> EAlignBits);
				
				if (PostSize > 0)
				{
					pPostBlock->f_SetType(EBlockType_Normal);
					pPostBlock->f_SetNext(PostSize >> EAlignBits);
					pPostBlock->f_SetPrev(AlignedSize >> EAlignBits);
					pNext->f_SetPrev(PostSize >> EAlignBits);
					if (CFillDebug::EDoFills)
					{
						fp_FillBlock(pAlignedBlock);
						fp_FillBlock(pPostBlock);
					}
					f_Free((void *)((mint)pBlock + EBlockPreSize));
					f_Free((void *)((mint)pPostBlock + EBlockPreSize));
				}
				else
				{
					pNext->f_SetPrev(AlignedSize >> EAlignBits);
					if (CFillDebug::EDoFills)
						fp_FillBlock(pAlignedBlock);
					f_Free((void *)((mint)pBlock + EBlockPreSize));
				}
				
				return (void *)((mint)pAlignedBlock + EBlockPreSize);
			}
			else
			{
				CBlock *pBlock = (CBlock *)(((uint8 *)pMem) - EBlockPreSize);
				CBlock *pNext = pBlock->f_GetNext();
				CBlock *pAlignedBlock = (CBlock *)(fg_AlignUp((mint)pBlock, _Align) - EBlockPreSize);
				CBlock *pPostBlock = (CBlock *)((mint)pAlignedBlock + AlignedSize);
				mint PostSize = (mint)pNext - (mint)pPostBlock;
				DMibFastCheck(PostSize >= ESmallestBlock);
				
				pAlignedBlock->f_SetType(EBlockType_Normal);
				pAlignedBlock->f_SetNext(AlignedSize >> EAlignBits);
				
				pPostBlock->f_SetType(EBlockType_Normal);
				pPostBlock->f_SetNext(PostSize >> EAlignBits);
				pPostBlock->f_SetPrev(AlignedSize >> EAlignBits);
				pNext->f_SetPrev(PostSize >> EAlignBits);
				if (CFillDebug::EDoFills)
					fp_FillBlock(pPostBlock);
				f_Free((void *)((mint)pPostBlock + EBlockPreSize));
				
				
				return (void *)((mint)pAlignedBlock + EBlockPreSize);
			}
			#endif
		}
	}
}
