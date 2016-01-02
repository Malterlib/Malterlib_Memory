// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{

		template <typename t_CParams>
		fp32 TCMemoryManagerArena<t_CParams>::f_Overhead(void const *_pMemory, TCMemoryManagerSlabShared<t_CParams> const *_pSlab)
		{
			auto SubSlab = mint((uint8 *)_pMemory - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;

			mint SlabType = _pSlab->m_SlabType;
			auto pData = _pSlab->f_GetSubSlabData();

			mint iSubSlab;
			mint SlabBucket;

			if (SlabType > 0)
			{
				iSubSlab = t_CParams::fs_DivideBySlabMultiplier(SubSlab, SlabType);
				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				SlabBucket = pData[iSubSlab].m_Allocated.m_Type;
			}
			else
			{
				iSubSlab = SubSlab;
				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				SlabBucket = pData[iSubSlab].m_Allocated.m_Type;
				if (SlabBucket < mc_nSmallSizeSlabs)
					return fp_OverheadSmall(_pMemory, (TCMemoryManagerSlab<t_CParams, 0> const *)_pSlab, SlabBucket);
			}
			
			SlabBucket -= 2;

			mint AlignedSize;
			mint SlabBucketStart = mint(1) << SlabBucket;
			AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));
			
			fp32 OverheadPerByte = _pSlab->f_OverheadPerByte();
			
			return fp32(AlignedSize) * OverheadPerByte;
		}
		
		template <typename t_CParams>
		mint TCMemoryManagerArena<t_CParams>::f_Size(void const *_pMemory, TCMemoryManagerSlabShared<t_CParams> const *_pSlab)
		{
			auto SubSlab = mint((uint8 *)_pMemory - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;

			mint SlabType = _pSlab->m_SlabType;
			auto pData = _pSlab->f_GetSubSlabData();

			mint iSubSlab;
			mint SlabBucket;

			if (SlabType > 0)
			{
				iSubSlab = t_CParams::fs_DivideBySlabMultiplier(SubSlab, SlabType);
				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				SlabBucket = pData[iSubSlab].m_Allocated.m_Type;
			}
			else
			{
				iSubSlab = SubSlab;
				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				SlabBucket = pData[iSubSlab].m_Allocated.m_Type;
				if (SlabBucket < mc_nSmallSizeSlabs)
					return fp_SizeSmall(_pMemory, (TCMemoryManagerSlab<t_CParams, 0> const *)_pSlab, SlabBucket);
			}

			SlabBucket -= 2;

			mint AlignedSize;
			mint SlabBucketStart = mint(1) << SlabBucket;
			AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));
			
			return AlignedSize;
		}
		
		template <typename t_CParams>
		bool TCMemoryManagerArena<t_CParams>::fp_CheckFree(CMemoryManagerSubSlab_NormalLink *_pLink, bool _bBreak)
		{
			bool bError = false;
			uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pLink + 1, t_CParams::mc_SlabSize);
			CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

			TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);
			
			mint AlignedSize = f_Size(_pLink, pSlab);
			
			if (this->mc_EnableCallbacks)
			{
				uint8 *pStart = (uint8 *)(_pLink + 1);
				uint8 *pEnd = (uint8 *)_pLink + (AlignedSize * _pLink->m_nBlocks);
				
				if (this->f_OnCheckFree(pStart, pEnd - pStart, _bBreak))
					bError = true;
				
			}
			
			return bError;
		}
		
		template <typename t_CParams>
		inline_always void *TCMemoryManagerArena<t_CParams>::fp_AllocNormal(mint &_Size)
		{
			CNormalFreeList *pList;
			mint Size = fg_AlignUp(_Size, mc_MinNormalSizeAlignment);
			
			DMibFastCheck(Size >= 20);

			mint AlignedSize;
			mint SubIndex;
			mint SlabBucket;
			{
				{
					SlabBucket = NMib::fg_GetHighestBitSetNoZero(Size);
					mint SlabBucketT8 = SlabBucket - t_CParams::mc_SizesPerLevelShift;
					mint SlabBucketStart = mint(1) << SlabBucket;
					mint SlabBucketGranularity = (mint(1) << SlabBucketT8);
					AlignedSize = fg_AlignUp(Size, SlabBucketGranularity);
					SubIndex = (AlignedSize - SlabBucketStart) >> SlabBucketT8;
					SlabBucket += SubIndex >> t_CParams::mc_SizesPerLevelShift;
					SubIndex &= (mint(1) << t_CParams::mc_SizesPerLevelShift) - 1;

					if (likely(Size > mc_Level0SmallestSize))
					{
						mint SlabIndex = SlabBucket - 4;

						DMibFastCheck((SlabIndex-1) < mc_nNormalSizeLists);
						DMibFastCheck(SubIndex < t_CParams::mc_NumSizesPerLevel);
 					
						pList = &m_NormalSizeSlabs[SubIndex][SlabIndex-1];
					}
					else
					{

						mint InternalSlabBucketStart = sizeof(void *) * 2;
						mint SlabIndex = (AlignedSize - InternalSlabBucketStart) / mc_MinNormalSizeAlignment;

						DMibFastCheck(AlignedSize == Size);
						DMibFastCheck(AlignedSize >= 8);
						DMibFastCheck(SlabIndex < mc_nLevel0Lists);
						DMibFastCheck(Size != mc_Level0SmallestSize || SlabIndex == (mc_nLevel0Lists - 1));
					
						pList = &m_NormalSizeSlabsLevel0[SlabIndex];
					}
				}
				
				auto &List = *pList;

				auto pAlloc = List.f_GetFirst();
				
				if (likely(pAlloc))
				{
					if (pAlloc->m_nBlocks > 1)
					{
						uint32 iBlock = --pAlloc->m_nBlocks;
						
						pAlloc = (CMemoryManagerSubSlab_NormalLink *)((uint8 *)pAlloc + (AlignedSize * iBlock));
						
						if (this->mc_EnableCallbacks)
							this->f_OnCheckFree((uint8 *)pAlloc, AlignedSize, true);
					}
					else 
					{
						static_assert(sizeof(*pAlloc) == sizeof(void *) * 2 + 4, "Should be packed");
						
						pAlloc->m_Link.f_UnsafeUnlinkFirst();
						if (this->mc_EnableCallbacks)
							this->f_OnCheckFree((uint8 *)(pAlloc + 1), AlignedSize - sizeof(*pAlloc), true);
					}
					
					mint SlabStart = fg_AlignDown((mint)pAlloc, t_CParams::mc_SlabSize);
					auto Offset = ((mint)pAlloc - SlabStart) / t_CParams::mc_SubSlabSize;
					mint iAlloc = t_CParams::fs_DivideBySlabMultiplier(Offset, SubIndex);

					uint8 *pEndOfSlab = (uint8 *)SlabStart + t_CParams::mc_SlabSize;
					CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));
					TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);

					auto pData = pSlab->f_GetSubSlabData();

					DMibFastCheck(iAlloc < pSlab->f_GetNumSubSlabs());
					
					DMibFastCheck(pData[iAlloc].m_Allocated.m_nAllocs < 1024);
					if (++pData[iAlloc].m_Allocated.m_nAllocs == 1)
						fp_SubSlabNoLongerPending(pSlab, iAlloc);
						
	//					DMibTrace("++{} {} {}" DMibNewLine, pData[iAlloc].m_Allocated.m_nAllocs << iAlloc << pSlab->m_SlabType);

	#if DMibPPtrBits >= 64 && defined(DMibPNoUnalignedAccess)
					if ((AlignedSize & 4) && AlignedSize < 32)
					{
						mint SubSlabMultiplier = t_CParams::ms_SlabTypeInfo[SubIndex].m_SubSlabMutiplier;

						mint BaseAddress = SlabStart + iAlloc * SubSlabMultiplier * t_CParams::mc_SubSlabSize;

						pAlloc = (CMemoryManagerSubSlab_NormalLink *)(BaseAddress + ((((mint)pAlloc - BaseAddress) / AlignedSize) * AlignedSize));
					}
	#endif
					if (this->mc_EnableCallbacks)
						this->f_OnAlloc((uint8 *)pAlloc, AlignedSize);
					_Size = AlignedSize;
					return pAlloc;
				}
			}

			auto pRet = fp_AllocNormalUncached(pList, AlignedSize, SubIndex, SlabBucket);
			_Size = AlignedSize;
			return pRet;
		}
		
		template <typename t_CParams>
		inline_never void TCMemoryManagerArena<t_CParams>::fp_AllocNormalBatch(mint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
		{
			CNormalFreeList *pList;
			mint Size = fg_AlignUp(_Size, mc_MinNormalSizeAlignment);
			
			DMibFastCheck(Size >= 20);

			mint AlignedSize;
			mint SubIndex;
			mint SlabBucket;
			{
				{
					SlabBucket = NMib::fg_GetHighestBitSetNoZero(Size);
					mint SlabBucketT8 = SlabBucket - t_CParams::mc_SizesPerLevelShift;
					mint SlabBucketStart = mint(1) << SlabBucket;
					mint SlabBucketGranularity = (mint(1) << SlabBucketT8);
					AlignedSize = fg_AlignUp(Size, SlabBucketGranularity);
					SubIndex = (AlignedSize - SlabBucketStart) >> SlabBucketT8;
					SlabBucket += SubIndex >> t_CParams::mc_SizesPerLevelShift;
					SubIndex &= (mint(1) << t_CParams::mc_SizesPerLevelShift) - 1;

					if (likely(Size > mc_Level0SmallestSize))
					{
						mint SlabIndex = SlabBucket - 4;

						DMibFastCheck((SlabIndex-1) < mc_nNormalSizeLists);
						DMibFastCheck(SubIndex < t_CParams::mc_NumSizesPerLevel);
 					
						pList = &m_NormalSizeSlabs[SubIndex][SlabIndex-1];
					}
					else
					{

						mint InternalSlabBucketStart = sizeof(void *) * 2;
						mint SlabIndex = (AlignedSize - InternalSlabBucketStart) / mc_MinNormalSizeAlignment;

						DMibFastCheck(AlignedSize == Size);
						DMibFastCheck(AlignedSize >= 8);
						DMibFastCheck(SlabIndex < mc_nLevel0Lists);
						DMibFastCheck(Size != mc_Level0SmallestSize || SlabIndex == (mc_nLevel0Lists - 1));
					
						pList = &m_NormalSizeSlabsLevel0[SlabIndex];
					}
				}
				
				auto &List = *pList;

				while (1)
				{
					auto pAlloc = List.f_GetFirst();
					
					if (likely(pAlloc))
					{
						if (pAlloc->m_nBlocks > 1)
						{
							uint32 iBlock = --pAlloc->m_nBlocks;
							
							pAlloc = (CMemoryManagerSubSlab_NormalLink *)((uint8 *)pAlloc + (AlignedSize * iBlock));
							
							if (this->mc_EnableCallbacks)
								this->f_OnCheckFree((uint8 *)pAlloc, AlignedSize, true);
						}
						else 
						{
							static_assert(sizeof(*pAlloc) == sizeof(void *) * 2 + 4, "Should be packed");
							
							pAlloc->m_Link.f_UnsafeUnlinkFirst();
							if (this->mc_EnableCallbacks)
								this->f_OnCheckFree((uint8 *)(pAlloc + 1), AlignedSize - sizeof(*pAlloc), true);
						}
						
						mint SlabStart = fg_AlignDown((mint)pAlloc, t_CParams::mc_SlabSize);
						auto Offset = ((mint)pAlloc - SlabStart) / t_CParams::mc_SubSlabSize;
						mint iAlloc = t_CParams::fs_DivideBySlabMultiplier(Offset, SubIndex);

						uint8 *pEndOfSlab = (uint8 *)SlabStart + t_CParams::mc_SlabSize;
						CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));
						TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);

						auto pData = pSlab->f_GetSubSlabData();

						DMibFastCheck(iAlloc < pSlab->f_GetNumSubSlabs());
						
						DMibFastCheck(pData[iAlloc].m_Allocated.m_nAllocs < 1024);
						if (++pData[iAlloc].m_Allocated.m_nAllocs == 1)
							fp_SubSlabNoLongerPending(pSlab, iAlloc);
							
		//					DMibTrace("++{} {} {}" DMibNewLine, pData[iAlloc].m_Allocated.m_nAllocs << iAlloc << pSlab->m_SlabType);

		#if DMibPPtrBits >= 64 && defined(DMibPNoUnalignedAccess)
						if ((AlignedSize & 4) && AlignedSize < 32)
						{
							mint SubSlabMultiplier = t_CParams::ms_SlabTypeInfo[SubIndex].m_SubSlabMutiplier;

							mint BaseAddress = SlabStart + iAlloc * SubSlabMultiplier * t_CParams::mc_SubSlabSize;

							pAlloc = (CMemoryManagerSubSlab_NormalLink *)(BaseAddress + ((((mint)pAlloc - BaseAddress) / AlignedSize) * AlignedSize));
						}
		#endif
						if (this->mc_EnableCallbacks)
							this->f_OnAlloc((uint8 *)pAlloc, AlignedSize);
						if (!_Functor(pAlloc, AlignedSize))
							break;
					}
					else
					{
						if (!_Functor(fp_AllocNormalUncached(pList, AlignedSize, SubIndex, SlabBucket), AlignedSize))
							break;
					}
				}
			}
		}		

		template <typename t_CParams>
		inline_never void *TCMemoryManagerArena<t_CParams>::fp_AllocNormalUncached(CNormalFreeList *_pList, mint _AlignedSize, mint _SubIndex, mint _SlabBucket)
		{
			mint SizeType = 0;
			mint SubSlabMultiplier = t_CParams::ms_SlabTypeInfo[_SubIndex].m_SubSlabMutiplier;
					
			mint MinSize = SubSlabMultiplier * t_CParams::mc_SubSlabSize;
					
			if (_AlignedSize > MinSize)
			{
				uint32 nSubSlabs = t_CParams::fs_DivideBySlabMultiplier(_AlignedSize/t_CParams::mc_SubSlabSize, _SubIndex);
				SizeType = NMib::fg_GetHighestBitSetNoZero(nSubSlabs);
				DMibFastCheck(((mint(1) << SizeType)) == nSubSlabs);
			}

			auto pSlab = fp_NewSlab(_SubIndex, SizeType);

			CMemoryManagerSubSlab_Free *pExistingSlab = nullptr;
			if (SizeType == 0)
				pExistingSlab = pSlab->m_FreeSubSlabs.f_UnsafePop();
			uint8 *pSlabAddress;
			mint iAlloc;
			auto pData = pSlab->f_GetSubSlabData();

			if (pExistingSlab)
			{
				fp_CheckSlabNoLongerGarbage(pSlab);
				
				--pSlab->m_nFreeSubSlabs;
				pSlabAddress = (uint8 *)pExistingSlab;
				iAlloc = mint((uint8 *)pSlabAddress - pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;
				iAlloc = t_CParams::fs_DivideBySlabMultiplier(iAlloc, _SubIndex);
			}
			else
			{
				iAlloc = pSlab->f_FindFreeBitAndSet(SizeType) << SizeType;

				pSlab->f_CommitSubSlabs(iAlloc, (mint(1) << SizeType));
				
				pSlabAddress = pSlab->f_GetSlabStart() + iAlloc * t_CParams::mc_SubSlabSize * SubSlabMultiplier;
			}

			DMibFastCheck(iAlloc >= 0);
			DMibFastCheck(iAlloc < pSlab->f_GetNumSubSlabs());
			
			pSlab->m_nAllocatedSubSlabs += mint(1) << SizeType;

			auto &Alloc = pData[iAlloc].m_Allocated;
			Alloc.m_Type = _SlabBucket + 2;

			DMibFastCheck(Alloc.m_Type >= mc_nSmallSizeSlabs);
			DMibFastCheck(Alloc.m_Type < 21);
			DMibFastCheck(Alloc.m_nAllocs == 0);
			++Alloc.m_nAllocs;

#if defined(DMibPNoUnalignedAccess)
			CMemoryManagerSubSlab_NormalLink *pNormalLink = (CMemoryManagerSubSlab_NormalLink *)fg_AlignUp(pSlabAddress, sizeof(void *));
#else
			CMemoryManagerSubSlab_NormalLink *pNormalLink = (CMemoryManagerSubSlab_NormalLink *)pSlabAddress;
#endif
			
			uint32 nBlocks = fg_Max(t_CParams::ms_NumAllocsPerSubSlab[_SubIndex] >> _SlabBucket, 1);
			DMibFastCheck(nBlocks == (t_CParams::mc_SubSlabSize * (mint(1) << SizeType) * SubSlabMultiplier) / _AlignedSize);
			
			if (nBlocks > 1)
			{
				--nBlocks;
				
				pNormalLink->m_nBlocks = nBlocks;
				_pList->f_UnsafeInsertFirst(pNormalLink);
				
				if (this->mc_EnableCallbacks)
					this->f_OnFillFree((uint8 *)(pNormalLink + 1), (pSlabAddress + nBlocks * _AlignedSize) - (uint8 *)(pNormalLink + 1));
				
				pSlabAddress = pSlabAddress + nBlocks * _AlignedSize;
			}

			if (this->mc_EnableCallbacks)
				this->f_OnAlloc((uint8 *)pSlabAddress, _AlignedSize);
			
			return pSlabAddress;
		}
		

		template <typename t_CParams>
		inline_always void TCMemoryManagerArena<t_CParams>::fp_FreeInline(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab)
		{
			auto SubSlab = mint((uint8 *)_pMemory - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;
			
			mint SlabType = _pSlab->m_SlabType;
			auto pData = _pSlab->f_GetSubSlabData();

			mint iSubSlab;
			mint SlabBucket;

			if (unlikely(SlabType > 0))
			{
				iSubSlab = t_CParams::fs_DivideBySlabMultiplier(SubSlab, SlabType);
				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				SlabBucket = pData[iSubSlab].m_Allocated.m_Type;
			}
			else
			{
				iSubSlab = SubSlab;
				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				SlabBucket = pData[iSubSlab].m_Allocated.m_Type;
				if (unlikely(SlabBucket < mc_nSmallSizeSlabs))
				{
					fp_FreeSmall(_pMemory, (TCMemoryManagerSlab<t_CParams, 0> *)_pSlab, SlabBucket);
					return;
				}
			}


			SlabBucket -= 2;

			mint AlignedSize;

			CNormalFreeList *pList;
			mint SlabIndex;

			if (likely(SlabBucket > 4))
			{
				mint SlabBucketStart = mint(1) << SlabBucket;

				AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

				SlabIndex = SlabBucket - 4;

				DMibFastCheck((SlabIndex-1) < mc_nNormalSizeLists);
				DMibFastCheck(SlabType < t_CParams::mc_NumSizesPerLevel);
				
				pList = &m_NormalSizeSlabs[SlabType][SlabIndex-1];
			}
			else
			{
				mint InternalSlabBucketStart = sizeof(void *) * 2;
				mint SlabBucketStart = mint(1) << SlabBucket;
				AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

				SlabIndex = (AlignedSize - InternalSlabBucketStart) / mc_MinNormalSizeAlignment;

				DMibFastCheck(SlabIndex < mc_nLevel0Lists);
				DMibFastCheck(AlignedSize != mc_Level0SmallestSize || SlabIndex == (mc_nLevel0Lists - 1));
				
				pList = &m_NormalSizeSlabsLevel0[SlabIndex];
			}

			CMemoryManagerSubSlab_NormalLink *pFreeMemory = (CMemoryManagerSubSlab_NormalLink *)_pMemory;

			if (this->mc_EnableCallbacks)
			{
				this->f_OnFree((uint8 *)pFreeMemory);
				this->f_OnFillFree((uint8 *)(pFreeMemory + 1), AlignedSize - sizeof(*pFreeMemory));
			}
			
			DMibFastCheck(pData[iSubSlab].m_Allocated.m_nAllocs > 0);
 			
			pList->f_UnsafeInsertFirst(pFreeMemory);
			pFreeMemory->m_nBlocks = 1;

			if (unlikely((--pData[iSubSlab].m_Allocated.m_nAllocs) == 0))
			{
				if (t_CParams::mc_DeferCleanup & EDeferCleanup_NoCleanup)
				{
				}
				else if (t_CParams::mc_DeferCleanup & EDeferCleanup_Allocs)
				{
//					DMibFastCheck(_pSlab->f_GetNumPendingBits() == _pSlab->m_nPendingSubSlabs);
					_pSlab->f_SetPendingBit(iSubSlab);
					
					fp_SlabHasGarbage(_pSlab);

					++_pSlab->m_nPendingSubSlabs;
					DMibFastCheck(_pSlab->f_HasPendingBit());
//					DMibFastCheck(_pSlab->f_GetNumPendingBits() == _pSlab->m_nPendingSubSlabs);
				}
				else
					fp_FreeSubSlab(_pSlab, iSubSlab);
			}
		}
		
	}
}