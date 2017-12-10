// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

namespace NMib
{
	namespace NMem
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
		
		inline void CDefaultMemoryManagerNotifier::CArena::f_OnFillFree(uint8 *_pMemory, mint _nBytes)
		{
		}
		
		inline bool CDefaultMemoryManagerNotifier::CArena::f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, bool _bBreak)
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
		
		inline void CDefaultMemoryManagerNotifier::CHeap::f_OnFillFree(uint8 *_pMemory, mint _nBytes)
		{
		}
		
		inline bool CDefaultMemoryManagerNotifier::CHeap::f_OnCheckFree(uint8 *_pUntouchedMemory, mint _nUntouchedBytes, bool _bBreak)
		{
			return false;
		}

		template <>
		CSlabTypeInfo DMalterlibMemoryConstExprWorkaround TCDefaultMemoryManagerParams<8>::ms_SlabTypeInfo[mc_NumSizesPerLevel] = {	{1},	{9},	{5},	{11},	{3},	{13},	{7},	{15}};

		template <>
		DMalterlibMemoryConstExprWorkaround uint16 TCDefaultMemoryManagerParams<8>::ms_NumSubSlabs[mc_NumSizesPerLevel] = {			4093,	455,	819,	372,	1365,	315,	585,	273};
		
		template <>
		DMalterlibMemoryConstExprWorkaround uint16 TCDefaultMemoryManagerParams<8>::ms_DivideMultiply[mc_NumSizesPerLevel] = {		1,		58255,	52429,	47663,	43691,	20165,	9363,	34953};
		template <>
		DMalterlibMemoryConstExprWorkaround uint8 TCDefaultMemoryManagerParams<8>::ms_DivideShift[mc_NumSizesPerLevel] = {			0,		19,		18,		19,		17,		18,		16,		19};
		
		template <>
		DMalterlibMemoryConstExprWorkaround uint16 TCDefaultMemoryManagerParams<8>::ms_NumAllocsPerSubSlab[mc_NumSizesPerLevel] = 
			{
				128 << 5
				, 1024 << 5
				, 512 << 5
				, 1024 << 5
				, 256 << 5
				, 1024 << 5
				, 512 << 5
				, 1024 << 5
			}
		;

		template <>
		CSlabTypeInfo DMalterlibMemoryConstExprWorkaround TCDefaultMemoryManagerParams<4>::ms_SlabTypeInfo[mc_NumSizesPerLevel] = {	{1},	{5},	{3},	{7}};

		template <>
		DMalterlibMemoryConstExprWorkaround uint16 TCDefaultMemoryManagerParams<4>::ms_NumSubSlabs[mc_NumSizesPerLevel] = {			4093,	819,	1365,	585};
		
		template <>
		DMalterlibMemoryConstExprWorkaround uint16 TCDefaultMemoryManagerParams<4>::ms_DivideMultiply[mc_NumSizesPerLevel] = {		1,		52429,	43691,	9363};
		template <>
		DMalterlibMemoryConstExprWorkaround uint8 TCDefaultMemoryManagerParams<4>::ms_DivideShift[mc_NumSizesPerLevel] = {			0,		18,		17,		16};
		
		template <>
		DMalterlibMemoryConstExprWorkaround uint16 TCDefaultMemoryManagerParams<4>::ms_NumAllocsPerSubSlab[mc_NumSizesPerLevel] = 
			{
				128 << 5
				, 512 << 5
				, 256 << 5
				, 512 << 5
			}
		;

		template <>
		CSlabTypeInfo DMalterlibMemoryConstExprWorkaround TCDefaultMemoryManagerParams<2>::ms_SlabTypeInfo[mc_NumSizesPerLevel] = {	{1},	{3}};

		template <>
		DMalterlibMemoryConstExprWorkaround uint16 TCDefaultMemoryManagerParams<2>::ms_NumSubSlabs[mc_NumSizesPerLevel] = {			4093,	1365};
		
		template <>
		DMalterlibMemoryConstExprWorkaround uint16 TCDefaultMemoryManagerParams<2>::ms_DivideMultiply[mc_NumSizesPerLevel] = {		1,		43691};
		template <>
		DMalterlibMemoryConstExprWorkaround uint8 TCDefaultMemoryManagerParams<2>::ms_DivideShift[mc_NumSizesPerLevel] = {			0,		17};
		
		template <>
		DMalterlibMemoryConstExprWorkaround uint16 TCDefaultMemoryManagerParams<2>::ms_NumAllocsPerSubSlab[mc_NumSizesPerLevel] = 
			{
				128 << 5
				, 256 << 5
			}
		;

		template <>
		CSlabTypeInfo DMalterlibMemoryConstExprWorkaround TCDefaultMemoryManagerParams<1>::ms_SlabTypeInfo[mc_NumSizesPerLevel] = {	{1}};

		template <>
		DMalterlibMemoryConstExprWorkaround uint16 TCDefaultMemoryManagerParams<1>::ms_NumSubSlabs[mc_NumSizesPerLevel] = {			4093};
		
		template <>
		DMalterlibMemoryConstExprWorkaround uint16 TCDefaultMemoryManagerParams<1>::ms_DivideMultiply[mc_NumSizesPerLevel] = {		1};
		template <>
		DMalterlibMemoryConstExprWorkaround uint8 TCDefaultMemoryManagerParams<1>::ms_DivideShift[mc_NumSizesPerLevel] = {			0};
		
		template <>
		DMalterlibMemoryConstExprWorkaround uint16 TCDefaultMemoryManagerParams<1>::ms_NumAllocsPerSubSlab[mc_NumSizesPerLevel] = 
			{
				128 << 5
			}
		;
		
		template <mint t_nSizesPerLevel>
		uint32 TCDefaultMemoryManagerParams<t_nSizesPerLevel>::fs_DivideBySlabMultiplier(uint32 _Offset, uint32 _SlabMultiplier)
		{
			DMibFastCheck(_Offset < (mc_SlabSize / mc_SubSlabSize));
			DMibFastCheck(_SlabMultiplier < mc_NumSizesPerLevel);
			uint32 Return = (_Offset * ms_DivideMultiply[_SlabMultiplier]) >> ms_DivideShift[_SlabMultiplier];
			DMibFastCheck(Return == _Offset / ms_SlabTypeInfo[_SlabMultiplier].m_SubSlabMutiplier);
			return Return;
		}
	}
}
