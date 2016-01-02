// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

namespace NMib
{
	namespace NMem
	{

		/***************************************************************************************************\
		|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯|
		| Debug																								|
		|___________________________________________________________________________________________________|
		\***************************************************************************************************/


		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::fp_TraceChunks()
		{
			typename TCHeapChunk<CAllocator>::CIterator Iter = m_pChunksTree;

			DMibTrace(DMibNewLine DMibNewLine DMibNewLine "TraceChunks" DMibNewLine, 0);

			while (Iter)
			{
				CChunk *pChunk = (CChunk *)(TCHeapChunk<CAllocator> *)Iter;
				if (pChunk->m_pHeap == this)
				{
					DMibTrace("Base {} End {}" DMibNewLine, (mint)pChunk->f_GetBasePointer() << (mint)pChunk->f_GetEndPointer());
					DMibListLinkD_Iter_FromTemplate(CSubChunk, m_Link) Iter2 = pChunk->m_lSubChunks;
					while (Iter2)
					{
						DMibTrace("    Sub {} {}" DMibNewLine, (mint)Iter2->m_pStart << (mint)Iter2->m_Size);
						++Iter2;
					}
				}

				++Iter;
			}
		}
		template <class t_CHeapParams>
		bint TCHeap<t_CHeapParams>::f_CheckHeap(bint _bBreak, bint _bTrace)
		{
			static bint bOnlyCheckFree = false;

			if (!bOnlyCheckFree)
			{
				// First check if the trees are alright
				if (!m_FreeBlockBucketsTree.f_CheckTree(_bBreak))
				{
					if (_bTrace)
						DMibTrace("m_FreeBlockBucketsTree.f_CheckTree failed" DMibNewLine, 0);
					return false;
				}
				if (!m_pChunksTree->f_CheckTree(_bBreak))
				{
					if (_bTrace)
						DMibTrace("m_pChunksTree->f_CheckTree failed" DMibNewLine, 0);
					return false;
				}
			}

			// Loop through all chunks and check that all next and prev ptrs are ok
			{
				typename TCHeapChunk<CAllocator>::CIterator Iter(m_pChunksTree);

				while (Iter)
				{
					TCHeapChunk<CAllocator> *pChunkBase = Iter.f_GetCurrent();
					if (pChunkBase->f_GetHeapIdent() == this)
					{
						CChunk *pChunk = (CChunk *)pChunkBase;
						if (!bOnlyCheckFree)
						{
							if (!pChunk->m_SubChunks.f_CheckTree(_bBreak))
							{
								if (_bTrace)
									DMibTrace("m_SubChunks.f_CheckTree failed" DMibNewLine, 0);
								return false;
							}

						}
						CBlock *pPrev = pChunk->f_GetFirstBlock();
						if (pPrev->f_Type() != EBlockType_Edge)
						{
							if (_bTrace)
								DMibTrace("Wrong block type" DMibNewLine, 0);
							if (_bBreak)
								DMibPDebugBreak; // Wrong block type
							return false;
						}

						if (pPrev->f_Prev())
						{
							if (_bBreak)
								DMibPDebugBreak; // No prev should exist
							return false;
						}

						CBlock *pCurrentBlock = pPrev->f_GetNext();

						while (pCurrentBlock)
						{
							CBlock *pPrevGotten = pCurrentBlock->f_GetPrev();
							if (pPrevGotten != pPrev)
							{
								if (_bTrace)
									DMibTrace("Broken prev link" DMibNewLine, 0);

								if (_bBreak)
									DMibPDebugBreak; // Broken prev link
								return false;
							}

							switch (pCurrentBlock->f_Type())
							{
							case EBlockType_Edge:
								pPrev = pCurrentBlock;
								pCurrentBlock = nullptr;
								break;
							case EBlockType_FreeExtended:
								{
									if (!ENeedExtendedBlocks)
									{
										if (_bTrace)
											DMibTrace("A extended block in a heap that don't need extended blocks" DMibNewLine, 0);
										if (_bBreak)
											DMibPDebugBreak; // A extended block in a heap that don't need extended blocks
										return false;

									}

									if (t_CHeapParams::EbOptimizeForSize && !bOnlyCheckFree)
									{
										CBlock_FreeExtendedBucket *pFreeBlockExt = ((CBlock_FreeExtendedBucket *)pCurrentBlock);
										if (!pFreeBlockExt->m_FreeBlockBucket.m_FreeBlocks.f_CheckList(_bBreak))
											return false;

										mint Size = pFreeBlockExt->f_GetSize();
										if (pFreeBlockExt->m_FreeBlockBucket.f_BucketUsed(this, Size))
										{
											mint Size = pFreeBlockExt->m_FreeBlockBucket.f_GetSize();
											if (fp_GetBlockBucketExact(Size) != &pFreeBlockExt->m_FreeBlockBucket)
											{
												if (_bTrace)
													DMibTrace("Must be in tree" DMibNewLine, 0);
												if (_bBreak)
													DMibPDebugBreak; // Must be in tree
												return false;
											}
										}
										else
										{
											mint Size = pFreeBlockExt->f_GetSize();
											if (Size > (mint)ELargestBlock)
												Size = (mint)ELargestBlock;

											if (fp_GetBlockBucketExact(Size) == &pFreeBlockExt->m_FreeBlockBucket)
											{
												if (_bTrace)
													DMibTrace("Must not be in tree" DMibNewLine, 0);
												if (_bBreak)
													DMibPDebugBreak; // Must not be in tree
												return false;

											}
										}
										if ((pFreeBlockExt->m_FreeBlockBucket.f_BucketUsed(this, Size) && pFreeBlockExt->m_FreeBlockBucket.m_FreeBlocks.f_IsEmpty())
											|| 
											(!pFreeBlockExt->m_FreeBlockBucket.m_FreeBlocks.f_IsEmpty() && !pFreeBlockExt->m_FreeBlockBucket.f_BucketUsed(this, Size)))
										{
											if (_bTrace)
												DMibTrace("Must be both in tree and have a list of objects" DMibNewLine, 0);
											if (_bBreak)
												DMibPDebugBreak; // Must be both in tree and have a list of objects

											return false;
										}
									}
								}
							case EBlockType_Free:
								{
									CBlock_Free *pFreeBlock = ((CBlock_Free *)pCurrentBlock);
									fp_CheckFreeBlock(pFreeBlock, pCurrentBlock->f_GetSize());
									if (!pFreeBlock->m_FreeLink.f_IsInList())
									{
										if (_bTrace)
											DMibTrace("Must be free" DMibNewLine, 0);
										if (_bBreak)
											DMibPDebugBreak; // Must be free

										return false;
									}
									if (!bOnlyCheckFree)
									{
										mint ToFind = pFreeBlock->f_GetSize();
										if (ToFind > (mint)ELargestBlock)
											ToFind = (mint)ELargestBlock;
										CFreeBlockBucket *pBucket = fp_GetBlockBucketExact(ToFind);
										if (!pBucket || !pBucket->m_FreeBlocks.f_Contains(pFreeBlock))
										{
											if (_bTrace)
											{
												DMibTrace("Must be in block tree for this size" DMibNewLine, 0);
											}

											if (_bBreak)
												DMibPDebugBreak; // Must be in block tree for this size
											return false;
										}
									}
									pPrev = pCurrentBlock;
									pCurrentBlock = pCurrentBlock->f_GetNext();
								}
								break;
							case EBlockType_Normal:
								{
									if (!bOnlyCheckFree)
										fp_CheckAllocatedBlock(pCurrentBlock);
									pPrev = pCurrentBlock;
									pCurrentBlock = pCurrentBlock->f_GetNextNormal();
								}
								break;
							default:
								if (_bTrace)
									DMibTrace("MegaError" DMibNewLine, 0);
								if (_bBreak)
									DMibPDebugBreak; // MegaError
								return false;
							}

						}
					}

					++Iter;
				}
			}

			// Loop through all sizeclasses and check that the sizes abide by the rules
			if (!bOnlyCheckFree)
			{
				CFreeBlockBucketIter Iter(m_FreeBlockBucketsTree);
				while (Iter)
				{
					if ((Iter->f_GetSize() & (~((mint)EAlignAnd))) != 0)
					{
						if (_bTrace)
							DMibTrace("Size must be aligned to alignment" DMibNewLine, 0);
						if (_bBreak)
							DMibPDebugBreak; // Size must be aligned to alignment

						return false;
					}

					++ Iter;
				}
			}


			return true;
		}

