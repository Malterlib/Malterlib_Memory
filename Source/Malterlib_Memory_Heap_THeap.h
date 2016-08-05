// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>
#include "Malterlib_Memory_Heap_THeap_Misc.h"

namespace NMib
{
	namespace NMem
	{

		template <class t_CHeapParams = TCHeapParams<> >
		class TCHeap
		{
		public:

			// Must make sure that we align to the platforms alignment 
			enum
			{
				 EAlignBitsIn = t_CHeapParams::EAlignBits
				,EAlignBits = EAlignBitsIn < 2 ? 2 : EAlignBitsIn
				,EAlign = 1 << constenum(EAlignBits)
				,EAlignAnd = (~((1 << EAlignBits) - 1))
				,EAlignAdd = (1 << EAlignBits) - 1
				,ENeedExtendedBlocks = t_CHeapParams::CBlock::ENeedExtendedBlocks
				,EFullSize = EAlignBits == 0 // Calculate if we have any alignment
				,ELargePages = t_CHeapParams::ELargePages
			};

			static const mint EFreeSizeBucketTreeThresholdBitsWanted = t_CHeapParams::EFreeSizeBucketTreeThresholdBits;
			static const mint EFreeSizeBucketTreeThresholdBits = (t_CHeapParams::CBlock::EUseableBits-1 + EAlignBits) < EFreeSizeBucketTreeThresholdBitsWanted ? (t_CHeapParams::CBlock::EUseableBits-1 + EAlignBits) : EFreeSizeBucketTreeThresholdBitsWanted;
			static const mint EFreeSizeBucketTreeThresholdListSize = 1 << (int(EFreeSizeBucketTreeThresholdBits - EAlignBits) < 0 ? 0 : int(EFreeSizeBucketTreeThresholdBits - EAlignBits));

			typedef typename t_CHeapParams::CAllocator CAllocator;
			typedef typename CAllocator::CPtrHolder CPtrHolder;
			typedef typename t_CHeapParams::CFillDebug CFillDebug;
			
			static inline_small bint f_CanCommit()
			{
				return CAllocator::f_CanCommit();
			}

			enum EBlockType
			{
				 EBlockType_Normal			= 0
				,EBlockType_Free			= 1
				,EBlockType_Edge			= 2
				,EBlockType_FreeExtended	= 3
			};

			// 4 Bytes
			class CBlock : public t_CHeapParams::CBlock
			{
			public:

				inline_small CBlock *f_GetNextNormal() const
				{
					return (CBlock *)(((uint8 *)this) + (t_CHeapParams::CBlock::f_Next() << EAlignBits));
				}

				inline_small CBlock *f_GetNextExtended() const
				{
					return ((CBlock_FreeExtendedBase *)this)->m_pNext;
				}

				inline_medium CBlock *f_GetNext() const
				{
					if (ENeedExtendedBlocks)
					{
						if (t_CHeapParams::CBlock::f_Type() == EBlockType_FreeExtended)
							return f_GetNextExtended();
						else
							return f_GetNextNormal();
					}
					else
						return f_GetNextNormal();
				}

				inline_small CBlock *f_GetPrevNormal() const
				{
					return (CBlock *)(((uint8 *)this) - (t_CHeapParams::CBlock::f_Prev() << EAlignBits));
				}

				inline_small CBlock *f_GetPrevExtended() const
				{
					return *((TCDynamicPtr<CPtrHolder, CBlock> *)(((uint8 *)this) - EBlockFreeExtendedSizeEnd));
				}

				inline_medium CBlock *f_GetPrev() const
				{
					if (t_CHeapParams::CBlock::f_Prev())
						return f_GetPrevNormal();
					else
						return f_GetPrevExtended();
				}

				inline_small mint f_GetSizeNormal() const
				{
					return t_CHeapParams::CBlock::f_Next() << EAlignBits;
				}

				inline_small mint f_GetSizeExtended() const
				{
					return (mint)(((uint8 *)(CBlock *)((CBlock_FreeExtendedBase *)this)->m_pNext) - (uint8 *)this);
				}

