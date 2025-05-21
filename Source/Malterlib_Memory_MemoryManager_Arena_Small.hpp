// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	inline_never void *TCMemoryManagerArena<t_CParams>::fp_AllocSmallSize(mint &_Size)
	{
		mint iSlab = fsp_GetSlabTypeFromSizeSmall(_Size);
		DMibMemLightweightTrack(m_pMemoryManager->fp_TrackAlloc(_Size));
		if (iSlab == 0) [[unlikely]]
			return fsp_AllocSmall<1>(this);
		return fsp_AllocSmallShared(this, iSlab);
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_AllocSmallSizeBatch(mint _Size, NFunction::TCFunctionNoAlloc<bool (void * _pAlloc, mint _Size)> const &_Functor)
	{
		DMibMemLightweightTrack
			(
				auto *pLocalArena = m_pMemoryManager->m_LocalArena.f_TryGet();
			)
		;

		mint iSlab = fsp_GetSlabTypeFromSizeSmall(_Size);
		if (iSlab == 0) [[unlikely]]
		{
			while (true)
			{
				DMibMemLightweightTrack
					(
						{
							if (TCMemoryManager<t_CParams>::fsp_ShouldTrackAlloc(pLocalArena))
								pLocalArena->f_TrackAlloc(1);
						}
					)
				;

				auto *pAlloc = fsp_AllocSmall<1>(this);
				if (!_Functor(pAlloc, 1))
					break;
			}
			return;
		}
		while (true)
		{
			DMibMemLightweightTrack
				(
					{
						if (TCMemoryManager<t_CParams>::fsp_ShouldTrackAlloc(pLocalArena))
							pLocalArena->f_TrackAlloc(_Size);
					}
				)
			;

			auto *pAlloc = fsp_AllocSmallShared(this, iSlab);
			if (!_Functor(pAlloc, _Size))
				break;
		}
		return;
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_FreeSmall(void *_pMemory, TCMemoryManagerSlab<t_CParams, 0> *_pSlab, mint _SlabType)
	{
		// Small slabs
		ESmallState SmallState;
		mint Index;
		TCMemoryManagerSubSlab_SmallSizeShared<t_CParams> *pSubSlab = fg_AlignDown((TCMemoryManagerSubSlab_SmallSizeShared<t_CParams> *)_pMemory, t_CParams::mc_SubSlabSize);

		if (_SlabType == 0) [[unlikely]]
		{
			Index = 0;
			TCMemoryManagerSubSlab_SmallSize<t_CParams, 1> *pSubSlab1 = (TCMemoryManagerSubSlab_SmallSize<t_CParams, 1> *)pSubSlab;
			if constexpr (mc_EnableCallbacks)
				pSubSlab1->f_OnFree(*this, _pMemory);
			SmallState = pSubSlab1->f_Free(_pMemory);
		}
		else
		{
			if constexpr (mc_EnableCallbacks)
				pSubSlab->f_OnFree(*this, _pMemory);

			mint Offset = (uint8 *)_pMemory - pSubSlab->f_GetArray();
			mint iAlloc;
			switch (_SlabType)
			{
			case 1:
				Index = 1;
				iAlloc = Offset / 2;
				break;
			case 2:
				Index = 2;
				iAlloc = Offset / 4;
				break;
			case 3:
				Index = 3;
				iAlloc = Offset / 8;
				break;
			case 4:
				if constexpr (mc_MinAlignment == 4)
				{
					Index = 4;
					iAlloc = Offset / 12;
					break;
				}
				// Intentional fall through
			case 5:
				Index = 5;
				iAlloc = Offset / 16;
				break;
			}
			SmallState = pSubSlab->f_Free(_pMemory, iAlloc);
		}

		if (SmallState != ESmallState_None) [[unlikely]]
			fp_FreeSmallShared(pSubSlab, _pSlab, Index, SmallState);
	}

	template <typename t_CParams>
	mint TCMemoryManagerArena<t_CParams>::fp_SizeSmall(void const * _pMemory, TCMemoryManagerSlab<t_CParams, 0> const * _pSlab, mint _SlabType) const
	{
		switch (_SlabType)
		{
		case 0:
			return 1;
		case 1:
			return 2;
		case 2:
			return 4;
		case 3:
			return 8;
		case 4:
			if constexpr (mc_MinAlignment == 4)
				return 12;
			else
				return 16;
		case 5:
			if constexpr (mc_MinAlignment == 4)
				return 16;
		default:
			DMibFastCheck(false);
			break;
		}
		return 0;
	}

	template <typename t_CParams>
	fp32 TCMemoryManagerArena<t_CParams>::fp_OverheadSmall(void const * _pMemory, TCMemoryManagerSlab<t_CParams, 0> const * _pSlab, mint _SlabType) const
	{
		switch (_SlabType)
		{
		case 0:
			return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>::mc_NumAllocs);
		case 1:
			return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 2>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 2>::mc_NumAllocs);
		case 2:
			return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 4>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 4>::mc_NumAllocs);
		case 3:
			return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 8>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 8>::mc_NumAllocs);
		case 4:
			if constexpr (mc_MinAlignment == 4)
				return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 12>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 12>::mc_NumAllocs);
			else
				return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 16>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 16>::mc_NumAllocs);
		case 5:
			if constexpr (mc_MinAlignment == 4)
				return fp32(sizeof(TCMemoryManagerSubSlab_SmallSize<t_CParams, 16>)) / fp32(TCMemoryManagerSubSlab_SmallSize<t_CParams, 16>::mc_NumAllocs);
		default:
			DMibFastCheck(false);
			break;
		}
		return 0.0;
	}

	template <typename t_CParams>
	inline_small mint TCMemoryManagerArena<t_CParams>::fsp_GetSlabTypeFromSizeSmall(mint &o_Size)
	{
		mint Size = o_Size;
		mint iSlab;
		if (Size < 2)
			iSlab = 0;
		else if (Size <= TCMemoryManagerArena<t_CParams>::mc_MinAlignment)
			iSlab = NMib::fg_GetHighestBitSetNoZero(Size - 1) + 1;
		else
			iSlab = TCMemoryManagerArena<t_CParams>::mc_nSmallSizeSlabsAligned + (Size - (TCMemoryManagerArena<t_CParams>::mc_MinAlignment + 1)) / TCMemoryManagerArena<t_CParams>::mc_MinAlignment;

		switch (iSlab)
		{
		case 0:
			Size = 1;
			break;
		case 1:
			Size = 2;
			break;
		case 2:
			Size = 4;
			break;
		case 3:
			Size = 8;
			break;
		case 4:
			if constexpr (mc_MinAlignment == 4)
			{
				Size = 12;
				break;
			}
			else
			{
				++iSlab;
				Size = 16;
				break;
			}
		case 5:
			if constexpr (mc_MinAlignment == 4)
			{
				Size = 16;
				break;
			}
#if DMibEnableSafeCheck > 0
		default:
			DMibFastCheck(false);
			Size = 1;
			break;
#endif
		}
		o_Size = Size;
		return iSlab;
	}

	template <typename t_CParams>
	inline_always void *TCMemoryManagerArena<t_CParams>::fsp_AllocSmallShared(TCMemoryManagerArena *_pThis, mint _Index)
	{
		using CSubSlab = TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>;
		auto &Slabs = _pThis->m_SmallSizeSlabs[_Index];
		CSubSlab *pSlab = (CSubSlab *)Slabs.f_GetFirst();
		if (!pSlab) [[unlikely]]
			return mc_SmallAllocCategoryJumpTable.m_Table[_Index](_pThis);

		bool bFull;
		void *pAlloc = pSlab->f_Alloc(bFull);
		if constexpr (mc_EnableCallbacks)
		{
			pSlab->f_OnCheckFree(*_pThis, pAlloc, EMemoryManagerCheckFlag_Default);
			pSlab->f_OnAlloc(*_pThis, pAlloc);
		}

		if (bFull) [[unlikely]]
		{
			pSlab->m_Params.m_Link.m_Link.f_UnsafeUnlink();
			_pThis->m_SmallSizeSlabsFull.f_UnsafeInsertFirst(pSlab->m_Params.m_Link);
		}

		return pAlloc;
	}

	template <typename t_CParams>
	template <mint tf_Size>
	inline_never void *TCMemoryManagerArena<t_CParams>::fsp_AllocSmall(TCMemoryManagerArena *_pThis)
	{
		using CSubSlab = TCMemoryManagerSubSlab_SmallSize<t_CParams, tf_Size>;

		auto &Slabs = _pThis->m_SmallSizeSlabs[CSubSlab::mc_SmallSlabIndex];
		CSubSlab *pSlab = (CSubSlab *)Slabs.f_GetFirst();
		if (!pSlab)
			pSlab = _pThis->fp_AllocSmallNoSlab<tf_Size>();

		bool bFull;
		void *pAlloc = pSlab->f_Alloc(bFull);
		if constexpr (mc_EnableCallbacks)
		{
			pSlab->f_OnCheckFree(*_pThis, pAlloc, EMemoryManagerCheckFlag_Default);
			pSlab->f_OnAlloc(*_pThis, pAlloc);
		}

		if (bFull)
		{
			pSlab->m_Params.m_Link.m_Link.f_UnsafeUnlink();
			_pThis->m_SmallSizeSlabsFull.f_UnsafeInsertFirst(pSlab->m_Params.m_Link);
		}

		return pAlloc;
	}

	template <typename t_CParams>
	template <mint tf_Size>
	inline_never TCMemoryManagerSubSlab_SmallSize<t_CParams, tf_Size> *TCMemoryManagerArena<t_CParams>::fp_AllocSmallNoSlab()
	{
		using CSubSlab = TCMemoryManagerSubSlab_SmallSize<t_CParams, tf_Size>;

		auto pFreeSlab = (TCMemoryManagerSlab<t_CParams, 0> *)m_PartiallyFreeSlabs[0][0].f_GetFirst();

		if (!pFreeSlab)
			pFreeSlab = (TCMemoryManagerSlab<t_CParams, 0> *)fp_NewSlab(0, 0);

		CMemoryManagerSubSlab_Free *pExistingSlab = pFreeSlab->m_FreeSubSlabs.f_UnsafePop();
		uint8 *pSlabAddress;
		aint iAlloc;
		if (pExistingSlab)
		{
			fp_CheckSlabNoLongerGarbage(pFreeSlab);

			--pFreeSlab->m_nFreeSubSlabs;

			pSlabAddress = (uint8 *)pExistingSlab;
			iAlloc = mint((uint8 *)pSlabAddress - pFreeSlab->f_GetSlabStart()) / t_CParams::mc_SubSlabSize;
		}
		else
		{
			iAlloc = pFreeSlab->f_FindFreeBitAndSet(0);

			pSlabAddress = pFreeSlab->f_GetSlabStart() + iAlloc * t_CParams::mc_SubSlabSize;
			pFreeSlab->f_CommitSubSlabs(iAlloc, 1);
		}
		++pFreeSlab->m_nAllocatedSubSlabs;

		mint SlabIndex = CSubSlab::mc_SmallSlabIndex;
		static_assert(CSubSlab::mc_SmallSlabIndex <= TCMemoryManagerSubSlabDataType<t_CParams>::mc_MaxType);

		pFreeSlab->m_SubSlabDataType[iAlloc].m_Type = SlabIndex;
		//DMibDTrace("Small: {}" DMibNewLine, pFreeSlab->m_SubSlabDataType[iAlloc]m_Type);

		CSubSlab *pSlab = new(pSlabAddress) CSubSlab();

		if constexpr (mc_EnableCallbacks)
			pSlab->f_OnFillFree(*this);

		DMibFastCheck(SlabIndex < mc_nSmallSizeSlabs);
		auto &Slabs = m_SmallSizeSlabs[SlabIndex];

		Slabs.f_Insert(pSlab->m_Params.m_Link);

		return pSlab;
	}

	template <typename t_CParams>
	inline_never void TCMemoryManagerArena<t_CParams>::fp_FreeSmallShared
		(
			TCMemoryManagerSubSlab_SmallSizeShared<t_CParams> *_pSubSlab
			, TCMemoryManagerSlab<t_CParams, 0> *_pSlab
			, mint _Index
			, ESmallState _SmallState
		)
	{
		if (_SmallState == ESmallState_IsFullyFree)
		{
			_pSubSlab->m_Params.m_Link.m_Link.f_UnsafeUnlink();
			CMemoryManagerSubSlab_Free *pFreeSubSlab = (CMemoryManagerSubSlab_Free *)_pSubSlab;
			pFreeSubSlab = new(pFreeSubSlab) CMemoryManagerSubSlab_Free();
			_pSlab->m_FreeSubSlabs.f_UnsafeInsert(pFreeSubSlab);
			if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_OneSizeBlocks) || !t_CParams::mc_bUseSmallSizes)
				fp_SlabHasGarbageInline(_pSlab);
			else
			{
				if (fp_FreeSmallSubSlabs(_pSlab))
					return;
			}
			++_pSlab->m_nFreeSubSlabs;
			--_pSlab->m_nAllocatedSubSlabs;
		}
		else
		{
			DMibFastCheck(_SmallState == ESmallState_WasFull);
			_pSubSlab->m_Params.m_Link.m_Link.f_UnsafeUnlink();

			m_SmallSizeSlabs[_Index].f_UnsafeInsert(_pSubSlab->m_Params.m_Link);
		}
	}

	template <typename t_CParams>
	template <mint tf_Size>
	bool TCMemoryManagerArena<t_CParams>::fp_CheckFreeSmall(EMemoryManagerCheckFlag _Flags)
	{
		bool bError = false;

		using CSubSlab = TCMemoryManagerSubSlab_SmallSize<t_CParams, tf_Size>;

		auto &Slabs = m_SmallSizeSlabs[CSubSlab::mc_SmallSlabIndex];
		for (auto iSlab = Slabs.f_GetIterator(); iSlab; ++iSlab)
		{
			CSubSlab * pSlab = (CSubSlab *)&*iSlab;
			if (pSlab->f_CheckFree(*this, _Flags))
				bError = true;
		}

		return bError;
	}
}
