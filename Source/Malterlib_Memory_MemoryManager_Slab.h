// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Container/BitArrayHierarchical>
#include <Mib/Container/BitArrayPowerTwo>

namespace NMib
{
	namespace NMem
	{
		struct CMemoryManagerSlabSharedPostfixHeader
		{
			uint32 m_SlabStartOffset;
			uint64 m_Magic;
		};

		struct CMemoryManagerAllocatedData
		{
			uint16 m_nAllocs:11;	// 0 - 1024
			uint16 m_Type:5;		// 0 - 20
			static CMemoryManagerAllocatedData fs_Create(uint16 _nAllocs, uint16 _Type)
			{
				CMemoryManagerAllocatedData Ret;
				Ret.m_nAllocs = _nAllocs;
				Ret.m_Type = _Type;
				return Ret;
			}
		};

		struct CMemoryManagerSubSlabData
		{
			union
			{
				//CNextFreeData m_Free;
				CMemoryManagerAllocatedData m_Allocated;
				uint16 m_RawData;
			};
		};

		template <typename t_CParams>
		struct TCMemoryManagerSlabShared
		{
		public:

			TCMemoryManagerSlabShared(uint32 _SlabType, TCMemoryManagerArena<t_CParams> * _pArena);
			virtual ~TCMemoryManagerSlabShared();

			TCMemoryManagerArena<t_CParams> * m_pArena;
			TCMemoryManager<t_CParams> * m_pMemoryManager;

			int64 m_FreeTimestamp;
			int64 m_NeedDecommitTimestamp;
			int64 m_HasGarbageTimestamp;

			uint32 m_SlabType;
			int16 m_FullySetLevel;
			uint16 m_nAllocatedSubSlabs;
			uint16 m_nFreeSubSlabs;
			uint16 m_nPendingSubSlabs;
			uint16 m_nCommittedHeaderSlabs;

			// Keep these last so struct is aligned on pointer size
			DMibMemoryManagerLink(TCMemoryManagerSlabShared, m_Link0);
			DMibMemoryManagerLink(TCMemoryManagerSlabShared, m_Link1);
			DMibMemoryManagerLink(TCMemoryManagerSlabShared, m_Link2);
			DMibMemoryManagerList(CMemoryManagerSubSlab_Free, m_Link) m_FreeSubSlabs;
			
			virtual aint f_FindFreeBitAndSet(mint _Level) = 0;
			virtual void f_SetBitFree(mint _Level, mint _Bit) = 0;
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
			virtual void f_DecommitSubSlabs(mint _iSubSlab, mint _nSubSlabs) = 0;
			virtual void f_DecommitDeferred() = 0;
			virtual void f_GetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_NumSubSlabs> & _Comitted) = 0;
			virtual void f_SetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_NumSubSlabs> const& _Comitted) = 0;
			
			mint f_GetNumSubSlabs() const;
			mint f_GetSubSlabMultiplier() const;
			uint8 * f_GetSlabStart();
			CMemoryManagerSubSlabData * f_GetSubSlabData();

			uint8 const * f_GetSlabStart() const;
			CMemoryManagerSubSlabData const * f_GetSubSlabData() const;


		};

		template <typename t_CParams, uint32 t_SlabType>
		struct TCMemoryManagerSlab : public TCMemoryManagerSlabShared<t_CParams>
		{
		public:

			static constexpr mint mc_SlabMultiplier = t_CParams::template TCGetSlabInfo<t_SlabType>::mc_Multiplier;
			static constexpr mint mc_TheoreticalSubSlabs = (t_CParams::mc_SlabSize - 1) / (t_CParams::mc_SubSlabSize * mc_SlabMultiplier);
			static constexpr mint mc_SubSlabs 
				=
				(
					t_CParams::mc_SlabSize 
					- 
					(
						sizeof(TCMemoryManagerSlabShared<t_CParams>)
						+ sizeof(NContainer::TCBitArrayPowerTwo<mc_TheoreticalSubSlabs, TCMemoryManagerArena<t_CParams>::mc_NumSubSlabSizeLevels, NContainer::TCBitArrayHierarchical>)
						+ sizeof(NContainer::TCBitArrayHierarchical<mc_TheoreticalSubSlabs>)
						+ sizeof(CMemoryManagerSubSlabData) * mc_TheoreticalSubSlabs
						+ sizeof(CMemoryManagerSlabSharedPostfixHeader)
					)
				) 
				/ (t_CParams::mc_SubSlabSize * mc_SlabMultiplier)
			;

			CMemoryManagerSubSlabData m_SubSlabData[mc_SubSlabs]; // This one has to stay first
			
			NContainer::TCBitArrayPowerTwo<mc_SubSlabs, TCMemoryManagerArena<t_CParams>::mc_NumSubSlabSizeLevels, NContainer::TCBitArrayHierarchical> m_Allocated;

			NContainer::TCBitArrayHierarchical<mc_SubSlabs> m_PendingFree;
			
			NContainer::TCBitArrayHierarchical<mc_SubSlabs> m_CommittedSubSlabs;
			NContainer::TCBitArrayHierarchical<mc_SubSlabs> m_DeferredDecommitSubSlabs;

			CMemoryManagerSlabSharedPostfixHeader m_PostfixHeader;
			
/*			virtual CMemoryManagerSubSlabData *f_GetSubSlabData() override
			{
				return m_SubSlabData;
			}*/

			TCMemoryManagerSlab(uint64 _Magic, TCMemoryManagerArena<t_CParams> * _pArena);
			~TCMemoryManagerSlab() override;

			aint f_FindFreeBitAndSet(mint _Level) override;
			void f_SetBitFree(mint _Level, mint _Bit) override;
			bool f_HasFreeBit(mint _Level) override;
			mint f_GetNumSetBits(mint _Level) override;
			bool f_IsFullyFree() override;


			void f_SetPendingBit(mint _Bit) override;
			bool f_ClearPendingBit(mint _Bit) override;
			bool f_HasPendingBit() override;
			mint f_GetNumPendingBits() override;
			void f_EnumPendingBits(NFunction::TCFunctionNoAlloc<bool (mint _Bit)> const& _fCallback) override;
			
			void f_CommitSubSlabs(mint _iSubSlab, mint _nSubSlabs) override;
			void f_DecommitSubSlabs(mint _iSubSlab, mint _nSubSlabs) override;
			void f_DecommitDeferred() override;
			void f_GetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_NumSubSlabs> & _Comitted) override;
			void f_SetCommitted(NContainer::TCBitArrayHierarchical<t_CParams::mc_NumSubSlabs> const& _Comitted) override;
			
			fp32 f_OverheadPerByte() const override;
			
			static TCMemoryManagerSlab *fs_CalcSlabLocation(uint8 * _pLocation);
		};
	}
}