				inline_medium mint f_GetSize() const
				{
					if (ENeedExtendedBlocks)
					{
						if (t_CHeapParams::CBlock::f_Type() == EBlockType_FreeExtended)
							return f_GetSizeExtended();
						else
							return f_GetSizeNormal();
					}
					else
					{
						return f_GetSizeNormal();
					}
				}				

				inline_small bint f_IsFree() const
				{
					return (t_CHeapParams::CBlock::f_Type() & 1);
				}
			};

			// 8 bytes
			class CBlock_Cached : public CBlock // 4 Bytes
			{
			public:
				DMibListLinkAllocatorSA_Link(CBlock_Cached, m_CacheLink, CAllocator);	// 4 Bytes
			};
			// 12 bytes
			class CBlock_Free : public CBlock // 4 Bytes
			{
			public:
				DMibListLinkAllocatorDSA_Link(CBlock_Free, m_FreeLink, CAllocator);	// 8 Bytes

				inline_small void f_Construct()
				{
					m_FreeLink.f_Construct();
				}
			};

			class CCompareALV_CFreeBlockBucket;

			class CFreeBlockBucket;

			// 12-16 bytes
			class CFreeBlockBucket : public t_CHeapParams::CSizeHolder
			{
			public:
				DMibIntrusiveLinkT(CFreeBlockBucket, NIntrusive::TCAVLLinkAggregate<NIntrusive::EAVLLinkType_Unaligned>, m_FreeBlockBucketsLink);
				DMibListLinkAllocatorDSA_List_FromTemplate(CBlock_Free, m_FreeLink, CAllocator) m_FreeBlocks;						// 4 Byte

				inline_small mint f_GetSize() const
				{
					if (t_CHeapParams::CSizeHolder::EStoresSize)
					{
						return t_CHeapParams::CSizeHolder::m_Size;
					}
					else
					{
						DMibFastCheck(m_FreeBlocks.f_GetFirst()); // "There has to be a free block to get the size"

						mint Size = m_FreeBlocks.f_GetFirst()->f_GetSize();
						if (Size > (mint)ELargestBlock)
							Size = (mint)ELargestBlock;

						return Size;
					}
				}

				bint f_BucketUsed(TCHeap *_pHeap, mint _Size)
				{
					if (EFreeSizeBucketTreeThresholdBits > 0)
					{
						if (EFreeSizeBucketTreeThresholdBits > 0 && _Size <= 1 << EFreeSizeBucketTreeThresholdBits)
							return (_pHeap->m_pFreeBuckets[(_Size >> EAlignBits) - 1] == this);
						return m_FreeBlockBucketsLink.f_IsInTree();
					}
					else
						return m_FreeBlockBucketsLink.f_IsInTree();
					
				}

				inline_small void f_Construct(CBlock_Free *_pFreeBlock, mint _Size, TCHeap *_pHeap)
				{
					if (t_CHeapParams::CSizeHolder::EStoresSize)
					{
						t_CHeapParams::CSizeHolder::m_Size = _Size;
					}
					m_FreeBlockBucketsLink.f_Construct();
					m_FreeBlocks.f_Construct();
					m_FreeBlocks.f_UnsafeInsert(_pFreeBlock);
					if (EFreeSizeBucketTreeThresholdBits > 0 && _Size <= 1 << EFreeSizeBucketTreeThresholdBits)
					{
						DMibFastCheck(_pHeap->m_pFreeBuckets[(_Size >> EAlignBits) - 1] == nullptr); // "No other block bucket must be stored here"
						_pHeap->m_pFreeBuckets[(_Size >> EAlignBits) - 1] = this;
					}
					else
						_pHeap->m_FreeBlockBucketsTree.f_Insert(this);
				}

				inline_small void f_SetSize(mint _Size)
				{
					if (t_CHeapParams::CSizeHolder::EStoresSize)
						t_CHeapParams::CSizeHolder::m_Size = _Size;
				}

