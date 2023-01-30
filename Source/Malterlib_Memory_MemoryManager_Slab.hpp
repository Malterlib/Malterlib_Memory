// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	////////////////////////////////
	// Shared

	template <typename t_CParams>
	TCMemoryManagerSlabShared<t_CParams>::TCMemoryManagerSlabShared
		(
			uint32 _SlabType
			, TCMemoryManagerArena<t_CParams> *_pArena
			, uint8 _nCommittedHeaderSubSlabs
		)
		: m_pArena(_pArena)
		, m_pMemoryManager(_pArena->m_pMemoryManager)
		, m_nAllocatedSubSlabs(0)
		, m_nFreeSubSlabs(0)
		, m_nPendingSubSlabs(0)
		, m_FullySetLevel(t_CParams::mc_NumSubSlabSizeLevels)
		, m_SlabType(_SlabType)
		, m_nCommittedHeaderSubSlabs(_nCommittedHeaderSubSlabs)
	{
		static_assert(t_CParams::mc_NumSubSlabSizeLevels < 128);
	}

	template <typename t_CParams>
	TCMemoryManagerSlabShared<t_CParams>::~TCMemoryManagerSlabShared()
	{
	}

	template <typename t_CParams>
	mint TCMemoryManagerSlabShared<t_CParams>::f_GetNumSubSlabs() const
	{
		return t_CParams::mc_NumSubSlabs[m_SlabType];
	}

	template <typename t_CParams>
	mint TCMemoryManagerSlabShared<t_CParams>::f_GetSubSlabMultiplier() const
	{
		return t_CParams::mc_SlabTypeInfo[m_SlabType].m_SubSlabMultiplier;
	}

	template <typename t_CParams>
	inline_always uint8 *TCMemoryManagerSlabShared<t_CParams>::f_GetSlabStart()
	{
		return fg_AlignDown(((uint8 *)this), t_CParams::mc_SlabSize);
	}

	template <typename t_CParams>
	inline_always TCMemoryManagerSubSlabDataType<t_CParams> *TCMemoryManagerSlabShared<t_CParams>::f_GetSubSlabDataType()
	{
		return (TCMemoryManagerSubSlabDataType<t_CParams> *)((uint8 *)(this + 1));
	}

	template <typename t_CParams>
	inline_always TCMemoryManagerSubSlabDataType<t_CParams> const *TCMemoryManagerSlabShared<t_CParams>::f_GetSubSlabDataType() const
	{
		return (TCMemoryManagerSubSlabDataType<t_CParams> const *)((uint8 const *)(this + 1));
	}

	template <typename t_CParams>
	inline_always TCMemoryManagerSubSlabDataAlloc<t_CParams> *TCMemoryManagerSlabShared<t_CParams>::f_GetSubSlabDataAlloc()
	{
		return m_pSubSlabDataAlloc;
	}

	template <typename t_CParams>
	inline_always TCMemoryManagerSubSlabDataAlloc<t_CParams> const *TCMemoryManagerSlabShared<t_CParams>::f_GetSubSlabDataAlloc() const
	{
		return m_pSubSlabDataAlloc;
	}

	template <typename t_CParams>
	inline_always uint8 const *TCMemoryManagerSlabShared<t_CParams>::f_GetSlabStart() const
	{
		return fg_AlignDown(((uint8 const *)this), t_CParams::mc_SlabSize);
	}

	/////////////////////////////////
	// Non-shared

	template <typename t_CParams, uint32 t_SlabType>
	TCMemoryManagerSlab<t_CParams, t_SlabType>::TCMemoryManagerSlab
		(
			uint64 _Magic
			, TCMemoryManagerArena<t_CParams> *_pArena
			, uint8 _nCommittedHeaderSubSlabs
		)
		: TCMemoryManagerSlabShared<t_CParams>(t_SlabType, _pArena, _nCommittedHeaderSubSlabs)
	{
		DMibFastCheck(m_Allocated.f_IsFullyFree());

		static_assert
			(
				(mc_nSubSlabs * t_CParams::mc_SubSlabSize * t_CParams::mc_SlabTypeInfo[t_SlabType].m_SubSlabMutiplier + sizeof(TCMemoryManagerSlab)) <= t_CParams::mc_SlabSize
				, "Too many sub slabs"
			)
		;
		static_assert
			(
				((mc_nSubSlabs + 1) * t_CParams::mc_SubSlabSize * t_CParams::mc_SlabTypeInfo[t_SlabType].m_SubSlabMutiplier + sizeof(TCMemoryManagerSlab)) > t_CParams::mc_SlabSize
				, "Too few sub slabs"
			)
		;

		uint8 *pEndOfSlab =  fg_AlignUp((uint8 *)this, t_CParams::mc_SlabSize);
		CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

		pHeader->m_Magic = NPrivate::fg_CalcMagic(pEndOfSlab, _Magic);
		pHeader->m_SlabStartOffset = pEndOfSlab - (uint8 *)this;

		DMibFastCheck(this->f_GetNumSubSlabs() == mc_nSubSlabs);

		for (mint i = 0; i < mc_nSubSlabs; ++i)
			m_SubSlabDataType[i].m_Type = 0;
		for (mint i = 0; i < mc_nSubSlabs; ++i)
			m_SubSlabDataAlloc[i].m_nAllocs = 0;
		this->m_pSubSlabDataAlloc = m_SubSlabDataAlloc;

		DMibFastCheck(this->f_GetSubSlabDataType() == m_SubSlabDataType);
		DMibFastCheck(this->f_GetSubSlabDataAlloc() == m_SubSlabDataAlloc);
	}

	template <typename t_CParams, uint32 t_SlabType>
	TCMemoryManagerSlab<t_CParams, t_SlabType>::~TCMemoryManagerSlab()
	{
		uint8 *pEndOfSlab =  fg_AlignUp((uint8 *)this, t_CParams::mc_SlabSize);
		CMemoryManagerSlabSharedPostfixHeader *pHeader = (CMemoryManagerSlabSharedPostfixHeader *)(pEndOfSlab - sizeof(CMemoryManagerSlabSharedPostfixHeader));

		pHeader->m_Magic = 0;
		pHeader->m_SlabStartOffset = 0;
	}

	template <typename t_CParams, uint32 t_SlabType>
	void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_SetPendingBit(mint _Bit)
	{
		m_PendingFree.template f_SetBit<true>(_Bit);
	}

	template <typename t_CParams, uint32 t_SlabType>
	mint TCMemoryManagerSlab<t_CParams, t_SlabType>::f_GetNumPendingBits()
	{
		mint nBitsSet = 0;
		for (mint iSlab = 0; iSlab < mc_nSubSlabs; ++iSlab)
		{
			if (m_PendingFree.f_GetBit(iSlab))
				++nBitsSet;
		}
		return nBitsSet;
	}

	template <typename t_CParams, uint32 t_SlabType>
	bool TCMemoryManagerSlab<t_CParams, t_SlabType>::f_HasPendingBit()
	{
		return !m_PendingFree.f_IsFullyFree();
	}

	template <typename t_CParams, uint32 t_SlabType>
	bool TCMemoryManagerSlab<t_CParams, t_SlabType>::f_ClearPendingBit(mint _Bit)
	{
		bool bRet = m_PendingFree.f_GetBit(_Bit);
		m_PendingFree.template f_SetBit<false>(_Bit);
		return bRet;
	}

	template <typename t_CParams, uint32 t_SlabType>
	void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_EnumPendingBits(NFunction::TCFunctionNoAlloc<bool (mint _Bit)> const& _fCallback)
	{
		m_PendingFree.f_EnumSetBits(_fCallback, 0);
	}

	template <typename t_CParams, uint32 t_SlabType>
	fp32 TCMemoryManagerSlab<t_CParams, t_SlabType>::f_OverheadPerByte() const
	{
		return
			fp32
			(
				t_CParams::mc_SlabSize
				-
				(
					mc_nSubSlabs
					* t_CParams::mc_SlabTypeInfo[t_SlabType].m_SubSlabMutiplier
					* t_CParams::mc_SubSlabSize
				)
			)
			/
			fp32(t_CParams::mc_SlabSize)
		;
	}

	template <typename t_CParams, uint32 t_SlabType>
	bool TCMemoryManagerSlab<t_CParams, t_SlabType>::f_SetBitFree(mint _Level, mint _Bit)
	{
		DMibFastCheck(m_Allocated.f_GetBit(_Level, _Bit));
		m_Allocated.template f_SetBit<false>(_Level, _Bit);
		DMibFastCheck(this->m_FullySetLevel < t_CParams::mc_NumSubSlabSizeLevels);

		auto pArena = this->m_pArena;

		if (m_Allocated.f_IsFullyFree())
		{
			// Unlink from any linked lists
			this->m_Link.f_Unlink();
			this->m_LinkToGarbageCollect.f_Unlink();
			if constexpr (t_CParams::mc_bUseSmallSizes)
				this->m_FreeSubSlabs.f_Clear();
			auto pNumaArena = pArena->m_pNumaArena;
			if constexpr (t_CParams::mc_bBackgroundCleanup)
				this->m_FreeTimestamp = pNumaArena->f_GetTimestamp();
			auto FullySetLevel = this->m_FullySetLevel;
			this->m_FullySetLevel = t_CParams::mc_NumSubSlabSizeLevels;

			bool bReturn = false;

			if (pArena->m_FreeSlabs.f_IsEmpty())
			{
				pArena->m_FreeSlabs.f_Insert(this);
				pArena->fp_RequestCleanup();
			}
			else
			{
				DMibLock(pNumaArena->m_FreeSlabsLock);
				this->m_pArena = nullptr;
				bool bWasEmpty = pNumaArena->m_FreeSlabs.f_IsEmpty();

				this->m_LinkToGarbageCollect.f_Unlink();

				if constexpr (t_CParams::mc_bBackgroundCleanup)
					DMibFastCheck(this->m_FreeTimestamp != 0);

				pNumaArena->m_FreeSlabs.f_InsertFirst(this);
				bool bPendingDecommit = this->m_LinkNeedDecommit.f_IsInList();
				if (bPendingDecommit)
					pNumaArena->m_FreeSlabsNeedingDecommit.f_InsertFirst(this);

				if (bPendingDecommit || !bWasEmpty)
					pArena->m_bWantNumaFreeSlabsCleanup = true;

				bReturn = true;
			}

			if (pArena->m_PartiallyFreeSlabs[t_SlabType][FullySetLevel].f_IsEmpty())
				pArena->m_PartiallyFreeSlabsAvailable[t_SlabType].template f_SetBit<false>(FullySetLevel);

			return bReturn;
		}
		else if (this->m_FullySetLevel == t_CParams::mc_NumSubSlabSizeLevels - 1)
			return false;

		mint iNewLevel = this->m_FullySetLevel;
		while (iNewLevel < t_CParams::mc_NumSubSlabSizeLevels - 1 && !m_Allocated.f_IsFullySet(iNewLevel + 1))
			++iNewLevel;

		if (iNewLevel != this->m_FullySetLevel)
		{
			pArena->m_PartiallyFreeSlabs[t_SlabType][iNewLevel].f_Insert(this);
			DMibFastCheck(m_Allocated.f_FindFreeBit(iNewLevel) >= 0);
			pArena->m_PartiallyFreeSlabsAvailable[t_SlabType].template f_SetBit<true>(iNewLevel);

			if (pArena->m_PartiallyFreeSlabs[t_SlabType][this->m_FullySetLevel].f_IsEmpty())
				pArena->m_PartiallyFreeSlabsAvailable[t_SlabType].template f_SetBit<false>(this->m_FullySetLevel);

			DMibFastCheck(iNewLevel < 128);
			this->m_FullySetLevel = iNewLevel;
		}

		return false;
	}

	template <typename t_CParams, uint32 t_SlabType>
	bool TCMemoryManagerSlab<t_CParams, t_SlabType>::f_IsFullyFree()
	{
		return m_Allocated.f_IsFullyFree();
	}

	template <typename t_CParams, uint32 t_SlabType>
	mint TCMemoryManagerSlab<t_CParams, t_SlabType>::f_GetNumSetBits(mint _Level)
	{
		mint nBitsSet = 0;
		for (mint iSlab = 0; iSlab < mc_nSubSlabs; ++iSlab)
		{
			if (m_Allocated.f_GetBit(_Level, iSlab))
				++nBitsSet;
		}
		return nBitsSet;
	}

	template <typename t_CParams, uint32 t_SlabType>
	bool TCMemoryManagerSlab<t_CParams, t_SlabType>::f_HasFreeBit(mint _Level)
	{
		return m_Allocated.f_FindFreeBit(_Level) >= 0;
	}

	template <typename t_CParams, uint32 t_SlabType>
	aint TCMemoryManagerSlab<t_CParams, t_SlabType>::f_FindFreeBitAndSet(mint _Level)
	{
		aint Ret;
		if constexpr (t_CParams::mc_bUseSlabFromEnd)
			Ret = m_Allocated.f_FindFreeBitReverseAndSet(_Level);
		else
			Ret = m_Allocated.f_FindFreeBitAndSet(_Level);

		DMibFastCheck(Ret >= 0); // Should not be called on full

		auto pArena = this->m_pArena;

		if (this->m_FullySetLevel == t_CParams::mc_NumSubSlabSizeLevels)
		{
			pArena->m_PartiallyFreeSlabs[t_SlabType][t_CParams::mc_NumSubSlabSizeLevels - 1].f_Insert(this);
			pArena->m_PartiallyFreeSlabsAvailable[t_SlabType].template f_SetBit<true>(t_CParams::mc_NumSubSlabSizeLevels - 1);
			this->m_FullySetLevel = t_CParams::mc_NumSubSlabSizeLevels - 1;
			return Ret;
		}

		aint iOldLevel = this->m_FullySetLevel;
		aint iNewLevel = iOldLevel;
		while (iNewLevel >= 0 && m_Allocated.f_IsFullySet(iNewLevel))
			--iNewLevel;

		if (iNewLevel != iOldLevel)
		{
			if (iNewLevel < 0)
				pArena->m_FullSlabs.f_Insert(this);
			else
			{
				pArena->m_PartiallyFreeSlabs[t_SlabType][iNewLevel].f_Insert(this);
				pArena->m_PartiallyFreeSlabsAvailable[t_SlabType].template f_SetBit<true>(iNewLevel);
				DMibFastCheck(m_Allocated.f_FindFreeBit(iNewLevel) >= 0);
			}

			if (pArena->m_PartiallyFreeSlabs[t_SlabType][iOldLevel].f_IsEmpty())
				pArena->m_PartiallyFreeSlabsAvailable[t_SlabType].template f_SetBit<false>(iOldLevel);

			DMibFastCheck(iNewLevel < 128);
			this->m_FullySetLevel = fg_Max(iNewLevel, 0);
		}

		return Ret;
	}

	template <typename t_CParams, uint32 t_SlabType>
	inline_always TCMemoryManagerSlab<t_CParams, t_SlabType> *TCMemoryManagerSlab<t_CParams, t_SlabType>::fs_CalcSlabLocation(uint8 *_pLocation)
	{
		if constexpr (t_CParams::mc_bRandomizeSlabHeader)
		{
			mint Location = (mint)_pLocation;
			mint Shift = TCHighestBitSetCorrect<mint, t_CParams::mc_SlabSize>::mc_Value;

			Location >>= Shift;
#if 0
			const static mint FreeSpace = TCAlignDown<mint, (t_CParams::mc_SubSlabSize - sizeof(TCMemoryManagerSlab)), alignof(TCMemoryManagerSlab)>::mc_Value;
			uint8 *pFinalLocation = _pLocation + fg_AlignDown(Location, alignof(TCMemoryManagerSlab)) % FreeSpace;
#else
			mint FreeSpace = mint(1) << (TCHighestBitSetCorrect<mint, (t_CParams::mc_SubSlabSize - sizeof(TCMemoryManagerSlab))>::mc_Value);
			mint ToAdd = (fg_AlignDown(Location, alignof(TCMemoryManagerSlab)) & (FreeSpace - 1));
			uint8 *pFinalLocation = _pLocation + ToAdd;
#endif
			return (TCMemoryManagerSlab *)pFinalLocation;
		}
		return (TCMemoryManagerSlab *)_pLocation;
	}
}
