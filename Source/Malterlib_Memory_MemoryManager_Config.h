// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#if 1
#	define DMibMemoryManagerLink DMibListLinkDS_Link
#	define DMibMemoryManagerList DMibListLinkDS_List
#	define DMibMemoryManagerList_FromTemplate DMibListLinkDS_List_FromTemplate
#else
#	define DMibMemoryManagerLink DMibListLinkD_Link
#	define DMibMemoryManagerList DMibListLinkD_List
#	define DMibMemoryManagerList_FromTemplate DMibListLinkD_List_FromTemplate
#endif

#include "Malterlib_Memory_MemoryManager_Slab.h"

#pragma once

namespace NMib::NMemory
{
	struct CSlabTypeInfo
	{
		uint8 m_SubSlabMutiplier;
	};

	enum EDeferCleanup
	{
		EDeferCleanup_None
		, EDeferCleanup_OneSizeBlocks = DMibBit(0)
		, EDeferCleanup_Commit = DMibBit(1)
		, EDeferCleanup_Allocs = DMibBit(2)
		, EDeferCleanup_NoCleanup = DMibBit(3)
	};

	enum EMemoryManagerCheckFlag
	{
		EMemoryManagerCheckFlag_None = 0
		, EMemoryManagerCheckFlag_Break = DMibBit(0)
		, EMemoryManagerCheckFlag_Unprotect = DMibBit(1)
		, EMemoryManagerCheckFlag_Protect = DMibBit(2)
		, EMemoryManagerCheckFlag_Default = EMemoryManagerCheckFlag_Break | EMemoryManagerCheckFlag_Unprotect
	};

	struct CDefaultMemoryManagerNotifier
	{
		struct CGlobal;

		struct CArena
		{
			// The arena notifier is called in the context of an arena so you can safely assume that this will only be called from one thread at a time

			enum
			{
				mc_EnableCallbacks = false
			};

			CArena(CGlobal *_pGlobal);

			void f_OnAlloc(uint8 *_pMemory, mint _nBytes);
			void f_OnFree(uint8 *_pMemory);
			void f_OnFreeOtherThread(uint8 *_pMemory);

			void f_OnFillFree(uint8 *_pMemory, mint _nBytes, EMemoryManagerCheckFlag _Flags = EMemoryManagerCheckFlag_Protect);
			bool f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, EMemoryManagerCheckFlag _Flags);
		};

		struct CHeap
		{
			// The heap notifiers are called when the heap lock is taken so you can assume thread safety

			enum
			{
				mc_EnableCallbacks = false
			};

			CHeap(CGlobal *_pGlobal);

			void f_OnAlloc(uint8 *_pMemory, mint _nBytes);
			void f_OnFree(uint8 *_pMemory);

			void f_OnFillFree(uint8 *_pMemory, mint _nBytes, EMemoryManagerCheckFlag _Flags = EMemoryManagerCheckFlag_Protect);
			bool f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, EMemoryManagerCheckFlag _Flags);
		};

		struct CGlobal
		{
			// The global notifier is called in the global context so you have to provide thread safety

			enum
			{
				mc_EnableCallbacks = false
			};

			template <typename tf_CMemoryManager>
			CGlobal(tf_CMemoryManager & _MemMan);

			void f_OnAlloc(uint8 *_pMemory, mint _nBytes);
			void f_OnFree(uint8 *_pMemory);

			void f_OnCommit(uint8 *_pMemory, mint _nBytes);
			void f_OnDecommit(uint8 *_pMemory, mint _nBytes);
		};
	};

#if defined(DPlatformFamily_macOS)
	static constexpr mint gc_OsMaxPageSize = 16*1024;
#else
	static constexpr mint gc_OsMaxPageSize = 4*1024;