				inline_small void f_AddBlockBucket(TCHeap *_pHeap, mint _Size)
				{
					mint Size = _Size;
					if (t_CHeapParams::CSizeHolder::EStoresSize)
						t_CHeapParams::CSizeHolder::m_Size = Size;
					if (EFreeSizeBucketTreeThresholdBits > 0 && Size <= 1 << EFreeSizeBucketTreeThresholdBits)
					{
						DMibFastCheck(_pHeap->m_pFreeBuckets[(Size >> EAlignBits) - 1] == nullptr); // "No other block bucket must be stored here"
						_pHeap->m_pFreeBuckets[(Size >> EAlignBits) - 1] = this;
					}
					else
						_pHeap->m_FreeBlockBucketsTree.f_Insert(this);
				}

				inline_small void f_Destruct(TCHeap *_pHeap)
				{
					mint Size = f_GetSize();
					if (EFreeSizeBucketTreeThresholdBits > 0 && Size <= 1 << EFreeSizeBucketTreeThresholdBits)
					{
						DMibFastCheck(_pHeap->m_pFreeBuckets[(Size >> EAlignBits) - 1] == this); // "Must be current that is stored here"
						_pHeap->m_pFreeBuckets[(Size >> EAlignBits) - 1] = nullptr;
					}
					else
						_pHeap->m_FreeBlockBucketsTree.f_Remove(this);
				}

				inline_small void f_Destruct(TCHeap *_pHeap, mint _Size)
				{
					mint Size = _Size;
					if (EFreeSizeBucketTreeThresholdBits > 0 && Size <= 1 << EFreeSizeBucketTreeThresholdBits)
					{
						DMibFastCheck(_pHeap->m_pFreeBuckets[(Size >> EAlignBits) - 1] == this); // "Must be current that is stored here"
						_pHeap->m_pFreeBuckets[(Size >> EAlignBits) - 1] = nullptr;
					}
					else
						_pHeap->m_FreeBlockBucketsTree.f_Remove(this);
				}

				inline_small void f_Transfer(CFreeBlockBucket *_pFrom, TCHeap *_pHeap)
				{
					m_FreeBlocks.f_UnsafeTransfer(_pFrom->m_FreeBlocks);
				}
			};


			class CCompareALV_CFreeBlockBucket
			{
			public:
				inline_small mint operator () (CFreeBlockBucket const &_Node) const
				{
					return _Node.f_GetSize();
				}
			};

			// 20 Bytes
			class CBlock_FreeExtendedBase : public CBlock_Free // 12 Bytes
			{
			public:
				TCDynamicPtr<CPtrHolder, CBlock> m_pNext; // 4 Bytes usually

				inline_small void f_Construct()
				{
					CBlock_Free::f_Construct();
				}
			};

			// 20 Bytes
			class CBlock_FreeExtended : public CBlock_FreeExtendedBase // 12 Bytes
			{
			public:
				// This is just a placeholder. The data is saved at the end of the block not here
				TCDynamicPtr<CPtrHolder, CBlock> m_pThisTag; // 4 Bytes usually

				inline_small void f_Construct()
				{
					CBlock_Free::f_Construct();
				}
			};

			// 36 bytes
			class CBlock_FreeExtendedBucket : public CBlock_FreeExtendedBase // 20 Bytes
			{
			public:
				CFreeBlockBucket m_FreeBlockBucket;		// 12 bytes

				// This is just a placeholder. The data is saved at the end of the block not here
				TCDynamicPtr<CPtrHolder, CBlock> m_pThisTag;			// 4 Bytes

				inline_small void f_Construct()
				{
					CBlock_Free::f_Construct();
					m_FreeBlockBucket.m_FreeBlockBucketsLink.f_Construct();
//					m_FreeBlockBucket.m_FreeBlocks.f_Construct();
				}
			};

			class CSubChunk
			{
			public:
				inline_small CSubChunk()
				{
				}
//				DMibListLinkAllocatorD_Link(CSubChunk, m_Link, CAllocator);

				class CCompare
				{
				public:
					inline_small TCDynamicPtr<CPtrHolder, void> const &operator () (CSubChunk const &_Node) const
					{
						return _Node.m_pStart;
					}
				};
				typedef TCDynamicPtr<CPtrHolder, void> CDynamicPtr;

