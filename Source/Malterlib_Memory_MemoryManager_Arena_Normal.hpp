// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	fp32 TCMemoryManagerArena<t_CParams>::f_Overhead(void const *_pMemory, TCMemoryManagerSlabShared<t_CParams> const *_pSlab)
	{
		auto SubSlab = umint((uint8 *)_pMemory - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;

		umint SlabType = _pSlab->m_SlabType;
		auto pData = _pSlab->f_GetSubSlabDataType();

		umint iSubSlab;
		umint SlabBucket;

		if constexpr (mc_bSpecialCaseSlabType0)
		{
			if (SlabType > 0)
			{
				iSubSlab = t_CParams::fs_DivideBySlabMultiplier(SubSlab, SlabType);
				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				SlabBucket = pData[iSubSlab].m_Type;
			}
			else
			{
				iSubSlab = SubSlab;
				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				SlabBucket = pData[iSubSlab].m_Type;
				if constexpr (mc_bUseSmallSizes)
				{
					if (SlabBucket < mc_nSmallSizeSlabs) [[unlikely]]
						return fp_OverheadSmall(_pMemory, (TCMemoryManagerSlab<t_CParams, 0> const *)_pSlab, SlabBucket);
				}
			}
		}
		else
		{
			iSubSlab = t_CParams::fs_DivideBySlabMultiplier(SubSlab, SlabType);
			DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
			SlabBucket = pData[iSubSlab].m_Type;
			if constexpr (mc_bUseSmallSizes)
			{
				if (SlabBucket < mc_nSmallSizeSlabs) [[unlikely]]
					return fp_OverheadSmall(_pMemory, (TCMemoryManagerSlab<t_CParams, 0> const *)_pSlab, SlabBucket);
			}
		}

		SlabBucket -= 2;

		umint AlignedSize;
		umint SlabBucketStart = umint(1) << SlabBucket;
		AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

		fp32 OverheadPerByte = _pSlab->f_OverheadPerByte();

		return fp32(AlignedSize) * OverheadPerByte;
	}

	template <typename t_CParams>
	umint TCMemoryManagerArena<t_CParams>::f_Size(void const *_pMemory, TCMemoryManagerSlabShared<t_CParams> const *_pSlab)
	{
		auto SubSlab = umint((uint8 *)_pMemory - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;

		umint SlabType = _pSlab->m_SlabType;
		auto pData = _pSlab->f_GetSubSlabDataType();

		umint iSubSlab;
		umint SlabBucket;

		if constexpr (mc_bSpecialCaseSlabType0)
		{
			if (SlabType > 0)
			{
				iSubSlab = t_CParams::fs_DivideBySlabMultiplier(SubSlab, SlabType);
				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				SlabBucket = pData[iSubSlab].m_Type;
			}
			else
			{
				iSubSlab = SubSlab;
				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				SlabBucket = pData[iSubSlab].m_Type;
				if constexpr (mc_bUseSmallSizes)
				{
					if (SlabBucket < mc_nSmallSizeSlabs) [[unlikely]]
						return fp_SizeSmall(_pMemory, (TCMemoryManagerSlab<t_CParams, 0> const *)_pSlab, SlabBucket);
				}
			}
		}
		else
		{
			iSubSlab = t_CParams::fs_DivideBySlabMultiplier(SubSlab, SlabType);
			DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
			SlabBucket = pData[iSubSlab].m_Type;
			if constexpr (mc_bUseSmallSizes)
			{
				if (SlabBucket < mc_nSmallSizeSlabs) [[unlikely]]
					return fp_SizeSmall(_pMemory, (TCMemoryManagerSlab<t_CParams, 0> const *)_pSlab, SlabBucket);
			}
		}

		SlabBucket -= 2;

		umint AlignedSize;
		umint SlabBucketStart = umint(1) << SlabBucket;
		AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

		return AlignedSize;
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::fp_CheckFree(CMemoryManagerSubSlab_NormalLink *_pLink, EMemoryManagerCheckFlag _Flags)
	{
		bool bError = false;
		uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pLink + 1, t_CParams::mc_SlabSize);
		CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

		TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);

		umint AlignedSize = f_Size(_pLink, pSlab);

		if constexpr (mc_EnableCallbacks)
		{
			uint8 *pStart = (uint8 *)(_pLink + 1);
			uint8 *pEnd;
			if constexpr (t_CParams::mc_bUseFreeBlockCounting)
				pEnd = (uint8 *)_pLink + (AlignedSize * _pLink->m_nBlocks);
			else
				pEnd = (uint8 *)_pLink + AlignedSize;

			if (this->f_OnCheckFree(pStart, pEnd - pStart, _Flags))
				bError = true;

		}

		return bError;
	}

	template <typename t_CParams>
	inline_always void *TCMemoryManagerArena<t_CParams>::fp_AllocNormal(umint &_Size)
	{
		CMemoryManagerSubSlab_NormalFreeList *pList;
		umint Size;
		if constexpr (mc_MinAllocSize > 1)
			Size = fg_AlignUp(fg_Max(_Size, mc_MinAllocSize), t_CParams::mc_MinNormalSizeAlignment);
		else
			Size = fg_AlignUp(_Size, t_CParams::mc_MinNormalSizeAlignment);

		DMibMemLightweightTrack(m_pMemoryManager->fp_TrackAlloc(Size));

		DMibFastCheck(Size >= t_CParams::mc_MinNormalAllocSize);

		umint AlignedSize;
		umint SubIndex;
		umint SlabBucket;
		{
			{
				SlabBucket = NMib::fg_GetHighestBitSetNoZero(Size);
				umint SlabBucketT8 = SlabBucket - t_CParams::mc_SizesPerLevelShift;
				umint SlabBucketStart = umint(1) << SlabBucket;
				umint SlabBucketGranularity = (umint(1) << SlabBucketT8);
				AlignedSize = fg_AlignUp(Size, SlabBucketGranularity);
				SubIndex = (AlignedSize - SlabBucketStart) >> SlabBucketT8;
				SlabBucket += SubIndex >> t_CParams::mc_SizesPerLevelShift;
				SubIndex &= (umint(1) << t_CParams::mc_SizesPerLevelShift) - 1;

				if constexpr (t_CParams::mc_bUseSmallSizes)
				{
					if (Size > mc_Level0SmallestSize) [[likely]]
					{
						umint SlabIndex = SlabBucket - 4;

						DMibFastCheck((SlabIndex-1) < mc_nNormalSizeLists);
						DMibFastCheck(SubIndex < t_CParams::mc_NumSizesPerLevel);

						pList = &m_NormalSizeSlabs[SubIndex][SlabIndex-1];
					}
					else
					{

						umint InternalSlabBucketStart = sizeof(void *) * 2;
						umint SlabIndex = (AlignedSize - InternalSlabBucketStart) / t_CParams::mc_MinNormalSizeAlignment;

						DMibFastCheck(AlignedSize == Size);
						DMibFastCheck(AlignedSize >= 8);
						DMibFastCheck(SlabIndex < mc_nLevel0Lists);
						DMibFastCheck(Size != mc_Level0SmallestSize || SlabIndex == (mc_nLevel0Lists - 1));

						pList = &m_NormalSizeSlabsLevel0[SlabIndex];
					}
				}
				else
				{
					umint SlabIndex = SlabBucket - 4;

					DMibFastCheck((SlabIndex) < mc_nNormalSizeLists);
					DMibFastCheck(SubIndex < t_CParams::mc_NumSizesPerLevel);

					pList = &m_NormalSizeSlabs[SubIndex][SlabIndex];
				}
			}

			auto &List = *pList;

		l_Retry:
			auto pAlloc = List.f_GetFirst();

			if (pAlloc) [[likely]]
			{
				if constexpr (t_CParams::mc_bUseFreeBlockCounting)
				{
					if (pAlloc->m_nBlocks > 1)
					{
						uint32 iBlock = --pAlloc->m_nBlocks;

						pAlloc = (CMemoryManagerSubSlab_NormalLink *)((uint8 *)pAlloc + (AlignedSize * iBlock));

						if constexpr (mc_EnableCallbacks)
							this->f_OnCheckFree((uint8 *)pAlloc, AlignedSize, EMemoryManagerCheckFlag_Default);
					}
					else
					{
						static_assert(sizeof(*pAlloc) == sizeof(void *) * 2 + 4, "Should be packed");

						pAlloc->m_Link.f_UnsafeUnlinkFirst();
						if constexpr (mc_EnableCallbacks)
							this->f_OnCheckFree((uint8 *)(pAlloc + 1), AlignedSize - sizeof(*pAlloc), EMemoryManagerCheckFlag_Default);
					}
				}
				else
				{
					DMibFastCheck(pAlloc->m_Link.f_IsInList());
					pAlloc->m_Link.f_UnsafeUnlinkFirst();
					if constexpr (mc_EnableCallbacks)
						this->f_OnCheckFree((uint8 *)(pAlloc + 1), AlignedSize - sizeof(*pAlloc), EMemoryManagerCheckFlag_Default);
				}

				umint SlabStart = fg_AlignDown((umint)pAlloc, t_CParams::mc_SlabSize);
				auto Offset = ((umint)pAlloc - SlabStart) / t_CParams::mc_SubSlabSize;
				umint iAlloc = t_CParams::fs_DivideBySlabMultiplier(Offset, SubIndex);

				uint8 *pEndOfSlab = (uint8 *)SlabStart + t_CParams::mc_SlabSize;
				CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));
				TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);

				auto pData = pSlab->f_GetSubSlabDataAlloc();

				DMibFastCheck(iAlloc < pSlab->f_GetNumSubSlabs());

				auto &Data = pData[iAlloc];

				DMibFastCheck(Data.m_nAllocs < TCMemoryManagerSubSlabDataAlloc<t_CParams>::mc_MaxAllocs);
				if (++Data.m_nAllocs == 1) [[unlikely]]
					fp_SubSlabNoLongerPending(pSlab, iAlloc);

//					DMibTrace("++{} {} {}" DMibNewLine, Data.m_nAllocs, iAlloc, pSlab->m_SlabType);

#if DMibPPtrBits >= 64 && defined(DMibPNoUnalignedAccess)
				if constexpr (t_CParams::mc_bUseFreeBlockCounting)
				{
					if ((AlignedSize & 4) && AlignedSize < 32)
					{
						umint SubSlabMultiplier = t_CParams::mc_SlabTypeInfo[SubIndex].m_SubSlabMutiplier;

						umint BaseAddress = SlabStart + iAlloc * SubSlabMultiplier * t_CParams::mc_SubSlabSize;

						pAlloc = (CMemoryManagerSubSlab_NormalLink *)(BaseAddress + ((((umint)pAlloc - BaseAddress) / AlignedSize) * AlignedSize));
					}
				}
#endif
				if constexpr (mc_EnableCallbacks)
					this->f_OnAlloc((uint8 *)pAlloc, AlignedSize);
				_Size = AlignedSize;
				return pAlloc;
			}
			else
			{
				if (fp_ProcessMessages(pList))
					goto l_Retry;
			}
		}

		auto pRet = fp_AllocNormalUncached(pList, AlignedSize, SubIndex, SlabBucket);
		_Size = AlignedSize;
		return pRet;
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_AllocNormalBatch(umint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor)
	{
		CMemoryManagerSubSlab_NormalFreeList *pList;
		umint Size;
		if constexpr (mc_MinAllocSize > 1)
			Size = fg_AlignUp(fg_Max(_Size, mc_MinAllocSize), t_CParams::mc_MinNormalSizeAlignment);
		else
			Size = fg_AlignUp(_Size, t_CParams::mc_MinNormalSizeAlignment);

		DMibFastCheck(Size >= t_CParams::mc_MinNormalAllocSize);

		umint AlignedSize;
		umint SubIndex;
		umint SlabBucket;
		{
			{
				SlabBucket = NMib::fg_GetHighestBitSetNoZero(Size);
				umint SlabBucketT8 = SlabBucket - t_CParams::mc_SizesPerLevelShift;
				umint SlabBucketStart = umint(1) << SlabBucket;
				umint SlabBucketGranularity = (umint(1) << SlabBucketT8);
				AlignedSize = fg_AlignUp(Size, SlabBucketGranularity);
				SubIndex = (AlignedSize - SlabBucketStart) >> SlabBucketT8;
				SlabBucket += SubIndex >> t_CParams::mc_SizesPerLevelShift;
				SubIndex &= (umint(1) << t_CParams::mc_SizesPerLevelShift) - 1;

				if constexpr (t_CParams::mc_bUseSmallSizes)
				{
					if (Size > mc_Level0SmallestSize) [[likely]]
					{
						umint SlabIndex = SlabBucket - 4;

						DMibFastCheck((SlabIndex-1) < mc_nNormalSizeLists);
						DMibFastCheck(SubIndex < t_CParams::mc_NumSizesPerLevel);

						pList = &m_NormalSizeSlabs[SubIndex][SlabIndex-1];
					}
					else
					{

						umint InternalSlabBucketStart = sizeof(void *) * 2;
						umint SlabIndex = (AlignedSize - InternalSlabBucketStart) / t_CParams::mc_MinNormalSizeAlignment;

						DMibFastCheck(AlignedSize == Size);
						DMibFastCheck(AlignedSize >= 8);
						DMibFastCheck(SlabIndex < mc_nLevel0Lists);
						DMibFastCheck(Size != mc_Level0SmallestSize || SlabIndex == (mc_nLevel0Lists - 1));

						pList = &m_NormalSizeSlabsLevel0[SlabIndex];
					}
				}
				else
				{
					umint SlabIndex = SlabBucket - 4;

					DMibFastCheck((SlabIndex) < mc_nNormalSizeLists);
					DMibFastCheck(SubIndex < t_CParams::mc_NumSizesPerLevel);

					pList = &m_NormalSizeSlabs[SubIndex][SlabIndex];
				}
			}

			auto &List = *pList;

			DMibMemLightweightTrack
				(
					auto *pLocalArena = m_pMemoryManager->m_LocalArena.f_TryGet();
				)
			;

			while (1)
			{
				DMibMemLightweightTrack
					(
						{
							if (TCMemoryManager<t_CParams>::fsp_ShouldTrackAlloc(pLocalArena))
								pLocalArena->f_TrackAlloc(AlignedSize);
						}
					)
				;

				auto pAlloc = List.f_GetFirst();

				if (pAlloc) [[likely]]
				{
					if constexpr (t_CParams::mc_bUseFreeBlockCounting)
					{
						if (pAlloc->m_nBlocks > 1)
						{
							uint32 iBlock = --pAlloc->m_nBlocks;

							pAlloc = (CMemoryManagerSubSlab_NormalLink *)((uint8 *)pAlloc + (AlignedSize * iBlock));

							if constexpr (mc_EnableCallbacks)
								this->f_OnCheckFree((uint8 *)pAlloc, AlignedSize, EMemoryManagerCheckFlag_Default);
						}
						else
						{
							static_assert(sizeof(*pAlloc) == sizeof(void *) * 2 + 4, "Should be packed");

							pAlloc->m_Link.f_UnsafeUnlinkFirst();
							if constexpr (mc_EnableCallbacks)
								this->f_OnCheckFree((uint8 *)(pAlloc + 1), AlignedSize - sizeof(*pAlloc), EMemoryManagerCheckFlag_Default);
						}
					}
					else
					{
						pAlloc->m_Link.f_UnsafeUnlinkFirst();
						if constexpr (mc_EnableCallbacks)
							this->f_OnCheckFree((uint8 *)(pAlloc + 1), AlignedSize - sizeof(*pAlloc), EMemoryManagerCheckFlag_Default);
					}

					umint SlabStart = fg_AlignDown((umint)pAlloc, t_CParams::mc_SlabSize);
					auto Offset = ((umint)pAlloc - SlabStart) / t_CParams::mc_SubSlabSize;
					umint iAlloc = t_CParams::fs_DivideBySlabMultiplier(Offset, SubIndex);

					uint8 *pEndOfSlab = (uint8 *)SlabStart + t_CParams::mc_SlabSize;
					CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));
					TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);

					auto pData = pSlab->f_GetSubSlabDataAlloc();

					DMibFastCheck(iAlloc < pSlab->f_GetNumSubSlabs());

					auto &Data = pData[iAlloc];

					DMibFastCheck(Data.m_nAllocs < TCMemoryManagerSubSlabDataAlloc<t_CParams>::mc_MaxAllocs);
					if (++Data.m_nAllocs == 1)
						fp_SubSlabNoLongerPending(pSlab, iAlloc);

	//					DMibTrace("++{} {} {}" DMibNewLine, pData[iAlloc].m_nAllocs, iAlloc, pSlab->m_SlabType);

	#if DMibPPtrBits >= 64 && defined(DMibPNoUnalignedAccess)
					if constexpr (t_CParams::mc_bUseFreeBlockCounting)
					{
						if ((AlignedSize & 4) && AlignedSize < 32)
						{
							umint SubSlabMultiplier = t_CParams::mc_SlabTypeInfo[SubIndex].m_SubSlabMutiplier;

							umint BaseAddress = SlabStart + iAlloc * SubSlabMultiplier * t_CParams::mc_SubSlabSize;

							pAlloc = (CMemoryManagerSubSlab_NormalLink *)(BaseAddress + ((((umint)pAlloc - BaseAddress) / AlignedSize) * AlignedSize));
						}
					}
	#endif
					if constexpr (mc_EnableCallbacks)
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
	inline_never void *TCMemoryManagerArena<t_CParams>::fp_AllocNormalUncached(CMemoryManagerSubSlab_NormalFreeList *_pList, umint _AlignedSize, umint _SubIndex, umint _SlabBucket)
	{
		umint SizeType = 0;
		umint SubSlabMultiplier = t_CParams::mc_SlabTypeInfo[_SubIndex].m_SubSlabMutiplier;

		umint MinSize = SubSlabMultiplier * t_CParams::mc_SubSlabSize;

		if (_AlignedSize > MinSize)
		{
			uint32 nSubSlabs = t_CParams::fs_DivideBySlabMultiplier(_AlignedSize/t_CParams::mc_SubSlabSize, _SubIndex);
			SizeType = NMib::fg_GetHighestBitSetNoZero(nSubSlabs);
			DMibFastCheck(((umint(1) << SizeType)) == nSubSlabs);
		}

		auto pSlab = fp_NewSlab(_SubIndex, SizeType);

		uint8 *pSlabAddress;
		umint iAlloc;

		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			CMemoryManagerSubSlab_Free *pExistingSlab = nullptr;
			if (SizeType == 0)
				pExistingSlab = pSlab->m_FreeSubSlabs.f_UnsafePop();

			if (pExistingSlab)
			{
				fp_CheckSlabNoLongerGarbage(pSlab);

				--pSlab->m_nFreeSubSlabs;
				pSlabAddress = (uint8 *)pExistingSlab;
				iAlloc = umint((uint8 *)pSlabAddress - pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;
				iAlloc = t_CParams::fs_DivideBySlabMultiplier(iAlloc, _SubIndex);

				if constexpr (mc_EnableCallbacks)
					pSlab->f_OnCommitSubSlabs(iAlloc, (umint(1) << SizeType));
			}
			else
			{
				iAlloc = pSlab->f_FindFreeBitAndSet(SizeType) << SizeType;
				pSlab->f_CommitSubSlabs(iAlloc, (umint(1) << SizeType));
				pSlabAddress = pSlab->f_GetSlabStart() + iAlloc * t_CParams::mc_SubSlabSize * SubSlabMultiplier;
			}
		}
		else
		{
			iAlloc = pSlab->f_FindFreeBitAndSet(SizeType) << SizeType;
			pSlab->f_CommitSubSlabs(iAlloc, (umint(1) << SizeType));
			pSlabAddress = pSlab->f_GetSlabStart() + iAlloc * t_CParams::mc_SubSlabSize * SubSlabMultiplier;
		}

		auto pDataType = pSlab->f_GetSubSlabDataType();
		auto pDataAlloc = pSlab->f_GetSubSlabDataAlloc();

		DMibFastCheck(iAlloc >= 0);
		DMibFastCheck(iAlloc < pSlab->f_GetNumSubSlabs());

		pSlab->m_nAllocatedSubSlabs += umint(1) << SizeType;

		DMibFastCheck(_SlabBucket + 2 <= TCMemoryManagerSubSlabDataType<t_CParams>::mc_MaxType);
		pDataType[iAlloc].m_Type = _SlabBucket + 2;

		DMibFastCheck(pDataType[iAlloc].m_Type >= mc_nSmallSizeSlabs);
		DMibFastCheck(pDataType[iAlloc].m_Type < t_CParams::mc_NumSizeLevels + 2);
		DMibFastCheck(pDataAlloc[iAlloc].m_nAllocs == 0);
		++pDataAlloc[iAlloc].m_nAllocs;

		DMibFastCheck(_SlabBucket >= t_CParams::mc_MinNormalSlabBucket);

		uint32 nBlocks = fg_Max(t_CParams::mc_NumAllocsPerSubSlab[_SubIndex] >> (_SlabBucket - t_CParams::mc_MinNormalSlabBucket), 1);
		DMibFastCheck(nBlocks == (t_CParams::mc_SubSlabSize * (umint(1) << SizeType) * SubSlabMultiplier) / _AlignedSize);

		if (nBlocks > 1)
		{
			--nBlocks;
			if constexpr (t_CParams::mc_bUseFreeBlockCounting)
			{
#if defined(DMibPNoUnalignedAccess)
				CMemoryManagerSubSlab_NormalLink *pNormalLink;
				if constexpr (t_CParams::mc_bUseFreeBlockCounting)
					pNormalLink = (CMemoryManagerSubSlab_NormalLink *)fg_AlignUp(pSlabAddress, sizeof(void *));
				else
					pNormalLink = (CMemoryManagerSubSlab_NormalLink *)pSlabAddress;
#else
				CMemoryManagerSubSlab_NormalLink *pNormalLink = (CMemoryManagerSubSlab_NormalLink *)pSlabAddress;
#endif

				pNormalLink->m_nBlocks = nBlocks;
				_pList->f_UnsafeInsertFirst(pNormalLink);

				if constexpr (mc_EnableCallbacks)
					this->f_OnFillFree((uint8 *)(pNormalLink + 1), (pSlabAddress + nBlocks * _AlignedSize) - (uint8 *)(pNormalLink + 1));

				pSlabAddress = pSlabAddress + nBlocks * _AlignedSize;
			}
			else
			{
				if constexpr (mc_EnableCallbacks)
					this->f_OnFillFree(pSlabAddress + _AlignedSize, nBlocks * _AlignedSize);

				CMemoryManagerSubSlab_NormalFreeList TempList;

				if constexpr (t_CParams::mc_PreventCacheConflictSize)
				{
					if (((umint)pSlabAddress & (umint)(t_CParams::mc_PreventCacheConflictSize - 1)) == 0 && _AlignedSize <= t_CParams::mc_PreventCacheConflictSizeMaxBlockSize)
					{
						smint ToRemove = DMibPMemoryCacheLineSize;
						while (ToRemove > 0 && nBlocks > 1)
						{
							--nBlocks;
							pSlabAddress += _AlignedSize;
							ToRemove -= _AlignedSize;
						}
					}
				}

				for (auto pLinkAddress = pSlabAddress + nBlocks  * _AlignedSize; pLinkAddress > pSlabAddress; pLinkAddress -= _AlignedSize)
					_pList->f_UnsafeInsertFirst((CMemoryManagerSubSlab_NormalLink *)pLinkAddress);
			}
		}

		if constexpr (mc_EnableCallbacks)
			this->f_OnAlloc((uint8 *)pSlabAddress, _AlignedSize);

		return pSlabAddress;
	}


	template <typename t_CParams>
	inline_always void TCMemoryManagerArena<t_CParams>::fp_FreeInline(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab)
	{
		auto SubSlab = umint((uint8 *)_pMemory - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;

		umint SlabType = _pSlab->m_SlabType;
		auto pData = _pSlab->f_GetSubSlabDataType();

		umint iSubSlab;
		umint SlabBucket;

		if constexpr (mc_bSpecialCaseSlabType0)
		{
			if (SlabType == 0)
			{
				iSubSlab = SubSlab;
				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				SlabBucket = pData[iSubSlab].m_Type;
				if constexpr (t_CParams::mc_bUseSmallSizes)
				{
					if (SlabBucket < mc_nSmallSizeSlabs) [[unlikely]]
					{
						fp_FreeSmall(_pMemory, (TCMemoryManagerSlab<t_CParams, 0> *)_pSlab, SlabBucket);
						return;
					}
				}
			}
			else
			{
				iSubSlab = t_CParams::fs_DivideBySlabMultiplier(SubSlab, SlabType);
				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				SlabBucket = pData[iSubSlab].m_Type;
			}
		}
		else
		{
			iSubSlab = t_CParams::fs_DivideBySlabMultiplier(SubSlab, SlabType);
			DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
			SlabBucket = pData[iSubSlab].m_Type;
			if constexpr (t_CParams::mc_bUseSmallSizes)
			{
				if (SlabBucket < mc_nSmallSizeSlabs) [[unlikely]]
				{
					fp_FreeSmall(_pMemory, (TCMemoryManagerSlab<t_CParams, 0> *)_pSlab, SlabBucket);
					return;
				}
			}
		}

		SlabBucket -= 2;

		umint AlignedSize;

		CMemoryManagerSubSlab_NormalFreeList *pList;
		umint SlabIndex;

		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			if (SlabBucket > 4) [[likely]]
			{
				umint SlabBucketStart = umint(1) << SlabBucket;

				AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

				SlabIndex = SlabBucket - 4;

				DMibFastCheck((SlabIndex-1) < mc_nNormalSizeLists);
				DMibFastCheck(SlabType < t_CParams::mc_NumSizesPerLevel);

				pList = &m_NormalSizeSlabs[SlabType][SlabIndex-1];
			}
			else
			{
				umint InternalSlabBucketStart = sizeof(void *) * 2;
				umint SlabBucketStart = umint(1) << SlabBucket;
				AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

				SlabIndex = (AlignedSize - InternalSlabBucketStart) / t_CParams::mc_MinNormalSizeAlignment;

				DMibFastCheck(SlabIndex < mc_nLevel0Lists);
				DMibFastCheck(AlignedSize != mc_Level0SmallestSize || SlabIndex == (mc_nLevel0Lists - 1));

				pList = &m_NormalSizeSlabsLevel0[SlabIndex];
			}
		}
		else
		{
			DMibFastCheck(SlabBucket >= 4);

			umint SlabBucketStart = umint(1) << SlabBucket;

			AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

			SlabIndex = SlabBucket - 4;

			DMibFastCheck(SlabIndex < mc_nNormalSizeLists);
			DMibFastCheck(SlabType < t_CParams::mc_NumSizesPerLevel);

			pList = &m_NormalSizeSlabs[SlabType][SlabIndex];
		}

		CMemoryManagerSubSlab_NormalLink *pFreeMemory = (CMemoryManagerSubSlab_NormalLink *)_pMemory;

		if constexpr (mc_EnableCallbacks)
		{
			this->f_OnFree((uint8 *)pFreeMemory);
			this->f_OnFillFree((uint8 *)(pFreeMemory + 1), AlignedSize - sizeof(*pFreeMemory));
		}

		auto pDataAlloc = _pSlab->f_GetSubSlabDataAlloc();

		DMibFastCheck(pDataAlloc[iSubSlab].m_nAllocs > 0);

		if constexpr (t_CParams::mc_bUseFreeBlockCounting)
			pFreeMemory->m_nBlocks = 1;

		pList->f_UnsafeInsertFirst(pFreeMemory);

		if ((--pDataAlloc[iSubSlab].m_nAllocs) == 0) [[unlikely]]
		{
			if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_NoCleanup) != 0)
			{
			}
			else if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_Allocs) != 0)
			{
//				DMibFastCheck(_pSlab->f_GetNumPendingBits() == _pSlab->m_nPendingSubSlabs);
				if constexpr (t_CParams::mc_MaxPendingSubSlabs)
				{
					if (_pSlab->m_nPendingSubSlabs >= t_CParams::mc_MaxPendingSubSlabs)
					{
						(void)fp_FreeSubSlab(_pSlab, iSubSlab);
						return;
					}
				}

				_pSlab->f_SetPendingBit(iSubSlab);

				fp_SlabHasGarbage(_pSlab);

				++_pSlab->m_nPendingSubSlabs;
				DMibFastCheck(_pSlab->f_HasPendingBit());
//				DMibFastCheck(_pSlab->f_GetNumPendingBits() == _pSlab->m_nPendingSubSlabs);
			}
			else
				(void)fp_FreeSubSlab(_pSlab, iSubSlab); // _pSlab will potentially be invalid after this call
		}
	}
}
