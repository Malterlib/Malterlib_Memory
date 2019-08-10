// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	int64 TCMemoryManagerArena<t_CParams>::f_GarbageCollect(ENumaArenaCleanup &_oCleanup, int64 _Timestamp)
	{
		int64 NextTimestamp = TCLimitsInt<int64>::mc_Max;

		for (auto iSlabType = 0; iSlabType < t_CParams::mc_NumSizesPerLevel; ++iSlabType)
			NextTimestamp = fg_Min(NextTimestamp, fp_GarbageCollectPerform(iSlabType, _Timestamp));

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
			pNumaArena->m_FreeSlabs.f_Insert(pFreeSlab);
			if (pFreeSlab->m_Link2.f_IsInList())
				pNumaArena->m_FreeSlabsNeedingDecommit.f_Insert(pFreeSlab);
			if (!bWasEmpty)
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
	bool TCMemoryManagerArena<t_CParams>::fp_GarbageCollectPerform(mint _SlabType)
	{
		DMibFastCheck(_SlabType < t_CParams::mc_NumSizesPerLevel);

		if (auto pSlab = m_SlabsToGarbageCollect[_SlabType].f_GetFirst())
		{
			pSlab->f_EnumPendingBits
				(
					[&](mint _Bit) -> bool
					{
						fp_FreeSubSlab(pSlab, _Bit);
						fp_SubSlabNoLongerPending(pSlab, _Bit);
						return true;
					}
				)
			;

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
	int64 TCMemoryManagerArena<t_CParams>::fp_GarbageCollectPerform(mint _SlabType, int64 _Timestamp)
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

			pSlab->f_EnumPendingBits
				(
					[&](mint _Bit) -> bool
					{
						fp_FreeSubSlab(pSlab, _Bit);
						fp_SubSlabNoLongerPending(pSlab, _Bit);
						return true;
					}
				)
			;

			if constexpr (t_CParams::mc_bUseSmallSizes)
			{
				if (pSlab->m_Link1.f_IsInList())
					fp_FreeSmallSubSlabs(pSlab);
			}

			DMibFastCheck(!pSlab->m_Link1.f_IsInList());
		}

		return NextTimestamp;
	}

	template <typename t_CParams>
	void TCMemoryManagerArena<t_CParams>::fp_GarbageCollectFull()
	{
		fp_CheckMessages();
		for (mint i = 0; i < t_CParams::mc_NumSizesPerLevel; ++i)
		{
			for (auto iSlab = m_SlabsToGarbageCollect[i].f_GetIterator(); iSlab; )
			{
				auto pSlab = &*iSlab;
				++iSlab;

				pSlab->f_EnumPendingBits
					(
						[&](mint _Bit) -> bool
						{
							fp_FreeSubSlab(pSlab, _Bit);
							fp_SubSlabNoLongerPending(pSlab, _Bit);
							return true;
						}
					)
				;

				if constexpr (t_CParams::mc_bUseSmallSizes)
				{
					if (pSlab->m_Link1.f_IsInList())
						fp_FreeSmallSubSlabs(pSlab);
				}

				DMibFastCheck(!pSlab->m_Link1.f_IsInList());
			}
		}
	}

	template <typename t_CParams>
	bool TCMemoryManagerArena<t_CParams>::fp_GarbageCollect(mint _SlabType)
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
			_pSlab->m_HasGarbageTimestamp = m_pNumaArena->f_GetTimestamp();
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
			_pSlab->m_Link1.f_Unlink();
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
