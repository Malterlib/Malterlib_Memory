// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
	{
		
		template <typename t_CParams, uint32 t_SlabType>
		void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_CommitSubSlabs(mint _iSubSlab, mint _nSubSlabs)
		{
			if (this->m_pMemoryManager->m_Allocator.f_CanCommit())
			{
				auto * pMemory = this->f_GetSlabStart();
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
				if (t_CParams::mc_DeferCleanup & EDeferCleanup_Commit)
				{
					m_DeferredDecommitSubSlabs.template f_SetBitRange<false>(_iSubSlab, _nSubSlabs);
					
					if (this->m_Link2.f_IsInList() && m_DeferredDecommitSubSlabs.f_IsFullyFree())
						this->m_Link2.f_Unlink();
				}
			}
		}
		
		
		template <typename t_CParams, uint32 t_SlabType>
		void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_DecommitSubSlabs(mint _iSubSlab, mint _nSubSlabs)
		{
			if (this->m_pMemoryManager->m_Allocator.f_CanCommit())
			{
				if (t_CParams::mc_DeferCleanup & EDeferCleanup_Commit)
				{
					m_DeferredDecommitSubSlabs.template f_SetBitRange<true>(_iSubSlab, _nSubSlabs);
					if (t_CParams::mc_bBackgroundCleanup)
						this->m_NeedDecommitTimestamp = this->m_pArena->m_pNumaArena->f_GetTimestamp();
					this->m_pArena->fp_RequestCleanup();
					if (!this->m_Link2.f_IsInList())
						this->m_pArena->m_SlabsNeedingDecommit.f_Insert(this);
				}
				else
				{
					auto * pMemory = this->f_GetSlabStart();
					m_CommittedSubSlabs.f_EnumSetBitRanges
						(
							[&](mint _Bit, mint _nBits) -> bool
							{
								this->m_pMemoryManager->m_Allocator.f_Decommit(pMemory + _Bit * t_CParams::mc_SubSlabSize * mc_SlabMultiplier, _nBits * t_CParams::mc_SubSlabSize * mc_SlabMultiplier);
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
		void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_GetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_NumSubSlabs> & _Comitted)
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
		void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_SetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_NumSubSlabs> const& _Comitted)
		{
			_Comitted.f_EnumSetBitRanges
				(
					[&](mint _Bit, mint _nBits) -> bool
					{
						mint StartBit = (_Bit + mc_SlabMultiplier - 1) / mc_SlabMultiplier;
						mint EndBit = (_Bit + _nBits) / mc_SlabMultiplier;
						if (StartBit < EndBit)
						{
							m_CommittedSubSlabs.template f_SetBitRange<true>(StartBit, EndBit - StartBit);
							if (t_CParams::mc_DeferCleanup & EDeferCleanup_Commit)
								m_DeferredDecommitSubSlabs.template f_SetBitRange<true>(StartBit, EndBit - StartBit);
						}
						return true;
					}
					, 0
					, mc_SubSlabs * mc_SlabMultiplier
				)
			;
			
			if (t_CParams::mc_DeferCleanup & EDeferCleanup_Commit)
			{
				if (!this->m_Link2.f_IsInList() && !m_DeferredDecommitSubSlabs.f_IsFullyFree())
				{
					if (t_CParams::mc_bBackgroundCleanup)
						this->m_NeedDecommitTimestamp = this->m_pArena->m_pNumaArena->f_GetTimestamp();
					this->m_pArena->fp_RequestCleanup();
					this->m_pArena->m_SlabsNeedingDecommit.f_Insert(this);
				}
			}
		}
		
		template <typename t_CParams, uint32 t_SlabType>
		void TCMemoryManagerSlab<t_CParams, t_SlabType>::f_DecommitDeferred()
		{
			if (!this->m_pMemoryManager->m_Allocator.f_CanCommit() || !(t_CParams::mc_DeferCleanup & EDeferCleanup_Commit))
				return;
			
			auto * pMemory = this->f_GetSlabStart();
			
			m_DeferredDecommitSubSlabs.f_EnumSetBitRanges
				(
					[&](mint _Bit, mint _nBits) -> bool
					{
						m_CommittedSubSlabs.template f_SetBitRange<false>(_Bit, _nBits);
						this->m_pMemoryManager->m_Allocator.f_Decommit(pMemory + _Bit * t_CParams::mc_SubSlabSize * mc_SlabMultiplier, _nBits * t_CParams::mc_SubSlabSize * mc_SlabMultiplier);
						return true;
					}
				)
			;
			m_DeferredDecommitSubSlabs.template f_SetBitRange<false>(0, mc_SubSlabs);
			
			this->m_Link2.f_Unlink();
		}		
	}
}