#endif

	struct CDefaultMemoryManagerParams
	{
		static constexpr mint mc_NumSizesPerLevel = 8;
		static constexpr mint mc_SubSlabSize = gc_OsMaxPageSize;						// Should be the page size
		static constexpr bool mc_bRandomizeSlabHeader = false;
		static constexpr bool mc_bBackgroundCleanup = true;
		static constexpr EDeferCleanup mc_DeferCleanup = (EDeferCleanup)(EDeferCleanup_Allocs | EDeferCleanup_Commit | EDeferCleanup_OneSizeBlocks);

#if DMibPPtrBits >= 64
		static constexpr uint32 mc_MaxArenas = 1; // The number of max arenas if arenas are limited
#elif DMibPPtrBits == 32
		static constexpr uint32 mc_MaxArenas = 8; // The number of max arenas if arenas are limited
#else
#	error "Decide max arenas"
#endif

		static constexpr uint32 mc_BackgroundCleanupLifetime = 50; // The number of milleseconds that garbage should be kept before being cleaned up.
		static constexpr uint32 mc_BackgroundCleanupLifetimeDecommit = 500; // The number of milleseconds that garbage pages should be kept before being decommitted.

		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_None;
		static constexpr bool mc_bFullGarbageCollect  = false;
		static constexpr bool mc_bUseSmallSizes = true;
		static constexpr bool mc_bSpecialCaseSlabType0 = false;
		static constexpr bool mc_bUseFreeBlockCounting = false;
#ifdef DMibSanitizerEnabled_UndefinedBehavior
		static constexpr bool mc_bAllowUnalignedFreeList = false;
#else
		static constexpr bool mc_bAllowUnalignedFreeList = true;
#endif
		static constexpr bool mc_bUseSlabFromEnd = false;

#if (defined(DArchitecture_arm64) || defined(DArchitecture_arm64e)) && defined(DPlatformFamily_macOS)
		static constexpr mint mc_PreventCacheConflictSize = 64 * 1024 * 8 / 4;
#else
		static constexpr mint mc_PreventCacheConflictSize = 32 * 1024 * 8 / 4; // Use 0 to disable. Default = 32 KB 8-way / 4
#endif
		static constexpr mint mc_PreventCacheConflictSizeMaxOverhead = 32; // 1 / x maximum overhead

		static constexpr mint mc_MaxPendingSubSlabs = 0;

		using CAllocator = CAllocator_Virtual;
		using CNotifier = CDefaultMemoryManagerNotifier;
	};

	template <mint t_nSizesPerLevel>
	struct TCMemoryManagerParamsSizesPerLevel;

	template <>
	struct TCMemoryManagerParamsSizesPerLevel<16>
	{
		static constexpr CSlabTypeInfo mc_SlabTypeInfo[16] = {{1}, {17}, {9}, {19}, {5}, {21}, {11}, {23}, {3}, {25}, {13}, {27}, {7}, {29}, {15}, {31}};
		static constexpr uint16 mc_DivideMultiply[16] = {1, 61681, 58255, 55189, 52429, 49933, 47663, 45591, 43691, 41944, 40330, 38837, 37450, 36158, 34953, 33826};
		static constexpr uint8 mc_DivideShift[16] = {0, 20, 19, 20, 18, 20, 19, 20, 17, 20, 19, 20, 18, 20, 19, 20};
	};

	template <>
	struct TCMemoryManagerParamsSizesPerLevel<8>
	{
		static constexpr CSlabTypeInfo mc_SlabTypeInfo[8] = {{1}, {9}, {5}, {11}, {3}, {13}, {7}, {15}};
		static constexpr uint16 mc_DivideMultiply[8] = {1, 58255, 52429, 47663, 43691, 20165, 9363, 34953};
		static constexpr uint8 mc_DivideShift[8] = {0, 19, 18, 19, 17, 18, 16, 19};
	};

	template <>
	struct TCMemoryManagerParamsSizesPerLevel<4>
	{
		static constexpr CSlabTypeInfo mc_SlabTypeInfo[4] = {{1}, {5}, {3}, {7}};
		static constexpr uint16 mc_DivideMultiply[4] = {1, 52429, 43691, 9363};
		static constexpr uint8 mc_DivideShift[4] =  {0, 18, 17, 16};
	};

	template <>
	struct TCMemoryManagerParamsSizesPerLevel<2>
	{
		static constexpr CSlabTypeInfo mc_SlabTypeInfo[2] = {{1}, {3}};
		static constexpr uint16 mc_DivideMultiply[2] = {1, 43691};
		static constexpr uint8 mc_DivideShift[2]= {0, 17};
	};

	template <>
	struct TCMemoryManagerParamsSizesPerLevel<1>
	{
		static constexpr CSlabTypeInfo mc_SlabTypeInfo[1] = {{1}};
		static constexpr uint16 mc_DivideMultiply[1] = {1};
		static constexpr uint8 mc_DivideShift[1] = {0};
	};

	template <typename t_CParams = CDefaultMemoryManagerParams, typename t_CIndexList = typename NMeta::TCMakeConsecutiveIndices<t_CParams::mc_NumSizesPerLevel>::CType>
	struct TCMemoryManagerParams;

	struct CMemoryManagerParamsBase
	{
	};

	template <typename t_CParams, mint ...tp_Indices>
	struct TCMemoryManagerParams<t_CParams, NMeta::TCIndices<tp_Indices...>> : public t_CParams, TCMemoryManagerParamsSizesPerLevel<t_CParams::mc_NumSizesPerLevel>, CMemoryManagerParamsBase
	{
		static_assert(!NTraits::TCIsBaseOf<t_CParams, CMemoryManagerParamsBase>::mc_Value, "You shouldn't override TCMemoryManagerParams directly");

		static_assert
			(
				t_CParams::mc_NumSizesPerLevel == 16
				|| t_CParams::mc_NumSizesPerLevel == 8
				|| t_CParams::mc_NumSizesPerLevel == 4
				|| t_CParams::mc_NumSizesPerLevel == 2
				|| t_CParams::mc_NumSizesPerLevel == 1
				, "Unsupported option"
			)
		;

		static constexpr mint mc_SizesPerLevelShift = TCHighestBitSetCorrect<mint, t_CParams::mc_NumSizesPerLevel>::mc_Value;

		static constexpr mint mc_SlabSize = t_CParams::mc_SubSlabSize * 1024 * 4;			// Carefully choosen to minimize waste in different subslab types
		static constexpr mint mc_MaxNumSubSlabs = mc_SlabSize / t_CParams::mc_SubSlabSize;

		static constexpr mint mc_MaxHeapAllocSize = mc_SlabSize;
		static constexpr mint mc_MinHeapAllocSize = mc_SlabSize / 32;
		static constexpr mint mc_HeapChunkSize = mc_SlabSize * 2;
		static constexpr mint mc_HeapBlockSize = mc_MinHeapAllocSize / t_CParams::mc_NumSizesPerLevel;

		static constexpr mint mc_MaxSlabAllocSize = mc_MinHeapAllocSize - (mc_MinHeapAllocSize /2) / t_CParams::mc_NumSizesPerLevel;

		static constexpr mint mc_NumSizeLevels = TCHighestBitSetCorrect<mint, mc_MaxSlabAllocSize>::mc_Value + 1;
		static constexpr mint mc_NumNormalSizeLevels = mc_NumSizeLevels - 4;

		static constexpr mint mc_NumSubSlabSizeLevels = mc_NumSizeLevels - TCHighestBitSetCorrect<mint, t_CParams::mc_SubSlabSize>::mc_Value;

		static constexpr mint mc_MinAlignmentCalc = t_CParams::mc_bAllowUnalignedFreeList ? 16 / t_CParams::mc_NumSizesPerLevel : fg_Max(16 / t_CParams::mc_NumSizesPerLevel, sizeof(void *));
		static constexpr mint mc_MinNormalSizeAlignment = mc_MinAlignmentCalc < 4 ? 4 : mc_MinAlignmentCalc;

		static constexpr mint mc_SmallSizeSlabsLargestSize = t_CParams::mc_bUseSmallSizes ? (t_CParams::mc_bUseFreeBlockCounting ? 16 : 12) : 0;

		static constexpr mint mc_PreventCacheConflictSizeMaxBlockSize = t_CParams::mc_PreventCacheConflictSize / t_CParams::mc_PreventCacheConflictSizeMaxOverhead; // 1 / x maximum overhead

		static constexpr mint mc_MinNormalAllocSizeAfterSmallSize =
			fg_AlignUpConstExpr
			(
				mc_SmallSizeSlabsLargestSize + (fg_RoundPowerOfTwoDown(mc_SmallSizeSlabsLargestSize) / t_CParams::mc_NumSizesPerLevel)
				, mc_MinNormalSizeAlignment
			)
		;
		static constexpr mint mc_MinNormalAllocSize = fg_MaxConstexpr
			(
				fg_AlignUpConstExpr(t_CParams::mc_bUseFreeBlockCounting ? (sizeof(void *) * 2 + 4) : sizeof(void *) * 2, mc_MinNormalSizeAlignment)
				, mc_MinNormalAllocSizeAfterSmallSize
			)
		;

		using CSubSlabIndex = typename NTraits::TCUnsigned<typename NTraits::TCIntFromSizeLarger<(NMib::fg_GetHighestBitSetNoZero(mc_MaxNumSubSlabs - 1) + 1 + 7) / 8>::CType>::CType;

		static constexpr mint fs_CalculateNumAllocsPerSubSlab(mint _Index)
			{
				if (_Index == 0)
					return t_CParams::mc_SubSlabSize;

				mint LowestBitSet = fg_GetLowestBitSetNoZero(_Index);
				mint SizesPerLevelBits = fg_GetHighestBitSetNoZero(t_CParams::mc_NumSizesPerLevel);
				return t_CParams::mc_SubSlabSize * (mint(1) << (SizesPerLevelBits - LowestBitSet));
			}
		;

		static constexpr mint mc_MinNormalSlabBucket = NMib::fg_GetHighestBitSetNoZero(mc_MinNormalAllocSize);

		static constexpr mint mc_MaxAllocsPerSubSlab = []
			{
				return fg_MaxConstexpr((fs_CalculateNumAllocsPerSubSlab(tp_Indices) >> mc_MinNormalSlabBucket)...);
			}
			()
		;

		static constexpr mint mc_MaxAllocsPerSubSlabActual = []
			{
				return fg_MaxConstexpr((fs_CalculateNumAllocsPerSubSlab(tp_Indices) >> (tp_Indices == 0 ? mc_MinNormalSlabBucket : mc_MinNormalSlabBucket + 1))...);
			}
			()
		;

		static constexpr mint mc_MaxSubSlabMultipliedSize = []
			{
				return fg_MaxConstexpr((TCMemoryManagerParamsSizesPerLevel<t_CParams::mc_NumSizesPerLevel>::mc_SlabTypeInfo[tp_Indices].m_SubSlabMutiplier * t_CParams::mc_SubSlabSize)...);
			}
			()
		;

		static CSubSlabIndex constexpr mc_NumSubSlabs[t_CParams::mc_NumSizesPerLevel] =
			{
				CSubSlabIndex
				(

					TCMemoryManagerSlabShared<TCMemoryManagerParams>::template fs_CalculateSubSlabs<TCMemoryManagerParamsSizesPerLevel<t_CParams::mc_NumSizesPerLevel>::mc_SlabTypeInfo[tp_Indices].m_SubSlabMutiplier>()
				)...
			}
		;

		using CNumAllocsPerSubSlabIndex = typename NTraits::TCUnsigned
			<
				typename NTraits::TCIntFromSizeLarger<(NMib::fg_GetHighestBitSetNoZero(mc_MaxAllocsPerSubSlab) + 1 + 7) / 8>::CType
			>::CType
		;

		static constexpr CNumAllocsPerSubSlabIndex mc_NumAllocsPerSubSlab[t_CParams::mc_NumSizesPerLevel] =
			{
				(fs_CalculateNumAllocsPerSubSlab(tp_Indices) >> mc_MinNormalSlabBucket)...
			}
		;

		inline_always static uint32 fs_DivideBySlabMultiplier(uint32 _Offset, uint32 _SlabMultiplier);
		inline_always static uint32 fs_GetSlabTypeMetaSize(uint32 _SlabType);
	};

	struct CMemoryManagerParams_NoCommit : public CDefaultMemoryManagerParams
	{
		typedef CAllocator_VirtualNoCommit CAllocator;
	};

	struct CDefaultMemoryManagerParams_Tests : public CDefaultMemoryManagerParams
	{
		static constexpr bool mc_bBackgroundCleanup = false; // Background cleanups will hurt predictability
	};
}
