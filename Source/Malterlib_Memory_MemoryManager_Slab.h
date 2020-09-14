// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Container/BitArrayHierarchical>
#include <Mib/Container/BitArrayPowerTwo>

#include "Malterlib_Memory_MemoryManager_SubSlab.h"

namespace NMib::NMemory
{
	template <typename t_CParams>
	struct TCMemoryManagerArena;

	template <typename t_CParams>
	struct TCMemoryManager;
	

	struct CMemoryManagerSlabSharedPostfixHeader
	{
		uint32 m_SlabStartOffset;
		uint64 m_Magic;

		DMibSuppressThreadSanitizer inline_always uint64 f_GetMagic() const
		{
			return m_Magic;
		}
	};

	template <typename t_CParams>
	struct TCMemoryManagerSubSlabDataAlloc
	{
		static constexpr mint mc_nAllocBits = NMib::fg_GetHighestBitSetNoZero(t_CParams::mc_MaxAllocsPerSubSlabActual) + 1;
		static constexpr mint mc_MaxAllocs = DMibBit(mc_nAllocBits) - 1;

		using CStorageType = typename NTraits::TCUnsigned<typename NTraits::TCIntFromSizeLarger<(mc_nAllocBits + 7) / 8>::CType>::CType;

		CStorageType m_nAllocs;
	};

	template <typename t_CParams>
	struct TCMemoryManagerSubSlabDataType
	{
		static constexpr mint mc_nTypeBits = NMib::fg_GetHighestBitSetNoZero(t_CParams::mc_NumSizeLevels + 3) + 1;
		static constexpr mint mc_MaxType = DMibBit(mc_nTypeBits) - 1;

		using CStorageType = typename NTraits::TCUnsigned<typename NTraits::TCIntFromSizeLarger<(mc_nTypeBits + 7) / 8>::CType>::CType;

		CStorageType m_Type;
	};

	struct CMemoryManagerSubSlabFreeList
	{
	};

	template <typename t_CParams>
	struct TCMemoryManagerSlabShared
	{
		TCMemoryManagerSlabShared(uint32 _SlabType, TCMemoryManagerArena<t_CParams> * _pArena, uint8 _nCommittedHeaderSubSlabs);
		virtual ~TCMemoryManagerSlabShared();

		TCMemoryManagerArena<t_CParams> *m_pArena;
		TCMemoryManager<t_CParams> *m_pMemoryManager;

		using CSubSlabIndex = typename t_CParams::CSubSlabIndex;

		int64 m_FreeTimestamp = 0;
		int64 m_NeedDecommitTimestamp = 0;
		int64 m_HasGarbageTimestamp = 0;

		uint32 m_SlabType;
		CSubSlabIndex m_nAllocatedSubSlabs;
		CSubSlabIndex m_nFreeSubSlabs;
		CSubSlabIndex m_nPendingSubSlabs;
		int8 m_FullySetLevel;
		uint8 m_nCommittedHeaderSubSlabs;

		// Keep these last so struct is aligned on pointer size
		DMibMemoryManagerLink(TCMemoryManagerSlabShared, m_Link);
		DMibMemoryManagerLink(TCMemoryManagerSlabShared, m_LinkToGarbageCollect);
		DMibMemoryManagerLink(TCMemoryManagerSlabShared, m_LinkNeedDecommit);
		DMibMemoryManagerList(CMemoryManagerSubSlab_Free, m_Link) m_FreeSubSlabs;

		TCMemoryManagerSubSlabDataAlloc<t_CParams> *m_pSubSlabDataAlloc;

		virtual aint f_FindFreeBitAndSet(mint _Level) = 0;
		[[nodiscard]] virtual bool f_SetBitFree(mint _Level, mint _Bit) = 0;
		virtual bool f_HasFreeBit(mint _Level) = 0;
		virtual mint f_GetNumSetBits(mint _Level) = 0;
		virtual bool f_IsFullyFree() = 0;

		virtual void f_SetPendingBit(mint _Bit) = 0;
		virtual bool f_ClearPendingBit(mint _Bit) = 0;
		virtual bool f_HasPendingBit() = 0;
		virtual mint f_GetNumPendingBits() = 0;
		virtual void f_EnumPendingBits(NFunction::TCFunctionNoAlloc<bool (mint _Bit)> const& _fCallback) = 0;

		virtual fp32 f_OverheadPerByte() const = 0;