				DMibIntrusiveLinkT(CSubChunk, NIntrusive::TCAVLLink<NIntrusive::EAVLLinkType_Unaligned>, m_AVLLink);
				
				TCDynamicPtr<CPtrHolder, void> m_pStart;
				mint m_Size;
			};


			// 16 Bytes
			class CChunk : public TCHeapChunk<CAllocator>
			{
			
			public:
				
				TCDynamicPtr<CPtrHolder, TCHeap> m_pHeap;

				typedef NIntrusive::TCAVLTree<typename CSubChunk::CLinkTraits_m_AVLLink, typename CSubChunk::CCompare, CAllocator> CSubChunkTree;
				typedef typename CSubChunkTree::CIterator CSubChunkIter;

				CSubChunkTree m_SubChunks; // 4 Bytes

				inline_small void *f_GetBasePointer()
				{
					return TCHeapChunk<CAllocator>::m_pBase;
				}
				inline_small CBlock *f_GetFirstBlock()
				{
					return (CBlock *)((uint8 *)f_GetBasePointer() + EBlockFirstAdd - EBlockNonAllocSize);
				}
				inline_small void *f_GetEndPointer()
				{
					return (uint8 *)TCHeapChunk<CAllocator>::m_pBase + TCHeapChunk<CAllocator>::m_Size;
				}
				inline_small CBlock *f_GetLastBlock()
				{
					return (CBlock *)((uint8 *)f_GetEndPointer() - EBlockNonAllocSize);
				}

				void fp_Destroy(typename TCHeapChunk<CAllocator>::CTree &_Tree);
				void f_Trim();
				void f_TrimEnd();
				void f_TrimStart();
				void f_Commit(void *_pMem, mint _Size);
				void f_Decommit(void *_pMem, mint _Size);

				virtual void f_Destroy(typename TCHeapChunk<CAllocator>::CTree &_Tree)
				{
					fp_Destroy(_Tree);
				}

				virtual mint f_Size(const void *_pBlock)
				{
					DMibFastCheck(((mint)_pBlock > (mint)f_GetBasePointer()) && ((mint)_pBlock < (mint)f_GetEndPointer())); // "The block is outside of the chunk"
					return m_pHeap->f_Size(_pBlock);
				}

				virtual mint f_SizeAndHeap(const void *_pBlock, void * &_pHeap)
				{
					DMibFastCheck(((mint)_pBlock > (mint)f_GetBasePointer()) && ((mint)_pBlock < (mint)f_GetEndPointer())); // "The block is outside of the chunk"
					_pHeap = m_pHeap;
					return m_pHeap->f_Size(_pBlock);;
				}

				virtual fp32 f_Overhead(void const *_pBlock)
				{
					DMibFastCheck(((mint)_pBlock > (mint)f_GetBasePointer()) && ((mint)_pBlock < (mint)f_GetEndPointer())); // "The block is outside of the chunk"
					return m_pHeap->f_Overhead(_pBlock);
				}

				virtual void f_Free(void *_pBlock)
				{
					DMibFastCheck(((mint)_pBlock > (mint)f_GetBasePointer()) && ((mint)_pBlock < (mint)f_GetEndPointer())); // "The block is outside of the chunk"

					m_pHeap->fp_Free(this, _pBlock);
				}

				virtual void* f_GetExtraData(void *_pBlock)
				{
					return m_pHeap->f_GetExtraData(_pBlock);
				}

				virtual void* f_Realloc(void *_pBlock, mint &_NewSize)
				{
					DMibFastCheck(((mint)_pBlock > (mint)f_GetBasePointer()) && ((mint)_pBlock < (mint)f_GetEndPointer())); // "The block is outside of the chunk"
					return m_pHeap->fp_Realloc(this, _pBlock, _NewSize);
				}