		template <class t_CHeapParams>
		void *TCHeap<t_CHeapParams>::f_EnumAllocatedBlocksStart()
		{
			mint Size = NMib::fg_AlignUp(sizeof(CEnumContext), CAllocator_Virtual::f_GranularityAlloc(ELargePages));
			CEnumContext *pContext = new(CAllocator_Virtual::f_Alloc(Size)) CEnumContext();
			typename TCHeapChunk<CAllocator>::CIterator Iter(*m_pChunksTree);

			pContext->m_pNextChunk = Iter;

			return pContext;
		}

		template <class t_CHeapParams>
		void TCHeap<t_CHeapParams>::f_EnumAllocatedBlocksFinish(void *_pContext)
		{
			((CEnumContext *)_pContext)->~CEnumContext();;

			CAllocator_Virtual::f_Free(_pContext);
		}

		template <class t_CHeapParams>
		void *TCHeap<t_CHeapParams>::fp_EnumNextBlock(CEnumContext *_pContext)
		{
			CEnumContext *pContext = _pContext;
			while (pContext->m_pBlock)
			{
				int Type = pContext->m_pBlock->f_Type();
				if (Type == EBlockType_Normal)
				{
					void *pBlock = ((uint8 *)pContext->m_pBlock + EBlockPreSize);
					pContext->m_pBlock = pContext->m_pBlock->f_GetNext();
					return pBlock;
				}
				else if (Type == EBlockType_Edge)
				{
					pContext->m_pBlock = nullptr;
				}
				else
				{
					pContext->m_pBlock = pContext->m_pBlock->f_GetNext();
				}
			}

			return nullptr;
		}

		template <class t_CHeapParams>
		void *TCHeap<t_CHeapParams>::f_EnumAllocatedBlocksNext(void * _pContext)
		{
			CEnumContext *pContext = (CEnumContext *)_pContext;

			void *pReturn = fp_EnumNextBlock(pContext);
			if (pReturn)
				return pReturn;

			typename TCHeapChunk<CAllocator>::CIterator Iter;

			if (!pContext->m_pNextChunk)
				return nullptr;

			Iter.f_InitForSearch(*m_pChunksTree);

			Iter.f_FindEqualForward(pContext->m_pNextChunk->m_pBase);


			while (Iter)
			{
				CChunk *pChunk = (CChunk *)(TCHeapChunk<CAllocator> *)Iter;
				if (pChunk->m_pHeap == this)
				{
					pContext->m_pBlock = pChunk->f_GetFirstBlock()->f_GetNext();

					void *pReturn = fp_EnumNextBlock(pContext);
					if (pReturn)
					{
						++Iter;
						pContext->m_pNextChunk = Iter;
						return pReturn;
					}
				}

				++Iter;
			}

			return nullptr;
		}
	}
}
