// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	enum ESmallState
	{
		ESmallState_None
		, ESmallState_WasFull
		, ESmallState_IsFullyFree
	};

	template <typename t_CParams>
	struct TCMemoryManagerSubSlab_SmallSizeShared
	{
		struct CParams
		{
			CMemoryManagerSubSlab_SmallSizeLink m_Link;
			uint16 m_FirstFreeList;
			uint16 m_nAllocated;
			uint16 m_AllocSize;
			uint16 m_Alignment;
		};

		CParams m_Params;

		TCMemoryManagerSubSlab_SmallSizeShared(umint _AllocSize);

		uint8 *f_GetArray();
		void *f_Alloc(bool &_bFull);
		ESmallState f_Free(void *_pAlloc, umint _iAlloc);
		void f_OnAlloc(TCMemoryManagerArena<t_CParams> &_Arena, void *_pAlloc);
		void f_OnFree(TCMemoryManagerArena<t_CParams> &_Arena, void *_pAlloc);
		void f_OnFillFree(TCMemoryManagerArena<t_CParams> &_Arena);
		bool f_OnCheckFree(TCMemoryManagerArena<t_CParams> &_Arena, void *_pAlloc, EMemoryManagerCheckFlag _Flags);
		bool f_CheckFree(TCMemoryManagerArena<t_CParams> &_Arena, EMemoryManagerCheckFlag _Flags);
	};

	template <typename t_CParams, umint t_AllocSize>
	struct TCMemoryManagerSubSlab_SmallSize : TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>
	{
		using typename TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>::CParams;
		static constexpr umint mc_Alignment = 1 << gc_LowestBitSet<t_AllocSize>;
		static constexpr umint mc_NumAllocs = (t_CParams::mc_SubSlabSize - TCAlignUp<umint, sizeof(CParams), mc_Alignment>::mc_Value) / t_AllocSize;
		static constexpr umint mc_SmallSlabIndex
			=
			(
				t_AllocSize <= TCMemoryManagerArena<t_CParams>::mc_MinAlignment
				? NMib::gc_HighestBitSet<t_AllocSize>
				:
				(
					TCMemoryManagerArena<t_CParams>::mc_nSmallSizeSlabsAligned
					+ (t_AllocSize - (TCMemoryManagerArena<t_CParams>::mc_MinAlignment * 2)) / TCMemoryManagerArena<t_CParams>::mc_MinAlignment
				)
			)
		;
		static_assert(mc_NumAllocs < 0xFFFF, "Out of bounds");

		static_assert(!t_CParams::mc_bUseSmallSizes || mc_SmallSlabIndex < TCMemoryManagerArena<t_CParams>::mc_nSmallSizeSlabs, "Out of range");
		static_assert(!t_CParams::mc_bUseSmallSizes || t_AllocSize != 1);
		static_assert(!t_CParams::mc_bUseSmallSizes || t_AllocSize != 2 ||	mc_SmallSlabIndex == 1);
		static_assert(!t_CParams::mc_bUseSmallSizes || t_AllocSize != 4 ||	mc_SmallSlabIndex == 2);
		static_assert(!t_CParams::mc_bUseSmallSizes || t_AllocSize != 8 ||	mc_SmallSlabIndex == 3);
		static_assert(!t_CParams::mc_bUseSmallSizes || t_AllocSize != 12 ||	mc_SmallSlabIndex == 4);
		static_assert(!t_CParams::mc_bUseSmallSizes || t_AllocSize != 16 ||	mc_SmallSlabIndex == 5);

		TCMemoryManagerSubSlab_SmallSize();
	};

	template <typename t_CParams>
	struct TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>
	{
		struct CParams
		{
			CMemoryManagerSubSlab_SmallSizeLink m_Link;
			uint8 m_FirstFreeList;
			uint8 m_FreeListsNext[(t_CParams::mc_SubSlabSize + 254) / 255];
			uint8 m_FreeLists[(t_CParams::mc_SubSlabSize + 254) / 255];
			uint16 m_nAllocated;
		};

		CParams m_Params;

		static constexpr umint mc_NumAllocs = (t_CParams::mc_SubSlabSize - sizeof(CParams));
		static constexpr umint mc_NumAllocRegions = (mc_NumAllocs + 254) / 255;
		static constexpr umint mc_SmallSlabIndex = 0;
		static_assert(mc_NumAllocRegions < 256, "Out of bounds");

		TCMemoryManagerSubSlab_SmallSize();

		uint8 *f_GetArray();
		void *f_Alloc(bool &_bFull);
		ESmallState f_Free(void *_pAlloc);
		void f_OnAlloc(TCMemoryManagerArena<t_CParams> &_Arena, void *_pAlloc);
		void f_OnFree(TCMemoryManagerArena<t_CParams> &_Arena, void *_pAlloc);
		void f_OnFillFree(TCMemoryManagerArena<t_CParams> &_Arena);
		bool f_OnCheckFree(TCMemoryManagerArena<t_CParams> &_Arena, void *_pAlloc, EMemoryManagerCheckFlag _Flags);
		bool f_CheckFree(TCMemoryManagerArena<t_CParams> &_Arena, EMemoryManagerCheckFlag _Flags);
	};
}
