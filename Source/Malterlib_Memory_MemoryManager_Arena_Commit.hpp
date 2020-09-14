// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	template <typename t_CParams, uint32 t_SlabType>
	void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_OnCommitSubSlabs(mint _iSubSlab, mint _nSubSlabs)
	{
		if constexpr (mc_EnableCallbacks && t_CParams::CAllocator::f_CanCommit())
		{
			auto * pMemory = this->f_GetSlabStart();
			this->m_pMemoryManager->f_OnCommit(pMemory + _iSubSlab * t_CParams::mc_SubSlabSize * mc_SlabMultiplier, _nSubSlabs * t_CParams::mc_SubSlabSize * mc_SlabMultiplier);
		}
	}

	template <typename t_CParams, uint32 t_SlabType>
	void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_CommitSubSlabs(mint _iSubSlab, mint _nSubSlabs)
	{
		if constexpr (t_CParams::CAllocator::f_CanCommit())
		{
			auto * pMemory = this->f_GetSlabStart();
			if constexpr (mc_EnableCallbacks)
				this->m_pMemoryManager->f_OnCommit(pMemory + _iSubSlab * t_CParams::mc_SubSlabSize * mc_SlabMultiplier, _nSubSlabs * t_CParams::mc_SubSlabSize * mc_SlabMultiplier);

			m_CommittedSubSlabs.f_EnumFreeBitRanges
				(
					[&](mint _Bit, mint _nBits) -> bool
					{
						this->m_pMemoryManager->m_Allocator.f_Commit(pMemory + _Bit * t_CParams::mc_SubSlabSize * mc_SlabMultiplier, _nBits * t_CParams::mc_SubSlabSize * mc_SlabMultiplier);

						return true;
					}
					, _iSubSlab
					, _iSubSlab + _nSubSlabs
				)
			;

			m_CommittedSubSlabs.template f_SetBitRange<true>(_iSubSlab, _nSubSlabs);
			if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_Commit) != 0)
			{
				m_DeferredDecommitSubSlabs.template f_SetBitRange<false>(_iSubSlab, _nSubSlabs);

				if (this->m_LinkNeedDecommit.f_IsInList() && m_DeferredDecommitSubSlabs.f_IsFullyFree())
					this->m_LinkNeedDecommit.f_Unlink();
			}
		}
	}


	template <typename t_CParams, uint32 t_SlabType>
	void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_DecommitSubSlabs(mint _iSubSlab, mint _nSubSlabs)
	{
		if constexpr (t_CParams::CAllocator::f_CanCommit())
		{
			if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_Commit) != 0)
			{
				m_DeferredDecommitSubSlabs.template f_SetBitRange<true>(_iSubSlab, _nSubSlabs);
				if constexpr (t_CParams::mc_bBackgroundCleanup)
					this->m_NeedDecommitTimestamp = this->m_pArena->m_pNumaArena->f_GetTimestamp();
				this->m_pArena->fp_RequestCleanup();
				if (!this->m_LinkNeedDecommit.f_IsInList())
					this->m_pArena->m_SlabsNeedingDecommit.f_Insert(this);
			}
			else
			{
				auto * pMemory = this->f_GetSlabStart();
				m_CommittedSubSlabs.f_EnumSetBitRanges
					(
						[&](mint _Bit, mint _nBits) -> bool
						{
							this->m_pMemoryManager->m_Allocator.f_Decommit
								(
									pMemory + _Bit * t_CParams::mc_SubSlabSize * mc_SlabMultiplier
									, _nBits * t_CParams::mc_SubSlabSize * mc_SlabMultiplier
								)
							;
							if constexpr (mc_EnableCallbacks)
							{
								this->m_pMemoryManager->f_OnDecommit
									(
										pMemory + _Bit * t_CParams::mc_SubSlabSize * mc_SlabMultiplier
										, _nBits * t_CParams::mc_SubSlabSize * mc_SlabMultiplier
									)
								;
							}

							return true;
						}
						, _iSubSlab
						, _iSubSlab + _nSubSlabs
					)
				;

				m_CommittedSubSlabs.template f_SetBitRange<false>(_iSubSlab, _nSubSlabs);
			}
		}
	}

	template <typename t_CParams, uint32 t_SlabType>
	void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_GetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_MaxNumSubSlabs> & _Comitted)
	{
		m_CommittedSubSlabs.f_EnumSetBitRanges
			(
				[&](mint _Bit, mint _nBits) -> bool
				{
					_Comitted.template f_SetBitRange<true>(_Bit * mc_SlabMultiplier, _nBits * mc_SlabMultiplier);
					return true;
				}
			)
		;
	}

	template <typename t_CParams, uint32 t_SlabType>
	void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_SetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_MaxNumSubSlabs> const& _Comitted)
	{
		constexpr bool c_bHasWaste = (((sizeof(*this) + t_CParams::mc_SubSlabSize - 1) / t_CParams::mc_SubSlabSize) + mc_nSubSlabs * mc_SlabMultiplier) < t_CParams::mc_MaxNumSubSlabs;

		auto * pMemory = this->f_GetSlabStart();
		_Comitted.f_EnumSetBitRanges
			(
				[&](mint _Bit, mint _nBits) -> bool
				{
					mint StartBit = (_Bit) / mc_SlabMultiplier;
					mint EndBit = (_Bit + _nBits + mc_SlabMultiplier - 1) / mc_SlabMultiplier;
					DMibFastCheck(c_bHasWaste || EndBit <= mc_nSubSlabs);
					if constexpr (c_bHasWaste)
					{
						if (EndBit > mc_nSubSlabs)
						{
							mint StartCommit = fg_Max(_Bit, mc_nSubSlabs * mc_SlabMultiplier);
							mint EndCommit = _Bit + _nBits;
							if (StartCommit < EndCommit)
							{
								mint nToCommit = EndCommit - StartCommit;
								this->m_pMemoryManager->m_Allocator.f_Decommit(pMemory + StartCommit * t_CParams::mc_SubSlabSize, (nToCommit) * t_CParams::mc_SubSlabSize);
								if constexpr (mc_EnableCallbacks)
									this->m_pMemoryManager->f_OnDecommit(pMemory + StartCommit * t_CParams::mc_SubSlabSize, (nToCommit) * t_CParams::mc_SubSlabSize);

								_nBits -= nToCommit;
								--EndBit;
								DMibFastCheck(EndBit <= mc_nSubSlabs);
							}
						}
					}
					{
						mint StartCommit = StartBit * mc_SlabMultiplier;
						mint EndCommit = _Bit;
						if (StartCommit < EndCommit)
						{
							this->m_pMemoryManager->m_Allocator.f_Commit(pMemory + StartCommit * t_CParams::mc_SubSlabSize, (EndCommit - StartCommit) * t_CParams::mc_SubSlabSize);
							if constexpr (mc_EnableCallbacks)
								this->m_pMemoryManager->f_OnCommit(pMemory + StartCommit * t_CParams::mc_SubSlabSize, (EndCommit - StartCommit) * t_CParams::mc_SubSlabSize);
						}
					}
					{
						mint StartCommit = _Bit + _nBits;
						mint EndCommit = EndBit * mc_SlabMultiplier;
						if (StartCommit < EndCommit)
						{
							this->m_pMemoryManager->m_Allocator.f_Commit(pMemory + StartCommit * t_CParams::mc_SubSlabSize, (EndCommit - StartCommit) * t_CParams::mc_SubSlabSize);
							if constexpr (mc_EnableCallbacks)
								this->m_pMemoryManager->f_OnCommit(pMemory + StartCommit * t_CParams::mc_SubSlabSize, (EndCommit - StartCommit) * t_CParams::mc_SubSlabSize);
						}
					}
					if (StartBit < EndBit)
					{
						m_CommittedSubSlabs.template f_SetBitRange<true>(StartBit, EndBit - StartBit);
						if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_Commit) != 0)
							m_DeferredDecommitSubSlabs.template f_SetBitRange<true>(StartBit, EndBit - StartBit);
					}
					return true;
				}
				, 0
				, t_CParams::mc_MaxNumSubSlabs
			)
		;

		if constexpr ((t_CParams::mc_DeferCleanup & EDeferCleanup_Commit) != 0)
		{
			if (!this->m_LinkNeedDecommit.f_IsInList() && !m_DeferredDecommitSubSlabs.f_IsFullyFree())
			{
				if constexpr (t_CParams::mc_bBackgroundCleanup)
					this->m_NeedDecommitTimestamp = this->m_pArena->m_pNumaArena->f_GetTimestamp();
				this->m_pArena->fp_RequestCleanup();
				this->m_pArena->m_SlabsNeedingDecommit.f_Insert(this);
			}
		}
	}

	template <typename t_CParams, uint32 t_SlabType>
	void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_DecommitDeferred()
	{
		if constexpr (!t_CParams::CAllocator::f_CanCommit() || !(t_CParams::mc_DeferCleanup & EDeferCleanup_Commit))
			return;

		auto * pMemory = this->f_GetSlabStart();

		m_DeferredDecommitSubSlabs.f_EnumSetBitRanges
			(
				[&](mint _Bit, mint _nBits) -> bool
				{
					m_CommittedSubSlabs.template f_SetBitRange<false>(_Bit, _nBits);
					this->m_pMemoryManager->m_Allocator.f_Decommit(pMemory + _Bit * t_CParams::mc_SubSlabSize * mc_SlabMultiplier, _nBits * t_CParams::mc_SubSlabSize * mc_SlabMultiplier);
					if constexpr (mc_EnableCallbacks)
						this->m_pMemoryManager->f_OnDecommit(pMemory + _Bit * t_CParams::mc_SubSlabSize * mc_SlabMultiplier, _nBits * t_CParams::mc_SubSlabSize * mc_SlabMultiplier);

					return true;
				}
			)
		;
		m_DeferredDecommitSubSlabs.template f_SetBitRange<false>(0, mc_nSubSlabs);

		this->m_LinkNeedDecommit.f_Unlink();
	}
}
