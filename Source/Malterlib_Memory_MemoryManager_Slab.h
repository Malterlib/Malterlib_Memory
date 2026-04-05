// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

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

#ifdef DMibSanitizerEnabled_Thread
		DMibSuppressThreadSanitizer inline_never
#else
		inline_always
#endif
		uint64 f_GetMagic() const
		{
			return m_Magic;
		}
	};

	template <typename t_CParams>
	struct TCMemoryManagerSubSlabDataAlloc
	{
		static constexpr umint mc_nAllocBits = NMib::fg_GetHighestBitSetNoZero(t_CParams::mc_MaxAllocsPerSubSlabActual) + 1;
		static constexpr umint mc_MaxAllocs = DMibBit(mc_nAllocBits) - 1;

		using CStorageType = NTraits::TCUnsigned<NTraits::TCIntFromSizeLarger<(mc_nAllocBits + 7) / 8>>;

		CStorageType m_nAllocs;
	};

	template <typename t_CParams>
	struct TCMemoryManagerSubSlabDataType
	{
		static constexpr umint mc_nTypeBits = NMib::fg_GetHighestBitSetNoZero(t_CParams::mc_NumSizeLevels + 3) + 1;
		static constexpr umint mc_MaxType = DMibBit(mc_nTypeBits) - 1;

		using CStorageType = NTraits::TCUnsigned<NTraits::TCIntFromSizeLarger<(mc_nTypeBits + 7) / 8>>;

		CStorageType m_Type;
	};

	struct CMemoryManagerSubSlabFreeList
	{
	};

	template <typename t_CParams>
	struct alignas(16) TCMemoryManagerSlabShared
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

		virtual aint f_FindFreeBitAndSet(umint _Level) = 0;
		[[nodiscard]] virtual bool f_SetBitFree(umint _Level, umint _Bit) = 0;
		virtual bool f_HasFreeBit(umint _Level) = 0;
		virtual umint f_GetNumSetBits(umint _Level) = 0;
		virtual bool f_IsFullyFree() = 0;

		virtual void f_SetPendingBit(umint _Bit) = 0;
		virtual bool f_ClearPendingBit(umint _Bit) = 0;
		virtual bool f_HasPendingBit() = 0;
		virtual umint f_GetNumPendingBits() = 0;
		virtual void f_EnumPendingBits(NFunction::TCFunctionNoAlloc<bool (umint _Bit)> const& _fCallback) = 0;

		virtual fp32 f_OverheadPerByte() const = 0;

		virtual void f_CommitSubSlabs(umint _iSubSlab, umint _nSubSlabs) = 0;
		virtual void f_OnCommitSubSlabs(umint _iSubSlab, umint _nSubSlabs) = 0;
		virtual void f_DecommitSubSlabs(umint _iSubSlab, umint _nSubSlabs) = 0;
		virtual void f_DecommitDeferred() = 0;
		virtual void f_GetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_MaxNumSubSlabs> & _Comitted) = 0;
		virtual void f_SetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_MaxNumSubSlabs> const& _Comitted) = 0;

		umint f_GetNumSubSlabs() const;
		umint f_GetSubSlabMultiplier() const;
		uint8 * f_GetSlabStart();
		TCMemoryManagerSubSlabDataType<t_CParams> *f_GetSubSlabDataType();
		TCMemoryManagerSubSlabDataAlloc<t_CParams> *f_GetSubSlabDataAlloc();

		uint8 const * f_GetSlabStart() const;
		TCMemoryManagerSubSlabDataType<t_CParams> const *f_GetSubSlabDataType() const;
		TCMemoryManagerSubSlabDataAlloc<t_CParams> const *f_GetSubSlabDataAlloc() const;

		template <umint t_SlabMultiplier>
		static constexpr umint fs_CalculateSubSlabs()
		{
			constexpr umint c_TheoreticalSubSlabs = (t_CParams::mc_SlabSize - 1) / (t_CParams::mc_SubSlabSize * t_SlabMultiplier);
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
	struct alignas(16) TCMemoryManagerSlab : public TCMemoryManagerSlabShared<t_CParams>
	{
		static constexpr umint mc_SlabMultiplier = t_CParams::mc_SlabTypeInfo[t_SlabType].m_SubSlabMutiplier;
		static constexpr umint mc_nSubSlabs = TCMemoryManagerSlabShared<t_CParams>::template fs_CalculateSubSlabs<mc_SlabMultiplier>();
		static constexpr bool mc_EnableCallbacks = t_CParams::CNotifier::CArena::mc_EnableCallbacks;

		alignas(16) TCMemoryManagerSubSlabDataType<t_CParams> m_SubSlabDataType[mc_nSubSlabs]; // This one has to stay first
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

		aint f_FindFreeBitAndSet(umint _Level) override;
		bool f_SetBitFree(umint _Level, umint _Bit) override;
		bool f_HasFreeBit(umint _Level) override;
		umint f_GetNumSetBits(umint _Level) override;
		bool f_IsFullyFree() override;


		void f_SetPendingBit(umint _Bit) override;
		bool f_ClearPendingBit(umint _Bit) override;
		bool f_HasPendingBit() override;
		umint f_GetNumPendingBits() override;
		void f_EnumPendingBits(NFunction::TCFunctionNoAlloc<bool (umint _Bit)> const& _fCallback) override;

		void f_CommitSubSlabs(umint _iSubSlab, umint _nSubSlabs) override;
		void f_OnCommitSubSlabs(umint _iSubSlab, umint _nSubSlabs) override;
		void f_DecommitSubSlabs(umint _iSubSlab, umint _nSubSlabs) override;
		void f_DecommitDeferred() override;
		void f_GetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_MaxNumSubSlabs> & _Comitted) override;
		void f_SetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_MaxNumSubSlabs> const& _Comitted) override;

		fp32 f_OverheadPerByte() const override;

		static TCMemoryManagerSlab *fs_CalcSlabLocation(uint8 * _pLocation);
	};

	template <typename t_CParams, umint t_Index>
	static constexpr umint fg_MemoryManagerSlabSize()
	{
		return sizeof(TCMemoryManagerSlab<t_CParams, t_Index>);
	}
}
