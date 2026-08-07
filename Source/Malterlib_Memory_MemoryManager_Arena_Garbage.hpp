// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::fsp_MergeSubSlabLists(CNormalFreeStoreList &_ListA, CNormalFreeStoreList &_ListB, CNormalFreeStoreList &o_Out)
	{
		if constexpr (mc_bSubSlabStore)
		{
			while (!_ListA.f_IsEmpty() && !_ListB.f_IsEmpty())
			{
				if ((void *)_ListA.f_GetFirst() < (void *)_ListB.f_GetFirst())
					o_Out.f_UnsafeInsertLast(_ListA.f_Pop());
				else
					o_Out.f_UnsafeInsertLast(_ListB.f_Pop());
			}

			o_Out.f_Insert(_ListA);
			o_Out.f_Insert(_ListB);
		}
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::fsp_SortSubSlabList(CNormalFreeStoreList &_List)
	{
		if constexpr (mc_bSubSlabStore)
		{
			// Bottom-up merge sort by node address. Node order equals sub-slab address order
			// within a slab (nodes are a header array indexed by sub-slab) and slab address
			// order across slabs (headers live at the end of their slab).
			constexpr umint c_nLevels = 40;
			CNormalFreeStoreList Runs[c_nLevels];
			umint UsedLevels = 0;

			while (auto pNode = _List.f_Pop())
			{
				CNormalFreeStoreList Carry;
				Carry.f_UnsafeInsertLast(pNode);

				umint iLevel = 0;
				while (UsedLevels & (umint(1) << iLevel))
				{
					CNormalFreeStoreList Merged;
					fsp_MergeSubSlabLists(Runs[iLevel], Carry, Merged);
					Carry.f_Transfer(Merged);
					UsedLevels &= ~(umint(1) << iLevel);
					++iLevel;
				}

				DMibFastCheck(iLevel < c_nLevels);
				Runs[iLevel].f_Transfer(Carry);
				UsedLevels |= umint(1) << iLevel;
			}

			CNormalFreeStoreList Result;
			while (UsedLevels)
			{
				umint iLevel = NMib::fg_GetLowestBitSetNoZero(UsedLevels);
				UsedLevels &= UsedLevels - 1;

				CNormalFreeStoreList Merged;
				fsp_MergeSubSlabLists(Result, Runs[iLevel], Merged);
				Result.f_Transfer(Merged);
			}

			_List.f_Transfer(Result);
		}
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::fp_SortSubSlabListsIfDirty()
	{
		if constexpr (mc_bSubSlabStore && t_CParams::mc_bGlobalAddressOrder)
		{
			if (!m_bSubSlabListsDirty)
				return;

			m_bSubSlabListsDirty = false;

			if constexpr (t_CParams::mc_bUseSmallSizes)
			{
				for (umint i = 0; i < mc_nLevel0Lists; ++i)
					fsp_SortSubSlabList(m_NormalSizeSlabsLevel0[i]);
			}
			for (umint i = 0; i < mc_nNormalSizeClasses; ++i)
				fsp_SortSubSlabList(m_NormalSizeSlabs[i]);
		}
	}

	template <typename t_CParams>
	int64 TCMemoryManagerArena<t_CParams>::f_GarbageCollect(ENumaArenaCleanup &_oCleanup, int64 _Timestamp, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena)
	{
		int64 NextTimestamp = TCLimitsInt<int64>::mc_Max;

		fp_SortSubSlabListsIfDirty();

		for (auto iSlabType = 0; iSlabType < t_CParams::mc_NumSizesPerLevel; ++iSlabType)
		{
			NextTimestamp = fg_Min(NextTimestamp, fp_GarbageCollectPerform(iSlabType, _Timestamp, _pLocalArena));

			if (_Timestamp != (TCLimitsInt<int64>::mc_Max - 1) && f_IsContended(_pLocalArena))
				return 0;
		}

		auto pNumaArena = m_pNumaArena;

		for (auto iFreeSlab = m_FreeSlabs.f_GetIterator(); iFreeSlab; )
		{
			auto pFreeSlab = &*iFreeSlab;
			++iFreeSlab;

#if 0 // Rather that these free arenas should end up in shared storage
			if (pFreeSlab->m_FreeTimestamp > _Timestamp)
			{
				NextTimestamp = fg_Min(NextTimestamp, pFreeSlab->m_FreeTimestamp);
				continue;
			}
#endif

			DMibLock(pNumaArena->m_FreeSlabsLock);

			bool bWasEmpty = pNumaArena->m_FreeSlabs.f_IsEmpty();
			pFreeSlab->m_pArena = nullptr;
			if constexpr (t_CParams::mc_bBackgroundCleanup)
				DMibFastCheck(pFreeSlab->m_FreeTimestamp != 0);
			pNumaArena->m_FreeSlabs.f_Insert(pFreeSlab);
			bool bPendingDecommit = pFreeSlab->m_LinkNeedDecommit.f_IsInList();
			if (bPendingDecommit)
				pNumaArena->m_FreeSlabsNeedingDecommit.f_Insert(pFreeSlab);
			if (bPendingDecommit || !bWasEmpty)
				_oCleanup |= ENumaArenaCleanup_FreeSlabs;
		}

		return NextTimestamp;
	}

	template <typename t_CParams>
	int64 TCMemoryManagerArena<t_CParams>::f_DecommitDeferred(int64 _Timestamp)
	{
		int64 NextTimestamp = TCLimitsInt<int64>::mc_Max;

		if constexpr (!(t_CParams::mc_DeferCleanup & EDeferCleanup_Commit))
			return NextTimestamp;

		for (auto iSlab = m_SlabsNeedingDecommit.f_GetIterator(); iSlab; )
		{
			auto pSlab = &*iSlab;
			++iSlab;

			if (pSlab->m_NeedDecommitTimestamp <= _Timestamp)
				pSlab->f_DecommitDeferred();
			else
				NextTimestamp = fg_Min(NextTimestamp, pSlab->m_NeedDecommitTimestamp);
		}

		return NextTimestamp;
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::fp_GarbageCollectPerform(umint _SlabType)
	{
		DMibFastCheck(_SlabType < t_CParams::mc_NumSizesPerLevel);

		while (auto pSlab = m_SlabsToGarbageCollect[_SlabType].f_GetFirst())
		{
			bool bAborted = false;
			pSlab->f_EnumPendingBits
				(
					[&](umint _Bit) -> bool
					{
						if (fp_FreeSubSlab(pSlab, _Bit))
						{
							bAborted = true;
							return false;
						}

						fp_SubSlabNoLongerPending(pSlab, _Bit);
						return true;
					}
				)
			;

			if (bAborted)
				continue;

			if constexpr (t_CParams::mc_bUseSmallSizes)
			{
				auto pNewFirst = m_SlabsToGarbageCollect[_SlabType].f_GetFirst();

				if (pNewFirst == pSlab)
					fp_FreeSmallSubSlabs(pSlab);

				DMibFastCheck(m_SlabsToGarbageCollect[_SlabType].f_GetFirst() != pSlab);
			}

			return true;
		}

		return false;
	}

	template <typename t_CParams>
	int64 TCMemoryManagerArena<t_CParams>::fp_GarbageCollectPerform(umint _SlabType, int64 _Timestamp, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena)
	{
		int64 NextTimestamp = TCLimitsInt<int64>::mc_Max;
		DMibFastCheck(_SlabType < t_CParams::mc_NumSizesPerLevel);

		for (auto iSlab = m_SlabsToGarbageCollect[_SlabType].f_GetIterator(); iSlab; )
		{
			auto pSlab = &*iSlab;
			++iSlab;

			if (pSlab->m_HasGarbageTimestamp > _Timestamp)
			{
				NextTimestamp = fg_Min(NextTimestamp, pSlab->m_HasGarbageTimestamp);
				continue;
			}

			struct CParams
			{
				TCMemoryManagerSlabShared<t_CParams> *m_pSlab;
				TCMemoryManagerThreadLocal<t_CParams> *m_pLocalArena;
				int64 m_Timestamp;
				bool m_bAborted = false;
			};

			CParams Params;
			Params.m_pSlab = pSlab;
			Params.m_pLocalArena = _pLocalArena;
			Params.m_Timestamp = _Timestamp;

			pSlab->f_EnumPendingBits
				(
					[this, &Params](umint _Bit) -> bool
					{
						if (fp_FreeSubSlab(Params.m_pSlab, _Bit))
						{
							Params.m_bAborted = true;
							return false;
						}

						fp_SubSlabNoLongerPending(Params.m_pSlab, _Bit);

						if (Params.m_Timestamp != (TCLimitsInt<int64>::mc_Max - 1) && f_IsContended(Params.m_pLocalArena))
						{
							Params.m_bAborted = true;
							return false;
						}

						return true;
					}
				)
			;

			if (_Timestamp != (TCLimitsInt<int64>::mc_Max - 1) && f_IsContended(_pLocalArena))
				return 0;

			if (Params.m_bAborted)
				continue;

			if constexpr (t_CParams::mc_bUseSmallSizes)
			{
				if (pSlab->m_LinkToGarbageCollect.f_IsInList())
				{
					if (fp_FreeSmallSubSlabs(pSlab))
						continue;
				}
			}

			DMibFastCheck(!pSlab->m_LinkToGarbageCollect.f_IsInList());
		}

		return NextTimestamp;
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::fp_GarbageCollectFull(bool _bRetain)
	{
		fp_CheckMessages();
		fp_SortSubSlabListsIfDirty();

		// mc_nOwnerReclaimRetainSubSlabs is a compile-time config, so a zero retain count
		// removes the release hysteresis from the pending-bit walk entirely
		constexpr bool c_bRetainSubSlabs = t_CParams::mc_nOwnerReclaimRetainSubSlabs != 0;

		for (umint i = 0; i < t_CParams::mc_NumSizesPerLevel; ++i)
		{
			[[maybe_unused]] umint nRetain = 0;
			if constexpr (c_bRetainSubSlabs)
				nRetain = _bRetain ? t_CParams::mc_nOwnerReclaimRetainSubSlabs : 0;

			for (auto iSlab = m_SlabsToGarbageCollect[i].f_GetIterator(); iSlab; )
			{
				auto pSlab = &*iSlab;
				++iSlab;

				struct CParams
				{
					TCMemoryManagerSlabShared<t_CParams> *m_pSlab;
					umint m_nRetain = 0;
					bool m_bAborted = false;
					bool m_bRetained = false;
				};

				CParams Params;
				Params.m_pSlab = pSlab;
				if constexpr (c_bRetainSubSlabs)
					Params.m_nRetain = nRetain;

				pSlab->f_EnumPendingBits
					(
						[&](umint _Bit) -> bool
						{
							if constexpr (c_bRetainSubSlabs)
							{
								if (Params.m_nRetain)
								{
									bool bReusable = true;
									if constexpr (mc_bSubSlabStore)
									{
										// Only sub-slabs in the free store benefit from
										// retention: single block allocations are neither
										// listed nor current and can only be reclaimed here
										if (!Params.m_pSlab->m_pSubSlabNodes[_Bit].m_Link.f_IsInList())
										{
											auto pData = Params.m_pSlab->f_GetSubSlabDataType();
											umint SlabType = Params.m_pSlab->m_SlabType;
											umint SlabBucket = pData[_Bit].m_Type - 2;
											umint AlignedSize = (umint(1) << SlabBucket) + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

											bReusable
												= fp_GetCurrentSubSlabSlot(SlabType, SlabBucket, AlignedSize)->m_pFreeState
												== &Params.m_pSlab->m_pSubSlabFreeState[_Bit]
											;
										}
									}

									if (bReusable)
									{
										// Release hysteresis: keep the lowest pending sub-slabs
										// carved and listed for instant reuse instead of paying
										// the reclaim and re-carve cycle; enumeration order makes
										// the retained set the lowest addresses
										--Params.m_nRetain;
										Params.m_bRetained = true;
										return true;
									}
								}
							}

							if (fp_FreeSubSlab(Params.m_pSlab, _Bit))
							{
								Params.m_bAborted = true;
								return false;
							}

							fp_SubSlabNoLongerPending(Params.m_pSlab, _Bit);
							return true;
						}
					)
				;

				if constexpr (c_bRetainSubSlabs)
					nRetain = Params.m_nRetain;

				if (Params.m_bAborted)
					continue;

				if constexpr (c_bRetainSubSlabs)
				{
					if (Params.m_bRetained)
						continue;
				}

				if constexpr (t_CParams::mc_bUseSmallSizes)
				{
					if (pSlab->m_LinkToGarbageCollect.f_IsInList())
					{
						if (fp_FreeSmallSubSlabs(pSlab))
							continue;
					}
				}

				DMibFastCheck(!pSlab->m_LinkToGarbageCollect.f_IsInList());
			}
		}
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_ConsolidateSubSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab)
	{
		// Not implemented for block-counted free lists; those consolidate naturally through
		// their multi-block entries
		if constexpr (t_CParams::mc_bUseFreeBlockCounting)
			return;
		else if constexpr (mc_bSubSlabBitmaps)
		{
			// A fully free bitmap already hands out blocks in ascending address order
			return;
		}
		else if constexpr (mc_bSubSlabLists)
		{
			// All blocks are free (the sub-slab is pending); dropping the chain and rewinding
			// the carve cursor restarts allocation in ascending address order
			auto &FreeState = _pSlab->m_pSubSlabFreeState[_iSubSlab];

			DMibFastCheck(_pSlab->f_GetSubSlabDataAlloc()[_iSubSlab].m_nAllocs == 0);

			if (FreeState.m_CarveEnd == FreeState.m_CarveStart)
				return;

			FreeState.m_pFreeHead = nullptr;
			FreeState.m_CarveOffset = FreeState.m_CarveStart;

			if constexpr (mc_EnableCallbacks)
			{
				umint SlabType = _pSlab->m_SlabType;
				uint8 *pBase = _pSlab->f_GetSlabStart() + _iSubSlab * (t_CParams::mc_SlabTypeInfo[SlabType].m_SubSlabMutiplier * t_CParams::mc_SubSlabSize);
				this->f_OnFillFree(pBase + umint(FreeState.m_CarveStart) * 4, umint(FreeState.m_CarveEnd - FreeState.m_CarveStart) * 4);
			}
		}
		else
		{
			auto pData = _pSlab->f_GetSubSlabDataType();
			auto pSlabData = _pSlab->f_GetSlabStart();
			umint SlabType = _pSlab->m_SlabType;

			DMibFastCheck(_iSubSlab < _pSlab->f_GetNumSubSlabs());

			if constexpr (t_CParams::mc_bUseSmallSizes)
			{
				if (pData[_iSubSlab].m_Type < mc_nSmallSizeSlabs)
					return;
			}

			umint SlabBucket = pData[_iSubSlab].m_Type - 2;

			DMibFastCheck(SlabBucket < t_CParams::mc_NumSizeLevels);
			DMibFastCheck(_pSlab->f_GetSubSlabDataAlloc()[_iSubSlab].m_nAllocs == 0);

			umint SubSlabMultiplier = t_CParams::mc_SlabTypeInfo[SlabType].m_SubSlabMutiplier;
			umint MinSize = SubSlabMultiplier * t_CParams::mc_SubSlabSize;

			umint SlabBucketStart = umint(1) << SlabBucket;
			umint AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

			// A sub-slab holding a single block has nothing to consolidate
			if (AlignedSize * 2 > MinSize)
				return;

			auto pStart = pSlabData + _iSubSlab * t_CParams::mc_SubSlabSize * SubSlabMultiplier;
			auto pStartRemove = pStart;
			auto pEnd = pStart + t_CParams::mc_SubSlabSize * SubSlabMultiplier;

			if constexpr (t_CParams::mc_PreventCacheConflictSize)
			{
				uint32 nBlocks = fg_Max(t_CParams::mc_NumAllocsPerSubSlab[SlabType] >> (SlabBucket - t_CParams::mc_MinNormalSlabBucket), 1);

				if (((umint)pStartRemove & (umint)(t_CParams::mc_PreventCacheConflictSize - 1)) == 0 && AlignedSize <= t_CParams::mc_PreventCacheConflictSizeMaxBlockSize && nBlocks > 1)
				{
					--nBlocks;
					smint ToRemove = DMibPMemoryCacheLineSize;
					while (ToRemove > 0 && nBlocks > 1)
					{
						pStartRemove += AlignedSize;
						ToRemove -= AlignedSize;
						--nBlocks;
					}
				}
			}

			// Already consolidated from an earlier pass; the run's internal links stay intact
			// until it is consumed, and a consumed run means the sub-slab is no longer pending
			CMemoryManagerSubSlab_NormalLink *pFirstBlock = (CMemoryManagerSubSlab_NormalLink *)pStartRemove;
			if ((void const *)pFirstBlock->m_Link.f_GetNext() == (void const *)(pStartRemove + AlignedSize))
				return;

			CMemoryManagerSubSlab_NormalFreeList *pList;

			if constexpr (t_CParams::mc_bUseSmallSizes)
			{
				if (SlabBucket > 4) [[likely]]
				{
					DMibFastCheck(SlabBucket - 5 < mc_nNormalSizeLists);
					DMibFastCheck(SlabType < t_CParams::mc_NumSizesPerLevel);

					pList = &m_NormalSizeSlabs[fs_NormalSizeClass(SlabBucket, SlabType)];
				}
				else
				{
					umint InternalSlabBucketStart = sizeof(void *) * 2;
					umint SlabIndex = (AlignedSize - InternalSlabBucketStart) / t_CParams::mc_MinNormalSizeAlignment;

					DMibFastCheck(SlabIndex < mc_nLevel0Lists);

					pList = &m_NormalSizeSlabsLevel0[SlabIndex];
				}
			}
			else
			{
				DMibFastCheck(SlabBucket - t_CParams::mc_MinNormalSlabBucket < mc_nNormalSizeLists);
				DMibFastCheck(SlabType < t_CParams::mc_NumSizesPerLevel);

				pList = &m_NormalSizeSlabs[fs_NormalSizeClass(SlabBucket, SlabType)];
			}

			umint nBlocksInList = umint(pEnd - pStartRemove) / AlignedSize;

			for (umint iBlock = 0; iBlock < nBlocksInList; ++iBlock)
				((CMemoryManagerSubSlab_NormalLink *)(pStartRemove + iBlock * AlignedSize))->m_Link.f_UnsafeUnlink();

			// Reinsert as an ascending contiguous run at the head, matching the layout of a
			// freshly carved sub-slab, so allocations walk sequential addresses again
			for (umint iBlock = nBlocksInList; iBlock-- > 0; )
				pList->f_UnsafeInsertFirst((CMemoryManagerSubSlab_NormalLink *)(pStartRemove + iBlock * AlignedSize));
		}
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::fp_GarbageCollectConsolidate()
	{
		fp_CheckMessages();
		fp_SortSubSlabListsIfDirty();

		if constexpr (t_CParams::mc_bUseFreeBlockCounting || mc_bSubSlabBitmaps) // Does not support fp_ConsolidateSubSlab
			return;

		for (umint i = 0; i < t_CParams::mc_NumSizesPerLevel; ++i)
		{
			for (auto iSlab = m_SlabsToGarbageCollect[i].f_GetIterator(); iSlab; )
			{
				auto pSlab = &*iSlab;
				++iSlab;

				pSlab->f_EnumPendingBits
					(
						[&](umint _Bit) -> bool
						{
							fp_ConsolidateSubSlab(pSlab, _Bit);
							return true;
						}
					)
				;
			}
		}
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::fp_GarbageCollect(umint _SlabType)
	{
		fp_CheckMessages();

		if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_NoCleanup) != 0)
			return false;

		return fp_GarbageCollectPerform(_SlabType);
	}

	template <typename t_CParams>
	inline_always void TCMemoryManagerArena<t_CParams>::fp_SlabHasGarbageInline(TCMemoryManagerSlabShared<t_CParams> *_pSlab)
	{
		if (_pSlab->m_nPendingSubSlabs + _pSlab->m_nFreeSubSlabs == 0)
			m_SlabsToGarbageCollect[_pSlab->m_SlabType].f_Insert(_pSlab);

		if constexpr (t_CParams::mc_bBackgroundCleanup)
			_pSlab->m_HasGarbageTimestamp = TCMemoryManagerNumaArena<t_CParams>::fs_GetTimestamp();
		this->fp_RequestCleanup();
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_SlabHasGarbage(TCMemoryManagerSlabShared<t_CParams> *_pSlab)
	{
		return fp_SlabHasGarbageInline(_pSlab);
	}

	template <typename t_CParams>
	inline_always void TCMemoryManagerArena<t_CParams>::fp_CheckSlabNoLongerGarbage(TCMemoryManagerSlabShared<t_CParams> *_pSlab)
	{
		if (_pSlab->m_nPendingSubSlabs + _pSlab->m_nFreeSubSlabs <= 1)
			_pSlab->m_LinkToGarbageCollect.f_Unlink();
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_SubSlabNoLongerPending(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab)
	{
//			DMibFastCheck(_pSlab->f_GetNumPendingBits() == _pSlab->m_nPendingSubSlabs);
		bool bRemoved = _pSlab->f_ClearPendingBit(_iSubSlab);
		DMibFastCheck(bRemoved);
		if (bRemoved)
		{
			fp_CheckSlabNoLongerGarbage(_pSlab);
			--_pSlab->m_nPendingSubSlabs;
			DMibFastCheck(_pSlab->m_nPendingSubSlabs > 0 || !_pSlab->f_HasPendingBit());
		}
//			DMibFastCheck(_pSlab->f_GetNumPendingBits() == _pSlab->m_nPendingSubSlabs);
	}
}
