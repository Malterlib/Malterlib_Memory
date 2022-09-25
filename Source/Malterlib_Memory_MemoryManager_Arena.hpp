// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

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
		DMibFastCheck((uint8 *)&m_Lock == fg_AlignUp((uint8 *)&m_Lock, mint(DMibPMemoryCacheLineSize)));
		DMibFastCheck((uint8 *)&m_MessagesAvailable == fg_AlignUp((uint8 *)&m_MessagesAvailable, mint(DMibPMemoryCacheLineSize)));
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::fp_RequestCleanup()
	{
		m_bWantCleanup = true;
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::f_IsContended(TCMemoryManagerThreadLocal<t_CParams> *_pLocalArena) const
	{
		return m_LockContended.f_Load(NAtomic::EMemoryOrder_Relaxed) > 0;
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::f_ReturnCheckout()
	{
		DMibFastCheck(m_CheckoutCount.f_Load() > 0);
		if (m_CheckoutCount.f_FetchSub(1, NAtomic::EMemoryOrder_Relaxed) == 1)
		{
//			fp_CheckMessages();
			bool bNeedCleanup = fp_CheckCleanup();
			m_Lock.f_UnlockNoSanitize();

			if (bNeedCleanup)
				m_pNumaArena->f_OnNeedCleanup();

			return true;
		}
		return false;
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::f_ForkedChild()
	{
		m_Lock.f_ForkedChildLocked();
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::f_ReturnCheckoutLight()
	{
		DMibFastCheck(m_CheckoutCount.f_Load() == 0); // Should not have been increased
		bool bNeedCleanup = false;
//		fp_CheckMessages();
		if (m_bWantCleanup)
			bNeedCleanup = fp_CheckCleanup();

		m_Lock.f_UnlockNoSanitize();

		if (bNeedCleanup)
			m_pNumaArena->f_OnNeedCleanup();
	}

	template <typename t_CParams>
	TCMemoryManagerArena<t_CParams>::~TCMemoryManagerArena()
	{
		{
			for (mint iSlab = 0; iSlab < mc_nSmallSizeSlabs; ++iSlab)
				m_SmallSizeSlabs[iSlab].f_Clear();
		}
		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			for (mint iSlab = 0; iSlab < mc_nLevel0Lists; ++iSlab)
				m_NormalSizeSlabsLevel0[iSlab].f_Clear();
		}
		{
			for (mint iSlab2 = 0; iSlab2 < t_CParams::mc_NumSizesPerLevel; ++iSlab2)
			{
				for (mint iSlab = 0; iSlab < mc_nNormalSizeLists; ++iSlab)
					m_NormalSizeSlabs[iSlab2][iSlab].f_Clear();
			}
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
			mint nSlabs = sizeof(m_PartiallyFreeSlabs) / sizeof(m_PartiallyFreeSlabs[0]);
			for (mint iSlab = 0; iSlab < nSlabs; ++iSlab)
			{
				for (mint iLevel = 0; iLevel < t_CParams::mc_NumSubSlabSizeLevels; ++iLevel)
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
			auto iAlloc = mint(pSlabAddress - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;
			iAlloc = t_CParams::fs_DivideBySlabMultiplier(iAlloc, SlabType);
			_pSlab->f_DecommitSubSlabs(iAlloc, 1);
			if (_pSlab->f_SetBitFree(0, iAlloc))
				return true;
		}
		DMibFastCheck(_pSlab->m_nFreeSubSlabs == 0);
		fp_CheckSlabNoLongerGarbage(_pSlab);

		return false;
	}

	template <typename t_CParams>
	inline_small void *TCMemoryManagerArena<t_CParams>::f_AllocWithSize(mint &_Size)
	{
		//fp_CheckMessages();
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

		if (m_bWantNumaFreeSlabsCleanup) [[unlikely]]
			fp_CheckCleanupNumaFree();
		return pAlloc;
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::f_AllocBatch(mint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
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
		//fp_CheckMessages();
		fp_FreeInline(_pMemory, _pSlab);
		if (m_bWantNumaFreeSlabsCleanup) [[unlikely]]
			fp_CheckCleanupNumaFree();
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_Free(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pSlab)
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
		mint SlabType = _pSlab->m_SlabType;

		CMessage *pFreeLink = (CMessage *)_pMemory;
		EMessageType FreeLinkType = EMessageType_FreeNormalBlock;

		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			if (SlabType == 0)
			{
				auto pData = _pSlab->f_GetSubSlabDataType();
				auto SubSlab = mint((uint8 *)_pMemory - _pSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;

				mint iSubSlab = SubSlab;
				mint SlabBucket = pData[iSubSlab].m_Type;

#if DMibPPtrBits == 32
				constexpr mint c_TooSmallBucket = 1;
#elif DMibPPtrBits == 64
				constexpr mint c_TooSmallBucket = 2;
#else
#error "Implement this"
#endif

				if
					(
						SlabBucket <= c_TooSmallBucket
						||
						(
							!t_CParams::mc_bAllowUnalignedFreeList
							&& ((mint)_pMemory & (sizeof(void *) - 1))
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
							MemoryManager.fp_CheckoutHelper(LocalArena)->m_CheckoutCount.f_FetchAdd(1, NAtomic::EMemoryOrder_Relaxed);
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

		f_AddMessage(pFreeLink, FreeLinkType, _pLocalArena);
		m_pNumaArena->f_RequestCleanupWeak(ENumaArenaCleanup_ProcessMessages);
	}

	template <typename t_CParams>
	inline_small mint TCMemoryManagerArena<t_CParams>::fs_GetAllocSize(mint _Size)
	{
		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			if (_Size > t_CParams::mc_SmallSizeSlabsLargestSize) [[likely]]
			{
				mint Size = fg_AlignUp(_Size, t_CParams::mc_MinNormalSizeAlignment);
				DMibFastCheck(Size >= sizeof(void *) * 2);
				mint SlabBucket = NMib::fg_GetHighestBitSetNoZero(Size);
				mint SlabBucketGranularity = (mint(1) << (SlabBucket - t_CParams::mc_SizesPerLevelShift));
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
			mint Size = fg_AlignUp(fg_Max(_Size, mc_MinAllocSize), t_CParams::mc_MinNormalSizeAlignment);
			mint SlabBucket = NMib::fg_GetHighestBitSetNoZero(Size);
			mint SlabBucketGranularity = (mint(1) << (SlabBucket - t_CParams::mc_SizesPerLevelShift));
			return fg_AlignUp(Size, SlabBucketGranularity);
		}
		return 0;
	}

	template <typename t_CParams>
	inline_never bool TCMemoryManagerArena<t_CParams>::fp_FreeSubSlab(TCMemoryManagerSlabShared<t_CParams> *_pSlab, uint32 _iSubSlab)
	{
		auto pData = _pSlab->f_GetSubSlabDataType();
		auto pSlabData = _pSlab->f_GetSlabStart();
		mint SlabType = _pSlab->m_SlabType;

		DMibFastCheck(_iSubSlab < _pSlab->f_GetNumSubSlabs());

		mint SlabBucket = pData[_iSubSlab].m_Type - 2;

		DMibFastCheck(SlabBucket < t_CParams::mc_NumSizeLevels);
		DMibFastCheck(_pSlab->f_GetSubSlabDataAlloc()[_iSubSlab].m_nAllocs == 0);

		mint AlignedSize;

		mint SubSlabMultiplier = t_CParams::mc_SlabTypeInfo[SlabType].m_SubSlabMutiplier;
		mint MinSize = SubSlabMultiplier * t_CParams::mc_SubSlabSize;

		mint SlabBucketStart = mint(1) << SlabBucket;
		AlignedSize = SlabBucketStart + (SlabType << (SlabBucket - t_CParams::mc_SizesPerLevelShift));

		uint32 nSubSlabs = 1;
		if (AlignedSize > MinSize)
			nSubSlabs = t_CParams::fs_DivideBySlabMultiplier(AlignedSize/t_CParams::mc_SubSlabSize, SlabType);

		auto pStart = pSlabData + _iSubSlab * t_CParams::mc_SubSlabSize * SubSlabMultiplier;
		auto pEnd = pStart + t_CParams::mc_SubSlabSize * SubSlabMultiplier * nSubSlabs;
		for (auto pToRemove = pStart; pToRemove < pEnd; )
		{
			CMemoryManagerSubSlab_NormalLink *pFreeBlock = (CMemoryManagerSubSlab_NormalLink *)pToRemove;
			if constexpr (t_CParams::mc_bUseFreeBlockCounting)
				pToRemove += AlignedSize * pFreeBlock->m_nBlocks;
			else
				pToRemove += AlignedSize;
			pFreeBlock->m_Link.f_UnsafeUnlink();
		}

		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			if (!(t_CParams::mc_DeferCleanup & EDeferCleanup_OneSizeBlocks) || nSubSlabs > 1)
			{
				_pSlab->f_DecommitSubSlabs(_iSubSlab, nSubSlabs);

				mint Level = NMib::fg_GetHighestBitSetNoZero(nSubSlabs);

				DMibFastCheck(((_iSubSlab >> Level) << Level) == _iSubSlab);
				if (_pSlab->f_SetBitFree(Level, _iSubSlab >> Level))
					return true;
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

			mint Level = NMib::fg_GetHighestBitSetNoZero(nSubSlabs);

			DMibFastCheck(((_iSubSlab >> Level) << Level) == _iSubSlab);
			if (_pSlab->f_SetBitFree(Level, _iSubSlab >> Level))
				return true;
		}

		_pSlab->m_nAllocatedSubSlabs -= nSubSlabs;

		return false;
	}

	template <typename t_CParams>
	mint TCMemoryManagerArena<t_CParams>::f_GetNumFreeSlabs()
	{
		mint nSlabs = 0;

		nSlabs += m_FreeSlabs.f_GetLen();

		return nSlabs;
	}

	template <typename t_CParams>
	mint TCMemoryManagerArena<t_CParams>::f_GetNumUsedSlabs()
	{
		mint nSlabs = 0;
		for (mint iSlabType = 0; iSlabType < t_CParams::mc_NumSizesPerLevel; ++iSlabType)
		{
			for (mint i = 0; i < t_CParams::mc_NumSubSlabSizeLevels; ++i)
				nSlabs += m_PartiallyFreeSlabs[iSlabType][i].f_GetLen();
		}

		nSlabs += m_FullSlabs.f_GetLen();

		return nSlabs;
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::f_CheckFree(EMemoryManagerCheckFlag _Flags)
	{
		bool bError = false;
		if constexpr (t_CParams::mc_bUseSmallSizes)
		{
			for (mint i = 0; i < mc_nLevel0Lists; ++i)
			{
				for (auto iAlloc = m_NormalSizeSlabsLevel0[i].f_GetIterator(); iAlloc; ++iAlloc)
				{
					if (fp_CheckFree(&*iAlloc, _Flags))
						bError = true;

				}
			}
		}
		for (mint j = 0; j < t_CParams::mc_NumSizesPerLevel; ++j)
		{
			for (mint i = 0; i < mc_nNormalSizeLists; ++i)
			{
				for (auto iAlloc = m_NormalSizeSlabs[j][i].f_GetIterator(); iAlloc; ++iAlloc)
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
			if constexpr (mc_bUseFreeBlockCounting)
			{
				if (fp_CheckFreeSmall<16>(_Flags))
					bError = true;
			}
		}

		return bError;
	}
}
