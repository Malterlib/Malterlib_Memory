// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams>
	template <uint32 t_SlabType>
	TCMemoryManagerSlabShared<t_CParams> *TCMemoryManagerArena<t_CParams>::fp_CreateSlab(void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab)
	{
		uint8 *pMemory = (uint8 *)_pMemory;
		umint AlignedSize = fg_AlignUp(sizeof(TCMemoryManagerSlab<t_CParams, t_SlabType>), t_CParams::mc_SubSlabSize);
		TCMemoryManagerSlab<t_CParams, t_SlabType> *pSlabMem = TCMemoryManagerSlab<t_CParams, t_SlabType>::fs_CalcSlabLocation((uint8 *)(pMemory + t_CParams::mc_SlabSize - AlignedSize));

		umint iSubSlab = ((uint8 *)pSlabMem - pMemory) / t_CParams::mc_SubSlabSize;
		umint nSubSlabs = AlignedSize / t_CParams::mc_SubSlabSize;

		TCMemoryManagerSlab<t_CParams, t_SlabType> *pSlab;

		if (_pOldSlab)
		{
			DMibFastCheck(_pOldSlab->f_IsFullyFree());
			NContainer::TCBitArrayHierarchical<t_CParams::mc_MaxNumSubSlabs> OldCommitted;
			_pOldSlab->f_GetCommitted(OldCommitted);
			umint nCommittedHeaderSubSlabs = _pOldSlab->m_nCommittedHeaderSubSlabs;

			DMibFastCheck(nSubSlabs < 256);

			if (nSubSlabs > nCommittedHeaderSubSlabs)
			{
				umint nToCommit = nSubSlabs - nCommittedHeaderSubSlabs;
				if constexpr (mc_EnableCallbacks)
					m_pMemoryManager->f_OnCommit(pMemory + iSubSlab * t_CParams::mc_SubSlabSize, nToCommit * t_CParams::mc_SubSlabSize);
				OldCommitted.f_EnumFreeBitRanges
					(
						[&](umint _iSubSlab, umint _nSubSlabs)
						{
							m_pMemoryManager->m_Allocator.f_Commit(pMemory + _iSubSlab * t_CParams::mc_SubSlabSize, _nSubSlabs * t_CParams::mc_SubSlabSize);
							return true;
						}
						, iSubSlab
						, iSubSlab + nToCommit
					)
				;
				OldCommitted.template f_SetBitRange<false>(iSubSlab, nToCommit);
			}
			else if (nSubSlabs < nCommittedHeaderSubSlabs)
			{
				umint nAlreadyCommitted = nCommittedHeaderSubSlabs - nSubSlabs;
				OldCommitted.template f_SetBitRange<true>(iSubSlab - nAlreadyCommitted, nAlreadyCommitted);
			}
			pSlab = new((void *)pSlabMem) TCMemoryManagerSlab<t_CParams, t_SlabType>(m_Magic, this, nSubSlabs);
			pSlab->f_SetCommitted(OldCommitted);
		}
		else
		{
			m_pMemoryManager->m_Allocator.f_Commit(pMemory + iSubSlab * t_CParams::mc_SubSlabSize, nSubSlabs * t_CParams::mc_SubSlabSize);
			if constexpr (mc_EnableCallbacks)
				m_pMemoryManager->f_OnCommit(pMemory + iSubSlab * t_CParams::mc_SubSlabSize, nSubSlabs * t_CParams::mc_SubSlabSize);
			pSlab = new((void *)pSlabMem) TCMemoryManagerSlab<t_CParams, t_SlabType>(m_Magic, this, nSubSlabs);
		}

		return pSlab;
	}

	template <typename t_CParams>
	TCMemoryManagerSlabShared<t_CParams> * TCMemoryManagerArena<t_CParams>::fp_CreateSlabDynamic(uint32 _SlabType, void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab)
	{
		if constexpr (t_CParams::mc_NumSizesPerLevel == 16)
		{
			switch(_SlabType & 15)
			{
			case 0: return fp_CreateSlab<0>(_pMemory, _pOldSlab);
			case 1: return fp_CreateSlab<1>(_pMemory, _pOldSlab);
			case 2: return fp_CreateSlab<2>(_pMemory, _pOldSlab);
			case 3: return fp_CreateSlab<3>(_pMemory, _pOldSlab);
			case 4: return fp_CreateSlab<4>(_pMemory, _pOldSlab);
			case 5: return fp_CreateSlab<5>(_pMemory, _pOldSlab);
			case 6: return fp_CreateSlab<6>(_pMemory, _pOldSlab);
			case 7: return fp_CreateSlab<7>(_pMemory, _pOldSlab);
			case 8: return fp_CreateSlab<8>(_pMemory, _pOldSlab);
			case 9: return fp_CreateSlab<9>(_pMemory, _pOldSlab);
			case 10: return fp_CreateSlab<10>(_pMemory, _pOldSlab);
			case 11: return fp_CreateSlab<11>(_pMemory, _pOldSlab);
			case 12: return fp_CreateSlab<12>(_pMemory, _pOldSlab);
			case 13: return fp_CreateSlab<13>(_pMemory, _pOldSlab);
			case 14: return fp_CreateSlab<14>(_pMemory, _pOldSlab);
			case 15: return fp_CreateSlab<15>(_pMemory, _pOldSlab);
			}
			return nullptr;
		}
		else if constexpr (t_CParams::mc_NumSizesPerLevel == 8)
		{
			switch(_SlabType & 7)
			{
			case 0: return fp_CreateSlab<0>(_pMemory, _pOldSlab);
			case 1: return fp_CreateSlab<1>(_pMemory, _pOldSlab);
			case 2: return fp_CreateSlab<2>(_pMemory, _pOldSlab);
			case 3: return fp_CreateSlab<3>(_pMemory, _pOldSlab);
			case 4: return fp_CreateSlab<4>(_pMemory, _pOldSlab);
			case 5: return fp_CreateSlab<5>(_pMemory, _pOldSlab);
			case 6: return fp_CreateSlab<6>(_pMemory, _pOldSlab);
			case 7: return fp_CreateSlab<7>(_pMemory, _pOldSlab);
			}
			return nullptr;
		}
		else if constexpr (t_CParams::mc_NumSizesPerLevel == 4)
		{
			switch(_SlabType & 3)
			{
			case 0: return fp_CreateSlab<0>(_pMemory, _pOldSlab);
			case 1: return fp_CreateSlab<1>(_pMemory, _pOldSlab);
			case 2: return fp_CreateSlab<2>(_pMemory, _pOldSlab);
			case 3: return fp_CreateSlab<3>(_pMemory, _pOldSlab);
			}
			return nullptr;
		}
		else if constexpr (t_CParams::mc_NumSizesPerLevel == 2)
		{
			switch(_SlabType & 1)
			{
			case 0: return fp_CreateSlab<0>(_pMemory, _pOldSlab);
			case 1: return fp_CreateSlab<1>(_pMemory, _pOldSlab);
			}
			return nullptr;
		}
		else if constexpr (t_CParams::mc_NumSizesPerLevel == 1)
		{
			return fp_CreateSlab<0>(_pMemory, _pOldSlab);
		}
		else
		{
			static_assert(t_CParams::mc_NumSizesPerLevel == 0, "Unsupported sizes per level");
		}
	}

	template <typename t_CParams>
	TCMemoryManagerSlabShared<t_CParams> *TCMemoryManagerArena<t_CParams>::fp_NewSlab(uint32 _SlabType, uint32 _SizeType)
	{
		TCMemoryManagerSlabShared<t_CParams> *pSlab;

		aint iFree;
		if constexpr (t_CParams::mc_bFullGarbageCollect)
		{
			// Do full garbage collection
			fp_GarbageCollectFull();
			iFree = m_PartiallyFreeSlabsAvailable[_SlabType].f_FindUpperBound(_SizeType);
		}
		else
		{
			// Only collect garbage that is relevant to this slab type
			iFree = m_PartiallyFreeSlabsAvailable[_SlabType].f_FindUpperBound(_SizeType);
			while (iFree < 0 && fp_GarbageCollect(_SlabType))
				iFree = m_PartiallyFreeSlabsAvailable[_SlabType].f_FindUpperBound(_SizeType);
		}

		if (iFree >= 0)
		{
			DMibFastCheck(iFree <= t_CParams::mc_NumSubSlabSizeLevels);
			pSlab = m_PartiallyFreeSlabs[_SlabType][iFree].f_GetFirst();
			DMibFastCheck(pSlab);
			DMibFastCheck(pSlab->f_HasFreeBit(_SizeType));
			return pSlab;
		}

		auto pFreeSlab = m_FreeSlabs.f_Pop();

		if (!pFreeSlab)
		{
			//fp_GarbageCollectFull();
			DMibLock(m_pNumaArena->m_FreeSlabsLock);
			pFreeSlab = m_pNumaArena->m_FreeSlabs.f_Pop();
			if (pFreeSlab)
			{
				pFreeSlab->m_LinkNeedDecommit.f_Unlink();
				DMibFastCheck(!pFreeSlab->m_LinkToGarbageCollect.f_IsInList());
			}
		}
		else
		{
			pFreeSlab->m_LinkNeedDecommit.f_Unlink();
			DMibFastCheck(!pFreeSlab->m_LinkToGarbageCollect.f_IsInList());
		}

		void *pMemory;
		if (pFreeSlab)
			pMemory = fg_AlignDown((uint8 *)pFreeSlab, t_CParams::mc_SlabSize);
		else
		{
			pMemory = m_pMemoryManager->m_Allocator.f_AllocAligned
				(
					t_CParams::mc_SlabSize
					, t_CParams::mc_SlabSize
					, (t_CParams::CAllocator::f_CanCommit() ? EAllocationFlag_NoCommit : EAllocationFlag_None) | EAllocationFlag_WillFreeWithSize | t_CParams::mc_AllocationFlags
					, m_NumaNode
				)
			;

			if constexpr (mc_EnableCallbacks)
				m_pMemoryManager->f_OnDecommit((uint8 *)pMemory, t_CParams::mc_SlabSize);
		}

		pSlab = fp_CreateSlabDynamic(_SlabType, pMemory, pFreeSlab);

		m_FreeSlabs.f_InsertFirst(pSlab);

		return pSlab;
	}
}
