// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include "Malterlib_Memory_MemoryManager_Arena_Small.hpp"
#include "Malterlib_Memory_MemoryManager_Arena_Normal.hpp"
#include "Malterlib_Memory_MemoryManager_Arena_Slab.hpp"
#include "Malterlib_Memory_MemoryManager_Arena_Message.hpp"
#include "Malterlib_Memory_MemoryManager_Arena_Garbage.hpp"
#include "Malterlib_Memory_MemoryManager_Arena_Commit.hpp"

namespace NMib::NMemory
{
	template <typename t_CParams>
	TCMemoryManagerArena<t_CParams>::TCMemoryManagerArena
		(
			TCMemoryManager<t_CParams> *_pMemoryManager
			, uint64 _Magic
			, ENumaNode _NumaNode
			, TCMemoryManagerNumaArena<t_CParams> *_pNumaArena
			, bool _bLimitedArenas
		)
		: t_CParams::CNotifier::CArena(_pMemoryManager)
		, m_pMemoryManager(_pMemoryManager)
		, m_Magic(_Magic)
		, m_NumaNode(_NumaNode)
		, m_pNumaArena(_pNumaArena)
		, m_bLimitedArenas(_bLimitedArenas)
	{
		DMibFastCheck((uint8 *)&m_LockState.m_Lock == fg_AlignUp((uint8 *)&m_LockState.m_Lock, umint(DMibPMemoryCacheLineSize)));
		if constexpr (mc_bUseFreeMessages)
			DMibFastCheck((uint8 *)&m_MessageState.m_MessagesAvailable == fg_AlignUp((uint8 *)&m_MessageState.m_MessagesAvailable, umint(DMibPMemoryCacheLineSize)));
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::fp_RequestCleanup()
	{
		m_bWantCleanup = true;
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::f_IsContended(TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena) const
	{
		if (_pLocalArena->m_bForcingCleanup)
			return false;
		return m_LockState.m_LockContended.f_Load(NAtomic::gc_MemoryOrder_Relaxed) > 0;
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::f_ReturnCheckout()
	{
		DMibFastCheck(m_LockState.m_CheckoutCount.f_Load() > 0);
		if (m_LockState.m_CheckoutCount.f_FetchSub(1, NAtomic::gc_MemoryOrder_Relaxed) == 1)
		{
			bool bNeedCleanup = fp_CheckCleanup();
			m_LockState.m_Lock.f_UnlockNoSanitize();

			if (bNeedCleanup)
				m_pNumaArena->f_OnNeedCleanup();

			return true;
		}
		return false;
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::f_ForkedChild()
	{
		m_LockState.m_Lock.f_ForkedChildLocked();
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::f_ReturnCheckoutLight()
	{
		DMibFastCheck(m_LockState.m_CheckoutCount.f_Load() == 0); // Should not have been increased
		bool bNeedCleanup = false;
		if (m_bWantCleanup)
			bNeedCleanup = fp_CheckCleanup();

		m_LockState.m_Lock.f_UnlockNoSanitize();

		if (bNeedCleanup)
			m_pNumaArena->f_OnNeedCleanup();
	}

	template <typename t_CParams>
	TCMemoryManagerArena<t_CParams>::~TCMemoryManagerArena()
	{
		{
			for (umint iSlab = 0; iSlab < mc_nSmallSizeSlabs; ++iSlab)
				m_SmallSizeSlabs[iSlab].f_Clear();
		}
		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			for (umint iSlab = 0; iSlab < mc_nLevel0Lists; ++iSlab)
				m_NormalSizeSlabsLevel0[iSlab].f_Clear();
		}
		{
			for (umint iSlab = 0; iSlab < mc_nNormalSizeClasses; ++iSlab)
				m_NormalSizeSlabs[iSlab].f_Clear();
		}
		m_SmallSizeSlabsFull.f_Clear();

		while (auto pSlab = m_FreeSlabs.f_Pop())
		{
			if constexpr (mc_EnableCallbacks)
				m_pMemoryManager->f_OnCommit(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);

			pSlab->~TCMemoryManagerSlabShared<t_CParams>();
			m_pMemoryManager->m_Allocator.f_Free(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);
		}

		{
			for (umint iSlab = 0; iSlab < t_CParams::mc_NumSizesPerLevel; ++iSlab)
			{
				for (umint iLevel = 0; iLevel < t_CParams::mc_NumSubSlabSizeLevels; ++iLevel)
				{
					while (auto pSlab = m_PartiallyFreeSlabs[iSlab][iLevel].f_Pop())
					{
						if constexpr (mc_EnableCallbacks)
							m_pMemoryManager->f_OnCommit(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);

						pSlab->~TCMemoryManagerSlabShared<t_CParams>();
						m_pMemoryManager->m_Allocator.f_Free(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);
					}
					m_PartiallyFreeSlabsAvailable[iSlab].template f_SetBit<false>(iLevel);
				}
			}
		}

		while (auto pSlab = m_FullSlabs.f_Pop())
		{
			if constexpr (mc_EnableCallbacks)
				m_pMemoryManager->f_OnCommit(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);

			pSlab->~TCMemoryManagerSlabShared<t_CParams>();
			m_pMemoryManager->m_Allocator.f_Free(pSlab->f_GetSlabStart(), t_CParams::mc_SlabSize);
		}
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::fp_FreeSmallSubSlabs(TCMemoryManagerSlabShared<t_CParams> *_pSlab)
	{
		auto SlabType = _pSlab->m_SlabType;
		for (auto iSubSlab = _pSlab->m_FreeSubSlabs.f_GetIterator(); iSubSlab; )
		{
			auto pSubSlab = &(*iSubSlab);
			++iSubSlab;
			--_pSlab->m_nFreeSubSlabs;
			pSubSlab->~CMemoryManagerSubSlab_Free();
			auto pSlabAddress = (uint8 *)pSubSlab;
			auto iAlloc = umint(pSlabAddress - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;
			iAlloc = t_CParams::fs_DivideBySlabMultiplier(iAlloc, SlabType);
			_pSlab->f_DecommitSubSlabs(iAlloc, 1);
			if (_pSlab->f_SetBitFree(0, iAlloc))
			{
				if (m_bWantNumaFreeSlabsCleanup) [[unlikely]]
					fp_CheckCleanupNumaFree();

				return true;
			}
		}
		DMibFastCheck(_pSlab->m_nFreeSubSlabs == 0);
		fp_CheckSlabNoLongerGarbage(_pSlab);

		if (m_bWantNumaFreeSlabsCleanup) [[unlikely]]
			fp_CheckCleanupNumaFree();

		return false;
	}

	template <typename t_CParams>
	inline_small void *TCMemoryManagerArena<t_CParams>::f_AllocWithSize(umint &_Size)
	{
		void * pAlloc;
		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			if (_Size > t_CParams::mc_SmallSizeSlabsLargestSize) [[likely]]
				pAlloc = fp_AllocNormal(_Size);
			else
				pAlloc = fp_AllocSmallSize(_Size);
		}
		else
			pAlloc = fp_AllocNormal(_Size);

		return pAlloc;
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::f_AllocBatch(umint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, umint _Size)> const &_Functor)
	{
		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			if (_Size > t_CParams::mc_SmallSizeSlabsLargestSize) [[likely]]
				fp_AllocNormalBatch(_Size, _Functor);
			else
				fp_AllocSmallSizeBatch(_Size, _Functor);
		}
		else
			fp_AllocNormalBatch(_Size, _Functor);

		if (m_bWantNumaFreeSlabsCleanup) [[unlikely]]
			fp_CheckCleanupNumaFree();
	}

	template <typename t_CParams>
	inline_small void TCMemoryManagerArena<t_CParams>::f_FreeThisThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab)
	{
		fp_FreeInline(_pMemory, _pSlab);
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_Free(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab)
	{
		return fp_FreeInline(_pMemory, _pSlab);
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_FreeFromMessage(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab)
	{
		return fp_FreeInline(_pMemory, _pSlab);
	}

	template <typename t_CParams>
	inline_small void TCMemoryManagerArena<t_CParams>::f_FreeOtherThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena)
	{
		fp_FreeOtherThread(_pMemory, _pSlab, _pLocalArena);
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_FreeOtherThread(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab, TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena)
	{
		umint SlabType = _pSlab->m_SlabType;

		[[maybe_unused]] CMessage *pFreeLink = (CMessage *)_pMemory;
		EMessageType FreeLinkType = EMessageType_FreeNormalBlock;

		this->f_OnFreeOtherThread((uint8 *)_pMemory);

		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			if (SlabType == 0)
			{
				auto pData = _pSlab->f_GetSubSlabDataType();
				auto SubSlab = umint((uint8 *)_pMemory - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;

				umint iSubSlab = SubSlab;
				umint SlabBucket = pData[iSubSlab].m_Type;

#if DMibPPtrBits == 32
				constexpr umint c_TooSmallBucket = 1;
#elif DMibPPtrBits == 64
				constexpr umint c_TooSmallBucket = 2;
#else
#error "Implement this"
#endif

				if
					(
						SlabBucket <= c_TooSmallBucket
						||
						(
							!t_CParams::mc_bAllowUnalignedFreeList
							&& ((umint)_pMemory & (sizeof(void *) - 1))
						)
					) [[unlikely]]
				{
					TCMemoryManagerCheckoutLight<t_CParams> Checkout(nullptr);
					if (_pLocalArena && _pLocalArena->m_pArena == nullptr)
					{
						auto &LocalArena = *_pLocalArena;
						auto &MemoryManager = *m_pMemoryManager;
						if (MemoryManager.m_bCanDoLazyCheckout && !LocalArena.m_TemporaryReturnCheckoutCount)
						{
							MemoryManager.fp_CheckoutHelper(LocalArena)->m_LockState.m_CheckoutCount.f_FetchAdd(1, NAtomic::gc_MemoryOrder_Relaxed);
							DMibFastCheck(fg_GetSys()->f_ThreadCreated());
							LocalArena.m_bLazyCheckout = true;
						}
						else
							Checkout = MemoryManager.fp_Checkout(LocalArena);
						if (LocalArena.m_pArena == _pSlab->m_pArena)
						{
							f_FreeThisThread(_pMemory, _pSlab);
							return;
						}
					}

					DMibMemLightweightTrackDisableScope;

					FreeLinkType = EMessageType_FreeSmallBlock;
					CMessage_FreeSmallBlock *pFreeLinkSmallBlock = (CMessage_FreeSmallBlock *)m_pMemoryManager->f_AllocAligned(sizeof(CMessage_FreeSmallBlock), 1);
					pFreeLinkSmallBlock->m_pBlock = _pMemory;
					pFreeLink = pFreeLinkSmallBlock;
				}
			}
		}

		if constexpr (mc_bSubSlabStore && t_CParams::mc_bReapInCleanup)
		{
			if (FreeLinkType == EMessageType_FreeNormalBlock)
			{
				auto SubSlab = umint((uint8 *)_pMemory - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;

				umint iSubSlab;
				if constexpr (mc_bSpecialCaseSlabType0)
					iSubSlab = SlabType == 0 ? SubSlab : t_CParams::fs_DivideBySlabMultiplier(SubSlab, SlabType);
				else
					iSubSlab = t_CParams::fs_DivideBySlabMultiplier(SubSlab, SlabType);

				DMibFastCheck(iSubSlab < _pSlab->f_GetNumSubSlabs());
				umint SlabBucket = _pSlab->f_GetSubSlabDataType()[iSubSlab].m_Type;

				bool bNormal = true;
				if constexpr (mc_bUseSmallSizes)
					bNormal = SlabBucket >= mc_nSmallSizeSlabs;

				if (bNormal)
				{
					SlabBucket -= 2;

					umint SubSlabMultiplier = t_CParams::mc_SlabTypeInfo[SlabType].m_SubSlabMutiplier;
					umint MinSize = SubSlabMultiplier * t_CParams::mc_SubSlabSize;
					umint SlabBucketStart = umint(1) << SlabBucket;
					umint AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));
					uint8 *pBase = _pSlab->f_GetSlabStart() + iSubSlab * MinSize;

					bool bRemote = true;
					if constexpr (mc_bSubSlabBitmaps && !t_CParams::mc_bReapDenseBitmaps)
					{
						// Dense sub-slabs keep their bitmap words in-band; without dense reap
						// they stay on the message path so remote frees never touch data pages
						bRemote = AlignedSize > MinSize || fsp_GetSubSlabBlocks(SlabType, SlabBucket) <= 64;
					}

					if (bRemote)
					{
						bool bWasEmpty;

						auto &Remote = _pSlab->m_pSubSlabRemoteFree[iSubSlab];
						if constexpr (mc_bSubSlabBitmaps)
						{
							umint Index = 0;
							umint nBlocks = 0;
							if (AlignedSize <= MinSize)
							{
								nBlocks = fsp_GetSubSlabBlocks(SlabType, SlabBucket);
								Index = fsp_BitmapBlockIndex(umint((uint8 *)_pMemory - pBase), SlabType, SlabBucket);
							}

							if (nBlocks > 64)
							{
								// Dense sub-slab: per-block pending bits live in-band after the
								// free bitmap words; the header slot is their nonzero-summary,
								// mirroring the free-side layout
								umint nWords = (nBlocks + 63) / 64;
								auto pPendingWords = (NAtomic::TCAtomic<uint64> *)(pBase + fsp_BitmapRemoteWordsOffset(nWords));
								umint iWord = Index >> 6;

								uint64 PreviousWord = pPendingWords[iWord].f_FetchOr(uint64(1) << (Index & 63), NAtomic::gc_MemoryOrder_Release);
								DMibFastCheck(!(PreviousWord & (uint64(1) << (Index & 63)))); // Double free

								bWasEmpty = false;
								if (PreviousWord == 0)
								{
									uint64 PreviousSummary = Remote.m_Bits.f_FetchOr(uint64(1) << iWord, NAtomic::gc_MemoryOrder_Release);
									bWasEmpty = PreviousSummary == 0;
								}
							}
							else
							{
								uint64 Previous = Remote.m_Bits.f_FetchOr(uint64(1) << Index, NAtomic::gc_MemoryOrder_Release);
								DMibFastCheck(!(Previous & (uint64(1) << Index))); // Double free
								bWasEmpty = Previous == 0;
							}
						}
						else
						{
							uint32 Offset = uint32(umint((uint8 *)_pMemory - pBase) / 4);
							uint32 OldHead;
							while (1)
							{
								OldHead = Remote.m_Head.f_Load(NAtomic::gc_MemoryOrder_Relaxed);
								*(uint16 *)_pMemory = OldHead == 0 ? CMemoryManagerSubSlabFreeState::mc_ChainEnd : uint16(OldHead - 1);
								if (Remote.m_Head.f_CompareExchangeWeak(OldHead, Offset + 1, NAtomic::gc_MemoryOrder_Release))
									break;

								yield_cpu;
							}
							bWasEmpty = OldHead == 0;
						}

						if (bWasEmpty)
						{
							// Only empty-to-pending transitions propagate upward, so the common
							// case is a single fetch_or; redundant sets at a level mean the bit
							// above is already set (or its setter is about to set it)
							umint iWord = iSubSlab >> 6;
							uint64 PreviousSummary = _pSlab->m_pRemoteFreeSummary[iWord].f_FetchOr(uint64(1) << (iSubSlab & 63), NAtomic::gc_MemoryOrder_Release);

							if (PreviousSummary == 0)
							{
								uint64 PreviousTop = _pSlab->m_pRemoteFreeNotify->m_SummaryTop.f_FetchOr(uint64(1) << iWord, NAtomic::gc_MemoryOrder_Release);
								if (PreviousTop == 0)
								{
									// The cleanup request rides along with the slab push, which
									// the summary hierarchy already coalesces per pending epoch;
									// requesting on every cross-thread free would bounce a
									// NUMA-node-shared line against the consumer's
									// exchange-to-zero. Skipping the request where the push is
									// skipped is safe: a slab behind a non-empty level is already
									// in the stack, and the consumer clears the request before it
									// steals the stack. The checkout-return path also re-arms on
									// fp_HasRemoteFrees
									fp_PushRemoteFreeSlab(_pSlab);
									m_pNumaArena->f_RequestCleanupWeak(ENumaArenaCleanup_ProcessMessages);
								}
							}
						}

						return;
					}
				}
			}
		}

		if constexpr (mc_bUseFreeMessages)
		{
			f_AddMessage(pFreeLink, FreeLinkType, _pLocalArena);
			m_pNumaArena->f_RequestCleanupWeak(ENumaArenaCleanup_ProcessMessages);
		}
		else
		{
			// Unreachable: with no block metadata the dense reap pending bits handled every
			// cross-thread free above
			DMibFastCheck(false);
		}
	}

	template <typename t_CParams>
	inline_small umint TCMemoryManagerArena<t_CParams>::fs_GetAllocSize(umint _Size)
	{
		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			if (_Size > t_CParams::mc_SmallSizeSlabsLargestSize) [[likely]]
			{
				umint Size = fg_AlignUp(_Size, t_CParams::mc_MinNormalSizeAlignment);
				DMibFastCheck(Size >= sizeof(void *) * 2);
				umint SlabBucket = NMib::fg_GetHighestBitSetNoZero(Size);
				umint SlabBucketGranularity = (umint(1) << (SlabBucket - t_CParams::mc_SizesPerLevelShift));
				return fg_AlignUp(Size, SlabBucketGranularity);
			}
			else
			{
				fsp_GetSlabTypeFromSizeSmall(_Size);
				return _Size;
			}
		}
		else
		{
			umint Size = fg_AlignUp(fg_Max(_Size, mc_MinAllocSize), t_CParams::mc_MinNormalSizeAlignment);
			umint SlabBucket = NMib::fg_GetHighestBitSetNoZero(Size);
			umint SlabBucketGranularity = (umint(1) << (SlabBucket - t_CParams::mc_SizesPerLevelShift));
			return fg_AlignUp(Size, SlabBucketGranularity);
		}
		return 0;
	}

	template <typename t_CParams>
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_FreeSubSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab)
	{
		auto pData = _pSlab->f_GetSubSlabDataType();
		auto pSlabData = _pSlab->f_GetSlabStart();
		umint SlabType = _pSlab->m_SlabType;

		DMibFastCheck(_iSubSlab < _pSlab->f_GetNumSubSlabs());

		umint SlabBucket = pData[_iSubSlab].m_Type - 2;

		DMibFastCheck(SlabBucket < t_CParams::mc_NumSizeLevels);
		DMibFastCheck(_pSlab->f_GetSubSlabDataAlloc()[_iSubSlab].m_nAllocs == 0);

		umint AlignedSize;

		umint SubSlabMultiplier = t_CParams::mc_SlabTypeInfo[SlabType].m_SubSlabMutiplier;
		umint MinSize = SubSlabMultiplier * t_CParams::mc_SubSlabSize;

		umint SlabBucketStart = umint(1) << SlabBucket;
		AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

		uint32 nSubSlabs = 1;
		if (AlignedSize > MinSize)
			nSubSlabs = t_CParams::fs_DivideBySlabMultiplier(AlignedSize/t_CParams::mc_SubSlabSize, SlabType);

		auto pStart = pSlabData + _iSubSlab * t_CParams::mc_SubSlabSize * SubSlabMultiplier;

		if constexpr (mc_bSubSlabStore)
		{
			// The sub-slab may be the current one for its size class; the cache must not
			// outlive the sub-slab
			auto pCurrent = fp_GetCurrentSubSlabSlot(SlabType, SlabBucket, AlignedSize);
			if (pCurrent->m_pFreeState == &_pSlab->m_pSubSlabFreeState[_iSubSlab])
				pCurrent->m_pFreeState = nullptr;

			// The free store lives with the sub-slab; unlinking the sub-slab node releases
			// every block at once (f_Unlink is safe for nodes that were never listed)
			_pSlab->m_pSubSlabNodes[_iSubSlab].m_Link.f_Unlink();
			if constexpr (mc_bSubSlabBitmaps)
				_pSlab->m_pSubSlabFreeState[_iSubSlab] = {0};
			else
				_pSlab->m_pSubSlabFreeState[_iSubSlab] = {nullptr, 0, 0, 0};
		}
		else
		{
			auto pStartRemove = pStart;
			auto pEnd = pStart + t_CParams::mc_SubSlabSize * SubSlabMultiplier * nSubSlabs;
			if constexpr (t_CParams::mc_PreventCacheConflictSize && !t_CParams::mc_bUseFreeBlockCounting)
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

			for (auto pToRemove = pStartRemove; pToRemove < pEnd; )
			{
				CMemoryManagerSubSlab_NormalLink *pFreeBlock = (CMemoryManagerSubSlab_NormalLink *)pToRemove;
				if constexpr (t_CParams::mc_bUseFreeBlockCounting)
					pToRemove += AlignedSize * pFreeBlock->m_nBlocks;
				else
					pToRemove += AlignedSize;
				pFreeBlock->m_Link.f_UnsafeUnlink();
			}
		}

		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			if (!(t_CParams::mc_DeferCleanup & EDeferCleanup_OneSizeBlocks) || nSubSlabs > 1)
			{
				_pSlab->f_DecommitSubSlabs(_iSubSlab, nSubSlabs);

				umint Level = NMib::fg_GetHighestBitSetNoZero(nSubSlabs);

				DMibFastCheck(((_iSubSlab >> Level) << Level) == _iSubSlab);
				if (_pSlab->f_SetBitFree(Level, _iSubSlab >> Level))
				{
					if (m_bWantNumaFreeSlabsCleanup) [[unlikely]]
						fp_CheckCleanupNumaFree();

					return true;
				}
			}
			else
			{
				CMemoryManagerSubSlab_Free *pFreeSubSlab = (CMemoryManagerSubSlab_Free *)pStart;
				pFreeSubSlab = new(pFreeSubSlab) CMemoryManagerSubSlab_Free();
				_pSlab->m_FreeSubSlabs.f_UnsafeInsert(pFreeSubSlab);
				fp_SlabHasGarbageInline(_pSlab);
				++_pSlab->m_nFreeSubSlabs;
			}
		}
		else
		{
			_pSlab->f_DecommitSubSlabs(_iSubSlab, nSubSlabs);

			umint Level = NMib::fg_GetHighestBitSetNoZero(nSubSlabs);

			DMibFastCheck(((_iSubSlab >> Level) << Level) == _iSubSlab);
			if (_pSlab->f_SetBitFree(Level, _iSubSlab >> Level))
			{
				if (m_bWantNumaFreeSlabsCleanup) [[unlikely]]
					fp_CheckCleanupNumaFree();

				return true;
			}
		}

		_pSlab->m_nAllocatedSubSlabs -= nSubSlabs;

		if (m_bWantNumaFreeSlabsCleanup) [[unlikely]]
			fp_CheckCleanupNumaFree();

		return false;
	}

	template <typename t_CParams>
	umint TCMemoryManagerArena<t_CParams>::f_GetNumFreeSlabs()
	{
		umint nSlabs = 0;

		nSlabs += m_FreeSlabs.f_GetLen();

		return nSlabs;
	}

	template <typename t_CParams>
	umint TCMemoryManagerArena<t_CParams>::f_GetNumUsedSlabs()
	{
		umint nSlabs = 0;
		for (umint iSlabType = 0; iSlabType < t_CParams::mc_NumSizesPerLevel; ++iSlabType)
		{
			for (umint i = 0; i < t_CParams::mc_NumSubSlabSizeLevels; ++i)
				nSlabs += m_PartiallyFreeSlabs[iSlabType][i].f_GetLen();
		}

		nSlabs += m_FullSlabs.f_GetLen();

		return nSlabs;
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::f_CheckFree(EMemoryManagerCheckFlag _Flags)
	{
		bool bError = false;
		if constexpr (mc_bSubSlabStore)
		{
			auto fCheckNode = [&](CMemoryManagerSubSlab_ListNode *_pNode)
				{
					umint iSubSlab;
					auto pSlab = fsp_SlabFromSubSlabNode(_pNode, iSubSlab);
					if (fp_CheckFreeSubSlabChain(pSlab, iSubSlab, _Flags))
						bError = true;
				}
			;
			auto fCheckCurrent = [&](CNormalFreeStoreCurrent &_Current)
				{
					if (_Current.m_pFreeState && fp_CheckFreeSubSlabChain(_Current.m_pSlab, _Current.m_iSubSlab, _Flags))
						bError = true;
				}
			;

			if constexpr (t_CParams::mc_bUseSmallSizes)
			{
				for (umint i = 0; i < mc_nLevel0Lists; ++i)
				{
					for (auto iNode = m_NormalSizeSlabsLevel0[i].f_GetIterator(); iNode; ++iNode)
						fCheckNode(&*iNode);

					fCheckCurrent(m_CurrentSubSlabs.m_Level0[i]);
				}
			}
			for (umint i = 0; i < mc_nNormalSizeClasses; ++i)
			{
				for (auto iNode = m_NormalSizeSlabs[i].f_GetIterator(); iNode; ++iNode)
					fCheckNode(&*iNode);

				fCheckCurrent(m_CurrentSubSlabs.m_Normal[i]);
			}

			// Freed multi sub-slab single block allocations are only marked pending: they are
			// never listed or current, so their fill is only reachable through the pending bits
			struct CPendingCheckParams
			{
				TCMemoryManagerSlabShared<t_CParams> *m_pSlab;
				EMemoryManagerCheckFlag m_Flags;
				bool m_bError = false;
			};

			for (umint SlabType = 0; SlabType < t_CParams::mc_NumSizesPerLevel; ++SlabType)
			{
				for (auto iSlab = m_SlabsToGarbageCollect[SlabType].f_GetIterator(); iSlab; ++iSlab)
				{
					CPendingCheckParams Params;
					Params.m_pSlab = &*iSlab;
					Params.m_Flags = _Flags;

					Params.m_pSlab->f_EnumPendingBits
						(
							[&](umint _Bit) -> bool
							{
								auto pSlab = Params.m_pSlab;
								auto pData = pSlab->f_GetSubSlabDataType();
								umint SlabType = pSlab->m_SlabType;
								umint SubSlabMultiplier = t_CParams::mc_SlabTypeInfo[SlabType].m_SubSlabMutiplier;
								umint MinSize = SubSlabMultiplier * t_CParams::mc_SubSlabSize;

								umint SlabBucket = pData[_Bit].m_Type - 2;
								umint AlignedSize = (umint(1) << SlabBucket) + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

								if (AlignedSize > MinSize)
								{
									// Multi sub-slab single block allocations are filled in whole
									// and have no free store state
									if (this->f_OnCheckFree(pSlab->f_GetSlabStart() + _Bit * t_CParams::mc_SubSlabSize * SubSlabMultiplier, AlignedSize, Params.m_Flags))
										Params.m_bError = true;

									return true;
								}

								// A one block sub-slab is never listed or current, so its free
								// store is only reachable through the pending bits
								if (SlabBucket >= t_CParams::mc_MinNormalSlabBucket && fsp_GetSubSlabBlocks(SlabType, SlabBucket) == 1)
								{
									if (fp_CheckFreeSubSlabChain(pSlab, _Bit, Params.m_Flags))
										Params.m_bError = true;
								}

								return true;
							}
						)
					;

					if (Params.m_bError)
						bError = true;
				}
			}
		}
		else
		{
			if constexpr (t_CParams::mc_bUseSmallSizes)
			{
				for (umint i = 0; i < mc_nLevel0Lists; ++i)
				{
					for (auto iAlloc = m_NormalSizeSlabsLevel0[i].f_GetIterator(); iAlloc; ++iAlloc)
					{
						if (fp_CheckFree(&*iAlloc, _Flags))
							bError = true;

					}
				}
			}
			for (umint i = 0; i < mc_nNormalSizeClasses; ++i)
			{
				for (auto iAlloc = m_NormalSizeSlabs[i].f_GetIterator(); iAlloc; ++iAlloc)
				{
					if (fp_CheckFree(&*iAlloc, _Flags))
						bError = true;
				}
			}
		}

		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			if (fp_CheckFreeSmall<1>(_Flags))
				bError = true;
			if (fp_CheckFreeSmall<2>(_Flags))
				bError = true;
			if (fp_CheckFreeSmall<4>(_Flags))
				bError = true;
			if (fp_CheckFreeSmall<8>(_Flags))
				bError = true;
			if constexpr (mc_MinAlignment == 4)
			{
				if (fp_CheckFreeSmall<12>(_Flags))
					bError = true;
			}
			if constexpr (t_CParams::mc_SmallSizeSlabsLargestSize == 16)
			{
				if (fp_CheckFreeSmall<16>(_Flags))
					bError = true;
			}
		}

		return bError;
	}
}
