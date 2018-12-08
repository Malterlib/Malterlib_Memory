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
		mint AlignedSize = fg_AlignUp(sizeof(TCMemoryManagerSlab<t_CParams, t_SlabType>), t_CParams::mc_SubSlabSize);
		TCMemoryManagerSlab<t_CParams, t_SlabType> *pSlabMem = TCMemoryManagerSlab<t_CParams, t_SlabType>::fs_CalcSlabLocation((uint8 *)(pMemory + t_CParams::mc_SlabSize - AlignedSize));

		mint iSubSlab = ((uint8 *)pSlabMem - pMemory) / t_CParams::mc_SubSlabSize;
		mint nSubSlabs = AlignedSize / t_CParams::mc_SubSlabSize;

		TCMemoryManagerSlab<t_CParams, t_SlabType> *pSlab;

		if (_pOldSlab)
		{
			DMibFastCheck(_pOldSlab->f_IsFullyFree());
			NContainer::TCBitArrayHierarchical<t_CParams::mc_NumSubSlabs> OldCommitted;
			_pOldSlab->f_GetCommitted(OldCommitted);

			if (nSubSlabs > _pOldSlab->m_nCommittedHeaderSlabs)
				m_pMemoryManager->m_Allocator.f_Commit(pMemory + iSubSlab * t_CParams::mc_SubSlabSize, (nSubSlabs - _pOldSlab->m_nCommittedHeaderSlabs) * t_CParams::mc_SubSlabSize);
			else if (nSubSlabs < _pOldSlab->m_nCommittedHeaderSlabs)
			{
				mint nToDecommit = _pOldSlab->m_nCommittedHeaderSlabs - nSubSlabs;
				m_pMemoryManager->m_Allocator.f_Decommit(pMemory + (iSubSlab - nToDecommit) * t_CParams::mc_SubSlabSize, nToDecommit * t_CParams::mc_SubSlabSize);
			}
			pSlab = new((void *)pSlabMem) TCMemoryManagerSlab<t_CParams, t_SlabType>(m_Magic, this);
			pSlab->f_SetCommitted(OldCommitted);
		}
		else
		{
			m_pMemoryManager->m_Allocator.f_Commit(pMemory + iSubSlab * t_CParams::mc_SubSlabSize, nSubSlabs * t_CParams::mc_SubSlabSize);
			pSlab = new((void *)pSlabMem) TCMemoryManagerSlab<t_CParams, t_SlabType>(m_Magic, this);
		}

		pSlab->m_nCommittedHeaderSlabs = nSubSlabs;
		return pSlab;
	}

	template <typename t_CParams>
	template <mint tf_nSlabSizes>
	typename TCEnableIf<tf_nSlabSizes == 8, TCMemoryManagerSlabShared<t_CParams> *>::CType TCMemoryManagerArena<t_CParams>::fp_CreateSlabDynamic(uint32 _SlabType, void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab)
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

	template <typename t_CParams>
	template <mint tf_nSlabSizes>
	typename TCEnableIf<tf_nSlabSizes == 4, TCMemoryManagerSlabShared<t_CParams> *>::CType TCMemoryManagerArena<t_CParams>::fp_CreateSlabDynamic(uint32 _SlabType, void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab)
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

	template <typename t_CParams>
	template <mint tf_nSlabSizes>
	typename TCEnableIf<tf_nSlabSizes == 2, TCMemoryManagerSlabShared<t_CParams> *>::CType TCMemoryManagerArena<t_CParams>::fp_CreateSlabDynamic(uint32 _SlabType, void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab)
	{
		switch(_SlabType & 1)
		{
		case 0: return fp_CreateSlab<0>(_pMemory, _pOldSlab);
		case 1: return fp_CreateSlab<1>(_pMemory, _pOldSlab);
		}
		return nullptr;
	}

	template <typename t_CParams>
	template <mint tf_nSlabSizes>
	typename TCEnableIf<tf_nSlabSizes == 1, TCMemoryManagerSlabShared<t_CParams> *>::CType TCMemoryManagerArena<t_CParams>::fp_CreateSlabDynamic(uint32 _SlabType, void *_pMemory, TCMemoryManagerSlabShared<t_CParams> *_pOldSlab)
	{
		return fp_CreateSlab<0>(_pMemory, _pOldSlab);
	}

	template <typename t_CParams>
	TCMemoryManagerSlabShared<t_CParams> *TCMemoryManagerArena<t_CParams>::fp_NewSlab(uint32 _SlabType, uint32 _SizeType)
	{
		TCMemoryManagerSlabShared<t_CParams> *pSlab;

#if 1
		// Do full garbage collection
		fp_GarbageCollectFull();
		aint iFree = m_PartiallyFreeSlabsAvailable[_SlabType].f_FindUpperBound(_SizeType);

#else
		// Only collect garbage that is relevant to this slab type
		aint iFree = m_PartiallyFreeSlabsAvailable[_SlabType].f_FindUpperBound(_SizeType);
		while (iFree < 0 && fp_GarbageCollect(_SlabType))
			iFree = m_PartiallyFreeSlabsAvailable[_SlabType].f_FindUpperBound(_SizeType);
#endif

		if (iFree >= 0)
		{
			DMibFastCheck(iFree <= mc_NumSubSlabSizeLevels);
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
				pFreeSlab->m_Link2.f_Unlink();
		}
		else
			pFreeSlab->m_Link2.f_Unlink();

		void *pMemory;
		if (pFreeSlab)
		{
			pMemory = fg_AlignDown((uint8 *)pFreeSlab, t_CParams::mc_SlabSize);
		}
		else
		{
			pMemory = m_pMemoryManager->m_Allocator.f_AllocAligned
				(
					t_CParams::mc_SlabSize
					, t_CParams::mc_SlabSize
					, (m_pMemoryManager->m_Allocator.f_CanCommit() ? EAllocationFlag_NoCommit : EAllocationFlag_None) | EAllocationFlag_WillFreeWithSize | t_CParams::mc_AllocationFlags
					, m_NumaNode
				)
			;
		}

		pSlab = fp_CreateSlabDynamic<t_CParams::mc_NumSizesPerLevel>(_SlabType, pMemory, pFreeSlab);

		m_FreeSlabs.f_InsertFirst(pSlab);

		return pSlab;
	}
}