				virtual void* f_Resize(void *_pBlock, mint &_NewSize)
				{
					DMibFastCheck(((mint)_pBlock > (mint)f_GetBasePointer()) && ((mint)_pBlock < (mint)f_GetEndPointer())); // "The block is outside of the chunk"
					return m_pHeap->fp_Resize(this, _pBlock, _NewSize);
				}
				
				virtual bint f_ContainsBlock(void *_pBlock)
				{
					return ((mint)_pBlock > (mint)f_GetBasePointer()) && ((mint)_pBlock < (mint)f_GetEndPointer());
				}

				virtual void* f_GetHeapIdent() const
				{
					return m_pHeap;
				}
			};


			enum
			{
				 EUseableBits = t_CHeapParams::CBlock::EUseableBits
				,ELargestBlockBits = (EUseableBits + EAlignBits) >= sizeof(mint)*8 ? 0 : EUseableBits + EAlignBits
				,ESmallestBlock = sizeof(CBlock_Free)
				,EBlockSize = sizeof(CBlock)
				,EBlockExtraSize = (t_CHeapParams::EExtraSpace + EAlign - 1) & (~constenum(EAlign - 1))
				,EBlockNonGuardSize = EBlockSize + EBlockExtraSize
				,EBlockPreGuardSize = (t_CHeapParams::EGuardPreSize + EAlign - 1) & (~constenum(EAlign - 1))
				,EBlockExtraOffset = EBlockExtraSize + EBlockPreGuardSize
				,EBlockPreGuardOffset = EBlockPreGuardSize
				,EBlockPreSize = EBlockSize + EBlockExtraSize + EBlockPreGuardSize
				,EBlockPostSize = t_CHeapParams::EGuardPostSize
				,EBlockPostGuardSize = EBlockPostSize
				,EBlockWholeSize = EBlockPreSize + EBlockPostSize
				,EBlockNonAllocSize = EBlockSize
				,EBlockFreeSize = sizeof(CBlock_Free)
				,EBlockFreeExtendedSize = t_CHeapParams::EbOptimizeForSize ? sizeof(CBlock_FreeExtendedBucket) : sizeof(CBlock_FreeExtended)
				,EBlockFreeExtendedSizeEnd = sizeof(TCDynamicPtr<CPtrHolder, CBlock>)
				,EBlockFreeExtendedSizeStart = EBlockFreeExtendedSize - EBlockFreeExtendedSizeEnd
				,EBlockFirstAdd = constenum(EBlockNonAllocSize) > constenum(EAlign) ? constenum(EBlockNonAllocSize) : constenum(EAlign)
				,EBlockMinNext = constenum(EAlign) > constenum(EBlockSize) ? constenum(EAlign) : constenum(EBlockSize)
				,EBlockCacheSize = t_CHeapParams::EBlockCacheSize
				,EMaxCachedBlockSize = ((EBlockCacheSize + EAlignAdd) & EAlignAnd)
				,ECachedBlocks = EMaxCachedBlockSize ? (constenum(EMaxCachedBlockSize - ESmallestBlock) >> constenum(EAlignBits)) + 1 : 1
				,ETLSFStartBits = 7
				,ETLSFPrimarySize = sizeof(void *) * 8 - ETLSFStartBits
				,ETLSFSecondarySize = 8
			};
			
			static constexpr EAllocationFlag mc_AllocationFlags = t_CHeapParams::mc_AllocationFlags;

			static constexpr mint ELargestBlock = ((EUseableBits + EAlignBits) >= sizeof(mint)*8 ? (DMibBitRangeTyped(1, (sizeof(mint)*8-1), mint)) : ((mint)1 << ELargestBlockBits));

            static inline_small mint f_SmallestBlock()
			{
				return ESmallestBlock;
			}

			static inline_small mint f_LargestBlock()
			{				
				return fg_AlignDown((mint)(ELargestBlock) - mint(1 << EAlignBits) - mint((EBlockWholeSize > ESmallestBlock ? EBlockWholeSize : ESmallestBlock)), 1 << EAlignBits) - mint(EBlockSize);
//				return (mint)(ELargestBlock - (1 << EAlignBits)) - (mint)(EBlockWholeSize > ESmallestBlock ? EBlockWholeSize : ESmallestBlock);
			}

