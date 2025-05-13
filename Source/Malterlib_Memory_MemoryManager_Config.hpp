// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	///
	/// Global
	/// ======

	template <typename tf_CMemoryManager>
	inline CDefaultMemoryManagerNotifier::CGlobal::CGlobal(tf_CMemoryManager & _MemMan)
	{
	}

	inline void CDefaultMemoryManagerNotifier::CGlobal::f_OnAlloc(uint8 *_pMemory, mint _nBytes)
	{
	}

	inline void CDefaultMemoryManagerNotifier::CGlobal::f_OnFree(uint8 *_pMemory)
	{
	}

	inline void CDefaultMemoryManagerNotifier::CGlobal::f_OnCommit(uint8 *_pMemory, mint _nBytes)
	{
	}

	inline void CDefaultMemoryManagerNotifier::CGlobal::f_OnDecommit(uint8 *_pMemory, mint _nBytes)
	{
	}

	///
	/// Arena
	/// =====

	inline CDefaultMemoryManagerNotifier::CArena::CArena(CGlobal *_pGlobal)
	{
	}

	inline void CDefaultMemoryManagerNotifier::CArena::f_OnAlloc(uint8 *_pMemory, mint _nBytes)
	{
	}

	inline void CDefaultMemoryManagerNotifier::CArena::f_OnFree(uint8 *_pMemory)
	{
	}

	inline void CDefaultMemoryManagerNotifier::CArena::f_OnFreeOtherThread(uint8 *_pMemory)
	{
	}

	inline void CDefaultMemoryManagerNotifier::CArena::f_OnFillFree(uint8 *_pMemory, mint _nBytes, EMemoryManagerCheckFlag _Flags)
	{
	}

	inline bool CDefaultMemoryManagerNotifier::CArena::f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, EMemoryManagerCheckFlag _Flags)
	{
		return false;
	}

	///
	/// Heap
	/// ====

	inline CDefaultMemoryManagerNotifier::CHeap::CHeap(CGlobal *_pGlobal)
	{
	}

	inline void CDefaultMemoryManagerNotifier::CHeap::f_OnAlloc(uint8 *_pMemory, mint _nBytes)
	{
	}

	inline void CDefaultMemoryManagerNotifier::CHeap::f_OnFree(uint8 *_pMemory)
	{
	}

	inline void CDefaultMemoryManagerNotifier::CHeap::f_OnFillFree(uint8 *_pMemory, mint _nBytes, EMemoryManagerCheckFlag _Flags)
	{
	}

	inline bool CDefaultMemoryManagerNotifier::CHeap::f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, EMemoryManagerCheckFlag _Flags)
	{
		return false;
	}

#ifndef DDocumentation_Doxygen
	template <typename t_CParams, mint ...tp_Indices>
	uint32 TCMemoryManagerParams<t_CParams, NMeta::TCIndices<tp_Indices...>>::fs_DivideBySlabMultiplier(uint32 _Offset, uint32 _SlabMultiplier)
	{
		using CConstants = TCMemoryManagerParamsSizesPerLevel<t_CParams::mc_NumSizesPerLevel>;
		DMibFastCheck(_Offset < (mc_SlabSize / t_CParams::mc_SubSlabSize));
		DMibFastCheck(_SlabMultiplier < t_CParams::mc_NumSizesPerLevel);
		uint32 Return = (_Offset * CConstants::mc_DivideMultiply[_SlabMultiplier]) >> CConstants::mc_DivideShift[_SlabMultiplier];
		DMibFastCheck(Return == _Offset / CConstants::mc_SlabTypeInfo[_SlabMultiplier].m_SubSlabMutiplier);
		return Return;
	}

	template <typename t_CParams, mint ...tp_Indices>
	uint32 TCMemoryManagerParams<t_CParams, NMeta::TCIndices<tp_Indices...>>::fs_GetSlabTypeMetaSize(uint32 _SlabType)
	{
		static constexpr mint c_MaxMetadataBlockSize = []
			{
				return fg_MaxConstexpr(fg_MemoryManagerSlabSize<TCMemoryManagerParams, tp_Indices>()...);
			}
			()
		;

		using namespace NMib::NTraits;
		using CMetadataBlockSize = typename TCUnsigned<typename TCIntFromSizeLarger<(NMib::fg_GetHighestBitSetNoZero(c_MaxMetadataBlockSize) + 1 + 7) / 8, true>::CType>::CType;

		static constexpr CMetadataBlockSize c_MetadataBlockSize[t_CParams::mc_NumSizesPerLevel] =
			{
				fg_MemoryManagerSlabSize<TCMemoryManagerParams, tp_Indices>()...
			}
		;

		return fg_AlignUp(c_MetadataBlockSize[_SlabType], t_CParams::mc_SubSlabSize);
	}
#endif
}
