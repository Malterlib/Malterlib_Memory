// Copyright © 2024 Favro Holding AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Container/Map>
#include <Mib/String/String>

namespace NMib::NMemory
{
	enum class EMemoryStatisticUnit
	{
		mc_Number
		, mc_Bytes
	};

	enum class EMemoryStatisticComparisonOperator
	{
		mc_None
		, mc_LessThan
		, mc_GreaterThan
		, mc_Equal
	};

	struct CSystemMemoryStatisticCharacteristics
	{
		template <typename tf_CStr>
		void f_Format(tf_CStr &o_Str) const;
		
		auto operator <=> (CSystemMemoryStatisticCharacteristics const &) const = default;

		static NStr::CStr fs_UnitToStr(EMemoryStatisticUnit _Unit);
		static NStr::CStr fs_ComparisonOperatorToStr(EMemoryStatisticComparisonOperator _Operator);

		EMemoryStatisticUnit m_Unit = EMemoryStatisticUnit::mc_Number;
		EMemoryStatisticComparisonOperator m_ValueComparisonOperator = EMemoryStatisticComparisonOperator::mc_None;

		uint64 m_WarningValue = 0;
		uint64 m_ErrorValue = 0;
	};

	struct CSystemMemoryStatistic
	{
		template <typename tf_CStr>
		void f_Format(tf_CStr &o_Str) const;

		auto operator <=> (CSystemMemoryStatistic const &) const = default;

		uint64 m_Value = 0;
		CSystemMemoryStatisticCharacteristics m_Characteristics;
	};

	struct CSystemMemoryStatistics
	{
		template <typename tf_CStr>
		void f_Format(tf_CStr &o_Str) const;

		NContainer::TCMap<NStr::CStr, CSystemMemoryStatistic> m_Statistics;
	};

	enum class EMemoryStatisticsDetailLevel : uint32
	{
		mc_Basic
		, mc_Detailed
		, mc_All
	};
}

namespace NMib::NMemory::NPlatform
{
	CSystemMemoryStatistics fg_Memory_GetStatistics(EMemoryStatisticsDetailLevel _DetailLevel);
}

#ifndef DMibPNoShortCuts
	using namespace NMib::NMemory;
#endif

#include "Malterlib_Memory_Platform.hpp"