			static inline_small mint f_MaxGranularity()
			{
				mint Max = ESmallestBlock;
				Max = fg_Max(Max, mint(1) << EAlignBits);
				return Max;
			}

			/***************************************************************************************************\
			|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
			| Utils																								|
			|___________________________________________________________________________________________________|
			\***************************************************************************************************/

			static inline_small uint8 * fp_AlignDown(uint8 *_pAlign)
			{
				return NMib::fg_AlignDown(_pAlign, CAllocator::f_GranularityCommit(ELargePages));
			}

			static inline_small uint8 * fp_AlignUp(uint8 *_pAlign)
			{
				return NMib::fg_AlignUp(_pAlign, CAllocator::f_GranularityCommit(ELargePages));
			}

			static inline_small mint fp_AlignDown(mint _Align)
			{
				return NMib::fg_AlignDown(_Align, CAllocator::f_GranularityCommit(ELargePages));
			}

			static inline_small mint fp_AlignUp(mint _Align)
			{
				return NMib::fg_AlignUp(_Align, CAllocator::f_GranularityCommit(ELargePages));
			}

			/***************************************************************************************************\
			|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
			| Commit																							|
			|___________________________________________________________________________________________________|
			\***************************************************************************************************/

			static inline_small mint fsp_CommitSize_FreeStart(mint _BlockSize);
			static inline_small mint fsp_CommitSize_FreeExtended();
			static inline_small mint fsp_CommitSize_FreeExtendedStart();
			static inline_small mint fsp_CommitSize_FreeExtendedEnd();
			static inline_small void fsp_CommitRange(CChunk *_pChunk, uint8 *_pStart, uint8 *_pEnd);
			static inline_small void fsp_CommitRangeAlreadyCommitted(CChunk *_pChunk, uint8 *_pStart, uint8 *_pEnd, uint8 *_pAlreadyCommited);
			static inline_small void fsp_DecommitRangeAligned(CChunk *_pChunk, uint8 *_pStart, uint8 *_pEnd);
			static inline_small void fsp_CommitDualRange(CChunk *_pChunk, uint8 *_pStart, uint8 *_pEnd, uint8 *_pStart2, uint8 *_pEnd2);
			static inline_small void fsp_CommitDualRangeAlreadyCommitted(CChunk *_pChunk, uint8 *_pStart, uint8 *_pEnd, uint8 *_pStart2, uint8 *_pEnd2, uint8 *_pAlreadyCommited);
			static inline_small void fsp_DecommitDualRangeAligned(CChunk *_pChunk, uint8 *_pStart, uint8 *_pEnd, uint8 *_pStart2, uint8 *_pEnd2);
			static inline_medium void fsp_CommitBlockNextExtended(CChunk *_pChunk, CBlock* _pBlock, CBlock* _pNextBlockOld, CBlock* _pNextBlock);
			static inline_medium void fsp_CommitBlockNextFree(CChunk *_pChunk, CBlock* _pBlock, CBlock* _pNextBlockOld, CBlock* _pNextBlock);
			static inline_medium void fsp_CommitBlockNormal(CChunk *_pChunk, CBlock* _pBlock, CBlock *_pNextBlock);
			static inline_medium void fsp_CommitBlockExtended(CChunk *_pChunk, CBlock* _pBlock, CBlock *_pNextBlock);

			/***************************************************************************************************\
			|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
			| Construct																							|
			|___________________________________________________________________________________________________|
			\***************************************************************************************************/


			inline_small void f_Construct(typename TCHeapChunk<CAllocator>::CTree *_pChunksTree)
			{
				m_pChunksTree = _pChunksTree;
				m_bHaveFastChunks = false;
				m_pFreeChunk = nullptr;
				fg_MemClear(m_pFreeBuckets);
//				m_PoolFreeBlockBuckets.m_Pool.AddChunk(m_DefaultPoolChunkData);
			}

