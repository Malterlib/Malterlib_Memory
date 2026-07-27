// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	inline_always TCMemoryManagerSlabShared<t_CParams> *TCMemoryManagerArena<t_CParams>::fsp_SlabFromSubSlabNode(CMemoryManagerSubSlab_ListNode *_pNode, umint &o_iSubSlab)
	{
		uint8 *pEndOfSlab = fg_AlignUp((uint8 *)_pNode + 1, t_CParams::mc_SlabSize);
		CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));
		TCMemoryManagerSlabShared<t_CParams> *pSlab = (TCMemoryManagerSlabShared<t_CParams> *)(pEndOfSlab - pHeader->m_SlabStartOffset);

		o_iSubSlab = umint(_pNode - pSlab->m_pSubSlabNodes);
		DMibFastCheck(o_iSubSlab < pSlab->f_GetNumSubSlabs());

		return pSlab;
	}

	template <typename t_CParams>
	inline_always umint TCMemoryManagerArena<t_CParams>::fsp_GetSubSlabBlocks(umint _SlabType, umint _SlabBucket)
	{
		return fg_Max(umint(t_CParams::mc_SlabTypeFast[_SlabType].m_nAllocsPerSubSlab) >> (_SlabBucket - t_CParams::mc_MinNormalSlabBucket), umint(1));
	}

	template <typename t_CParams>
	inline_always umint TCMemoryManagerArena<t_CParams>::fsp_BitmapRemoteWordsOffset(umint _nWords)
	{
		// mc_bReapInCleanup with dense sub-slabs: the per-block pending words follow the free
		// bitmap words in-band, padded to the next cache line so remote fetch_or traffic does
		// not false-share the owner-hot free words
		return fg_AlignUp(_nWords * sizeof(uint64), umint(DMibPMemoryCacheLineSize));
	}

	template <typename t_CParams>
	inline_always umint TCMemoryManagerArena<t_CParams>::fsp_BitmapBlockIndex(umint _ByteOffset, umint _SlabType, umint _SlabBucket)
	{
		// AlignedSize = m_SubSlabMutiplier[_SlabType] << Shift, so dividing by the block size is
		// a shift followed by the multiplier reciprocal. This cannot reuse
		// fs_DivideBySlabMultiplier: its precondition is sub-slab indices below
		// mc_MaxNumSubSlabs, while the scaled value here is block index times multiplier, up to
		// mc_MaxAllocsPerSubSlabActual * multiplier. The reciprocal stays exact over that range
		// for every multiplier table (the verification check below holds for all block counts
		// the static asserts admit).
		DMibFastCheck(_SlabType < t_CParams::mc_NumSizesPerLevel);

		auto const &TypeFast = t_CParams::mc_SlabTypeFast[_SlabType];

		umint Shift = _SlabBucket - t_CParams::mc_SizesPerLevelShift + TypeFast.m_BlockIndexShift;
		uint32 Scaled = uint32(_ByteOffset >> Shift);

		uint32 Return = uint32((Scaled * uint32(TypeFast.m_DivideMultiply)) >> TypeFast.m_DivideShift);

		DMibFastCheck(Return == Scaled / TypeFast.m_SubSlabMultiplier);

		return Return;
	}

	// Called when the cached word is empty: clear its (stale) summary bit and move the cursor
	// to the lowest word that still has free bits, or drop the current sub-slab when none is
	// left. Frees during the drain may have refilled words below the cursor, so the rescan
	// also restores lowest-address-first at word granularity.
	template <typename t_CParams>
	inline_never bool TCMemoryManagerArena<t_CParams>::fsp_BitmapAdvanceCurrentWord(typename TCMemoryManagerArena<t_CParams>::CNormalFreeStoreCurrent &_Current, umint _AlignedSize)
	{
		auto &FreeState = *_Current.m_pFreeState;

		if (_Current.m_pWord == &FreeState.m_Bits)
		{
			DMibFastCheck(FreeState.m_Bits == 0);
			_Current.m_pFreeState = nullptr;

			return false;
		}

		FreeState.m_Bits &= ~(uint64(1) << (_Current.m_WordIndexBase >> 6));

		if (FreeState.m_Bits == 0)
		{
			_Current.m_pFreeState = nullptr;

			return false;
		}

		umint iWord = NMib::fg_GetLowestBitSetNoZero(FreeState.m_Bits);

		_Current.m_pWord = (uint64 *)_Current.m_pBase + iWord;
		_Current.m_WordIndexBase = uint32(iWord * 64);
		_Current.m_pWordBase = _Current.m_pBase + iWord * 64 * _AlignedSize;

		return true;
	}

	template <typename t_CParams>
	inline_always bool TCMemoryManagerArena<t_CParams>::fsp_BitmapFreeBlock
		(
			CMemoryManagerSubSlabFreeBits &_Bits
			, uint8 *_pBase
			, void *_pMemory
			, umint _AlignedSize
			, umint _SlabType
			, umint _SlabBucket
			, umint _nBlocks
		)
	{
		umint Index = fsp_BitmapBlockIndex(umint((uint8 *)_pMemory - _pBase), _SlabType, _SlabBucket);

		DMibFastCheck(Index < _nBlocks);
		DMibFastCheck(_pBase + Index * _AlignedSize == (uint8 *)_pMemory);

		if (_nBlocks <= 64)
		{
			// The header slot is the bitmap itself
			bool bWasExhausted = _Bits.m_Bits == 0;

			DMibFastCheck(!(_Bits.m_Bits & (uint64(1) << Index))); // Double free
			_Bits.m_Bits |= uint64(1) << Index;

			return bWasExhausted;
		}

		uint64 *pWords = (uint64 *)_pBase;
		umint iWord = Index >> 6;
		uint64 Word = pWords[iWord];

		DMibFastCheck(!(Word & (uint64(1) << (Index & 63)))); // Double free

		pWords[iWord] = Word | (uint64(1) << (Index & 63));

		// A non-empty in-band word always has its summary bit set, so the header slot only has to
		// be read and written by the free that makes the word non-empty. Every producer keeps
		// this: the carve sets the summary per non-empty word, the reap sets it for every word it
		// fills, and the two clearers (cursor advance and batch detach) only clear the bit of a
		// word they have just emptied. That also means an exhausted sub-slab must have every word
		// empty, so this free's word being non-empty proves it was not exhausted.
		if (Word != 0) [[likely]]
		{
			DMibFastCheck(_Bits.m_Bits & (uint64(1) << iWord));

			return false;
		}

		bool bWasExhausted = _Bits.m_Bits == 0;

		_Bits.m_Bits |= uint64(1) << iWord;

		return bWasExhausted;
	}

	template <typename t_CParams>
	inline_always void TCMemoryManagerArena<t_CParams>::fp_SetCurrentSubSlab(CNormalFreeStoreCurrent *_pCurrent, TCMemoryManagerSlabShared<t_CParams> *_pSlab, umint _iSubSlab)
	{
		if constexpr (mc_bSubSlabStore)
		{
			umint SubSlabMultiplier = t_CParams::mc_SlabTypeFast[_pSlab->m_SlabType].m_SubSlabMultiplier;

			_pCurrent->m_pFreeState = &_pSlab->m_pSubSlabFreeState[_iSubSlab];
			_pCurrent->m_pBase = _pSlab->f_GetSlabStart() + _iSubSlab * (SubSlabMultiplier * t_CParams::mc_SubSlabSize);
			_pCurrent->m_pDataAlloc = &_pSlab->f_GetSubSlabDataAlloc()[_iSubSlab];
			_pCurrent->m_pSlab = _pSlab;
			_pCurrent->m_iSubSlab = uint32(_iSubSlab);

			if constexpr (mc_bSubSlabBitmaps)
			{
				auto &FreeState = _pSlab->m_pSubSlabFreeState[_iSubSlab];
				umint SlabBucket = _pSlab->f_GetSubSlabDataType()[_iSubSlab].m_Type - 2;

				if (fsp_GetSubSlabBlocks(_pSlab->m_SlabType, SlabBucket) <= 64)
				{
					_pCurrent->m_pWord = &FreeState.m_Bits;
					_pCurrent->m_WordIndexBase = 0;
					_pCurrent->m_pWordBase = _pCurrent->m_pBase;
				}
				else
				{
					DMibFastCheck(FreeState.m_Bits != 0);

					umint iWord = NMib::fg_GetLowestBitSetNoZero(FreeState.m_Bits);
					umint AlignedSize = (umint(1) << SlabBucket) + (_pSlab->m_SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

					_pCurrent->m_pWord = (uint64 *)_pCurrent->m_pBase + iWord;
					_pCurrent->m_WordIndexBase = uint32(iWord * 64);
					_pCurrent->m_pWordBase = _pCurrent->m_pBase + iWord * 64 * AlignedSize;
				}
			}
		}
	}

	template <typename t_CParams>
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_ExtendSubSlabChain(CMemoryManagerSubSlabFreeState &_FreeState, uint8 *_pBase, umint _AlignedSize)
	{
		umint CarveOffset = umint(_FreeState.m_CarveOffset) * 4;
		umint CarveEnd = umint(_FreeState.m_CarveEnd) * 4;

		if (CarveOffset >= CarveEnd)
			return false;

		DMibFastCheck(_FreeState.m_pFreeHead == nullptr);

		// Thread up to one page worth of carved blocks into the chain in ascending address
		// order; batching keeps carving off the allocation fast path while bounding how much
		// fresh memory is touched ahead of use
		constexpr umint c_MaxExtendBytes = 4096;

		umint nBlocks = (CarveEnd - CarveOffset) / _AlignedSize;
		umint nMax = c_MaxExtendBytes / _AlignedSize;

		if (nMax == 0)
			nMax = 1;
		if (nBlocks > nMax)
			nBlocks = nMax;

		uint8 *pBlock = _pBase + CarveOffset;
		_FreeState.m_pFreeHead = pBlock;

		for (umint i = 1; i < nBlocks; ++i)
		{
			uint8 *pNext = pBlock + _AlignedSize;

			// The carved region is protected in whole, but chained blocks keep their link word
			// unprotected so the chain can be threaded and popped
			if constexpr (mc_EnableCallbacks)
				this->f_OnCheckFree(pBlock, sizeof(void *), EMemoryManagerCheckFlag_Default);

			*(void **)pBlock = pNext;
			pBlock = pNext;
		}

		if constexpr (mc_EnableCallbacks)
			this->f_OnCheckFree(pBlock, sizeof(void *), EMemoryManagerCheckFlag_Default);

		*(void **)pBlock = nullptr;
		_FreeState.m_CarveOffset = uint16((CarveOffset + nBlocks * _AlignedSize) / 4);

		return true;
	}

	template <typename t_CParams>
	inline_always typename TCMemoryManagerArena<t_CParams>::CNormalFreeStoreCurrent *TCMemoryManagerArena<t_CParams>::fp_GetCurrentSubSlabSlot(umint _SlabType, umint _SlabBucket, umint _AlignedSize)
	{
		if constexpr (!mc_bSubSlabStore)
			return nullptr; // No current sub-slab slots; never dereferenced in this mode
		else if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			if (_SlabBucket > 4) [[likely]]
				return &m_CurrentSubSlabs.m_Normal[fs_NormalSizeClass(_SlabBucket, _SlabType)];
			else
				return &m_CurrentSubSlabs.m_Level0[(_AlignedSize - sizeof(void *) * 2) / t_CParams::mc_MinNormalSizeAlignment];
		}
		else
			return &m_CurrentSubSlabs.m_Normal[fs_NormalSizeClass(_SlabBucket, _SlabType)];
	}

	template <typename t_CParams>
	inline_always typename TCMemoryManagerArena<t_CParams>::CNormalFreeStoreList *TCMemoryManagerArena<t_CParams>::fp_GetNormalFreeList(umint _SlabType, umint _SlabBucket, umint _AlignedSize)
	{
		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			if (_SlabBucket > 4) [[likely]]
				return &m_NormalSizeSlabs[fs_NormalSizeClass(_SlabBucket, _SlabType)];
			else
				return &m_NormalSizeSlabsLevel0[(_AlignedSize - sizeof(void *) * 2) / t_CParams::mc_MinNormalSizeAlignment];
		}
		else
			return &m_NormalSizeSlabs[fs_NormalSizeClass(_SlabBucket, _SlabType)];
	}

	template <typename t_CParams>
	inline_always constexpr umint TCMemoryManagerArena<t_CParams>::fs_NormalSizeClass(umint _SlabBucket, umint _SubIndex)
	{
		constexpr umint c_FirstBucket = mc_bUseSmallSizes ? 5 : t_CParams::mc_MinNormalSlabBucket;

		return ((_SlabBucket - c_FirstBucket) << t_CParams::mc_SizesPerLevelShift) + _SubIndex;
	}

	template <typename t_CParams>
	inline_always void TCMemoryManagerArena<t_CParams>::fsp_GetCarveRange
		(
			uint8 *_pSubSlabStart
			, umint _AlignedSize
			, umint _SlabType
			, umint _SlabBucket
			, umint &o_StartOffset
			, umint &o_EndOffset
		)
	{
		uint32 nBlocks = fg_Max(t_CParams::mc_NumAllocsPerSubSlab[_SlabType] >> (_SlabBucket - t_CParams::mc_MinNormalSlabBucket), 1);
		DMibFastCheck(_AlignedSize <= t_CParams::mc_SlabTypeInfo[_SlabType].m_SubSlabMutiplier * t_CParams::mc_SubSlabSize);

		umint StartOffset = 0;
		umint nFree = nBlocks - 1;

		if constexpr (t_CParams::mc_PreventCacheConflictSize)
		{
			if
				(
					((umint)_pSubSlabStart & (umint)(t_CParams::mc_PreventCacheConflictSize - 1)) == 0
					&& _AlignedSize <= t_CParams::mc_PreventCacheConflictSizeMaxBlockSize
				)
			{
				smint ToRemove = DMibPMemoryCacheLineSize;
				while (ToRemove > 0 && nFree > 1)
				{
					StartOffset += _AlignedSize;
					ToRemove -= _AlignedSize;
					--nFree;
				}
			}
		}

		o_StartOffset = StartOffset;
		o_EndOffset = StartOffset + (nFree + 1) * _AlignedSize;
	}

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
		CNormalFreeStoreList *pList;
		umint Size;
		umint SizeMinus1;
		{
			// The minimum-size clamp and the minimum alignment both fold into one OR on
			// size-minus-one, which the class split below consumes directly
			umint Clamped;
			if constexpr (mc_MinAllocSize > 1)
				Clamped = fg_Max(_Size, mc_MinAllocSize);
			else
				Clamped = _Size;

			SizeMinus1 = (Clamped - 1) | (t_CParams::mc_MinNormalSizeAlignment - 1);
			Size = SizeMinus1 + 1;
		}

		DMibMemLightweightTrack(m_pMemoryManager->fp_TrackAlloc(Size));

		DMibFastCheck(Size >= t_CParams::mc_MinNormalAllocSize);

		umint AlignedSize;
		umint SubIndex;
		umint SlabBucket;
		{
			{
				// Closed-form size class: with the level split at bit e = highest bit of
				// size-minus-one, the top mc_SizesPerLevelShift+1 bits of size-minus-one are
				// (1 << shift) + sub-index - 1, and the round-up to the class size and the
				// carry from the last sub-index into the next level's index zero both fall
				// out of the same expression. The OR of the guard bit only matters for the
				// smallest class, where size-minus-one can sit below one full level.
				constexpr umint c_LevelShift = t_CParams::mc_SizesPerLevelShift;
				constexpr umint c_LevelMask = (umint(1) << c_LevelShift) - 1;

				umint TopBit = NMib::fg_GetHighestBitSetNoZero(SizeMinus1 | (umint(1) << c_LevelShift));
				umint ClassIndex = (TopBit << c_LevelShift) + (SizeMinus1 >> (TopBit - c_LevelShift)) - c_LevelMask;

				SlabBucket = ClassIndex >> c_LevelShift;
				SubIndex = ClassIndex & c_LevelMask;
				AlignedSize = ((umint(1) << c_LevelShift) + SubIndex) << (SlabBucket - c_LevelShift);

				// The class is correct exactly when it is the smallest one that fits the size
				DMibFastCheck(AlignedSize >= Size);
				DMibFastCheck(AlignedSize - Size < (umint(1) << (SlabBucket - c_LevelShift)));
				DMibFastCheck(SubIndex <= c_LevelMask);

				if constexpr (t_CParams::mc_bUseSmallSizes)
				{
					if (Size > mc_Level0SmallestSize) [[likely]]
					{
						DMibFastCheck(SlabBucket - 5 < mc_nNormalSizeLists);
						DMibFastCheck(SubIndex < t_CParams::mc_NumSizesPerLevel);

						pList = &m_NormalSizeSlabs[fs_NormalSizeClass(SlabBucket, SubIndex)];
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
					DMibFastCheck(SlabBucket - t_CParams::mc_MinNormalSlabBucket < mc_nNormalSizeLists);
					DMibFastCheck(SubIndex < t_CParams::mc_NumSizesPerLevel);

					pList = &m_NormalSizeSlabs[fs_NormalSizeClass(SlabBucket, SubIndex)];
				}
			}

			auto &List = *pList;

		l_Retry:
			if constexpr (mc_bSubSlabStore)
			{
				auto &Current = *fp_GetCurrentSubSlabSlot(SubIndex, SlabBucket, AlignedSize);

				if (Current.m_pFreeState == nullptr) [[unlikely]]
				{
					// f_Pop fully resets the node's link state; the node must not look linked
					// when its slab is later destroyed or reused
					auto pNode = List.f_Pop();

					if (pNode)
					{
						umint iSubSlab;
						auto pSlab = fsp_SlabFromSubSlabNode(pNode, iSubSlab);

						fp_SetCurrentSubSlab(&Current, pSlab, iSubSlab);
					}
					else if (fp_ProcessMessages(pList))
						goto l_Retry;
				}

				if (Current.m_pFreeState != nullptr) [[likely]]
				{
					uint8 *pAlloc;

					if constexpr (mc_bSubSlabBitmaps)
					{
						uint64 Word = *Current.m_pWord;

						if (Word == 0) [[unlikely]]
						{
							if (!fsp_BitmapAdvanceCurrentWord(Current, AlignedSize))
							{
								// Exhausted; pick the next sub-slab
								goto l_Retry;
							}

							Word = *Current.m_pWord;
							DMibFastCheck(Word != 0);
						}

						umint iBit = NMib::fg_GetLowestBitSetNoZero(Word);

						*Current.m_pWord = Word & (Word - 1);
						pAlloc = Current.m_pWordBase + iBit * AlignedSize;

						DMibFastCheck(pAlloc == Current.m_pBase + (Current.m_WordIndexBase + iBit) * AlignedSize);

						if constexpr (mc_EnableCallbacks)
							this->f_OnCheckFree(pAlloc, AlignedSize, EMemoryManagerCheckFlag_Default);
					}
					else
					{
						uint8 *pBase = Current.m_pBase;
						auto &FreeState = *Current.m_pFreeState;
						void *pFree = FreeState.m_pFreeHead;

						if (pFree == nullptr) [[unlikely]]
						{
							if (!fp_ExtendSubSlabChain(FreeState, pBase, AlignedSize))
							{
								// Exhausted; drop the cache and pick the next sub-slab
								Current.m_pFreeState = nullptr;
								goto l_Retry;
							}

							pFree = FreeState.m_pFreeHead;
						}

						pAlloc = (uint8 *)pFree;
						FreeState.m_pFreeHead = *(void **)pAlloc;
						if constexpr (mc_EnableCallbacks)
							this->f_OnCheckFree(pAlloc + sizeof(void *), AlignedSize - sizeof(void *), EMemoryManagerCheckFlag_Default);
					}

					DMibFastCheck(Current.m_pDataAlloc->m_nAllocs < TCMemoryManagerSubSlabDataAlloc<t_CParams>::mc_MaxAllocs);
					if (++Current.m_pDataAlloc->m_nAllocs == 1) [[unlikely]]
						fp_SubSlabNoLongerPending(Current.m_pSlab, Current.m_iSubSlab);

					if constexpr (mc_EnableCallbacks)
						this->f_OnAlloc(pAlloc, AlignedSize);
					_Size = AlignedSize;
					return pAlloc;
				}
			}
			else
			{
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
							static_assert(mc_bSubSlabLists || sizeof(*pAlloc) == sizeof(void *) * 2 + 4, "Should be packed");

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
		}

		auto pRet = fp_AllocNormalUncached(pList, fp_GetCurrentSubSlabSlot(SubIndex, SlabBucket, AlignedSize), AlignedSize, SubIndex, SlabBucket);
		_Size = AlignedSize;
		return pRet;
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_AllocNormalBatch(umint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor)
	{
		CNormalFreeStoreList *pList;
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
						DMibFastCheck(SlabBucket - 5 < mc_nNormalSizeLists);
						DMibFastCheck(SubIndex < t_CParams::mc_NumSizesPerLevel);

						pList = &m_NormalSizeSlabs[fs_NormalSizeClass(SlabBucket, SubIndex)];
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
					DMibFastCheck(SlabBucket - t_CParams::mc_MinNormalSlabBucket < mc_nNormalSizeLists);
					DMibFastCheck(SubIndex < t_CParams::mc_NumSizesPerLevel);

					pList = &m_NormalSizeSlabs[fs_NormalSizeClass(SlabBucket, SubIndex)];
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

				if constexpr (mc_bSubSlabStore)
				{
					auto &Current = *fp_GetCurrentSubSlabSlot(SubIndex, SlabBucket, AlignedSize);

					if (Current.m_pFreeState == nullptr) [[unlikely]]
					{
						// f_Pop fully resets the node's link state; the node must not look
						// linked when its slab is later destroyed or reused
						auto pNode = List.f_Pop();

						if (pNode)
						{
							umint iSubSlab;
							auto pSlab = fsp_SlabFromSubSlabNode(pNode, iSubSlab);

							fp_SetCurrentSubSlab(&Current, pSlab, iSubSlab);
						}
						else
						{
							if (!_Functor(fp_AllocNormalUncached(pList, &Current, AlignedSize, SubIndex, SlabBucket), AlignedSize))
								break;

							continue;
						}
					}

					uint8 *pBase = Current.m_pBase;

					if constexpr (mc_bSubSlabBitmaps)
					{
						// Detach up to one full word of free blocks and stream them to the
						// functor with no per-block bitmap bookkeeping. The detached blocks are
						// committed as allocated up front, so functor reentry into the
						// allocator (nested allocations or frees, including into this
						// sub-slab) observes fully consistent state; unconsumed blocks on
						// early stop or unwind are given back through the normal free path.
						auto &FreeState = *Current.m_pFreeState;

						bool bHeaderBitmap = fsp_GetSubSlabBlocks(SubIndex, SlabBucket) <= 64;

						uint64 Word = *Current.m_pWord;
						umint IndexBase = Current.m_WordIndexBase;

						if (Word == 0) [[unlikely]]
						{
							if (!fsp_BitmapAdvanceCurrentWord(Current, AlignedSize))
								continue;

							Word = *Current.m_pWord;
							IndexBase = Current.m_WordIndexBase;
						}

						DMibFastCheck(Word != 0);

						// Detaching empties the cursor word; clear its summary bit by index
						// (under the cursor it is not necessarily the lowest set bit)
						*Current.m_pWord = 0;
						if (!bHeaderBitmap)
							FreeState.m_Bits &= ~(uint64(1) << (IndexBase >> 6));

						umint nDetached = fg_NumBitsSet(Word);

						auto &DataAlloc = *Current.m_pDataAlloc;
						umint nAllocsBefore = DataAlloc.m_nAllocs;

						DMibFastCheck(nAllocsBefore + nDetached <= TCMemoryManagerSubSlabDataAlloc<t_CParams>::mc_MaxAllocs);
						DataAlloc.m_nAllocs = (typename TCMemoryManagerSubSlabDataAlloc<t_CParams>::CStorageType)(nAllocsBefore + nDetached);
						if (nAllocsBefore == 0) [[unlikely]]
							fp_SubSlabNoLongerPending(Current.m_pSlab, Current.m_iSubSlab);

						auto pSlab = Current.m_pSlab;
						auto iSubSlab = Current.m_iSubSlab;

						if (FreeState.m_Bits == 0)
							Current.m_pFreeState = nullptr;

						// Unconsumed detached blocks are given back in bulk on early stop or
						// unwind: the remaining word is OR'd back and the counters and list
						// membership restored, mirroring fp_FreeInline's bitmap tail once
						// instead of per block. The blocks were never reported to the
						// notifier, so no callbacks are involved. Running as a scope guard
						// covers the functor unwinding, which would otherwise leave the
						// detached blocks committed as allocated forever
						auto GiveBack = g_OnScopeExit / [&]
							{
								if (!Word)
									return;

								umint nBack = fg_NumBitsSet(Word);

								// The functor may have reentered the allocator, so the sub-slab
								// state is re-derived here: exhausted means no free bits and
								// therefore neither current nor listed
								bool bWasExhausted = FreeState.m_Bits == 0;

								if (bHeaderBitmap)
									FreeState.m_Bits |= Word;
								else
								{
									((uint64 *)pBase)[IndexBase / 64] |= Word;
									FreeState.m_Bits |= uint64(1) << (IndexBase / 64);
								}

								if (bWasExhausted)
								{
									// A reentered free with a nested allocation can have recached
									// this sub-slab as current; a current sub-slab must not also
									// be listed
									if (Current.m_pFreeState != &FreeState)
									{
										if constexpr (t_CParams::mc_bGlobalAddressOrder)
											m_bSubSlabListsDirty = true;
										List.f_UnsafeInsertFirst(&pSlab->m_pSubSlabNodes[iSubSlab]);
									}
								}

								DMibFastCheck(DataAlloc.m_nAllocs >= nBack);
								DataAlloc.m_nAllocs = (typename TCMemoryManagerSubSlabDataAlloc<t_CParams>::CStorageType)(DataAlloc.m_nAllocs - nBack);

								if (DataAlloc.m_nAllocs == 0) [[unlikely]]
								{
									if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_NoCleanup) != 0)
									{
									}
									else if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_Allocs) != 0)
									{
										if constexpr (t_CParams::mc_MaxPendingSubSlabs)
										{
											if (pSlab->m_nPendingSubSlabs >= t_CParams::mc_MaxPendingSubSlabs)
											{
												(void)fp_FreeSubSlab(pSlab, iSubSlab);
												return;
											}
										}

										pSlab->f_SetPendingBit(iSubSlab);

										fp_SlabHasGarbage(pSlab);

										++pSlab->m_nPendingSubSlabs;
										DMibFastCheck(pSlab->f_HasPendingBit());
									}
									else
										(void)fp_FreeSubSlab(pSlab, iSubSlab);
								}
							}
						;

						while (1)
						{
							umint Index = IndexBase + NMib::fg_GetLowestBitSetNoZero(Word);
							Word &= Word - 1;

							uint8 *pStreamAlloc = pBase + Index * AlignedSize;

							if constexpr (mc_EnableCallbacks)
							{
								this->f_OnCheckFree(pStreamAlloc, AlignedSize, EMemoryManagerCheckFlag_Default);
								this->f_OnAlloc(pStreamAlloc, AlignedSize);
							}

							if (!_Functor(pStreamAlloc, AlignedSize))
								return;

							if (!Word)
								break;

							DMibMemLightweightTrack
								(
									{
										if (TCMemoryManager<t_CParams>::fsp_ShouldTrackAlloc(pLocalArena))
											pLocalArena->f_TrackAlloc(AlignedSize);
									}
								)
							;
						}

						continue;
					}
					else
					{
						auto &FreeState = *Current.m_pFreeState;
						void *pFree = FreeState.m_pFreeHead;

						if (pFree == nullptr) [[unlikely]]
						{
							if (!fp_ExtendSubSlabChain(FreeState, pBase, AlignedSize))
							{
								// Exhausted; drop the cache and pick the next sub-slab
								Current.m_pFreeState = nullptr;
								continue;
							}

							pFree = FreeState.m_pFreeHead;
						}

						uint8 *pAlloc = (uint8 *)pFree;
						FreeState.m_pFreeHead = *(void **)pAlloc;
						if constexpr (mc_EnableCallbacks)
							this->f_OnCheckFree(pAlloc + sizeof(void *), AlignedSize - sizeof(void *), EMemoryManagerCheckFlag_Default);

						DMibFastCheck(Current.m_pDataAlloc->m_nAllocs < TCMemoryManagerSubSlabDataAlloc<t_CParams>::mc_MaxAllocs);
						if (++Current.m_pDataAlloc->m_nAllocs == 1)
							fp_SubSlabNoLongerPending(Current.m_pSlab, Current.m_iSubSlab);

						if constexpr (mc_EnableCallbacks)
							this->f_OnAlloc(pAlloc, AlignedSize);
						if (!_Functor(pAlloc, AlignedSize))
							break;

						continue;
					}
				}
				else
				{
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
						if (!_Functor(fp_AllocNormalUncached(pList, fp_GetCurrentSubSlabSlot(SubIndex, SlabBucket, AlignedSize), AlignedSize, SubIndex, SlabBucket), AlignedSize))
							break;
					}
				}
			}
		}
	}

	template <typename t_CParams>
	inline_never void *TCMemoryManagerArena<t_CParams>::fp_AllocNormalUncached(CNormalFreeStoreList *_pList, CNormalFreeStoreCurrent *_pCurrent, umint _AlignedSize, umint _SubIndex, umint _SlabBucket)
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

		if constexpr (mc_bSubSlabStore)
		{
			auto &FreeState = pSlab->m_pSubSlabFreeState[iAlloc];

			if (_AlignedSize > MinSize)
			{
				// Multi-sub-slab single block; the free store is never used and the block is
				// reclaimed through the pending path when freed
				if constexpr (mc_bSubSlabBitmaps)
					FreeState = {0};
				else
					FreeState = {nullptr, 0, 0, 0};
			}
			else if constexpr (mc_bSubSlabBitmaps)
			{
				umint nBlocks = fsp_GetSubSlabBlocks(_SubIndex, _SlabBucket);

				// Reserve leading blocks for the in-band bitmap words of dense sub-slabs and
				// for cache-conflict avoidance; the first returned block follows the reserve
				umint nReserved = 0;
				if (nBlocks > 64)
				{
					umint nWords = (nBlocks + 63) / 64;
					if constexpr (t_CParams::mc_bReapInCleanup && t_CParams::mc_bReapDenseBitmaps)
					{
						// Dense pending words follow the free words in-band; zero them here (the
						// carve owns the sub-slab, no remote free can be in flight for it yet)
						nReserved = (fsp_BitmapRemoteWordsOffset(nWords) + nWords * sizeof(uint64) + _AlignedSize - 1) / _AlignedSize;

						uint64 *pPendingWords = (uint64 *)(pSlabAddress + fsp_BitmapRemoteWordsOffset(nWords));
						for (umint iWord = 0; iWord < nWords; ++iWord)
							pPendingWords[iWord] = 0;
					}
					else
						nReserved = (nWords * sizeof(uint64) + _AlignedSize - 1) / _AlignedSize;
				}

				if constexpr (t_CParams::mc_PreventCacheConflictSize)
				{
					if
						(
							((umint)pSlabAddress & (umint)(t_CParams::mc_PreventCacheConflictSize - 1)) == 0
							&& _AlignedSize <= t_CParams::mc_PreventCacheConflictSizeMaxBlockSize
						)
					{
						smint ToRemove = smint(DMibPMemoryCacheLineSize) - smint(nReserved * _AlignedSize);
						while (ToRemove > 0 && nBlocks - nReserved > 2)
						{
							++nReserved;
							ToRemove -= _AlignedSize;
						}
					}
				}

				umint FirstFree = nReserved + 1;
				if (nBlocks <= 64)
				{
					uint64 All = nBlocks == 64 ? ~uint64(0) : ((uint64(1) << nBlocks) - 1);
					FreeState.m_Bits = FirstFree >= 64 ? 0 : (All & ~((uint64(1) << FirstFree) - 1));
				}
				else
				{
					uint64 *pWords = (uint64 *)pSlabAddress;
					umint nWords = (nBlocks + 63) / 64;
					uint64 Summary = 0;

					for (umint iWord = 0; iWord < nWords; ++iWord)
					{
						uint64 Word = ~uint64(0);
						umint WordBase = iWord * 64;

						if (WordBase + 64 > nBlocks)
							Word &= (uint64(1) << (nBlocks - WordBase)) - 1;
						if (WordBase < FirstFree)
						{
							umint nLowClear = FirstFree - WordBase;
							Word &= nLowClear >= 64 ? 0 : ~((uint64(1) << nLowClear) - 1);
						}

						pWords[iWord] = Word;
						if (Word)
							Summary |= uint64(1) << iWord;
					}

					FreeState.m_Bits = Summary;
				}

				if (FreeState.m_Bits != 0)
				{
					// This sub-slab becomes the current one for its size class; it only enters
					// the list when it regains space after exhaustion
					DMibFastCheck(_pCurrent->m_pFreeState == nullptr);
					fp_SetCurrentSubSlab(_pCurrent, pSlab, iAlloc);

					if constexpr (mc_EnableCallbacks)
						this->f_OnFillFree(pSlabAddress + FirstFree * _AlignedSize, (nBlocks - FirstFree) * _AlignedSize);
				}

				pSlabAddress += nReserved * _AlignedSize;
			}
			else
			{
				umint StartOffset;
				umint EndOffset;
				fsp_GetCarveRange(pSlabAddress, _AlignedSize, _SubIndex, _SlabBucket, StartOffset, EndOffset);

				FreeState.m_pFreeHead = nullptr;
				FreeState.m_CarveStart = uint16(StartOffset / 4);
				FreeState.m_CarveOffset = uint16((StartOffset + _AlignedSize) / 4);
				FreeState.m_CarveEnd = uint16(EndOffset / 4);

				if (EndOffset - StartOffset > _AlignedSize)
				{
					// This sub-slab becomes the current one for its size class; it only enters
					// the list when it regains space after exhaustion
					DMibFastCheck(_pCurrent->m_pFreeState == nullptr);
					fp_SetCurrentSubSlab(_pCurrent, pSlab, iAlloc);

					if constexpr (mc_EnableCallbacks)
						this->f_OnFillFree(pSlabAddress + StartOffset + _AlignedSize, EndOffset - StartOffset - _AlignedSize);
				}

				pSlabAddress += StartOffset;
			}

			if constexpr (mc_EnableCallbacks)
				this->f_OnAlloc((uint8 *)pSlabAddress, _AlignedSize);

			return pSlabAddress;
		}
		else
		{
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
	}


	template <typename t_CParams>
	inline_always void TCMemoryManagerArena<t_CParams>::fp_FreeInline(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab)
	{
		uint8 *pSlabStart = _pSlab->f_GetSlabStart();
		auto SubSlab = umint((uint8 *)_pMemory - pSlabStart) / t_CParams::mc_SubSlabSize;

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

		// The size-class list is only needed when the sub-slab has to be relisted or the block is
		// pushed on an arena-wide list, both below, so only the block size is derived here
		umint AlignedSize = (umint(1) << SlabBucket) + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

		DMibFastCheck(t_CParams::mc_bUseSmallSizes || SlabBucket >= t_CParams::mc_MinNormalSlabBucket);
		DMibFastCheck(SlabType < t_CParams::mc_NumSizesPerLevel);
		DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());

		// The notifier sees the free first so a layered debug manager can report double frees
		// before the allocator's own consistency checks trip on them
		if constexpr (mc_EnableCallbacks)
			this->f_OnFree((uint8 *)_pMemory);

		auto pDataAlloc = _pSlab->f_GetSubSlabDataAlloc();

		DMibFastCheck(pDataAlloc[iSubSlab].m_nAllocs > 0);

		if constexpr (mc_bSubSlabStore)
		{

			umint MinSize = t_CParams::mc_SlabTypeFast[SlabType].m_SubSlabMultiplier * t_CParams::mc_SubSlabSize;

			if (AlignedSize <= MinSize) [[likely]]
			{
				if constexpr (mc_bSubSlabBitmaps)
				{
					uint8 *pBase = pSlabStart + iSubSlab * MinSize;

					bool bWasExhausted = fsp_BitmapFreeBlock
						(
							_pSlab->m_pSubSlabFreeState[iSubSlab]
							, pBase
							, _pMemory
							, AlignedSize
							, SlabType
							, SlabBucket
							, fsp_GetSubSlabBlocks(SlabType, SlabBucket)
						)
					;

					if constexpr (mc_EnableCallbacks)
						this->f_OnFillFree((uint8 *)_pMemory, AlignedSize);

					if (bWasExhausted) [[unlikely]]
						fp_FreeRelistSubSlab(_pSlab, iSubSlab, SlabType, SlabBucket, AlignedSize);
				}
				else
				{
					auto &FreeState = _pSlab->m_pSubSlabFreeState[iSubSlab];

					bool bWasExhausted
						= FreeState.m_pFreeHead == nullptr
						&& FreeState.m_CarveOffset >= FreeState.m_CarveEnd
					;

					*(void **)_pMemory = FreeState.m_pFreeHead;
					FreeState.m_pFreeHead = _pMemory;

					if constexpr (mc_EnableCallbacks)
						this->f_OnFillFree((uint8 *)_pMemory + sizeof(void *), AlignedSize - sizeof(void *));

					if (bWasExhausted) [[unlikely]]
					{
						// Single-block sub-slabs are never listed; they are reclaimed through the
						// pending path below. A drained sub-slab can also still be cached as
						// current (exhaustion is only discovered by the next allocation); the
						// pushed block is reachable through the head, so it must not be listed
						// as well
						bool bCurrent = fp_GetCurrentSubSlabSlot(SlabType, SlabBucket, AlignedSize)->m_pFreeState == &FreeState;

						if (!bCurrent && umint(FreeState.m_CarveEnd - FreeState.m_CarveStart) * 4 > AlignedSize)
						{
							if constexpr (t_CParams::mc_bGlobalAddressOrder)
								m_bSubSlabListsDirty = true;

							// Always append at the tail: a relisted sub-slab then accumulates
							// frees while queued behind the others, so it is popped with a long
							// chain and the current-sub-slab switch cost amortizes over many
							// allocations
							fp_GetNormalFreeList(SlabType, SlabBucket, AlignedSize)->f_UnsafeInsertLast(&_pSlab->m_pSubSlabNodes[iSubSlab]);
						}
					}
				}
			}
			else if constexpr (mc_EnableCallbacks)
				this->f_OnFillFree((uint8 *)_pMemory, AlignedSize);
		}
		else
		{
			CMemoryManagerSubSlab_NormalLink *pFreeMemory = (CMemoryManagerSubSlab_NormalLink *)_pMemory;

			if constexpr (mc_EnableCallbacks)
				this->f_OnFillFree((uint8 *)(pFreeMemory + 1), AlignedSize - sizeof(*pFreeMemory));

			if constexpr (t_CParams::mc_bUseFreeBlockCounting)
				pFreeMemory->m_nBlocks = 1;

			auto *pList = fp_GetNormalFreeList(SlabType, SlabBucket, AlignedSize);

			pList->f_UnsafeInsertFirst(pFreeMemory);
		}

		if ((--pDataAlloc[iSubSlab].m_nAllocs) == 0) [[unlikely]]
			fp_FreeSubSlabWentEmpty(_pSlab, uint32(iSubSlab));
	}

	// The tail of a free that emptied its sub-slab. Kept out of line so the free entry point stays
	// a compact hot path: inlining the pending-bit array, the garbage list and fp_FreeSubSlab into
	// it cost register pressure and instruction cache lines on every free.
	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_FreeSubSlabWentEmpty(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab)
	{
		if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_NoCleanup) != 0)
		{
		}
		else if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_Allocs) != 0)
		{
			if constexpr (t_CParams::mc_MaxPendingSubSlabs)
			{
				if (_pSlab->m_nPendingSubSlabs >= t_CParams::mc_MaxPendingSubSlabs)
				{
					(void)fp_FreeSubSlab(_pSlab, _iSubSlab);
					return;
				}
			}

			_pSlab->f_SetPendingBit(_iSubSlab);

			fp_SlabHasGarbage(_pSlab);

			++_pSlab->m_nPendingSubSlabs;
			DMibFastCheck(_pSlab->f_HasPendingBit());
		}
		else
			(void)fp_FreeSubSlab(_pSlab, _iSubSlab); // _pSlab will potentially be invalid after this call
	}

	// A sub-slab that was exhausted has free space again, so it goes back on its size-class list.
	// Out of line for the same reason: this is the only thing on the free path that needs the
	// size-class list and the current-sub-slab slot.
	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_FreeRelistSubSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab, umint _iSubSlab, umint _SlabType, umint _SlabBucket, umint _AlignedSize)
	{
		// A drained sub-slab can still be cached as current (exhaustion is only discovered by the
		// next allocation); the freed bit is reachable through the cursor, so it must not be
		// listed as well. Append at the tail: a relisted sub-slab then accumulates frees while
		// queued behind the others, so it is popped with many free bits and the current-sub-slab
		// switch cost amortizes over many allocations
		if (fp_GetCurrentSubSlabSlot(_SlabType, _SlabBucket, _AlignedSize)->m_pFreeState == &_pSlab->m_pSubSlabFreeState[_iSubSlab])
			return;

		if constexpr (t_CParams::mc_bGlobalAddressOrder)
			m_bSubSlabListsDirty = true;

		fp_GetNormalFreeList(_SlabType, _SlabBucket, _AlignedSize)->f_UnsafeInsertLast(&_pSlab->m_pSubSlabNodes[_iSubSlab]);
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::fp_CheckFreeSubSlabChain(TCMemoryManagerSlabShared<t_CParams> *_pSlab, umint _iSubSlab, EMemoryManagerCheckFlag _Flags)
	{
		bool bError = false;

		if constexpr (mc_EnableCallbacks && mc_bSubSlabStore)
		{
			auto pSlab = _pSlab;
			umint iSubSlab = _iSubSlab;

			umint SlabType = pSlab->m_SlabType;
			umint SlabBucket = pSlab->f_GetSubSlabDataType()[iSubSlab].m_Type - 2;
			umint SlabBucketStart = umint(1) << SlabBucket;
			umint AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

			uint8 *pBase = pSlab->f_GetSlabStart() + iSubSlab * (t_CParams::mc_SlabTypeInfo[SlabType].m_SubSlabMutiplier * t_CParams::mc_SubSlabSize);
			auto &FreeState = pSlab->m_pSubSlabFreeState[iSubSlab];

			if constexpr (mc_bSubSlabBitmaps)
			{
				umint nBlocks = fsp_GetSubSlabBlocks(SlabType, SlabBucket);

				auto fCheckBlock = [&](umint _Index)
					{
						if (this->f_OnCheckFree(pBase + _Index * AlignedSize, AlignedSize, _Flags))
							bError = true;
					}
				;

				if (nBlocks <= 64)
				{
					for (uint64 Bits = FreeState.m_Bits; Bits; Bits &= Bits - 1)
						fCheckBlock(NMib::fg_GetLowestBitSetNoZero(Bits));
				}
				else
				{
					uint64 *pWords = (uint64 *)pBase;
					for (uint64 Summary = FreeState.m_Bits; Summary; Summary &= Summary - 1)
					{
						umint iWord = NMib::fg_GetLowestBitSetNoZero(Summary);
						for (uint64 Bits = pWords[iWord]; Bits; Bits &= Bits - 1)
							fCheckBlock(iWord * 64 + NMib::fg_GetLowestBitSetNoZero(Bits));
					}
				}
			}
			else
			{
				for (void *pFree = FreeState.m_pFreeHead; pFree != nullptr; )
				{
					uint8 *pBlock = (uint8 *)pFree;
					pFree = *(void **)pBlock;

					if (this->f_OnCheckFree(pBlock + sizeof(void *), AlignedSize - sizeof(void *), _Flags))
						bError = true;
				}

				if (FreeState.m_CarveOffset < FreeState.m_CarveEnd)
				{
					if (this->f_OnCheckFree(pBase + umint(FreeState.m_CarveOffset) * 4, umint(FreeState.m_CarveEnd - FreeState.m_CarveOffset) * 4, _Flags))
						bError = true;
				}
			}
		}

		return bError;
	}
}
