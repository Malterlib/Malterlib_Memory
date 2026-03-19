// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib::NMemory
{
	/////////////////////////////////////////////
	// Shared -> 2 byte

	template <typename t_CParams>
	void TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>::f_OnAlloc(TCMemoryManagerArena<t_CParams> &_Arena, void *_pAlloc)
	{
		umint AllocSize = m_Params.m_AllocSize;
		_Arena.f_OnAlloc((uint8 *)_pAlloc, AllocSize);
	}

	template <typename t_CParams>
	void TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>::f_OnFree(TCMemoryManagerArena<t_CParams> &_Arena, void *_pAlloc)
	{
		umint AllocSize = m_Params.m_AllocSize;
		_Arena.f_OnFree((uint8 *)_pAlloc);
		_Arena.f_OnFillFree((uint8 *)_pAlloc + sizeof(uint16), AllocSize - sizeof(uint16));
	}

	template <typename t_CParams>
	void TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>::f_OnFillFree(TCMemoryManagerArena<t_CParams> &_Arena)
	{
		umint AllocSize = m_Params.m_AllocSize;
		if (AllocSize <= sizeof(uint16))
			return; // All bytes are used, so no bytes to check

		uint8 *pArray = f_GetArray();

		umint NumAllocs = (t_CParams::mc_SubSlabSize - fg_AlignUp(sizeof(CParams), m_Params.m_Alignment)) / AllocSize;
		umint AllocSizeUint16 = AllocSize / sizeof(uint16);

		uint16 *pAlloc = (uint16 *)pArray;
		uint16 *pAllocEnd = pAlloc + AllocSizeUint16 * NumAllocs;
		for (; pAlloc != pAllocEnd; pAlloc += AllocSizeUint16)
			_Arena.f_OnFillFree((uint8 *)(pAlloc + 1), AllocSize - sizeof(*pAlloc));
	}

	template <typename t_CParams>
	bool TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>::f_OnCheckFree(TCMemoryManagerArena<t_CParams> &_Arena, void *_pAlloc, EMemoryManagerCheckFlag _Flags)
	{
		umint AllocSize = m_Params.m_AllocSize;
		return _Arena.f_OnCheckFree((uint8 *)_pAlloc + sizeof(uint16), AllocSize - sizeof(uint16), _Flags);
	}

	template <typename t_CParams>
	bool TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>::f_CheckFree(TCMemoryManagerArena<t_CParams> &_Arena, EMemoryManagerCheckFlag _Flags)
	{
		bool bError = false;

		uint16 iAlloc = m_Params.m_FirstFreeList;

		uint8 *pArray = f_GetArray();

		while (iAlloc != 0xFFFF)
		{
			uint16 *pAlloc = (uint16 *)(pArray + iAlloc * m_Params.m_AllocSize);

			if (f_OnCheckFree(_Arena, pAlloc, _Flags))
				bError = true;

			iAlloc = *pAlloc;
		}

		return bError;
	}

	template <typename t_CParams>
	TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>::TCMemoryManagerSubSlab_SmallSizeShared(umint _AllocSize)
	{
		m_Params.m_FirstFreeList = 0;
		m_Params.m_nAllocated = 0;
		m_Params.m_AllocSize = _AllocSize;
		m_Params.m_Alignment = 1 << fg_GetLowestBitSetNoZero(_AllocSize);

		uint8 *pArray = f_GetArray();

		umint NumAllocs = (t_CParams::mc_SubSlabSize - fg_AlignUp(sizeof(CParams), m_Params.m_Alignment)) / _AllocSize;

		umint AllocSizeUint16 = _AllocSize / sizeof(uint16);

		uint16 *pAlloc = (uint16 *)pArray;
		uint16 *pAllocEnd = pAlloc + AllocSizeUint16 * (NumAllocs - 1);
		for (uint16 i = 1; pAlloc != pAllocEnd; pAlloc += AllocSizeUint16, ++i)
			*pAlloc = i;

		*pAlloc = 0xFFFF;
	}

	template <typename t_CParams>
	inline_small uint8 *TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>::f_GetArray()
	{
		return fg_AlignUp((uint8 *)(this + 1), m_Params.m_Alignment);
	}

	template <typename t_CParams>
	inline_small void *TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>::f_Alloc(bool &_bFull)
	{
		uint16 iAlloc = m_Params.m_FirstFreeList;
		DMibFastCheck(iAlloc != 0xFFFF); // Should not be called on full
		uint8 *pArray = f_GetArray();
		uint16 *pAlloc = (uint16 *)(pArray + iAlloc * m_Params.m_AllocSize);
		uint16 NextMessage = *pAlloc;
		m_Params.m_FirstFreeList = NextMessage;
		++m_Params.m_nAllocated;
		_bFull = NextMessage == 0xFFFF;
		return pAlloc;
	}

	template <typename t_CParams>
	inline_small ESmallState TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>::f_Free(void *_pAlloc, umint _iAlloc)
	{
		uint16 iAlloc = m_Params.m_FirstFreeList;
		ESmallState SmallState;
		if (iAlloc == 0xFFFF)
			SmallState = ESmallState_WasFull;
		else if (m_Params.m_nAllocated == 1)
			SmallState = ESmallState_IsFullyFree;
		else
			SmallState = ESmallState_None;

		DMibFastCheck(_iAlloc < ((t_CParams::mc_SubSlabSize - fg_AlignUp(sizeof(CParams), m_Params.m_Alignment)) / m_Params.m_AllocSize));
		uint16 *pAlloc = (uint16 *)(_pAlloc);
		*pAlloc = iAlloc;
		m_Params.m_FirstFreeList = _iAlloc;
		--m_Params.m_nAllocated;
		return SmallState;
	}

	/////////////////////////////////////////////
	// Small -> 2 byte


	template <typename t_CParams, umint t_AllocSize>
	TCMemoryManagerSubSlab_SmallSize<t_CParams, t_AllocSize>::TCMemoryManagerSubSlab_SmallSize() : TCMemoryManagerSubSlab_SmallSizeShared<t_CParams>(t_AllocSize)
	{
	}

	/////////////////////////////////////////////
	// Small -> 1 byte

	template <typename t_CParams>
	void TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>::f_OnAlloc(TCMemoryManagerArena<t_CParams> &_Arena, void *_pAlloc)
	{
		_Arena.f_OnAlloc((uint8 *)_pAlloc, 1);
	}

	template <typename t_CParams>
	void TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>::f_OnFree(TCMemoryManagerArena<t_CParams> &_Arena, void *_pAlloc)
	{
		_Arena.f_OnFree((uint8 *)_pAlloc);
	}

	template <typename t_CParams>
	void TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>::f_OnFillFree(TCMemoryManagerArena<t_CParams> &_Arena)
	{
		// All bytes are used so no action
	}

	template <typename t_CParams>
	bool TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>::f_OnCheckFree(TCMemoryManagerArena<t_CParams> &_Arena, void *_pAlloc, EMemoryManagerCheckFlag _Flags)
	{
		return false;
	}

	template <typename t_CParams>
	bool TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>::f_CheckFree(TCMemoryManagerArena<t_CParams> &_Arena, EMemoryManagerCheckFlag _Flags)
	{
		return false;
	}

	template <typename t_CParams>
	TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>::TCMemoryManagerSubSlab_SmallSize()
	{
		uint8 *pArray = f_GetArray();

		m_Params.m_FirstFreeList = 0;
		m_Params.m_nAllocated = 0;
		for (umint i = 0; i < mc_NumAllocRegions - 1; ++i)
		{
			m_Params.m_FreeListsNext[i] = i + 1;
			m_Params.m_FreeLists[i] = 0;
			uint8 *pAlloc = pArray + i * 255;
			for (umint i = 0; i < 254; ++i)
				pAlloc[i] = i + 1;
			pAlloc[254] = 0xFF;
		}
		{
			umint nAllocs = mc_NumAllocs - (mc_NumAllocRegions - 1) * 255;
			m_Params.m_FreeListsNext[mc_NumAllocRegions - 1] = 0xFF;
			m_Params.m_FreeLists[mc_NumAllocRegions - 1] = 0;
			uint8 *pAlloc = pArray + (mc_NumAllocRegions - 1) * 255;
			for (umint i = 0; i < nAllocs - 1; ++i)
				pAlloc[i] = i + 1;
			pAlloc[nAllocs - 1] = 0xFF;
		}
	}

	template <typename t_CParams>
	uint8 *TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>::f_GetArray()
	{
		return (uint8 *)(this + 1);
	}

	template <typename t_CParams>
	inline_small void *TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>::f_Alloc(bool &_bFull)
	{
		uint8 iAllocRegion = m_Params.m_FirstFreeList;
		DMibFastCheck(iAllocRegion != 0xFF); // Should not be called on full
		uint8 iAlloc = m_Params.m_FreeLists[iAllocRegion];
		DMibFastCheck(iAlloc != 0xFF); // Should not be called on full
		uint8 *pArray = f_GetArray() + iAllocRegion * 255;

		uint8 *pAlloc = pArray + iAlloc;

		uint8 NextMessage = *pAlloc;
		m_Params.m_FreeLists[iAllocRegion] = NextMessage;
		if (NextMessage == 0xFF)
		{
			uint8 NextFreeList = m_Params.m_FreeListsNext[iAllocRegion];
			m_Params.m_FirstFreeList = NextFreeList;
			_bFull = NextFreeList == 0xFF;
		}
		else
			_bFull = false;
		++m_Params.m_nAllocated;
		return pAlloc;
	}

	template <typename t_CParams>
	inline_small ESmallState TCMemoryManagerSubSlab_SmallSize<t_CParams, 1>::f_Free(void *_pAlloc)
	{
		uint8 *pArray = f_GetArray();
		umint Offset = (uint8 *)_pAlloc - pArray;
		uint8 iAllocRegion = Offset / 255;
		uint16 iAllocNext = m_Params.m_FirstFreeList;

		ESmallState SmallState;
		if (iAllocNext == 0xFF)
			SmallState = ESmallState_WasFull;
		else if (m_Params.m_nAllocated == 1)
			SmallState = ESmallState_IsFullyFree;
		else
			SmallState = ESmallState_None;

		umint iAlloc2 = Offset - iAllocRegion * 255;
		DMibFastCheck(iAlloc2 < 255);
		uint8 *pAlloc = (uint8 *)(_pAlloc);
		*pAlloc = m_Params.m_FreeLists[iAllocRegion];
		m_Params.m_FreeLists[iAllocRegion] = iAlloc2;
		if (iAllocNext == 0xFF)
		{
			m_Params.m_FreeListsNext[iAllocRegion] = iAllocNext;
			m_Params.m_FirstFreeList = iAllocRegion;
		}
		--m_Params.m_nAllocated;
		return SmallState;
	}
}