			inline_medium mint fp_BlockSize(CBlock *_pBlock)
			{
				if (_pBlock->f_Type())
				{
					DMibFastCheck((mint)((CBlock_FreeExtendedBase *)_pBlock)->m_pNext > (mint)this); // "Heap error (probably memory overwrite)
					return (mint)((CBlock_FreeExtendedBase *)_pBlock)->m_pNext - (mint)this;
				}
				else
				{
					return (_pBlock->m_Next << EAlignBits);
				}
			}
			
			enum
			{
				EbOptimizeForSize = t_CHeapParams::EbOptimizeForSize
			};
			//TCPool<CFreeBlockBucket> m_PoolFreeBlockBuckets;

			inline_small CFreeBlockBucket *fp_NewFreeBlockBucket(mint _Size)
			{
				return m_PoolFreeBlockBuckets.f_New();
			}
			inline_small void fp_DeleteFreeBlockBucket(CFreeBlockBucket *_pFreeBlockBucket)
			{
				m_PoolFreeBlockBuckets.f_Delete(_pFreeBlockBucket);
			}


			void fp_AddChunk(void *_pChunk, mint _ChunkSize);
			void fp_FillFreeBlock(CBlock_Free *_pFreeBlock);
			void fp_CheckFreeBlock(CBlock_Free *_pFreeBlock, mint _Size);
			void fp_CheckAllocatedBlock(CBlock *_pBlock);
			void fp_InitFreeBlock(CBlock_Free *pFreeBlock, CBlock *pNextBlock, CBlock *pPrevBlock);

			template <bint t_bRemove>
			void fp_UntieFreeBlock(CBlock_Free* _pFreeBlock);
			template <bint t_bRemove>
			CFreeBlockBucket *fp_UntieFreeBlockRet(CBlock_Free* _pFreeBlock);

			/***************************************************************************************************\
			|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
			| Block bucket																						|
			|___________________________________________________________________________________________________|
			\***************************************************************************************************/

			inline_small CFreeBlockBucket *fp_GetBlockBucket(mint _Size);
			inline_small CFreeBlockBucket *fp_GetBlockBucket(mint _Size, mint &_PrevSize);
			inline_small CFreeBlockBucket *fp_GetBlockBucket(mint _Size, CFreeBlockBucket *&_pPrev);
			inline_small CFreeBlockBucket *fp_GetBlockBucketExact(mint _Size);

            /***************************************************************************************************\
            |¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
            | Alloc																								|
            |___________________________________________________________________________________________________|
            \***************************************************************************************************/

			void *f_Alloc(mint &_Size);
			void *fp_AllocInternal(mint &_Size);
			
			inline_always CFreeBlockBucket *fp_GetFreeBlockBucketInline(mint _Size, mint &_PrevSize);
			CFreeBlockBucket *fp_GetFreeBlockBucket(mint _Size, mint &_PrevSize);
			
			void *fp_AllocInternalFromBlock(mint &_Size, CFreeBlockBucket *_pFreeBlockBucket, mint _PrevSize, mint _Alignment);
			template <bool tf_bAlign>
			inline_always void *fp_AllocInternalFromBlockInline(mint &_Size, CFreeBlockBucket *_pFreeBlockBucket, mint _PrevSize, mint _Alignment);

            /***************************************************************************************************\
            |¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
            | Free																								|
            |___________________________________________________________________________________________________|
            \***************************************************************************************************/

			void fp_FreeBlockCache();
			void f_Clear()
			{
				fp_FreeBlockCache();
			}

			inline_small void fp_Free(CChunk *_pChunk, void *_pMem);
			inline_small void f_Free(void *_pMem);
			void fp_FreeInternal(CChunk *_pChunk, CBlock *_pBlock);


			/***************************************************************************************************\
			|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
			| Alloc Aligned																						|
			|___________________________________________________________________________________________________|
			\***************************************************************************************************/

			inline_small void *f_AllocAligned(mint &_Size, mint _Align);

			/***************************************************************************************************\
			|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
			| Misc																								|
			|___________________________________________________________________________________________________|
			\***************************************************************************************************/

