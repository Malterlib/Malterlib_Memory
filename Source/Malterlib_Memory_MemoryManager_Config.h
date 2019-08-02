// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

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

			void f_OnFillFree(uint8 *_pMemory, mint _nBytes);
			bool f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, bool _bBreak);
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

			void f_OnFillFree(uint8 *_pMemory, mint _nBytes);
			bool f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, bool _bBreak);
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
		};


	};

	template <mint t_nSizesPerLevel, mint t_SlabType>
	struct TCDefaultMemoryManagerParams_GetSlabInfo
	{
	};

	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<8, 0>
	{
		static constexpr uint8 mc_Multiplier = 1;
	};
	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<8, 1>
	{
		static constexpr uint8 mc_Multiplier = 9;
	};
	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<8, 2>
	{
		static constexpr uint8 mc_Multiplier = 5;
	};
	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<8, 3>
	{
		static constexpr uint8 mc_Multiplier = 11;
	};
	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<8, 4>
	{
		static constexpr uint8 mc_Multiplier = 3;
	};
	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<8, 5>
	{
		static constexpr uint8 mc_Multiplier = 13;
	};
	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<8, 6>
	{
		static constexpr uint8 mc_Multiplier = 7;
	};
	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<8, 7>
	{
		static constexpr uint8 mc_Multiplier = 15;
	};

	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<4, 0>
	{
		static constexpr uint8 mc_Multiplier = 1;
	};
	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<4, 1>
	{
		static constexpr uint8 mc_Multiplier = 5;
	};
	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<4, 2>
	{
		static constexpr uint8 mc_Multiplier = 3;
	};
	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<4, 3>
	{
		static constexpr uint8 mc_Multiplier = 7;
	};

	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<2, 0>
	{
		static constexpr uint8 mc_Multiplier = 1;
	};
	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<2, 1>
	{
		static constexpr uint8 mc_Multiplier = 3;
	};

	template <>
	struct TCDefaultMemoryManagerParams_GetSlabInfo<1, 0>
	{
		static constexpr uint8 mc_Multiplier = 1;
	};

#ifdef DCompiler_MSVC
#define DMalterlibMemoryConstExprWorkaround const
#else
#define DMalterlibMemoryConstExprWorkaround constexpr
#endif

	template <mint t_nSizesPerLevel>
	struct TCDefaultMemoryManagerParams
	{
		static_assert(t_nSizesPerLevel == 8 || t_nSizesPerLevel == 4 || t_nSizesPerLevel == 2 || t_nSizesPerLevel == 1, "Unsupported option");

		static constexpr mint mc_NumSizesPerLevel = t_nSizesPerLevel;
		static constexpr mint mc_MaxSlabAllocSize = 512*1024 - (256*1024) / mc_NumSizesPerLevel;
		static constexpr mint mc_SizesPerLevelShift = TCHighestBitSetCorrect<mint, t_nSizesPerLevel>::mc_Value;

		static constexpr mint mc_SubSlabSize = 4 * 1024;						// Should be the page size
		static constexpr mint mc_SlabSize = mc_SubSlabSize * 1024 * 4;			// Carefully choosen to minimize waste in different subslab types
		static constexpr mint mc_NumSubSlabs = mc_SlabSize / mc_SubSlabSize;
		static constexpr mint mc_MaxHeapAllocSize = mc_SlabSize;
		static constexpr mint mc_HeapChunkSize = mc_SlabSize * 2;
		static constexpr mint mc_HeapBlockSize = 64*1024;
		static constexpr bool mc_bRandomizeSlabHeader = false;
		static constexpr bool mc_bBackgroundCleanup = true;
		static constexpr EDeferCleanup mc_DeferCleanup = (EDeferCleanup)(EDeferCleanup_Allocs | EDeferCleanup_Commit | EDeferCleanup_OneSizeBlocks);

		static constexpr uint32 mc_BackgroundCleanupLifetime = 500; // The number of milleseconds that garbage should be kept before being cleaned up.
		static constexpr uint32 mc_BackgroundCleanupLifetimeDecommit = 10*1000; // The number of milleseconds that garbage pages should be kept before being decommitted.

		static constexpr mint mc_NumSizeLevels = TCHighestBitSetCorrect<mint, mc_MaxSlabAllocSize>::mc_Value + 1;
		static constexpr mint mc_NumNormalSizeLevels = mc_NumSizeLevels - 4;

		static constexpr EAllocationFlag mc_AllocationFlags = EAllocationFlag_None;
		static constexpr bool mc_bFullGarbageCollect = false;

		static DMalterlibMemoryConstExprWorkaround uint16 ms_NumSubSlabs[mc_NumSizesPerLevel];
		static DMalterlibMemoryConstExprWorkaround CSlabTypeInfo ms_SlabTypeInfo[mc_NumSizesPerLevel];
		static DMalterlibMemoryConstExprWorkaround uint16 ms_DivideMultiply[mc_NumSizesPerLevel];
		static DMalterlibMemoryConstExprWorkaround uint8 ms_DivideShift[mc_NumSizesPerLevel];
		static DMalterlibMemoryConstExprWorkaround uint16 ms_NumAllocsPerSubSlab[mc_NumSizesPerLevel];

		typedef CAllocator_Virtual CAllocator;
		typedef CDefaultMemoryManagerNotifier CNotifier;

		template <int32 t_SlabType>
		struct TCGetSlabInfo
		{
			static constexpr uint8 mc_Multiplier = TCDefaultMemoryManagerParams_GetSlabInfo<t_nSizesPerLevel, t_SlabType>::mc_Multiplier;
		};

		inline_always static uint32 fs_DivideBySlabMultiplier(uint32 _Offset, uint32 _SlabMultiplier);
	};

	typedef TCDefaultMemoryManagerParams<8> CDefaultMemoryManagerParams;

	struct CDefaultMemoryManagerParams_NoCommit : public CDefaultMemoryManagerParams
	{
		typedef CAllocator_VirtualNoCommit CAllocator;

	};

	struct CDefaultMemoryManagerParams_Tests : public TCDefaultMemoryManagerParams<8>
	{
		static constexpr bool mc_bBackgroundCleanup = false; // Background cleanups will hurt predictability
	};
}
