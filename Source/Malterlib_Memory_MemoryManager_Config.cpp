// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include "Malterlib_Memory_MemoryManager_Config.h"

namespace NMib
{
	namespace NMem
	{
		template <>
		CSlabTypeInfo const TCDefaultMemoryManagerParams<8>::ms_SlabTypeInfo[mc_NumSizesPerLevel] = {	{1},	{9},	{5},	{11},	{3},	{13},	{7},	{15}};

		template <>
		const uint16 TCDefaultMemoryManagerParams<8>::ms_NumSubSlabs[mc_NumSizesPerLevel] = {			4093,	455,	819,	372,	1365,	315,	585,	273};
		
		template <>
		const uint16 TCDefaultMemoryManagerParams<8>::ms_DivideMultiply[mc_NumSizesPerLevel] = {		1,		58255,	52429,	47663,	43691,	20165,	9363,	34953};
		template <>
		const uint8 TCDefaultMemoryManagerParams<8>::ms_DivideShift[mc_NumSizesPerLevel] = {			0,		19,		18,		19,		17,		18,		16,		19};
		
		template <>
		const uint16 TCDefaultMemoryManagerParams<8>::ms_NumAllocsPerSubSlab[mc_NumSizesPerLevel] = 
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
		CSlabTypeInfo const TCDefaultMemoryManagerParams<4>::ms_SlabTypeInfo[mc_NumSizesPerLevel] = {	{1},	{5},	{3},	{7}};

		template <>
		const uint16 TCDefaultMemoryManagerParams<4>::ms_NumSubSlabs[mc_NumSizesPerLevel] = {			4093,	819,	1365,	585};
		
		template <>
		const uint16 TCDefaultMemoryManagerParams<4>::ms_DivideMultiply[mc_NumSizesPerLevel] = {		1,		52429,	43691,	9363};
		template <>
		const uint8 TCDefaultMemoryManagerParams<4>::ms_DivideShift[mc_NumSizesPerLevel] = {			0,		18,		17,		16};
		
		template <>
		const uint16 TCDefaultMemoryManagerParams<4>::ms_NumAllocsPerSubSlab[mc_NumSizesPerLevel] = 
			{
				128 << 5
				, 512 << 5
				, 256 << 5
				, 512 << 5
			}
		;

		template <>
		CSlabTypeInfo const TCDefaultMemoryManagerParams<2>::ms_SlabTypeInfo[mc_NumSizesPerLevel] = {	{1},	{3}};

		template <>
		const uint16 TCDefaultMemoryManagerParams<2>::ms_NumSubSlabs[mc_NumSizesPerLevel] = {			4093,	1365};
		
		template <>
		const uint16 TCDefaultMemoryManagerParams<2>::ms_DivideMultiply[mc_NumSizesPerLevel] = {		1,		43691};
		template <>
		const uint8 TCDefaultMemoryManagerParams<2>::ms_DivideShift[mc_NumSizesPerLevel] = {			0,		17};
		
		template <>
		const uint16 TCDefaultMemoryManagerParams<2>::ms_NumAllocsPerSubSlab[mc_NumSizesPerLevel] = 
			{
				128 << 5
				, 256 << 5
			}
		;

		template <>
		CSlabTypeInfo const TCDefaultMemoryManagerParams<1>::ms_SlabTypeInfo[mc_NumSizesPerLevel] = {	{1}};

		template <>
		const uint16 TCDefaultMemoryManagerParams<1>::ms_NumSubSlabs[mc_NumSizesPerLevel] = {			4093};
		
		template <>
		const uint16 TCDefaultMemoryManagerParams<1>::ms_DivideMultiply[mc_NumSizesPerLevel] = {		1};
		template <>
		const uint8 TCDefaultMemoryManagerParams<1>::ms_DivideShift[mc_NumSizesPerLevel] = {			0};
		
		template <>
		const uint16 TCDefaultMemoryManagerParams<1>::ms_NumAllocsPerSubSlab[mc_NumSizesPerLevel] = 
			{
				128 << 5
			}
		;

	}
}