			inline_medium void *f_Realloc(void *_pMem, mint &_NewSize);
			inline_medium void *fp_Realloc(CChunk *_pChunk, void *_pMem, mint &_NewSize);
			inline_medium void *f_Resize(void *_pMem, mint &_NewSize);
			inline_medium void *fp_Resize(CChunk *_pChunk, void *_pMem, mint &_NewSize);
			inline_medium mint f_Size(const void *_pMem);
			inline_medium fp32 f_Overhead(void const *_pMem);
			void *f_GetExtraData(void *_pMem);
			void fp_FillBlock(CBlock *_pBlock);

			/************************************************************************************************\
			||¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯||
			|| Debug functions
			||______________________________________________________________________________________________||
			\************************************************************************************************/

			bint f_CheckHeap(bint _bBreak, bint _bTrace);

			void fp_TraceChunks();

			class CEnumContext
			{
			public:
				TCHeapChunk<CAllocator> *m_pNextChunk;
				
				CBlock *m_pBlock;

				CEnumContext()
				{
					m_pBlock = nullptr;
				}

			};

			void *f_EnumAllocatedBlocksStart();
			void f_EnumAllocatedBlocksFinish(void *_pContext);
			void *fp_EnumNextBlock(CEnumContext *_pContext);
			void *f_EnumAllocatedBlocksNext(void * _pContext);
			/*
			class CTreeContainer
			{
			public:
				DMibTreeAVLAllocator_Tree_FromTemplate(CFreeBlockBucket, m_FreeBlockBucketsLink, CCompareALV_CFreeBlockBucket, CAllocator) m_FreeBlockBucketsTree;
			};

			CTreeContainer m_FreeTreeContainers[1][1]; // One container per size
			*/

			bint m_bHaveFastChunks;
			DMibListLinkS_ListNoLastPtr_FromTemplate(CBlock_Cached, m_CacheLink) m_aCachedBlocks[ECachedBlocks];
			CFreeBlockBucket *m_pFreeBuckets[EFreeSizeBucketTreeThresholdListSize];

			typedef NIntrusive::TCAVLTree<typename CFreeBlockBucket::CLinkTraits_m_FreeBlockBucketsLink, CCompareALV_CFreeBlockBucket, CAllocator> CFreeBlockBucketsTree;
			typedef typename CFreeBlockBucketsTree::CIterator CFreeBlockBucketIter;
			CFreeBlockBucketsTree m_FreeBlockBucketsTree;

			TCDynamicPtr<CPtrHolder, typename TCHeapChunk<CAllocator>::CTree> m_pChunksTree;

			TCPool<CFreeBlockBucket, (EbOptimizeForSize) * 2 + (1 - EbOptimizeForSize) * 128, NThread::CNoLock, CPoolType_Growing, CAllocator> m_PoolFreeBlockBuckets;
			TCPool<CSubChunk, 128, NThread::CNoLock, CPoolType_Freeable, CAllocator> m_PoolSubChunks;
			TCPool<CChunk, 128, NThread::CNoLock, CPoolType_Freeable, CAllocator> m_PoolChunks;
			TCDynamicPtr<CPtrHolder, CChunk> m_pFreeChunk;

		};

		// Static member in template
	/*	template <aint t_GrowSize, typename t_CAllocator, mint t_AlignBits, aint t_ExtraSpace, aint t_GuardPreSize, aint t_GuardPostSize, typename t_CFillDebug, bint t_bOptimizeForSize, typename t_CSizeHolder> 
			NMib::NAggregate::TCAggregate< TCHeap<t_GrowSize, t_CAllocator, t_AlignBits, t_ExtraSpace, t_GuardPreSize, t_GuardPostSize, t_CFillDebug, t_bOptimizeForSize, t_CSizeHolder>::CFreeBlockBucketPool> TCHeap<t_GrowSize, t_CAllocator, t_AlignBits, t_ExtraSpace, t_GuardPreSize, t_GuardPostSize, t_CFillDebug, t_bOptimizeForSize, t_CSizeHolder>::ms_FreeBlockBucketPool = {false};
		*/

	}
}