		virtual void f_CommitSubSlabs(mint _iSubSlab, mint _nSubSlabs) = 0;
		virtual void f_OnCommitSubSlabs(mint _iSubSlab, mint _nSubSlabs) = 0;
		virtual void f_DecommitSubSlabs(mint _iSubSlab, mint _nSubSlabs) = 0;
		virtual void f_DecommitDeferred() = 0;
		virtual void f_GetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_MaxNumSubSlabs> & _Comitted) = 0;
		virtual void f_SetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_MaxNumSubSlabs> const& _Comitted) = 0;

		mint f_GetNumSubSlabs() const;
		mint f_GetSubSlabMultiplier() const;
		uint8 * f_GetSlabStart();
		TCMemoryManagerSubSlabDataType<t_CParams> *f_GetSubSlabDataType();
		TCMemoryManagerSubSlabDataAlloc<t_CParams> *f_GetSubSlabDataAlloc();

		uint8 const * f_GetSlabStart() const;
		TCMemoryManagerSubSlabDataType<t_CParams> const *f_GetSubSlabDataType() const;
		TCMemoryManagerSubSlabDataAlloc<t_CParams> const *f_GetSubSlabDataAlloc() const;

		template <mint t_SlabMultiplier>
		static constexpr mint fs_CalculateSubSlabs()
		{
			constexpr mint c_TheoreticalSubSlabs = (t_CParams::mc_SlabSize - 1) / (t_CParams::mc_SubSlabSize * t_SlabMultiplier);
			return
				(
					t_CParams::mc_SlabSize
					-
					(
						sizeof(TCMemoryManagerSlabShared<t_CParams>)
						+ sizeof(TCMemoryManagerSubSlabDataType<t_CParams>) * c_TheoreticalSubSlabs
						+ sizeof(TCMemoryManagerSubSlabDataAlloc<t_CParams>) * c_TheoreticalSubSlabs
						+ sizeof(NContainer::TCBitArrayPowerTwo<c_TheoreticalSubSlabs, t_CParams::mc_NumSubSlabSizeLevels, NContainer::TCBitArrayHierarchical>)
						+ sizeof(NContainer::TCBitArrayHierarchical<c_TheoreticalSubSlabs>) * 3
						+ sizeof(CMemoryManagerSlabSharedPostfixHeader)
					)
				)
				/ (t_CParams::mc_SubSlabSize * t_SlabMultiplier)
			;
		}
	};


	template <typename t_CParams, uint32 t_SlabType>
	struct TCMemoryManagerSlab : public TCMemoryManagerSlabShared<t_CParams>
	{
		static constexpr mint mc_SlabMultiplier = t_CParams::mc_SlabTypeInfo[t_SlabType].m_SubSlabMutiplier;
		static constexpr mint mc_nSubSlabs = TCMemoryManagerSlabShared<t_CParams>::template fs_CalculateSubSlabs<mc_SlabMultiplier>();
		static constexpr bool mc_EnableCallbacks = t_CParams::CNotifier::CArena::mc_EnableCallbacks;

		TCMemoryManagerSubSlabDataType<t_CParams> m_SubSlabDataType[mc_nSubSlabs]; // This one has to stay first
		TCMemoryManagerSubSlabDataAlloc<t_CParams> m_SubSlabDataAlloc[mc_nSubSlabs];

		NContainer::TCBitArrayPowerTwo<mc_nSubSlabs, t_CParams::mc_NumSubSlabSizeLevels, NContainer::TCBitArrayHierarchical> m_Allocated;

		NContainer::TCBitArrayHierarchical<mc_nSubSlabs> m_PendingFree;

		NContainer::TCBitArrayHierarchical<mc_nSubSlabs> m_CommittedSubSlabs;
		NContainer::TCBitArrayHierarchical<mc_nSubSlabs> m_DeferredDecommitSubSlabs;

		CMemoryManagerSlabSharedPostfixHeader m_PostfixHeader;

/*			virtual TCMemoryManagerSubSlabData<t_CParams> *f_GetSubSlabData() override
		{
			return m_SubSlabData;
		}*/

		TCMemoryManagerSlab(uint64 _Magic, TCMemoryManagerArena<t_CParams> * _pArena, uint8 _nCommittedHeaderSubSlabs);
		~TCMemoryManagerSlab() override;

		aint f_FindFreeBitAndSet(mint _Level) override;
		bool f_SetBitFree(mint _Level, mint _Bit) override;
		bool f_HasFreeBit(mint _Level) override;
		mint f_GetNumSetBits(mint _Level) override;
		bool f_IsFullyFree() override;


		void f_SetPendingBit(mint _Bit) override;
		bool f_ClearPendingBit(mint _Bit) override;
		bool f_HasPendingBit() override;
		mint f_GetNumPendingBits() override;
		void f_EnumPendingBits(NFunction::TCFunctionNoAlloc<bool (mint _Bit)> const& _fCallback) override;

		void f_CommitSubSlabs(mint _iSubSlab, mint _nSubSlabs) override;
		void f_OnCommitSubSlabs(mint _iSubSlab, mint _nSubSlabs) override;
		void f_DecommitSubSlabs(mint _iSubSlab, mint _nSubSlabs) override;
		void f_DecommitDeferred() override;
		void f_GetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_MaxNumSubSlabs> & _Comitted) override;
		void f_SetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_MaxNumSubSlabs> const& _Comitted) override;

		fp32 f_OverheadPerByte() const override;

		static TCMemoryManagerSlab *fs_CalcSlabLocation(uint8 * _pLocation);
	};

	template <typename t_CParams, mint t_Index>
	static constexpr mint fg_MemoryManagerSlabSize()
	{
		return sizeof(TCMemoryManagerSlab<t_CParams, t_Index>);
	}
}
